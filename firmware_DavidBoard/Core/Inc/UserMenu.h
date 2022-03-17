#pragma once

/**
 * This module manages the User menu
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
void UserMenu_Init(void);

/**
* Based on current input, previous input, and current data, outputs the string that needs to pushed to the LCD.
* This is a non blocking function that returns right away.
* @param[in]        DataBuffer                All systems data.
* @param[out]       outputStringTop           string to be shown on the top row of the LCD
* @param[out]       outputStringBottom        string to be shown on the bottom row of the LCD
*/
void UserMenu_DetermineLCDString(const DataBuffer_t *DataBuffer, char *outputStringTop, char *outputStringBottom);

/**
* Returns the user requested temperature and temperature bounds.
* @param[out]       OutputUserSettings           Struct containing the user settings.
*/
void UserMenu_GetUserSettings(UserSettings_t *OutputUserSettings);
