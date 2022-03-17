
#include "Flash.h"
#include <string.h>
#include <stdio.h>

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

// peripheral handles
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

// structures and variables
int loop_counter;
int rolling_avg_sample_counter;

uint32_t head_address;
float rolling_average[15];

storage_buffer_t storage_buffer;
rolling_average_buffer_t rolling_average_buffer;


//private function declarations
static void AssertSlaveSelect(void);
static void DeassertSlaveSelect(void);
void sFLASH_WriteEnable(void);
void sFLASH_WriteDisable(void);

void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

void sFLASH_ChipErase(void);
uint32_t sFLASH_ReadID(void);


void find_log_head(uint32_t *adress);

void rolling_avg_buffer_update(float value);

void uart_send_log_entry(uint8_t* buffer);

/***********************************************************************************************************************
 * SPI FLASH Low Level Functions
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
 * Misc Low Level Functions
 **********************************************************************************************************************/

void rolling_avg_buffer_update(float value)
{
	rolling_average_buffer.buffer[rolling_average_buffer.index] = value;
	rolling_average_buffer.index ++;
	if(sFLASH_ROLLING_AVERAGE_SAMPLE_COUNT <= rolling_average_buffer.index)
	{
		rolling_average_buffer.index = 0;
	}
}

void find_log_head(uint32_t *adress)
{
	uint8_t read_buffer[4];
	sFLASH_ReadBuffer(read_buffer, 0, 4);
	head_address =
			(uint32_t)read_buffer[0] << 0 |
			(uint32_t)read_buffer[1] << 8 |
			(uint32_t)read_buffer[2] << 16|
			(uint32_t)read_buffer[3] << 24;
	if(head_address == 0)
	{
		head_address = 32;		// if no address found, initiate to beginning of flash
	}
}


/***********************************************************************************************************************
 * High Level Functions
 **********************************************************************************************************************/


void Flash_Init(void)
{
	loop_counter= 0;
	rolling_avg_sample_counter = 0;

	sFLASH_WriteEnable();

//	find_log_head(&head_address);
	head_address = 256;

	return;
}

int Flash_LogData(const DataBuffer_t *DataBuffer, const SystemOutputState_t *SystemOutputState)
{
	// update min and max temperatures
	for (int i = 0; i < 4; i ++)
	{
		if (storage_buffer.max_temperature < DataBuffer->temperature[i])
		{
			storage_buffer.max_temperature = DataBuffer->temperature[i];
		}
		if (DataBuffer->temperature[i] < storage_buffer.min_temperature)
		{
			storage_buffer.min_temperature = DataBuffer->temperature[i];
		}
	}

	// update rolling average every sFLASH_ROLLING_AVERAGE_SAMPLE_PERIOD cycles
	if(sFLASH_ROLLING_AVERAGE_SAMPLE_PERIOD <= rolling_avg_sample_counter)
	{
		float average = 0;
		for (int i = 0; i < 4; i ++)
		{
			average += DataBuffer->temperature[i];
		}
		rolling_avg_buffer_update(average/4);
		rolling_avg_sample_counter = 0;	//reset counter

	}
	else
	{
		rolling_avg_sample_counter ++;
	}

	// store log entry in flash every sFLASH_LOGGING_PERIOD cycles
	if(sFLASH_LOGGING_PERIOD <= loop_counter)
	{
		// calculate rolling average
		float avg_temp = 0;
		for (int i = 0; i < sFLASH_ROLLING_AVERAGE_SAMPLE_COUNT; i ++)
		{
			avg_temp += rolling_average_buffer.buffer[i];
		}

		// populate buffer with new readings
//		time_stamp;
		storage_buffer.average_temperature = avg_temp/sFLASH_ROLLING_AVERAGE_SAMPLE_COUNT;
		storage_buffer.batteryVoltage = DataBuffer->batteryVoltage;
		storage_buffer.mainsFailed = DataBuffer->mainsFailed;
		storage_buffer.batteryIsCharging = SystemOutputState->batteryIsCharging;
		storage_buffer.compressorIsOn = SystemOutputState->compressorIsOn;
		storage_buffer.fanIsOn = SystemOutputState->fanIsOn;

		// convert buffer into char array for writing
		uint8_t write_buffer[32];
		int size = 32;
		for (uint8_t *pointer = (uint8_t *)&storage_buffer; size--; pointer++)
		{
			write_buffer[31-size] = *pointer;
		}

		// write char array
		sFLASH_WriteBuffer(write_buffer, head_address, 32);

		HAL_Delay(10);

		uint8_t read_buffer[32];
		sFLASH_ReadBuffer(read_buffer, head_address, 32);

		// update head address variable and in flash
		head_address += 32;
		if(SFLASH_TOTAL_ADDRESS_SIZE <= head_address)
		{
			head_address = 32;		// reset head back to zero when it reaches the end of memory
		}
	    uint8_t write_buffer2[3] = {
	    		(head_address & 0x000000ff) >> 0,
				(head_address & 0x0000ff00) >> 8,
				(head_address & 0x00ff0000) >> 16
	    };
	    sFLASH_WriteBuffer(write_buffer2, 0, 1);

	}
	else
	{
		loop_counter ++;
	}

    return RETROFRIGERATION_SUCCEEDED;
}

int Flash_PassDataToUSB(bool* usb_logs_requested)
{
	if(*usb_logs_requested)
	{
		*usb_logs_requested = false;
//		uint32_t read_pointer = head_address;
		uint32_t read_pointer = 32;

		uint8_t read_buffer[32];

		while(read_pointer < head_address)
		{
			sFLASH_ReadBuffer(read_buffer, read_pointer, 32);
			HAL_Delay(100);
			read_pointer +=32;
			uart_send_log_entry(read_buffer);
		}
//		read_pointer = 32;
//		while(read_pointer < head_address)
//		{
//			sFLASH_ReadBuffer(read_buffer, read_pointer, 32);
//			read_pointer +=32;
//			HAL_UART_Transmit(&huart1, read_buffer, 32, sFLASH_TIMEOUT);
//			HAL_UART_Transmit(&huart1, return_carriage, 2, sFLASH_TIMEOUT);
//		}
	}
	return RETROFRIGERATION_SUCCEEDED;
}

void uart_send_log_entry(uint8_t* buffer)
{
	char write_buffer[14];
	char write_buffer_seperator[3] = " | ";
	uint8_t return_carriage[2] = {"\r\n"};

	uint32_t min_temperature =
			(uint32_t)buffer[4] << 0 |
			(uint32_t)buffer[5] << 8 |
			(uint32_t)buffer[6] << 16|
			(uint32_t)buffer[7] << 24;
    float min_temperature_float;
    memcpy(&min_temperature_float, &min_temperature, sizeof(float));
	snprintf (write_buffer, 14, "Min Temp: %f", min_temperature_float);

	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 14);
	HAL_Delay(inter_UART_TX_delay);
	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
	HAL_Delay(inter_UART_TX_delay);


	uint32_t max_temperature =
			(uint32_t)buffer[8] << 0 |
			(uint32_t)buffer[9] << 8 |
			(uint32_t)buffer[10] << 16|
			(uint32_t)buffer[11] << 24;
    float max_temperature_float;
    memcpy(&max_temperature_float, &max_temperature, sizeof(float));
	snprintf (write_buffer, 14, "Max Temp: %f", max_temperature_float);

	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 14);
	HAL_Delay(inter_UART_TX_delay);
	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
	HAL_Delay(inter_UART_TX_delay);


	uint32_t avg_temperature =
			(uint32_t)buffer[8] << 0 |
			(uint32_t)buffer[9] << 8 |
			(uint32_t)buffer[10] << 16|
			(uint32_t)buffer[11] << 24;
    float avg_temperature_float;
    memcpy(&avg_temperature_float, &avg_temperature, sizeof(float));
	snprintf (write_buffer, 14, "AVG Temp: %f", avg_temperature_float);

	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 14);
	HAL_Delay(inter_UART_TX_delay);
	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
	HAL_Delay(inter_UART_TX_delay);

	uint32_t batt_voltage =
			(uint32_t)buffer[8] << 0 |
			(uint32_t)buffer[9] << 8 |
			(uint32_t)buffer[10] << 16|
			(uint32_t)buffer[11] << 24;
    float batt_voltage_float;
    memcpy(&batt_voltage_float, &batt_voltage, sizeof(float));
	snprintf (write_buffer, 14, "Bat Volt: %f", batt_voltage_float);

	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 14);
	HAL_Delay(inter_UART_TX_delay);
	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
	HAL_Delay(inter_UART_TX_delay);


//	float batteryVoltage;		// 4 bytes
//	bool mainsFailed;			// 1 byte
//    bool batteryIsCharging;		// 1 byte
//    bool compressorIsOn;		// 1 byte
//    bool fanIsOn;				// 1 byte
	HAL_UART_Transmit(&huart1, return_carriage, 2, sFLASH_TIMEOUT);

	return;
}


