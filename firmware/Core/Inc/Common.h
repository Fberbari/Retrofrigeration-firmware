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
typedef struct
{
    //5 probe readings in degrees celsius
    float t1;
    float t2;
    float t3;
    float t4;
    float t5;
}temp_probe_readings;
#define CTRL_LOOP_FREQUENCY 50  // Hz


#define RETROFRIGERATION_BUSY         2
#define RETROFRIGERATION_SUCCEEDED    1
#define RETROFRIGERATION_FAILED       0
