#pragma once

/**
 * This module holds the interfaces to all data logging (either through Wifi or flash).
 * Author(s): Elaine
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/


typedef struct
{
    float average_temp;
    float min_temp;
    float max_temp;

}LogData_t;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/**
* Initializes internal parameters.
* Should be called exactly once before anything is attempted to be done with this module.
* Failing to call this function, or calling it more than once, voids all guarantees made by this module.
*/
void Logs_Init(void);

/**
* Logs data from sensors, passes to control module.
* @param[in]        LogData          struct containing the average, min and max temperature
* @return           LOG_SUCCEEDED or LOG_FAILED
*/
int Logs_LogLCD(const LogData_t *LogData);
int Logs_LogWifi(const LogData_t *LogData);
int Logs_LogFlash(const LogData_t *LogData);
