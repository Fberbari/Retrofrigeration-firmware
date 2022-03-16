#pragma once

/**
 * This module turns ADC voltage readings into temperature (in Celsius)
 * Author(s): Jacob Smith
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

#define ADC_QUANTIZATION 1024                     	//Number of unique ADC values
#define ADC_V_FS 2048								//ADC full-scale voltage (mV)
#define BOARD_V_FS 3.3                            	//Full-scale board voltage
#define BOARD_V_FS_1000 3300						//Full-scale board voltage in mV
#define PCB_DIVIDING_RESISTANCE 12000               //On-board resistor used for voltage division
//Coeffs for Steinhart-Hart R-T equation of a thermistor
//#define STEINHART_HART_A 0.00146162571738         //
//#define STEINHART_HART_B 0.000239364498811        //
//#define STEINHART_HART_C 0.0000000096852654336    //9.6852654336×10^−8
#define KELVIN_TO_CELSIUS 273.15                  //Steinhart-Hart eqn gives T in K - use this to convert to Celsius
#define BETA_PARAMETER 3892
#define RESIST_PARAMETER 0.00480714677511
#define RESIST_PARAM_INVERSE 208.0236046
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

