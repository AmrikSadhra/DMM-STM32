#ifndef _UTILS_H_
#define _UTILS_H_
#include "STM32F4xx.h"
#include "stdlib.h"

//Simple mapping function for an input range to an output
double map(double x, double in_min, double in_max, double out_min, double out_max);
//Converts integer (STM32 BUS) to string containing binary contents 
char *int2bin(int a, char *buffer, int buf_size);
//Delays number of tick Systicks (happens every 1 ms)
void Delay (uint32_t dlyTicks);
//Compare two floats by checking within var precision distance
int compare_float(float f1, float f2);
//Generate random number between min and max
double rand_between(double min, double max);
#endif /*_UTILS_H_*/

#define ADC_VALUE_3V 58981 //the value the ADC will output at 3V for 10v range


