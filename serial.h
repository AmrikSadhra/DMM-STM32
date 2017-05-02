#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stm32f4xx.h> 
#include <stm32f4xx_usart.h>
#include <misc.h> 
#include "dmm_config.h"
#include "stdio.h"
#include "utils.h"
#include "stdbool.h"
#include "queue.h"
#include "string.h"

/* Serial Functions */
void serial_init(uint32_t baudRate);

void bluetooth_init(uint32_t baudRate);

void bt_send_str(char *str);

void printBus(uint32_t bus);

/* Globally available Variables */
extern Queue *bluetoothQueue, *debugQueue;
extern bool bluetoothConnected;

#endif /*_SERIAL_H_*/
