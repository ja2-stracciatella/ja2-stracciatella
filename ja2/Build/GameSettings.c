#ifdef PRECOMPILEDHEADERS
	#include "JA2 All.h"
	#include "HelpScreen.h"
	#include "Campaign.h"
	#include "Cheats.h"
#else
	#include	"Types.h"
	#include	"GameSettings.h"
	#include	"FileMan.h"
	#include	"String.h"
	#include	"Sound Control.h"
	#include	"SaveLoadScreen.h"
	#include	"Music Control.h"
	#include	"Options Screen.h"
	#include	"Overhead.h"
	#include	"GameVersion.h"
	#include	"LibraryDataBase.h"
	#include	"Debug.h"
	#include	"Language Defines.h"
	#include "HelpScreen.h"
#endif

#include	"Text.h"

#define				GAME_SETTINGS_FILE		"..\\Ja2.set"

#define				GAME_INI_FILE					"..\\Ja2.ini"

#define				CD_ROOT_DIR						"DATA\\"

GAME_SETTINGS		gGameSettings;
GAME_OPTIONS		gGameOptions;

extern	SGPFILENAME	gCheckFilenames[];
extern	CHAR8		gzErrorMsg[256];

void		InitGameSettings();

BOOLEAN GetCdromLocationFromIniFile( STR pRootOfCdromDrive );

extern BOOLEAN DoJA2FilesExistsOnDrive( CHAR8 *zCdLocation );

BOOLEAN GetCDromDriveLetter( STR8	pString );
BOOLEAN	IsDriveLetterACDromDrive( STR pDriveLetter );
void		CDromEjectionErrorMessageBoxCallBack( UINT8 bExitValue );


//Change this number when we want any who gets the new build to reset the options
#define				GAME_SETTING_CURRENT_VERSION		522



BOOLEAN LoadGameSettings()
{
	HWFILE	hFile;
	UINT32	uiNumBytesRead;


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
		hFile = FileOpen( GAME_SETTINGS_FILE, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE );
		if( !hFile )
		{
			FileClose( hFile );
			InitGameSettings();
			return(FALSE);
		}

		FileRead( hFile, &gGameSettings, sizeof( GAME_SETTINGS ), &uiNumBytesRead );
		if( uiNumBytesRead != sizeof( GAME_SETTINGS ) )
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
	UINT32	uiNumBytesWritten;



	//create the file
	hFile = FileOpen( GAME_SETTINGS_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE );
	if( !hFile )
	{
		FileClose( hFile );
		return(FALSE);
	}



	//Record the current settings into the game settins structure

	gGameSettings.ubSoundEffectsVolume = (UINT8)GetSoundEffectsVolume( );
	gGameSettings.ubSpeechVolume = (UINT8)GetSpeechVolume( );
	gGameSettings.ubMusicVolumeSetting = (UINT8)MusicGetVolume( );

	strcpy( gGameSettings.zVersionNumber, czVersionNumber );

	gGameSettings.uiSettingsVersionNumber = GAME_SETTING_CURRENT_VERSION;

	//Write the game settings to disk
	FileWrite( hFile, &gGameSettings, sizeof( GAME_SETTINGS ), &uiNumBytesWritten );
	if( uiNumBytesWritten != sizeof( GAME_SETTINGS ) )
	{
		FileClose( hFile );
		return(FALSE);
	}

	FileClose( hFile );

	return( TRUE );
}

void InitGameSettings()
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


BOOLEAN GetCDLocation( )
{
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
			sprintf( zCdLocation, "c" );
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
}



BOOLEAN GetCDromDriveLetter( STR8	pString )
{
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
}


/*


	//Determine the type of drive the CDrom is on
	uiDriveType = GetDriveType( zCdLocation );
	switch( uiDriveType )
	{
		// The root directory does not exist.
		case DRIVE_NO_ROOT_DIR:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The root directory does not exist." ) );
			break;


		// The disk can be removed from the drive.
		case DRIVE_REMOVABLE:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The disk can be removed from the drive." ) );
			break;


		// The disk cannot be removed from the drive.
		case DRIVE_FIXED:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The disk cannot be removed from the drive." ) );
			break;


		// The drive is a remote (network) drive.
		case DRIVE_REMOTE:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The drive is a remote (network) drive." ) );
			break;


		// The drive is a CD-ROM drive.
		case DRIVE_CDROM:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The drive is a CD-ROM drive." ) );
			break;


		// The drive is a RAM disk.
		case DRIVE_RAMDISK:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The drive is a RAM disk." ) );
			break;


		// The drive type cannot be determined.
		case DRIVE_UNKNOWN:
		default:
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("CDrom Info '%s': %s", zCdLocation, "The drive type cannot be determined." ) );
			break;
	}


*/

BOOLEAN CheckIfGameCdromIsInCDromDrive()
{
	CHAR8		zVolumeNameBuffer[512];
	UINT32	uiVolumeSerialNumber=0;
	UINT32	uiMaxComponentLength=0;
	UINT32	uiFileSystemFlags=0;
	CHAR8		zFileSystemNameBuffer[512];
	CHAR8		zCdLocation[ SGPFILENAME_LEN ];
	CHAR8		zCdFile[ SGPFILENAME_LEN ];

	CHAR8		zCdromRootDrive[512];
	BOOLEAN	fFailed = FALSE;
	UINT32	uiVolumeReturnValue;
	UINT32	uiLastError = ERROR_SUCCESS;

	if( !GetCdromLocationFromIniFile( zCdromRootDrive ) )
		return( FALSE );

	uiVolumeReturnValue = GetVolumeInformation( zCdromRootDrive, zVolumeNameBuffer, 512, &uiVolumeSerialNumber, &uiMaxComponentLength, &uiFileSystemFlags, zFileSystemNameBuffer, 512 );

	if( !uiVolumeReturnValue )
	{
		 uiLastError = GetLastError();
	}

	// OK, build filename
	sprintf( zCdFile, "%s%s", zCdLocation, gCheckFilenames[ Random( 2 ) ] );

		//If the cdrom drive is no longer in the drive
	if( uiLastError == ERROR_NOT_READY || ( !FileExists( zCdFile ) ) )
	{
			CHAR8 sString[512];

			//if a game has been started, add the msg about saving the game to a different entry
			if( gTacticalStatus.fHasAGameBeenStarted )
			{
				sprintf( sString, "%S  %S", pMessageStrings[ MSG_INTEGRITY_WARNING ], pMessageStrings[ MSG_CDROM_SAVE_GAME ] );

				SaveGame( SAVE__ERROR_NUM, pMessageStrings[ MSG_CDROM_SAVE ] );
			}
			else
			{
				sprintf( sString, "%S", pMessageStrings[ MSG_INTEGRITY_WARNING ] );
			}

      // ATE: These are ness. due to reference counting
      // in showcursor(). I'm not about to go digging in low level stuff at this
      // point in the game development, so keep these here, as this works...
      ShowCursor(TRUE);
      ShowCursor(TRUE);
      ShutdownWithErrorBox( sString );

      //DoTester( );
      //MessageBox(NULL, sString, "Error", MB_OK | MB_ICONERROR  );

			return( FALSE );
	}

	return( TRUE );
}


BOOLEAN GetCdromLocationFromIniFile( STR pRootOfCdromDrive )
{
	UINT32	uiRetVal=0;

	//Do a crude check to make sure the Ja2.ini file is the right on

	uiRetVal = GetPrivateProfileString( "Ja2 Settings","CD", "", pRootOfCdromDrive, SGPFILENAME_LEN, GAME_INI_FILE );
	if( uiRetVal == 0 )
	{
		pRootOfCdromDrive[0] = '\0';
		return( FALSE);
	}
	else
	{
		//add the :\ to the dir
		strcat( pRootOfCdromDrive, ":\\" );
		return( TRUE );
	}
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


BOOLEAN IsDriveLetterACDromDrive( STR pDriveLetter )
{
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
}

void DisplayGameSettings( )
{
	//Display the version number
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s (%S)", pMessageStrings[ MSG_VERSION ], zVersionLabel, czVersionNumber );

	//Display the difficulty level
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[ GIO_DIF_LEVEL_TEXT ], gzGIOScreenText[ gGameOptions.ubDifficultyLevel + GIO_EASY_TEXT - 1 ] );

	//Iron man option
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[ GIO_GAME_SAVE_STYLE_TEXT ], gzGIOScreenText[ GIO_SAVE_ANYWHERE_TEXT + gGameOptions.fIronManMode ] );

	// Gun option
	if( gGameOptions.fGunNut )
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT ], gzGIOScreenText[ GIO_GUN_NUT_TEXT ] );
	else
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT ], gzGIOScreenText[ GIO_REDUCED_GUNS_TEXT ] );

	//Sci fi option
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[ GIO_GAME_STYLE_TEXT ], gzGIOScreenText[ GIO_REALISTIC_TEXT + gGameOptions.fSciFi ] );

	//Timed Turns option
	// JA2Gold: no timed turns
	//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT ], gzGIOScreenText[ GIO_NO_TIMED_TURNS_TEXT + gGameOptions.fTurnTimeLimit ] );

	if( CHEATER_CHEAT_LEVEL() )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[58], CurrentPlayerProgressPercentage(), HighestPlayerProgressPercentage() );
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
