#ifndef _BTN_H_
#define _BTN_H_
	#include "STM32F4xx.h"
	#include <stdint.h>

void BTN_Init(void);
uint32_t BTN_Get(void);

#endif /*_BTN_H_*/
