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
#include "dac.h"

#define BUFFER_SIZE 128
#define DEBUG 1
bool BluetoothMode = false;
void menu(uint8_t,double);

//Stage Initialisation check booleans
bool alphaInit = false;
bool betaInit = false;
bool gammaInit = false;

//Global variables

void mode_switch_init(){
		//Initialisation of stage Alpha
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
			GPIO_InitTypeDef GPIO_InitStruct2;
			GPIO_InitStruct2.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
			GPIO_InitStruct2.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct2.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStruct2.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStruct2.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_Init(GPIOB, &GPIO_InitStruct2);
	
			//Initialisation of Stage Beta
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
			GPIO_InitTypeDef GPIO_InitStruct;
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 ;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/* Function to intiialise all used peripherals    */
void initialise_Peripherals(void){ 
			ADC1_init();
			lcd_init(LCD_LINES_TWO, LCD_CURSOR_OFF, LCD_CBLINK_OFF, BUFFER_SIZE);
			serial_init(9600);
			bluetooth_init(9600);
			switch_init();
			mode_switch_init();
	
			
			//Connect Read stage to Voltage
			stageAlpha(1);
}

//has three stages: ground, voltage,capacitance
void stageAlpha(int mode){	
			if(!alphaInit){
				mode_switch_init();
				alphaInit = true;
			}
			//Clear Range switch bits
			GPIOB->ODR &= ~(3UL << 4);
			switch(mode){
				case 0:
					GPIOB->ODR &= ~(3UL << 4);
					break;
				case 1:
					GPIOB->ODR |= (1UL << 4);//setting mode to voltage
					break;
				case 2:
					GPIOB->ODR |= (2UL << 4);
					break;
				default:
					break;
			}
}

void stageBeta(int mode){
	if(!betaInit){
				mode_switch_init();
				betaInit = true;
			}
	//Clear bits 
	GPIOE->ODR &= ~(3UL << 5);
	switch(mode){
		case 0://Voltage
			GPIOE->ODR &= ~(3UL << 5);
			break;
		case 1://Current
			GPIOE->ODR |= (1UL << 5);
			break;
		case 2://RMS
			GPIOE->ODR |= (2UL << 5);
			break;
		case 3://Resistance
			GPIOE->ODR |= (3UL << 5);
			break;
		default:
			break;
	}
}

void stageGamma(int mode){
	if(!gammaInit){
				mode_switch_init();
				gammaInit = true;
			}
	switch(mode){
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
	
	dac_initialise();
  
	while(1) {                                    /* Loop forever               */
		//Read Averaged and ranged ADC1 value
		double ADC1_valueScaled = read_ADC1();
		menu(menuPosition, ADC1_valueScaled);
  }
	
}

void menu(uint8_t menuPosition, double scaledInput){
	//Bluetooth Override of Buttons
	char *bluetoothSwitchPacket = DequeueString(bluetoothQueue);
	
//	if(strcmp(bluetoothSwitchPacket,"<m:1>") == 0) menuPosition = 1;
//	if(strcmp(bluetoothSwitchPacket,"<m:2>") == 0) menuPosition = 2;
//	if(strcmp(bluetoothSwitchPacket,"<m:3>") == 0) menuPosition = 3;

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
			stageBeta(0);
			
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Volt:0->10V");
					sprintf(lcd_line2,"%lf V",scaledInput);
					break;
				case 1:
					sprintf(lcd_line1,"Volt:0->1V");
					sprintf(lcd_line2,"%lf V",scaledInput);
					break;
				case 2:
					sprintf(lcd_line1,"Volt:0->100mV");
					sprintf(lcd_line2,"%lf mV",scaledInput*1000);
					break;
				case 3:
					sprintf(lcd_line1,"Volt:0->10mV");
					sprintf(lcd_line2,"%lf mV",scaledInput*1000);
					break;
				default://Invalid range
					ADC1_currentRange =0;
					break;
			}
			sendPacket(1, scaledInput, ADC1_currentRange);
			break;
		case 2://current
			stageBeta(1);
			current = scaledInput/10;
			switch(ADC1_currentRange){
				case 0:
					sprintf(lcd_line1,"Amp:-1->1A");
					sprintf(lcd_line2,"%lf A",current);
					break;
				case 1:
					sprintf(lcd_line1,"Amp:-100m->100mA");
					sprintf(lcd_line2,"%lf A",current);
					break;
				case 2:
					sprintf(lcd_line1,"Amp:-10m->10mA");
					sprintf(lcd_line2,"%lf mA",current*1000);
					break;
				case 3:
					sprintf(lcd_line1,"Amp:-1m->1mA");
					sprintf(lcd_line2,"%lf mA",current*1000);
					break;
				default://Invalid range
					ADC1_currentRange =0;
					break;
				
			}
			sendPacket(2, current, ADC1_currentRange);
			break;
		case 3://resistance
			stageBeta(3);
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
			sendPacket(3, resistance, ADC1_currentRange);
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


