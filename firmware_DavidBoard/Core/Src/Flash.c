
#include "Flash.h"

//find size of array
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

// SPI1 handle
extern SPI_HandleTypeDef hspi1;


// low level functions; private
// flag set access
static void AssertSlaveSelect(void);
static void DeassertSlaveSelect(void);
void sFLASH_WriteEnable(void);
void sFLASH_WriteDisable(void);
// read write access
void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
// miscellaneous access
void sFLASH_ChipErase(void);
uint32_t sFLASH_ReadID(void);



/***********************************************************************************************************************
 * DEFINITIONS: Low Level Functions
 **********************************************************************************************************************/
static void AssertSlaveSelect(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    return;
}

static void DeassertSlaveSelect(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    return;
}

void sFLASH_WriteEnable(void)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_WREN};

    AssertSlaveSelect();
    HAL_SPI_Transmit (&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();

    return;
}

void sFLASH_WriteDisable(void)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_WRDI};

    AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();

    return;
}

void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_PPRO};
    uint8_t address_buffer[3] = {
    		(WriteAddr & 0x000000ff) >> 0,
			(WriteAddr & 0x0000ff00) >> 8,
			(WriteAddr & 0x00ff0000) >> 16
    };

	AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    HAL_SPI_Transmit(&hspi1, address_buffer, 3, sFLASH_TIMEOUT);
    HAL_SPI_Transmit(&hspi1, pBuffer, NumByteToWrite, sFLASH_TIMEOUT);
    DeassertSlaveSelect();

	return;
}

void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_READ};
    uint8_t address_buffer[3] = {
    		(ReadAddr & 0x000000ff) >> 0,
			(ReadAddr & 0x0000ff00) >> 8,
			(ReadAddr & 0x00ff0000) >> 16
    };

	AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    HAL_SPI_Transmit(&hspi1, address_buffer, 3, sFLASH_TIMEOUT);
    HAL_SPI_Receive(&hspi1, pBuffer, NumByteToRead, sFLASH_TIMEOUT);
    DeassertSlaveSelect();

	return;
}

void sFLASH_ChipErase(void)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_CE};

    AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();

    return;
}

/***********************************************************************************************************************
 * High Level Functions
 **********************************************************************************************************************/


void Flash_Init(void)
{
	loop_counter = 0;

	sFLASH_WriteEnable();
	return;
}

int Flash_StoreData(const DataBuffer_t *DataBuffer)
{
	static bool flash_has_been_read = false;
	static uint8_t read_value;

	if(!flash_has_been_read)
	{
		uint8_t write_buffer[1] = {0xAE};
		uint8_t read_buffer[1];
		uint32_t address = 0x0;

		sFLASH_WriteBuffer(write_buffer, address, 1);
		HAL_Delay(20);
		sFLASH_ReadBuffer(read_buffer, address, 1);


		read_value = read_buffer[0];
		//read_value = 0xAE;
		flash_has_been_read = true;
	}


	if(read_value & 0x1)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
	read_value = read_value >> 1;
	if(!read_value)
	{
		flash_has_been_read = false;
	}

    return RETROFRIGERATION_SUCCEEDED;
}

int Flash_PassDataToUSB(void)
{
	return 0;
}




