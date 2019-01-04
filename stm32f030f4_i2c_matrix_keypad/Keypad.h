#pragma once

//HAL includes
#include "stm32f0xx_hal.h"

//STL includes
#include <stdbool.h>

//project includes
#include "bithelper.h"

//keypad constants 
#define KEYPAD_MAX_PINS 4
#define KEYPAD_LIST_MAX 10
#define KEYPAD_DEBOUNCE_TIME 10
#define KEYPAD_HOLD_TIME 500

//define shortcuts
#define KEYPAD_OPEN false
#define KEYPAD_CLOSED true
#define KEYPAD_NO_KEY '\0'

//internal enum
typedef enum{ IDLE, PRESSED, HOLD, RELEASED } KeyState;

//internal keypad structs
typedef struct
{
	unsigned int gpio_pin;
	GPIO_TypeDef* gpio_port;
} Pin;
typedef struct
{
	int num_rows;
	int num_columns;
} KeypadConfig;
typedef struct
{
	char mapping;
	int coordinate;
	KeyState state;
	bool dirty;
} Key;

//interface to export
struct keypad {
	bool(*add_row)(GPIO_TypeDef* gpio_port, unsigned int gpio_pin);
	bool(*add_col)(GPIO_TypeDef* gpio_port, unsigned int gpio_pin);
	void(*set_keymap)(char* keymap_in);
	char(*get_key)(void);
};
extern const struct keypad Keypad;
