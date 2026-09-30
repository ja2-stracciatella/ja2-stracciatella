/*********************************************************************************
* SGP Digital Sound Module
*
*		This module handles the playing of digital samples, preloaded or streamed.
*
* Derek Beland, May 28, 1997
*********************************************************************************/

#include "Debug.h"
#include "SoundMan.h"
#include "SGPFile.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <string_theory/string>

#include <algorithm>
#include <assert.h>
#include <vector>
#include <stdexcept>

// Miniaudio includes needs some defines

#define STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION

#define MA_NO_GENERATION
#define MA_NO_ENCODING
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

#undef MINIAUDIO_IMPLEMENTATION

using SOUNDTAG = ma_sound;
using SAMPLETAG = int;

static BOOLEAN fSoundSystemInit = FALSE; // Startup called
static BOOLEAN gfEnableStartup  = TRUE;  // Allow hardware to start up


void SoundEnableSound(BOOLEAN fEnable)
{
	gfEnableStartup = fEnable;
}

bool IsSoundEnabled()
{
	return gfEnableStartup;
}

//namespace {
void Require(ma_result result)
{
	if (result == MA_SUCCESS) return;

	throw std::runtime_error("");
}

ma_result MiniaudioReadProc(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t *bytesRead)
{
	auto file = reinterpret_cast<SGPFile *>(pDecoder->pUserData);

	*bytesRead = file->readAtMost(pBufferOut, bytesToRead);
	return *bytesRead == bytesToRead ? MA_SUCCESS : MA_ERROR;
}

ma_result MiniaudioSeekProc(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin)
{
	auto file = reinterpret_cast<SGPFile *>(pDecoder->pUserData);

	try {
		file->seek(static_cast<INT32>(byteOffset), [origin] {
			switch (origin)
			{
				case ma_seek_origin_current: return FileSeekMode::FILE_SEEK_FROM_CURRENT;
				case ma_seek_origin_start:   return FileSeekMode::FILE_SEEK_FROM_START;
				case ma_seek_origin_end:     return FileSeekMode::FILE_SEEK_FROM_END;
			}}());
	}
	catch (...)
	{
		return MA_ERROR;
	}
	return MA_SUCCESS;
}

class SoundEngine
{
	std::map<UINT32, ma_sound> sounds;
	ma_engine engine;


public:
	SOUNDTAG * FromID(SoundManagerID id)
	{
		auto pos = sounds.find(id);
		return pos != sounds.end() ? &pos->second : nullptr;
	}

	SoundManagerID GetUniqueID()
	{
		for (UINT32 id = 1; id < SOUND_ERROR; ++id)
		{
			if (sounds.find(id) == sounds.end())
			{
				return id;
			}
		}
		return SOUND_ERROR;
	}

	void Init([[maybe_unused]]bool noSound)
	{
		{
			auto cfg = ma_engine_config_init();
			Require(ma_engine_init(&cfg, &engine));
			Require(ma_engine_start(&engine));
		}

		{
			auto cfg = ma_resource_manager_config_init();
			cfg.pVFS = nullptr;
		}
		auto resourceMgr = ma_engine_get_resource_manager(&engine);
		//auto  ma_resource_manager_config_init();
	}

	void Uninit()
	{
		ma_engine_uninit(&engine);
	}

	ma_decoder * GetDecoder(char const * filename)
	{
		auto decoder = std::make_unique<ma_decoder>();

		SGPFile * file{ GCM->openGameResForReading(filename) };

		ma_decoder_init(MiniaudioReadProc, MiniaudioSeekProc, file, nullptr, decoder.get());

		return decoder.release();
	}

	auto GetSoundObject(ma_decoder * decoder)
	{
		for (UINT32 id = 1; id < SOUND_ERROR; ++id)
		{
			auto [ pos, created ] = sounds.try_emplace(id);
			if (created)
			{
				Require(ma_sound_init_from_data_source(&engine, decoder, 0, nullptr, &pos->second));
				return pos;
			}
		}

		throw std::logic_error("Could not create a new ma_sound object");
	}
};

static SoundEngine gEngine;

/* Searches out a sound instance referred to by its ID number.
 *
 * Returns: If the instance was found, the pointer to the channel.  NULL
 *          otherwise. */
static SOUNDTAG * SoundGetChannelByID(SoundManagerID id)
{
	return gfEnableStartup ? gEngine.FromID(id) : nullptr;
}
//}

void InitializeSoundManager(bool noSound)
{
	if (fSoundSystemInit) return;

	gEngine.Init(noSound);

	fSoundSystemInit = true;
}


static SAMPLETAG* SoundLoadBuffer(UINT8* inMemoryBuffer, UINT32 uiBufferSize, ma_format format, UINT32 channels, int freq);


void ShutdownSoundManager(void)
{
	gEngine.Uninit();
	fSoundSystemInit = FALSE;
}


//static SOUNDTAG*  SoundGetFreeChannel(void);
//static SAMPLETAG* SoundLoadSample(const char* pFilename);
static UINT32     SoundStartSample(SAMPLETAG* sample, SOUNDTAG* channel, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data);


UINT32 SoundPlay(const char* pFilename, UINT32 volume, UINT32 pan, UINT32 loop, [[maybe_unused]] void (*end_callback)(void*), [[maybe_unused]] void* data)
{
	if (!fSoundSystemInit) return SOUND_ERROR;

	auto decoder = gEngine.GetDecoder(pFilename);

	auto result = gEngine.GetSoundObject(decoder);
	auto id = result->first;
	auto * sound = &result->second;

	SoundSetPan(id, pan);
	SoundSetVolume(id, volume);
	ma_sound_set_looping(sound, loop > 1);
	ma_sound_start(sound);

	SLOGI("Playing {}; ID {}", pFilename, id);
	return id;
}

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

#if 0
	s->pName           = name;
	s->uiPanMax        = 64;
	s->uiMaxInstances  = 1;

	SOUNDTAG* const channel = SoundGetFreeChannel();
	if (channel == NULL) return SOUND_ERROR;
#endif
	SOUNDTAG * channel{};
	return SoundStartSample(s, channel, volume, pan, loop, end_callback, data);
}


UINT32 SoundPlayRandom(const char* pFilename, UINT32 time_min, UINT32 time_max, UINT32 vol_min, UINT32 vol_max, UINT32 pan_min, UINT32 pan_max, UINT32 max_instances)
{
	SLOGD("playing random Sound: \"{}\"", pFilename);

	if (!fSoundSystemInit) return SOUND_ERROR;
#if 0
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
#endif
}


BOOLEAN SoundIsPlaying(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;
#if 0
	const SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	return channel != NULL &&  channel->State != CHANNEL_FREE;
#endif
}


static BOOLEAN SoundStopChannel(SOUNDTAG* channel);


BOOLEAN SoundStop(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return FALSE;
	if (!SoundIsPlaying(uiSoundID)) return FALSE;
#if 0
	SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	SoundStopChannel(channel);
#endif
	return TRUE;
}


void SoundStopAll(void)
{
	if (!fSoundSystemInit) return;

#if 0
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
#endif
}


BOOLEAN SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume)
{
	SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	ma_sound_set_volume(channel, std::min(uiVolume, UINT32(MAXVOLUME)) / 127.0f);
	return TRUE;
}


BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan)
{
	ma_sound * const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return FALSE;

	ma_sound_set_pan(channel, std::min(uiPan, 127U) / 127.0f);
	return TRUE;
}


UINT32 SoundGetVolume(UINT32 uiSoundID)
{
	ma_sound * const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return SOUND_ERROR;

	return static_cast<UINT32>(ma_sound_get_volume(channel) * 127.0f);
}


static BOOLEAN SoundRandomShouldPlay(const SAMPLETAG* s);
static UINT32 SoundStartRandom(SAMPLETAG* s);


void SoundServiceRandom(void)
{
#if 0
	FOR_EACH(SAMPLETAG, i, pSampleList)
	{
		if (SoundRandomShouldPlay(i)) SoundStartRandom(i);
	}
#endif
}


/* Determines whether a random sound is ready for playing or not.
 *
 * Returns: TRUE if a the sample should be played. */
static BOOLEAN SoundRandomShouldPlay(const SAMPLETAG* s)
{
	return FALSE;
#if 0
	return
		s->uiFlags & SAMPLE_RANDOM &&
		s->uiTimeNext <= GetClock() &&
		s->uiInstances < s->uiMaxInstances;
#endif
}


/* Starts an instance of a random sample.
 *
 * Returns: TRUE if a new random sound was created, FALSE if nothing was done. */
static UINT32 SoundStartRandom(SAMPLETAG* s)
{
#if 0
	const UINT32 volume = s->uiVolMin + Random(s->uiVolMax - s->uiVolMin);
	const UINT32 pan    = s->uiPanMin + Random(s->uiPanMax - s->uiPanMin);

	const UINT32 uiSoundID = SoundStartSample(s, channel, volume, pan, 1, NULL, NULL);
	if (uiSoundID == SOUND_ERROR) return NO_SAMPLE;

	s->uiTimeNext =
		GetClock() +
		s->uiTimeMin +
		Random(s->uiTimeMax - s->uiTimeMin);
	return uiSoundID;
#endif
	return SOUND_ERROR;
}


void SoundStopAllRandom(void)
{
#if 0
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
#endif
}


void maResultToRuntimeError(ma_result result, const char* functionName) {
		if (result != MA_SUCCESS) {
			throw std::runtime_error(ST::format("{}: {}", functionName, ma_result_description(result)).c_str());
		}
}



UINT32 SoundGetPosition(UINT32 uiSoundID)
{
	if (!fSoundSystemInit) return 0;

	const SOUNDTAG* const channel = SoundGetChannelByID(uiSoundID);
	if (channel == NULL) return 0;

	ma_uint64 cursor;
	return ma_sound_get_cursor_in_pcm_frames(channel, &cursor) == MA_SUCCESS
		? static_cast<UINT32>(cursor)
		: 0;
}


static void SoundFreeSample(SAMPLETAG* s);


static SAMPLETAG* SoundGetCached(const char* pFilename);
static SAMPLETAG* SoundLoadDisk(const char* pFilename);


/* Tries to locate a sound by looking at what is currently loaded in the cache.
 *
 * Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
 *          in the cache. */
static SAMPLETAG* SoundGetCached(const char* pFilename)
{
#if 0
	if (pFilename[0] == '\0') return NULL; // XXX HACK0009

	FOR_EACH(SAMPLETAG, i, pSampleList)
	{
		if (i->pName.compare_i(pFilename) == 0) return i;
	}
#endif
	return NULL;
}

/* Loads a sound from a buffer into the cache.
 * The sound system will take over ownership over the buffer
 *
 * Returns: The sample if successful, NULL otherwise. */
static SAMPLETAG* SoundLoadBuffer(UINT8* inMemoryBuffer, UINT32 uiBufferSize, ma_format format, UINT32 channels, int freq)
{
#if 0
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
		maResultToRuntimeError(ma_data_converter_init(&config, NULL, converter), "ma_data_converter_init");

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
#endif
	return nullptr;
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
#if 0
	UINT8* inMemoryBuffer = NULL;
	SGPFile* hFile = NULL;
	SDL_IOStream* rwOps = NULL;
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
			if (SDL_ReadIO(rwOps, inMemoryBuffer, hFileLen) != hFileLen) {
				throw std::runtime_error("Could not read the whole file");
			}
			SDL_CloseIO(rwOps);
			rwOps = SDL_IOFromConstMem(inMemoryBuffer, hFileLen);
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
			SLOGD("SoundLoadDisk success creating file stream for \"{}\"", pFilename);
		} else {
			SLOGD("SoundLoadDisk success creating in-memory stream for \"{}\"", pFilename);
		}
		return s;
	}
	catch (const std::runtime_error& err)
	{
		SLOGE("SoundLoadDisk Error for \"{}\": {}", pFilename, err.what());
		// Clean up possible allocations
		if (hFile != NULL) {
			delete hFile;
		}
		if (rwOps != NULL) {
			SDL_CloseIO(rwOps);
		}
		if (decoder != NULL) {
			ma_free(decoder, NULL);
		}
		return NULL;
	}
#endif
}


/* Removes the least-used sound from the cache to make room.
 *
 * Returns: TRUE if a sample was freed, FALSE if none */
static SAMPLETAG * SoundCleanCache()
{
#if 0
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
		SLOGD("freeing sample {} \"{}\" with {} hits", candidate - pSampleList, candidate->pName, candidate->uiCacheHits);
		SoundFreeSample(candidate);
	}
	return candidate;
#endif
	return nullptr;
}


/* Returns an available sample. Clears out other samples if necessary
 *
 * Returns: A free sample or NULL if none are left. */
static SAMPLETAG* SoundGetEmptySample(void)
{
#if 0
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
#endif
	return NULL;
}

// Frees up a sample referred to by its index slot number.
static void SoundFreeSample(SAMPLETAG* s)
{
#if 0
	if (!(s->uiFlags & SAMPLE_ALLOCATED)) return;

	SLOGD("SoundFreeSample: Freeing sample {}", s - pSampleList);

	assert(s->uiInstances == 0);

	if (s->pDecoder != NULL) {
		ma_decoder_uninit(s->pDecoder);
		ma_free(s->pDecoder, NULL);
	}
	if (s->pDataConverter != NULL) {
		ma_data_converter_uninit(s->pDataConverter, NULL);
		ma_free(s->pDataConverter, NULL);
	}
	if (s->pRWOps != NULL) {
		SDL_CloseIO(s->pRWOps);
	}
	// Note: s->pFile is closed and deleted by SDL_RWclose implicitly, but s->pInMemoryBuffer is not
	if (s->pInMemoryBuffer != NULL) {
		delete[] s->pInMemoryBuffer;
	}
	*s = SAMPLETAG{};
#endif
}

#if 0
void SDLCALL SoundCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
	if (additional_amount <= 0) return;

	Uint8* data = SDL_stack_alloc(Uint8, additional_amount);
	if (!data) return;

	// 16-bit stereo = 2 bytes per value, 2 values per sample
	UINT32 want_bytes = static_cast<UINT32>(additional_amount);
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
				if (rbResult == MA_AT_END) {
					// Ring buffer is empty and servicing is done, channel can be freed
					Sound->State = CHANNEL_DEAD;
					continue;
				}
				if (rbResult != MA_SUCCESS) {
					SLOGE("Could not acquire read pointer for channel {}: {}", Sound - pSoundList, ma_result_description(rbResult));
					continue;
				}

				for (UINT32 i = 0; i < samples; ++i)
				{
					gMixBuffer[2 * i + 0] += src[2 * i + 0] * vol_l >> 7;
					gMixBuffer[2 * i + 1] += src[2 * i + 1] * vol_r >> 7;
				}

				rbResult = ma_pcm_rb_commit_read(Sound->pRingBuffer, samples);

				if (Sound->DoneServicing && samples == 0) {
					Sound->State = CHANNEL_DEAD;
				}

				if (rbResult != MA_SUCCESS && rbResult != MA_AT_END) {
					SLOGE("Could not commit read pointer for channel {}: {}", Sound - pSoundList, ma_result_description(rbResult));
				} else {
					ringBuffersNeedService |= DoesChannelRingBufferNeedService(Sound);
				}
			}
		}
	}

	// Clip sounds and fill the stream
	INT16* data_i16 = (INT16*)data;
	for (UINT32 i = 0; i < want_values; ++i)
	{
		if (gMixBuffer[i] >= INT16_MAX)     data_i16[i] = INT16_MAX;
		else if(gMixBuffer[i] <= INT16_MIN) data_i16[i] = INT16_MIN;
		else                                data_i16[i] = (INT16)gMixBuffer[i];
	}

	SDL_PutAudioStreamData(stream, data, additional_amount);
	SDL_stack_free(data);

	if (ringBuffersNeedService) {
		// We try to lock the mutex. If it is already locked, buffers are already serviced
		if (mutexBuffersNeedService.try_lock()) {
			fBuffersNeedService = true;
			mutexBuffersNeedService.unlock();
			conditionBuffersNeedService.notify_one();
		}
	}
}
#endif

#if 0
/*
 * Initializes SDL Audio Subsystem and the channel ring buffers
 */
static BOOLEAN SoundInitHardware(void)
{
	try {
		if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			throw std::runtime_error(ST::format("SDL_InitSubSystem returned error: {}", SDL_GetError()).c_str());
		}

		SDL_zero(gTargetAudioSpec);
		gTargetAudioSpec.freq     = SOUND_FREQ;
		gTargetAudioSpec.format   = SOUND_FORMAT;
		gTargetAudioSpec.channels = SOUND_CHANNELS;

		SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &gTargetAudioSpec, SoundCallback, nullptr);
		if (!stream) {
			throw std::runtime_error(ST::format("SDL_OpenAudioDeviceStream returned error: {}", SDL_GetError()).c_str());
		}
		gAudioStream = stream;
		gAudioDeviceID = SDL_GetAudioStreamDevice(stream);

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

		bufferServiceThread = SDL_CreateThread(SoundServiceBuffers, "SoundManBufferServiceThread", (void *)NULL);
		if (!bufferServiceThread) {
			throw std::runtime_error(ST::format("SDL_CreateThread for SoundManBufferServiceThread returned error: {}", SDL_GetError()).c_str());
		}

		SDL_ResumeAudioDevice(gAudioDeviceID);

		return TRUE;
	} catch (const std::runtime_error& err) {
		SLOGE("SoundInitHardware: {}", err.what());
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
			SLOGE("SoundManBufferServiceThread exited with code: {}", returnValue);
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
		if (gAudioStream != NULL) {
			SDL_DestroyAudioStream(gAudioStream);
			gAudioStream = NULL;
		}
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}
#endif

/* Finds an unused sound channel in the channel list.
 *
 * Returns: Pointer to a sound channel if one was found, NULL if not. */
static SOUNDTAG* SoundGetFreeChannel(void)
{
#if 0
	FOR_EACH(SOUNDTAG, i, pSoundList)
	{
		if (i->State == CHANNEL_FREE) return i;
	}
#endif
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
//	SLOGD("playing channel {} sample {} file \"{}\"", channel - pSoundList, sample - pSampleList, sample->pName);

	if (!fSoundSystemInit) return SOUND_ERROR;
#if 0
	channel->uiFadeVolume  = volume;
	channel->Loops         = loop;
	channel->Pan           = pan;
	channel->EOSCallback   = end_callback;
	channel->pCallbackData = data;
#endif
	UINT32 uiSoundID = SoundGetUniqueID();
#if 0
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
#endif
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
	ma_sound_stop(channel);
	return TRUE;
}


void SoundStopRandom(UINT32 uiSample)
{
#if 0
	// CHECK FOR VALID SAMPLE
	SAMPLETAG* const s = &pSampleList[uiSample];
	if (s->uiFlags & SAMPLE_ALLOCATED)
	{
		s->uiFlags &= ~SAMPLE_RANDOM;
	}
#endif
}

void SoundServiceStreams()
{
}
