#ifndef _SWITCHES_H_
#define _SWITCHES_H_
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "serial.h"

#define DEBUG 1

void switch_init(void);
extern uint8_t menuPosition;

#endif /*_SWITCHES_H_*/

