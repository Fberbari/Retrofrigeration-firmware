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

/***********************************************************************************************************************
 * Prototypes
 **********************************************************************************************************************/

static void ButtonPressedSinceLastCall(const PushButtonStates_t *PushButtonStates);

static Menu_State_t MenuMain_State(char *outputString);
static Menu_State_t MenuSetTemp_State(char *outputString);
static Menu_State_t MenuSetLowBound_State(char *outputString);
static Menu_State_t MenuSetUpBound_State(char *outputString);
static Menu_State_t MenuTempError_State(char *outputString);

static int fridgeCurrentTemp;

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


void UserMenu_DetermineLCDString(const PushButtonStates_t *PushButtonStates, int currentTemp, char *outputString)
{
    ButtonPressedSinceLastCall(PushButtonStates);

    fridgeCurrentTemp = currentTemp;

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
            nextState = MenuMain_State(outputString);
            break;

        case MENU_SET_TEMP:
            nextState = MenuSetTemp_State(outputString);
            break;

        case MENU_SET_LOW_BOUND:
            nextState = MenuSetLowBound_State(outputString);
            break;

        case MENU_SET_UP_BOUND:
            nextState = MenuSetUpBound_State(outputString);
            break;

        case MENU_TEMP_ERROR:
            nextState = MenuTempError_State(outputString);
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

static void ButtonPressedSinceLastCall(const PushButtonStates_t *PushButtonStates)
{
    static int i;
    static PushButtonStates_t PreviousPushButtonStates;

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

    if ( (PushButtonStates->button[0] != PreviousPushButtonStates.button[0]) && (PushButtonStates->button[0] == BUTTON_PRESSED) )
    {
        ButtonNewlyPressed.up = true;
    }
    if ( (PushButtonStates->button[1] != PreviousPushButtonStates.button[1]) && (PushButtonStates->button[1] == BUTTON_PRESSED) )
    {
        ButtonNewlyPressed.down = true;
    }
    if ( (PushButtonStates->button[2] != PreviousPushButtonStates.button[2]) && (PushButtonStates->button[2] == BUTTON_PRESSED) )
    {
        ButtonNewlyPressed.left = true;
    }
    if ( (PushButtonStates->button[3] != PreviousPushButtonStates.button[3]) && (PushButtonStates->button[3] == BUTTON_PRESSED) )
    {
        ButtonNewlyPressed.right = true;
    }

    PreviousPushButtonStates = *PushButtonStates;
}

static Menu_State_t MenuMain_State(char *outputString)
{
    snprintf(outputString, 16, "Temperature: %dC", fridgeCurrentTemp);

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

static Menu_State_t MenuSetTemp_State(char *outputString)
{
    snprintf(outputString, 16, "Set Temp: %dC", UserSettings.setTemp);


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

    snprintf(outputString, 16, "Set Temp: %dC", UserSettings.setTemp);

    return MENU_SET_TEMP;
}

static Menu_State_t MenuSetLowBound_State(char *outputString)
{
    snprintf(outputString, 16, "Low Bound: %dC", UserSettings.tempBoundLow);

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

    snprintf(outputString, 16, "Low Bound: %dC", UserSettings.tempBoundLow);

    return MENU_SET_LOW_BOUND;
}

static Menu_State_t MenuSetUpBound_State(char *outputString)
{
    snprintf(outputString, 16, "High Bound: %dC", UserSettings.tempBoundHigh);

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

    snprintf(outputString, 16, "High Bound: %dC", UserSettings.tempBoundHigh);

    return MENU_SET_UP_BOUND;
}

static Menu_State_t MenuTempError_State(char *outputString)
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
        snprintf(outputString, 16, "Temp too high!");
    }
    else if (fridgeCurrentTemp < UserSettings.tempBoundLow)
    {
        snprintf(outputString, 16, "Temp too low!");
    }

    return MENU_MAIN;
}
