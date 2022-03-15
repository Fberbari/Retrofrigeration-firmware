#include "Controller.h"
#include "Actuators.h"
#include "I2CManager.h"
#include "UserMenu.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

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

// state input/output data
static ActuatorCommands_t ActuatorCommands;

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

    Actuators_Init();

    StartPeriodTimer();

    I2CManager_Init();

    UserMenu_Init();

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
    I2CManager_GetPushButtonStates(&PushButtonStates);

    return CTRL_LOG_DATA;
}

static Controller_State_t LogData_State(void)
{
    char LCDString[16];

    UserMenu_DetermineLCDString(&PushButtonStates, 7, LCDString);
    I2CManager_SendToLCD(LCDString);

    I2CManager_LaunchExchange();

    return CTRL_DO_MATH;
}

static Controller_State_t DoMath_State(void)
{
    static int loopCounter;
    static int comp_off_counter;
    //get 5 temp readings
    float temps[5] = {DataBuffer.temperature[0], DataBuffer.temperature[1], DataBuffer.temperature[2], DataBuffer.temperature[3], DataBuffer.temperature[4]}; //5 temp probe readings
    float temp_diffs[5]; //to store deltas between probe readings and avg temp
    float avgTemp = 0; //mean temperature
    float fan_treshold = 1; //desired fan treshold (set to 1C for now, can be changed)

    for (int i=0; i<5; i++)
        avgTemp += 0.2*temps[i]; //calculate mean temp

    //fan control
    if(loopCounter % 100 ==0) //every 2 seconds
        for (int i=0; i<5; i++)
        {
            temp_diffs[i] = temps[i] - avgTemp; //calculate deviation from mean temp.
            if (temp_diffs[i] >= fan_treshold || temp_diffs[i] <= -1*fan_treshold)
            {
                ActuatorCommands.fan = FAN_ON; //if any probe exceeds treshold, turn on internal fan
                break;
            }
            else ActuatorCommands.fan = FAN_OFF;
        }

    //compressor control
    if (avgTemp > 3 && comp_off_counter >= 50*75) //on at 3C if off for at least 75s
    {
        loopCounter =0; //reset loop counter every so often to avoid overflow
        ActuatorCommands.compressor = COMPRESSOR_ON;
    }
  
    else if (avgTemp < 1) //off at 1C
    {
        ActuatorCommands.compressor = COMPRESSOR_OFF;
        comp_off_counter =0; //reset compressor off counter since compressor is switched off
    }

    //increment both counters
    loopCounter++;
    comp_off_counter++;
  
    return CTRL_ACTUATE_FRIDGE;
}

static Controller_State_t ActuateFridge_State(void)
{
    int r = Actuators_ActuateSystem(&ActuatorCommands);

    if (r != RETROFRIGERATION_SUCCEEDED)
    {
        return CTRL_FAILED;
    }

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
