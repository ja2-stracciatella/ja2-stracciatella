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


// Sample status flags
enum
{
	SAMPLE_ALLOCATED = 1U << 0,
	SAMPLE_LOCKED    = 1U << 1,
	SAMPLE_RANDOM    = 1U << 2
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


// Struct definition for sample slots in the cache
// Holds the regular sample data, as well as the data for the random samples
typedef struct
{
	CHAR8   pName[128];  // Path to sample data
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


// Low level
static BOOLEAN SoundInitHardware(void);
static UINT32 SoundGetUniqueID(void);
static BOOLEAN SoundPlayStreamed(const char* pFilename);
static BOOLEAN SoundCleanCache(void);

// Global variables
static const UINT32 guiSoundDefaultVolume  = MAXVOLUME;
static const UINT32 guiSoundMemoryLimit    = SOUND_DEFAULT_MEMORY; // Maximum memory used for sounds
static       UINT32 guiSoundMemoryUsed     = 0;                    // Memory currently in use
static const UINT32 guiSoundCacheThreshold = SOUND_DEFAULT_THRESH; // Double-buffered threshold

// Local module variables
static BOOLEAN fSoundSystemInit = FALSE; // Startup called T/F
static BOOLEAN gfEnableStartup  = TRUE;  // Allow hardware to starup

// Sample cache list for files loaded
static SAMPLETAG pSampleList[SOUND_MAX_CACHED];
// Sound channel list for output channels
static SOUNDTAG pSoundList[SOUND_MAX_CHANNELS];


void SoundEnableSound(BOOLEAN fEnable)
{
	gfEnableStartup=fEnable;
}


static void SoundInitCache(void);


void InitializeSoundManager(void)
{
	if (fSoundSystemInit) ShutdownSoundManager();

	memset(pSoundList, 0, sizeof(pSoundList));

#ifndef SOUND_DISABLE
	if (gfEnableStartup && SoundInitHardware()) fSoundSystemInit = TRUE;
#endif

	SoundInitCache();

	guiSoundMemoryUsed = 0;
}


static void SoundEmptyCache(void);
static void SoundShutdownHardware(void);


void ShutdownSoundManager(void)
{
	SoundStopAll();
	SoundEmptyCache();
	SoundShutdownHardware();
	fSoundSystemInit=FALSE;
}


static SOUNDTAG*  SoundGetFreeChannel(void);
static SAMPLETAG* SoundLoadSample(const char* pFilename);
static UINT32     SoundStartSample(SAMPLETAG* sample, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data);


UINT32 SoundPlay(const char* pFilename, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
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

	SAMPLETAG* const sample = SoundLoadSample(pFilename);
	if (sample == NULL) return SOUND_ERROR;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

	return SoundStartSample(sample, channel, volume, pan, loop, end_callback, data);
}


static UINT32 SoundStartStream(const char* pFilename, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data);


UINT32 SoundPlayStreamedFile(const char* pFilename, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
#if 1
	// TODO0003 implement streaming
	return SoundPlay(pFilename, volume, pan, loop, end_callback, data);
#else
	if (!fSoundSystemInit) return SOUND_ERROR;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

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
		return SoundStartStream(pFilename, channel, volume, pan, loop, end_callback, data);
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
	UINT32 uiRetVal = SoundStartStream(pFileHandlefileName, channel, volume, pan, loop, end_callback, data);

	//if it succeeded, record the file handle
	if (uiRetVal != SOUND_ERROR)
	{
		channel->hFile = hFile;
	}
	else
	{
		FileClose(hFile);
	}

	return uiRetVal;
#endif
}


UINT32 SoundPlayRandom(const char* pFilename, UINT32 time_min, UINT32 time_max, UINT32 vol_min, UINT32 vol_max, UINT32 pan_min, UINT32 pan_max, UINT32 max_instances)
{
	SNDDBG("RAND \"%s\"\n", pFilename);

	if (!fSoundSystemInit) return SOUND_ERROR;

	SAMPLETAG* const s = SoundLoadSample(pFilename);
	if (s == NULL) return SOUND_ERROR;

	s->uiFlags        |= SAMPLE_RANDOM | SAMPLE_LOCKED;
	s->uiTimeMin       = time_min;
	s->uiTimeMax       = time_max;
	s->uiVolMin        = vol_min;
	s->uiVolMax        = vol_max;
	s->uiPanMin        = pan_min;
	s->uiPanMax        = pan_max;
	s->uiMaxInstances  = max_instances;

	s->uiTimeNext =
		GetClock() +
		s->uiTimeMin +
		Random(s->uiTimeMax - s->uiTimeMin);

	return s - pSampleList;
}


static SOUNDTAG* SoundGetChannelByID(UINT32 uiSoundID);


BOOLEAN SoundIsPlaying(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;

	const SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	return channel != NULL && channel->State != CHANNEL_FREE;
}


static BOOLEAN SoundStopChannel(SOUNDTAG* channel);


BOOLEAN SoundStop(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;
	if (!SoundIsPlaying(uiSoundID)) return FALSE;

	SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	SoundStopChannel(channel);
	return TRUE;
}


void SoundStopAll(void)
{
	if (!fSoundSystemInit) return;

	SDL_PauseAudio(1);
	for (SOUNDTAG* i = pSoundList; i != endof(pSoundList); ++i)
	{
		if (SoundStopChannel(i))
		{
			assert(i->pSample->uiInstances != 0);
			i->pSample->uiInstances -= 1;
			i->pSample               = NULL;
			i->uiSoundID             = SOUND_ERROR;
			i->State                 = CHANNEL_FREE;
		}
	}
	SDL_PauseAudio(0);
}


BOOLEAN SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume)
{
	if (!fSoundSystemInit) return FALSE;

	SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	channel->uiFadeVolume = __min(uiVolume, MAXVOLUME);
	return TRUE;
}


BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan)
{
	if (!fSoundSystemInit) return FALSE;

	SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	channel->Pan = __min(uiPan, 127);
	return TRUE;
}


UINT32 SoundGetVolume(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return SOUND_ERROR;

	const SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return SOUND_ERROR;

	return channel->uiFadeVolume;
}


static BOOLEAN SoundRandomShouldPlay(const SAMPLETAG* s);
static UINT32 SoundStartRandom(SAMPLETAG* s);


void SoundServiceRandom(void)
{
	for (SAMPLETAG* i = pSampleList; i != endof(pSampleList); ++i)
	{
		if (SoundRandomShouldPlay(i)) SoundStartRandom(i);
	}
}


/* Determines whether a random sound is ready for playing or not.
 *
 * Returns: TRUE if a the sample should be played. */
static BOOLEAN SoundRandomShouldPlay(const SAMPLETAG* s)
{
	return
		s->uiFlags & SAMPLE_RANDOM &&
		s->uiTimeNext <= GetClock() &&
		s->uiInstances < s->uiMaxInstances;
}


/* Starts an instance of a random sample.
 *
 * Returns: TRUE if a new random sound was created, FALSE if nothing was done. */
static UINT32 SoundStartRandom(SAMPLETAG* s)
{
	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return NO_SAMPLE;

	const UINT32 volume = s->uiVolMin + Random(s->uiVolMax - s->uiVolMin);
	const UINT32 pan    = s->uiPanMin + Random(s->uiPanMax - s->uiPanMin);

	const UINT32 uiSoundID = SoundStartSample(s, channel, volume, pan, 1, NULL, NULL);
	if (uiSoundID == SOUND_ERROR) return NO_SAMPLE;

	s->uiTimeNext =
		GetClock() +
		s->uiTimeMin +
		Random(s->uiTimeMax - s->uiTimeMin);
	return uiSoundID;
}


void SoundStopAllRandom(void)
{
	// Stop all currently playing random sounds
	for (SOUNDTAG* i = pSoundList; i != endof(pSoundList); ++i)
	{
		if (i->State == CHANNEL_PLAY && i->pSample->uiFlags & SAMPLE_RANDOM)
		{
			SoundStopChannel(i);
		}
	}

	// Unlock all random sounds so they can be dumped from the cache, and
	// take the random flag off so they won't be serviced/played
	for (SAMPLETAG* i = pSampleList; i != endof(pSampleList); ++i)
	{
		if (i->uiFlags & SAMPLE_RANDOM)
		{
			i->uiFlags &= ~(SAMPLE_RANDOM | SAMPLE_LOCKED);
		}
	}
}


void SoundServiceStreams(void)
{
	if (!fSoundSystemInit) return;

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
}


UINT32 SoundGetPosition(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return 0;

	const SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
#if 0
	UINT32 uiPosition = 0;
	UINT32 uiFreq = 0;
	UINT32 uiFormat = 0;
	if (channel != NULL)
	{
		if (channel->hMSSStream != NULL)
		{
			uiPosition = (UINT32)AIL_stream_position(channel->hMSSStream);
			uiFreq     = (UINT32)channel->hMSSStream->samp->playback_rate;
			uiFormat   = (UINT32)channel->hMSSStream->samp->format;

		}
		else if (channel->hMSS != NULL)
		{
			uiPosition = (UINT32)AIL_sample_position(channel->hMSS);
			uiFreq     = (UINT32)channel->hMSS->playback_rate;
			uiFormat   = (UINT32)channel->hMSS->format;
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
	if (channel == NULL) return 0;

	const UINT32 now = GetClock();
	return now - channel->uiTimeStamp;
#endif
}


// Zeros out the structures of the sample list.
static void SoundInitCache(void)
{
	memset(pSampleList, 0, sizeof(pSampleList));
}


static void SoundFreeSample(SAMPLETAG* s);


// Frees up all samples in the cache.
static void SoundEmptyCache(void)
{
	SoundStopAll();

	for (SAMPLETAG* i = pSampleList; i != endof(pSampleList); ++i)
	{
		SoundFreeSample(i);
	}
}


static SAMPLETAG* SoundGetCached(const char* pFilename);
static SAMPLETAG* SoundLoadDisk(const char* pFilename);


static SAMPLETAG* SoundLoadSample(const char* pFilename)
{
	SAMPLETAG* const s = SoundGetCached(pFilename);
	if (s != NULL) return s;

	return SoundLoadDisk(pFilename);
}


/* Tries to locate a sound by looking at what is currently loaded in the cache.
 *
 * Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
 *          in the cache. */
static SAMPLETAG* SoundGetCached(const char* pFilename)
{
	if (pFilename[0] == '\0') return NULL; // XXX HACK0009

	for (SAMPLETAG* i = pSampleList; i != endof(pSampleList); ++i)
	{
		if (strcasecmp(i->pName, pFilename) == 0) return i;
	}

	return NULL;
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


static SAMPLETAG* SoundGetEmptySample(void);


/* Loads a sound file from disk into the cache, allocating memory and a slot
 * for storage.
 *
 * Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
 *          in the cache. */
static SAMPLETAG* SoundLoadDisk(const char* pFilename)
{
	Assert(pFilename != NULL);

	HWFILE hFile = FileOpen(pFilename, FILE_ACCESS_READ);
	if (hFile == 0) return NULL;

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
	SAMPLETAG* s = SoundGetEmptySample();
	if (s == NULL)
	{
		SoundCleanCache();
		s = SoundGetEmptySample();
	}

	// if we still don't have a sample slot
	if (s == NULL)
	{
		FastDebugMsg(String("SoundLoadDisk:  ERROR: Trying to play %s, sound channels are full\n", pFilename));
		goto error_out;
	}

	memset(s, 0, sizeof(*s));

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
				SNDDBG("LOAD file \"%s\" format %u channels %u rate %u bits %u to slot %u\n", pFilename, FormatTag, Channels, Rate, BitsPerSample, s - pSampleList);
				switch (FormatTag)
				{
					case WAVE_FORMAT_PCM: break;

					case WAVE_FORMAT_DVI_ADPCM:
						if (!FileSeek(hFile, 4 , FILE_SEEK_FROM_CURRENT)) goto error_out;
						break;

					default: goto error_out;
				}

				s->uiSpeed = Rate;
				s->fStereo = (Channels != 1);
				s->ubBits  = BitsPerSample;
				break;
			}

			case FOURCC('f', 'a', 'c', 't'):
			{
				UINT32 Samples;
				if (!FileRead(hFile, &Samples, sizeof(Samples))) goto error_out;
				s->uiSoundSize = Samples * (s->fStereo ? 2 : 1) * 2;
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

						s->uiSoundSize = Size;
						s->pData       = Data;
						goto sound_loaded;
					}

					case WAVE_FORMAT_DVI_ADPCM:
					{
						INT16* Data = malloc(s->uiSoundSize);

						UINT CountSamples = s->uiSoundSize >> (1 + (s->fStereo ? 1 : 0));
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
								s->ubBits = 16;
								s->pData  = Data;
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
										s->ubBits = 16;
										s->pData  = Data;
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
	guiSoundMemoryUsed += s->uiSoundSize;
	strcpy(s->pName, pFilename);
	s->uiFlags     = SAMPLE_ALLOCATED;
	s->uiInstances = 0;
	return s;

error_out:
	FileClose(hFile);
	return NULL;
}


static BOOLEAN SoundSampleIsPlaying(const SAMPLETAG* s);


/* Removes the least-used sound from the cache to make room.
 *
 * Returns: TRUE if a sample was freed, FALSE if none */
static BOOLEAN SoundCleanCache(void)
{
	SAMPLETAG* candidate = NULL;

	for (SAMPLETAG* i = pSampleList; i != endof(pSampleList); ++i)
	{
		if (i->uiFlags & SAMPLE_ALLOCATED &&
				!(i->uiFlags & SAMPLE_LOCKED) &&
				(candidate == NULL || candidate->uiCacheHits > i->uiCacheHits))
		{
			if (!SoundSampleIsPlaying(i)) candidate = i;
		}
	}

	if (candidate != NULL)
	{
		SNDDBG("FREE sample %u \"%s\" with %u hits\n", candidate - pSampleList, candidate->pName, candidate->uiCacheHits);
		SoundFreeSample(candidate);
		return TRUE;
	}

	return FALSE;
}

//*******************************************************************************
// Low Level Interface (Local use only)
//*******************************************************************************

// Returns TRUE/FALSE that a sample is currently in use for playing a sound.
static BOOLEAN SoundSampleIsPlaying(const SAMPLETAG* s)
{
	return s->uiInstances > 0;
}


/* Returns an available sample.
 *
 * Returns: A free sample or NULL if none are left. */
static SAMPLETAG* SoundGetEmptySample(void)
{
	for (SAMPLETAG* i = pSampleList; i != endof(pSampleList); ++i)
	{
		if (!(i->uiFlags & SAMPLE_ALLOCATED)) return i;
	}

	return NULL;
}


// Frees up a sample referred to by its index slot number.
static void SoundFreeSample(SAMPLETAG* s)
{
	if (!(s->uiFlags & SAMPLE_ALLOCATED)) return;

	assert(s->uiInstances == 0);

	guiSoundMemoryUsed -= s->uiSoundSize;
	free(s->pData);
	memset(s, 0, sizeof(*s));
}


/* Searches out a sound instance referred to by its ID number.
 *
 * Returns: If the instance was found, the pointer to the channel.  NULL
 *          otherwise. */
static SOUNDTAG* SoundGetChannelByID(UINT32 uiSoundID)
{
	for (SOUNDTAG* i = pSoundList; i != endof(pSoundList); ++i)
	{
		if (i->uiSoundID == uiSoundID) return i;
	}

	return NULL;
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
				INT32 VolLeft  = Sound->uiFadeVolume * (127 - Sound->Pan) / MAXVOLUME;
				INT32 VolRight = Sound->uiFadeVolume * (  0 + Sound->Pan) / MAXVOLUME;
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


static void SoundShutdownHardware(void)
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}


/* Finds an unused sound channel in the channel list.
 *
 * Returns: Pointer to a sound channel if one was found, NULL if not. */
static SOUNDTAG* SoundGetFreeChannel(void)
{
	for (SOUNDTAG* i = pSoundList; i != endof(pSoundList); ++i)
	{
		if (i->State == CHANNEL_FREE) return i;
	}

	return NULL;
}


/* Starts up a sample on the specified channel. Override parameters are passed
 * in through the structure pointer pParms. Any entry with a value of 0xffffffff
 * will be filled in by the system.
 *
 * Returns: Unique sound ID if successful, SOUND_ERROR if not. */
static UINT32 SoundStartSample(SAMPLETAG* sample, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
	SNDDBG("PLAY channel %u sample %u file \"%s\"\n", channel - pSoundList, sample - pSampleList, sample->pName);

	if (!fSoundSystemInit) return SOUND_ERROR;

	channel->uiFadeVolume  = volume;
	channel->Loops         = loop;
	channel->Pan           = pan;
	channel->EOSCallback   = end_callback;
	channel->pCallbackData = data;

	UINT32 uiSoundID = SoundGetUniqueID();
	channel->uiSoundID    = uiSoundID;
	channel->pSample      = sample;
	channel->uiTimeStamp  = GetClock();
	channel->Pos          = 0;
	channel->State        = CHANNEL_PLAY;

	sample->uiInstances++;
	sample->uiCacheHits++;

	return uiSoundID;
}


/* Starts up a stream on the specified channel. Override parameters are passed
 * in through the structure pointer pParms. Any entry with a value of 0xffffffff
 * will be filled in by the system.
 *
 * Returns: Unique sound ID if successful, SOUND_ERROR if not. */
static UINT32 SoundStartStream(const char* pFilename, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
#if 1 // XXX TODO
	FIXME
	return SOUND_ERROR;
#else
	if (!fSoundSystemInit) return SOUND_ERROR;

	channel->hMSSStream = AIL_open_stream(hSoundDriver, pFilename, SOUND_DEFAULT_STREAM)
	if (channel->hMSSStream == NULL)
	{
		SoundCleanCache();
		channel->hMSSStream = AIL_open_stream(hSoundDriver, pFilename, SOUND_DEFAULT_STREAM);
	}

	if (channel->hMSSStream == NULL)
	{
		char AILString[200];
		sprintf(AILString, "Stream Error: %s", AIL_last_error());
		DebugMsg(TOPIC_GAME, DBG_LEVEL_0, AILString);
		return SOUND_ERROR;
	}

	AIL_set_stream_volume(    channel->hMSSStream, volume);
	AIL_set_stream_loop_count(channel->hMSSStream, loop);
	AIL_set_stream_pan(       channel->hMSSStream, pan);

	AIL_start_stream(channel->hMSSStream);

	UINT32 uiSoundID=SoundGetUniqueID();
	channel->uiSoundID = uiSoundID;

	channel->EOSCallback   = end_callback;
	channel->pCallbackData = data;

	channel->uiTimeStamp  = GetClock();
	channel->uiFadeVolume = SoundGetVolumeIndex(uiChannel);

	return uiSoundID;
#endif
}


/* Returns a unique ID number with every call. Basically it's just a 32-bit
 * static value that is incremented each time. */
static UINT32 SoundGetUniqueID(void)
{
	static UINT32 uiNextID = 0;

	if (uiNextID == SOUND_ERROR) uiNextID++;

	return uiNextID++;
}


/* Returns TRUE/FALSE whether a sound file should be played as a streamed
 * sample, or loaded into the cache. The decision is based on the size of the
 * file compared to the guiSoundCacheThreshold.
 *
 * Returns: TRUE if it should be streamed, FALSE if loaded. */
static BOOLEAN SoundPlayStreamed(const char* pFilename)
{
	HWFILE hDisk = FileOpen(pFilename, FILE_ACCESS_READ);
	if (hDisk == 0) return FALSE;

	UINT32 uiFilesize = FileGetSize(hDisk);
	FileClose(hDisk);
	return uiFilesize >= guiSoundCacheThreshold;
}


/* Stops a sound referred to by its channel.  This function is the only one
 * that should be deallocating sample handles. The random sounds have to have
 * their counters maintained, and using this as the central function ensures
 * that they stay in sync.
 *
 * Returns: TRUE if the sample was stopped, FALSE if it could not be found. */
static BOOLEAN SoundStopChannel(SOUNDTAG* channel)
{
	if (!fSoundSystemInit) return FALSE;

	if (channel->pSample == NULL) return FALSE;

	SNDDBG("STOP channel %u\n", channel - pSoundList);
	channel->State = CHANNEL_STOP;
	return TRUE;
}


void SoundStopRandom(UINT32 uiSample)
{
	// CHECK FOR VALID SAMPLE
	SAMPLETAG* const s = &pSampleList[uiSample];
	if (s->uiFlags & SAMPLE_ALLOCATED)
	{
		s->uiFlags &= ~SAMPLE_RANDOM;
	}
}
