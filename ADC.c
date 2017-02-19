#include "ADC.h"

double calibrationValue;
bool isInitialised = false;

void ADC1_init(void) {
	//Fix for infinite call in calibration routine
	if(!isInitialised){
		isInitialised = true;
	} else
	{
		return;
	}
	
	//Set registers to bring up ADC1 in correct mode
	RCC->APB2ENR  |= ((1UL <<  8) );         /* Enable ADC1 clock                */
	RCC->AHB1ENR  |= ((1UL <<  2) );         /* Enable GPIOC clock                */
	GPIOC->MODER = 0xffffffff;
	GPIOC->PUPDR = 0;
	ADC1->CR1 = 0x00;
	ADC1->CR1 |= (1UL << 11);
	ADC1->CR2 = 0x00;
	ADC1->CR2 |= (1UL << 10) ;					/* right alignement of 12 bits */
	ADC->CCR = 0x00;
	ADC1->SQR1 = 0x01;								/* 1 conversion at a time */
	ADC1->SMPR1 = 0x00;
	ADC1->SMPR1 = 0x0300;
	ADC1->SQR1 = 0x01;
	ADC1->SQR3 = 0x0e;								/* ADC_IN14 = 0x0e: ADC_IN15 = 0x0f */
	ADC1->CR2 |= (1UL << 0);
	
	//Take 5 samples to calibrate ADC (remove base noise)
	calibrationValue = calibrate_ADC1();
}
	
//Function to read ADC and retun scaled value
double read_ADC1 (void) {
	//Initialise ADC1 inline if not up
	if(!isInitialised) ADC1_init();
	
	//Begin ADC1 read
	ADC1->CR2 |= (1UL << 30)	;		/* set SWSTART to 1 to start conversion */
	Delay(100);
	
	//Scale the 32 bit ADC input to between 0 and 3v
	return map((ADC1->DR << 4) - calibrationValue, 0, UINT16_MAX, ADC_IN_MIN, ADC_IN_MAX);
}

double scale_ADC1(uint16_t adc1_raw){
	//Scale the 32 bit ADC input to between 0 and 3v
	return map(adc1_raw  - calibrationValue, 0, UINT16_MAX, ADC_IN_MIN, ADC_IN_MAX);
}

//Function to read ADC and return raw value
uint32_t read_ADC1_raw (void) {
	//Initialise ADC1 inline if not up
	if(!isInitialised) ADC1_init();
	
	//Begin ADC1 read
	ADC1->CR2 |= (1UL << 30)	;		/* set SWSTART to 1 to start conversion */
	Delay(100);
	
	//Return raw 32bit uint
	return (ADC1->DR << 4);
	}

void set_cont_ADC1(void){
	ADC1->CR2 |= (1UL << 30)	;		/* set SWSTART to 1 to start conversion */
	//Can wire to button to enable cont
	ADC1->CR2 |= (1UL << 1)	;		
}

//Function to set ADC to continuous read mode
unsigned int read_cont_ADC1 (void) {
	//TODO: Inside spinlock, increment i to MAX_RETRY_ADC_CONT. Throw exception.
	while(!((ADC1->CR2 & (1 << 10))>0));
	return ADC1->DR;
}

//Calibrate out ADC noise (NEEDS TO RUN WHILST ADC NOT CONNECTED TO ANYTHING)
double calibrate_ADC1(void){
	double sum = 0;
	int maxLoops = MAX_CALIBRATION_LOOPS;

	for(int j = 0; j < maxLoops; j++){
			sum += (double) read_ADC1();
	}
	
	return sum/MAX_CALIBRATION_LOOPS;
}

