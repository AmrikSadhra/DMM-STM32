#include "diode.h"

bool diodeInitialised;

void diodeInit(){
	//Initialisation of Range
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
			//Initialise ZDT test inputs
			GPIO_InitTypeDef GPIO_InitStruct;
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_Init(GPIOA, &GPIO_InitStruct);
			
			//Initialise Diode test switch pin
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	diodeInitialised = true;
}


double readDiode(uint8_t diodeTestMode){
	if(!diodeInitialised) diodeInit();
	
	static uint8_t prevDiodeTestMode;
	
	//If testing different diode type, send correct mode pins
	if(prevDiodeTestMode != diodeTestMode){
			GPIOE->ODR &= ~(1<<1);//Clear Test mode
			
			switch(diodeTestMode){
				case DTM_DIODE_TEST:
					//Do nothing as already clear
				break;
				
				case ZDT_DIODE_TEST:
					GPIOE->ODR |= (1<<1); //Set ZDT high
				break;
			}
	}
	
	prevDiodeTestMode = diodeTestMode; //Update previous test mode with current
	
		//TODO: may need to apply bitmask
		switch(diodeTestMode){
			case ZDT_DIODE_TEST:
				//A3 High
				if((GPIOA->IDR >> 10) & 1){
					return 4.0f; //Overload! Open diode.
				} else {//A3 Low
					if((GPIOA->IDR >> 9) & 1){ //A2 High
						return 3.0f; //Silicone Diode, good.
					} else {//A2 Low
						if((GPIOA->IDR >> 7) & 1){ //A1 High
						return 2.0f; //Bad diode!
					} else {//A1 Low
						if((GPIOA->IDR >> 6) & 1){ //A0 High
						return 1.0f; //Germanium diode, good!
					}
				}
			}
		}	
			break;
			
			case DTM_DIODE_TEST:
				return map((double) read_ADC1_raw(), 0, ADC_VALUE_3V, 0, 12);
		}
		
			return 2.0f; //Return bad diode if somehow passed bad test mode into function 
}
