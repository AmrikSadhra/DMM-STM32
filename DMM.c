/*------------------------------------------------------*
 * Name:    DMM.c 																			*
 *------------------------------------------------------*/
#include "DMM.h"

//Provided Libraries
#include <stdio.h>
#include "STM32F4xx.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "LED.h"
#include "ADC.h"
#include "misc.h"
#include "math.h"
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
bool BluetoothMode = false;
void menu(uint8_t,double);
//Global variables


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
		//sendPacket(1, ADC1_valueScaled, ADC1_currentRange);
		menu(menuPosition,ADC1_valueScaled);
		//Pull commands out of the App buffer
		//lcd_write_string(DequeueString(debugQueue), 0, 0);

		//Blit to LED's
		Delay(100.0);
  }
	
}

void menu(uint8_t menuPosition,double scaledInput){
	static uint8_t prev_menuPosition;
	lcd_clear_display();
	//detecting if menu selection has changed
	if(menuPosition != prev_menuPosition){
		prev_menuPosition = menuPosition;
	}
	double current =0;
	double resistance = 0;
	char lcd_line1[16] ="ERROR";
	char lcd_line2[16] ="ERROR";
	switch(menuPosition){
		case 1: //voltage
			sprintf(lcd_line2,"%lf",scaledInput);
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Volt:0->10V");
					break;
				case 1:
					sprintf(lcd_line1,"Volt:0->1V");
					break;
				case 2:
					sprintf(lcd_line1,"Volt:0->100mV");
					break;
				case 3:
					sprintf(lcd_line1,"Volt:0->10mV");
					break;
				default://Invalid range
					ADC1_currentRange =0;
					break;
			}
			break;
		case 2://current
			current = scaledInput/10;
			sprintf(lcd_line2,"%lf",current);
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Amp:-1->1A");
					break;
				case 1:
					sprintf(lcd_line1,"Amp:-100m->100mA");
					break;
				case 2:
					sprintf(lcd_line1,"Amp:-10m->10mV");
					break;
				case 3:
					sprintf(lcd_line1,"Amp:-1m->1mV");
					break;
				default://Invalid range
					ADC1_currentRange =0;
					break;
			}
			break;
		case 3://resistance
			resistance = fabs(scaledInput)/0.000010;
			sprintf(lcd_line2,"%lf",resistance);
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Res:0->1M Ohm");
					break;
				case 1:
					sprintf(lcd_line1,"Res:0->100k Ohm");
					break;
				case 2:
					sprintf(lcd_line1,"Res:0->10k Ohm");
					break;
				case 3:
					sprintf(lcd_line1,"Res:0->1k Ohm");
					break;
				default://invalid range
					ADC1_currentRange =0;
					break;
			}
			break;
		case 4://attempt bluetooth
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		default:
			break;
	}
	lcd_write_string(lcd_line1, 0, 0);
	lcd_write_string(lcd_line2, 1, 0);
}


