#include "UartPrinter.h"

static char uart_tx_buffer[UART_BUFFER_SIZE];
static char* uart_tx_cr = "\r\n";

//todo: for some reason I don't understand snprintf is messing with \r\n sequence... so 
//it needs to be echo'd separate from the string. if it becomes a problem maybe make this flaggable?
static void println(char* line)
{
#ifdef HAL_UART_MODULE_ENABLED
	int str_len = 0;	
	snprintf(uart_tx_buffer, UART_BUFFER_SIZE, "%s", line);
	str_len = strcspn(uart_tx_buffer, "\0");
	if (HAL_UART_Transmit(&huart1, (uint8_t*)line, str_len, 5000) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UART_Transmit(&huart1, (uint8_t*)uart_tx_cr, 2, 5000) != HAL_OK)
	{
		Error_Handler();
	}
#endif
}

const struct uartprinter UartPrinter = { 
	.println = println, 
};