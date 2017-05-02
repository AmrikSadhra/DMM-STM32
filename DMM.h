#ifndef __DMM_H
#define __DMM_H

//Configuration file
#include "dmm_config.h"

//Provided Libraries
#include <stdio.h>
#include "STM32F4xx.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "misc.h"
#include "math.h"
#include "stdbool.h"
//Made with Love (by us)
#include "adc.h"
#include "capacitance.h"
#include "serial.h"
#include "packet.h"
#include "utils.h"
#include "queue.h"
#include "switches.h"
#include "dac.h"
#include "diode.h"

void dmmModeSelect(uint8_t function);

#endif
