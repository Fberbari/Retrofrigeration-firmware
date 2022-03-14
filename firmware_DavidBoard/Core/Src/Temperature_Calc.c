/*
 * TempCalc.c
 *
 *  Created on: Mar 13, 2022
 *      Author: JacobS
 */

#include "Temperature_Calc.h"

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


static double V_1;
static double R_1;
//static double T_K_inverse;
static double T_Celsius;


/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

double Temperature_ADCtoCelsius(uint16_t ADC_VALUE);


/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

double Temperature_ADCtoCelsius(uint16_t ADC_VALUE)
{
    if(ADC_VALUE <= 0 || ADC_VALUE >= ADC_QUANTIZATION-1)
    {
        return -KELVIN_TO_CELSIUS; //should there be an ERROR code?
    }
    V_1 = (ADC_VALUE*BOARD_V_FS)/(ADC_QUANTIZATION-1);
    R_1 = V_1 * PCB_DIVIDING_RESISTANCE / (BOARD_V_FS - V_1);
    /*
    T_K_inverse = STEINHART_HART_A + STEINHART_HART_B*log(R_1)+STEINHART_HART_C*pow(log(R_1), 3); //Steinhart-Hart equation of thermistor temp-resistance relation. Accurate but uses lots of resources
    T_Celsius = 1/T_K_inverse-273.15;
    */
    T_Celsius = BETA_PARAMETER/log(R_1/RESIST_PARAMETER)-KELVIN_TO_CELSIUS; //B parameter equation - slightly less accurate than Steinhart-Hart, but less resources used
    return T_Celsius;
}
