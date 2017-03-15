
#ifndef __DMM_H
#define __DMM_H
#include "stdbool.h"
#include "ADC.h"
void initialise_Peripherals(void);
extern bool BluetoothMode;
void stageAlpha(int mode);
void stageBeta(int mode);
void stageGamma(int mode);
#endif
