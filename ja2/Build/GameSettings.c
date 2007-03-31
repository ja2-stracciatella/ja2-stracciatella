#include "Types.h"
#include "GameSettings.h"
#include "FileMan.h"
#include "Sound_Control.h"
#include "SaveLoadScreen.h"
#include "Music_Control.h"
#include "Options_Screen.h"
#include "Overhead.h"
#include "GameVersion.h"
#include "LibraryDataBase.h"
#include "Debug.h"
#include "Language_Defines.h"
#include "HelpScreen.h"
#include "Shade_Table_Util.h"
#include "Meanwhile.h"
#include "Cheats.h"
#include "Message.h"
#include "Campaign.h"
#include "StrategicMap.h"
#include "Queen_Command.h"
#include "ScreenIDs.h"
#include "Random.h"
#include "SGP.h"
#include "SaveLoadGame.h"
#include	"Text.h"


#define				GAME_SETTINGS_FILE		"../Ja2.set"

#define				GAME_INI_FILE					"../Ja2.ini"

#define				CD_ROOT_DIR						"DATA/"

GAME_SETTINGS		gGameSettings;
GAME_OPTIONS		gGameOptions;

extern BOOLEAN DoJA2FilesExistsOnDrive( CHAR8 *zCdLocation );


//Change this number when we want any who gets the new build to reset the options
#define				GAME_SETTING_CURRENT_VERSION		522


static void InitGameSettings(void);


BOOLEAN LoadGameSettings()
{
	HWFILE	hFile;

	//if the game settings file does NOT exist, or if it is smaller then what it should be
	if( !FileExists( GAME_SETTINGS_FILE ) || FileSize( GAME_SETTINGS_FILE ) != sizeof( GAME_SETTINGS ) )
	{
		//Initialize the settings
		InitGameSettings();

		//delete the shade tables aswell
		DeleteShadeTableDir( );
	}
	else
	{
		hFile = FileOpen(GAME_SETTINGS_FILE, FILE_ACCESS_READ | FILE_OPEN_EXISTING);
		if( !hFile )
		{
			InitGameSettings();
			return(FALSE);
		}

		if (!FileRead(hFile, &gGameSettings, sizeof(GAME_SETTINGS)))
		{
			FileClose( hFile );
			InitGameSettings();
			return(FALSE);
		}

		FileClose( hFile );
	}


	//if the version in the game setting file is older then the we want, init the game settings
	if( gGameSettings.uiSettingsVersionNumber < GAME_SETTING_CURRENT_VERSION )
	{
		//Initialize the settings
		InitGameSettings();

		//delete the shade tables aswell
		DeleteShadeTableDir( );

		return( TRUE );
	}


	//
	//Do checking to make sure the settings are valid
	//
	if( gGameSettings.bLastSavedGameSlot < 0 || gGameSettings.bLastSavedGameSlot >= NUM_SAVE_GAMES )
		gGameSettings.bLastSavedGameSlot = -1;

	if( gGameSettings.ubMusicVolumeSetting > HIGHVOLUME )
		gGameSettings.ubMusicVolumeSetting = MIDVOLUME;

	if( gGameSettings.ubSoundEffectsVolume > HIGHVOLUME )
		gGameSettings.ubSoundEffectsVolume = MIDVOLUME;

	if( gGameSettings.ubSpeechVolume > HIGHVOLUME )
		gGameSettings.ubSpeechVolume = MIDVOLUME;


	//make sure that at least subtitles or speech is enabled
	if( !gGameSettings.fOptions[ TOPTION_SUBTITLES ] && !gGameSettings.fOptions[ TOPTION_SPEECH ] )
	{
		gGameSettings.fOptions[ TOPTION_SUBTITLES ]						= TRUE;
		gGameSettings.fOptions[ TOPTION_SPEECH ]							= TRUE;
	}


	//
	//	Set the settings
	//

	SetSoundEffectsVolume( gGameSettings.ubSoundEffectsVolume );
	SetSpeechVolume( gGameSettings.ubSpeechVolume );
	MusicSetVolume( gGameSettings.ubMusicVolumeSetting );

#ifndef BLOOD_N_GORE_ENABLED
	gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ]				= FALSE;
#endif

	//if the user doesnt want the help screens present
	if( gGameSettings.fHideHelpInAllScreens )
	{
		gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen = 0;
	}
	else
	{
		//Set it so that every screens help will come up the first time ( the 'x' will be set )
		gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen = 0xffff;
	}

	return( TRUE );
}


BOOLEAN	SaveGameSettings()
{
	HWFILE		hFile;

	//create the file
	hFile = FileOpen(GAME_SETTINGS_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
	if( !hFile )
	{
		return(FALSE);
	}



	//Record the current settings into the game settins structure

	gGameSettings.ubSoundEffectsVolume = (UINT8)GetSoundEffectsVolume( );
	gGameSettings.ubSpeechVolume = (UINT8)GetSpeechVolume( );
	gGameSettings.ubMusicVolumeSetting = (UINT8)MusicGetVolume( );

	strcpy( gGameSettings.zVersionNumber, czVersionNumber );

	gGameSettings.uiSettingsVersionNumber = GAME_SETTING_CURRENT_VERSION;

	//Write the game settings to disk
	if (!FileWrite(hFile, &gGameSettings, sizeof(GAME_SETTINGS)))
	{
		FileClose( hFile );
		return(FALSE);
	}

	FileClose( hFile );

	return( TRUE );
}


static void InitGameSettings(void)
{
	memset( &gGameSettings, 0, sizeof( GAME_SETTINGS ) );

	//Init the Game Settings
	gGameSettings.bLastSavedGameSlot		= -1;
	gGameSettings.ubMusicVolumeSetting	= 63;
	gGameSettings.ubSoundEffectsVolume	= 63;
	gGameSettings.ubSpeechVolume				= 63;

	//Set the settings
	SetSoundEffectsVolume( gGameSettings.ubSoundEffectsVolume );
	SetSpeechVolume( gGameSettings.ubSpeechVolume );
	MusicSetVolume( gGameSettings.ubMusicVolumeSetting );

	gGameSettings.fOptions[ TOPTION_SUBTITLES ]									= TRUE;
	gGameSettings.fOptions[ TOPTION_SPEECH ]										= TRUE;
	gGameSettings.fOptions[ TOPTION_KEY_ADVANCE_SPEECH ]				= FALSE;
	gGameSettings.fOptions[ TOPTION_RTCONFIRM ]									= FALSE;
	gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ]							= FALSE;
	gGameSettings.fOptions[ TOPTION_TRACKING_MODE ]							= TRUE;
	gGameSettings.fOptions[ TOPTION_MUTE_CONFIRMATIONS ]				= FALSE;
	gGameSettings.fOptions[ TOPTION_ANIMATE_SMOKE ]				      = TRUE;
	gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ]							= TRUE;
	gGameSettings.fOptions[ TOPTION_DONT_MOVE_MOUSE ]						= FALSE;
	gGameSettings.fOptions[ TOPTION_OLD_SELECTION_METHOD ]			= FALSE;
	gGameSettings.fOptions[ TOPTION_ALWAYS_SHOW_MOVEMENT_PATH ] = FALSE;

	gGameSettings.fOptions[ TOPTION_SLEEPWAKE_NOTIFICATION ]		= TRUE;

	gGameSettings.fOptions[ TOPTION_USE_METRIC_SYSTEM ]					= FALSE;

	#ifndef BLOOD_N_GORE_ENABLED
		gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ]						= FALSE;
	#endif

	gGameSettings.fOptions[ TOPTION_MERC_ALWAYS_LIGHT_UP ]			= FALSE;
	gGameSettings.fOptions[ TOPTION_SMART_CURSOR ]							= FALSE;

	gGameSettings.fOptions[ TOPTION_SNAP_CURSOR_TO_DOOR ]				= TRUE;
	gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ]								= TRUE;
	gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ]					= TRUE;
	gGameSettings.fOptions[ TOPTION_TOGGLE_WIREFRAME ]					= TRUE;
	gGameSettings.fOptions[ TOPTION_3D_CURSOR ]									= FALSE;
	// JA2Gold
	gGameSettings.fOptions[ TOPTION_MERC_CASTS_LIGHT ]					= TRUE;

	gGameSettings.ubSizeOfDisplayCover = 4;
	gGameSettings.ubSizeOfLOS = 4;

	//Since we just set the settings, save them
	SaveGameSettings();
}

void InitGameOptions()
{
	memset( &gGameOptions, 0, sizeof( GAME_OPTIONS ) );

	//Init the game options
	gGameOptions.fGunNut					 = FALSE;
	gGameOptions.fSciFi						 = TRUE;
	gGameOptions.ubDifficultyLevel = DIF_LEVEL_EASY;
	//gGameOptions.fTurnTimeLimit		 = FALSE;
	gGameOptions.fIronManMode			 = FALSE;

}


#ifndef DATADIR
#	error No DATADIR specified
#endif


static BOOLEAN GetCDromDriveLetter(STR8 pString);
static BOOLEAN IsDriveLetterACDromDrive(STR pDriveLetter);


BOOLEAN GetCDLocation( )
{
#if 1 // XXX TODO
	FIXME
	strcpy(gzCdDirectory, DATADIR "/Data/");
	return TRUE;
#else
	UINT32	uiStrngLength = 0;
	CHAR8		zCdLocation[ SGPFILENAME_LEN ];
	UINT32	uiDriveType=0;
	UINT32	uiRetVal=0;

	//Do a crude check to make sure the Ja2.ini file is the right on

	uiRetVal = GetPrivateProfileString( "Ja2 Settings","CD", "", zCdLocation, SGPFILENAME_LEN, GAME_INI_FILE );
	if( uiRetVal == 0 || !IsDriveLetterACDromDrive( zCdLocation ) )
	{
		// the user most likely doesnt have the file, or the user has messed with the file
		// build a new one

		//First delete the old file
		FileDelete( GAME_INI_FILE );

		//Get the location of the cdrom drive
		if( GetCDromDriveLetter( zCdLocation ) )
		{
			CHAR8		*pTemp;

			//if it succeeded
			pTemp = strrchr( zCdLocation, ':' );
			pTemp[0] = '\0';
		}
		else
		{
			//put in a default location
			strcpy(zCdLocation, "c");
		}

		//Now create a new file
		 WritePrivateProfileString( "Ja2 Settings", "CD", zCdLocation, GAME_INI_FILE );

		 GetPrivateProfileString( "Ja2 Settings","CD", "", zCdLocation, SGPFILENAME_LEN, GAME_INI_FILE );
	}

	uiStrngLength = strlen( zCdLocation );

	//if the string length is less the 1 character, it is a drive letter
	if( uiStrngLength == 1 )
	{
		sprintf( gzCdDirectory, "%s:\\%s", zCdLocation, CD_ROOT_DIR );
	}

	//else it is most likely a network location
	else if( uiStrngLength > 1 )
	{
		sprintf( gzCdDirectory, "%s\\%s", zCdLocation, CD_ROOT_DIR );
	}
	else
	{
		//no path was entered
		gzCdDirectory[ 0 ] = '.';
	}

	return( TRUE );
#endif
}


static BOOLEAN GetCDromDriveLetter(STR8 pString)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	UINT32	uiSize=0;
	UINT8		ubCnt=0;
	CHAR8		zDriveLetters[512];
	CHAR8		zDriveLetter[16];
	UINT32	uiDriveType;

	uiSize = GetLogicalDriveStrings( 512, zDriveLetters );

	for( ubCnt=0;ubCnt<uiSize;ubCnt++ )
	{
		//if the current char is not null
		if( zDriveLetters[ ubCnt ] != '\0' )
		{
			//get the string
			zDriveLetter[ 0 ] = zDriveLetters[ ubCnt ];
			ubCnt++;
			zDriveLetter[ 1 ] = zDriveLetters[ ubCnt ];
			ubCnt++;
			zDriveLetter[ 2 ] = zDriveLetters[ ubCnt ];

			zDriveLetter[ 3 ] = '\0';

			//Get the drive type
			uiDriveType = GetDriveType( zDriveLetter );
			switch( uiDriveType )
			{
				// The drive is a CD-ROM drive.
				case DRIVE_CDROM:
					strcpy( pString, zDriveLetter );

					if ( DoJA2FilesExistsOnDrive( pString ) )
					{
						return( TRUE );
					}
					break;

				default:
					break;
			}
		}
	}

	return( FALSE );
#endif
}


void CDromEjectionErrorMessageBoxCallBack( UINT8 bExitValue )
{
	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		guiPreviousOptionScreen = GAME_SCREEN;

		//if we are in a game, save the game
		if( gTacticalStatus.fHasAGameBeenStarted )
		{
			SaveGame( SAVE__ERROR_NUM, pMessageStrings[ MSG_CDROM_SAVE ] );
		}

 		//quit the game
		gfProgramIsRunning = FALSE;
	}
}


static BOOLEAN IsDriveLetterACDromDrive(STR pDriveLetter)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	UINT32	uiDriveType;
	CHAR8		zRootName[512];

	sprintf( zRootName, "%s:\\", pDriveLetter );

	//Get the drive type
	uiDriveType = GetDriveType( zRootName );
	switch( uiDriveType )
	{
		// The drive is a CD-ROM drive.
#ifdef JA2BETAVERSION
		case DRIVE_NO_ROOT_DIR:
		case DRIVE_REMOTE:
#endif
		case DRIVE_CDROM:
			return( TRUE );
			break;
	}

	return( FALSE );
#endif
}

void DisplayGameSettings( )
{
	//Display the version number
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%S: %S (%s)", pMessageStrings[MSG_VERSION], zVersionLabel, czVersionNumber);

	//Display the difficulty level
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%S: %S", gzGIOScreenText[GIO_DIF_LEVEL_TEXT], gzGIOScreenText[gGameOptions.ubDifficultyLevel + GIO_EASY_TEXT - 1]);

	//Iron man option
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%S: %S", gzGIOScreenText[GIO_GAME_SAVE_STYLE_TEXT], gzGIOScreenText[GIO_SAVE_ANYWHERE_TEXT + gGameOptions.fIronManMode]);

	// Gun option
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%S: %S", gzGIOScreenText[GIO_GUN_OPTIONS_TEXT], gzGIOScreenText[gGameOptions.fGunNut ? GIO_GUN_NUT_TEXT : GIO_REDUCED_GUNS_TEXT]);

	//Sci fi option
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%S: %S", gzGIOScreenText[GIO_GAME_STYLE_TEXT], gzGIOScreenText[GIO_REALISTIC_TEXT + gGameOptions.fSciFi]);

	//Timed Turns option
	// JA2Gold: no timed turns
	//ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%S: %S", gzGIOScreenText[GIO_TIMED_TURN_TITLE_TEXT], gzGIOScreenText[GIO_NO_TIMED_TURNS_TEXT + gGameOptions.fTurnTimeLimit]);

	if (CHEATER_CHEAT_LEVEL())
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[58], CurrentPlayerProgressPercentage(), HighestPlayerProgressPercentage());
	}
}

BOOLEAN MeanwhileSceneSeen( UINT8 ubMeanwhile )
{
	UINT32	uiCheckFlag;

	if ( ubMeanwhile > 32 || ubMeanwhile > NUM_MEANWHILES )
	{
		return( FALSE );
	}

	uiCheckFlag = 0x1 << ubMeanwhile;

	if ( gGameSettings.uiMeanwhileScenesSeenFlags & uiCheckFlag )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

BOOLEAN SetMeanwhileSceneSeen( UINT8 ubMeanwhile )
{
	UINT32	uiCheckFlag;

	if ( ubMeanwhile > 32 || ubMeanwhile > NUM_MEANWHILES )
	{
		// can't set such a flag!
		return( FALSE );
	}
	uiCheckFlag = 0x1 << ubMeanwhile;
	gGameSettings.uiMeanwhileScenesSeenFlags |= uiCheckFlag;
	return( TRUE );
}

BOOLEAN	CanGameBeSaved()
{
	//if the iron man mode is on
	if( gGameOptions.fIronManMode )
	{
		//if we are in turn based combat
		if( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			//no save for you
			return( FALSE );
		}

		//if there are enemies in the current sector
		if( gWorldSectorX != -1 && gWorldSectorY != -1 &&
				gWorldSectorX != 0 && gWorldSectorY != 0 &&
				NumEnemiesInAnySector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) > 0 )
		{
			//no save for you
			return( FALSE );
		}

		//All checks failed, so we can save
		return( TRUE );
	}
	else
	{
		return( TRUE );
	}
}
