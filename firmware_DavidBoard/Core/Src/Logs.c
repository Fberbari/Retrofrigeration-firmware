#include <esp8266.h>
#include "Logs.h"
#include <string.h>
/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void Logs_Init(UART_HandleTypeDef *huart)
{
	char * SSID = "Cloudwifi-280-907";
	char * PASSWD = "WCRI2013";
//	char * IP = "10.10.113.162";

//	char * d1 = "AT\r\n";
	char data1[4];
	sprintf (data1, "AT\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data1, 4);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data1, 4);
//	sprintf (data, "");

	//
//	char * d2 = "AT+RST\r\n";
	char data2[8];
	sprintf (data2, "AT+RST\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data2, 8);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data2, 8);
//	sprintf (data, "");

//	char * d3 = "AT+CWMODE=3\r\n";
	char data3[13];
	sprintf (data3, "AT+CWMODE=3\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data3, 13);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data3, 13);
//	sprintf (data, "");

//	char * d1 = "AT\r\n";
	char data[80];
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
	HAL_UART_Transmit_IT (huart, (uint8_t *) data, 41);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data, 41);
//	sprintf (data, "");

	char * d4 = "AT+CIPMUX=1\r\n";
	char data4[sizeof(d4)];
	sprintf (data4, "AT+CIPMUX=1\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data4, 13);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data4, 13);
//	sprintf (data, "");

	char * d5 = "AT+CIPSERVER=1,80\r\n";
	char data5[sizeof(d5)];
	sprintf (data5, "AT+CIPSERVER=1,80\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) data5, 19);
	HAL_Delay(5000);
	HAL_UART_Receive_IT (huart, (uint8_t *) data5, 19);
}


int Logs_LogWifi(DataBuffer_t *DataBuffer, UART_HandleTypeDef *huart)
{



//	char data[16];
//	sprintf (data, "AT+CIPSEND=%d,%d\r\n", 0, len);
//
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data, 13);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data, 13);
//	sprintf (data, "");

	//asdfasfd
//	    char * s = "a";
//
//	    HAL_UART_Transmit_IT (huart, (uint8_t *) s, sizeof(s));
////	    HAL_Delay();
//
//	    HAL_UART_Receive_IT (huart, (uint8_t *) s, sizeof(s));
//	ESP_Init("Cloudwifi-280-907", "WCRI2013", "10.10.113.162", &huart);
//	return 0;
//	char data[30];
//	ESP_Init("Cloudwifi-280-907", "WCRI2013", "10.10.113.162", &huart);
	char data[200];
//	int len = strlen (str);
	sprintf(data, "\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%d\",\"%d\",", \
			DataBuffer->temperature[0], DataBuffer->temperature[1], \
			DataBuffer->temperature[2], DataBuffer->temperature[3], \
			DataBuffer->fanIsOn, DataBuffer->compressorIsOn);



	char * s = "AT+CIPSEND=0,2\r\n";
	char sad[sizeof(s)];
	sprintf (sad, "AT+CIPSEND=0,2\r\n");

	HAL_UART_Transmit_IT (huart, (uint8_t *) sad, 18);
	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) sad, 18);
//	HAL_Delay(5000);

	char * b = "ER\r\n";
	char bd[sizeof(b)];
	sprintf (bd, "ER\r\n");
	HAL_UART_Transmit_IT (huart, (uint8_t *) bd, 2);
	HAL_Delay(5000);
//	Wifi_Data(data, &huart);
//
//  // if wifi not connected, return 0
    return 1;
}

int Wifi_Data(char *str, UART_HandleTypeDef *huart)
{
//	char * SSID = "Cloudwifi-280-907";
//	char * PASSWD = "WCRI2013";
////	char * IP = "10.10.113.162";
//
//	char * d1 = "AT\r\n";
//	char data1[sizeof(d1)];
//	sprintf (data1, "AT\r\n");
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data1, sizeof(data1));
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data1, sizeof(data1));
////	sprintf (data, "");
//
//	//
//	char * d2 = "AT+RST\r\n";
//	char data2[sizeof(d2)];
//	sprintf (data2, "AT+RST\r\n");
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data2, 8);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data2, 8);
////	sprintf (data, "");
//
//	char * d3 = "AT+CWMODE=3\r\n";
//	char data3[sizeof(d3)];
//	sprintf (data3, "AT+CWMODE=3\r\n");
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data3, 13);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data3, 13);
////	sprintf (data, "");
//
////	char * d1 = "AT\r\n";
//	char data[80];
//	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data, 41);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data, 41);
////	sprintf (data, "");
//
//	char * d4 = "AT+CIPMUX=1\r\n";
//	char data4[sizeof(d4)];
//	sprintf (data4, "AT+CIPMUX=1\r\n");
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data4, 13);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data4, 13);
////	sprintf (data, "");
//
//	char * d5 = "AT+CIPSERVER=1,80\r\n";
//	char data5[sizeof(d5)];
//	sprintf (data5, "AT+CIPSERVER=1,80\r\n");
//	HAL_UART_Transmit_IT (huart, (uint8_t *) data5, 19);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data5, 19);

//	int len = strlen (str);
	int len = 1;
//	int str_len = strlen (len);
	int str_len = 1;
//	int data_len =
//	char data[15+str_len];
	char data[16];
	char strdata[len];
	sprintf (data, "AT+CIPSEND=0,%d\r\n", len);
	sprintf (strdata, str);

	HAL_UART_Transmit_IT (huart, (uint8_t *) data, 15+str_len);
	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) data, 15+str_len);
//	HAL_Delay(5000);
	HAL_UART_Transmit_IT (huart, (uint8_t *) strdata, len);
//	HAL_Delay(5000);
//	HAL_UART_Receive_IT (huart, (uint8_t *) strdata, len);

//	char * d = "AT+CIPCLOSE=5\r\n";
//	char data5[sizeof(d5)];
//	sprintf (data5, "AT+CIPSERVER=1,80\r\n");
//	sprintf (data, "AT+CIPCLOSE=5\r\n");


	return 1;
}

int Logs_LogFlash(DataBuffer_t *DataBuffer)
{
    // SPI to EEPROM
    return 1;
}
