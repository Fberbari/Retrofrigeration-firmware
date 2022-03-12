#include "Controller.h"
#include "Actuators.h"

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
extern TIM_HandleTypeDef htim2;

// state input/output data
static ActuatorCommands_t ActuatorCommands;

static Controller_State_t currentState;
static bool periodHasPassed;

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
    Actuators_Init();

    StartPeriodTimer();

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

    return CTRL_LOG_DATA;
}

static Controller_State_t LogData_State(void)
{
    return CTRL_DO_MATH;
}

float savedTemps[];
float loopCounter;


static Controller_State_t DoMath_State(void)
{
    float temps[5] = {t1, t2, t3, t4, t5}; //5 temp probe readings
    float temp_diffs[5]; //to store deltas between probe readings and avg temp
    float avgTemp = 0; //mean temperature
    float fan_treshold = 1; //desired fan treshold
    int actuate[2] = {0, 0}; //first position is fan, second is compressor

    for (int i=0, i<5; i++)
        avgTemp += temps[i] / 5; //calculate mean temp

    //fan control
    if(loopCounter % 200 ==0) //every 2 seconds
        for (int i=0, i<5; i++)
        {
            temp_diffs[i] = temps[i] - avgTemp; //calculate deviation from mean temp.
            if (temp_diffs[i] >= fan_treshold || temp_diffs[i] <= -1*fan_treshold)
                actuate[0] = 1; //if any probe exceeds treshold, turn on internal fan
        }

    //compressor control
    if(loopCounter % (50*120) ==0) //every 2 min
    {
        if (avgTemp > 3) //on at 3C
            actuate[1] = 1;
        else if (avgTemp < 1)
            actuate[1] = 0; //off at 1C
        loopCounter =0;
    }


    loopCounter++;
    return actuate;
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

        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

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
    HAL_TIM_Base_Start_IT(&htim2);
}


/****************************Interrupt Handlers****************************/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    periodHasPassed = true;
}
