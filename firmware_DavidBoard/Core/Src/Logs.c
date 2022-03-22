#include "Logs.h"
#include <string.h>
/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void Logs_Init(UART_HandleTypeDef *huart)
{
//	char * SSID = "Cloudwifi-280-907"; //17
//	char * PASSWD = "WCRI2013"; //8
//	int len = 41; //25+16

	char * SSID = "BELL516"; //7
	char * PASSWD = "E99F7E315FDD"; //12
	int len = 35; //19+16

	char data1[4];
	sprintf (data1, "AT\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data1, 4);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data1, 4);

	char data2[8];
	sprintf (data2, "AT+RST\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data2, 8);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data2, 8);

	char data3[13];
	sprintf (data3, "AT+CWMODE=3\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data3, 13);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data3, 13);

	char data[len];
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD); //12+41
	HAL_UART_Transmit_IT (huart, (uint8_t *) data, len);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data, len);

	char * d4 = "AT+CIPMUX=1\r\n";
	char data4[sizeof(d4)];
	sprintf (data4, "AT+CIPMUX=1\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data4, 13);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data4, 13);

	char * d5 = "AT+CIPSERVER=1,80\r\n";
	char data5[sizeof(d5)];
	sprintf (data5, "AT+CIPSERVER=1,80\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data5, 19);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data5, 19);
}

int Logs_LogWifi(DataBuffer_t *DataBuffer, UART_HandleTypeDef *huart, bool send)
{
	int a1 = (int) (DataBuffer->temperature[0]*100);
	int b1 = (int) (DataBuffer->temperature[1]*100);
	int c1 = (int) (DataBuffer->temperature[2]*100);
	int d1 = (int) (DataBuffer->temperature[3]*100);
	int fan = DataBuffer->fanIsOn;
	int compressor = DataBuffer->compressorIsOn;

	int size = snprintf(NULL, 0, "%d,%d,%d,%d,%d,%d", a1, b1, c1, d1, fan, compressor);
	char data[size];

	sprintf(data, "%d,%d,%d,%d,%d,%d", a1, b1, c1, d1, fan, compressor);

	int len = 16;
	if(size > 9)
		len++;

	char sad[len];
	sprintf (sad, "AT+CIPSEND=0,%d\r\n", size); //16+
	if (!(send))
		HAL_UART_Transmit_IT (huart, (uint8_t *) sad, len);

	if (send)
		HAL_UART_Transmit_IT (huart, (uint8_t *) data, size);

//  // if wifi not connected, return 0
    return 1;
}

int Logs_LogFlash(DataBuffer_t *DataBuffer)
{
    // SPI to EEPROM
    return 1;
}
