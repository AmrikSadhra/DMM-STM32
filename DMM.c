/*----------------------------------------------------------------------------
 * Name:    DMM.c 																			*/

#include <stdio.h>
#include "STM32F4xx.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "LED.h"
#include "ADC.h"
#include "serial.h"
#include "utils.h"

#define BUFFER_SIZE 128
#define ADC_IN_MIN 0
#define ADC_IN_MAX 3
#define DEBUG 1


//For Header File
void initialise_Peripherals(void);


volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */
/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;
}

/*----------------------------------------------------------------------------
  delays number of tick Systicks (happens every 1 ms)
 *----------------------------------------------------------------------------*/
void Delay (uint32_t dlyTicks) {                                              
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}


/*----------------------------------------------------------------------------
  Function that initializes Button pins
 *----------------------------------------------------------------------------*/
void BTN_Init(void) {

  RCC->AHB1ENR  |= ((1UL <<  0) );              /* Enable GPIOA clock         */

  GPIOA->MODER    &= ~((3UL << 2*0)  );         /* PA.0 is input              */
  GPIOA->OSPEEDR  &= ~((3UL << 2*0)  );         /* PA.0 is 50MHz Fast Speed   */
  GPIOA->OSPEEDR  |=  ((2UL << 2*0)  ); 
  GPIOA->PUPDR    &= ~((3UL << 2*0)  );         /* PA.0 is no Pull up         */
}

/*----------------------------------------------------------------------------
  Function that read Button pins
 *----------------------------------------------------------------------------*/
uint32_t BTN_Get(void) {

 return (GPIOA->IDR & (1UL << 0));
}

/* Function to intiialise ADC1    */


void initialise_Peripherals(void){
  BTN_Init();  
	ADC1_init();
	//Initialise LCD
	lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, BUFFER_SIZE);
	serial_init();
	bluetooth_init();
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
 
	double ADC1_valueScaled = 0;
	
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }
	

	initialise_Peripherals();
	//Take 5 samples 
	double calibrationValue = calibrate_ADC1();
	lcd_clear_display();
		lcd_write_string("Meow", 0, 0);
 
  while(1) {                                    /* Loop forever               */
		//Scale the 32 bit ADC input to between 0 and 3v
		ADC1_valueScaled = map(read_ADC1() - calibrationValue, 0, UINT16_MAX, ADC_IN_MIN, ADC_IN_MAX);

		#ifdef DEBUG
			//printf("[Hardware Subsystem] ADC_1 Scaled Voltage %f\n~", ADC1_valueScaled);
			bt_send_str("Banana\n");
		#endif
		
		//Blit to LED's
		GPIOD->ODR = ADC1_valueScaled * 16;
		Delay(100.0);
  }
  
}

