#include "serial.h"
#include "stdbool.h"

//-------- INTERRUPT GLOBALS ----------
Queue *bluetoothQueue, *debugQueue;
bool isPacket = false;
int iU, jU, iB, jB; //Indexes for UART and BT buffers

char debugBuf[MAX_SERIAL_IN_LENGTH + 1] = {'\0'};
char bluetoothBuf[MAX_SERIAL_IN_LENGTH + 1] = {'\0'};

void serial_init(uint32_t baudRate) {
	debugQueue = QueueConstructor(MAX_DBG_BUF_SIZE, "Debug Queue");
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  //This is for the GPIO pins used as TX and RX
  GPIO_InitTypeDef GPIO_InitStruct;

  /* This sequence sets up the TX and RX pins 
   * so they work correctly with the USART1 peripheral
   */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // Pins 2 (TX) and 3 (RX) are used
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; // the pins are configured as alternate function so the USART peripheral has access to them
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // this defines the IO speed and has nothing to do with the baudrate!
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // this defines the output type as push pull mode (as opposed to open drain)
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // this activates the pullup resistors on the IO pins
  GPIO_Init(GPIOA, & GPIO_InitStruct); // now all the values are passed to the GPIO_Init() function which sets the GPIO registers

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); //
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

  USART_InitTypeDef USART_InitStruct; // this is for the USART2 initilization

  USART_InitStruct.USART_BaudRate = baudRate; // the baudrate is set to the value we passed into this init function
  USART_InitStruct.USART_WordLength = USART_WordLength_8b; // we want the data frame size to be 8 bits (standard)
  USART_InitStruct.USART_StopBits = USART_StopBits_1; // we want 1 stop bit (standard)
  USART_InitStruct.USART_Parity = USART_Parity_No; // we don't want a parity bit (standard)
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
  USART_Init(USART2, & USART_InitStruct); // again all the properties are passed to the USART_Init function which takes care of all the bit setting

  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // enable the USART2 receive interrupt 

  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; // we want to configure the USART2 interrupts
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // this sets the priority group of the USART1 interrupts
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // this sets the subpriority inside the group
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // the USART2 interrupts are globally enabled
  NVIC_Init( & NVIC_InitStructure); // the properties are passed to the NVIC_Init function which takes care of the low level stuff	

  //Finally this enables the complete USART1 peripheral
  USART_Cmd(USART2, ENABLE);
}

void bt_send_str(char * str) {
  int i = 0;

  while (str[i] != 0) {
    USART3->DR = str[i++];
    while ((USART3->SR & USART_SR_TXE) == 0);
  }
}

int uart_rec(void) {
  while ((USART2->SR & USART_SR_RXNE) == 0);
  return USART2->DR & 0x1FF;
}

int bt_rec(void) {
  while ((USART3->SR & USART_SR_RXNE) == 0);
  return USART3->DR & 0x1FF;
}

void USART2_IRQHandler(void) {
  if (USART_GetITStatus(USART2, USART_IT_RXNE)) {
    iU = uart_rec();
		
		//Signify start of packet
    if (iU == '<') {
      isPacket = true;
    }

    if (isPacket) {
      debugBuf[jU] = iU;
      jU++; //Bump the write index for next write

      if (iU == '>') {
        isPacket = false;
        jU = 0;
				EnqueueString(debugQueue, debugBuf);
      }
    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
  }
}
 //Check if interrupt was because data is received
   

void bluetooth_init(uint32_t baudRate) {
	bluetoothQueue = QueueConstructor(MAX_BT_BUF_SIZE, "Bluetooth Queue");
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, & GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3); //
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

  USART_InitTypeDef USART_InitStruct;

  USART_InitStruct.USART_BaudRate = baudRate;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART3, & USART_InitStruct);

  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( & NVIC_InitStructure);

  USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void) {
  if (USART3->SR & USART_SR_RXNE) {
      iB = bt_rec();
		
      if (jB == 10) {
        bluetoothBuf[jB] = iB;
				bluetoothBuf[jB] = '\0';
        jB = 0;
				EnqueueString(bluetoothQueue, bluetoothBuf);
      } else {
        bluetoothBuf[jB++] = iB;
      }
    }
}

void printBus(uint32_t bus) {
  char buf[sizeof(bus) * 8];
  char * busString = int2bin(bus, buf, (sizeof(bus) * 8) - 1);
  printf("%s\n\r", busString);
}
