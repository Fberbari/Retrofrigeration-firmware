#pragma once

/**
 * This module contains the main control loop of the system.
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
void Controller_Init(void);

/**
* Executes a cycle of controller.
* Meant to be called in an infinite loop.
*/
void Controller_SaveTheAfricans(void);
