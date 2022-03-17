/*
 * esp8266.c
 *
 */
#include "esp8266.h"
#include "stdio.h"
#include "string.h"
#include "Common.h"
//extern UART_HandleTypeDef huart1;
//extern UART_HandleTypeDef huart2;
//
//
//#define wifi_uart &huart2


char buffer[20];


/*****************************************************************************************************************************************/

void ESP_Init (char *SSID, char *PASSWD, char *IP, UART_HandleTypeDef *huart)
{
	char data[80];
	char * d = "AT\r\n";
	HAL_StatusTypeDef status = HAL_UART_Transmit_IT (huart, (uint8_t *) d, sizeof(d));
	HAL_Delay(1000);

	d = "AT+RST\r\n";
	HAL_UART_Transmit_IT (huart, (uint8_t *) d, sizeof(d));
	HAL_Delay(1000);

	d = "AT+CWMODE=3\r\n";
	HAL_UART_Transmit_IT (huart, (uint8_t *) d, sizeof(d));
	HAL_Delay(1000);

	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
	HAL_UART_Transmit_IT (huart, (uint8_t *) data, sizeof(data));
	HAL_Delay(1000);

	d = "AT+CIPMUX=1\r\n";
	HAL_UART_Transmit_IT (huart, (uint8_t *) d, sizeof(d));
	HAL_Delay(1000);

	d = "AT+CIPSERVER=1,80\r\n";
	HAL_UART_Transmit_IT (huart, (uint8_t *) d, sizeof(d));
	HAL_Delay(1000);


//	HAL_UART_Receive_IT (huart, (uint8_t *) s, sizeof(s));
//	Ringbuf_init();
//
//	Uart_sendstring("AT+RST\r\n", wifi_uart);
//	/********* AT **********/
//	Uart_flush(wifi_uart);
//	Uart_sendstring("AT\r\n", wifi_uart);
//	while(!(Wait_for("OK\r\n", wifi_uart)));
//
//	/********* AT+CWMODE=1 **********/
//	Uart_flush(wifi_uart);
//	Uart_sendstring("AT+CWMODE=3\r\n", wifi_uart);
//	while (!(Wait_for("OK\r\n", wifi_uart)));
//
////	/********* AT+STAIP=10.10.113.162 **********/
////	Uart_flush(wifi_uart);
////	sprintf (data, "AT+STAIP=\"%s\"\r\n", IP);
////	Uart_sendstring(data, wifi_uart);
////	while (!(Wait_for("OK\r\n", wifi_uart)));
//
//	/********* AT+CWJAP="SSID","PASSWD" **********/
//	Uart_flush(wifi_uart);
////	Uart_sendstring("connecting... to the provided AP\n", pc_uart);
//	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWD);
//	Uart_sendstring(data, wifi_uart);
//	while (!(Wait_for("OK\r\n", wifi_uart)));
//
////	/********* AT+CIFSR **********/
////	Uart_flush(wifi_uart);
////	Uart_sendstring("AT+CIFSR\r\n", wifi_uart);
////	while (!(Wait_for("CIFSR:STAIP,\"", wifi_uart)));
////	while (!(Copy_upto("\"",buffer, wifi_uart)));
////	while (!(Wait_for("OK\r\n", wifi_uart)));
//
//	/********* AT+CIPMUX **********/
//	Uart_flush(wifi_uart);
//	Uart_sendstring("AT+CIPMUX=1\r\n", wifi_uart);
//	while (!(Wait_for("OK\r\n", wifi_uart)));
//
//	/********* AT+CIPSERVER **********/
//	Uart_flush(wifi_uart);
//	Uart_sendstring("AT+CIPSERVER=1,80\r\n", wifi_uart);
//	while (!(Wait_for("OK\r\n", wifi_uart)));
}




int Server_Send (char *str, int Link_ID)
{
//	int len = strlen (str);
//	char data[30];
//	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, len);
//	Uart_sendstring(data, wifi_uart);
//	while (!(Wait_for(">", wifi_uart)));
//	Uart_sendstring (str, wifi_uart);
//	while (!(Wait_for("SEND OK", wifi_uart)));
//	sprintf (data, "AT+CIPCLOSE=5\r\n");
//	Uart_sendstring(data, wifi_uart);
//	while (!(Wait_for("OK\r\n", wifi_uart)));
	return 1;
}
