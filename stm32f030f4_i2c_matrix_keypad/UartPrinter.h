#pragma once

#define UART_BUFFER_SIZE 32

#include <string.h>

#include "stm32f0xx_hal.h"

//#if DEBUG == 1
//#define LOG(...) SEGGER_RTT_printf(0, __VA_ARGS__)
//#else
//#define LOG(...)
//#endif

extern UART_HandleTypeDef huart1;

struct uartprinter
{
	void(*println)(char* line);
};
extern const struct uartprinter UartPrinter;
