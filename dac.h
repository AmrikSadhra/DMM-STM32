#ifndef _DAC_H_
#define _DAC_H_

#include <stm32f4xx.h>

#define   WAVE_RES          128                                  // Waveform resolution
#define   DAC_DHR12R1_ADDR  0x40007408                           // DMA writes into this reg on every request		
#define   CNT_FREQ          83000000                             // TIM6 counter clock (prescaled APB1)		
//TODO: Tweak. We get 5.048kHz clock at base with this tim6 clock.

double* frequencyResponse(uint32_t sweepStart, uint32_t sweepEnd, uint32_t sweepResolution);
void dac_initialise(void);

#endif /*_DAC_H_*/

