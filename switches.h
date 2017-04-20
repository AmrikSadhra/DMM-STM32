#ifndef _SWITCHES_H_
#define _SWITCHES_H_
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "serial.h"
#include "dmm_config.h"


//Initialise Switches
void switch_init(void);

/*--------- INTERRUPT GLOBALS ----------*/
//Externally available global for menu position
extern uint8_t menuPosition;

#endif /*_SWITCHES_H_*/

