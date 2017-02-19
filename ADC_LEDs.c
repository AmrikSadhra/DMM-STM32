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
uint8_t ADC1_currentRange = 1;
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
		bool rangeSwitch = false;
		static uint8_t ADC1_prevRange;
	
		//If RAW value greater than max value it can return - switch tolerance, probably time to change range (unless we maxed on range)
		if((adc1_raw  > UINT16_MAX - SWITCHING_RANGE_TOLERANCE)&&(ADC1_currentRange != 4)){
				ADC1_currentRange++;	
				rangeSwitch = true;
			} else if((adc1_raw  < SWITCHING_RANGE_TOLERANCE)&&(ADC1_currentRange != 0)){//If we're close (and not at lowest range)
			ADC1_currentRange--;
			rangeSwitch = true;
		}
			
		if(rangeSwitch){
			#ifdef DEBUG	
				printf("[Hardware Subsystem] ADC1 Range switch requested. Switching, raw ADC read: %d. Old Range: %d New Range: %d\r\n~", adc1_raw, ADC1_prevRange, ADC1_currentRange);
			#endif
			
			//Clear Range switch bits
			GPIOE->ODR &= ~(7UL << 3);
			
		switch(ADC1_currentRange){
				case 0:
					//Do nothing for Range 0 as pins already cleared
					break;			
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
				//Just to test a pretty LED, also we need EN pin so this will be used at *some* point
				case 4:
					//Range 4
					GPIOE->ODR |= (4UL << 3);
					break;
			}
			ADC1_prevRange = ADC1_currentRange;
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
		uint16_t ADC1_valueRaw = read_ADC1_raw();
		double ADC1_valueScaled = scale_ADC1(ADC1_valueRaw);
		
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


