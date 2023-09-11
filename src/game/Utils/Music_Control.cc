#include "SoundMan.h"
#include "Types.h"
#include "Music_Control.h"
#include "JAScreens.h"
#include "Creature_Spreading.h"
#include "Timer_Control.h"
#include "StrategicMap.h"
#include "ContentMusic.h"
#include "ScreenIDs.h"
#include "Logger.h"

#include "ContentManager.h"
#include "GameInstance.h"

static UINT32  uiMusicHandle   = NO_SAMPLE;
static UINT32  uiMusicVolume   = 50;
static BOOLEAN fMusicPlaying   = FALSE;
static BOOLEAN fMusicFadingOut = FALSE;
static BOOLEAN fMusicFadingIn  = FALSE;

static BOOLEAN gfMusicEnded = FALSE;

MusicMode        gubMusicMode    = MUSIC_NONE;
static MusicMode gubOldMusicMode = MUSIC_NONE;

static INT8 gbVictorySongCount = 0;
static INT8 gbDeathSongCount   = 0;

static INT8 gbFadeSpeed = 1;


BOOLEAN        gfForceMusicToTense = FALSE;
static BOOLEAN gfDontRestartSong   = FALSE;


static BOOLEAN MusicFadeIn(void);
static void MusicStop(void);
static void MusicStopCallback(void* pData);


void MusicPlay(const ST::string* pFilename)
{
	MusicStop();

	uiMusicHandle = SoundPlay(pFilename->c_str(), 0, 64, 1, MusicStopCallback, NULL);

	if(uiMusicHandle!=SOUND_ERROR)
	{
		SLOGD("Music Play {} {}", uiMusicHandle, gubMusicMode);

		gfMusicEnded	= FALSE;
		fMusicPlaying	= TRUE;
		MusicFadeIn();
		return;
	}

	SLOGE("Music Play Error {} {}", uiMusicHandle, gubMusicMode);
}


static void StartMusicBasedOnMode(void);


void MusicSetVolume(UINT32 uiVolume)
{
	INT32 uiOldMusicVolume = uiMusicVolume;

	uiMusicVolume = std::min(uiVolume, UINT32(MAXVOLUME));

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
static void MusicStop(void)
{
	SLOGD("Music Stop {} {} {}", fMusicPlaying, uiMusicHandle, gubMusicMode);
	if(!fMusicPlaying)
	{
		return;
	}

	if(uiMusicHandle!=NO_SAMPLE)
	{
		SoundStop(uiMusicHandle);
		uiMusicHandle = NO_SAMPLE;
	}
	else if(!gfMusicEnded)
	{
		SLOGW("expected either music data or the end of the music (mode={}, handle={}, ended={})", gubMusicMode, uiMusicHandle, gfMusicEnded);
	}
	fMusicPlaying = FALSE;
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

void MusicPoll(void)
{
	INT32 iVol;

	SoundServiceStreams();
	SoundServiceRandom();

	// Handle Sound every sound overhead time....
	if ( COUNTERDONE( MUSICOVERHEAD )  )
	{
		if(fMusicFadingIn)
		{
			if(uiMusicHandle!=NO_SAMPLE)
			{
				iVol=SoundGetVolume(uiMusicHandle);
				iVol = std::min((INT32) uiMusicVolume, iVol + gbFadeSpeed);
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
				iVol = std::max(iVol - gbFadeSpeed, 0);

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
			SLOGD("Music End Loop {} {}", uiMusicHandle, gubMusicMode);

			// If we were in victory mode, change!
			if ( gbVictorySongCount == 1 || gbDeathSongCount == 1 )
			{
				if ( gbDeathSongCount == 1 && guiCurrentScreen == GAME_SCREEN )
				{
					CheckAndHandleUnloadingOfCurrentWorld();
				}

				if ( gbVictorySongCount == 1 )
				{
						SetMusicMode( MUSIC_TACTICAL_NOTHING );
				}
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


void SetMusicMode(MusicMode ubMusicMode)
{
	static MusicMode bPreviousMode = MUSIC_NONE;


	// OK, check if we want to restore
	if ( ubMusicMode == MUSIC_RESTORE )
	{
		if ( bPreviousMode == MUSIC_TACTICAL_VICTORY || bPreviousMode == MUSIC_TACTICAL_DEFEAT )
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

	// if different, start a new music song
	if ( gubOldMusicMode != ubMusicMode )
	{
		// Set mode....
		gubMusicMode = ubMusicMode;

		SLOGD("Music New Mode {} {}", uiMusicHandle, gubMusicMode);

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
	SLOGD("StartMusicBasedOnMode() {} {}", uiMusicHandle, gubMusicMode);
	MusicMode next = gubMusicMode;

	switch (gubMusicMode) {
		case MUSIC_TACTICAL_NOTHING:
			if (gfUseCreatureMusic) {
				next = MUSIC_TACTICAL_CREATURE_NOTHING;
			}
			break;
		case MUSIC_TACTICAL_ENEMYPRESENT:
			if (gfUseCreatureMusic) {
				next = MUSIC_TACTICAL_CREATURE_ENEMYPRESENT;
			}
			break;
		case MUSIC_TACTICAL_BATTLE:
			if (gfUseCreatureMusic) {
				next = MUSIC_TACTICAL_CREATURE_BATTLE;
			}
			break;
		default: // ignore other modes
			break;
	}

	switch (gubMusicMode) {
		case MUSIC_TACTICAL_VICTORY:
			gbVictorySongCount++;
			if (gfUseCreatureMusic && !gWorldSector.z) {
				//We just killed all the creatures that just attacked the town.
				gfUseCreatureMusic = FALSE;
			}
			break;
		case MUSIC_TACTICAL_DEFEAT:
			gbDeathSongCount++;
			break;
		default: // ignore other modes
			break;
	}

	switch (gubMusicMode) {
		case MUSIC_MAIN_MENU:
		case MUSIC_LAPTOP:
		case MUSIC_TACTICAL_NOTHING:
		case MUSIC_TACTICAL_ENEMYPRESENT:
		case MUSIC_TACTICAL_BATTLE:
		case MUSIC_TACTICAL_CREATURE_NOTHING:
		case MUSIC_TACTICAL_CREATURE_ENEMYPRESENT:
		case MUSIC_TACTICAL_CREATURE_BATTLE:
		case MUSIC_TACTICAL_VICTORY:
		case MUSIC_TACTICAL_DEFEAT:
			// ATE: Don't fade in
			gbFadeSpeed = (INT8)uiMusicVolume;
			MusicPlay( GCM->getMusicForMode(next) );
			break;
		default:
			MusicFadeOut();
	}
}


static void MusicStopCallback(void* pData)
{
	SLOGD("Music EndCallback {} {}", uiMusicHandle, gubMusicMode);

	gfMusicEnded  = TRUE;
	uiMusicHandle = NO_SAMPLE;
}


void SetMusicFadeSpeed( INT8 bFadeSpeed )
{
	gbFadeSpeed = bFadeSpeed;
}
