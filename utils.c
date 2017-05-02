#include "utils.h"

/*--------- INTERRUPT GLOBALS ----------*/
volatile uint32_t msTicks;                     //Counts 1ms timeTicks


//Map number form range in_min, in_max to out_in to out_max
double map(double x, double in_min, double in_max, double out_min, double out_max) {
    //If ADC input value saturates ADC input, wrap to MAX value to prevent strange math bugs
    // (because max ADC input is 3.3V but our DMM only has a range 0-3V so max is not uint16_t_MAX)
    if (x > ADC_VALUE_3V) {
        x = ADC_VALUE_3V;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


//Convert a integer into its constituent binary, as string for debug
char *int2bin(int a, char *buffer, int buf_size) {
    int i;
    buffer += (buf_size - 1);

    for (i = 31; i >= 0; i--) {
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
void Delay(uint32_t dlyTicks) {
    uint32_t curTicks;

    curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}


int compare_float(float f1, float f2) {
    float precision = 0.00001;
    if (((f1 - precision) < f2) &&
        ((f1 + precision) > f2)) {
        return 1;
    }
    else {
        return 0;
    }
}

double rand_between(double min, double max) {
    double scaled = (double) rand() / RAND_MAX;

    return (double) ((max - min) * scaled + min);
}

