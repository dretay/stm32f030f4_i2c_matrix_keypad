#pragma once

#include <stdbool.h>

#include "SerialCommandAdapter.h"
#include "UartPrinter.h"


#include "stm32f0xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

struct i2cserialcommandadapter {	
	SerialCommandAdapter*(*configure)(void);		
};

extern const struct i2cserialcommandadapter I2CSerialCommandAdapter;