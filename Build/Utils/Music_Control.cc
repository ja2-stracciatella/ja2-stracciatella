#include "Directories.h"
#include "Types.h"
#include "Music_Control.h"
#include "Random.h"
#include "GameScreen.h"
#include "JAScreens.h"
#include "Creature_Spreading.h"
#include "Timer_Control.h"
#include "StrategicMap.h"
#include "Debug.h"
#include "ScreenIDs.h"

#ifdef JA2DEMO
#	include "Fade_Screen.h"
#	include "Overhead.h"
#endif


static UINT32  uiMusicHandle   = NO_SAMPLE;
static UINT32  uiMusicVolume   = 50;
static BOOLEAN fMusicPlaying   = FALSE;
static BOOLEAN fMusicFadingOut = FALSE;
static BOOLEAN fMusicFadingIn  = FALSE;

static BOOLEAN gfMusicEnded = FALSE;

UINT8        gubMusicMode    = 0;
static UINT8 gubOldMusicMode = 0;

static INT8 gbVictorySongCount = 0;
static INT8 gbDeathSongCount   = 0;

static INT8 bNothingModeSong;
static INT8 bEnemyModeSong;
static INT8 bBattleModeSong;

static INT8 gbFadeSpeed = 1;


const char* const szMusicList[]=
{
	MUSICDIR "/marimbad 2.wav",
	MUSICDIR "/menumix1.wav",
	MUSICDIR "/nothing A.wav",
	MUSICDIR "/nothing B.wav",
#ifdef JA2DEMO
	MUSICDIR "/nothing A.wav",
	MUSICDIR "/nothing B.wav",
	MUSICDIR "/tensor B.wav",
	MUSICDIR "/tensor B.wav",
	MUSICDIR "/tensor B.wav",
#else
	MUSICDIR "/nothing C.wav",
	MUSICDIR "/nothing D.wav",
	MUSICDIR "/tensor A.wav",
	MUSICDIR "/tensor B.wav",
	MUSICDIR "/tensor C.wav",
#endif
	MUSICDIR "/triumph.wav",
	MUSICDIR "/death.wav",
	MUSICDIR "/battle A.wav",
	MUSICDIR "/tensor B.wav",
	MUSICDIR "/creepy.wav",
	MUSICDIR "/creature battle.wav"
};


BOOLEAN	       gfForceMusicToTense = FALSE;
static BOOLEAN gfDontRestartSong   = FALSE;


static BOOLEAN MusicFadeIn(void);
static BOOLEAN MusicStop(void);
static void MusicStopCallback(void* pData);


void MusicPlay(UINT32 uiNum)
{
	if(fMusicPlaying)
		MusicStop();

	uiMusicHandle = SoundPlayStreamedFile(szMusicList[uiNum], 0, 64, 1, MusicStopCallback, NULL);

	if(uiMusicHandle!=SOUND_ERROR)
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music PLay %d %d", uiMusicHandle, gubMusicMode  ) );

		gfMusicEnded				= FALSE;
		fMusicPlaying=TRUE;
		MusicFadeIn();
		return;
	}

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music PLay %d %d", uiMusicHandle, gubMusicMode  ) );
}


static void StartMusicBasedOnMode(void);


void MusicSetVolume(UINT32 uiVolume)
{
  INT32 uiOldMusicVolume = uiMusicVolume;

	uiMusicVolume = __min(uiVolume, MAXVOLUME);

	if(uiMusicHandle!=NO_SAMPLE)
	{
    // get volume and if 0 stop music!
    if ( uiMusicVolume == 0 )
    {
      gfDontRestartSong = TRUE;
      MusicStop( );
    }
		else
		{
			SoundSetVolume(uiMusicHandle, uiMusicVolume);
		}
	}
	else
	{
		// If here, check if we need to re-start music
		// Have we re-started?
		if ( uiMusicVolume > 0 && uiOldMusicVolume == 0 )
		{
			StartMusicBasedOnMode( );
		}
	}
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


//		Stops the currently playing music.
//
//	Returns:	TRUE if the music was stopped, FALSE if an error occurred
static BOOLEAN MusicStop(void)
{
	if(uiMusicHandle!=NO_SAMPLE)
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music Stop %d %d", uiMusicHandle, gubMusicMode ) );

		SoundStop(uiMusicHandle);
		fMusicPlaying=FALSE;
		uiMusicHandle = NO_SAMPLE;
		return(TRUE);
	}

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music Stop %d %d", uiMusicHandle, gubMusicMode ) );

	return(FALSE);
}


//		Fades out the current song.
//
//	Returns:	TRUE if the music has begun fading, FALSE if an error occurred
static BOOLEAN MusicFadeOut(void)
{
	if(uiMusicHandle!=NO_SAMPLE)
	{
		fMusicFadingOut=TRUE;
		return(TRUE);
	}
	return(FALSE);
}


//		Fades in the current song.
//
//	Returns:	TRUE if the music has begun fading in, FALSE if an error occurred
static BOOLEAN MusicFadeIn(void)
{
	if(uiMusicHandle!=NO_SAMPLE)
	{
		fMusicFadingIn=TRUE;
		return(TRUE);
	}
	return(FALSE);
}


static void DoneFadeOutDueToEndMusic(void);


void MusicPoll(void)
{
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
}


void SetMusicMode(UINT8 ubMusicMode)
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
}


static void StartMusicBasedOnMode(void)
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
}


static void MusicStopCallback(void* pData)
{
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Music EndCallback %d %d", uiMusicHandle, gubMusicMode  ) );

	gfMusicEnded  = TRUE;
	uiMusicHandle = NO_SAMPLE;
}


void SetMusicFadeSpeed( INT8 bFadeSpeed )
{
	gbFadeSpeed = bFadeSpeed;
}


static void DoneFadeOutDueToEndMusic(void)
{
	// Quit game....
	InternalLeaveTacticalScreen( MAINMENU_SCREEN );
	//SetPendingNewScreen( MAINMENU_SCREEN );
}
