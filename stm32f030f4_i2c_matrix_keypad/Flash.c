#include "Flash.h"

static const int FLASH_MAGIC_INT = 1234567890;

static HAL_StatusTypeDef configure(void)
{
	int first_run = Flash.get(0, type_int).int_val;
	if (first_run != FLASH_MAGIC_INT)
	{
		if (Flash.erase() != HAL_OK)
		{
			Error_Handler();
		}		
		if (Flash.set_int(0, FLASH_MAGIC_INT) != HAL_OK)
		{
			Error_Handler();
		}
	}
	return HAL_OK;
}
static HAL_StatusTypeDef erase(void) {

	static FLASH_EraseInitTypeDef EraseInitStruct;
	unsigned long page_error;
	HAL_StatusTypeDef status;
	
	HAL_FLASH_Unlock();	
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
	EraseInitStruct.NbPages     = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	status = HAL_FLASHEx_Erase(&EraseInitStruct, &page_error);
		
	HAL_FLASH_Lock();		
	
	return status;
}
static HAL_StatusTypeDef marshal(unsigned int idx, unsigned int value) {
	unsigned long address;
	HAL_StatusTypeDef status;
	HAL_FLASH_Unlock();	
	//https://stackoverflow.com/questions/28498191/cant-write-to-flash-memory-after-erase
	//grr... looks like by default you can only write once to flash w/o clearing this...
	CLEAR_BIT(FLASH->CR, (FLASH_CR_PG));

	address = FLASH_USER_START_ADDR + (idx * sizeof(unsigned int));
	if (address < FLASH_USER_END_ADDR) {
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, value);		
		HAL_FLASH_Lock();
		return status;
	}
	
	HAL_FLASH_Lock();
	return HAL_ERROR;
}

static __IO unsigned int unmarshal(unsigned int idx) {
	unsigned long address;

	address = FLASH_USER_START_ADDR + (idx * sizeof(unsigned int));
	
	if (address < FLASH_USER_END_ADDR) {
		return *(__IO unsigned int*)address;
	}	
	
	return 0;
}
static unsigned int record_size(FLASH_RECORD *record) {
	switch (record->val_type)
	{
	case type_float: 
		return sizeof(float);
	case type_int: 
		return sizeof(int);
	case type_uint:
		return sizeof(unsigned int);
	case type_double:
		return sizeof(double);
	case type_bool:
		return sizeof(bool);
	}
}

static HAL_StatusTypeDef set(FLASH_RECORD record) {
	unsigned int i = 0;
	HAL_StatusTypeDef status;

	for (; i < record_size(&record) / 4; i++)
	{
		status = marshal(record.idx++, record.uint_array[i]);
		if (status != HAL_OK)
		{
			return status;
		}							
	}
	return HAL_OK;
}
static HAL_StatusTypeDef set_double(unsigned int idx, double value) {
	FLASH_RECORD record;
	record.val_type = type_double;
	record.idx = idx * 2;
	record.double_val = value;

	return set(record);
}
static HAL_StatusTypeDef set_float(unsigned int idx, float value) {
	FLASH_RECORD record;
	record.val_type = type_float;
	record.idx = idx;
	record.float_val = value;

	return set(record);
}
static HAL_StatusTypeDef set_int(unsigned int idx, int value) {
	FLASH_RECORD record;
	record.val_type = type_int;
	record.idx = idx;
	record.int_val = value;

	return set(record);
}
static FLASH_RECORD get(unsigned int idx, VALUE_TYPE val_type) {
	unsigned int i = 0;	 
	FLASH_RECORD record;
	record.idx = idx;
	if (val_type == type_double)
	{
		idx *= 2;
	}
	record.val_type = val_type;
	
	for (; i < record_size(&record) / 4; i++) {
		record.uint_array[i] = unmarshal(idx++);
	}

	return record;
}



const struct flash Flash = { 
	.configure = configure,
	.erase = erase,
	.set = set,
	.set_double = set_double,
	.set_float = set_float,
	.set_int = set_int,
	.get = get
};
