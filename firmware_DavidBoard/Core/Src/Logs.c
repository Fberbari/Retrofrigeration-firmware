#include <esp8266.h>
#include "Logs.h"

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void Logs_Init(void)
{
}

int Logs_LogLCD(const LogData_t * LogData)
{
    return 1;
}

//int Logs_LogWifi(const LogData_t * LogData)
void Logs_LogWifi()
{
	ESP_Init("Cloudwifi-280-907", "WCRI2013");
	Server_Send("ddfsdf", -48);
  // check wifi connnection
  // if wifi not connected, return 0
  // Wifi_Init()
  // Wifi_TcpIp_GetConnectionStatus
    // UART to TCP IP
    return 1;
}

int Logs_LogFlash(const LogData_t * LogData)
{
    // SPI to EEPROM
    return 1;
}
