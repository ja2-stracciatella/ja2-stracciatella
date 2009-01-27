#include "Types.h"
#include "Button_Sound_Control.h"
#include "Sound_Control.h"
#include "JAScreens.h"
#include "ScreenIDs.h"


void SpecifyButtonSoundScheme(GUIButtonRef const b, ButtonSoundScheme scheme)
{
#ifdef JA2
	if (scheme == BUTTON_SOUND_SCHEME_GENERIC)
	{
		switch (guiCurrentScreen)
		{
			case MAINMENU_SCREEN:
			case OPTIONS_SCREEN:
			case LOADSAVE_SCREEN:
			case SAVE_LOAD_SCREEN:
			case INIT_SCREEN:
				scheme = BUTTON_SOUND_SCHEME_BIGSWITCH3;
				break;

			case LAPTOP_SCREEN:
				scheme = BUTTON_SOUND_SCHEME_COMPUTERBEEP2;
				break;

			case AUTORESOLVE_SCREEN:
			case MAP_SCREEN:
			case GAME_SCREEN:
			case SHOPKEEPER_SCREEN:
				scheme = BUTTON_SOUND_SCHEME_SMALLSWITCH2;
				break;

			case GAME_INIT_OPTIONS_SCREEN:
				scheme = BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2;
				break;

			//Anything not handled gets NO sound.
			//SHOPKEEPER_SCREEN,
			//GAME_SCREEN,
			//MSG_BOX_SCREEN,
			//ERROR_SCREEN,
			//ANIEDIT_SCREEN,
			//PALEDIT_SCREEN,
			//DEBUG_SCREEN,
			//SEX_SCREEN,
		}
	}
#endif
	b->ubSoundSchemeID = scheme;
}


void PlayButtonSound(GUI_BUTTON const* const btn, ButtonSound const sound_type)
{
	UINT32 sample;
	switch (sound_type)
	{
		case BUTTON_SOUND_CLICKED_ON:
			switch (btn->ubSoundSchemeID)
			{
#ifdef JA2
				case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH1: sample = VSM_SWITCH1_IN;      break;
				case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2: sample = VSM_SWITCH2_IN;      break;
				case BUTTON_SOUND_SCHEME_SMALLSWITCH1:     sample = SM_SWITCH1_IN;       break;
				case BUTTON_SOUND_SCHEME_SMALLSWITCH2:     sample = SM_SWITCH2_IN;       break;
				case BUTTON_SOUND_SCHEME_SMALLSWITCH3:     sample = SM_SWITCH3_IN;       break;
				case BUTTON_SOUND_SCHEME_BIGSWITCH3:       sample = BIG_SWITCH3_IN;      break;
				case BUTTON_SOUND_SCHEME_COMPUTERBEEP2:    sample = COMPUTER_BEEP2_IN;   break;
				case BUTTON_SOUND_SCHEME_COMPUTERSWITCH1:  sample = COMPUTER_SWITCH1_IN; break;
#endif

				default: return;
			}
			break;

		case BUTTON_SOUND_CLICKED_OFF:
			switch (btn->ubSoundSchemeID)
			{
#ifdef JA2
				case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH1: sample = VSM_SWITCH1_OUT;      break;
				case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2: sample = VSM_SWITCH2_OUT;      break;
				case BUTTON_SOUND_SCHEME_SMALLSWITCH1:     sample = SM_SWITCH1_OUT;       break;
				case BUTTON_SOUND_SCHEME_SMALLSWITCH2:     sample = SM_SWITCH2_OUT;       break;
				case BUTTON_SOUND_SCHEME_SMALLSWITCH3:     sample = SM_SWITCH3_OUT;       break;
				case BUTTON_SOUND_SCHEME_BIGSWITCH3:       sample = BIG_SWITCH3_OUT;      break;
				case BUTTON_SOUND_SCHEME_COMPUTERBEEP2:    sample = COMPUTER_BEEP2_OUT;   break;
				case BUTTON_SOUND_SCHEME_COMPUTERSWITCH1:  sample = COMPUTER_SWITCH1_OUT; break;
#endif

				default: return;
			}
			break;

		case BUTTON_SOUND_DISABLED_CLICK:
			switch (btn->ubSoundSchemeID)
			{
#ifdef JA2
				case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH1:
				case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2:
				case BUTTON_SOUND_SCHEME_SMALLSWITCH1:
				case BUTTON_SOUND_SCHEME_SMALLSWITCH2:
				case BUTTON_SOUND_SCHEME_SMALLSWITCH3:
				case BUTTON_SOUND_SCHEME_BIGSWITCH3:
				case BUTTON_SOUND_SCHEME_COMPUTERBEEP2:
				case BUTTON_SOUND_SCHEME_COMPUTERSWITCH1:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					return;
#endif

				default: return;
			}

		default: return;
	}
	PlayJA2Sample(sample, 15, 1, MIDDLEPAN);
}
