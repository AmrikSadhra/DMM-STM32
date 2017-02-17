/*----------------------------------------------------------------------------
 * Name:    DMM.c 																			*/

#include <stdio.h>
#include "STM32F4xx.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "LED.h"
#include "ADC.h"
#include "serial.h"
#include "packet.h"
#include "utils.h"
#include "sig_gen.h"
#include "queue.h"

#define BUFFER_SIZE 128
#define DEBUG 1

//For Header File
void initialise_Peripherals(void);


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
	lcd_write_string("Meow", 0, 0);
 

  while(1) {                                    /* Loop forever               */
		double ADC1_valueScaled = read_ADC1();

		#ifdef DEBUG
			printf("[Hardware Subsystem] ADC_1 Scaled Voltage %f\r\n~", ADC1_valueScaled);
			sendPacket(1, ADC1_valueScaled, 1);
		#endif
		
		lcd_clear_display();
		lcd_write_string(DequeueString(debugQueue), 0, 0);
		
		//Blit to LED's
		GPIOD->ODR = ADC1_valueScaled * 16;
		Delay(100.0);
  }
  
}

