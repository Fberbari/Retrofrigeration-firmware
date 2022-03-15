#pragma once

#include "main.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * This module holds any definitions, structures, functions or otherwise
 * that are useful to a multitude of other modules.
 */

/***********************************************************************************************************************
 * General Definitions
 **********************************************************************************************************************/
typedef struct
{
    //5 probe readings in degrees celsius
    float t1;
    float t2;
    float t3;
    float t4;
    float t5;
}temp_probe_readings_t;
#define CTRL_LOOP_FREQUENCY 50  // Hz


#define RETROFRIGERATION_BUSY         2
#define RETROFRIGERATION_SUCCEEDED    1
#define RETROFRIGERATION_FAILED       0

/***********************************************************************************************************************
 * State data structures definitions
 **********************************************************************************************************************/

#define FAN_OFF 0
#define FAN_ON 1

#define COMPRESSOR_OFF 0
#define COMPRESSOR_ON 1

typedef struct
{
    int fan;          // either FAN_ON or FAN_OFF (all other values illegal)
    int compressor;   // either COMPRESSOR_ON or COMPRESSOR_OFF (all other values illegal)

}ActuatorCommands_t;
