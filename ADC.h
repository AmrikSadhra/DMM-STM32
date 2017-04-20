#ifndef _ADC_H_
#define _ADC_H_
#include "STM32F4xx.h"
#include "utils.h"
#include "serial.h"
#include <stdbool.h>
#include "math.h"

#define SWITCHING_RANGE_TOLERANCE 2000
#define NUM_AVG_SAMPLES 50
#define NUM_CAL_SAMPLES 5

#define ADC_SAMPLE_TIME 50E-3 //Time for ADC to take a single sample (used for freq response sampling)

#define DEBUG 1

//Integer holding current ADC Range
extern uint8_t ADC1_currentRange;

void ADC1_init(void);
/* Functions to read ADC and return value */
double read_ADC1 (void);
double read_ADC1_NOAVERAGE (void);

/* function to set ADC to cont */
unsigned int read_cont_ADC1 (void);
void set_cont_ADC1(void);	

#endif /*_ADC_H_*/
