#pragma once

#include "stm32f0xx_hal.h"

#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "SerialCommandAdapter.h"
#include "bithelper.h"
#include "UartPrinter.h"

typedef struct {
	char command[8];
	bool(*function)(char* rx_buffer, char* tx_buffer);
} SerialCommand_Command;

#define SERIAL_COMMAND_CNT 11

struct serialcommand {
	void(*configure)(SerialCommandAdapter*, void(*idle_fn_in)(void));	
	bool(*register_command)(int command_in, void* function_in);	
	void(*next)(void);	
	
};


extern const struct serialcommand SerialCommand;
