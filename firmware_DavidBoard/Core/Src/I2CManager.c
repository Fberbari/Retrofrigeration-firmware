
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

// this connects directly to the LCD
#define IOEXPAND2_SLAVE_ADDRESS 0x3F
#define IOEXPAND2_SLAVE_ADDRESS_W (IOEXPAND2_SLAVE_ADDRESS << 1)

#define LCD_CLEAR_CMD 0x01

#define LCD_EXCHANGE_IN_PROGRESS 0
#define LCD_EXCHANGE_COMPLETE 1

// Only used for the initialization polling I2C functions
#define DEFAULT_TIMEOUT 1000 // ms

typedef enum I2CbusStatus
{
    READY,
    BUSY_ADC,
    BUSY_IOEXPAND_1_READING,
    BUSY_IOEXPAND_1_WRITING,
    BUSY_LCD,
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

static char strToWrite[20];
static bool strHasBeenWritten;

static I2CBusStatus_t currentBusStatus;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

static void InitAdc(void);
static void LaunchADCExchange(void);

static void InitIOExpand1(void);
static void LaunchIOExpand1ReadExchange(void);
static void LaunchIOExpand1WriteExchange(void);

static void InitLCD(void);
static int ExecuteLCDExchange(void);
static void LCDSendCommandPolling(uint8_t cmd);
static void LCDSendCommandIT(uint8_t cmd);
static void LCDSendDataIT(char data);
static void LCDSendStringPolling (char *str);
static void LCDSendDataPolling (char data);

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void I2CManager_Init(void)
{
    currentBusStatus = READY;

    adcDataNew = false;
    IOExpand1ReadDataNew = false;
    IOExpand1DataWritten = true;
    strHasBeenWritten = true;
    strToWrite[0] = ' ';

    InitAdc();
    InitIOExpand1();
    InitLCD();
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

int I2CManager_SendToLCD(const char *str)
{
    if(currentBusStatus == FAILED)
    {
        return RETROFRIGERATION_FAILED;
    }
    else if(currentBusStatus != READY)
    {
        return RETROFRIGERATION_BUSY;
    }

    if(strlen(str) > 36)
    {
        return RETROFRIGERATION_FAILED;
    }

    if (! strHasBeenWritten)
    {
        return RETROFRIGERATION_BUSY;
    }

    strcpy(&strToWrite[1], str);

    strHasBeenWritten = false;

    return RETROFRIGERATION_SUCCEEDED;
}

int I2CManager_LaunchExchange(void)
{
    if (currentBusStatus == FAILED)
    {
        return RETROFRIGERATION_FAILED;
    }

    else if (currentBusStatus != READY)
    {
        return RETROFRIGERATION_BUSY;
    }

	currentBusStatus = BUSY_IOEXPAND_1_READING;

	LaunchIOExpand1ReadExchange();

    return RETROFRIGERATION_SUCCEEDED;
}

static void InitAdc(void)
{
    uint8_t data[2] = {ADC_SETUP_BYTE, ADC_CONFIG_BYTE};
    HAL_I2C_Master_Transmit(&hi2c2, ADC_SLAVE_ADDRESS_W, data, sizeof(data), DEFAULT_TIMEOUT);
}

static void InitIOExpand1(void)
{
    // sets all inputs to weak high. Outputs set to 0 to start.
    uint8_t data = 0xf8;
    HAL_I2C_Master_Transmit(&hi2c2, IOEXPAND1_SLAVE_ADDRESS_W, &data, sizeof(data), DEFAULT_TIMEOUT);

}

static void InitLCD(void)
{
    // 4 bit initialisation
    HAL_Delay(50);  // wait for >40ms
    LCDSendCommandPolling (0x30);
    HAL_Delay(5);  // wait for >4.1ms
    LCDSendCommandPolling (0x30);
    HAL_Delay(1);  // wait for >100us
    LCDSendCommandPolling (0x30);
    HAL_Delay(10);
    LCDSendCommandPolling (0x20);  // 4bit mode
    HAL_Delay(10);

  // dislay initialisation
    LCDSendCommandPolling (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
    HAL_Delay(10);
    LCDSendCommandPolling (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
    HAL_Delay(10);
    LCDSendCommandPolling (0x01);  // clear display
    HAL_Delay(10);
    HAL_Delay(10);
    LCDSendCommandPolling (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
    HAL_Delay(10);
    LCDSendCommandPolling (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
    HAL_Delay(100);
    LCDSendStringPolling("RETROFRIGERATION");
    HAL_Delay(2000);
}

static void LaunchADCExchange(void)
{
    HAL_I2C_Master_Receive_IT(&hi2c2, ADC_SLAVE_ADDRESS_R, rawAdcData, sizeof(rawAdcData));
}

static void LaunchIOExpand1ReadExchange(void)
{
	HAL_I2C_Master_Receive_IT(&hi2c2, IOEXPAND1_SLAVE_ADDRESS_R, &rawIOExpand1ReadData, sizeof(rawIOExpand1ReadData));
}

static void LaunchIOExpand1WriteExchange(void)
{
	IOExpand1WriteData |= (0xf8); // top 5 pins are inputs and so should always be written high.
    HAL_I2C_Master_Transmit_IT(&hi2c2, IOEXPAND1_SLAVE_ADDRESS_W, &IOExpand1WriteData, sizeof(IOExpand1WriteData));
}

static int ExecuteLCDExchange(void)
{
    static int currentIndex = -1;

    if (currentIndex < 0)
    {
        currentIndex ++;
        LCDSendCommandIT(LCD_CLEAR_CMD);
    }
    else if (strToWrite[currentIndex] == '\0')
    {
        currentIndex = -1;
        return LCD_EXCHANGE_COMPLETE;
    }
    else
    {
        currentIndex ++;
        LCDSendDataIT(strToWrite[currentIndex - 1]);
    }

    return LCD_EXCHANGE_IN_PROGRESS;

}

static void LCDSendStringPolling (char *str)
{
    while (*str) LCDSendDataPolling (*str++);
}

static void LCDSendDataPolling (char data)
{
    HAL_Delay(30);
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (data&0xf0);
    data_l = ((data<<4)&0xf0);
    data_t[0] = data_u|0x0D;  //en=1, rs=0
    data_t[1] = data_u|0x09;  //en=0, rs=0
    data_t[2] = data_l|0x0D;  //en=1, rs=0
    data_t[3] = data_l|0x09;  //en=0, rs=0
    HAL_I2C_Master_Transmit(&hi2c2, IOEXPAND2_SLAVE_ADDRESS_W, data_t, sizeof(data_t), DEFAULT_TIMEOUT);
}

static void LCDSendCommandPolling(uint8_t cmd)
{
    HAL_Delay(10);
    uint8_t data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd&0xf0);
    data_l = ((cmd<<4)&0xf0);
    data_t[0] = data_u|0x0C;  //en=1, rs=0
    data_t[1] = data_u|0x08;  //en=0, rs=0
    data_t[2] = data_l|0x0C;  //en=1, rs=0
    data_t[3] = data_l|0x08;  //en=0, rs=0

    HAL_I2C_Master_Transmit(&hi2c2, IOEXPAND2_SLAVE_ADDRESS_W, data_t, sizeof(data_t), DEFAULT_TIMEOUT);
}

static void LCDSendCommandIT(uint8_t cmd)
{
  uint8_t data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd&0xf0);
    data_l = ((cmd<<4)&0xf0);
    data_t[0] = data_u|0x0C;  //en=1, rs=0
    data_t[1] = data_u|0x08;  //en=0, rs=0
    data_t[2] = data_l|0x0C;  //en=1, rs=0
    data_t[3] = data_l|0x08;  //en=0, rs=0

    HAL_I2C_Master_Transmit_IT(&hi2c2, IOEXPAND2_SLAVE_ADDRESS_W, data_t, sizeof(data_t));
}

static void LCDSendDataIT(char data)
{
    uint8_t data_u, data_l;
    uint8_t data_t[4];
    data_u = (data&0xf0);
    data_l = ((data<<4)&0xf0);
    data_t[0] = data_u|0x0D;  //en=1, rs=0
    data_t[1] = data_u|0x09;  //en=0, rs=0
    data_t[2] = data_l|0x0D;  //en=1, rs=0
    data_t[3] = data_l|0x09;  //en=0, rs=0

    HAL_I2C_Master_Transmit_IT(&hi2c2, IOEXPAND2_SLAVE_ADDRESS_W,data_t, sizeof(data_t));
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

        case BUSY_ADC:
            ExecuteLCDExchange();
            adcDataNew = true;
            currentBusStatus = BUSY_LCD;
            break;

        default:
            currentBusStatus = FAILED;
            break;
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    switch(currentBusStatus)
    {

        case BUSY_IOEXPAND_1_WRITING:
            IOExpand1DataWritten = true;
            currentBusStatus = BUSY_ADC;
            LaunchADCExchange();
            break;

        case BUSY_LCD:
        {
            int status = ExecuteLCDExchange();

            if(status == LCD_EXCHANGE_IN_PROGRESS)
            {
                currentBusStatus = BUSY_LCD;
            }
            else
            {
                strHasBeenWritten = true;
                currentBusStatus = READY;
            }
            break;
        }


        default:
            currentBusStatus = FAILED;
            break;
    }
}
