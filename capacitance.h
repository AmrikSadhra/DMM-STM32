#include "stm32f4xx_tim.h"
#include "dmm_config.h"
#include "stdbool.h"
#include "serial.h"
#include "packet.h"

//Capacitance Ranges
#define CAP_RANGE_0 0 //910 Ohm divider
#define CAP_RANGE_1 1 //910k Ohm Divider

void measureCapacitance(void);

extern bool timerDone;
extern double capacitance;

