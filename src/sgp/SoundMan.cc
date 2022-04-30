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
#include <atomic>
#include <mutex>
#include <condition_variable>

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

// The audio device will be opened with the following values
#define SOUND_FREQ      44100
#define SOUND_FORMAT    AUDIO_S16SYS
#define SOUND_MA_SOUND_FORMAT ma_format::ma_format_s16
#define SOUND_CHANNELS  2
#define SOUND_SAMPLES   1024

// Threshold from which a sound is streamed from file instead of from memory (currently 1 MB)
#define SOUND_FILE_STREAMING_THRESHOLD (1024 * 1024)
// Buffer size for a single channel in the sound system
#define SOUND_RING_BUFFER_SIZE (128 * SOUND_SAMPLES)

// Struct definition for sample slots in the cache
// Holds the regular sample data, as well as the data for the random samples
struct SAMPLETAG
{
	ST::string pName;  // Path to sample data

	UINT8* pInMemoryBuffer; // pointer to sample data memory (if playing from an in-memory buffer)
	UINT32 uiBufferSize; // The size of the in-memory buffer
	ma_format eInMemoryFormat;
	UINT32 uiInMemoryChannels;

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
	SAMPLETAG*    pSample;
	UINT32        uiSoundID;
	void          (*EOSCallback)(void*);
	void*         pCallbackData;
	UINT32        uiTimeStamp;
	HWFILE        hFile;
	UINT32        uiFadeVolume;
	UINT32        uiFadeRate;
	UINT32        uiFadeTime;
	UINT32        Loops;
	UINT32        Pan;

	// The following properties might be accessed from multiple threads, so they need to be thread safe (or accessed in a thread safe way)
	ma_pcm_rb*    pRingBuffer; // Pointer to the ring buffer that holds decoded and converted data
	UINT32 State; // This represents the state of the sound (PLAYING / DEAD)
	UINT32 Pos; // This represents the position of the sound that we are currently at (in samples)
	BOOLEAN DoneServicing;
};

static BOOLEAN fSoundSystemInit = FALSE; // Startup called
static BOOLEAN gfEnableStartup  = TRUE;  // Allow hardware to start up
static std::vector<INT32> gMixBuffer;

SDL_AudioSpec gTargetAudioSpec;
ma_decoder_config gTargetDecoderConfig;

// These synchronization primitives are used to signal between the buffer servicing thread and the sound callback thread
// When the callback thread notices that the buffer for a channel is less than half filled, it will notify the stream processing
// thread that it needs to do some processing through the condition_variable
// When the system needs to be shut down, fShutdownBufferServiceThread needs to be set to true and the buffer servicing thread
// needs to be notified using the condition_variable
SDL_Thread *bufferServiceThread = NULL;
std::mutex mutexBuffersNeedService;
std::condition_variable conditionBuffersNeedService;
BOOLEAN fBuffersNeedService = FALSE;
BOOLEAN fShutdownBufferServiceThread = FALSE;

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
static int SoundServiceBuffers(void *_ptr);

void InitializeSoundManager(void)
{
	if (fSoundSystemInit) ShutdownSoundManager();

	std::fill(std::begin(pSoundList), std::end(pSoundList), SOUNDTAG{});

	if (gfEnableStartup && SoundInitHardware()) fSoundSystemInit = TRUE;

	SoundInitCache();
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

	SAMPLETAG* const sample = SoundLoadSample(pFilename);
	if (sample == NULL) return SOUND_ERROR;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

	return SoundStartSample(sample, channel, volume, pan, loop, end_callback, data);
}

static SAMPLETAG* SoundLoadBuffer(UINT8* buf, UINT32 bufSize, ma_format format, UINT32 channels, int freq);
static BOOLEAN    SoundCleanCache(void);
static SAMPLETAG* SoundGetEmptySample(void);

/* Play a sound sample from a Smacker Flick
 *
 * Allocates space for the sound sample within the sound system
 */
UINT32 SoundPlayFromSmackBuff(const char* name, UINT8 channels, UINT8 depth, UINT32 rate, std::vector<UINT8>& buf, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
	ma_format format;

	if (buf.empty()) return SOUND_ERROR;

	//Originaly Sound Blaster could only play mono unsigned 8-bit PCM data.
	//Later it became capable of playing 16-bit audio data, but needed to be signed and LSB.
	//They were the de facto standard so I'm assuming smacker uses the same.
	if (depth == 8) format = ma_format_u8;
	else if (depth == 16) format = ma_format_s16;
	else return SOUND_ERROR;

	UINT32 uiBufferSize = buf.size();
	UINT8* inMemoryBuffer = new UINT8[uiBufferSize]{};
	memcpy(inMemoryBuffer, buf.data(), uiBufferSize);
	if (format == ma_format_s16) {
		// We expect the Endianess for the Smacker buffer to be little endian, but ma_format_s16 is native endian, so we need to do some conversion
		convertLittleEndianBufferToNativeEndianU16(inMemoryBuffer, uiBufferSize);
	}

	SAMPLETAG* s = SoundLoadBuffer(inMemoryBuffer, uiBufferSize, format, channels, rate);
	if (s == NULL) return SOUND_ERROR;

	s->pName           = name;
	s->uiPanMax        = 64;
	s->uiMaxInstances  = 1;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;

	return SoundStartSample(s, channel, volume, pan, loop, end_callback, data);
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

	channel->uiFadeVolume = std::min(uiVolume, UINT32(MAXVOLUME));
	return TRUE;
}


BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan)
{
	if (!fSoundSystemInit) return FALSE;

	SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	channel->Pan = std::min(uiPan, 127U);
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

static BOOLEAN DoesChannelRingBufferNeedService(SOUNDTAG* channel) {
	if (channel->DoneServicing) {
		return FALSE;
	}
	auto bytesToWrite = ma_pcm_rb_available_write(channel->pRingBuffer);
	// If the ring buffer is still filled more than half the way, we dont need to service the stream
	return bytesToWrite >= SOUND_RING_BUFFER_SIZE / 2;
}

static void FillRingBuffer(SOUNDTAG* channel) {
	auto sample = channel->pSample;

	try {
		if (!DoesChannelRingBufferNeedService(channel)) {
			return;
		}
		auto bytesToWrite = ma_pcm_rb_available_write(channel->pRingBuffer);
		if (bytesToWrite < 0) {
			throw std::runtime_error("Read pointer is after write pointer, this should not happen");
		}

		void* pFramesInClientFormat;
		auto result = ma_pcm_rb_acquire_write(channel->pRingBuffer, &bytesToWrite, &pFramesInClientFormat);
		if (result != MA_SUCCESS) {
			throw std::runtime_error(ST::format("ma_pcm_rb_acquire_write: {}", ma_result_description(result)).c_str());
		}
		ma_uint64 framesRead = 0;
		if (sample->pDecoder != NULL) {
			auto result = ma_decoder_seek_to_pcm_frame(sample->pDecoder, channel->Pos);
			if (result != MA_SUCCESS) {
				throw std::runtime_error(ST::format("ma_decoder_seek_to_pcm_frame: {}", ma_result_description(result)).c_str());
			}
			// We stream from file
			framesRead = ma_decoder_read_pcm_frames(sample->pDecoder, pFramesInClientFormat, bytesToWrite);
		} else if (sample->pDataConverter != NULL) {
			auto bytesPerFrame = ma_get_bytes_per_frame(sample->eInMemoryFormat, sample->uiInMemoryChannels);
			auto posInBytes = ma_data_converter_get_required_input_frame_count(sample->pDataConverter, channel->Pos) * bytesPerFrame;
			auto requiredInputFrameCount = ma_data_converter_get_required_input_frame_count(sample->pDataConverter, bytesToWrite);
			// We might not have as many bytes available
			auto availableFrames = std::min(requiredInputFrameCount * bytesPerFrame, sample->uiBufferSize - posInBytes) / bytesPerFrame;
			auto expectedOutputFrameCount = ma_data_converter_get_expected_output_frame_count(sample->pDataConverter, availableFrames);

			auto result = ma_data_converter_process_pcm_frames(
				sample->pDataConverter,
				sample->pInMemoryBuffer + posInBytes,
				&availableFrames,
				pFramesInClientFormat,
				&expectedOutputFrameCount
			);
			if (result != MA_SUCCESS) {
				throw std::runtime_error(ST::format("ma_data_converter_process_pcm_frames: {}", ma_result_description(result)).c_str());
			}
			framesRead = expectedOutputFrameCount;
		} else {
			throw std::runtime_error("Dont know how to process ring buffer");
		}
		result = ma_pcm_rb_commit_write(channel->pRingBuffer, framesRead, pFramesInClientFormat);
		if (result != MA_SUCCESS) {
			throw std::runtime_error(ST::format("ma_pcm_rb_commit_write: {}", ma_result_description(result)).c_str());
		}

		channel->Pos += framesRead;
		if (framesRead < bytesToWrite) {
			// If the sound is looped, continue to fill buffer in the next iteration
			if (channel->Loops > 1) {
				channel->Loops -= 1;
				channel->Pos = 0;
			} else {
				channel->DoneServicing = TRUE;
			}
		}
	} catch (const std::runtime_error& err) {
		STLOGE("Error processing audio stream for channel {}, sample {}, file \"{}\": {}",
			channel - pSoundList, sample - pSampleList, sample->pName, err.what());
	}
}

static int SoundServiceBuffers(void *_ptr)
{
	SLOGD("Started SoundManBufferServiceThread");
	while (1) {
		std::unique_lock<std::mutex> lk(mutexBuffersNeedService);
		conditionBuffersNeedService.wait(lk, []{
			return fBuffersNeedService || fShutdownBufferServiceThread;
		});
		if (fShutdownBufferServiceThread) {
			SLOGD("Stopped SoundManBufferServiceThread");
			return 0;
		}
		if (fBuffersNeedService) {
			for (UINT32 i = 0; i < lengthof(pSoundList); i++)
			{
				SOUNDTAG* Sound = &pSoundList[i];
				if (Sound->State == CHANNEL_PLAY) {
					FillRingBuffer(Sound);
				}
			}
			fBuffersNeedService = FALSE;
		}
		lk.unlock();
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
			STLOGD("DEAD channel {} file \"{}\" (refcount {})", i, Sound->pSample->pName, Sound->pSample->uiInstances);
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

/* Loads a sound from a buffer into the cache.
 * The sound system will take over ownership over the buffer
 *
 * Returns: The sample if successful, NULL otherwise. */
static SAMPLETAG* SoundLoadBuffer(UINT8* inMemoryBuffer, UINT32 uiBufferSize, ma_format format, UINT32 channels, int freq)
{
	try {
		SAMPLETAG* s = SoundGetEmptySample();

		// if we don't have a sample slot
		if (s == NULL)
		{
			throw std::runtime_error("sound channels are full");
		}

		ma_data_converter_config config = ma_data_converter_config_init(
			format,
			SOUND_MA_SOUND_FORMAT,
			channels,
			gTargetAudioSpec.channels,
			freq,
			gTargetAudioSpec.freq
		);
		ma_data_converter* converter = (ma_data_converter*)ma_malloc(sizeof(ma_data_converter), NULL);
		auto result = ma_data_converter_init(&config, converter);
		if (result != MA_SUCCESS) {
			throw std::runtime_error(ST::format("ma_data_converter_init: {}", ma_result_description(result)).c_str());
		}

		s->pInMemoryBuffer = inMemoryBuffer;
		s->uiBufferSize = uiBufferSize;
		s->pDataConverter = converter;
		s->eInMemoryFormat = format;
		s->uiInMemoryChannels = channels;

		s->uiFlags |= SAMPLE_ALLOCATED;

		SLOGD("SoundLoadBuffer Success");
		return s;
	} catch (const std::runtime_error& err) {
		SLOGE("SoundLoadBuffer Error: {}", err.what());
		return NULL;
	}
}

size_t MiniaudioReadProc(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead) {
	auto rwOps = (SDL_RWops*)pDecoder->pUserData;
	return SDL_RWread(rwOps, pBufferOut, sizeof(UINT8), bytesToRead);
}

ma_bool32 MiniaudioSeekProc(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin) {
	auto rwOps = (SDL_RWops*)pDecoder->pUserData;
	auto sdlOrigin = RW_SEEK_SET;
	if (origin == ma_seek_origin::ma_seek_origin_current) {
		sdlOrigin = RW_SEEK_CUR;
	}
	if (origin == ma_seek_origin::ma_seek_origin_end) {
		sdlOrigin = RW_SEEK_END;
	}

	return SDL_RWseek(rwOps, byteOffset, sdlOrigin) != -1;
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

	UINT8* inMemoryBuffer = NULL;
	SGPFile* hFile = NULL;
	SDL_RWops* rwOps = NULL;
	ma_decoder* decoder = NULL;

	try
	{
		auto isStreamed = TRUE;
		SAMPLETAG* s = SoundGetEmptySample();

		// if we don't have a sample slot
		if (s == NULL)
		{
			throw std::runtime_error("sound channels are full");
		}

		hFile = GCM->openGameResForReading(pFilename);
		rwOps = hFile->getRwOps();
		auto hFileLen = hFile->size();
		if (hFileLen <= SOUND_FILE_STREAMING_THRESHOLD) {
			// If the file length is below the streaming threshold we store the raw data in the inMemoryBuffer
			inMemoryBuffer = new UINT8[hFileLen]{};
			if (SDL_RWread(rwOps, inMemoryBuffer, sizeof(UINT8), hFileLen) != hFileLen) {
				throw std::runtime_error("Could not read the whole file");
			}
			SDL_RWclose(rwOps);
			rwOps = SDL_RWFromConstMem(inMemoryBuffer, hFileLen);
			hFile = NULL;
			isStreamed = FALSE;
		}

		// Initialize decoder to convert WAV/MP3/OGG data to raw sample data
		decoder = (ma_decoder*)ma_malloc(sizeof(ma_decoder), NULL);
		auto result = ma_decoder_init(MiniaudioReadProc, MiniaudioSeekProc, rwOps, &gTargetDecoderConfig, decoder);
		if (result != MA_SUCCESS) {
			throw std::runtime_error(ST::format("Error initializing sound decoder for file \"{}\"- {}", pFilename, ma_result_description(result)).c_str());
		}
		s->pFile = hFile;
		s->pInMemoryBuffer = inMemoryBuffer;
		s->pRWOps = rwOps;
		s->pDecoder = decoder;
		s->pName = pFilename;

		s->uiFlags |= SAMPLE_ALLOCATED;

		if (isStreamed) {
			SLOGD("SoundLoadDisk success creating file stream for \"%s\"", pFilename);
		} else {
			SLOGD("SoundLoadDisk success creating in-memory stream for \"%s\"", pFilename);
		}
		return s;
	}
	catch (const std::runtime_error& err)
	{
		SLOGE("SoundLoadDisk Error for \"%s\": %s", pFilename, err.what());
		// Clean up possible allocations
		if (hFile != NULL) {
			delete hFile;
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
		STLOGD("freeing sample {} \"{}\" with {} hits", candidate - pSampleList, candidate->pName, candidate->uiCacheHits);
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

	SLOGD("SoundFreeSample: Freeing sample %d", s - pSampleList);

	assert(s->uiInstances == 0);

	if (s->pDecoder != NULL) {
		ma_decoder_uninit(s->pDecoder);
		ma_free(s->pDecoder, NULL);
	}
	if (s->pDataConverter != NULL) {
		ma_data_converter_uninit(s->pDataConverter);
		ma_free(s->pDataConverter, NULL);
	}
	if (s->pRWOps != NULL) {
		SDL_RWclose(s->pRWOps);
	}
	// Note: s->pFile is closed and deleted by SDL_RWclose implicitly, but s->pInMemoryBuffer is not
	if (s->pInMemoryBuffer != NULL) {
		delete[] s->pInMemoryBuffer;
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

	auto ringBuffersNeedService = FALSE;

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
				const INT vol_l   = Sound->uiFadeVolume * (127 - Sound->Pan) / MAXVOLUME;
				const INT vol_r   = Sound->uiFadeVolume * (  0 + Sound->Pan) / MAXVOLUME;
				UINT32    samples = want_samples;
				const INT16* src;
				auto rbResult = ma_pcm_rb_acquire_read(Sound->pRingBuffer, &samples, (void**)&src);
				if (rbResult != MA_SUCCESS) {
					SLOGE("Could not aquire read pointer for channel %d: %s", Sound - pSoundList, ma_result_description(rbResult));
					continue;
				}

				for (UINT32 i = 0; i < samples; ++i)
				{
					gMixBuffer[2 * i + 0] += src[2 * i + 0] * vol_l >> 7;
					gMixBuffer[2 * i + 1] += src[2 * i + 1] * vol_r >> 7;
				}

				if (samples < want_samples) {
					Sound->State = CHANNEL_DEAD;
				}

				rbResult = ma_pcm_rb_commit_read(Sound->pRingBuffer, samples, (void**)src);
				if (rbResult != MA_SUCCESS) {
					SLOGE("Could not commit read pointer for channel %d: %s", Sound - pSoundList, ma_result_description(rbResult));
				} else {
					ringBuffersNeedService |= DoesChannelRingBufferNeedService(Sound);
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

	if (ringBuffersNeedService) {
		// We try to lock the mutex. If it is already locked, buffers are already serviced
		if (mutexBuffersNeedService.try_lock()) {
			fBuffersNeedService = true;
			mutexBuffersNeedService.unlock();
			conditionBuffersNeedService.notify_one();
		}
	}
}


/*
 * Initializes SDL Audio Subsystem and the channel ring buffers
 */
static BOOLEAN SoundInitHardware(void)
{
	try {
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
			throw std::runtime_error(ST::format("SDL_InitSubSystem returned error: {}", SDL_GetError()).c_str());
		}

		gTargetAudioSpec.freq     = SOUND_FREQ;
		gTargetAudioSpec.format   = SOUND_FORMAT;
		gTargetAudioSpec.channels = SOUND_CHANNELS;
		gTargetAudioSpec.samples  = SOUND_SAMPLES;
		gTargetAudioSpec.callback = SoundCallback;
		gTargetAudioSpec.userdata = NULL;

		if (SDL_OpenAudio(&gTargetAudioSpec, NULL) != 0) {
			throw std::runtime_error(ST::format("SDL_OpenAudio returned error: {}", SDL_GetError()).c_str());
		}

		gTargetDecoderConfig = ma_decoder_config_init(SOUND_MA_SOUND_FORMAT, gTargetAudioSpec.channels, gTargetAudioSpec.freq);

		std::fill(std::begin(pSoundList), std::end(pSoundList), SOUNDTAG{});
		for(auto channel = std::begin(pSoundList); channel != std::end(pSoundList); ++channel) {
			channel->pRingBuffer = (ma_pcm_rb*)ma_malloc(sizeof(ma_pcm_rb), NULL);
			ma_result result = ma_pcm_rb_init(SOUND_MA_SOUND_FORMAT, SOUND_CHANNELS, SOUND_RING_BUFFER_SIZE, NULL, NULL, channel->pRingBuffer);
			if (result != MA_SUCCESS) {
				throw std::runtime_error(ST::format(
					"ma_pcm_rb_init for channel {} returned error: {}",
					channel - pSoundList,
					ma_result_description(result)
				).c_str());
			}
		}

		fShutdownBufferServiceThread = FALSE;
		bufferServiceThread = SDL_CreateThread(SoundServiceBuffers, "SoundManBufferServiceThread", (void *)NULL);
		if (bufferServiceThread == NULL) {
			throw std::runtime_error(ST::format("SDL_CreateThread for SoundManBufferServiceThread returned error: {}", SDL_GetError()).c_str());
		}

		SDL_PauseAudio(0);
		return TRUE;

	} catch (const std::runtime_error& err) {
		STLOGE("SoundInitHardware: {}", err.what());
		SoundShutdownHardware();
		return FALSE;
	}
}


/*
 * Shutdown SDL Audio Subsystem, if initialized and the channel ring buffers if initialized
 */
static void SoundShutdownHardware(void)
{
	if (bufferServiceThread != NULL) {
		{
			std::lock_guard<std::mutex> lk(mutexBuffersNeedService);
			fShutdownBufferServiceThread = true;
		}
		conditionBuffersNeedService.notify_one();
		int returnValue = 1;
		SDL_WaitThread(bufferServiceThread, &returnValue);
		if (returnValue != 0) {
			SLOGE("SoundManBufferServiceThread exited with code: %d", returnValue);
		}
	}
	for(auto channel = std::begin(pSoundList); channel != std::end(pSoundList); ++channel) {
		if (channel->pRingBuffer != NULL) {
			ma_pcm_rb_uninit(channel->pRingBuffer);
			ma_free(channel->pRingBuffer, NULL);
			channel->pRingBuffer = NULL;
		}
	}
	if (SDL_WasInit(SDL_INIT_AUDIO) != 0) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
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

/* Starts up a sample on the specified channel. Override parameters are passed
 * in through the structure pointer pParms. Any entry with a value of 0xffffffff
 * will be filled in by the system.
 *
 * Returns: Unique sound ID if successful, SOUND_ERROR if not. */
static UINT32 SoundStartSample(SAMPLETAG* sample, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data)
{
	STLOGD("playing channel {} sample {} file \"{}\"", channel - pSoundList, sample - pSampleList, sample->pName);

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
	channel->DoneServicing = FALSE;

	// Reset ring buffer
	ma_pcm_rb_reset(channel->pRingBuffer);
	// Fill ring buffer with initial data
	FillRingBuffer(channel);

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

	STLOGD("stopping channel channel {}", (channel - pSoundList));
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
