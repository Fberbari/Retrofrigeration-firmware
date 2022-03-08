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

static Controller_State_t currentState;

static bool periodHasPassed;

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

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

static Controller_State_t DoMath_State(void)
{
    return CTRL_ACTUATE_FRIDGE;
}

static Controller_State_t ActuateFridge_State(void)
{
    return CTRL_WAIT_FOR_TIMER;
}

static Controller_State_t WaitForTimer_State(void)
{
    return CTRL_WAIT_FOR_TIMER;
}

static Controller_State_t Failed_State(void)
{
    return CTRL_FAILED;
}
