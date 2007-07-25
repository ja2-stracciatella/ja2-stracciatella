/*********************************************************************************
* SGP Digital Sound Module
*
*		This module handles the playing of digital samples, preloaded or streamed.
*
* Derek Beland, May 28, 1997
*********************************************************************************/

#include "Debug.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "Random.h"
#include "SoundMan.h"
#include "Timer.h"
#include <SDL.h>
#include <assert.h>


// Uncomment this to disable the startup of sound hardware
//#define SOUND_DISABLE


#ifdef WITH_SOUND_DEBUG
#	define SNDDBG(fmt, ...) fprintf(stderr, ">>>> SND: " fmt, __VA_ARGS__);
#else
#	define SNDDBG(fmt, ...) (void)0;
#endif


/*
 * from\to FREE PLAY STOP DEAD
 *    FREE       M
 *    PLAY  2         M    C
 *    STOP  2              C
 *    DEAD  M         1
 *
 * M = Regular state transition done by main thread
 * C = Regular state transition done by sound callback
 * 1 = Unimportant race, dead channel can be marked stopped by main thread
 *     Gets marked as dead again in the next sound callback run
 * 2 = Only when stopping all sounds, sound callback is deactivated when this
 *     happens
 */
enum
{
	CHANNEL_FREE,
	CHANNEL_PLAY,
	CHANNEL_STOP,
	CHANNEL_DEAD
};


// global settings
#define		SOUND_MAX_CACHED			128						// number of cache slots

#ifdef JA2
#define		SOUND_MAX_CHANNELS		16						// number of mixer channels
#else
#define		SOUND_MAX_CHANNELS		32						// number of mixer channels
#endif


#define SOUND_DEFAULT_MEMORY (16 * 1024 * 1024) // default memory limit
#define		SOUND_DEFAULT_THRESH	(256*8024)		// size for sample to be double-buffered
#define		SOUND_DEFAULT_STREAM	(64*1024)			// double-buffered buffer size

// playing/random value to indicate default
#define		SOUND_PARMS_DEFAULT		0xffffffff


// Struct definition for sample slots in the cache
// Holds the regular sample data, as well as the data for the random samples
typedef struct
{
	CHAR8   pName[128];  // Path to sample data
	UINT32  uiSize;      // Size of sample data
	UINT32  uiSoundSize; // Playable sound size
	UINT32  uiFlags;     // Status flags
	UINT32  uiSpeed;     // Playback frequency
	BOOLEAN fStereo;     // Stereo/Mono
	UINT8   ubBits;      // 8/16 bits
	PTR     pData;       // pointer to sample data memory
	UINT32  uiCacheHits;

	// Random sound data
	UINT32  uiTimeNext;
	UINT32  uiTimeMin, uiTimeMax;
	UINT32  uiVolMin, uiVolMax;
	UINT32  uiPanMin, uiPanMax;
	UINT32  uiInstances;
	UINT32  uiMaxInstances;
} SAMPLETAG;


// Structure definition for slots in the sound output
// These are used for both the cached and double-buffered streams
typedef struct
{
	volatile UINT State;
	SAMPLETAG* pSample;
	UINT32     uiSoundID;
	void       (*EOSCallback)(void*);
	void*      pCallbackData;
	UINT32     uiTimeStamp;
	HWFILE     hFile;
	UINT32     uiFadeVolume;
	UINT32     uiFadeRate;
	UINT32     uiFadeTime;
	UINT32 Pos;
	UINT32 Loops;
	UINT32 Pan;
} SOUNDTAG;


// Local Function Prototypes
static BOOLEAN SoundInitCache(void);
static BOOLEAN SoundShutdownCache(void);
static UINT32 SoundLoadSample(const char* pFilename);
static UINT32 SoundGetCached(const char* pFilename);
static UINT32 SoundLoadDisk(const char* pFilename);

// Low level
static UINT32 SoundGetEmptySample(void);
static UINT32 SoundFreeSampleIndex(UINT32 uiSample);
static UINT32 SoundGetIndexByID(UINT32 uiSoundID);
static BOOLEAN SoundInitHardware(void);
static BOOLEAN SoundShutdownHardware(void);
static UINT32 SoundGetFreeChannel(void);
static UINT32 SoundStartSample(UINT32 uiSample, UINT32 uiChannel, const SOUNDPARMS* pParms);
static UINT32 SoundStartStream(const char* pFilename, UINT32 uiChannel, const SOUNDPARMS* pParms);
static UINT32 SoundGetUniqueID(void);
static BOOLEAN SoundPlayStreamed(const char* pFilename);
static BOOLEAN SoundCleanCache(void);
static BOOLEAN SoundSampleIsPlaying(UINT32 uiSample);
static BOOLEAN SoundIndexIsPlaying(UINT32 uiSound);
static BOOLEAN SoundStopIndex(UINT32 uiSound);
static UINT32 SoundGetVolumeIndex(UINT32 uiChannel);
static BOOLEAN SoundSetVolumeIndex(UINT32 uiChannel, UINT32 uiVolume);

// Global variables
static const UINT32 guiSoundDefaultVolume = 127;
static UINT32 guiSoundMemoryLimit    = SOUND_DEFAULT_MEMORY; // Maximum memory used for sounds
static UINT32 guiSoundMemoryUsed     = 0;                    // Memory currently in use
static UINT32 guiSoundCacheThreshold = SOUND_DEFAULT_THRESH; // Double-buffered threshold

// Local module variables
static BOOLEAN fSoundSystemInit = FALSE; // Startup called T/F
static BOOLEAN gfEnableStartup  = TRUE;  // Allow hardware to starup

// Sample cache list for files loaded
static SAMPLETAG pSampleList[SOUND_MAX_CACHED];
// Sound channel list for output channels
static SOUNDTAG pSoundList[SOUND_MAX_CHANNELS];


//*******************************************************************************
// SoundEnableSound
//
//	Allows or disallows the startup of the sound hardware.
//
//	Returns:	Nothing.
//
//*******************************************************************************
void SoundEnableSound(BOOLEAN fEnable)
{
	gfEnableStartup=fEnable;
}


//*******************************************************************************
// InitializeSoundManager
//
//	Zeros out the structs for the system info, and initializes the cache.
//
//	Returns:	TRUE always
//
//*******************************************************************************
BOOLEAN InitializeSoundManager(void)
{
	if (fSoundSystemInit) ShutdownSoundManager();

	memset(pSoundList, 0, sizeof(pSoundList));

#ifndef SOUND_DISABLE
	if (gfEnableStartup && SoundInitHardware()) fSoundSystemInit = TRUE;
#endif

	SoundInitCache();

	guiSoundMemoryLimit    = SOUND_DEFAULT_MEMORY;
	guiSoundMemoryUsed     = 0;
	guiSoundCacheThreshold = SOUND_DEFAULT_THRESH;

	return TRUE;
}

//*******************************************************************************
// ShutdownSoundManager
//
//		Silences all currently playing sound, deallocates any memory allocated,
//	and releases the sound hardware.
//
//*******************************************************************************
void ShutdownSoundManager(void)
{
	SoundStopAll();
	SoundShutdownCache();
	SoundShutdownHardware();
	fSoundSystemInit=FALSE;
}

//*******************************************************************************
// SoundPlay
//
//		Starts a sample playing. If the sample is not loaded in the cache, it will
//	be found and loaded. The pParms structure is used to
//	override the attributes of the sample such as playback speed, and to specify
//	a volume. Any entry containing SOUND_PARMS_DEFAULT will be set by the system.
//
//	Returns:	If the sound was started, it returns a sound ID unique to that
//						instance of the sound
//						If an error occured, SOUND_ERROR will be returned
//
//
//	!!Note:  Can no longer play streamed files
//
//*******************************************************************************

UINT32 SoundPlay(const char *pFilename, SOUNDPARMS *pParms)
{
	if (!fSoundSystemInit) return SOUND_ERROR;

#if 0 // TODO0003 implement streaming
	if (SoundPlayStreamed(pFilename))
	{
		//Trying to play a sound which is bigger then the 'guiSoundCacheThreshold'

		// This line was causing a page fault in the Wiz 8 project, so
		// I changed it to the second line, which works OK. -- DB

		//DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("\n*******\nSoundPlay():  ERROR:  trying to play %s which is bigger then the 'guiSoundCacheThreshold', use SoundPlayStreamedFile() instead\n", pFilename));

		FastDebugMsg(String("SoundPlay: ERROR: Trying to play %s sound is too lardge to load into cache, use SoundPlayStreamedFile() instead\n", pFilename));
		return SOUND_ERROR;
	}
#endif

	UINT32 uiSample = SoundLoadSample(pFilename);
	if (uiSample == NO_SAMPLE) return SOUND_ERROR;

	UINT32 uiChannel = SoundGetFreeChannel();
	if (uiChannel == SOUND_ERROR) return SOUND_ERROR;

	return SoundStartSample(uiSample, uiChannel, pParms);
}


//*******************************************************************************
// SoundPlayStreamedFile
//
//		The sample will
//	be played as a double-buffered sample. The pParms structure is used to
//	override the attributes of the sample such as playback speed, and to specify
//	a volume. Any entry containing SOUND_PARMS_DEFAULT will be set by the system.
//
//	Returns:	If the sound was started, it returns a sound ID unique to that
//						instance of the sound
//						If an error occured, SOUND_ERROR will be returned
//
//*******************************************************************************
UINT32	SoundPlayStreamedFile(const char *pFilename, SOUNDPARMS *pParms )
{
#if 1
	// TODO0003 implement streaming
	return SoundPlay(pFilename, pParms);
#else
	if (!fSoundSystemInit) return SOUND_ERROR;

	UINT32 uiChannel = SoundGetFreeChannel();
	if (uiChannel == SOUND_ERROR) return SOUND_ERROR;

	//Open the file
	HWFILE hFile = FileOpen(pFilename, FILE_ACCESS_READ | FILE_OPEN_EXISTING);
	if (!hFile)
	{
		FastDebugMsg(String("\n*******\nSoundPlayStreamedFile():  ERROR:  Couldnt open '%s' in SoundPlayStreamedFile()\n", pFilename ) );
		return SOUND_ERROR;
	}

	// MSS cannot determine which provider to play if you don't give it a real filename
	// so if the file isn't in a library, play it normally
	if (DB_EXTRACT_LIBRARY(hFile) == REAL_FILE_LIBRARY_ID)
	{
		FileClose(hFile);
		return SoundStartStream(pFilename, uiChannel, pParms);
	}

	//Get the real file handle of the file
	FILE* hRealFileHandle = GetRealFileHandleFromFileManFileHandle(hFile);
	if (hRealFileHandle == NULL)
	{
		FastDebugMsg(String("\n*******\nSoundPlayStreamedFile():  ERROR:  Couldnt get a real file handle for '%s' in SoundPlayStreamedFile()\n", pFilename ) );
		return SOUND_ERROR;
	}

	//Convert the file handle into a 'name'
	char pFileHandlefileName[128];
	sprintf(pFileHandlefileName, "\\\\\\\\%d", hRealFileHandle);

	//Start the sound stream
	UINT32 uiRetVal = SoundStartStream( pFileHandlefileName, uiChannel, pParms);

	//if it succeeded, record the file handle
	if (uiRetVal != SOUND_ERROR)
	{
		pSoundList[uiChannel].hFile = hFile;
	}
	else
	{
		FileClose(hFile);
	}

	return uiRetVal;
#endif
}

//*******************************************************************************
// SoundPlayRandom
//
//		Registers a sample to be played randomly within the specified parameters.
//	Parameters are passed in through pParms. Any parameter containing
//	SOUND_PARMS_DEFAULT will be set by the system. Only the uiTimeMin entry may
//	NOT be defaulted.
//
//	* Samples designated "random" are ALWAYS loaded into the cache, and locked
//	in place. They are never double-buffered, and this call will fail if they
//	cannot be loaded. *
//
//	Returns:	If successful, it returns the sample index it is loaded to, else
//						SOUND_ERROR is returned.
//
//*******************************************************************************
UINT32 SoundPlayRandom(const char* pFilename, const RANDOMPARMS* pParms)
{
	SNDDBG("RAND \"%s\"\n", pFilename);

	if (!fSoundSystemInit) return SOUND_ERROR;

	UINT32 uiSample = SoundLoadSample(pFilename);
	if (uiSample == NO_SAMPLE) return SOUND_ERROR;

	SAMPLETAG* Sample = &pSampleList[uiSample];

	Sample->uiFlags |= SAMPLE_RANDOM | SAMPLE_LOCKED;

	if (pParms->uiTimeMin == SOUND_PARMS_DEFAULT)
		return SOUND_ERROR;
	else
		Sample->uiTimeMin = pParms->uiTimeMin;

	if (pParms->uiTimeMax == SOUND_PARMS_DEFAULT)
		Sample->uiTimeMax = pParms->uiTimeMin;
	else
		Sample->uiTimeMax = pParms->uiTimeMax;

	if (pParms->uiVolMin == SOUND_PARMS_DEFAULT)
		Sample->uiVolMin = guiSoundDefaultVolume;
	else
		Sample->uiVolMin = pParms->uiVolMin;

	if (pParms->uiVolMax == SOUND_PARMS_DEFAULT)
		Sample->uiVolMax = guiSoundDefaultVolume;
	else
		Sample->uiVolMax = pParms->uiVolMax;

	if (pParms->uiPanMin == SOUND_PARMS_DEFAULT)
		Sample->uiPanMin = 64;
	else
		Sample->uiPanMin = pParms->uiPanMin;

	if (pParms->uiPanMax == SOUND_PARMS_DEFAULT)
		Sample->uiPanMax = 64;
	else
		Sample->uiPanMax = pParms->uiPanMax;

	if (pParms->uiMaxInstances == SOUND_PARMS_DEFAULT)
		Sample->uiMaxInstances = 1;
	else
		Sample->uiMaxInstances = pParms->uiMaxInstances;

	Sample->uiTimeNext =
		GetClock() +
		Sample->uiTimeMin +
		Random(Sample->uiTimeMax - Sample->uiTimeMin);

	return uiSample;
}


//*******************************************************************************
// SoundIsPlaying
//
//		Returns TRUE/FALSE that an instance of a sound is still playing.
//
//*******************************************************************************
BOOLEAN SoundIsPlaying(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
	return uiSound != NO_SAMPLE && SoundIndexIsPlaying(uiSound);
}


//*****************************************************************************************
// SoundIndexIsPlaying
//
// Returns TRUE/FALSE whether a sound channel's sample is currently playing.
//
// Returns BOOLEAN            - TRUE = playing, FALSE = stopped or nothing allocated
//
// UINT32 uiSound             - Channel number of sound
//
// Created:  2/24/00 Derek Beland
//*****************************************************************************************
static BOOLEAN SoundIndexIsPlaying(UINT32 uiSound)
{
	return
		fSoundSystemInit &&
		pSoundList[uiSound].State != CHANNEL_FREE;
}

//*******************************************************************************
// SoundStop
//
//		Stops the playing of a sound instance, if still playing.
//
//	Returns:	TRUE if the sample was actually stopped, FALSE if it could not be
//						found, or was not playing.
//
//*******************************************************************************
BOOLEAN SoundStop(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;
	if (!SoundIsPlaying(uiSoundID)) return FALSE;

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
	if (uiSound == NO_SAMPLE) return FALSE;

	SoundStopIndex(uiSound);
	return TRUE;
}


//*******************************************************************************
// SoundStopAll
//
//		Stops all currently playing sounds.
//
//	Returns:	TRUE, always
//
//*******************************************************************************
BOOLEAN SoundStopAll(void)
{
	if (!fSoundSystemInit) return TRUE;

	SDL_PauseAudio(1);
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		if (SoundStopIndex(uiCount))
		{
			SOUNDTAG* Sound = &pSoundList[uiCount];
			assert(Sound->pSample->uiInstances != 0);
			--Sound->pSample->uiInstances;
			Sound->pSample   = NULL;
			Sound->uiSoundID = SOUND_ERROR;
			Sound->State = CHANNEL_FREE;
		}
	}
	SDL_PauseAudio(0);

	return TRUE;
}


//*******************************************************************************
// SoundSetVolume
//
//		Sets the volume on a currently playing sound.
//
//	Returns:	TRUE if the volume was actually set on the sample, FALSE if the
//						sample had already expired or couldn't be found
//
//*******************************************************************************
BOOLEAN SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume)
{
	if (!fSoundSystemInit) return FALSE;

	UINT32 uiVolCap = __min(uiVolume, 127);

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
	if (uiSound == NO_SAMPLE) return FALSE;

	pSoundList[uiSound].uiFadeVolume = uiVolume;
	return SoundSetVolumeIndex(uiSound, uiVolume);
}


//*****************************************************************************************
// SoundSetVolumeIndex
//
// Sounds the volume on a sound channel.
//
// Returns BOOLEAN            - TRUE if the volume was set
//
// UINT32 uiChannel           - Sound channel
// UINT32 uiVolume            - New volume 0-127
//
// Created:  3/17/00 Derek Beland
//*****************************************************************************************
static BOOLEAN SoundSetVolumeIndex(UINT32 uiChannel, UINT32 uiVolume)
{
	if (!fSoundSystemInit) return FALSE;

	pSoundList[uiChannel].uiFadeVolume = __min(uiVolume, 127);
	return TRUE;
}

//*******************************************************************************
// SoundSetPan
//
//		Sets the pan on a currently playing sound.
//
//	Returns:	TRUE if the pan was actually set on the sample, FALSE if the
//						sample had already expired or couldn't be found
//
//*******************************************************************************
BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan)
{
	if (!fSoundSystemInit) return FALSE;

	UINT32 uiPanCap = __min(uiPan, 127);

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
	if (uiSound == NO_SAMPLE) return FALSE;

	pSoundList[uiSound].Pan = uiPanCap;

	return TRUE;
}


//*******************************************************************************
// SoundGetVolume
//
//		Returns the current volume setting of a sound that is playing. If the sound
//	has expired, or could not be found, SOUND_ERROR is returned.
//
//*******************************************************************************
UINT32 SoundGetVolume(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return SOUND_ERROR;

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
	if (uiSound == NO_SAMPLE) SOUND_ERROR;

	return SoundGetVolumeIndex(uiSound);
}


//*****************************************************************************************
// SoundGetVolumeIndex
//
// Returns the current volume of a sound channel.
//
// Returns UINT32             - Volume 0-127
//
// UINT32 uiChannel           - Channel
//
// Created:  3/17/00 Derek Beland
//*****************************************************************************************
static UINT32 SoundGetVolumeIndex(UINT32 uiChannel)
{
	if (!fSoundSystemInit) return SOUND_ERROR;

	return pSoundList[uiChannel].uiFadeVolume;
}


static BOOLEAN SoundRandomShouldPlay(UINT32 uiSample);
static UINT32 SoundStartRandom(UINT32 uiSample);


//*******************************************************************************
// SoundServiceRandom
//
//		This function should be polled by the application if random samples are
//	used. The time marks on each are checked and if it is time to spawn a new
//	instance of the sound, the number already in existance are checked, and if
//	there is room, a new one is made and the count updated.
//		If random samples are not being used, there is no purpose in polling this
//	function.
//
//	Returns:	TRUE if a new random sound was created, FALSE if nothing was done.
//
//*******************************************************************************
BOOLEAN SoundServiceRandom(void)
{
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
	{
		if (SoundRandomShouldPlay(uiCount)) SoundStartRandom(uiCount);
	}

	return FALSE;
}

//*******************************************************************************
// SoundRandomShouldPlay
//
//	Determines whether a random sound is ready for playing or not.
//
//	Returns:	TRUE if a the sample should be played.
//
//*******************************************************************************
static BOOLEAN SoundRandomShouldPlay(UINT32 uiSample)
{
	SAMPLETAG* Sample = &pSampleList[uiSample];
	return
		Sample->uiFlags & SAMPLE_RANDOM &&
		Sample->uiTimeNext <= GetClock() &&
		Sample->uiInstances < Sample->uiMaxInstances;
}

//*******************************************************************************
// SoundStartRandom
//
//	Starts an instance of a random sample.
//
//	Returns:	TRUE if a new random sound was created, FALSE if nothing was done.
//
//*******************************************************************************
static UINT32 SoundStartRandom(UINT32 uiSample)
{
	UINT32 uiChannel = SoundGetFreeChannel();
	if (uiChannel == SOUND_ERROR) return NO_SAMPLE;

	SAMPLETAG* Sample = &pSampleList[uiSample];

	SOUNDPARMS spParms;
	memset(&spParms, 0xff, sizeof(SOUNDPARMS));
	spParms.uiVolume   = Sample->uiVolMin + Random(Sample->uiVolMax - Sample->uiVolMin);
	spParms.uiPan      = Sample->uiPanMin + Random(Sample->uiPanMax - Sample->uiPanMin);
	spParms.uiLoop     = 1;

	UINT32 uiSoundID = SoundStartSample(uiSample, uiChannel, &spParms);
	if (uiSoundID == SOUND_ERROR) return NO_SAMPLE;

	Sample->uiTimeNext =
		GetClock() +
		Sample->uiTimeMin +
		Random(Sample->uiTimeMax - Sample->uiTimeMin);
	return uiSoundID;
}

//*******************************************************************************
// SoundStopAllRandom
//
//		This function should be polled by the application if random samples are
//	used. The time marks on each are checked and if it is time to spawn a new
//	instance of the sound, the number already in existance are checked, and if
//	there is room, a new one is made and the count updated.
//		If random samples are not being used, there is no purpose in polling this
//	function.
//
//	Returns:	TRUE if a new random sound was created, FALSE if nothing was done.
//
//*******************************************************************************
BOOLEAN SoundStopAllRandom(void)
{
	// Stop all currently playing random sounds
	for (UINT32 uiChannel = 0; uiChannel < SOUND_MAX_CHANNELS; uiChannel++)
	{
		SOUNDTAG* Sound = &pSoundList[uiChannel];

		if (Sound->State == CHANNEL_PLAY)
		{
			// if this was a random sample, decrease the iteration count
			if (Sound->pSample->uiFlags & SAMPLE_RANDOM)
			{
				SoundStopIndex(uiChannel);
			}
		}
	}

	// Unlock all random sounds so they can be dumped from the cache, and
	// take the random flag off so they won't be serviced/played
	for (UINT32 uiSample = 0; uiSample < SOUND_MAX_CACHED; uiSample++)
	{
		SAMPLETAG* Sample = &pSampleList[uiSample];
		if (Sample->uiFlags & SAMPLE_RANDOM)
		{
			Sample->uiFlags &= ~(SAMPLE_RANDOM | SAMPLE_LOCKED);
		}
	}

	return FALSE;
}

//*******************************************************************************
// SoundServiceStreams
//
//		Can be polled in tight loops where sound buffers might starve due to heavy
//	hardware use, etc. Streams DO NOT normally need to be serviced manually, but
//	in some cases (heavy file loading) it might be desirable.
//
//		If you are using the end of sample callbacks, you must call this function
//	periodically to check the sample's status.
//
//	Returns:	TRUE always.
//
//*******************************************************************************
BOOLEAN SoundServiceStreams(void)
{
	if (!fSoundSystemInit) return TRUE;

	for (UINT32 i = 0; i < lengthof(pSoundList); i++)
	{
		SOUNDTAG* Sound = &pSoundList[i];
		if (Sound->State == CHANNEL_DEAD)
		{
			SNDDBG("DEAD channel %u file \"%s\" (refcount %u)\n", i, Sound->pSample->pName, Sound->pSample->uiInstances);
			if (Sound->EOSCallback != NULL) Sound->EOSCallback(Sound->pCallbackData);
			assert(Sound->pSample->uiInstances != 0);
			Sound->pSample->uiInstances--;
			Sound->pSample   = NULL;
			Sound->uiSoundID = SOUND_ERROR;
			Sound->State     = CHANNEL_FREE;
		}
	}

	return TRUE;
}


//*******************************************************************************
// SoundGetPosition
//
//	Reports the current time position of the sample.
//
//	Note: You should be checking SoundIsPlaying very carefully while
//	calling this function.
//
//	Returns:	The current time of the sample in milliseconds.
//
//*******************************************************************************
UINT32 SoundGetPosition(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return 0;

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
#if 0
	UINT32 uiPosition = 0;
	UINT32 uiFreq = 0;
	UINT32 uiFormat = 0;
	if (uiSound != NO_SAMPLE)
	{
		SOUNDTAG* Sound = &pSoundList[uiSound];

		if (Sound->hMSSStream != NULL)
		{
			uiPosition = (UINT32)AIL_stream_position(Sound->hMSSStream);
			uiFreq     = (UINT32)Sound->hMSSStream->samp->playback_rate;
			uiFormat   = (UINT32)Sound->hMSSStream->samp->format;

		}
		else if (Sound->hMSS != NULL)
		{
			uiPosition = (UINT32)AIL_sample_position(Sound->hMSS);
			uiFreq     = (UINT32)Sound->hMSS->playback_rate;
			uiFormat   = (UINT32)Sound->hMSS->format;
		}
	}

	UINT32 uiBytesPerSample = 0;
	switch (uiFormat)
	{
		case DIG_F_MONO_8:		uiBytesPerSample = 1; break;
		case DIG_F_MONO_16:		uiBytesPerSample = 2; break;
		case DIG_F_STEREO_8:	uiBytesPerSample = 2; break;
		case DIG_F_STEREO_16:	uiBytesPerSample = 4; break;
	}

	if (uiFreq == 0) return 0;

	return uiPosition / uiBytesPerSample / (uiFreq / 1000);
#else
	if (uiSound == NO_SAMPLE) return 0;

	SOUNDTAG* Sound = &pSoundList[uiSound];

	UINT32 uiTime = GetClock();
	// check for rollover
	UINT32 uiPosition;
	if (uiTime < Sound->uiTimeStamp)
	{
		uiPosition = 0 - Sound->uiTimeStamp + uiTime;
	}
	else
	{
		uiPosition = uiTime - Sound->uiTimeStamp;
	}

	return uiPosition;
#endif
}


//*******************************************************************************
// SoundInitCache
//
//		Zeros out the structures of the sample list.
//
//*******************************************************************************
static BOOLEAN SoundInitCache(void)
{
	memset(pSampleList, 0, sizeof(pSampleList));
	return TRUE;
}


static BOOLEAN SoundEmptyCache(void);


//*******************************************************************************
// SoundShutdownCache
//
//		Empties out the cache.
//
//	Returns: TRUE, always
//
//*******************************************************************************
static BOOLEAN SoundShutdownCache(void)
{
	SoundEmptyCache();
	return TRUE;
}


//*******************************************************************************
// SoundEmptyCache
//
//		Frees up all samples in the cache.
//
//	Returns: TRUE, always
//
//*******************************************************************************
static BOOLEAN SoundEmptyCache(void)
{
	SoundStopAll();

	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
	{
		SoundFreeSampleIndex(uiCount);
	}

	return TRUE;
}


//*******************************************************************************
// SoundLoadSample
//
//		Frees up all samples in the cache.
//
//	Returns: TRUE, always
//
//*******************************************************************************
static UINT32 SoundLoadSample(const char* pFilename)
{
	UINT32 uiSample = SoundGetCached(pFilename);
	if (uiSample != NO_SAMPLE) return uiSample;

	return SoundLoadDisk(pFilename);
}


//*******************************************************************************
// SoundGetCached
//
//		Tries to locate a sound by looking at what is currently loaded in the
//	cache.
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
static UINT32 SoundGetCached(const char* pFilename)
{
	if (pFilename[0] == '\0') return SOUND_ERROR; // XXX HACK0009

	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
	{
		if (strcasecmp(pSampleList[uiCount].pName, pFilename) == 0) return uiCount;
	}

	return NO_SAMPLE;
}


#define FOURCC(a, b, c, d) ((UINT8)(d) << 24 | (UINT8)(c) << 16 | (UINT8)(b) << 8 | (UINT8)(a))


enum WaveFormatTag
{
	WAVE_FORMAT_UNKNOWN   = 0x0000,
	WAVE_FORMAT_PCM       = 0x0001,
	WAVE_FORMAT_DVI_ADPCM = 0x0011
};


static inline int Clamp(int min, int x, int max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}


//*******************************************************************************
// SoundLoadDisk
//
//		Loads a sound file from disk into the cache, allocating memory and a slot
//	for storage.
//
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
static UINT32 SoundLoadDisk(const char* pFilename)
{
	Assert(pFilename != NULL);

	HWFILE hFile = FileOpen(pFilename, FILE_ACCESS_READ);
	if (hFile == 0) return NO_SAMPLE;

	UINT32 uiSize = FileGetSize(hFile);

	// if insufficient memory, start unloading old samples until either
	// there's nothing left to unload, or we fit
	while (uiSize + guiSoundMemoryUsed > guiSoundMemoryLimit)
	{
		if (!SoundCleanCache())
		{
			SNDDBG("Not enough memory. Size: %u, Used: %u, Max: %u\n", uiSize, guiSoundMemoryUsed, guiSoundMemoryLimit);
			FastDebugMsg(String("SoundLoadDisk:  ERROR:  trying to play %s, not enough memory\n", pFilename));
			goto error_out;
		}
	}

	// if all the sample slots are full, unloading one
	UINT32 uiSample = SoundGetEmptySample();
	if (uiSample == NO_SAMPLE)
	{
		SoundCleanCache();
		uiSample = SoundGetEmptySample();
	}

	// if we still don't have a sample slot
	if (uiSample == NO_SAMPLE)
	{
		FastDebugMsg(String("SoundLoadDisk:  ERROR: Trying to play %s, sound channels are full\n", pFilename));
		goto error_out;
	}

	SAMPLETAG* Sample = &pSampleList[uiSample];
	memset(Sample, 0, sizeof(SAMPLETAG));

  if (!FileSeek(hFile, 12, FILE_SEEK_FROM_CURRENT)) goto error_out;

	UINT16 FormatTag = WAVE_FORMAT_UNKNOWN;
	UINT16 BlockAlign = 0;
	for (;;)
	{
		UINT32 Tag;
		UINT32 Size;

		if (!FileRead(hFile, &Tag, sizeof(Tag)))   goto error_out;
		if (!FileRead(hFile, &Size, sizeof(Size))) goto error_out;

		switch (Tag)
		{
			case FOURCC('f', 'm', 't', ' '):
			{
				UINT16 Channels;
				UINT32 Rate;
				UINT16 BitsPerSample;

				if (!FileRead(hFile, &FormatTag,     sizeof(FormatTag)))     goto error_out;
				if (!FileRead(hFile, &Channels,      sizeof(Channels)))      goto error_out;
				if (!FileRead(hFile, &Rate,          sizeof(Rate)))          goto error_out;
				if (!FileSeek(hFile, 4 , FILE_SEEK_FROM_CURRENT))            goto error_out;
				if (!FileRead(hFile, &BlockAlign,    sizeof(BlockAlign)))    goto error_out;
				if (!FileRead(hFile, &BitsPerSample, sizeof(BitsPerSample))) goto error_out;
				SNDDBG("LOAD file \"%s\" format %u channels %u rate %u bits %u to slot %u\n", pFilename, FormatTag, Channels, Rate, BitsPerSample, uiSample);
				switch (FormatTag)
				{
					case WAVE_FORMAT_PCM: break;

					case WAVE_FORMAT_DVI_ADPCM:
						if (!FileSeek(hFile, 4 , FILE_SEEK_FROM_CURRENT)) goto error_out;
						break;

					default: goto error_out;
				}

				Sample->uiSpeed = Rate;
				Sample->fStereo = (Channels != 1);
				Sample->ubBits  = BitsPerSample;
				break;
			}

			case FOURCC('f', 'a', 'c', 't'):
			{
				UINT32 Samples;
				if (!FileRead(hFile, &Samples, sizeof(Samples))) goto error_out;
				Sample->uiSoundSize = Samples * (Sample->fStereo ? 2 : 1) * 2;
				break;
			}

			case FOURCC('d', 'a', 't', 'a'):
			{
				switch (FormatTag)
				{
					case WAVE_FORMAT_PCM:
					{
						void* Data = malloc(Size);
						if (Data == NULL) goto error_out;
						if (!FileRead(hFile, Data, Size))
						{
							free(Data);
							goto error_out;
						}

						Sample->uiSoundSize = Size;
						Sample->pData       = Data;
						goto sound_loaded;
					}

					case WAVE_FORMAT_DVI_ADPCM:
					{
						INT16* Data = malloc(Sample->uiSoundSize);

						UINT CountSamples = Sample->uiSoundSize >> (1 + (Sample->fStereo ? 1 : 0));
						INT16* D = Data;

						for (;;)
						{
							INT16 CurSample_;
							if (!FileRead(hFile, &CurSample_, sizeof(CurSample_))) goto error_out;

							UINT8 StepIndex_;
							if (!FileRead(hFile, &StepIndex_, sizeof(StepIndex_))) goto error_out;

							if (!FileSeek(hFile, 1 , FILE_SEEK_FROM_CURRENT)) goto error_out; // reserved byte

							INT32 CurSample = CurSample_;
							INT32 StepIndex = StepIndex_;

							*D++ = CurSample;
							if (--CountSamples == 0)
							{
								Sample->ubBits = 16;
								Sample->pData  = Data;
								goto sound_loaded;
							}

							UINT DataCount = BlockAlign / 4;
							while (--DataCount != 0)
							{
								UINT32 DataWord;
								if (!FileRead(hFile, &DataWord, sizeof(DataWord))) goto error_out;
								for (UINT i = 0; i < 8; i++)
								{
									static const INT16 StepTable[] =
									{
										    7,     8,     9,    10,    11,    12,    13,    14,
										   16,    17,    19,    21,    23,    25,    28,    31,
										   34,    37,    41,    45,    50,    55,    60,    66,
										   73,    80,    88,    97,   107,   118,   130,   143,
										  157,   173,   190,   209,   230,   253,   279,   307,
										  337,   371,   408,   449,   494,   544,   598,   658,
										  724,   796,   876,   963,  1060,  1166,  1282,  1411,
										 1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
										 3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
										 7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
										15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
										32767
									};

									static const INT8 IndexTable[] =
									{
										-1, -1, -1, -1, 2, 4, 6, 8
									};

#if 1
									INT32 Diff = ((DataWord & 7) * 2 + 1) * StepTable[StepIndex] >> 3;
#else
									INT32 Diff = 0;
									if (DataWord & 4) Diff += StepTable[StepIndex];
									if (DataWord & 2) Diff += StepTable[StepIndex] >> 1;
									if (DataWord & 1) Diff += StepTable[StepIndex] >> 2;
									Diff += StepTable[StepIndex] >> 3;
#endif
									if (DataWord & 8) Diff = -Diff;
									CurSample = Clamp(-32768, CurSample + Diff, 32767);
									StepIndex = Clamp(0, StepIndex + IndexTable[DataWord & 7], 88);
									DataWord >>= 4;

									*D++ = CurSample;
									if (--CountSamples == 0)
									{
										Sample->ubBits = 16;
										Sample->pData  = Data;
										goto sound_loaded;
									}
								}
							}
						}
					}

					default: goto error_out;
				}
			}

			default:
				if (!FileSeek(hFile, Size, FILE_SEEK_FROM_CURRENT)) goto error_out;
				break;
		}
	}

sound_loaded:
	FileClose(hFile);
	guiSoundMemoryUsed += Sample->uiSoundSize;
	strcpy(Sample->pName, pFilename);
	Sample->uiFlags = SAMPLE_ALLOCATED;
	Sample->uiInstances = 0;
	return uiSample;

error_out:
	FileClose(hFile);
	return NO_SAMPLE;
}


//*******************************************************************************
// SoundCleanCache
//
//		Removes the least-used sound from the cache to make room.
//
//	Returns:	TRUE if a sample was freed, FALSE if none
//
//*******************************************************************************
static BOOLEAN SoundCleanCache(void)
{
	UINT32 uiLowestHits = NO_SAMPLE;
	UINT32 uiLowestHitsCount = 0;

	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
	{
		SAMPLETAG* Sample = &pSampleList[uiCount];
		if ((Sample->uiFlags & SAMPLE_ALLOCATED) && !(Sample->uiFlags & SAMPLE_LOCKED))
		{
			if (uiLowestHits == NO_SAMPLE || uiLowestHitsCount > Sample->uiCacheHits)
			{
				if (!SoundSampleIsPlaying(uiCount))
				{
					uiLowestHits = uiCount;
					uiLowestHitsCount = Sample->uiCacheHits;
				}
			}
		}
	}

	if (uiLowestHits != NO_SAMPLE)
	{
		SNDDBG("FREE sample %u \"%s\" with %u hits\n", uiLowestHits, pSampleList[uiLowestHits].pName, uiLowestHitsCount);
		SoundFreeSampleIndex(uiLowestHits);
		return TRUE;
	}

	return FALSE;
}

//*******************************************************************************
// Low Level Interface (Local use only)
//*******************************************************************************

//*******************************************************************************
// SoundSampleIsPlaying
//
//		Returns TRUE/FALSE that a sample is currently in use for playing a sound.
//
//*******************************************************************************
static BOOLEAN SoundSampleIsPlaying(UINT32 uiSample)
{
	return pSampleList[uiSample].uiInstances > 0;
}

//*******************************************************************************
// SoundGetEmptySample
//
//		Returns the slot number of an available sample index.
//
//	Returns:	A free sample index, or NO_SAMPLE if none are left.
//
//*******************************************************************************
static UINT32 SoundGetEmptySample(void)
{
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
	{
		if (!(pSampleList[uiCount].uiFlags & SAMPLE_ALLOCATED)) return uiCount;
	}

	return NO_SAMPLE;
}


//*******************************************************************************
// SoundFreeSampleIndex
//
//		Frees up a sample referred to by it's index slot number.
//
//	Returns:	Slot number if something was free, NO_SAMPLE otherwise.
//
//*******************************************************************************
static UINT32 SoundFreeSampleIndex(UINT32 uiSample)
{
	SAMPLETAG* Sample = &pSampleList[uiSample];

	if (!(Sample->uiFlags & SAMPLE_ALLOCATED)) return NO_SAMPLE;

	assert(Sample->uiInstances == 0);

	guiSoundMemoryUsed -= Sample->uiSoundSize;
	free(Sample->pData);
	memset(Sample, 0, sizeof(SAMPLETAG));
	return uiSample;
}

//*******************************************************************************
// SoundGetIndexByID
//
//		Searches out a sound instance referred to by it's ID number.
//
//	Returns:	If the instance was found, the slot number. NO_SAMPLE otherwise.
//
//*******************************************************************************
static UINT32 SoundGetIndexByID(UINT32 uiSoundID)
{
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		if (pSoundList[uiCount].uiSoundID == uiSoundID) return uiCount;
	}

	return NO_SAMPLE;
}


static void SoundCallback(void* userdata, Uint8* stream, int len)
{
	UINT16* Stream = (UINT16*)stream;

	// XXX TODO proper mixing, mainly clipping
	for (UINT32 i = 0; i < lengthof(pSoundList); i++)
	{
		SOUNDTAG* Sound = &pSoundList[i];

		switch (Sound->State)
		{
			default:
			case CHANNEL_FREE:
			case CHANNEL_DEAD:
				continue;

			case CHANNEL_STOP:
				Sound->State = CHANNEL_DEAD;
				continue;

			case CHANNEL_PLAY:
			{
				SAMPLETAG* Sample = Sound->pSample;
				UINT32 Samples = len / 4;
				INT32 VolLeft  = Sound->uiFadeVolume * (127 - Sound->Pan) / 127;
				INT32 VolRight = Sound->uiFadeVolume * (  0 + Sound->Pan) / 127;
				UINT32 Amount;

mixing:
				switch (Sample->ubBits)
				{
					case 8:
						if (Sample->fStereo)
						{
							Amount = min(Samples, (Sample->uiSoundSize - Sound->Pos) / 2);
							for (UINT32 i = 0; i < Amount * 2; i += 2)
							{
								Stream[i + 0] += (((UINT8*)Sample->pData + Sound->Pos)[i + 0] - 128) * VolLeft  << 1;
								Stream[i + 1] += (((UINT8*)Sample->pData + Sound->Pos)[i + 1] - 128) * VolRight << 1;
							}
							Sound->Pos += Amount * 2;
						}
						else
						{
							Amount = min(Samples, Sample->uiSoundSize - Sound->Pos);
							for (UINT32 i = 0; i < Amount; i++)
							{
								UINT32 Data = (((UINT8*)Sample->pData + Sound->Pos)[i] - 128) << 1;
								Stream[i * 2 + 0] += Data * VolLeft;
								Stream[i * 2 + 1] += Data * VolRight;
							}
							Sound->Pos += Amount;
						}
						break;

					case 16:
						if (Sample->fStereo)
						{
							Amount = min(Samples, (Sample->uiSoundSize - Sound->Pos) / 4);
							for (UINT32 i = 0; i < Amount * 2; i += 2)
							{
								Stream[i + 0] += ((INT16*)((UINT8*)Sample->pData + Sound->Pos))[i + 0] * VolLeft  >> 7;
								Stream[i + 1] += ((INT16*)((UINT8*)Sample->pData + Sound->Pos))[i + 1] * VolRight >> 7;
							}
							Sound->Pos += Amount * 4;
						}
						else
						{
							Amount = min(Samples, (Sample->uiSoundSize - Sound->Pos) / 2);
							for (UINT32 i = 0; i < Amount; i++)
							{
								UINT32 Data = ((INT16*)((UINT8*)Sample->pData + Sound->Pos))[i];
								Stream[i * 2 + 0] += Data * VolLeft  >> 7;
								Stream[i * 2 + 1] += Data * VolRight >> 7;
							}
							Sound->Pos += Amount * 2;
						}
						break;
				}
				if (Sound->Pos == Sample->uiSoundSize)
				{
					if (Sound->Loops != 1)
					{
						if (Sound->Loops != 0) --Sound->Loops;
						Sound->Pos = 0;
						Samples -= Amount;
						if (Samples != 0) goto mixing;
					}
					else
					{
						Sound->State = CHANNEL_DEAD;
					}
				}
			}
		}
	}
}


static BOOLEAN SoundInitHardware(void)
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	SDL_AudioSpec spec;
	spec.freq     = 22050;
	spec.format   = AUDIO_S16SYS;
	spec.channels = 2;
	spec.samples  = 1024;
	spec.callback = SoundCallback;
	spec.userdata = NULL;

	if (SDL_OpenAudio(&spec, NULL) != 0) return FALSE;

	memset(pSoundList, 0, sizeof(pSoundList));
	SDL_PauseAudio(0);
	return TRUE;
}


static BOOLEAN SoundShutdownHardware(void)
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	return TRUE;
}


//*******************************************************************************
// SoundGetFreeChannel
//
//		Finds an unused sound channel in the channel list.
//
//	Returns:	Index of a sound channel if one was found, SOUND_ERROR if not.
//
//*******************************************************************************
static UINT32 SoundGetFreeChannel(void)
{
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		if (pSoundList[uiCount].State == CHANNEL_FREE) return uiCount;
	}

	return SOUND_ERROR;
}

//*******************************************************************************
// SoundStartSample
//
//		Starts up a sample on the specified channel. Override parameters are passed
//	in through the structure pointer pParms. Any entry with a value of 0xffffffff
//	will be filled in by the system.
//
//	Returns:	Unique sound ID if successful, SOUND_ERROR if not.
//
//*******************************************************************************
static UINT32 SoundStartSample(UINT32 uiSample, UINT32 uiChannel, const SOUNDPARMS* pParms)
{
	SNDDBG("PLAY channel %u sample %u file \"%s\"\n", uiChannel, uiSample, pSampleList[uiSample].pName);

	if (!fSoundSystemInit) return SOUND_ERROR;

	SOUNDTAG* Sound = &pSoundList[uiChannel];
	SAMPLETAG* Sample = &pSampleList[uiSample];

	if (pParms != NULL && pParms->uiVolume != SOUND_PARMS_DEFAULT)
	{
		Sound->uiFadeVolume = pParms->uiVolume;
	}
	else
	{
		Sound->uiFadeVolume = guiSoundDefaultVolume;
	}

	if (pParms != NULL && pParms->uiLoop != SOUND_PARMS_DEFAULT)
	{
		Sound->Loops = pParms->uiLoop;
	}
	else
	{
		Sound->Loops = 1;
	}

	if (pParms != NULL && pParms->uiPan != SOUND_PARMS_DEFAULT)
	{
		Sound->Pan = pParms->uiPan;
	}
	else
	{
		Sound->Pan = 64;
	}

	if (pParms != NULL && (UINT32)pParms->EOSCallback != SOUND_PARMS_DEFAULT)
	{
		Sound->EOSCallback   = pParms->EOSCallback;
		Sound->pCallbackData = pParms->pCallbackData;
	}
	else
	{
		Sound->EOSCallback   = NULL;
		Sound->pCallbackData = NULL;
	}

	UINT32 uiSoundID = SoundGetUniqueID();
	Sound->uiSoundID    = uiSoundID;
	Sound->pSample      = Sample;
	Sound->uiTimeStamp  = GetClock();
	Sound->Pos          = 0;
	Sound->State        = CHANNEL_PLAY;

	Sample->uiInstances++;
	Sample->uiCacheHits++;

	return uiSoundID;
}

//*******************************************************************************
// SoundStartStream
//
//		Starts up a stream on the specified channel. Override parameters are passed
//	in through the structure pointer pParms. Any entry with a value of 0xffffffff
//	will be filled in by the system.
//
//	Returns:	Unique sound ID if successful, SOUND_ERROR if not.
//
//*******************************************************************************
static UINT32 SoundStartStream(const char* pFilename, UINT32 uiChannel, const SOUNDPARMS* pParms)
{
#if 1 // XXX TODO
	FIXME
	return SOUND_ERROR;
#else
	if (!fSoundSystemInit) return SOUND_ERROR;

	SOUNDTAG* Sound = &pSoundList[uiChannel];

	Sound->hMSSStream = AIL_open_stream(hSoundDriver, pFilename, SOUND_DEFAULT_STREAM)
	if (Sound->hMSSStream == NULL)
	{
		SoundCleanCache();
		Sound->hMSSStream = AIL_open_stream(hSoundDriver, pFilename, SOUND_DEFAULT_STREAM);
	}

	if (Sound->hMSSStream == NULL)
	{
		char AILString[200];
		sprintf(AILString, "Stream Error: %s", AIL_last_error());
		DebugMsg(TOPIC_GAME, DBG_LEVEL_0, AILString);
		return SOUND_ERROR;
	}

	if (pParms != NULL && pParms->uiVolume != SOUND_PARMS_DEFAULT)
	{
		AIL_set_stream_volume(Sound->hMSSStream, pParms->uiVolume);
	}
	else
	{
		AIL_set_stream_volume(Sound->hMSSStream, guiSoundDefaultVolume);
	}

	if (pParms != NULL && pParms->uiLoop != SOUND_PARMS_DEFAULT)
	{
		AIL_set_stream_loop_count(Sound->hMSSStream, pParms->uiLoop);
	}

	if (pParms != NULL && pParms->uiPan != SOUND_PARMS_DEFAULT)
	{
		AIL_set_stream_pan(Sound->hMSSStream, pParms->uiPan);
	}

	AIL_start_stream(Sound->hMSSStream);

	UINT32 uiSoundID=SoundGetUniqueID();
	Sound->uiSoundID = uiSoundID;

	if (pParms != NULL && (UINT32)pParms->EOSCallback != SOUND_PARMS_DEFAULT)
	{
		Sound->EOSCallback   = pParms->EOSCallback;
		Sound->pCallbackData = pParms->pCallbackData;
	}
	else
	{
		Sound->EOSCallback   = NULL;
		Sound->pCallbackData = NULL;
	}

	Sound->uiTimeStamp  = GetClock();
	Sound->uiFadeVolume = SoundGetVolumeIndex(uiChannel);

	return uiSoundID;
#endif
}

//*******************************************************************************
// SoundGetUniqueID
//
//		Returns a unique ID number with every call. Basically it's just a 32-bit
// static value that is incremented each time.
//
//*******************************************************************************
static UINT32 SoundGetUniqueID(void)
{
	static UINT32 uiNextID = 0;

	if (uiNextID == SOUND_ERROR) uiNextID++;

	return uiNextID++;
}

//*******************************************************************************
// SoundPlayStreamed
//
//		Returns TRUE/FALSE whether a sound file should be played as a streamed
//	sample, or loaded into the cache. The decision is based on the size of the
//	file compared to the guiSoundCacheThreshold.
//
//	Returns:	TRUE if it should be streamed, FALSE if loaded.
//
//*******************************************************************************
static BOOLEAN SoundPlayStreamed(const char* pFilename)
{
	HWFILE hDisk = FileOpen(pFilename, FILE_ACCESS_READ);
	if (hDisk == 0) return FALSE;

	UINT32 uiFilesize = FileGetSize(hDisk);
	FileClose(hDisk);
	return uiFilesize >= guiSoundCacheThreshold;
}


//*******************************************************************************
// SoundStopIndex
//
//		Stops a sound referred to by it's slot number. This function is the only
//	one that should be deallocating sample handles. The random sounds have to have
//	their counters maintained, and using this as the central function ensures
//	that they stay in sync.
//
//	Returns:	TRUE if the sample was stopped, FALSE if it could not be found.
//
//*******************************************************************************
static BOOLEAN SoundStopIndex(UINT32 uiChannel)
{
	if (!fSoundSystemInit) return FALSE;
	if (uiChannel == NO_SAMPLE) return FALSE;

	SOUNDTAG* Sound = &pSoundList[uiChannel];

	if (Sound->pSample == NULL) return FALSE;

	SNDDBG("STOP channel %u\n", uiChannel);
	Sound->State = CHANNEL_STOP;
	return TRUE;
}


void SoundRemoveSampleFlags( UINT32 uiSample, UINT32 uiFlags )
{
	// CHECK FOR VALID SAMPLE
	if (pSampleList[uiSample].uiFlags & SAMPLE_ALLOCATED)
	{
		//REMOVE
		pSampleList[uiSample].uiFlags &= ~uiFlags;
	}
}
