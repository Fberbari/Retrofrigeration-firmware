#include "Logs.h"
#include "esp8266.h"

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void Logs_Init(void)
{
}

int Logs_LogData(const LogData_t * LogData)
{
    if(Logs_WifiData(* LogData)):
        return LOG_SUCCEEDED;
    elif(Logs_FlashData(* LogData)):
        return LOG_SUCCEEDED;
    else:
        return LOG_FAILED;
}

bool Logs_WifiData(const LogData_t * LogData)
{
  // check wifi connnection
  // if wifi not connected, return 0
  // Wifi_Init()
  // Wifi_TcpIp_GetConnectionStatus
    // UART to TCP IP
    return 1;
}

bool Logs_FlashData(const LogData_t * LogData)
{
    // SPI to EEPROM
    return 1;
}
