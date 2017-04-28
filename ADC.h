#ifndef _ADC_H_
#define _ADC_H_

#include "STM32F4xx.h"
#include "dmm_config.h"
#include "dmm.h"
#include "serial.h"
#include "dmm.h"
#include "utils.h"
#include <stdbool.h>
#include "math.h"

/* Globally available integer holding current ADC Range */
extern uint8_t ADC1_currentRange;
extern bool rangeChange;

/* Functions to read ADC and return value */
void ADC1_init(void);
double read_ADC1 (bool resistanceMode);
double read_ADC1_NOAVERAGE (void);
uint16_t read_ADC1_raw (void);


#endif /*_ADC_H_*/
