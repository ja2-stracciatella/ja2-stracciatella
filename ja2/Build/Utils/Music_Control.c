#ifdef PRECOMPILEDHEADERS
	#include "Utils_All.h"
#else
	#include "Types.h"
	#include "Music_Control.h"
	#include "SoundMan.h"
	#include "Random.h"
	#include "GameScreen.h"
	#include "JAScreens.h"
	#include "Creature_Spreading.h"
	#include "Soldier_Control.h"
	#include "Overhead.h"
	#include "Timer_Control.h"
	#include "StrategicMap.h"
	#include "Fade_Screen.h"
#endif


UINT32 uiMusicHandle=NO_SAMPLE;
UINT32 uiMusicVolume=50;
BOOLEAN fMusicPlaying=FALSE;
BOOLEAN fMusicFadingOut=FALSE;
BOOLEAN fMusicFadingIn=FALSE;

BOOLEAN	gfMusicEnded = FALSE;

UINT8		gubMusicMode = 0;
UINT8		gubOldMusicMode = 0;

INT8		gbVictorySongCount = 0;
INT8		gbDeathSongCount = 0;

INT8		bNothingModeSong;
INT8		bEnemyModeSong;
INT8		bBattleModeSong;

INT8		gbFadeSpeed = 1;

#ifdef JA2DEMO

CHAR8 *szMusicList[NUM_MUSIC]=
{
	"MUSIC\\marimbad 2.wav",
	"MUSIC\\menumix1.wav",
	"MUSIC\\nothing A.wav",
	"MUSIC\\nothing B.wav",
	"MUSIC\\nothing A.wav",
	"MUSIC\\nothing B.wav",
	"MUSIC\\tensor B.wav",
	"MUSIC\\tensor B.wav",
	"MUSIC\\tensor B.wav",
	"MUSIC\\triumph.wav",
	"MUSIC\\death.wav",
	"MUSIC\\battle A.wav",
	"MUSIC\\tensor B.wav",
	"MUSIC\\creepy.wav",
	"MUSIC\\creature battle.wav",
};


#else

CHAR8 *szMusicList[NUM_MUSIC]=
{
	"MUSIC\\marimbad 2.wav",
	"MUSIC\\menumix1.wav",
	"MUSIC\\nothing A.wav",
	"MUSIC\\nothing B.wav",
	"MUSIC\\nothing C.wav",
	"MUSIC\\nothing D.wav",
	"MUSIC\\tensor A.wav",
	"MUSIC\\tensor B.wav",
	"MUSIC\\tensor C.wav",
	"MUSIC\\triumph.wav",
	"MUSIC\\death.wav",
	"MUSIC\\battle A.wav",
	"MUSIC\\tensor B.wav",
	"MUSIC\\creepy.wav",
	"MUSIC\\creature battle.wav",
};

#endif

BOOLEAN	gfForceMusicToTense = FALSE;
BOOLEAN gfDontRestartSong   = FALSE;

BOOLEAN StartMusicBasedOnMode( );
void	DoneFadeOutDueToEndMusic( void );
extern void HandleEndDemoInCreatureLevel( );


BOOLEAN NoEnemiesInSight( )
{
	SOLDIERTYPE             *pSoldier;
	INT32										cnt;

	// Loop through our guys
	// End the turn of player charactors
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

  // look for all mercs on the same team,
  for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++, pSoldier++ )
	{
		if ( pSoldier->bActive && pSoldier->bLife >= OKLIFE )
		{
			if ( pSoldier->bOppCnt != 0 )
			{
				return( FALSE );
			}
		}
	}

	return( TRUE );
}

void MusicStopCallback( void *pData );

//********************************************************************************
// MusicPlay
//
//		Starts up one of the tunes in the music list.
//
//	Returns:	TRUE if the music was started, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicPlay(UINT32 uiNum)
{
#ifndef WINDOWED_MODE

SOUNDPARMS spParms;

	if(fMusicPlaying)
		MusicStop();

	memset(&spParms, 0xff, sizeof(SOUNDPARMS));
	spParms.uiPriority=PRIORITY_MAX;
	spParms.uiVolume=0;

	spParms.EOSCallback = MusicStopCallback;


	uiMusicHandle=SoundPlayStreamedFile(szMusicList[uiNum], &spParms);

	if(uiMusicHandle!=SOUND_ERROR)
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music PLay %d %d", uiMusicHandle, gubMusicMode  ) );

		gfMusicEnded				= FALSE;
		fMusicPlaying=TRUE;
		MusicFadeIn();
		return(TRUE);
	}

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music PLay %d %d", uiMusicHandle, gubMusicMode  ) );


#endif

	return(FALSE);
}

//********************************************************************************
// MusicSetVolume
//
//		Sets the volume on the currently playing music.
//
//	Returns:	TRUE if the volume was set, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicSetVolume(UINT32 uiVolume)
{
  INT32 uiOldMusicVolume = uiMusicVolume;

#ifndef WINDOWED_MODE

	uiMusicVolume=__min(uiVolume, 127);

	if(uiMusicHandle!=NO_SAMPLE)
	{
    // get volume and if 0 stop music!
    if ( uiMusicVolume == 0 )
    {
      gfDontRestartSong = TRUE;
      MusicStop( );
      return( TRUE );
    }

		SoundSetVolume(uiMusicHandle, uiMusicVolume);

		return(TRUE);
	}

  // If here, check if we need to re-start music
  // Have we re-started?
  if ( uiMusicVolume > 0 && uiOldMusicVolume == 0 )
  {
    StartMusicBasedOnMode( );
  }

#endif

	return(FALSE);
}

//********************************************************************************
// MusicGetVolume
//
//		Gets the volume on the currently playing music.
//
//	Returns:	TRUE if the volume was set, FALSE if an error occurred
//
//********************************************************************************
UINT32 MusicGetVolume(void)
{
		return(uiMusicVolume);
}

//********************************************************************************
// MusicStop
//
//		Stops the currently playing music.
//
//	Returns:	TRUE if the music was stopped, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicStop(void)
{

#ifndef WINDOWED_MODE


	if(uiMusicHandle!=NO_SAMPLE)
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music Stop %d %d", uiMusicHandle, gubMusicMode ) );

		SoundStop(uiMusicHandle);
		fMusicPlaying=FALSE;
		uiMusicHandle = NO_SAMPLE;
		return(TRUE);
	}

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music Stop %d %d", uiMusicHandle, gubMusicMode ) );

#endif

	return(FALSE);
}

//********************************************************************************
// MusicFadeOut
//
//		Fades out the current song.
//
//	Returns:	TRUE if the music has begun fading, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicFadeOut(void)
{
	if(uiMusicHandle!=NO_SAMPLE)
	{
		fMusicFadingOut=TRUE;
		return(TRUE);
	}
	return(FALSE);
}

//********************************************************************************
// MusicFadeIn
//
//		Fades in the current song.
//
//	Returns:	TRUE if the music has begun fading in, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicFadeIn(void)
{
	if(uiMusicHandle!=NO_SAMPLE)
	{
		fMusicFadingIn=TRUE;
		return(TRUE);
	}
	return(FALSE);
}

//********************************************************************************
// MusicPoll
//
//		Handles any maintenance the music system needs done. Should be polled from
//	the main loop, or somewhere with a high frequency of calls.
//
//	Returns:	TRUE always
//
//********************************************************************************
BOOLEAN MusicPoll( BOOLEAN fForce )
{

#ifndef WINDOWED_MODE
	INT32 iVol;

	SoundServiceStreams();
	SoundServiceRandom();

	// Handle Sound every sound overhead time....
	if ( COUNTERDONE( MUSICOVERHEAD )  )
	{
		// Reset counter
		RESETCOUNTER( MUSICOVERHEAD );

		if(fMusicFadingIn)
		{
			if(uiMusicHandle!=NO_SAMPLE)
			{
				iVol=SoundGetVolume(uiMusicHandle);
				iVol=__min( (INT32)uiMusicVolume, iVol+gbFadeSpeed );
				SoundSetVolume(uiMusicHandle, iVol);
				if(iVol==(INT32)uiMusicVolume)
				{
					fMusicFadingIn=FALSE;
					gbFadeSpeed = 1;
				}
			}
		}
		else if(fMusicFadingOut)
		{
			if(uiMusicHandle!=NO_SAMPLE)
			{
				iVol=SoundGetVolume(uiMusicHandle);
				iVol=(iVol >=1)? iVol-gbFadeSpeed : 0;

				iVol=__max( (INT32)iVol, 0 );

				SoundSetVolume(uiMusicHandle, iVol);
				if(iVol==0)
				{
					MusicStop();
					fMusicFadingOut=FALSE;
					gbFadeSpeed = 1;
				}
			}
		}

	//#endif

		if ( gfMusicEnded )
		{
			// OK, based on our music mode, play another!
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music End Loop %d %d", uiMusicHandle, gubMusicMode ) );

			// If we were in victory mode, change!
			if ( gbVictorySongCount == 1 || gbDeathSongCount == 1 )
			{
				if ( gbDeathSongCount == 1 && guiCurrentScreen == GAME_SCREEN )
				{
#ifdef JA2DEMO
					gFadeOutDoneCallback = DoneFadeOutDueToEndMusic;
					FadeOutGameScreen( );
#else
					CheckAndHandleUnloadingOfCurrentWorld();
#endif
				}

#ifdef JA2DEMO
				if ( gbVictorySongCount == 1 && guiCurrentScreen == GAME_SCREEN )
				{
						// Bring up dialogue...
						HandleEndDemoInCreatureLevel( );

						SetMusicMode( MUSIC_TACTICAL_NOTHING );
				}
#else
				if ( gbVictorySongCount == 1 )
				{
						SetMusicMode( MUSIC_TACTICAL_NOTHING );
				}
#endif
			}
			else
			{
        if ( !gfDontRestartSong )
        {
  				StartMusicBasedOnMode( );
	  		}
			}

			gfMusicEnded = FALSE;
      gfDontRestartSong = FALSE;
		}
	}
#endif

	return(TRUE);
}


BOOLEAN SetMusicMode( UINT8 ubMusicMode )
{
	static INT8 bPreviousMode = 0;


	// OK, check if we want to restore
	if ( ubMusicMode == MUSIC_RESTORE )
	{
    if ( bPreviousMode == MUSIC_TACTICAL_VICTORY || bPreviousMode == MUSIC_TACTICAL_DEATH )
    {
      bPreviousMode = MUSIC_TACTICAL_NOTHING;
    }

		ubMusicMode = bPreviousMode;
	}
	else
	{
		// Save previous mode...
		bPreviousMode = gubOldMusicMode;
	}

#ifdef JA2DEMO
	// ATE: Short circuit normal music modes and
	// If we a\were told to play tense mode,
	// change to nothing until we have set
	// the flag indicating that we can....
	if ( ubMusicMode == MUSIC_TACTICAL_ENEMYPRESENT && !gfForceMusicToTense )
	{
		ubMusicMode = MUSIC_TACTICAL_NOTHING;
	}

	gfForceMusicToTense = FALSE;

#endif

	// if different, start a new music song
	if ( gubOldMusicMode != ubMusicMode )
	{
		// Set mode....
		gubMusicMode = ubMusicMode;

		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music New Mode %d %d", uiMusicHandle, gubMusicMode  ) );

		gbVictorySongCount = 0;
		gbDeathSongCount = 0;

		if(uiMusicHandle!=NO_SAMPLE  )
		{
			// Fade out old music
			MusicFadeOut( );
		}
		else
		{
			// Change music!
			StartMusicBasedOnMode( );
		}

	}
	gubOldMusicMode = gubMusicMode;

	return( TRUE );
}


BOOLEAN StartMusicBasedOnMode( )
{
	static BOOLEAN fFirstTime = TRUE;

	if ( fFirstTime )
	{
		fFirstTime = FALSE;

		bNothingModeSong = NOTHING_A_MUSIC + (INT8)Random( 4 );

		bEnemyModeSong = TENSOR_A_MUSIC + (INT8)Random( 3 );

		bBattleModeSong = BATTLE_A_MUSIC + (INT8)Random( 2 );

	}


	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "StartMusicBasedOnMode() %d %d", uiMusicHandle, gubMusicMode ) );

	// Setup a song based on mode we're in!
	switch( gubMusicMode )
	{
		case MUSIC_MAIN_MENU:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay( MENUMIX_MUSIC );
			break;

		case MUSIC_LAPTOP:
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay( MARIMBAD2_MUSIC );
			break;

		case MUSIC_TACTICAL_NOTHING:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			if( gfUseCreatureMusic )
			{
				MusicPlay( CREEPY_MUSIC );
			}
			else
			{
				MusicPlay( bNothingModeSong );
				bNothingModeSong = NOTHING_A_MUSIC + (INT8)Random( 4 );
			}
			break;

		case MUSIC_TACTICAL_ENEMYPRESENT:
			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			if( gfUseCreatureMusic )
			{
				MusicPlay( CREEPY_MUSIC );
			}
			else
			{
				MusicPlay( bEnemyModeSong );
				bEnemyModeSong = TENSOR_A_MUSIC + (INT8)Random( 3 );
			}
			break;

		case MUSIC_TACTICAL_BATTLE:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			if( gfUseCreatureMusic )
			{
				MusicPlay( CREATURE_BATTLE_MUSIC );
			}
			else
			{
				MusicPlay( bBattleModeSong );
			}
			bBattleModeSong = BATTLE_A_MUSIC + (INT8)Random( 2 );
			break;

		case MUSIC_TACTICAL_VICTORY:

			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay( TRIUMPH_MUSIC );
			gbVictorySongCount++;

			if( gfUseCreatureMusic && !gbWorldSectorZ )
			{ //We just killed all the creatures that just attacked the town.
				gfUseCreatureMusic = FALSE;
			}
			break;

		case MUSIC_TACTICAL_DEATH:

			// ATE: Don't fade in EnemyPresent...
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay( DEATH_MUSIC );
			gbDeathSongCount++;
			break;

		default:
			MusicFadeOut( );
			break;
	}

	return( TRUE );
}


void MusicStopCallback( void *pData )
{
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music EndCallback %d %d", uiMusicHandle, gubMusicMode  ) );

	gfMusicEnded  = TRUE;
	uiMusicHandle = NO_SAMPLE;
}


void SetMusicFadeSpeed( INT8 bFadeSpeed )
{
	gbFadeSpeed = bFadeSpeed;
}

void FadeMusicForXSeconds( UINT32 uiDelay )
{
	INT16 sNumTimeSteps, sNumVolumeSteps;

	// get # time steps in delay....
	sNumTimeSteps = (INT16)( uiDelay / 10 );

	// Devide this by music volume...
	sNumVolumeSteps = (INT16)( uiMusicVolume / sNumTimeSteps );

	// Set fade delay...
	SetMusicFadeSpeed( (INT8)sNumVolumeSteps );
}


void	DoneFadeOutDueToEndMusic( void )
{
	// Quit game....
	InternalLeaveTacticalScreen( MAINMENU_SCREEN );
	//SetPendingNewScreen( MAINMENU_SCREEN );
}

