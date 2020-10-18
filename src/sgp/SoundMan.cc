/*********************************************************************************
* SGP Digital Sound Module
*
*		This module handles the playing of digital samples, preloaded or streamed.
*
* Derek Beland, May 28, 1997
*********************************************************************************/

#include "Buffer.h"
#include "Debug.h"
#include "FileMan.h"
#include "Random.h"
#include "SoundMan.h"
#include "Timer.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <SDL.h>
#include <string_theory/string>

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <climits>
#include <vector>
#include <iterator>
#include <stdexcept>

// Miniaudio includes needs some defines

#define STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION

#define MA_NO_DEVICE_IO
#define MA_NO_GENERATION
#define MA_NO_ENCODING
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

#undef MINIAUDIO_IMPLEMENTATION

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
	SAMPLE_RANDOM    = 1U << 2,
	SAMPLE_STEREO    = 1U << 3
};


#define SOUND_MAX_CACHED 128 // number of cache slots
#define SOUND_MAX_CHANNELS 16 // number of mixer channels

#define SOUND_DEFAULT_MEMORY (32 * 1024 * 1024) // default memory limit
#define SOUND_DEFAULT_THRESH ( 2 * 1024 * 1024) // size for sample to be double-buffered
#define SOUND_DEFAULT_STREAM (64 * 1024)        // double-buffered buffer size

// The audio device will be opened with the following values
#define SOUND_FREQ      44100
#define SOUND_FORMAT    AUDIO_S16SYS
#define SOUND_MA_SOUND_FORMAT ma_format::ma_format_s16
#define SOUND_CHANNELS  2
#define SOUND_SAMPLES   1024

// Buffer size for a single channel in the sound system
#define SOUND_RING_BUFFER_SIZE (512 * SOUND_SAMPLES)

// Struct definition for sample slots in the cache
// Holds the regular sample data, as well as the data for the random samples
struct SAMPLETAG
{
	ST::string pName;  // Path to sample data
	UINT32  n_samples;

	UINT8*  pData;       // pointer to sample data memory (if playing from an in-memory buffer)

	UINT8* pInMemoryBuffer; // pointer to sample data memory (if playing from an in-memory buffer)
	UINT32 uiBufferSize;
	ma_data_converter* pDataConverter; // pointer to a data converter that decodes the data from pData

	SGPFile* pFile;  // pointer to a SDL_RWops representing the file that we stream from
	SDL_RWops* pRWOps; // RWOps on either pData or pSource
	ma_decoder* pDecoder; // pointer to a decoder that decodes the data from the SDL_RWops

	UINT32  uiFlags;     // Status flags
	UINT32  uiCacheHits;

	// Random sound data
	UINT32  uiTimeNext;
	UINT32  uiTimeMin;
	UINT32  uiTimeMax;
	UINT32  uiVolMin;
	UINT32  uiVolMax;
	UINT32  uiPanMin;
	UINT32  uiPanMax;
	UINT32  uiInstances;
	UINT32  uiMaxInstances;
};


// Structure definition for slots in the sound output
// These are used for both the cached and double-buffered streams
struct SOUNDTAG
{
	volatile UINT State;
	ma_pcm_rb*    pRingBuffer; // Pointer to the ring buffer that holds decoded and transformed data
	SAMPLETAG*    pSample;
	UINT32        uiSoundID;
	void          (*EOSCallback)(void*);
	void*         pCallbackData;
	UINT32        uiTimeStamp;
	HWFILE        hFile;
	UINT32        uiFadeVolume;
	UINT32        uiFadeRate;
	UINT32        uiFadeTime;
	UINT32        pos;
	UINT32        Loops;
	UINT32        Pan;
};

static UINT32 GetSampleSize(const SAMPLETAG* const s);
static const UINT32 guiSoundMemoryLimit    = SOUND_DEFAULT_MEMORY; // Maximum memory used for sounds
static       UINT32 guiSoundMemoryUsed     = 0;                    // Memory currently in use
//static const UINT32 guiSoundCacheThreshold = SOUND_DEFAULT_THRESH; // Double-buffered threshold
static void IncreaseSoundMemoryUsedBySample(SAMPLETAG *sample) { guiSoundMemoryUsed += sample->n_samples * GetSampleSize(sample); }
static void DecreaseSoundMemoryUsedBySample(SAMPLETAG *sample) { guiSoundMemoryUsed -= sample->n_samples * GetSampleSize(sample); }

static BOOLEAN fSoundSystemInit = FALSE; // Startup called
static BOOLEAN gfEnableStartup  = TRUE;  // Allow hardware to start up
static std::vector<INT32> gMixBuffer;

SDL_AudioSpec gTargetAudioSpec;
ma_decoder_config gTargetDecoderConfig;

// Sample cache list for files loaded
static SAMPLETAG pSampleList[SOUND_MAX_CACHED];
// Sound channel list for output channels
static SOUNDTAG pSoundList[SOUND_MAX_CHANNELS];


void SoundEnableSound(BOOLEAN fEnable)
{
	gfEnableStartup = fEnable;
}

bool IsSoundEnabled()
{
	return gfEnableStartup;
}


static void    SoundInitCache(void);
static BOOLEAN SoundInitHardware(void);


void InitializeSoundManager(void)
{
	if (fSoundSystemInit) ShutdownSoundManager();

	std::fill(std::begin(pSoundList), std::end(pSoundList), SOUNDTAG{});

	if (gfEnableStartup && SoundInitHardware()) fSoundSystemInit = TRUE;

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
	fSoundSystemInit = FALSE;
	gMixBuffer.clear();
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
		SLOGE("Trying to play %s sound is too large to load into cache, use SoundPlayStreamedFile() instead", pFilename));
		return SOUND_ERROR;
	}
#endif

	SAMPLETAG* const sample = SoundLoadSample(pFilename);
	if (sample == NULL) return SOUND_ERROR;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

	return SoundStartSample(sample, channel, volume, pan, loop, end_callback, data);
}

static SAMPLETAG* SoundLoadBuffer(std::vector<UINT8>& buf, SDL_AudioFormat format, UINT8 channels, int freq);
static BOOLEAN    SoundCleanCache(void);
static SAMPLETAG* SoundGetEmptySample(void);

UINT32 SoundPlayFromSmackBuff(const char* name, UINT8 channels, UINT8 depth, UINT32 rate, std::vector<UINT8>& buf, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
	SDL_AudioFormat format;

	if (buf.empty()) return SOUND_ERROR;

	//Originaly Sound Blaster could only play mono unsigned 8-bit PCM data.
	//Later it became capable of playing 16-bit audio data, but needed to be signed and LSB.
	//They were the de facto standard so I'm assuming smacker uses the same.
	if (depth == 8) format = AUDIO_U8;
	else if (depth == 16) format = AUDIO_S16LSB;
	else return SOUND_ERROR;

	SAMPLETAG* s = SoundLoadBuffer(buf, format, channels, rate);
	if (s == NULL) return SOUND_ERROR;

	s->pName           = name;
	s->uiPanMax        = 64;
	s->uiMaxInstances  = 1;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

	return SoundStartSample(s, channel, volume, pan, loop, end_callback, data);
}

UINT32 SoundPlayStreamedFile(const char* pFilename, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
try
{
#if 1
	// TODO0003 implement streaming
	return SoundPlay(pFilename, volume, pan, loop, end_callback, data);
#else
	if (!fSoundSystemInit) return SOUND_ERROR;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

	AutoSGPFile hFile(GCM->openForReadingSmart(pFilename, true));

	// MSS cannot determine which provider to play if you don't give it a real filename
	// so if the file isn't in a library, play it normally
	if (DB_EXTRACT_LIBRARY(hFile) == REAL_FILE_LIBRARY_ID)
	{
		return SoundStartStream(pFilename, channel, volume, pan, loop, end_callback, data);
	}

	//Get the real file handle of the file
	File* hRealFileHandle = GetRealFileHandleFromFileManFileHandle(hFile);
	if (hRealFileHandle == NULL)
	{
		SLOGE("SoundPlayStreamedFile(): Couldnt get a real file handle for '%s' in SoundPlayStreamedFile()", pFilename );
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
		channel->hFile = hFile.Release();
	}

	return uiRetVal;
#endif
}
catch (...)
{
	SLOGE("SoundPlayStreamedFile(): Failed to play '%s'", pFilename);
	return SOUND_ERROR;
}


UINT32 SoundPlayRandom(const char* pFilename, UINT32 time_min, UINT32 time_max, UINT32 vol_min, UINT32 vol_max, UINT32 pan_min, UINT32 pan_max, UINT32 max_instances)
{
	SLOGD("playing random Sound: \"%s\"", pFilename);

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

	return (UINT32)(s - pSampleList);
}


static SOUNDTAG* SoundGetChannelByID(UINT32 uiSoundID);


BOOLEAN SoundIsPlaying(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;

	const SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	return channel != NULL &&  channel->State != CHANNEL_FREE;
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
	FOR_EACH(SOUNDTAG, i, pSoundList)
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
	FOR_EACH(SAMPLETAG, i, pSampleList)
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
	FOR_EACH(SOUNDTAG, i, pSoundList)
	{
		if (i->State == CHANNEL_PLAY && i->pSample->uiFlags & SAMPLE_RANDOM)
		{
			SoundStopChannel(i);
		}
	}

	// Unlock all random sounds so they can be dumped from the cache, and
	// take the random flag off so they won't be serviced/played
	FOR_EACH(SAMPLETAG, i, pSampleList)
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
			SLOGD(ST::format("DEAD channel {} file \"{}\" (refcount {})", i, Sound->pSample->pName, Sound->pSample->uiInstances));
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
	if (channel == NULL) return 0;

	const UINT32 now = GetClock();
	return now - channel->uiTimeStamp;
}


// Zeros out the structures of the sample list.
static void SoundInitCache(void)
{
	std::fill(std::begin(pSampleList), std::end(pSampleList), SAMPLETAG{});
}


static void SoundFreeSample(SAMPLETAG* s);


// Frees up all samples in the cache.
static void SoundEmptyCache(void)
{
	SoundStopAll();

	FOR_EACH(SAMPLETAG, i, pSampleList)
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

	FOR_EACH(SAMPLETAG, i, pSampleList)
	{
		if (i->pName.compare_i(pFilename) == 0) return i;
	}

	return NULL;
}

static UINT32 GetSampleSize(const SAMPLETAG* const s)
{
	return 2 * (s->uiFlags & SAMPLE_STEREO ? 2 : 1);
}

/* Converts audio data in the buffer */
static bool SoundConvertBuffer(std::vector<UINT8>& buf,
	SDL_AudioFormat from_format, UINT8 from_channels, int from_hz,
	SDL_AudioFormat to_format, UINT8 to_channels, int to_hz)
{
	Assert(from_channels > 0);
	Assert(to_channels > 0);
	Assert(from_hz > 0);
	Assert(to_hz > 0);

	SDL_version sdl_version_linked;
	SDL_GetVersion(&sdl_version_linked);
	bool is_sdl206 = (sdl_version_linked.major == 2 && sdl_version_linked.minor == 0 && sdl_version_linked.patch == 6);

	// SDL 2.0.6 crashes in SDL_ResampleAudio with an out of bounds read of an internal buffer
	// to avoid the crash we use a custom resampler
	int sdl_hz = is_sdl206 ? from_hz : to_hz;

	// apply SDL audio converter
	SDL_AudioCVT cvt;
	int ret = SDL_BuildAudioCVT(&cvt, from_format, from_channels, from_hz, to_format, to_channels, sdl_hz);
	if (ret == -1)
	{
		SLOGE("SoundConvertBuffer: unsupported conversion (format %x->%x channels %d->%d hz %d->%d(%d)) - %s", from_format, to_format, from_channels, to_channels, from_hz, sdl_hz, to_hz, SDL_GetError());
		return false;
	}
	if (cvt.needed)
	{
		// original size
		Assert(buf.size() <= INT_MAX);
		cvt.len = static_cast<int>(buf.size());

		// temporary size
		size_t tmpsize = static_cast<size_t>(buf.size() * cvt.len_mult);
		buf.resize(tmpsize);
		cvt.buf = buf.data();

		// convert
		if (SDL_ConvertAudio(&cvt) == -1)
		{
			SLOGE("SoundConvertBuffer: SDL_ConvertAudio failed - %s", SDL_GetError());
			return false;
		}

		// final size
		size_t finalsize = static_cast<size_t>(cvt.len_cvt);
		Assert(finalsize <= tmpsize);
		buf.resize(finalsize);
	}

	// apply custom resampler
	// uses linear interpolation between frames
	// it has a cheap cpu cost and produces little audible noise
	if (sdl_hz != to_hz)
	{
		size_t sample_bits = SDL_AUDIO_BITSIZE(to_format);
		Assert(sample_bits % 8 == 0);
		size_t frame_size = (sample_bits / 8) * to_channels;
		if (buf.size() % frame_size != 0)
		{
			SLOGE("SoundConvertBuffer: buffer size %u must be a multiple of frame_size %u", static_cast<UINT32>(buf.size()), static_cast<UINT32>(frame_size));
			return false;
		}

		if (to_format == AUDIO_S16LSB || to_format == AUDIO_S16MSB)
		{
			size_t sample_size = sizeof(INT16);
			Assert(sample_size == 2);
			if (to_format != AUDIO_S16SYS)
			{
				// to system endianess
				for (size_t i = 0; i < buf.size(); i += sample_size) {
					std::swap(buf[i], buf[i + 1]);
				}
			}
			// to interpolate the duration of the last frame we need an extra frame with silence
			buf.insert(buf.end(), frame_size, 0);
			// current position in the interpolation
			// when negative, a frame should be intepolated and from_hz added
			// when positive (or zero), the last frame should be updated and to_hz subtracted
			int pos = 0;
			INT16* last = nullptr;
			std::vector<UINT8> resampled;
			for (size_t i = 0; i < buf.size(); i += frame_size)
			{
				INT16* frame = reinterpret_cast<INT16*>(buf.data() + i);
				while (pos < 0) {
					Assert(last != nullptr);
					if (pos == -to_hz) {
						// we have the exact value of `t = 0`
						UINT8* bytes = reinterpret_cast<UINT8*>(last);
						resampled.insert(resampled.end(), bytes, bytes + frame_size);
					}
					else {
						// interpolate from the last frame to the current frame
						double t = static_cast<double>(pos + to_hz) / to_hz;
						for (size_t channel = 0; channel < to_channels; channel++)
						{
							double interpolated = (1.0 - t) * last[channel] + t * frame[channel];
							INT16 sample = static_cast<INT16>(round(interpolated));
							UINT8* bytes = reinterpret_cast<UINT8*>(&sample);
							resampled.insert(resampled.end(), bytes, bytes + sample_size);
						}
					}
					pos += from_hz;
				}
				// update the last frame
				last = frame;
				pos -= to_hz;
			}
			buf = std::move(resampled);
			if (to_format != AUDIO_S16SYS)
			{
				// from system endianess
				for (size_t i = 0; i < buf.size(); i += sample_size) {
					std::swap(buf[i], buf[i + 1]);
				}
			}
			return true;
		}
		SLOGE("SoundConvertBuffer: unsupported conversion (format %x channels %d hz %d->%d)", to_format, to_channels, sdl_hz, to_hz);
		return false;
	}
	return true;
}

/* Loads a sound from a buffer into the cache, allocating memory and a slot for storage.
 *
 * Returns: The sample if successful, NULL otherwise. */
static SAMPLETAG* SoundLoadBuffer(std::vector<UINT8>& buf, SDL_AudioFormat format, UINT8 channels, int freq)
{
	SAMPLETAG* s = SoundGetEmptySample();

	// if we don't have a sample slot
	if (s == NULL)
	{
		SLOGE("SoundLoadBuffer Error: sound channels are full");
		return NULL;
	}

	UINT8* inMemoryBuffer = new UINT8[buf.size()]{};
	memcpy(inMemoryBuffer, buf.data(), buf.size());

	ma_data_converter_config config = ma_data_converter_config_init(
		// TODO: Fix sound format
		SOUND_MA_SOUND_FORMAT,
		SOUND_MA_SOUND_FORMAT,
		channels,
		gTargetAudioSpec.channels,
		freq,
		gTargetAudioSpec.freq
	);
	ma_data_converter* converter = (ma_data_converter*)ma_malloc(sizeof(ma_data_converter), NULL);
	auto result = ma_data_converter_init(&config, converter);
	if (result != MA_SUCCESS) {
		SLOGE(ST::format(
			"Error initializing sound converter for buffer: {}",
			ma_result_description(result)
		));
		return NULL;
	}

	s->pInMemoryBuffer = inMemoryBuffer;
	s->uiBufferSize = buf.size();
	s->pDataConverter = converter;

	UINT8 samplechannels = std::min(channels, gTargetAudioSpec.channels);
	bool ok = SoundConvertBuffer(buf, format, channels, freq, gTargetAudioSpec.format, samplechannels, gTargetAudioSpec.freq);
	if (!ok)
	{
		SLOGE("SoundLoadBuffer Error: failed to convert data");
		return NULL;
	}

	if (buf.empty())
	{
		SLOGE("SoundLoadBuffer Error: buffer is empty");
		return NULL;
	}

	// if insufficient memory, start unloading old samples until either
	// there's nothing left to unload, or we fit
	UINT32 samplesize = static_cast<UINT32>(buf.size());
	while (samplesize + guiSoundMemoryUsed > guiSoundMemoryLimit)
	{
		if (!SoundCleanCache())
		{
			SLOGE("SoundLoadBuffer Error: not enough memory - Size: %u, Used: %u, Max: %u", samplesize, guiSoundMemoryUsed, guiSoundMemoryLimit);
			return NULL;
		}
	}

	UINT8* sampledata = new UINT8[samplesize]{};
	memcpy(sampledata, buf.data(), samplesize);

	s->pData = sampledata;
	s->uiFlags |= SAMPLE_ALLOCATED;
	s->n_samples = UINT32(samplesize / GetSampleSize(s));

	IncreaseSoundMemoryUsedBySample(s);

	return s;
}

size_t MiniaudioReadProc(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead) {
	auto rwOps = (SDL_RWops*)pDecoder->pUserData;
	return SDL_RWread(rwOps, pBufferOut, sizeof(UINT8), bytesToRead);
}

ma_bool32 MiniaudioSeekProc(ma_decoder* pDecoder, int byteOffset, ma_seek_origin origin) {
	auto rwOps = (SDL_RWops*)pDecoder->pUserData;
	auto sdlOrigin = RW_SEEK_SET;
	if (origin == ma_seek_origin::ma_seek_origin_current) {
		sdlOrigin = RW_SEEK_CUR;
	}
	if (origin == ma_seek_origin::ma_seek_origin_end) {
		sdlOrigin = RW_SEEK_END;
	}

	return SDL_RWseek(rwOps, byteOffset, sdlOrigin);
}


/* Loads a sound file from disk into the cache, allocating memory and a slot
 * for storage.
 *
 * Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
 *          in the cache. */
static SAMPLETAG* SoundLoadDisk(const char* pFilename)
{
	Assert(pFilename != NULL);

	if(pFilename[0] == '\0') {
		SLOGA("SoundLoadDisk Error: pFilename is an empty string.");
		return NULL;
	}

	SGPFile* hFile = NULL;
	SDL_RWops* rwOps = NULL;
	ma_decoder* decoder = NULL;

	try
	{
		hFile = GCM->openGameResForReading(pFilename);
		rwOps = FileGetRWOps(hFile);

		SDL_AudioSpec wavSpec;
		Uint32 wavLength;
		Uint8 *wavBuffer;

		if (SDL_LoadWAV_RW(rwOps, 0,  &wavSpec, &wavBuffer, &wavLength) == NULL) {
			throw std::runtime_error(ST::format("Error reading WAV file \"{}\"- {}", pFilename, SDL_GetError()).c_str());
		}

		std::vector<UINT8> buf(wavBuffer, wavBuffer + wavLength);
		SDL_FreeWAV(wavBuffer);

		SAMPLETAG* s = SoundLoadBuffer(buf, wavSpec.format, wavSpec.channels, wavSpec.freq);
		if (s == NULL)
		{
			throw std::runtime_error(ST::format("Error converting sound file \"%s\"", pFilename).c_str());
		}

		SDL_RWseek(rwOps, 0, RW_SEEK_SET);

		decoder = (ma_decoder*)ma_malloc(sizeof(ma_decoder), NULL);
		auto result = ma_decoder_init(MiniaudioReadProc, MiniaudioSeekProc, rwOps, &gTargetDecoderConfig, decoder);
		if (result != MA_SUCCESS) {
			throw std::runtime_error(ST::format(
				"Error initializing sound decoder for file \"{}\"- {}",
				pFilename,
				ma_result_description(result)
			).c_str());
		}
		s->pFile = hFile;
		s->pDecoder = decoder;
		s->pName = pFilename;

		SLOGD("SoundLoadDisk Success for file \"%s\"", pFilename);
		return s;
	}
	catch (const std::runtime_error& err)
	{
		SLOGE("SoundLoadDisk Error: %s", err.what());
		// Clean up possible allocations
		if (hFile != NULL) {
			FileClose(hFile);
		}
		if (rwOps != NULL) {
			SDL_FreeRW(rwOps);
		}
		if (decoder != NULL) {
			ma_free(decoder, NULL);
		}
		return NULL;
	}
}


// Returns TRUE/FALSE that a sample is currently in use for playing a sound.
static BOOLEAN SoundSampleIsPlaying(const SAMPLETAG* s)
{
	return s->uiInstances > 0;
}


/* Removes the least-used sound from the cache to make room.
 *
 * Returns: TRUE if a sample was freed, FALSE if none */
static BOOLEAN SoundCleanCache(void)
{
	SAMPLETAG* candidate = NULL;

	FOR_EACH(SAMPLETAG, i, pSampleList)
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
		SLOGD(ST::format("freeing sample {} \"{}\" with {} hits", candidate - pSampleList, candidate->pName, candidate->uiCacheHits));
		SoundFreeSample(candidate);
		return TRUE;
	}

	return FALSE;
}


/* Returns an available sample. Clears out other samples if necessary
 *
 * Returns: A free sample or NULL if none are left. */
static SAMPLETAG* SoundGetEmptySample(void)
{
	FOR_EACH(SAMPLETAG, i, pSampleList)
	{
		if (!(i->uiFlags & SAMPLE_ALLOCATED)) return i;
	}

	// Clean cache if no sample has been found yet and try again
	SoundCleanCache();

	FOR_EACH(SAMPLETAG, i, pSampleList)
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

	DecreaseSoundMemoryUsedBySample(s);
	delete s->pDecoder;
	if (s->pRWOps != NULL) {
		SDL_FreeRW(s->pRWOps);
	}
	if (s->pFile != NULL) {
		FileClose(s->pFile);
	}
	if (s->pData != NULL) {
		delete[] s->pData;
	}
	*s = SAMPLETAG{};
}


/* Searches out a sound instance referred to by its ID number.
 *
 * Returns: If the instance was found, the pointer to the channel.  NULL
 *          otherwise. */
static SOUNDTAG* SoundGetChannelByID(UINT32 uiSoundID)
{
	FOR_EACH(SOUNDTAG, i, pSoundList)
	{
		if (i->uiSoundID == uiSoundID) return i;
	}

	return NULL;
}


static void SoundCallback(void* userdata, Uint8* stream, int len)
{
	if (len < 0)
	{
		SLOGA("SoundCallback: unexpected negative len %d", len);
		return;
	}

	// 16-bit stereo = 2 bytes per value, 2 values per sample
	UINT32 want_bytes = static_cast<UINT32>(len);
	UINT32 want_values = want_bytes / sizeof(INT16);
	UINT32 want_samples = want_values / 2;

	gMixBuffer.assign(want_values, 0);

	// Mix sounds
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
				const SAMPLETAG* const s = Sound->pSample;
				const INT vol_l   = Sound->uiFadeVolume * (127 - Sound->Pan) / MAXVOLUME;
				const INT vol_r   = Sound->uiFadeVolume * (  0 + Sound->Pan) / MAXVOLUME;
				UINT32    samples = want_samples;
				UINT32    amount;
				const INT16* src;
				auto rbResult = ma_pcm_rb_acquire_read(Sound->pRingBuffer, &samples, (void**)&src);
				if (rbResult != MA_SUCCESS) {
					SLOGE(ST::format(
						"Could not aquire read pointer for {} sample {} file \"{}\": {}",
						Sound - pSoundList, s - pSampleList, s->pName,
						ma_result_description(rbResult)
					).c_str());
					continue;
				}

mixing:
				amount = MIN(samples, s->n_samples - Sound->pos);
				for (UINT32 i = 0; i < amount; ++i)
				{
					gMixBuffer[2 * i + 0] += src[2 * i + 0] * vol_l >> 7;
					gMixBuffer[2 * i + 1] += src[2 * i + 1] * vol_r >> 7;
				}

				Sound->pos += amount;
				if (Sound->pos == s->n_samples)
				{
					if (Sound->Loops != 1)
					{
						if (Sound->Loops != 0) --Sound->Loops;
						Sound->pos = 0;
						samples -= amount;
						if (samples != 0) goto mixing;
					}
					else
					{
						Sound->State = CHANNEL_DEAD;
					}
				}

				rbResult = ma_pcm_rb_commit_read(Sound->pRingBuffer, samples, (void**)src);
				if (rbResult != MA_SUCCESS) {
					SLOGE(ST::format(
						"Could not commit read pointer for {} sample {} file \"{}\": {}",
						Sound - pSoundList, s - pSampleList, s->pName,
						ma_result_description(rbResult)
					).c_str());
					continue;
				}
			}
		}
	}

	// Clip sounds and fill the stream
	INT16* Stream = (INT16*)stream;
	for (UINT32 i = 0; i < want_values; ++i)
	{
		if (gMixBuffer[i] >= INT16_MAX)     Stream[i] = INT16_MAX;
		else if(gMixBuffer[i] <= INT16_MIN) Stream[i] = INT16_MIN;
		else                                Stream[i] = (INT16)gMixBuffer[i];
	}

	// "The callback must completely initialize the buffer"
	// see: https://wiki.libsdl.org/SDL_AudioSpec
	UINT32 have_bytes = want_values * sizeof(INT16);
	std::fill_n(stream + have_bytes, want_bytes - have_bytes, 0);
}


static BOOLEAN SoundInitHardware(void)
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	gTargetAudioSpec.freq     = SOUND_FREQ;
	gTargetAudioSpec.format   = SOUND_FORMAT;
	gTargetAudioSpec.channels = SOUND_CHANNELS;
	gTargetAudioSpec.samples  = SOUND_SAMPLES;
	gTargetAudioSpec.callback = SoundCallback;
	gTargetAudioSpec.userdata = NULL;

	if (SDL_OpenAudio(&gTargetAudioSpec, NULL) != 0) return FALSE;

	gTargetDecoderConfig = ma_decoder_config_init(SOUND_MA_SOUND_FORMAT, gTargetAudioSpec.channels, gTargetAudioSpec.freq);

	std::fill(std::begin(pSoundList), std::end(pSoundList), SOUNDTAG{});
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
	FOR_EACH(SOUNDTAG, i, pSoundList)
	{
		if (i->State == CHANNEL_FREE) return i;
	}

	return NULL;
}


static UINT32 SoundGetUniqueID(void);

static void FillRingBuffer(SAMPLETAG* sample, SOUNDTAG* channel) {
	Assert(channel->pRingBuffer != NULL);

	auto bytesToWrite = ma_pcm_rb_available_write(channel->pRingBuffer);
	if (bytesToWrite < SOUND_RING_BUFFER_SIZE / 2) {
		// If the ring buffer is still filled more than half the way, do nothing
		return;
	}
	if (bytesToWrite < 0) {
		SLOGE(ST::format(
			"Read pointer is after write pointer for {} sample {} file \"{}\" bytesToWrite {}: {}",
			channel - pSoundList, sample - pSampleList, sample->pName, bytesToWrite
		).c_str());
		return;
	}

	void* pFramesInClientFormat;
	auto result = ma_pcm_rb_acquire_write(channel->pRingBuffer, &bytesToWrite, &pFramesInClientFormat);
	if (result != MA_SUCCESS) {
		SLOGE(ST::format(
			"Could not aquire write pointer for {} sample {} file \"{}\" bytesToWrite {}: {}",
			channel - pSoundList, sample - pSampleList, sample->pName, bytesToWrite,
			ma_result_description(result)
		).c_str());
		return;
	}
	ma_uint64 framesRead = 0;
	if (sample->pFile != NULL) {
		auto result = ma_decoder_seek_to_pcm_frame(sample->pDecoder, channel->pos);
		if (result != MA_SUCCESS) {
			SLOGE(ST::format(
				"Could not seek to current sound position for {} sample {} file \"{}\" bytesToWrite {}: {}",
				channel - pSoundList, sample - pSampleList, sample->pName, bytesToWrite,
				ma_result_description(result)
			).c_str());
			return;
		}
		// We stream from file
		framesRead = ma_decoder_read_pcm_frames(sample->pDecoder, pFramesInClientFormat, bytesToWrite);
		if (framesRead < bytesToWrite) {
			// TODO: Reached the end -> Loop when sound is looped
		}
	} else if (sample->pInMemoryBuffer != NULL) {
		auto requiredInputFrameCount = ma_data_converter_get_required_input_frame_count(sample->pDataConverter, bytesToWrite);
		auto availableBytes = MIN(requiredInputFrameCount, sample->uiBufferSize - channel->pos);
		auto expectedOutputFrameCount = ma_data_converter_get_expected_output_frame_count(sample->pDataConverter, availableBytes);
		
		auto result = ma_data_converter_process_pcm_frames(
			sample->pDataConverter,
			sample->pInMemoryBuffer + channel->pos,
			&availableBytes,
			pFramesInClientFormat,
			&expectedOutputFrameCount
		);
		if (result != MA_SUCCESS) {
			SLOGE(ST::format(
				"Error converting data for {} sample {} file \"{}\" bytesToWrite {}: {}",
				channel - pSoundList, sample - pSampleList, sample->pName, bytesToWrite,
				ma_result_description(result)
			).c_str());
			return;
		}
		framesRead = expectedOutputFrameCount;
	} else {
		SLOGE(ST::format(
			"Dont know how to fill stream for {} sample {} file \"{}\" bytesToWrite {}: {}",
			channel - pSoundList, sample - pSampleList, sample->pName, bytesToWrite,
			ma_result_description(result)
		).c_str());
	}
	result = ma_pcm_rb_commit_write(channel->pRingBuffer, framesRead, pFramesInClientFormat);
	if (result != MA_SUCCESS) {
		SLOGE(ST::format(
			"Could not commit write for {} sample {} file \"{}\" bytesToWrite {}: {}",
			channel - pSoundList, sample - pSampleList, sample->pName, bytesToWrite,
			ma_result_description(result)
		).c_str());
		return;
	}
}

/* Starts up a sample on the specified channel. Override parameters are passed
 * in through the structure pointer pParms. Any entry with a value of 0xffffffff
 * will be filled in by the system.
 *
 * Returns: Unique sound ID if successful, SOUND_ERROR if not. */
static UINT32 SoundStartSample(SAMPLETAG* sample, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
	SLOGD(ST::format("playing channel {} sample {} file \"{}\"", channel - pSoundList, sample - pSampleList, sample->pName));

	if (!fSoundSystemInit) return SOUND_ERROR;

	channel->uiFadeVolume  = volume;
	channel->Loops         = loop;
	channel->Pan           = pan;
	channel->EOSCallback   = end_callback;
	channel->pCallbackData = data;

	// Allocate ring buffer
	if (channel->pRingBuffer == NULL) {
		channel->pRingBuffer = (ma_pcm_rb*)ma_malloc(sizeof(ma_pcm_rb), NULL);
		ma_result result = ma_pcm_rb_init(SOUND_MA_SOUND_FORMAT, SOUND_CHANNELS, SOUND_RING_BUFFER_SIZE, NULL, NULL, channel->pRingBuffer);
		if (result != MA_SUCCESS) {
			SLOGE(ST::format(
					"Error initializing ring buffer for channel {} sample {} file \"{}\": {}",
					channel - pSoundList, sample - pSampleList, sample->pName,
					ma_result_description(result)
				).c_str());
			return SOUND_ERROR;
		}
	} else {
		ma_pcm_rb_reset(channel->pRingBuffer);
	}

	UINT32 uiSoundID = SoundGetUniqueID();
	channel->uiSoundID    = uiSoundID;
	channel->pSample      = sample;
	channel->uiTimeStamp  = GetClock();
	channel->pos          = 0;

	// Fill ring buffer with initial data
	FillRingBuffer(sample, channel);

	channel->State        = CHANNEL_PLAY;

	sample->uiInstances++;
	sample->uiCacheHits++;

	return uiSoundID;
}

/* Returns a unique ID number with every call. Basically it's just a 32-bit
 * static value that is incremented each time. */
static UINT32 SoundGetUniqueID(void)
{
	static UINT32 uiNextID = 0;

	if (uiNextID == SOUND_ERROR) uiNextID++;

	return uiNextID++;
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

	SLOGD(ST::format("stopping channel channel {}", (channel - pSoundList)));
	// TODO: Probably not necessary, we can also just reset the buffer
	if (channel->pRingBuffer != NULL) {
		ma_pcm_rb_uninit(channel->pRingBuffer);
		ma_free(channel->pRingBuffer, NULL);
		channel->pRingBuffer = NULL;
	}
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
