#include "Controller.h"
#include "I2CManager.h"
#include "UserMenu.h"
#include "TemperatureCalc.h"
#include "Logs.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

#define MAX_ALLOWABLE_TEMP_DIFF 1.0f // start fan if the temperature at any probe is this far from the average
#define REASONABLE_TEMP_DIFF 0.5f    // Do not stop fan until no probe is this far from the average temperature

typedef enum state
{
    CTRL_COLLECT_DATA,
    CTRL_LOG_DATA,
    CTRL_DO_MATH,
    CTRL_ACTUATE_FRIDGE,
    CTRL_WAIT_FOR_TIMER,
    CTRL_FAILED

}Controller_State_t;

/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

// HAL handles. These should all be extern, as they are defined and initialized by cubeMX (the code generation tool) in main.c
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

// state input/output data
static ActuatorCommands_t ActuatorCommands;
static UserSettings_t UserSettings;

static Controller_State_t currentState;
static bool periodHasPassed;
static DataBuffer_t DataBuffer;
static PushButtonStates_t PushButtonStates;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

static void StartPeriodTimer(void);

static Controller_State_t CollectData_State(void);
static Controller_State_t LogData_State(void);
static Controller_State_t DoMath_State(void);
static Controller_State_t ActuateFridge_State(void);
static Controller_State_t WaitForTimer_State(void);
static Controller_State_t Failed_State(void);

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void Controller_Init(void)
{
    // let things settle
    HAL_Delay(1000);

    StartPeriodTimer();

    I2CManager_Init();

    UserMenu_Init();

    Logs_Init(&huart2);

    // let things settle
    HAL_Delay(1000);

    currentState = CTRL_COLLECT_DATA;
}

void Controller_SaveTheAfricans(void)
{
    Controller_State_t nextState;
    switch(currentState)
    {
        case CTRL_COLLECT_DATA:
            nextState = CollectData_State();
            break;

        case CTRL_LOG_DATA:
            nextState = LogData_State();
            break;

        case CTRL_DO_MATH:
            nextState = DoMath_State();
            break;

        case CTRL_ACTUATE_FRIDGE:
            nextState = ActuateFridge_State();
            break;

        case CTRL_WAIT_FOR_TIMER:
            nextState = WaitForTimer_State();
            break;

        case CTRL_FAILED:
            nextState = Failed_State();
            break;

        default:
            nextState = CTRL_FAILED;
            break;
    }

    currentState = nextState;
}

static Controller_State_t CollectData_State(void)
{
    UserMenu_GetUserSettings(&UserSettings);
    I2CManager_GetPushButtonStates(&PushButtonStates);
    Temperature_ADCtoCelsius(&DataBuffer);

    for (int i = 0; i < NUM_USER_BUTTONS; i++)
    {
        // logical not because "closed" is represented as a 0 in DataBuffer and a 1 in PushbuttonStates
        DataBuffer.buttonIsClicked[i] = ! PushButtonStates.button[i];
    }

    return CTRL_LOG_DATA;
}

static Controller_State_t LogData_State(void)
{
    char LCDStringTop[17];
    char LCDStringBottom[17] = "Heloo";

    static uint32_t numLoopsServerInit;
    numLoopsServerInit++;

    UserMenu_DetermineLCDString(&DataBuffer, LCDStringTop, LCDStringBottom);

    I2CManager_SendToLCD(LCDStringTop, LCDStringBottom);

    I2CManager_LaunchExchange();



    if (numLoopsServerInit == 0)
    	Logs_LogWifi(&DataBuffer, &huart2, 0);

    if (numLoopsServerInit >= 5*CTRL_LOOP_FREQUENCY)
		Logs_LogWifi(&DataBuffer, &huart2, 1);

    if (numLoopsServerInit >= 10*CTRL_LOOP_FREQUENCY)
    	numLoopsServerInit = 0;

    return CTRL_DO_MATH;
}

static Controller_State_t DoMath_State(void)
{
    static uint32_t numLoopsSinceCompressorOff;
    numLoopsSinceCompressorOff ++;

    DataBuffer.averageTemperature = 0;
    for (int i = 0; i < NUM_TEMP_PROBES; i++)
    {
        DataBuffer.averageTemperature += DataBuffer.temperature[i];
    }
    DataBuffer.averageTemperature /= NUM_TEMP_PROBES;

   /****************************Fan Control****************************/

    bool allProbesReasonableDiff = true;

    for (int i = 0; i < NUM_TEMP_PROBES; i++)
    {
        int tempDiff = DataBuffer.temperature[i] - DataBuffer.averageTemperature;

        if (ABS(tempDiff) > MAX_ALLOWABLE_TEMP_DIFF)
        {
            allProbesReasonableDiff = false;
            ActuatorCommands.fan = FAN_ON; // turn ON if any exceed the max allowable threshold
            break;
        }
        else if (ABS(tempDiff) > REASONABLE_TEMP_DIFF)
        {
            allProbesReasonableDiff = false;
        }
    }

    if (allProbesReasonableDiff)   // only turn off fan once all probes are within the reasonable threshold
    {
        ActuatorCommands.fan = FAN_OFF;
    }

    /****************************Compressor Control****************************/

    if (DataBuffer.averageTemperature > UserSettings.setTemp)
    {
        // Compressor must be off for at least 75s before being turned on again
        if(numLoopsSinceCompressorOff >= 75*CTRL_LOOP_FREQUENCY)
        {
            ActuatorCommands.compressor = COMPRESSOR_ON;
        }
    }
  
    else if (DataBuffer.averageTemperature < UserSettings.setTemp)
    {
        ActuatorCommands.compressor = COMPRESSOR_OFF;
        numLoopsSinceCompressorOff = 0;
    }

    DataBuffer.fanIsOn = ActuatorCommands.fan;
    DataBuffer.compressorIsOn = ActuatorCommands.compressor;
  
    return CTRL_ACTUATE_FRIDGE;
}

static Controller_State_t ActuateFridge_State(void)
{
    I2CManager_SendActuatorCommands(&ActuatorCommands);

    return CTRL_WAIT_FOR_TIMER;
}

static Controller_State_t WaitForTimer_State(void)
{
    if (periodHasPassed)
    {
        periodHasPassed = false;

        return CTRL_COLLECT_DATA;
    }

    return CTRL_WAIT_FOR_TIMER;
}

static Controller_State_t Failed_State(void)
{
    return CTRL_FAILED;
}

static void StartPeriodTimer(void)
{
    HAL_TIM_Base_Start_IT(&htim3);
}


/****************************Interrupt Handlers****************************/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    periodHasPassed = true;
}
