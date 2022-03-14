#pragma once

#include "main.h"

#include <stdbool.h>

/**
 * This module holds any definitions, structures, functions or otherwise
 * that are useful to a multitude of other modules.
 */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

#define CTRL_LOOP_FREQUENCY 50  // Hz


#define RETROFRIGERATION_BUSY         2
#define RETROFRIGERATION_SUCCEEDED    1
#define RETROFRIGERATION_FAILED       0

typedef struct
{
    bool mainsFailed;          // read via the TPS2113 status
    bool batteryDoneCharging;
    float batteryVoltage;   //
    float temperature[4];
    bool buttonIsClicked[4];

}DataBuffer_t;

typedef struct
{
    bool batteryIsCharging;
    bool compressorIsOn;
    bool fanIsOn;

}SystemOutputState_t;




