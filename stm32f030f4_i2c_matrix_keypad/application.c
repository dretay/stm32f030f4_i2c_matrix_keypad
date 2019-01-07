#include "application.h"

static char latest_keypad = '\0';
static bool irq_attn_status = false;
static const int FLASH_I2C_INDEX = 1;
static void poll_keypad(void)
{
	char key = Keypad.get_key();
	if (key)
	{
		irq_attn_status = true;
		HAL_GPIO_WritePin(Keypad_IRQ_GPIO_Port, Keypad_IRQ_Pin, GPIO_PIN_SET);
		latest_keypad = key;
		UartPrinter.println(&key);
	}
}
static bool get_keypad(char* rx_buffer, char* tx_buffer)
{
	irq_attn_status = false;
	HAL_GPIO_WritePin(Keypad_IRQ_GPIO_Port, Keypad_IRQ_Pin, GPIO_PIN_RESET);
	tx_buffer[0] = latest_keypad;
	return true;
}
static bool set_address(char* rx_buffer, char* tx_buffer)
{
	hi2c1.Init.OwnAddress1 = rx_buffer[1];
	
	if (Flash.set_int(FLASH_I2C_INDEX, rx_buffer[1]) != HAL_OK)
	{
		Error_Handler();
	}
	
	NVIC_SystemReset();

	return false;
}
static bool erase_flash(char* rx_buffer, char* tx_buffer)
{
	if (Flash.erase() != HAL_OK)
	{
		Error_Handler();
	}		
}
//main application loop
static void run(void)
{
	
	Flash.configure();
	int i2c_address = Flash.get(FLASH_I2C_INDEX, type_int).int_val;
	if (i2c_address > 0)
	{
		hi2c1.Init.OwnAddress1 = i2c_address;
		if (HAL_I2C_Init(&hi2c1) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	} 

	Keypad.add_row(Row1_GPIO_Port, Row1_Pin);
	Keypad.add_row(Row2_GPIO_Port, Row2_Pin);
	Keypad.add_row(Row3_GPIO_Port, Row3_Pin);
	Keypad.add_row(Row4_GPIO_Port, Row4_Pin);
		
	Keypad.add_col(Col1_GPIO_Port, Col1_Pin);
	Keypad.add_col(Col2_GPIO_Port, Col2_Pin);
	Keypad.add_col(Col3_GPIO_Port, Col3_Pin);
	Keypad.add_col(Col4_GPIO_Port, Col4_Pin);

	Keypad.set_keymap("0123456789ABCDEF");
	

	SerialCommand.register_command(0x00, &get_keypad);
	SerialCommand.register_command(0x01, &erase_flash);
	SerialCommand.register_command(0x02, &set_address);
	SerialCommand.configure(I2CSerialCommandAdapter.configure(&hi2c1), &poll_keypad);

	UartPrinter.println("Ready!");
	
	while (true)
	{	
		/*
		I2C>(1)
		Searching I2C address space. Found devices at:
		0x20(0x10 W)
		I2C>[0x20 0x00 0x00]
		I2C>[0x21 rrrr]

		*/
	
		SerialCommand.next();
	}
}

const struct application Application = { 
	.run = run, 
};