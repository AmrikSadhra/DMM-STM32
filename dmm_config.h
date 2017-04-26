#ifndef _DMM_CONFIG_H_
#define _DMM_CONFIG_H_

/* --------------------------- Debug Settings ---------------------------- */
#define DMM_DEBUG //Uncomment to debug main DMM menu logic
#define DAC_DEBUG //Uncomment to debug DAC readings
//#define SWITCH_DEBUG //Uncomment to external interrupts on switches 
//#define QUEUE_DEBUG //Uncomment to debug actions of serial queues

/* ------------------------ Generic DMM Settings ------------------------- */

//Manual Menu navigation Settings
#define MANUAL_VALDOWN_BUTTON 1   //Value decrease button
#define MANUAL_DONE_BUTTON 2 			//Done entering value button
#define MANUAL_VALUP_BUTTON 3 		//Value increase button
#define MANUAL_CANCEL_BUTTON 4    //Cancel current value entering button

#define MANUAL_STEPDOWN_BUTTON 7  //Decrease stepsize button
#define MANUAL_STEPUP_BUTTON 8		//Increase stepsize button

#define NO_ACTION 0 							//Empty state (do nothing in menu)
#define UPDATE_DISPLAY 9					//State for updating display (arbitrary, larger than num buttons: 8)

//Signal Generation
#define SIG_GEN_STATE 420					//State for updating display (arbitrary, larger than num buttons: 8)

#define MANUAL_DEFAULT_FREQ 100    	//Default starting frequency
#define MANUAL_MIN_FREQ 100 		 	//Minimum frequency
#define MANUAL_MAX_FREQ 10000  	 	//Maximum frequency

#define MANUAL_DEFAULT_STEP 10		//Default step size for frequency (1, 10, 1000, 10000)
#define MANUAL_STEP_MULTIPLIER 10	//Amount to multiply stepsize by when dropping/bumping
#define MANUAL_MAX_STEPSIZE 10000 //Maximum stepsize for frequency
#define MANUAL_MIN_STEPSIZE 1			//Minimum stepsize for frequency

#define MANUAL_MIN_AMP 0.1f			 	//Minimum amplitude
#define MANUAL_MAX_AMP 1.0f			 	//Maximum amplitude
#define MANUAL_AMP_STEPSIZE 0.1f 	//Amplitude stepsize

//Frequency Response
#define MANUAL_MIN_STEP 1			 		//Minimum Step size
#define MANUAL_MAX_STEP 100			 	//Maximum Step Size
#define MANUAL_SWEEP_STEPSIZE 1 	//SweepResolution stepsize

/* ---------------------------- LCD Settings ----------------------------- */
#define BUFFER_SIZE 128

/* ----------------- Frequency Response/Sig Gen Settings ----------------- */

#define   WAVE_RES          128            // Waveform resolution
#define   DAC_DHR12R1_ADDR  0x40007408     // DMA writes into this reg on every request		
#define   CNT_FREQ          83000000       // TIM6 counter clock (prescaled APB1)	
#define   MAX_WAVE_SAMPLES          10     // Arbitrary value to determine how long to sample wave for Peak to Peak
#define   WAVE_GEN_VOLTAGE          3.28   // Arbitrary value to determine how long to sample wave for Peak to Peak

/* ---------------------------- Serial Settings -------------------------- */
#define MAX_SERIAL_IN_LENGTH 16							//Maximum size of debug or bluetooth packet				
#define MAX_BT_BUF_SIZE 10									//Size of bluetooth buffer serial queue (increase if dropping Android packets)
#define MAX_DBG_BUF_SIZE 10									//Size of debug buffer serial queue (increase if dropping debug packets)
#define DEFAULT_BAUD 9600										//Default baud rates for serial if none supplied

/* ----------------------------- ADC Settings ---------------------------- */
#define ADC_DEBUG 

//TODO: Fill these with cal curve data
#define CAL_COEFF_X3
#define CAL_COEFF_X2
#define CAL_COEFF_OFFSET

#define ADC_CALIBRATION_ENABLED 
#define SWITCHING_RANGE_TOLERANCE 2000
#define NUM_AVG_SAMPLES 50
#define NUM_CAL_SAMPLES 5

#define ADC_SAMPLE_TIME 50E-3 						 	//Time for ADC to take a single sample (used for freq response sampling)

/* ---- IRQ Priorities - Control which interrupt handlers have priority ---- */
#define TIM2_IQ_PRIORITY 0 									//Capacitance timer Handlers
#define TIM2_IQ_SUBPRIORITY 0

#define BLUETOOTH_IQ_PRIORITY 1							//Serial Handlers
#define BLUETOOTH_IQ_SUBPRIORITY 0
#define DEBUG_IQ_PRIORITY 1 
#define DEBUG_IQ_SUBPRIORITY 1 

#define SWITCH_IQ_PRIORITY 2								//Menu Switch Handlers
#define SWITCH_IQ_SUBPRIORITY 0  

#endif /*_DMM_CONFIG_H_*/
