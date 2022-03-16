#pragma once

/**
 * This module turns ADC voltage readings into temperature (in Celsius)
 * Author(s): Jacob Smith
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/**
* Converts the 10-bit ADC voltage reading from a thermistor into the thermistor's temperature in Celsius
* Only returns failed in the case of illegal arguments; this module is unaware of any actuator failures.
* Returns -KELVIN_TO_CELSIUS if the ADC voltage is 0 or 1023 //IS THIS CORRECT?
* @param[in]        ADC_VALUE          struct containing the ADCs voltage reading (as a fraction of Vdd)
* @return           Temperature reading in Celsius
*/

int Temperature_ADCtoCelsius(DataBuffer_t *DataBuffer);

