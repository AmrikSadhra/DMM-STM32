#include "stm32f4xx_tim.h"
#include "dmm_config.h"
#include "stdbool.h"
#include "serial.h"

void measureCapacitance(void);

extern double timeHigh;
extern bool timerDone;
extern uint32_t numHighTicks;
