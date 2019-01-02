#pragma once
#include <string.h>
#include <stdbool.h>
typedef struct {
	void(*write_it)(const void* buffer, int size, void(*callback)(void));	
	void(*read_it)(char *buffer, int size, void(*callback)(void));	
} SerialCommandAdapter;