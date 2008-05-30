#include "Types.h"
#include "Button_Sound_Control.h"
#include "Sound_Control.h"
#include "JAScreens.h"
#include "ScreenIDs.h"


void SpecifyButtonSoundScheme(GUIButtonRef const b, ButtonSoundScheme SoundScheme)
{
	b->ubSoundSchemeID = SoundScheme;
	if (SoundScheme == BUTTON_SOUND_SCHEME_GENERIC)
	{
		#ifdef JA2
		switch( guiCurrentScreen )
		{
			case MAINMENU_SCREEN:
			case OPTIONS_SCREEN:
			case LOADSAVE_SCREEN:
			case SAVE_LOAD_SCREEN:
			case INIT_SCREEN:
				b->ubSoundSchemeID = BUTTON_SOUND_SCHEME_BIGSWITCH3;
				break;

			case LAPTOP_SCREEN:
				b->ubSoundSchemeID = BUTTON_SOUND_SCHEME_COMPUTERBEEP2;
				break;

			case AUTORESOLVE_SCREEN:
			case MAP_SCREEN:
			case GAME_SCREEN:
			case SHOPKEEPER_SCREEN:
				b->ubSoundSchemeID = BUTTON_SOUND_SCHEME_SMALLSWITCH2;
				break;

			case GAME_INIT_OPTIONS_SCREEN:
				b->ubSoundSchemeID = BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2;
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
		#endif
		if (SoundScheme == BUTTON_SOUND_SCHEME_GENERIC)
			SoundScheme = BUTTON_SOUND_SCHEME_NONE;
	}
}


void PlayButtonSound(GUI_BUTTON const* const btn, ButtonSound const iSoundType)
{
	switch (btn->ubSoundSchemeID)
	{
		case BUTTON_SOUND_SCHEME_NONE:
		case BUTTON_SOUND_SCHEME_GENERIC:
			break;

#ifdef JA2

		case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH1:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(VSM_SWITCH1_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(VSM_SWITCH1_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(VSM_SWITCH2_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(VSM_SWITCH2_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_SMALLSWITCH1:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(SM_SWITCH1_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(SM_SWITCH1_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_SMALLSWITCH2:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(SM_SWITCH2_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(SM_SWITCH2_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_SMALLSWITCH3:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(SM_SWITCH3_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(SM_SWITCH3_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_BIGSWITCH3:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(BIG_SWITCH3_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(BIG_SWITCH3_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_COMPUTERBEEP2:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(COMPUTER_BEEP2_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(COMPUTER_BEEP2_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_COMPUTERSWITCH1:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample(COMPUTER_SWITCH1_IN, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample(COMPUTER_SWITCH1_OUT, 15, 1, MIDDLEPAN);
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile("Sounds/Disabled Button.wav", 15, 1, MIDDLEPAN);
					break;
			}
			break;

#endif

	}

}
