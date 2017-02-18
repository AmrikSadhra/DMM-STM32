/*----------------------------------------------------------------------------
 * Name:    DMM.c 																			*/

//Provided Libraries
#include <stdio.h>
#include "STM32F4xx.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "LED.h"
#include "ADC.h"
#include "BTN.h"
//Made with Love (by us)
#include "serial.h"
#include "packet.h"
#include "utils.h"
#include "queue.h"
//Stolen
#include "sig_gen.h"

#define BUFFER_SIZE 128
#define DEBUG 1

#define SWITCHING_RANGE_TOLERANCE 20000

//Global variables
int ADC1_currentRange = 1;
//For Header File
void initialise_Peripherals(void);

void Autorange_init(){
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, & GPIO_InitStruct);
}
/* Function to Automatically set the range dependent upon ADC1 value */
void autoRange(uint16_t adc1_raw){
	
		#ifdef DEBUG
				printf("[Hardware Subsystem] ADC1 Range switch requested. Switching, raw ADC read: %d. Old Range: %d ", adc1_raw, ADC1_currentRange);
		#endif
		
		//If RAW value greater than max value it can return - switch tolerance, probably time to change range
		if(adc1_raw  > UINT16_MAX - SWITCHING_RANGE_TOLERANCE){
				ADC1_currentRange++;	
				//Clear Range switch bits
				GPIOE->ODR &= ~(3UL << 3);
			} else if(adc1_raw  < SWITCHING_RANGE_TOLERANCE){//If we're close 
			ADC1_currentRange--;
		}
			
		#ifdef DEBUG
				printf("New Range: %d\r\n~",	ADC1_currentRange);
		#endif
		
		switch(ADC1_currentRange){
				case 1:
					//Range 1
					GPIOE->ODR |= (1UL << 3);
					break;
				case 2:
					//Range 2
					GPIOE->ODR |= (2UL << 3);
					break;
				case 3:
					//Range 3
					GPIOE->ODR |= (3UL << 3);
					break;
			}
}

/* Function to intiialise all used peripherals    */
void initialise_Peripherals(void){
  BTN_Init();  
	ADC1_init();
	Autorange_init();
	lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, BUFFER_SIZE);
	serial_init(9600);
	bluetooth_init(9600);
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }
	
	initialise_Peripherals();
	
	lcd_clear_display();
	lcd_write_string("Multimeter Starting..", 0, 0);
	
  while(1) {                                    /* Loop forever               */
		double ADC1_valueRaw = read_ADC1_raw();
		double ADC1_valueScaled = scale_ADC1( ADC1_valueRaw);
		
		autoRange(ADC1_valueRaw);
		
		#ifdef DEBUG
			printf("[Hardware Subsystem] ADC_1 Scaled Voltage %f\r\n~", ADC1_valueScaled);
			sendPacket(1, ADC1_valueScaled, 1);
		#endif
		
		lcd_clear_display();
		lcd_write_string(DequeueString(debugQueue), 0, 0);
		
		//Blit to LED's
		Delay(100.0);
  }
}


