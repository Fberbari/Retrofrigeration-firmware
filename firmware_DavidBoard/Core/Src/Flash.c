
#include "Flash.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>

// peripheral handles
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

// structures and variables
int loop_counter;					// counter for log entries
int rolling_avg_sample_counter;		// counter for updating rolling average

uint32_t head_address;				// next vacant address on flash to store next log

storage_buffer_t storage_buffer;					// log entry data format
rolling_average_buffer_t rolling_average_buffer;	// stores data points for rolling average

bool usb_logs_requested = false;
uint8_t mcp_rxBuffer[1];

//private function declarations
// flash related
static void AssertSlaveSelect(void);
static void DeassertSlaveSelect(void);
void sFLASH_WriteEnable(void);
void sFLASH_WriteDisable(void);
void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void sFLASH_ChipErase(void);
void sFLASH_ReadStatus(uint8_t* pBuffer);

// general
void find_log_head(uint32_t *adress);
void rolling_avg_buffer_update(int value);
void uart_send_log_entry(uint8_t* buffer);

int numPlaces(int n);	// returns number of digits in integer


/***********************************************************************************************************************
 * SPI FLASH Low Level Functions
 **********************************************************************************************************************/
static void AssertSlaveSelect(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    return;
}

static void DeassertSlaveSelect(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    return;
}

void sFLASH_WriteEnable(void)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_WREN};

    AssertSlaveSelect();
    HAL_SPI_Transmit (&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();
    HAL_Delay(2);

    return;
}

void sFLASH_WriteDisable(void)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_WRDI};

    AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();
    HAL_Delay(2);

    return;
}

void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    sFLASH_WriteEnable();

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
    HAL_Delay(10);

	return;
}

void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	sFLASH_WriteDisable();

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
    HAL_Delay(10);

	return;
}

void sFLASH_ChipErase(void)
{
	sFLASH_WriteEnable();

    uint8_t command_buffer[1] = {sFLASH_CMD_CE};

    AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();
	HAL_Delay(1000);

    return;
}

void sFLASH_ReadStatus(uint8_t* pBuffer)
{
    uint8_t command_buffer[1] = {sFLASH_CMD_RDSR};

	AssertSlaveSelect();
    HAL_SPI_Transmit(&hspi1, command_buffer, 1, sFLASH_TIMEOUT);
    HAL_SPI_Receive(&hspi1, pBuffer, 1, sFLASH_TIMEOUT);
    DeassertSlaveSelect();

	return;
}




/***********************************************************************************************************************
 * Misc Low Level Functions
 **********************************************************************************************************************/

void rolling_avg_buffer_update(int value)
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
	*adress = sFLASH_BASE_ADDRESS;
	uint8_t read_buffer[3];
	sFLASH_ReadBuffer(read_buffer, (uint32_t)sFLASH_BASE_ADDRESS, (uint16_t)3);
	*adress =
			(uint32_t)read_buffer[0] << 0 |
			(uint32_t)read_buffer[1] << 8 |
			(uint32_t)read_buffer[2] << 16;
	if(*adress == 0)
	{
		*adress = sFLASH_BASE_ADDRESS + 32;		// if no address found, initiate to beginning of flash
	}
}

int numPlaces (int n)
{
    if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}


/***********************************************************************************************************************
 * High Level Functions
 **********************************************************************************************************************/


void Flash_Init(void)
{
	loop_counter= 0;
	rolling_avg_sample_counter = 0;


	sFLASH_ChipErase();
	head_address = sFLASH_BASE_ADDRESS;
//	find_log_head(&head_address);		//fk it we're just gonna start from base every time

	// set up UART to receive any keystroke to request data
	HAL_UART_Receive_IT(&huart1, mcp_rxBuffer, 1);

	return;
}






int Flash_LogData(const DataBuffer_t *DataBuffer)
{
	// update min and max temperatures every cycle
	for (int i = 0; i < 4; i ++)
	{
		int temperature_in_int = (int)(DataBuffer->temperature[i]*100);
		if (storage_buffer.max_temperature < temperature_in_int)
		{
			storage_buffer.max_temperature = temperature_in_int;
		}
		if (temperature_in_int < storage_buffer.min_temperature)
		{
			storage_buffer.min_temperature = temperature_in_int;
		}
	}

	// update rolling average every sFLASH_ROLLING_AVERAGE_SAMPLE_PERIOD cycles
	if(sFLASH_ROLLING_AVERAGE_SAMPLE_PERIOD <= rolling_avg_sample_counter)
	{
		int average = 0;
		for (int i = 0; i < 4; i ++)
		{
			average += (int)(DataBuffer->temperature[i]*100);
		}
		rolling_avg_buffer_update((int)(average/4));
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
		int avg_temp = 0;
		for (int i = 0; i < sFLASH_ROLLING_AVERAGE_SAMPLE_COUNT; i ++)
		{
			avg_temp += rolling_average_buffer.buffer[i];
		}

		// populate buffer with new readings
//		time_stamp;
		storage_buffer.average_temperature = (int)(avg_temp/sFLASH_ROLLING_AVERAGE_SAMPLE_COUNT);
		storage_buffer.batteryVoltage = (int)(DataBuffer->batteryVoltage*100);
		storage_buffer.mainsFailed = DataBuffer->mainsFailed;
		storage_buffer.batteryIsCharging = DataBuffer->batteryIsCharging;
		storage_buffer.compressorIsOn = DataBuffer->compressorIsOn;
		storage_buffer.fanIsOn = DataBuffer->fanIsOn;

		// convert buffer into char array for writing
		uint8_t write_buffer[32];
	    memcpy(write_buffer, &storage_buffer, 32);

		// write char array
		sFLASH_WriteBuffer(write_buffer, head_address, 32);

		//test
		HAL_Delay(10);
		uint8_t read_buffer[32];
		sFLASH_ReadBuffer(read_buffer, head_address, 32);


		// update head address variable
		head_address += 32;
		if(SFLASH_TOTAL_ADDRESS_SIZE <= head_address)
		{
			head_address = sFLASH_BASE_ADDRESS + 32;		// reset head back to zero when it reaches the end of memory
		}

		// write updated head address to flash
//	    uint8_t write_buffer2[3] = {
//	    		(uint8_t)((head_address & 0x000000ff) >> 0),
//	    		(uint8_t)((head_address & 0x0000ff00) >> 8),
//	    		(uint8_t)((head_address & 0x00ff0000) >> 16)
//	    };
//	    sFLASH_WriteBuffer(write_buffer2, sFLASH_BASE_ADDRESS, 4);

	    // test
//	    HAL_Delay(10);
//	    uint8_t read_buffer2[32];
//	    sFLASH_ReadBuffer(read_buffer2, (uint32_t)sFLASH_BASE_ADDRESS, (uint16_t)32);

	    loop_counter = 0;
	}
	else
	{
		loop_counter ++;
	}

    return RETROFRIGERATION_SUCCEEDED;
}







int Flash_PassDataToUSB(void)
{
	if(usb_logs_requested)
	{
		usb_logs_requested = false;
		HAL_UART_Transmit(&huart1, (uint8_t*)"Generating Logs...\r\n", 20, sFLASH_TIMEOUT);
		HAL_UART_Transmit(&huart1, (uint8_t*)"Min Temp | Max Temp | Avg Temp | Bat Volt | Mains Failed | Bat Is Chg | Comp On | Fan On |\r\n", 92, sFLASH_TIMEOUT);

		uint32_t read_pointer = sFLASH_BASE_ADDRESS;
		uint8_t read_buffer[32];

		// iterate through all logs on flash up to head
		while(read_pointer < head_address)
		{
			sFLASH_ReadBuffer(read_buffer, read_pointer, 32);
			uart_send_log_entry(read_buffer);
			read_pointer += 32;
		}

		// send newline
		HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, sFLASH_TIMEOUT);
	}
	return RETROFRIGERATION_SUCCEEDED;
}



// given a 32 byte block of raw data read from the flash, format and pass to uart
void uart_send_log_entry(uint8_t* buffer)
{
	char write_buffer[48];
//	char write_buffer_seperator[3] = " | ";
	uint8_t return_carriage[2] = {"\r\n"};
	int digit_count = 0;

//	// min temperature
//	uint32_t min_temperature =
//			(uint32_t)buffer[4] << 0 |
//			(uint32_t)buffer[5] << 8 |
//			(uint32_t)buffer[6] << 16|
//			(uint32_t)buffer[7] << 24;
//    int min_temperature_intCast;
//    memcpy(&min_temperature_intCast, &min_temperature, sizeof(int));
//    digit_count = numPlaces(min_temperature_intCast);
//	snprintf (write_buffer, 16, "Min Temp: %d", min_temperature_intCast);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 10 + digit_count);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// max temperature
//	uint32_t max_temperature =
//			(uint32_t)buffer[8] << 0 |
//			(uint32_t)buffer[9] << 8 |
//			(uint32_t)buffer[10] << 16|
//			(uint32_t)buffer[11] << 24;
//    int max_temperature_intCast;
//    memcpy(&max_temperature_intCast, &max_temperature, sizeof(int));
//    digit_count = numPlaces(max_temperature_intCast);
//	snprintf (write_buffer, 16, "Max Temp: %d", max_temperature_intCast);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 10 + digit_count);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// average temperature
//	uint32_t avg_temperature =
//			(uint32_t)buffer[12] << 0 |
//			(uint32_t)buffer[13] << 8 |
//			(uint32_t)buffer[14] << 16|
//			(uint32_t)buffer[15] << 24;
//    int avg_temperature_intCast;
//    memcpy(&avg_temperature_intCast, &avg_temperature, sizeof(int));
//    digit_count = numPlaces(avg_temperature_intCast);
//	snprintf (write_buffer, 16, "AVG Temp: %d", avg_temperature_intCast);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 10 + digit_count);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// battery voltage
//	uint32_t batt_voltage =
//			(uint32_t)buffer[16] << 0 |
//			(uint32_t)buffer[17] << 8 |
//			(uint32_t)buffer[18] << 16|
//			(uint32_t)buffer[19] << 24;
//    int batt_voltage_intCast;
//    memcpy(&batt_voltage_intCast, &batt_voltage, sizeof(int));
//    digit_count = numPlaces(batt_voltage_intCast);
//	snprintf (write_buffer, 16, "Bat Volt: %d", batt_voltage_intCast);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 10 + digit_count);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// mains failed
//	snprintf (write_buffer, 16, "Mains Failed: %d", (bool)(buffer[21]));
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 15);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// battery is charging
//	snprintf (write_buffer, 16, "Bat Is Chg: %d", (bool)(buffer[22]));
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 13);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// compressor is on
//	snprintf (write_buffer, 16, "Comp On: %d", (bool)(buffer[23]));
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 10);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);
//
//	// fan is on
//	snprintf (write_buffer, 16, "Fan On: %d", (bool)(buffer[24]));
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer, 9);
//	HAL_Delay(inter_UART_TX_delay);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)write_buffer_seperator, 3);
//	HAL_Delay(inter_UART_TX_delay);

	uint32_t min_temperature =
			(uint32_t)buffer[4] << 0 |
			(uint32_t)buffer[5] << 8 |
			(uint32_t)buffer[6] << 16|
			(uint32_t)buffer[7] << 24;
	int min_temperature_intCast;
	memcpy(&min_temperature_intCast, &min_temperature, sizeof(int));
	digit_count += numPlaces(min_temperature_intCast);

	uint32_t max_temperature =
			(uint32_t)buffer[8] << 0 |
			(uint32_t)buffer[9] << 8 |
			(uint32_t)buffer[10] << 16|
			(uint32_t)buffer[11] << 24;
	int max_temperature_intCast;
	memcpy(&max_temperature_intCast, &max_temperature, sizeof(int));
	digit_count += numPlaces(max_temperature_intCast);

	uint32_t avg_temperature =
			(uint32_t)buffer[12] << 0 |
			(uint32_t)buffer[13] << 8 |
			(uint32_t)buffer[14] << 16|
			(uint32_t)buffer[15] << 24;
	int avg_temperature_intCast;
	memcpy(&avg_temperature_intCast, &avg_temperature, sizeof(int));
	digit_count += numPlaces(avg_temperature_intCast);

	uint32_t batt_voltage =
			(uint32_t)buffer[16] << 0 |
			(uint32_t)buffer[17] << 8 |
			(uint32_t)buffer[18] << 16|
			(uint32_t)buffer[19] << 24;
	int batt_voltage_intCast;
	memcpy(&batt_voltage_intCast, &batt_voltage, sizeof(int));
	digit_count += numPlaces(batt_voltage_intCast);

	snprintf (write_buffer, 48, "%d | %d | %d | %d | %d | %d | %d | %d",
			min_temperature_intCast,
			max_temperature_intCast,
			avg_temperature_intCast,
			batt_voltage_intCast,
			(bool)(buffer[21]),
			(bool)(buffer[22]),
			(bool)(buffer[23]),
			(bool)(buffer[24]));
	HAL_UART_Transmit(&huart1, (uint8_t*)write_buffer, 25 + digit_count, sFLASH_TIMEOUT);

	HAL_UART_Transmit(&huart1, return_carriage, 2, sFLASH_TIMEOUT);

	return;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	usb_logs_requested = true;
	HAL_UART_Receive_IT(&huart1, mcp_rxBuffer, 1);
}

