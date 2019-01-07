#include "SerialCommand.h"

//todo: these should be generalized out since it is being reused so much
//esp. the generic stuff... maybe implementors pass in concrete impls? esp all the tokenizing stuff

static SerialCommand_Command commands[SERIAL_COMMAND_CNT];
static unsigned int SerialCommand_Command_Usage_Mask = 0;
static unsigned int serial_command_count = 0;

SerialCommandAdapter *serial_adapter;
static void(*idle_fn)(void);

#define SERIAL_COMMAND_RX_BUFFER_SIZE  2
static char serial_command_rx_buffer[SERIAL_COMMAND_RX_BUFFER_SIZE+1];

#define SERIAL_COMMAND_TX_BUFFER_SIZE 4
static char serial_command_tx_buffer[SERIAL_COMMAND_TX_BUFFER_SIZE+1];

static bool(*command_processing_function)(char* rx_buffer, char* tx_buffer) = NULL;

//todo: this should be "a lower level" implementation that handles more transport coordination
//higher level stuff should happen in a concrete implementor?
static int8_t get_unused_idx(unsigned int* usage_mask) {
	uint8_t i = 0;
	for (i = 0; i < SERIAL_COMMAND_CNT; i++) {
		if (bitRead(*usage_mask, i) == 0) {
			bitSet(*usage_mask, i);
			return i;
		}	
	} 
	return -1;
}
static bool idx_valid(int8_t idx, unsigned int* usage_mask) {
	return idx >= 0 && idx < SERIAL_COMMAND_CNT && (bitRead(*usage_mask, idx) == 1);			
}
static int get_usage_count(unsigned int* usage_mask)
{
	int i = 0;
	for (; idx_valid(i, usage_mask); i++) ;
	return i;
}
//todo: this is still not right
static bool register_command(int command_in, void* function_in) {
	uint8_t idx;	
	idx = get_unused_idx(&SerialCommand_Command_Usage_Mask);
	if (idx_valid(idx, &SerialCommand_Command_Usage_Mask)) {
		commands[idx].command[0] = command_in;
		commands[idx].function = function_in;				
		serial_command_count = get_usage_count(&SerialCommand_Command_Usage_Mask);
		return true;
	}
	return false;
}
enum SERIAL_COMMAND_READ_STATUS
{
	READY    = 0,
	PENDING  = 1,	
	PROCESS  = 2,
	RESPOND  = 3
};
static volatile enum SERIAL_COMMAND_READ_STATUS serial_command_read_status = READY;

static void rx_callback(void)
{		
	for (int i = 0; i < serial_command_count; i++)
	{		
		if(serial_command_rx_buffer[0] == commands[i].command[0])
		{
			command_processing_function = commands[i].function;
			serial_command_read_status = PROCESS;
		}
	}
}
static void tx_callback(void)
{
	serial_command_read_status = READY;
}
static void next(void)
{
	serial_adapter->read_it(serial_command_rx_buffer, SERIAL_COMMAND_RX_BUFFER_SIZE, &rx_callback);
	serial_command_read_status = PENDING;
	UartPrinter.println("pending");

	while (serial_command_read_status !=  READY)
	{
		switch (serial_command_read_status)
		{
		case PROCESS:
			UartPrinter.println("processing");
			if (command_processing_function != NULL)
			{
				if (command_processing_function(serial_command_rx_buffer, serial_command_tx_buffer))
				{
					serial_adapter->write_it(serial_command_tx_buffer, SERIAL_COMMAND_TX_BUFFER_SIZE, &tx_callback);
					serial_command_read_status = RESPOND;
					break;
				}
				command_processing_function = NULL;
			}
			serial_command_read_status = READY;
			break;
		default:
			if (idle_fn != NULL)
			{
				idle_fn();
			}
		}
	}
}

static void configure(SerialCommandAdapter* serial_adapter_in, void(*idle_fn_in)(void)) {
	serial_adapter = serial_adapter_in;
	idle_fn = idle_fn_in;

}

const struct serialcommand SerialCommand = { 
	.register_command = register_command,
	.configure = configure,	
	.next = next,
};