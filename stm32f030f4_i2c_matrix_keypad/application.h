#pragma once

#include <stdbool.h>

#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"

#include "UartPrinter.h"
#include "Keypad.h"
#include "SerialCommand.h"
#include "I2CSerialCommandAdapter.h"
#include "Flash.h"

extern I2C_HandleTypeDef hi2c1;

#ifdef HAL_UART_MODULE_ENABLED
extern UART_HandleTypeDef huart1;
#endif
struct application
{
	void(*run)(void);
};
extern const struct application Application;