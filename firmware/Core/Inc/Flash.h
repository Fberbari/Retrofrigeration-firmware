#pragma once

/**
 * This module holds the interface to the flash chip
 * Author(s): David Liu
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
void Flash_Init(void);

/**
* Stores collected logs in the flash
* @param[in]        DataBuffer          struct with data to be logged
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int Flash_LogData(const DataBuffer_t *DataBuffer);
int Flash_PassDataToUSB(void);

