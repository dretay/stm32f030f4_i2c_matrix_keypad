#include "Keypad.h"

//vars related to keypad definition
static Pin row_pins[KEYPAD_MAX_PINS];
static unsigned int row_pins_mask = 0;
static Pin col_pins[KEYPAD_MAX_PINS];
static unsigned int col_pins_mask = 0;
static char keymap[KEYPAD_MAX_PINS*KEYPAD_MAX_PINS+1];
static KeypadConfig keypad_config;

//vars related to keypad state
static Key key[KEYPAD_LIST_MAX];
static unsigned int keypad_mask[KEYPAD_MAX_PINS];
static unsigned int start_time = 0;
static long hold_timer;

//returns the next unsed index from an array
static int8_t get_unused_idx(unsigned int* usage_mask) {
	uint8_t i = 0;
	for (i = 0; i < KEYPAD_MAX_PINS; i++) {
		if (bitRead(*usage_mask, i) == 0) {
			bitSet(*usage_mask, i);
			return i;
		}	
	} 
	return -1;
}

//verifies a given array index is valid 
static bool idx_valid(int8_t idx, unsigned int* usage_mask) {
	return idx >= 0 && idx < KEYPAD_MAX_PINS && (bitRead(*usage_mask, idx) == 1);			
}

//returns the number of "used" slots in an array
static int get_pin_count(unsigned int* usage_mask)
{
	int i = 0;
	for (; idx_valid(i, usage_mask); i++);
	return i;
}

//generic helper to add an element to an array
static bool add_generic(Pin* pins, unsigned int* usage_mask, GPIO_TypeDef* gpio_port, unsigned int gpio_pin) {	
	uint8_t idx;	
	idx = get_unused_idx(usage_mask);
	if (idx_valid(idx, usage_mask)) {
		pins[idx].gpio_pin = gpio_pin;
		pins[idx].gpio_port= gpio_port;		
		return true;
	}
	return false;
}

//adds a matric row pin
static bool add_row(GPIO_TypeDef* gpio_port, unsigned int gpio_pin)
{
	bool status = add_generic(row_pins, &row_pins_mask, gpio_port, gpio_pin);
	keypad_config.num_rows = get_pin_count(&row_pins_mask);
	return status;
}

//adds a matrix column pin
static bool add_col(GPIO_TypeDef* gpio_port, unsigned int gpio_pin)
{
	bool status =  add_generic(col_pins, &col_pins_mask, gpio_port, gpio_pin);
	keypad_config.num_columns = get_pin_count(&col_pins_mask);
	return status;
}

//defines the keymap, expectation is a string where element 0 is the first key and element n is the 
//last key in the matrix
static void set_keymap(char* keymap_in)
{
	snprintf(keymap, KEYPAD_MAX_PINS*KEYPAD_MAX_PINS+1,"%s", keymap_in);
}

//actually scan the keypad. pull each column pin down sequentially then store the state of the row pins in a mask to 
//evaluate in next key state 
static void scan_keys(void) {	
	for (int col = 0; col < keypad_config.num_columns; col++)
	{		
		HAL_GPIO_WritePin(col_pins[col].gpio_port, col_pins[col].gpio_pin, GPIO_PIN_RESET);
		for (int row = 0; row < keypad_config.num_rows; row++)
		{					
			bitWrite(keypad_mask[row], col, !HAL_GPIO_ReadPin(row_pins[row].gpio_port, row_pins[row].gpio_pin));	
		}
		HAL_GPIO_WritePin(col_pins[col].gpio_port, col_pins[col].gpio_pin, GPIO_PIN_SET);		
	}	
}

//scan the list of "active" keys for one matching the matrix coordinate being searched for
static int find_in_list_by_coordinate(int coordinate) {
	for (int i = 0; i < KEYPAD_LIST_MAX; i++) {
		if (key[i].coordinate == coordinate) {
			return i;
		}
	}
	return -1;
}
//set the state of the key and mark it as changed
static void set_key_state(int idx, KeyState nextState) {
	key[idx].state = nextState;
	key[idx].dirty = true;
}

//for a key that has changed state, move it to the next position in the state machine
static void next_key_state(int idx, bool button) {
	key[idx].dirty= false;

	switch (key[idx].state) {
	case IDLE:
		if (button == KEYPAD_CLOSED) {
			set_key_state(idx, PRESSED);
			hold_timer = HAL_GetTick();
		}		
		break;
	case PRESSED:
		if ((HAL_GetTick() - hold_timer) > KEYPAD_HOLD_TIME)
			set_key_state(idx, HOLD);
		else if (button == KEYPAD_OPEN)	
			set_key_state(idx, RELEASED);
		break;
	case HOLD:
		if (button == KEYPAD_OPEN)
			set_key_state(idx, RELEASED);
		break;
	case RELEASED:
		set_key_state(idx, IDLE);
		break;
	}
}

//loop through the list of keypad buttons that have had state change and invoke the next
//step in the state machine. also purge buttons that are idle from this list
//return true if anything changed in the monitored list
static bool update_list() {

	bool list_changed = false;		
	
	for(int i = 0 ; i < KEYPAD_LIST_MAX ; i++) {
		if (key[i].state == IDLE) {
			key[i].mapping = KEYPAD_NO_KEY;
			key[i].coordinate = -1;
			key[i].dirty = false;
		}
	}
	
	for(int r = 0 ; r < keypad_config.num_rows; r++) {		
		for (int c = 0; c < keypad_config.num_columns; c++) {
			bool button = bitRead(keypad_mask[r], c);
			char mapping = keymap[r * keypad_config.num_columns + c];
			int coordinate = r * keypad_config.num_columns + c;
			int idx = find_in_list_by_coordinate(coordinate);
			if(idx > -1) {
				next_key_state(idx, button);
			}
			if((idx == -1) && button) {
				for (int i = 0; i < KEYPAD_LIST_MAX; i++) {
					if (key[i].mapping == KEYPAD_NO_KEY) {
						key[i].mapping = mapping;
						key[i].coordinate = coordinate;
						key[i].state = IDLE; 	
						next_key_state(i, button);
						break;	
					}
				}
			}
		}
	}

	for(int i = 0 ; i < KEYPAD_LIST_MAX ; i++) {
		if (key[i].dirty)
		{
			list_changed = true;
		}		
	}
	return list_changed;
}

//check if any buttons have changed state 
static bool get_keys(void)
{
	bool key_activity = false;
	if (HAL_GetTick() - start_time > KEYPAD_DEBOUNCE_TIME)
	{
		scan_keys();
		key_activity = update_list();
		start_time = HAL_GetTick();
	}

	return key_activity;
}

//return the most recently changed key
static char get_key(void)
{
	if (get_keys() && key[0].dirty && (key[0].state == PRESSED))
	{	
		return key[0].mapping;
	}
	return KEYPAD_NO_KEY;
}

//export the interface
const struct keypad Keypad= { 
	.add_row = add_row, 
	.add_col = add_col, 
	.set_keymap = set_keymap,
	.get_key = get_key,
};