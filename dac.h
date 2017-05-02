#ifndef _DAC_H_
#define _DAC_H_

#include <stm32f4xx.h>
#include "dmm_config.h"
#include "utils.h"
#include "adc.h"
#include "serial.h"
#include "packet.h"

/*--------- Wave Types ----------*/
#define   SINE_TYPE        0
#define   SQUARE_TYPE    1
#define   SAW_TYPE                2
#define   NOISE_TYPE      3

extern char *waveTypes[];

/*--------- Functions ----------*/
void frequencyResponse(uint32_t sweepStart, uint32_t sweepEnd, uint32_t sweepResolution);

void generateSignal(uint32_t genFrequency, uint8_t signalType, float amplitude);

void stopGenerating(void);

#endif /*_DAC_H_*/

