#ifndef _ADC_H_
#define _ADC_H_
#include "STM32F4xx.h"
#include "utils.h"
#include <stdbool.h>

#define MAX_CALIBRATION_LOOPS 5;
#define ADC_IN_MIN 0
#define ADC_IN_MAX 3

void ADC1_init(void);
/* function to read ADC and retun value */
double read_ADC1 (void);

void set_cont_ADC1(void);	

/* function to set ADC to cont */
unsigned int read_cont_ADC1 (void);

double calibrate_ADC1(void);

#endif /*_ADC_H_*/
