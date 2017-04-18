#include "dac.h"
#include "utils.h"
#include "adc.h"

const uint16_t sine[WAVE_RES] = { 2048, 2145, 2242, 2339, 2435, 2530, 2624, 2717, 2808, 2897, 
                                      2984, 3069, 3151, 3230, 3307, 3381, 3451, 3518, 3581, 3640, 
                                      3696, 3748, 3795, 3838, 3877, 3911, 3941, 3966, 3986, 4002, 
                                      4013, 4019, 4020, 4016, 4008, 3995, 3977, 3954, 3926, 3894, 
                                      3858, 3817, 3772, 3722, 3669, 3611, 3550, 3485, 3416, 3344, 
                                      3269, 3191, 3110, 3027, 2941, 2853, 2763, 2671, 2578, 2483, 
                                      2387, 2291, 2194, 2096, 1999, 1901, 1804, 1708, 1612, 1517, 
                                      1424, 1332, 1242, 1154, 1068, 985, 904, 826, 751, 679, 
                                      610, 545, 484, 426, 373, 323, 278, 237, 201, 169, 
                                      141, 118, 100, 87, 79, 75, 76, 82, 93, 109, 
                                      129, 154, 184, 218, 257, 300, 347, 399, 455, 514, 
                                      577, 644, 714, 788, 865, 944, 1026, 1111, 1198, 1287, 
                                      1378, 1471, 1565, 1660, 1756, 1853, 1950, 2047 };           

														
static void TIM5_Config(void);
static void DAC1_Config(void);     
uint32_t   OUT_FREQ = 5000; // Output waveform frequency
uint16_t   TIM_PERIOD = 0; 	// Autoreload reg value
TIM_TimeBaseInitTypeDef TIM5_TimeBase;
																			
bool dacInitialised = false;

																												
void dac_initialise()
{
  GPIO_InitTypeDef gpio_A;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);                  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  gpio_A.GPIO_Pin  = GPIO_Pin_4;
  gpio_A.GPIO_Mode = GPIO_Mode_AN;
  gpio_A.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gpio_A);

  TIM5_Config();  
  DAC1_Config();
	
	dacInitialised = true;
}

static void TIM5_Config(void)
{
	TIM_PERIOD = ((CNT_FREQ)/((WAVE_RES)*(OUT_FREQ)));
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  TIM_TimeBaseStructInit(&TIM5_TimeBase); 
  TIM5_TimeBase.TIM_Period        = (uint16_t)TIM_PERIOD;          
  TIM5_TimeBase.TIM_Prescaler     = 0;       
  TIM5_TimeBase.TIM_ClockDivision = 0;    
  TIM5_TimeBase.TIM_CounterMode   = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM5, &TIM5_TimeBase);
  TIM_SelectOutputTrigger(TIM5, TIM_TRGOSource_Update);

  TIM_Cmd(TIM5, ENABLE);
}

static void DAC1_Config(void)
{
  DAC_InitTypeDef DAC_INIT;
	DMA_InitTypeDef DMA_INIT;		
  
  DAC_INIT.DAC_Trigger        = DAC_Trigger_T5_TRGO;
  DAC_INIT.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_INIT.DAC_OutputBuffer   = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_INIT);


  DMA_DeInit(DMA1_Stream5);
  DMA_INIT.DMA_Channel            = DMA_Channel_7;  
  DMA_INIT.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R1_ADDR;
  DMA_INIT.DMA_Memory0BaseAddr    = (uint32_t)&sine;
  DMA_INIT.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
  DMA_INIT.DMA_BufferSize         = WAVE_RES;
  DMA_INIT.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
  DMA_INIT.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_INIT.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_INIT.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
  DMA_INIT.DMA_Mode               = DMA_Mode_Circular;
  DMA_INIT.DMA_Priority           = DMA_Priority_High;
  DMA_INIT.DMA_FIFOMode           = DMA_FIFOMode_Disable;         
  DMA_INIT.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
  DMA_INIT.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
  DMA_INIT.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_INIT);

  DMA_Cmd(DMA1_Stream5, ENABLE);
  DAC_Cmd(DAC_Channel_1, ENABLE);
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}




double getPeakToPeak(){
	
	
	return read_ADC1();
}


double* frequencyResponse(uint32_t sweepStart, uint32_t sweepEnd, uint32_t sweepResolution){
	//Check DAC initialised before beginning frequency response
	if(!dacInitialised){
		dac_initialise();
	}
	
		#ifdef DEBUG
			printf("[Hardware Subsystem] Frequency Response beginning with sweep start: %d sweep end: %d sweep resolutionL: %d\r\n~", sweepStart, sweepEnd, sweepResolution);
		#endif
	
		int numResults = (sweepEnd-sweepStart)/sweepResolution;
		double *resultArray = malloc(numResults * sizeof(uint32_t));
		
		//Index into result array
		int i = 0;
	
		//Move from sweepstart to sweep end via stepnumber
		for(int OUT_FREQ = sweepStart; OUT_FREQ <= sweepEnd; OUT_FREQ += sweepResolution){			
			//Calculate new time period
			TIM_PERIOD = ((CNT_FREQ)/((WAVE_RES)*(OUT_FREQ)));
		
			TIM_TimeBaseStructInit(&TIM5_TimeBase); 
			TIM5_TimeBase.TIM_Period = (uint16_t)TIM_PERIOD;           
			TIM_TimeBaseInit(TIM5, &TIM5_TimeBase);
			TIM_SelectOutputTrigger(TIM5, TIM_TRGOSource_Update);
			
			//Wait for voltage to propagate through circuit
			Delay(10);

			//Take output reading 
			resultArray[++i] = getPeakToPeak(TIM_PERIOD);
		}
		
		return resultArray;
}



