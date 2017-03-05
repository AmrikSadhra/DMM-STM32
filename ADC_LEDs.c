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



double voltageLookup[4] = {10, 1, 0.1, 0.01};

//Global variables

//For Header File
void initialise_Peripherals(void);


/* Function to intiialise all used peripherals    */
void initialise_Peripherals(void){ 
	ADC1_init();
	lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, BUFFER_SIZE);
	serial_init(9600);
	bluetooth_init(9600);
	switch_init();
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
		
		//Read Averaged and ranged ADC1 value
		double ADC1_valueScaled = read_ADC1();
		//Send packet to Multimeter App containing multimeter value and range
		sendPacket(1, ADC1_valueScaled, ADC1_currentRange);

		//Blit to LED's
		Delay(100.0);
  }
	
}


