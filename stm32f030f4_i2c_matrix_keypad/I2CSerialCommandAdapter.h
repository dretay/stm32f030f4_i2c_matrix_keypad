#pragma once

#include <stdbool.h>

#include "SerialCommandAdapter.h"
#include "UartPrinter.h"


#include "stm32f0xx_hal.h"

struct i2cserialcommandadapter {	
	SerialCommandAdapter*(*configure)(I2C_HandleTypeDef* i2c_handle_in);		
};

extern const struct i2cserialcommandadapter I2CSerialCommandAdapter;