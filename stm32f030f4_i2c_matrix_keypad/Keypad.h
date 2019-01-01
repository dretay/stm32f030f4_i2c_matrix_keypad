#pragma once

#include "bithelper.h"
#include "UartPrinter.h"

#include "stm32f0xx_hal.h"

#include <stdbool.h>

#define KEYPAD_MAX_PINS 4
#define LIST_MAX 10

#define OPEN false
#define CLOSED true

const static char NO_KEY = '\0';

typedef struct
{
	unsigned int gpio_pin;
	GPIO_TypeDef* gpio_port;
} Pin;
typedef struct
{
	int rows;
	int columns;
} KeypadSize;

typedef enum{ IDLE, PRESSED, HOLD, RELEASED } KeyState;

typedef struct
{
	char key_char;
	int key_code;
	KeyState state;
	bool state_changed;
} Key;

struct keypad {
	bool(*add_row)(GPIO_TypeDef* gpio_port, unsigned int gpio_pin);
	bool(*add_col)(GPIO_TypeDef* gpio_port, unsigned int gpio_pin);
	void(*set_keymap)(char* keymap_in);
	char(*get_key)(void);
};
extern const struct keypad Keypad;
