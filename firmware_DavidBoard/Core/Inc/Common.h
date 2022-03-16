#pragma once

#include "main.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * This module holds any definitions, structures, functions or otherwise
 * that are useful to a multitude of other modules.
 */

/***********************************************************************************************************************
 * General Definitions
 **********************************************************************************************************************/
typedef struct
{
	//system inputs
    bool mainsFailed;			// 1 byte; read via the TPS2113 status
    bool batteryDoneCharging;	// 1 byte; read via PCF8574 IO expander
    float batteryVoltage;		// 4 bytes; read via MAX11609 ADC
    float temperature[4];		// 16 bytes; read via MAX11609 ADC
    bool buttonIsClicked[4];	// 4 bytes; read via PCF8574 IO expander

    //system outputs
    bool batteryIsCharging;		// 1 byte;
    bool compressorIsOn;		// 1 byte;
    bool fanIsOn;				// 1 byte;
} DataBuffer_t;



#define CTRL_LOOP_FREQUENCY 25  // Hz


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

#define BUTTON_PRESSED 0
#define BUTTON_OPEN 1

typedef struct
{
    int button[4];  // each element corresponds to a button state. Legal states are BUTTON_OPEN and BUTTON_PRESSED

}PushButtonStates_t;

typedef struct
{
    int setTemp;
    int tempBoundLow;
    int tempBoundHigh;
}UserSettings_t;
