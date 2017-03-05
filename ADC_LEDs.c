/*----------------------------------------------------------------------------
 * Name:    DMM.c 																			*/

//Provided Libraries
#include <stdio.h>
#include "STM32F4xx.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "LED.h"
#include "ADC.h"
#include "misc.h"
//Made with Love (by us)
#include "serial.h"
#include "packet.h"
#include "utils.h"
#include "queue.h"
#include "switches.h"
//Stolen
#include "sig_gen.h"

#define BUFFER_SIZE 128
#define DEBUG 1

#define SWITCHING_RANGE_TOLERANCE 2000

double voltageLookup[4] = {10, 1, 0.1, 0.01};

//Global variables
uint8_t ADC1_currentRange = 0;
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
  GPIO_Init(GPIOE, &GPIO_InitStruct);
}


/* Function to Automatically set the range dependent upon ADC1 value */
void autoRange(uint16_t adc1_raw){
		bool rangeSwitch = false;
		static uint8_t ADC1_prevRange;
	
		//If RAW value greater than max value it can return - switch tolerance, probably time to change range (unless we maxed on range)
		if(((adc1_raw  > UINT16_MAX - SWITCHING_RANGE_TOLERANCE)||(adc1_raw  < SWITCHING_RANGE_TOLERANCE))&&(ADC1_currentRange != 0)){
				ADC1_currentRange--;	
				rangeSwitch = true;
			} else if((adc1_raw  < UINT16_MAX*0.55)&&(adc1_raw  > UINT16_MAX*0.45)&&(ADC1_currentRange != 3)){//If we're close (and not at lowest range)
			ADC1_currentRange++;
			rangeSwitch = true;
		}
			

		if(rangeSwitch){
			#ifdef DEBUG	
				printf("[Hardware Subsystem] ADC1 Range switch requested. Switching, raw ADC read: %d. Old Range: %d New Range: %d\r\n~", adc1_raw, ADC1_prevRange, ADC1_currentRange);
			#endif
			
			//Clear Range switch bits
			GPIOE->ODR &= ~(7UL << 3);
			
		switch(ADC1_currentRange){
				case 0://10v in 
					//Do nothing for Range 0 as pins already cleared
					break;			
				case 1://1v in 
					//Range 1
					GPIOE->ODR |= (1UL << 3);
					break;
				case 2://0.1v
					//Range 2
					GPIOE->ODR |= (2UL << 3);
					break;
				case 3://0.01v
					//Range 3
					GPIOE->ODR |= (3UL << 3);
					break;
			}
			ADC1_prevRange = ADC1_currentRange;
		}
}


/* Function to intiialise all used peripherals    */
void initialise_Peripherals(void){ 
	ADC1_init();
	Autorange_init();
	lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, BUFFER_SIZE);
	serial_init(9600);
	bluetooth_init(9600);
	switch_init();
}

double runningAverage(uint8_t numSamples){
	uint16_t adc_raw = 0;

		for(int i=0;i<numSamples;i++){
			adc_raw += read_ADC1_raw();
		}
		double temp2 = (double)adc_raw;
		temp2 /= numSamples;
		return temp2;
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
		//uint16_t ADC1_valueRaw = 0;
		
		//Running Average
		double temp2 = runningAverage(10);
		
		double ADC1_valueScaled = 0.0f;
		
		//autoRange(ADC1_valueRaw);
		ADC1_currentRange=0;
		switch(ADC1_currentRange){
			case 0:
				ADC1_valueScaled = map(temp2, 0, ADC_VALUE_3V, -10, 10);
				break;
			case 1:
				ADC1_valueScaled = map(temp2, 0, ADC_VALUE_3V, -1, 1);
				break;
			case 2:
				ADC1_valueScaled = map(temp2, 0, ADC_VALUE_3V, -0.1, 0.1);
				break;
			case 3:
				ADC1_valueScaled = map(temp2, 0, ADC_VALUE_3V, -0.01, 0.01);
				break;
		}
		
		#ifdef DEBUG
			printf("[Hardware Subsystem] ADC_1 Scaled Voltage %f\r\n~", ADC1_valueScaled);
			sendPacket(1, ADC1_valueScaled, 1);
		#endif

		//Blit to LED's
		Delay(100.0);
  }
	
}


