#ifndef _UTILS_H_
#define _UTILS_H_
#include "STM32F4xx.h"

//Simple mapping function for an input range to an output
double map(double x, double in_min, double in_max, double out_min, double out_max);
//Converts integer (STM32 BUS) to string containing binary contents 
char *int2bin(int a, char *buffer, int buf_size);
//Delays number of tick Systicks (happens every 1 ms)
void Delay (uint32_t dlyTicks);
#endif /*_UTILS_H_*/
