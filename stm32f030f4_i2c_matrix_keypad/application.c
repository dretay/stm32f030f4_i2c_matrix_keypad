#include "application.h"
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
	
	while (true)
	{	
		char key = Keypad.get_key();
		if (key)
		{
			UartPrinter.println(&key);
		}
	}
}

const struct application Application = { 
	.run = run, 
};