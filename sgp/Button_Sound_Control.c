#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	#include "Button_System.h"
	#include "Button_Sound_Control.h"
	#include "Sound_Control.h"
	#include "JAScreens.h"
#endif

void SpecifyButtonSoundScheme( INT32 iButtonID, INT8 bSoundScheme )
{
	ButtonList[ iButtonID ]->ubSoundSchemeID = (UINT8)bSoundScheme;
	if( bSoundScheme == BUTTON_SOUND_SCHEME_GENERIC )
	{
		#ifdef JA2
		switch( guiCurrentScreen )
		{
			case MAINMENU_SCREEN:
			case OPTIONS_SCREEN:
			case LOADSAVE_SCREEN:
			case SAVE_LOAD_SCREEN:
			case INIT_SCREEN:
				ButtonList[ iButtonID ]->ubSoundSchemeID = BUTTON_SOUND_SCHEME_BIGSWITCH3;
				break;

			case LAPTOP_SCREEN:
				ButtonList[ iButtonID ]->ubSoundSchemeID = BUTTON_SOUND_SCHEME_COMPUTERBEEP2;
				break;

			case AUTORESOLVE_SCREEN:
			case MAP_SCREEN:
			case GAME_SCREEN:
			case SHOPKEEPER_SCREEN:
				ButtonList[ iButtonID ]->ubSoundSchemeID = BUTTON_SOUND_SCHEME_SMALLSWITCH2;
				break;

			case GAME_INIT_OPTIONS_SCREEN:
				ButtonList[ iButtonID ]->ubSoundSchemeID = BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2;
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
		if( bSoundScheme == BUTTON_SOUND_SCHEME_GENERIC )
			bSoundScheme = BUTTON_SOUND_SCHEME_NONE;
	}
}

void PlayButtonSound( INT32 iButtonID, INT32 iSoundType )
{
	if ( ButtonList[ iButtonID ] == NULL )
	{
		return;
	}

	switch( ButtonList[ iButtonID ]->ubSoundSchemeID )
	{
		case BUTTON_SOUND_SCHEME_NONE:
		case BUTTON_SOUND_SCHEME_GENERIC:
			break;

#ifdef JA2

		case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH1:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( VSM_SWITCH1_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( VSM_SWITCH1_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_VERYSMALLSWITCH2:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( VSM_SWITCH2_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( VSM_SWITCH2_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_SMALLSWITCH1:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( SM_SWITCH1_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( SM_SWITCH1_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_SMALLSWITCH2:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( SM_SWITCH2_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( SM_SWITCH2_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_SMALLSWITCH3:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( SM_SWITCH3_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( SM_SWITCH3_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_BIGSWITCH3:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( BIG_SWITCH3_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( BIG_SWITCH3_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_COMPUTERBEEP2:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( COMPUTER_BEEP2_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( COMPUTER_BEEP2_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;
		case BUTTON_SOUND_SCHEME_COMPUTERSWITCH1:
			switch( iSoundType )
			{
				case BUTTON_SOUND_CLICKED_ON:
					PlayJA2Sample( COMPUTER_SWITCH1_IN, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_CLICKED_OFF:
					PlayJA2Sample( COMPUTER_SWITCH1_OUT, RATE_11025, 15, 1, MIDDLEPAN );
					break;
				case BUTTON_SOUND_DISABLED_CLICK:
					PlayJA2SampleFromFile( "Sounds\\Disabled Button.wav", RATE_11025, 15, 1, MIDDLEPAN );
					break;
			}
			break;

#endif

	}

}
