#ifndef _DMM_CONFIG_H_
#define _DMM_CONFIG_H_

/* Generic DMM Settings */
#define DMM_DEBUG 

/* LCD Settings */
#define BUFFER_SIZE 128

/* Frequency Response/Sig Gen Settings */
//#define DAC_DEBUG 

#define   WAVE_RES          128                                  // Waveform resolution
#define   DAC_DHR12R1_ADDR  0x40007408                           // DMA writes into this reg on every request		
#define   CNT_FREQ          83000000                             // TIM6 counter clock (prescaled APB1)	
#define   MAX_WAVE_SAMPLES          10                           // Arbitrary value to determine how long to sample wave for Peak to Peak
#define   WAVE_GEN_VOLTAGE          3.28                         // Arbitrary value to determine how long to sample wave for Peak to Peak

/* Switch Settings */
#define SWITCH_DEBUG 

/* Queue Settings */
//#define QUEUE_DEBUG 

/* Serial Settings */
#define MAX_SERIAL_IN_LENGTH 16
#define SERIAL_TIMEOUT_MAX 30
#define MAX_BT_BUF_SIZE 10
#define MAX_DBG_BUF_SIZE 10
#define DEFAULT_BAUD 9600

/* ADC Settings */
#define ADC_DEBUG 

//TODO: Fill these with cal curve data
#define CAL_COEFF_X3
#define CAL_COEFF_X2
#define CAL_COEFF_OFFSET

#define ADC_CALIBRATION_ENABLED 
#define SWITCHING_RANGE_TOLERANCE 2000
#define NUM_AVG_SAMPLES 50
#define NUM_CAL_SAMPLES 5

#define ADC_SAMPLE_TIME 50E-3 //Time for ADC to take a single sample (used for freq response sampling)

/* IRQ Priorities - Control which interrupt handlers have priority */
//Used for capacitance
#define TIM2_IQ_PRIORITY 0 
#define TIM2_IQ_SUBPRIORITY 0

//Serial Intterupt Handlers
#define BLUETOOTH_IQ_PRIORITY 1
#define BLUETOOTH_IQ_SUBPRIORITY 0

#define DEBUG_IQ_PRIORITY 1 
#define DEBUG_IQ_SUBPRIORITY 1 

//Menu button inputs
#define SWITCH_IQ_PRIORITY 2
#define SWITCH_IQ_SUBPRIORITY 0  

#endif /*_DMM_CONFIG_H_*/
