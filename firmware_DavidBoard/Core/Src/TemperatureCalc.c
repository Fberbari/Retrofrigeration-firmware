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

/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

// HAL handle
/*extern I2C_HandleTypeDef hi2c2;
static uint8_t rawAdcData[16];
static bool adcDataNew;
static I2CBusStatus_t currentBusStatus;*/

static ThermistorADC_t ThermistorADC;
//static double V1;
//static double R1;
//static double T_K_inverse;
static double TCelsius;
static int V1000int;
static int R1000int;


/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

int Temperature_ADCtoCelsius(DataBuffer_t *DataBuffer);


/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

int Temperature_ADCtoCelsius(DataBuffer_t *DataBuffer)
{
    I2CManager_GetRawThermistorADC(&ThermistorADC);
    for (int i = 0; i <= 3; i++) 
    {
        if((ThermistorADC.thermistor[i]) <= 0 || (ThermistorADC.thermistor[i]) >= ADC_QUANTIZATION-1)
        {
            return RETROFRIGERATION_FAILED;
        }
        /*//Outdated - uses too much memory
        V1 = (ADC_VALUE*BOARD_V_FS)/(ADC_QUANTIZATION-1);
        R1 = V1 * PCB_DIVIDING_RESISTANCE / (BOARD_V_FS - V1);

        T_K_inverse = STEINHART_HART_A + STEINHART_HART_B*log(R1)+STEINHART_HART_C*pow(log(R1), 3); //Steinhart-Hart equation of thermistor temp-resistance relation. Accurate but uses lots of resources
        TCelsius = 1/T_K_inverse-273.15;
        */
        V1000int = ((ThermistorADC.thermistor[i])*ADC_V_FS)/(ADC_QUANTIZATION-1);
        R1000int = V1000int * PCB_DIVIDING_RESISTANCE / (BOARD_V_FS_1000 - V1000int);
        //TCelsius = BETA_PARAMETER/log(R1000int*RESIST_PARAM_INVERSE)-KELVIN_TO_CELSIUS; //B parameter equation - slightly less accurate than Steinhart-Hart, but less resources used
        TCelsius = 0.000000552853*R1000int*R1000int-0.0100254*R1000int+43.8252; //2nd-degree polynomial interpolation of T-R relation
        DataBuffer->temperature[i] = TCelsius;//TCelsius;

    }
    return RETROFRIGERATION_SUCCEEDED;
}
