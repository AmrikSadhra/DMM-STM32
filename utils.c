#include "utils.h"

//--------- INTERRUPT GLOBALS ----------
volatile uint32_t msTicks;                     //Counts 1ms timeTicks

//Map number form range in_min, in_max to out_in to out_max
double map(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


//Convert a integer into its constituent binary, as string for debug
char *int2bin(int a, char *buffer, int buf_size){
		int i;
		buffer += (buf_size - 1);
	
		for(i = 31; i >= 0; i--){
			*buffer-- = (a & 1) + '0';
			a >>= 1;
		}
		
		return buffer;
}

//SysTick_Handler for Delay function
void SysTick_Handler(void) {
  msTicks++;
}

//Delays number of tick Systicks (happens every 1 ms)
void Delay (uint32_t dlyTicks) {               
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}
