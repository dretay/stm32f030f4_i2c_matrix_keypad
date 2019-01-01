#pragma once

#include <stdbool.h>

#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"

#include "UartPrinter.h"
#include "Keypad.h"

struct application
{
	void(*run)(void);
};
extern const struct application Application;