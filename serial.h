#ifndef _SERIAL_H_
#define _SERIAL_H_
#include <stm32f4xx.h> 
#include <stm32f4xx_usart.h>
#include <misc.h> 
#include "stdio.h"
#include "utils.h"
#include "DMM.h"
#include "stdbool.h"
#include "queue.h"
#include "lcd_buffer.h"
#include "lcd_driver.h"
#include "string.h"

#define MAX_SERIAL_IN_LENGTH 16
#define SERIAL_TIMEOUT_MAX 30

#define MAX_BT_BUF_SIZE 10
#define MAX_DBG_BUF_SIZE 10

#define DEFAULT_BAUD 9600

void serial_init(uint32_t baudRate);
void bluetooth_init(uint32_t baudRate);
void bt_send_str(char *str);
void printBus(uint32_t bus);

extern Queue *bluetoothQueue, *debugQueue;
#endif /*_SERIAL_H_*/
