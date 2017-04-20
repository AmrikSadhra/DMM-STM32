#ifndef _DAC_H_
#define _DAC_H_

#include <stm32f4xx.h>

#define   WAVE_RES          128                                  // Waveform resolution
#define   DAC_DHR12R1_ADDR  0x40007408                           // DMA writes into this reg on every request		
#define   CNT_FREQ          83000000                             // TIM6 counter clock (prescaled APB1)	

#define   MAX_WAVE_SAMPLES          10                             // Arbitrary value to determine how long to sample wave for Peak to Peak
#define   WAVE_GEN_VOLTAGE          3.28                             // Arbitrary value to determine how long to sample wave for Peak to Peak

//Wave types
#define   SINE_TYPE     	0
#define   SQUARE_TYPE   	1   
#define   SAW_TYPE 				2

void frequencyResponse(uint32_t sweepStart, uint32_t sweepEnd, uint32_t sweepResolution);
void generateSignal(int signalType);

#endif /*_DAC_H_*/

