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
#include <SDL.h>
#include <string.h>

// Uncomment this to disable the startup of sound hardware
//#define SOUND_DISABLE


// global settings
#define		SOUND_MAX_CACHED			128						// number of cache slots

#ifdef JA2
#define		SOUND_MAX_CHANNELS		16						// number of mixer channels
#else
#define		SOUND_MAX_CHANNELS		32						// number of mixer channels
#endif


#define		SOUND_DEFAULT_MEMORY	(8048*1024)		// default memory limit
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
	PTR     pSoundStart; // pointer to start of sound data
	UINT32  uiCacheHits;

	// Random sound data
	UINT32  uiTimeNext;
	UINT32  uiTimeMin, uiTimeMax;
	UINT32  uiVolMin, uiVolMax;
	UINT32  uiPanMin, uiPanMax;
	UINT32  uiPriority;
	UINT32  uiInstances;
	UINT32  uiMaxInstances;

	UINT32  uiAilWaveFormat;  // AIL wave sample type
	UINT32  uiADPCMBlockSize; // Block size for compressed files
} SAMPLETAG;


// Structure definition for slots in the sound output
// These are used for both the cached and double-buffered streams
typedef struct
{
	SAMPLETAG* pSample;
	UINT32     uiSample;
	HSAMPLE    hMSS;
	HSTREAM    hMSSStream;
	UINT32     uiFlags;
	UINT32     uiSoundID;
	UINT32     uiPriority;
	void       (*EOSCallback)(void*);
	void*      pCallbackData;
	UINT32     uiTimeStamp;
	BOOLEAN    fLooping;
	HWFILE     hFile;
	UINT32     uiFadeVolume;
	UINT32     uiFadeRate;
	UINT32     uiFadeTime;
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

	if (SoundPlayStreamed(pFilename))
	{
		//Trying to play a sound which is bigger then the 'guiSoundCacheThreshold'

		// This line was causing a page fault in the Wiz 8 project, so
		// I changed it to the second line, which works OK. -- DB

		//DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("\n*******\nSoundPlay():  ERROR:  trying to play %s which is bigger then the 'guiSoundCacheThreshold', use SoundPlayStreamedFile() instead\n", pFilename));

		FastDebugMsg(String("SoundPlay: ERROR: Trying to play %s sound is too lardge to load into cache, use SoundPlayStreamedFile() instead\n", pFilename));
		return SOUND_ERROR;
	}

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

	if (pParms->uiPriority == SOUND_PARMS_DEFAULT)
		Sample->uiPriority = PRIORITY_RANDOM;
	else
		Sample->uiPriority = pParms->uiPriority;

	Sample->uiInstances = 0;

	UINT32 uiTicks = GetTickCount();
	Sample->uiTimeNext =
		GetTickCount() +
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
#if 1 // XXX TODO
	FIXME
	return FALSE;
#else
	if (!fSoundSystemInit) return FALSE;

	SOUNDTAG* Sound = &pSoundList[uiSound];
	INT32 iStatus = SMP_DONE;

	if (Sound->hMSS != NULL) {
		iStatus = AIL_sample_status(Sound->hMSS);
	}

	if (Sound->hMSSStream != NULL)
	{
		iStatus = AIL_stream_status(Sound->hMSSStream);
	}

	return iStatus != SMP_DONE && iStatus != SMP_STOPPED;
#endif
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

	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		SoundStopIndex(uiCount);
	}

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
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	if (!fSoundSystemInit) return FALSE;

	UINT32 uiVolCap = __min(uiVolume, 127);
	SOUNDTAG* Sound = &pSoundList[uiChannel];

	if (Sound->hMSS != NULL)
	{
		AIL_set_sample_volume(Sound->hMSS, uiVolCap);
	}

	if (Sound->hMSSStream != NULL)
	{
		AIL_set_stream_volume(Sound->hMSSStream, uiVolCap);
	}

	return TRUE;
#endif
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
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	if (!fSoundSystemInit) return FALSE;

	UINT32 uiPanCap = __min(uiPan, 127);

	UINT32 uiSound = SoundGetIndexByID(uiSoundID);
	if (uiSound == NO_SAMPLE) return FALSE;

	SOUNDTAG* Sound = &pSoundList[uiSound];

	if (Sound->hMSS != NULL)
	{
		AIL_set_sample_pan(Sound->hMSS, uiPanCap);
	}

	if (Sound->hMSSStream != NULL)
	{
		AIL_set_stream_pan(Sound->hMSSStream, uiPanCap);
	}

	return TRUE;
#endif
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
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	if (!fSoundSystemInit) return SOUND_ERROR;

	SOUNDTAG* Sound = &pSoundList[uiChannel];

	if (Sound->hMSS != NULL)
	{
		return (UINT32)AIL_sample_volume(Sound->hMSS);
	}

	if (Sound->hMSSStream != NULL)
	{
		return (UINT32)AIL_stream_volume(Sound->hMSSStream);
	}

	return SOUND_ERROR;
#endif
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
	UINT32 uiTicks = GetTickCount();
	SAMPLETAG* Sample = &pSampleList[uiSample];
	return
		Sample->uiFlags & SAMPLE_RANDOM &&
		Sample->uiTimeNext <= GetTickCount() &&
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
	spParms.uiPriority = Sample->uiPriority;

	UINT32 uiSoundID = SoundStartSample(uiSample, uiChannel, &spParms);
	if (uiSoundID == SOUND_ERROR) return NO_SAMPLE;

	Sample->uiTimeNext =
		GetTickCount() +
		Sample->uiTimeMin +
		Random(Sample->uiTimeMax - Sample->uiTimeMin);
	Sample->uiInstances++;
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

		if (Sound->hMSS != NULL)
		{
			// if this was a random sample, decrease the iteration count
			if (pSampleList[Sound->uiSample].uiFlags & SAMPLE_RANDOM)
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
#if 1 // XXX TODO
	return FALSE;
#else
	if (!fSoundSystemInit) return TRUE;

	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		SOUNDTAG* Sound = &pSoundList[uiCount];

		if (Sound->hMSSStream != NULL)
		{
			AIL_service_stream(Sound->hMSSStream, 0);
		}

		if (Sound->hMSS || Sound->hMSSStream)
		{
			// If a sound has a handle, but isn't playing, stop it and free up the handle
			if (!SoundIsPlaying(Sound->uiSoundID))
			{
				SoundStopIndex(uiCount);
			}
			else
			{ // Check the volume fades on currently playing sounds
				UINT32 uiVolume = SoundGetVolumeIndex(uiCount);
				UINT32 uiTime = GetTickCount();

				if (uiVolume != Sound->uiFadeVolume &&
						uiTime >= Sound->uiFadeTime + Sound->uiFadeRate)
				{
					if (uiVolume < Sound->uiFadeVolume)
					{
						SoundSetVolumeIndex(uiCount, ++uiVolume);
					}
					else if (uiVolume > Sound->uiFadeVolume)
					{
						uiVolume--;
						SoundSetVolumeIndex(uiCount, uiVolume);
					}

					Sound->uiFadeTime = uiTime;
				}
			}
		}
	}

	return TRUE;
#endif
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

	UINT32 uiTime = GetTickCount();
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
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
	{
		if (strcasecmp(pSampleList[uiCount].pName, pFilename) == 0) return uiCount;
	}

	return NO_SAMPLE;
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
#if 1 // XXX TODO
	FIXME
	return NO_SAMPLE;
#else
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
			FastDebugMsg(String("SoundLoadDisk:  ERROR:  trying to play %s, not enough memory\n", pFilename));
			FileClose(hFile);
			return NO_SAMPLE;
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
		FileClose(hFile);
		return NO_SAMPLE;
	}

	SAMPLETAG* Sample = &pSampleList[uiSample];
	memset(Sample, 0, sizeof(SAMPLETAG));

	Sample->pData = AIL_mem_alloc_lock(uiSize);
	if (Sample->pData) == NULL)
	{
		FastDebugMsg(String("SoundLoadDisk:  ERROR: Trying to play %s, AIL channels are full\n", pFilename ) );
		FileClose(hFile);
		return NO_SAMPLE;
	}

	guiSoundMemoryUsed += uiSize;

	FileRead(hFile, Sample->pData, uiSize, NULL);
	FileClose(hFile);

	strcpy(Sample->pName, pFilename);
	strupr(Sample->pName);
	Sample->uiSize   = uiSize;
	Sample->uiFlags |= SAMPLE_ALLOCATED;
	return uiSample;
#endif
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
			if (uiLowestHits == NO_SAMPLE || uiLowestHitsCount < Sample->uiCacheHits)
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
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		if (pSoundList[uiCount].uiSample == uiSample) return TRUE;
	}

	return FALSE;
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
#if 1 // XXX TODO
	FIXME
	return NO_SAMPLE;
#else
	SAMPLETAG* Sample = &pSampleList[uiSample];

	if (!(Sample->uiFlags & SAMPLE_ALLOCATED)) return NO_SAMPLE;

	if (Sample->pData != NULL)
	{
		guiSoundMemoryUsed -= Sample->uiSize;
		AIL_mem_free_lock(Sample->pData);
	}

	memset(Sample, 0, sizeof(SAMPLETAG));
	return uiSample;
#endif
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
	// TODO
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
		SOUNDTAG* Sound = &pSoundList[uiCount];
		if (!SoundIsPlaying(Sound->uiSoundID))
		{
			SoundStopIndex(uiCount);
		}

		if (Sound->hMSS == NULL && Sound->hMSSStream == NULL)
		{
			return uiCount;
		}
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
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	char AILString[200];

	if (!fSoundSystemInit) return SOUND_ERROR;

	SOUNDTAG* Sound = &pSoundList[uiChannel];

	Sound->hMSS = AIL_allocate_sample_handle(hSoundDriver);
	if (Sound->hMSS == NULL)
	{
		sprintf(AILString, "Sample Error: %s", AIL_last_error());
		FastDebugMsg(AILString);
		return SOUND_ERROR;
	}

	AIL_init_sample(Sound->hMSS);

	SAMPLETAG* Sample = &pSampleList[uiSample];

	if (!AIL_set_named_sample_file(Sound->hMSS, Sample->pName, Sample->pData, Sample->uiSize, 0))
	{
		AIL_release_sample_handle(Sound->hMSS);
		Sound->hMSS = NULL;

		sprintf(AILString, "AIL Set Sample Error: %s", AIL_last_error());
		DbgMessage(TOPIC_GAME, DBG_LEVEL_0, AILString);
		return SOUND_ERROR;
	}

	// Store the natural playback rate before we modify it below
	Sample->uiSpeed = AIL_sample_playback_rate(Sound->hMSS);

	if (pParms != NULL && pParms->uiVolume != SOUND_PARMS_DEFAULT)
	{
		AIL_set_sample_volume(Sound->hMSS, pParms->uiVolume);
	}
	else
	{
		AIL_set_sample_volume(Sound->hMSS, guiSoundDefaultVolume);
	}

	if (pParms != NULL && pParms->uiLoop != SOUND_PARMS_DEFAULT)
	{
		AIL_set_sample_loop_count(Sound->hMSS, pParms->uiLoop);

		// If looping infinately, lock the sample so it can't be unloaded
		// and mark it as a looping sound
		if (pParms->uiLoop == 0)
		{
			Sample->uiFlags  |= SAMPLE_LOCKED;
			Sound->fLooping  = TRUE;
		}
	}

	if (pParms != NULL && pParms->uiPan != SOUND_PARMS_DEFAULT)
	{
		AIL_set_sample_pan(Sound->hMSS, pParms->uiPan);
	}

	if (pParms != NULL && pParms->uiPriority != SOUND_PARMS_DEFAULT))
	{
		Sound->uiPriority = pParms->uiPriority;
	}
	else
	{
		Sound->uiPriority = PRIORITY_MAX;
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
	Sound->uiSample     = uiSample;
	Sound->uiTimeStamp  = GetTickCount();
	Sound->uiFadeVolume = SoundGetVolumeIndex(uiChannel);

	Sample->uiCacheHits++;

	AIL_start_sample(Sound->hMSS);

	return uiSoundID;
#endif
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
		DbgMessage(TOPIC_GAME, DBG_LEVEL_0, AILString);
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
	if (pParms != NULL)
	{
		Sound->uiPriority = pParms->uiPriority;
	}
	else
	{
		Sound->uiPriority = SOUND_PARMS_DEFAULT;
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

	Sound->uiTimeStamp  = GetTickCount();
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


static BOOLEAN SoundSampleIsInUse(UINT32 uiSample);


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
#if 1 // XXX TODO
	FIXME
	return FALSE;
#else
	if (!fSoundSystemInit) return FALSE;
	if (uiChannel == NO_SAMPLE) return FALSE;

	SOUNDTAG* Sound = &pSoundList[uiChannel];

	if (Sound->hMSS != NULL)
	{
		AIL_stop_sample(Sound->hMSS);
		AIL_release_sample_handle(Sound->hMSS);
		Sound->hMSS = NULL;

		UINT32 uiSample = Sound->uiSample;

		// if this was a random sample, decrease the iteration count
		if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM)
		{
			pSampleList[uiSample].uiInstances--;
		}

		if (Sound->EOSCallback != NULL)
		{
			Sound->EOSCallback(Sound->pCallbackData);
		}

		if (Sound->fLooping && !SoundSampleIsInUse(uiChannel))
		{
			SoundRemoveSampleFlags(uiSample, SAMPLE_LOCKED);
		}

		Sound->uiSample = NO_SAMPLE;
	}

	if (Sound->hMSSStream != NULL)
	{
		AIL_close_stream(Sound->hMSSStream);
		Sound->hMSSStream = NULL;
		if (Sound->EOSCallback != NULL)
		{
			Sound->EOSCallback(pSoundList[uiChannel].pCallbackData);
		}

		Sound->uiSample = NO_SAMPLE;
	}

	if (Sound->hFile != 0)
	{
		FileClose(Sound->hFile);
		Sound->hFile = 0;

		Sound->uiSample = NO_SAMPLE;
	}

	return TRUE;
#endif
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

//*******************************************************************************
// SoundSampleIsInUse
//
//	Returns:	TRUE if the sample index is currently being played by the system.
//
//*******************************************************************************
static BOOLEAN SoundSampleIsInUse(UINT32 uiSample)
{
	for (UINT32 uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
	{
		if (pSoundList[uiCount].uiSample == uiSample && SoundIsPlaying(uiCount))
		{
			return TRUE;
		}
	}

	return FALSE;
}
