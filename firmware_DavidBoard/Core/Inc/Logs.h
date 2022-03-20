#pragma once

/**
 * This module holds the interfaces to all data logging (either through Wifi or flash).
 * Author(s): Elaine
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/**
* Initializes internal parameters.
* Should be called exactly once before anything is attempted to be done with this module.
* Failing to call this function, or calling it more than once, voids all guarantees made by this module.
*/
void Logs_Init(UART_HandleTypeDef *huart);

/**
* Logs data from sensors, passes to control module.
* @param[in]        TempData          struct containing the average, min and max temperature
* @return           LOG_SUCCEEDED or LOG_FAILED
*/

int Logs_LogWifi(DataBuffer_t *DataBuffer, UART_HandleTypeDef *huart);
int Logs_LogFlash(DataBuffer_t *DataBuffer);
