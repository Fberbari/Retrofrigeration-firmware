/*
 * TempCalc.c
 *
 *  Created on: Mar 13, 2022
 *      Author: JacobS
 */

#include "TemperatureCalc.h"
#include "I2CManager.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
#define ADC_QUANTIZATION 1024                     	//Number of unique ADC values
#define ADC_V_FS 2048								//ADC full-scale voltage (mV)
#define BOARD_V_FS 3.3                            	//Full-scale board voltage
#define BOARD_V_FS_1000 3300						//Full-scale board voltage in mV
#define PCB_DIVIDING_RESISTANCE 12000               //On-board resistor used for voltage division
////Coeffs for Steinhart-Hart R-T equation of a thermistor
//#define STEINHART_HART_A 0.00146162571738         //
//#define STEINHART_HART_B 0.000239364498811        //
//#define STEINHART_HART_C 0.0000000096852654336    //9.6852654336×10^−8
////Coeffs for Beta Parameter R-T equation of a Thermistor
//#define BETA_PARAMETER 3892                         
//#define RESIST_PARAMETER 0.00480714677511
//#define RESIST_PARAM_INVERSE 208.0236046
#define KELVIN_TO_CELSIUS 273.15                    //Our eqn gives T in K - use this to convert to Celsius

/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

// HAL handle
/*extern I2C_HandleTypeDef hi2c2;
static uint8_t rawAdcData[16];
static bool adcDataNew;
static I2CBusStatus_t currentBusStatus;*/




/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

int Temperature_ADCtoCelsius(DataBuffer_t *DataBuffer)
{
    ThermistorADC_t ThermistorADC;
    double TCelsius;
    int V1000int;
    int R1000int;

    I2CManager_GetRawThermistorADC(&ThermistorADC);
    for (int i = 0; i <= 3; i++) 
    {
        if((ThermistorADC.thermistor[i]) <= 0 || (ThermistorADC.thermistor[i]) >= ADC_QUANTIZATION-1)
        {
            return RETROFRIGERATION_FAILED;
        }
        V1000int = ((ThermistorADC.thermistor[i])*ADC_V_FS)/(ADC_QUANTIZATION-1); //Convert ADC reading to a voltage. Integer math, so we do the calculation in mV to reduce rounding error
        R1000int = V1000int * PCB_DIVIDING_RESISTANCE / (BOARD_V_FS_1000 - V1000int); //Find unknown resistance of voltage divider.
        TCelsius = 0.000000552853*R1000int*R1000int-0.0100254*R1000int+43.8252; //2nd-degree polynomial interpolation of T-R relation - reasonably close to Steinhart-Hart equation in our operating range (+- 0.1 V)
        DataBuffer->temperature[i] = TCelsius;

    }
    return RETROFRIGERATION_SUCCEEDED;
}
