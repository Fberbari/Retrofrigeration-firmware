#pragma once

/**
 * This module holds the interfaces to all actuators.
 * At the moment that comprises the fridge fan and the fridge compressor.
 * Author(s): Anthony Berbari
 */

#include "Common.h"

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
