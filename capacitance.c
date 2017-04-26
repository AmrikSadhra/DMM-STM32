#include "capacitance.h"
#include "STM32F4xx.h"

uint32_t numHighTicks;
double timeHigh; //ayyyy
bool timerDone = true;

int averageIndex = 0;
double average[10];

void LED_Init1(){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
 
		//Pulse Generate
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_WriteBit(GPIOD, GPIO_Pin_13, Bit_RESET);
		
		//Capacitance Pulse read pin
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
	  GPIO_WriteBit(GPIOD, GPIO_Pin_12, Bit_RESET);
}


void processResults(){
	TIM_Cmd(TIM2, DISABLE); //Stop Timer
	
	double timeHigh = (double)(numHighTicks*2.4E-6);
	
	if(averageIndex < 10){ 
		if(timeHigh > 0){ 
			average[averageIndex] = timeHigh;
			printf("[Capacitance Measuring] Pulse size at %d: %f seconds\n\r", averageIndex, timeHigh);	
			averageIndex++;
		}
	} else {
		double runningTotal = 0;
		for(int i = 0; i < 10; i++){
			runningTotal += average[i];
		}
		runningTotal /= 10;
		printf("[Capacitance Measuring] Pulse size average: %f seconds\n\r", runningTotal);	
		averageIndex = 0;
	}
	
	timerDone = true;
	GPIO_WriteBit(GPIOD, GPIO_Pin_13, Bit_SET); //Set pulse pin high
}

void TIM2_IRQHandler()
{
   if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
			//Total Ticks, ticks with pin high
			//If Input pin high
			if((GPIOD->IDR >> 12) & 1){
				numHighTicks++;
			} else {	//If done detecting pulse	
				processResults();
			}
			
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		}
}


void Timer_Init(){
		TIM_TimeBaseInitTypeDef TIM2_InitStruct;
	
		/* Enable timer 2, using the Reset and Clock Control register */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		TIM2_InitStruct.TIM_Prescaler = 100;
		TIM2_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
		TIM2_InitStruct.TIM_Period = 1;
		TIM2_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseInit(TIM2, &TIM2_InitStruct);
	
		/* TIM IT enable */
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM2, ENABLE); /* start counting by enabling CEN in CR1 */

		/* Register Timer Interrupt handler */
	  NVIC_InitTypeDef 	NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = TIM2_IQ_PRIORITY;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = TIM2_IQ_SUBPRIORITY;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&	NVIC_InitStruct);
}

void measureCapacitance(){
	numHighTicks = 0;
	timerDone = false;
	LED_Init1();
	
	GPIO_WriteBit(GPIOD, GPIO_Pin_13, Bit_SET); //Set pulse pin high
	GPIO_WriteBit(GPIOD, GPIO_Pin_13, Bit_RESET);
	GPIO_WriteBit(GPIOD, GPIO_Pin_13, Bit_SET);
	
	Timer_Init();
}



