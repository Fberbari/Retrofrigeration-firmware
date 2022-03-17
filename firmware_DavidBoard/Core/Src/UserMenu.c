#include "UserMenu.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

typedef enum state
{
    MENU_MAIN,
    MENU_SET_TEMP,
    MENU_SET_LOW_BOUND,
    MENU_SET_UP_BOUND,
    MENU_TEMP_ERROR,
    MENU_HARD_ERROR

}Menu_State_t;

typedef struct
{
    bool left;
    bool right;
    bool up;
    bool down;
}ButtonNewlyPressed_t;


/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

static Menu_State_t CurrentMenuState;
static ButtonNewlyPressed_t ButtonNewlyPressed;
static UserSettings_t UserSettings;
static float fridgeCurrentTemp;


/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

static void DetermineBottomLCDString(const DataBuffer_t *DataBuffer, char *outputStringBottom);

static void ButtonPressedSinceLastCall(const DataBuffer_t *DataBuffer);

static Menu_State_t MenuMain_State(char *outputStringTop);
static Menu_State_t MenuSetTemp_State(char *outputStringTop);
static Menu_State_t MenuSetLowBound_State(char *outputStringTop);
static Menu_State_t MenuSetUpBound_State(char *outputStringTop);
static Menu_State_t MenuTempError_State(char *outputStringTop);

/***********************************************************************************************************************
 * Code
 **********************************************************************************************************************/

void UserMenu_Init(void)
{
    CurrentMenuState = MENU_MAIN;

    ButtonNewlyPressed.left = false;
    ButtonNewlyPressed.right = false;
    ButtonNewlyPressed.up = false;
    ButtonNewlyPressed.down = false;

    fridgeCurrentTemp = 7;

    UserSettings.setTemp = 7;
    UserSettings.tempBoundLow = -30;
    UserSettings.tempBoundHigh = 30;
}


void UserMenu_DetermineLCDString(const DataBuffer_t *DataBuffer, char *outputStringTop, char *outputStringBottom)
{
    // Bottom string looks the same regardless of state
    DetermineBottomLCDString(DataBuffer, outputStringBottom);

    ButtonPressedSinceLastCall(DataBuffer);

    fridgeCurrentTemp = DataBuffer->averageTemperature;

    if ((fridgeCurrentTemp > UserSettings.tempBoundHigh) || (fridgeCurrentTemp < UserSettings.tempBoundLow))
    {
        CurrentMenuState = MENU_TEMP_ERROR;
    }

    // Make sure Buzzer is off upon state entrance
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

    Menu_State_t nextState;
    switch(CurrentMenuState)
    {
        case MENU_MAIN:
            nextState = MenuMain_State(outputStringTop);
            break;

        case MENU_SET_TEMP:
            nextState = MenuSetTemp_State(outputStringTop);
            break;

        case MENU_SET_LOW_BOUND:
            nextState = MenuSetLowBound_State(outputStringTop);
            break;

        case MENU_SET_UP_BOUND:
            nextState = MenuSetUpBound_State(outputStringTop);
            break;

        case MENU_TEMP_ERROR:
            nextState = MenuTempError_State(outputStringTop);
            break;

        default:
            nextState = MENU_HARD_ERROR;
            break;
    }

    CurrentMenuState = nextState;
}


void UserMenu_GetUserSettings(UserSettings_t *OutputUserSettings)
{
    *OutputUserSettings = UserSettings;
}

static void DetermineBottomLCDString(const DataBuffer_t *DataBuffer, char *outputStringBottom)
{
    char fanState[4] = "OFF";
    char compressorState[4] = "OFF";

    if (DataBuffer->fanIsOn)
    {
        strcpy(fanState, "ON");
    }

    if (DataBuffer->compressorIsOn)
    {
        strcpy(compressorState, "ON");
    }

    snprintf(outputStringBottom, 16, "Cmp:%s Fan:%s", compressorState, fanState);
}

static void ButtonPressedSinceLastCall(const DataBuffer_t *DataBuffer)
{
    static int i;
    static bool previousButtonIsClickedState[4];

    // Don't do anything for a bit to let things stabilize.
    if(i < 10)
    {
        i++;
        return;
    }

    ButtonNewlyPressed.up = false;
    ButtonNewlyPressed.down = false;
    ButtonNewlyPressed.left = false;
    ButtonNewlyPressed.right = false;

    if ( (DataBuffer->buttonIsClicked[0] != previousButtonIsClickedState[0]) && (DataBuffer->buttonIsClicked[0]) )
    {
        ButtonNewlyPressed.up = true;
    }
    if ( (DataBuffer->buttonIsClicked[1] != previousButtonIsClickedState[1]) && (DataBuffer->buttonIsClicked[1]) )
    {
        ButtonNewlyPressed.down = true;
    }
    if ( (DataBuffer->buttonIsClicked[2] != previousButtonIsClickedState[2]) && (DataBuffer->buttonIsClicked[2]) )
    {
        ButtonNewlyPressed.left = true;
    }
    if ( (DataBuffer->buttonIsClicked[3] != previousButtonIsClickedState[3]) && (DataBuffer->buttonIsClicked[3]) )
    {
        ButtonNewlyPressed.right = true;
    }

    for (int i = 0; i < NUM_USER_BUTTONS; i++)
    {
        previousButtonIsClickedState[i] = DataBuffer->buttonIsClicked[i];
    }
}

static Menu_State_t MenuMain_State(char *outputStringTop)
{
    snprintf(outputStringTop, 16, "Temp now: %d.%dC", (int) fridgeCurrentTemp, ((int)(fridgeCurrentTemp*10))%10);

    if(ButtonNewlyPressed.right)
    {
        return MENU_SET_TEMP;
    }
    else if(ButtonNewlyPressed.left)
    {
        return MENU_SET_UP_BOUND;
    }

    return MENU_MAIN;
}

static Menu_State_t MenuSetTemp_State(char *outputStringTop)
{
    snprintf(outputStringTop, 16, "Set Temp: %dC", UserSettings.setTemp);


    if (ButtonNewlyPressed.right)
    {
        return MENU_SET_LOW_BOUND;
    }

    else if (ButtonNewlyPressed.left)
    {
        return MENU_MAIN;
    }

    if (ButtonNewlyPressed.up)
    {
        UserSettings.setTemp ++;
    }

    else if (ButtonNewlyPressed.down)
    {
        UserSettings.setTemp --;
    }

    if (UserSettings.tempBoundLow >= UserSettings.setTemp)
    {
        UserSettings.tempBoundLow = UserSettings.setTemp - 1;
    }
    else if (UserSettings.tempBoundHigh <= UserSettings.setTemp)
    {
        UserSettings.tempBoundHigh = UserSettings.setTemp + 1;
    }

    snprintf(outputStringTop, 16, "Set Temp: %dC", UserSettings.setTemp);

    return MENU_SET_TEMP;
}

static Menu_State_t MenuSetLowBound_State(char *outputStringTop)
{
    snprintf(outputStringTop, 16, "Low Bound: %dC", UserSettings.tempBoundLow);

    if(ButtonNewlyPressed.right)
    {
        return MENU_SET_UP_BOUND;
    }
    else if(ButtonNewlyPressed.left)
    {
        return MENU_SET_TEMP;
    }

    if (ButtonNewlyPressed.up)
    {
        UserSettings.tempBoundLow ++;
    }

    else if (ButtonNewlyPressed.down)
    {
        UserSettings.tempBoundLow --;
    }

    if (UserSettings.tempBoundLow >= UserSettings.setTemp )
    {
        UserSettings.tempBoundLow = UserSettings.setTemp - 1;
    }

    snprintf(outputStringTop, 16, "Low Bound: %dC", UserSettings.tempBoundLow);

    return MENU_SET_LOW_BOUND;
}

static Menu_State_t MenuSetUpBound_State(char *outputStringTop)
{
    snprintf(outputStringTop, 16, "High Bound: %dC", UserSettings.tempBoundHigh);

    if(ButtonNewlyPressed.right)
    {
        return MENU_MAIN;
    }
    else if(ButtonNewlyPressed.left)
    {
        return MENU_SET_LOW_BOUND;
    }

    if (ButtonNewlyPressed.up)
    {
        UserSettings.tempBoundHigh ++;
    }

    else if (ButtonNewlyPressed.down)
    {
        UserSettings.tempBoundHigh --;
    }

    else if (UserSettings.tempBoundHigh <= UserSettings.setTemp)
    {
        UserSettings.tempBoundHigh = UserSettings.setTemp + 1;
    }

    snprintf(outputStringTop, 16, "High Bound: %dC", UserSettings.tempBoundHigh);

    return MENU_SET_UP_BOUND;
}

static Menu_State_t MenuTempError_State(char *outputStringTop)
{

    static int counter;

    if(counter < CTRL_LOOP_FREQUENCY/4)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    }

    if (counter == CTRL_LOOP_FREQUENCY)
    {
        counter = 0;
    }

    counter ++;

    if (fridgeCurrentTemp > UserSettings.tempBoundHigh)
    {
        snprintf(outputStringTop, 16, "Temp too high!");
    }
    else if (fridgeCurrentTemp < UserSettings.tempBoundLow)
    {
        snprintf(outputStringTop, 16, "Temp too low!");
    }

    return MENU_MAIN;
}
