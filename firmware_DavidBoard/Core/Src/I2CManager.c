
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

#define IOEXPAND1_SLAVE_ADDRESS 0x38
#define IOEXPAND1_SLAVE_ADDRESS_W (IOEXPAND1_SLAVE_ADDRESS << 1)
#define IOEXPAND1_SLAVE_ADDRESS_R (IOEXPAND1_SLAVE_ADDRESS << 1) | 0x01

// Only used for the initialization polling I2C functions
#define DEFAULT_TIMEOUT 1000 // ms

typedef enum I2CbusStatus
{
    READY,
    BUSY_IOEXPAND_1_READING,
    BUSY_IOEXPAND_1_WRITING,
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
static bool adcDataNew;

static uint8_t rawIOExpand1ReadData;
static bool IOExpand1ReadDataNew;

static uint8_t IOExpand1WriteData;
static bool IOExpand1DataWritten;

static I2CBusStatus_t currentBusStatus;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

static void InitAdc(void);
static void LaunchADCExchange(void);

static void InitIOExpand1(void);
static void LaunchIOExpand1ReadExchange(void);
static void LaunchIOExpand1WriteExchange(void);

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void I2CManager_Init(void)
{
    currentBusStatus = READY;

    adcDataNew = false;
    IOExpand1ReadDataNew = false;
    IOExpand1DataWritten = true;

    InitAdc();
    InitIOExpand1();
}

int I2CManager_GetRawThermistorADC(ThermistorADC_t *ThermistorADC)
{
    if(currentBusStatus == FAILED)
    {
        return RETROFRIGERATION_FAILED;
    }

    if (! adcDataNew)
    {
        return RETROFRIGERATION_BUSY;
    }


    ThermistorADC->thermistor[0] = rawAdcData[8] + ((rawAdcData[9] & 0x3) << 8);
    ThermistorADC->thermistor[1] = rawAdcData[10] + ((rawAdcData[11] & 0x3) << 8);
    ThermistorADC->thermistor[2] = rawAdcData[12] + ((rawAdcData[13] & 0x3) << 8);
    ThermistorADC->thermistor[3] = rawAdcData[14] + ((rawAdcData[15] & 0x3) << 8);

    adcDataNew = false;

    return RETROFRIGERATION_SUCCEEDED;

}

int I2CManager_GetPushButtonStates(PushButtonStates_t *PushButtonStates)
{
    if(currentBusStatus == FAILED)
    {
        return RETROFRIGERATION_FAILED;
    }

    if (! IOExpand1ReadDataNew)
    {
        return RETROFRIGERATION_BUSY;
    }

    PushButtonStates->button[0] = (rawIOExpand1ReadData & 0x10) >> 4;
    PushButtonStates->button[1] = (rawIOExpand1ReadData & 0x20) >> 5;
    PushButtonStates->button[2] = (rawIOExpand1ReadData & 0x40) >> 6;
    PushButtonStates->button[3] = (rawIOExpand1ReadData & 0x80) >> 7;

    IOExpand1ReadDataNew = false;

    return RETROFRIGERATION_SUCCEEDED;
}

int I2CManager_SendActuatorCommands(const ActuatorCommands_t *ActuatorCommands)
{
    if(currentBusStatus == FAILED)
    {
        return RETROFRIGERATION_FAILED;
    }

    if (ActuatorCommands->compressor == COMPRESSOR_ON)
    {
        IOExpand1WriteData |= 0x01;
    }
    else
    {
        IOExpand1WriteData &= ~0x01;
    }

    if (ActuatorCommands->fan == FAN_ON)
    {
        IOExpand1WriteData |= 0x02;
    }
    else
    {
        IOExpand1WriteData &= ~0x02;
    }

    if (! IOExpand1DataWritten)
    {
        return RETROFRIGERATION_BUSY;
    }

    IOExpand1DataWritten = false;

    return RETROFRIGERATION_SUCCEEDED;
}

int I2CManager_LaunchExchange(void)
{
	currentBusStatus = BUSY_IOEXPAND_1_READING;
    LaunchIOExpand1ReadExchange();
}

static void InitAdc(void)
{
    uint8_t data[2] = {ADC_SETUP_BYTE, ADC_CONFIG_BYTE};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c2, ADC_SLAVE_ADDRESS_W, data, sizeof(data), DEFAULT_TIMEOUT);
}

static void InitIOExpand1(void)
{
    // sets all inputs to weak high. Outputs set to 0 to start.
    uint8_t data = 0xf8;
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c2, IOEXPAND1_SLAVE_ADDRESS_W, &data, sizeof(data), DEFAULT_TIMEOUT);

}

static void LaunchADCExchange(void)
{
    if(currentBusStatus != READY)
    {
        return;
    }

    HAL_I2C_Master_Receive_IT(&hi2c2, ADC_SLAVE_ADDRESS_R, rawAdcData, sizeof(rawAdcData));
}

static void LaunchIOExpand1ReadExchange(void)
{
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive_IT(&hi2c2, IOEXPAND1_SLAVE_ADDRESS_R, &rawIOExpand1ReadData, sizeof(rawIOExpand1ReadData));
}

static void LaunchIOExpand1WriteExchange(void)
{
	IOExpand1WriteData |= (0xf8); // top 5 pins are inputs and so should always be written high.
    HAL_I2C_Master_Transmit_IT(&hi2c2, IOEXPAND1_SLAVE_ADDRESS_W, &IOExpand1WriteData, sizeof(IOExpand1WriteData));
}

/****************************Interrupt Handlers****************************/

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    switch(currentBusStatus)
    {
        case BUSY_IOEXPAND_1_READING:
            IOExpand1ReadDataNew = true;
            currentBusStatus = BUSY_IOEXPAND_1_WRITING;
            LaunchIOExpand1WriteExchange();
            break;

        case BUSY_IOEXPAND_1_WRITING:
            LaunchADCExchange();
            currentBusStatus = BUSY_ADC;
            break;

        case BUSY_ADC:
            // launch IOExpand2
            adcDataNew = true;
            currentBusStatus = BUSY_IOEXPAND_2;
            break;

        case BUSY_IOEXPAND_2:
            currentBusStatus = READY;
            break;

        default:
            currentBusStatus = FAILED;
            break;
    }
}
