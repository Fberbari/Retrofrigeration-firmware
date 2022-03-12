#include "Logs.h"
#include "esp8266.h"

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

int Logs_LogWifi(const LogData_t * LogData)
{
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
