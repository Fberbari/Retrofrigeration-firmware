#pragma once

/**
 * This module holds the interfaces to all actuators.
 * At the moment that comprises the fridge fan and the fridge compressor.
 * Author(s): Anthony Berbari
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
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

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/**
* Initializes internal parameters.
* Should be called exactly once before anything is attempted to be done with this module.
* Failing to call this function, or calling it more than once, voids all guarantees made by this module.
*/
void Actuators_Init(void);

/**
* Actuates the actuators in the system according to the caller's instructions.
* This is a non blocking function that returns right away.
* Only returns failed in the case of illegal arguments, this module is unaware of any actuator failures.
* @param[in]        ActuatorCommands          struct containing the requested actuator states
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int Actuators_ActuateSystem(const ActuatorCommands_t *ActuatorCommands);
