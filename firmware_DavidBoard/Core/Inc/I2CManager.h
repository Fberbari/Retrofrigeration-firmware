#pragma once

/**
 * This module manages the common I2C bus.
 * It acts as a middleman between the higher level drivers and the I2C bus
 * in such a way that the high level drivers appear to be directly interacting with 
 * the peripherals hooked up to the bus.
 * At the moment, those 3 peripherals are the ADC connecting to the thermistors, the first
 * I/O expander, connecting to the fan, the compressor, and the the push buttons, and the second I/O expander, connecting to the LCD
 * Author(s): Anthony Berbari
 */

#include "Common.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

typedef struct
{
    uint16_t thermistor[4];  // 10 bit raw ADC values corresponding to each thermistor

}ThermistorADC_t;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/**
* Initializes internal parameters.
* Should be called exactly once before anything is attempted to be done with this module.
* Failing to call this function, or calling it more than once, voids all guarantees made by this module.
*/
void I2CManager_Init(void);

/**
* Provides the most recently collected data from the thermistor ADC.
* Returns succeeded if the data is valid, failed if it is invalid and busy if the reading has not been
* updated since last time this function was called.
* This is a non blocking function that returns right away.
* @param[out]        ThermistorADC          struct containing the most recent ADC readings
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int I2CManager_GetRawThermistorADC(ThermistorADC_t *ThermistorADC);

/**
* Provides the most recently measured states of all buttons in the system.
* Returns succeeded if the data is valid, failed if it is invalid and busy if the reading has not been
* updated since last time this function was called.
* This is a non blocking function that returns right away.
* @param[out]        PushButtonStates          struct containing the most recent button states
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int I2CManager_GetPushButtonStates(PushButtonStates_t *PushButtonStates);

/**
* Sets the appropriate bits in the I/O expander to the states specified in the input struct.
* Returns succeeded if the PREVIOUS transmission succeeded, failed if the previous transmission failed,
* and busy if the previous command was not yet transmitted. In the case of a busy return, the previous values
* will be overwritten and the module will attempt to send the most recent commands.
* This is a non blocking function that returns right away.
* @param[in]        ActuatorCommands         the desired actuator states
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int I2CManager_SendActuatorCommands(const ActuatorCommands_t *ActuatorCommands);


/**
* Displays the requested string on the LCD.
* The string must be at most 15 characters long, not including the null terminator.
* Characters will only be displayed on the top row.
* The string must end with the null terminator. Otherwise, undefined behavior will occur.
* Returns succeeded if the PREVIOUS transmission succeeded, failed if the previous transmission failed,
* and busy if the previous strings were not yet transmitted. In the case of a busy return, the parameter string will
* be ignored and the module will continue sending the previous string.
* This is a non blocking function that returns right away.
* @param[in]        topStr         the string to be displayed in the top row.
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int I2CManager_SendToLCD(const char *str);

/**
* Begins all I2C exchanges necessary to collect and transmit all the above data.
* This function should be called exactly once per control loop period, though is safe to call more often.
* Returns succeeded if the transmission was started successfully, failed if the transmission failed to start
* and busy if an exchange is in progress. In the case of a busy return, nothing happens.
* This function takes less than 5ms to finish executing.
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int I2CManager_LaunchExchange(void);
