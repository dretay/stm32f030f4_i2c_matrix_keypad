#include "application.h"

static char latest_keypad = '\0';
static void poll_keypad(void)
{
	char key = Keypad.get_key();
	if (key)
	{
		latest_keypad = key;
		UartPrinter.println(&key);
	}
}
static bool get_keypad(char* tx_buffer)
{
	tx_buffer[0] = latest_keypad;
	return true;
}
//main application loop
static void run(void)
{
	Keypad.add_row(Row1_GPIO_Port, Row1_Pin);
	Keypad.add_row(Row2_GPIO_Port, Row2_Pin);
	Keypad.add_row(Row3_GPIO_Port, Row3_Pin);
	Keypad.add_row(Row4_GPIO_Port, Row4_Pin);
		
	Keypad.add_col(Col1_GPIO_Port, Col1_Pin);
	Keypad.add_col(Col2_GPIO_Port, Col2_Pin);
	Keypad.add_col(Col3_GPIO_Port, Col3_Pin);
	Keypad.add_col(Col4_GPIO_Port, Col4_Pin);

	Keypad.set_keymap("0123456789ABCDEF");

	UartPrinter.println("Initialized!");

	SerialCommand.register_command(0x00, &get_keypad);
	SerialCommand.configure(I2CSerialCommandAdapter.configure(), &poll_keypad);
	
	while (true)
	{	
		/*
		I2C>(1)
		Searching I2C address space. Found devices at:
		0x20(0x10 W)
		I2C>[0x20 0x00]
		I2C>[0x21 rrrr]

		*/

		/*
		todo: 
		1) general cleanup (esp var names and array sizes)
		2) add timeout for i2c resetting 
		3) add irq pin
		4) same as rotary encoder - resettable address
		*/
		SerialCommand.next();
	}
}

const struct application Application = { 
	.run = run, 
};