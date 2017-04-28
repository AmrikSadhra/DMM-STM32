#ifndef _DIODE_H_
#define _DIODE_H_
#include "STM32F4xx.h"
#include "stdbool.h"
#include "dmm_config.h"
#include "utils.h"
#include "adc.h"

double readDiode(uint8_t diodeTestMode);

#endif /*_DIODE_H_*/
