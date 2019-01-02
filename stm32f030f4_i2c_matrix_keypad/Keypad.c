#include "Keypad.h"

static Pin row_pins[KEYPAD_MAX_PINS];
static unsigned int row_pins_usage_mask = 0;

static Pin col_pins[KEYPAD_MAX_PINS];
static unsigned int col_pins_usage_mask = 0;

static unsigned int bitmap[KEYPAD_MAX_PINS];

static unsigned int start_time = 0;
static unsigned int debounce_time = 10;

static char keymap[KEYPAD_MAX_PINS*KEYPAD_MAX_PINS+1];

static Key key[LIST_MAX];

static KeypadSize keypad_size;

static bool single_key = true;

static long hold_timer;
unsigned int hold_time= 500;

void(*keypadEventListener)(char) = 0;


static int8_t get_unused_idx(int* usage_mask) {
	uint8_t i = 0;
	for (i = 0; i < KEYPAD_MAX_PINS; i++) {
		if (bitRead(*usage_mask, i) == 0) {
			bitSet(*usage_mask, i);
			return i;
		}	
	} 
	return -1;
}
static bool idx_valid(int8_t idx, int* usage_mask) {
	return idx >= 0 && idx < KEYPAD_MAX_PINS && (bitRead(*usage_mask, idx) == 1);			
}
static int get_pin_count(int* usage_mask)
{
	int i = 0;
	for (; idx_valid(i, usage_mask); i++);
	return i;
}

static bool add_generic(Pin* pins, int* usage_mask, GPIO_TypeDef* gpio_port, unsigned int gpio_pin) {	
	uint8_t idx;	
	idx = get_unused_idx(usage_mask);
	if (idx_valid(idx, usage_mask)) {
		pins[idx].gpio_pin = gpio_pin;
		pins[idx].gpio_port= gpio_port;		
		return true;
	}
	return false;
}
static bool add_row(GPIO_TypeDef* gpio_port, unsigned int gpio_pin)
{
	bool status = add_generic(row_pins, &row_pins_usage_mask, gpio_port, gpio_pin);
	keypad_size.rows = get_pin_count(&row_pins_usage_mask);
	return status;

}

static bool add_col(GPIO_TypeDef* gpio_port, unsigned int gpio_pin)
{
	bool status =  add_generic(col_pins, &col_pins_usage_mask, gpio_port, gpio_pin);
	keypad_size.columns = get_pin_count(&col_pins_usage_mask);
	return status;

}

static void set_keymap(char* keymap_in)
{
	snprintf(keymap, KEYPAD_MAX_PINS*KEYPAD_MAX_PINS+1,"%s", keymap_in);
}

static void scan_keys(void) {	
	for (int col = 0; col < keypad_size.columns; col++)
	{		
		HAL_GPIO_WritePin(col_pins[col].gpio_port, col_pins[col].gpio_pin, GPIO_PIN_RESET);
		for (int row = 0; row < keypad_size.rows; row++)
		{	
			bool pinstate = !HAL_GPIO_ReadPin(row_pins[row].gpio_port, row_pins[row].gpio_pin);
			
			bitWrite(bitmap[row], col, pinstate);	
		}
		HAL_GPIO_WritePin(col_pins[col].gpio_port, col_pins[col].gpio_pin, GPIO_PIN_SET);		
	}
	
}
static int find_in_list_by_keychar(char keyChar) {
	for (int i = 0; i < LIST_MAX; i++) {
		if (key[i].key_char == keyChar) {
			return i;
		}
	}
	return -1;
}
static int find_in_list_by_keycode(int keyCode) {
	for (int i = 0; i < LIST_MAX; i++) {
		if (key[i].key_code == keyCode) {
			return i;
		}
	}
	return -1;
}
static void transition_to(int idx, KeyState nextState) {
	key[idx].state = nextState;
	key[idx].state_changed = true;
	
	if(single_key) {
		if ((keypadEventListener != NULL) && (idx == 0)) {
			keypadEventListener(key[0].key_char);
		}
	}
	
	else {
		if (keypadEventListener != NULL) {
			keypadEventListener(key[idx].key_char);
		}
	}
}
static void next_key_state(int idx, bool button) {
	key[idx].state_changed= false;

	switch (key[idx].state) {
	case IDLE:
		if (button == CLOSED) {
			transition_to(idx, PRESSED);
			hold_timer = HAL_GetTick();
		}		
		break;
	case PRESSED:
		if ((HAL_GetTick() - hold_timer) > hold_time)
			transition_to(idx, HOLD);
		else if (button == OPEN)	
			transition_to(idx, RELEASED);
		break;
	case HOLD:
		if (button == OPEN)
			transition_to(idx, RELEASED);
		break;
	case RELEASED:
		transition_to(idx, IDLE);
		break;
	}
}


static bool update_list() {

	bool anyActivity = false;		
	
	for(int i = 0 ; i < LIST_MAX ; i++) {
		if (key[i].state == IDLE) {
			key[i].key_char = NO_KEY;
			key[i].key_code = -1;
			key[i].state_changed = false;
		}
	}

	
	for(int r = 0 ; r < keypad_size.rows; r++) {		
		for (int c = 0; c < keypad_size.columns; c++) {
			bool button = bitRead(bitmap[r], c);
			char keyChar = keymap[r * keypad_size.columns + c];
			int keyCode = r * keypad_size.columns + c;
			int idx = find_in_list_by_keycode(keyCode);
			if(idx > -1) {
				next_key_state(idx, button);
			}
			if((idx == -1) && button) {
				for (int i = 0; i < LIST_MAX; i++) {
					if (key[i].key_char == NO_KEY) {
						key[i].key_char = keyChar;
						key[i].key_code = keyCode;
						key[i].state = IDLE; 	
						next_key_state(i, button);
						break;	
					}
				}
			}
		}
	}

	for(int i = 0 ; i < LIST_MAX ; i++) {
		if (key[i].state_changed) anyActivity = true;
	}

	return anyActivity;
}
static bool get_keys(void)
{
	bool key_activity = false;
	if (HAL_GetTick() - start_time > debounce_time)
	{
		scan_keys();
		key_activity = update_list();
		start_time = HAL_GetTick();
	}

	return key_activity;
}
static char get_key(void)
{
	if (get_keys() && key[0].state_changed && (key[0].state == PRESSED))
	{	
		return key[0].key_char;
	}
	return NO_KEY;
}


const struct keypad Keypad= { 
	.add_row = add_row, 
	.add_col = add_col, 
	.set_keymap = set_keymap,
	.get_key = get_key,
};