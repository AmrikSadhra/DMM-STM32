#ifndef _ADC_H_
#define _ADC_H_

#include "STM32F4xx.h"
#include "dmm_config.h"
#include "serial.h"
#include "dmm.h"
#include "utils.h"
#include <stdbool.h>
#include "math.h"

/* Globally available integer holding current ADC Range */
extern uint8_t ADC1_currentRange;

/* Functions to read ADC and return value */
void ADC1_init(void);
double read_ADC1 (void);
double read_ADC1_NOAVERAGE (void);

/* Function to set ADC to cont */
unsigned int read_cont_ADC1 (void);
void set_cont_ADC1(void);	

#endif /*_ADC_H_*/
