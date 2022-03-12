
#include "I2CManager.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

typedef struct
{
    uint16_t thermistor[4];  // 10 bit raw ADC values corresponding to each thermistor

}ThermistorADC_t;

#define BUTTON_OPEN 0
#define BUTTON_PRESSED 1

typedef struct
{
    int button[4];  // each element corresponds to a button state. Legal states are BUTTON_OPEN and BUTTON_PRESSED

}PushButtonStates_t;


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


// TOO Jacob to add LCD function


/**
* Begins all I2C exchanges necessary to collect and transmit all the above data.
* This function should be called exactly once per control loop period, though is safe to call more often.
* Returns succeeded if the transmission was started successfully, failed if the transmission failed to start
* and busy if an exchange is in progress. In the case of a busy return, nothing happens.
* This is a non blocking function that returns right away.
* @return           RETROFRIGERATION_SUCCEEDED, RETROFRIGERATION_BUSY or RETROFRIGERATION_FAILED
*/
int I2CManager_LaunchExchange(void);
