#include "I2CSerialCommandAdapter.h"

static SerialCommandAdapter adapter;
static I2C_HandleTypeDef *i2c_handle;

static void(*write_callback)(void);
static void(*read_callback)(void);

static void write_it(const char* buffer, int size, void(*callback)(void)) {
	write_callback = callback;
	UartPrinter.println("ready to tx");

	if (HAL_I2C_Slave_Transmit_IT(i2c_handle, (uint8_t *)buffer, size) != HAL_OK)
	{
		UartPrinter.println("HAL_I2C_Slave_Transmit_IT error");
	}
}

static void read_it(char *buffer, int size, void(*callback)(void)) {
	read_callback = callback;
	if (HAL_I2C_Slave_Receive_IT(i2c_handle, (uint8_t *)buffer, size) != HAL_OK)
	{		
		UartPrinter.println("HAL_I2C_Slave_Receive_IT error");
	}	
}

static SerialCommandAdapter* configure(I2C_HandleTypeDef *i2c_handle_in) {
	
	i2c_handle = i2c_handle_in;

	adapter.read_it = read_it;
	adapter.write_it = write_it;
	return &adapter;

}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	read_callback();
}
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	write_callback();
}

const struct i2cserialcommandadapter I2CSerialCommandAdapter = { 
	.configure = configure,		
};