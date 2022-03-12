
#include "I2CManager.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

#define ADC_SLAVE_ADDRESS 0x33
#define ADC_SLAVE_ADDRESS_W (ADC_SLAVE_ADDRESS << 1)
#define ADC_SLAVE_ADDRESS_R (ADC_SLAVE_ADDRESS << 1) | 0x01

// [7:7] Setup byte select, [6:4] Use internal reference, [3:3] external clock,
// [2:2] uni-polar mode, [1:1] don't reset config register, [0:0] don't care.
#define ADC_SETUP_BYTE 0xDA // 11011010

// [7:7] Configuration byte select, [6:5] scan select, [4:1] channel select, [0:0] single ended mode
#define ADC_CONFIG_BYTE 0x0F //00001111

// Only used for the initialization polling I2C functions
#define DEFAULT_TIMEOUT 1000 // ms

typedef enum I2CbusStatus
{
    READY,
    BUSY_IOEXPAND_1,
    BUSY_IOEXPAND_2,
    BUSY_ADC,
    FAILED
}I2CBusStatus_t;

/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

// HAL handle
extern I2C_HandleTypeDef hi2c2;


static uint8_t rawAdcData[16];


static I2CBusStatus_t currentBusStatus;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

static void InitAdc(void);
static void LaunchADCExchange(void);

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void I2CManager_Init(void)
{
    currentBusStatus = READY;

    InitAdc();
}

int I2CManager_GetRawThermistorADC(ThermistorADC_t *ThermistorADC)
{

}

int I2CManager_GetPushButtonStates(PushButtonStates_t *PushButtonStates)
{

}

int I2CManager_SendActuatorCommands(const ActuatorCommands_t *ActuatorCommands)
{

}

int I2CManager_LaunchExchange(void)
{

}

static void InitAdc(void)
{

    uint8_t data[2] = {ADC_SETUP_BYTE, ADC_CONFIG_BYTE};

    currentBusStatus = BUSY_ADC;

    HAL_I2C_Master_Transmit(&hi2c2, ADC_SLAVE_ADDRESS_W, data, sizeof(data), DEFAULT_TIMEOUT);

    currentBusStatus = READY;
}

static void LaunchADCExchange(void)
{
    currentBusStatus = BUSY_ADC;
    HAL_I2C_Master_Receive_IT(&hi2c2, ADC_SLAVE_ADDRESS_R, rawAdcData, sizeof(rawAdcData));
}

/****************************Interrupt Handlers****************************/

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    switch(currentBusStatus)
    {
        case BUSY_IOEXPAND_1:
            break;

        case BUSY_IOEXPAND_2:
            break;

        case BUSY_ADC:
            break;

        default:
            currentBusStatus = FAILED;
            break;
    }
}
