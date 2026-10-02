/*********************************************************************************
* SGP Digital Sound Module
*
*		This module handles the playing of digital samples, preloaded or streamed.
*
* Derek Beland, May 28, 1997
*********************************************************************************/

#include "SoundMan.h"
#include "SGPFile.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"
#include "Types.h"

#include <string_theory/format>
#include <algorithm>
#include <stdexcept>
#include <variant>
#include <vector>

// Miniaudio includes needs some defines

#define STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_GENERATION
#define MA_NO_ENCODING
#define MA_NO_RESOURCE_MANAGER
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

static BOOLEAN fSoundSystemInit = FALSE; // Startup called

bool IsSoundEnabled()
{
	return fSoundSystemInit;
}

//namespace {
constexpr float SoundLibsConversionFactor{ 127.0f };
// Convert an integer in the range [0..127] as used by the Miles Sound System
// to a float in the range [0..1] as used by miniaudio.
constexpr float MSStoMA(UINT32 value)
{
	return float(std::min(value, 127U)) / SoundLibsConversionFactor;
}

static void Require(ma_result result, char const * functionName)
{
	if (result == MA_SUCCESS) return;

	throw std::runtime_error(ST::format("miniaudio call of {} failed: {}",
		functionName, ma_result_description(result)).c_str());
}


struct SoundObject
{
	struct audio_buffer
	{
		ma_audio_buffer audioBuffer;
		std::vector<UINT8> rawPCMData;
	};

	ma_sound sound;
	std::variant<ma_decoder, audio_buffer> dataSource;

	void (*endCallback)(void *);
	void * callbackUserData; // Points to the memory buffer is the data source is an audio buffer

	~SoundObject()
	{
		ma_sound_uninit(&sound);
		if (dataSource.index() == 0)
		{
			ma_decoder & decoder{ std::get<ma_decoder>(dataSource) };
			ma_decoder_uninit(&decoder);
			DeleteSGPFile(reinterpret_cast<SGPFile *>(decoder.pUserData));
		}
		else
		{
			ma_audio_buffer_uninit(&std::get<audio_buffer>(dataSource).audioBuffer);
		}
	}
};

std::map<SoundManagerID, SoundObject> sounds;
ma_engine engine;


static void Start(SoundObject & so, UINT32 volume, UINT32 pan)
{
	ma_sound_set_volume(&so.sound, MSStoMA(volume));
	ma_sound_set_pan(&so.sound, MSStoMA(pan));
	ma_sound_start(&so.sound);
}


static ma_result MiniaudioReadProc(ma_decoder * pDecoder,
	void * pBufferOut, size_t bytesToRead, size_t * bytesRead)
{
	auto * file{ reinterpret_cast<SGPFile *>(pDecoder->pUserData) };

	try {
		*bytesRead = file->readAtMost(pBufferOut, bytesToRead);
	}
	catch (...) {
		return MA_IO_ERROR;
	}
	return MA_SUCCESS;
}

static ma_result MiniaudioSeekProc(ma_decoder * pDecoder,
	ma_int64 byteOffset, ma_seek_origin origin)
{
	auto * file{ reinterpret_cast<SGPFile *>(pDecoder->pUserData) };

	try
	{
		file->seek(static_cast<INT32>(byteOffset), [origin] {
			switch (origin)
			{
				default:
				case ma_seek_origin_current: return FILE_SEEK_FROM_CURRENT;
				case ma_seek_origin_start:   return FILE_SEEK_FROM_START;
				case ma_seek_origin_end:     return FILE_SEEK_FROM_END;
			}}());
	}
	catch (...)
	{
		return MA_BAD_SEEK;
	}
	return MA_SUCCESS;
}


static ma_sound * FromID(SoundManagerID id)
{
	auto pos{ sounds.find(id) };
	return pos != sounds.end() ? &pos->second.sound : nullptr;
}


static void InitDataSource(ma_data_source * source, SoundObject & so)
{
	Require(ma_sound_init_from_data_source(&engine, source, 0, nullptr,
		&so.sound), "ma_sound_init_from_data_source");
}


static auto GetSoundObject()
{
	static SoundManagerID currentID = 1;

	auto [ pos, created ] { sounds.try_emplace(currentID++) };
	if (created) return pos;

	throw std::logic_error("Could not create a new ma_sound object");
}

// Create a new sound object for a SGPFile.
static auto GetSoundObject(char const * filename)
{
	auto pos{ GetSoundObject() };
	SoundObject & so{ pos->second };
	SGPFile * file{ GCM->openGameResForReading(filename) };

	so.dataSource = ma_decoder{};
	auto * decoder{ &std::get<ma_decoder>(so.dataSource) };

	Require(ma_decoder_init(MiniaudioReadProc, MiniaudioSeekProc, file,
		nullptr, decoder), "ma_decoder_init");

	InitDataSource(decoder, so);
	return pos;
}

// Create a new sound object for a memory buffer of raw PCM data.
static auto GetSoundObject(std::vector<UINT8> & rawData, ma_format format,
	UINT32 channels, UINT32 rate)
{
	auto pos{ GetSoundObject() };
	SoundObject & so{ pos->second };
	ma_uint64 frames{ (rawData.size() / channels) / (format == ma_format_s16 ? 2 : 1) };

	so.dataSource = SoundObject::audio_buffer{};
	auto & audioBuffer{ std::get<SoundObject::audio_buffer>(so.dataSource) };

	// The audio object takes ownership of the underlying memory.
	audioBuffer.rawPCMData = std::move(rawData);

	auto config { ma_audio_buffer_config_init(format,
		channels, frames, audioBuffer.rawPCMData.data(), nullptr) };
	config.sampleRate = rate;

	Require(ma_audio_buffer_init(&config, &audioBuffer.audioBuffer), "ma_audio_buffer_init");

	InitDataSource(&audioBuffer.audioBuffer, so);
	return pos;
}


/* Searches out a sound instance referred to by its ID number.
 *
 * Returns: If the instance was found, the pointer to the channel.  NULL
 *          otherwise. */
static ma_sound * SoundGetByID(SoundManagerID id)
{
	return fSoundSystemInit ? FromID(id) : nullptr;
}
//}

void InitializeSoundManager(bool noSound)
{
	if (fSoundSystemInit || noSound) return;

	auto cfg { ma_engine_config_init() };
	Require(ma_engine_init(&cfg, &engine), "ma_engine_config_init");

	fSoundSystemInit = true;
}


void ShutdownSoundManager(void)
{
	SoundStopAll();
	ma_engine_uninit(&engine);
	fSoundSystemInit = false;
}


SoundManagerID SoundPlay(const char * pFilename, UINT32 volume, UINT32 pan,
	UINT32 loop, void (*end_callback)(void *), void * data)
{
	if (!fSoundSystemInit) return SOUND_ERROR;

	auto && [ id, soundObj] { *GetSoundObject(pFilename) };
	ma_sound * sound{ &soundObj.sound };

	ma_sound_set_looping(sound, loop > 1);

	if (end_callback)
	{
		soundObj.endCallback = end_callback;
		soundObj.callbackUserData = data;

		ma_sound_set_end_callback(sound, [](void * userData, ma_sound *)
		{
			auto cbd  { reinterpret_cast<SoundObject *>(userData) };
			cbd->endCallback(cbd->callbackUserData);
		}, &soundObj);
	}

	Start(soundObj, volume, pan);
	return id;
}

/* Play a sound sample from a Smacker Flick
 *
 * Allocates space for the sound sample within the sound system
 */
SoundManagerID SoundPlayRawPCMData(char const * name, UINT8 channels,
	UINT8 depth, UINT32 rate, std::vector<UINT8> & buf, UINT32 volume, UINT32 pan)
{
	SLOGI("SoundPlayRawPCMData {}", name);

	if (buf.empty()) return SOUND_ERROR;

	//Originaly Sound Blaster could only play mono unsigned 8-bit PCM data.
	//Later it became capable of playing 16-bit audio data, but needed to be signed and LSB.
	//They were the de facto standard so I'm assuming smacker uses the same.
	ma_format format;
	if (depth == 8) format = ma_format_u8;
	else if (depth == 16) format = ma_format_s16;
	else return SOUND_ERROR;

	if (format == ma_format_s16) {
		// We expect the Endianess for the Smacker buffer to be little endian, but ma_format_s16 is native endian, so we need to do some conversion
		convertLittleEndianBufferToNativeEndianU16(buf.data(), buf.size());
	}

	auto && [ id, soundObject] { *GetSoundObject(buf, format, channels, rate) };

	Start(soundObject, volume, pan);
	return id;
}


bool SoundIsPlaying(SoundManagerID id)
{
	ma_sound * const sound { SoundGetByID(id) };
	return sound && ma_sound_is_playing(sound);
}


bool SoundStop(SoundManagerID id)
{
	return sounds.erase(id) != 0;
}


void SoundStopAll(void)
{
	sounds.clear();
}


bool SoundSetVolume(SoundManagerID id, UINT32 uiVolume)
{
	ma_sound * const sound { SoundGetByID(id) };
	if (!sound) return FALSE;

	ma_sound_set_volume(sound, MSStoMA(uiVolume));
	return TRUE;
}


bool SoundSetPan(SoundManagerID id, UINT32 uiPan)
{
	ma_sound * const sound { SoundGetByID(id) };
	if (!sound) return FALSE;

	ma_sound_set_pan(sound, MSStoMA(uiPan));
	return TRUE;
}


UINT32 SoundGetVolume(SoundManagerID id)
{
	ma_sound * const sound { SoundGetByID(id) };
	if (!sound) return SOUND_ERROR;

	return static_cast<UINT32>(ma_sound_get_volume(sound) * SoundLibsConversionFactor);
}


UINT32 SoundPlayRandom(const char* pFilename, [[maybe_unused]] UINT32 time_min, [[maybe_unused]] UINT32 time_max, [[maybe_unused]] UINT32 vol_min,
[[maybe_unused]] UINT32 vol_max,[[maybe_unused]] UINT32 pan_min, [[maybe_unused]] UINT32 pan_max)
{
	// TODO: implement this
	SLOGI("playing random Sound: \"{}\"", pFilename);

	if (!fSoundSystemInit) return SOUND_ERROR;

	return SOUND_ERROR;
}


void SoundStopRandom(UINT32)
{
	// TODO: implement this
}


void SoundStopAllRandom()
{
	// TODO: implement this.
}


void SoundServiceRandom()
{
	// TODO: implement this
}


UINT32 SoundGetPosition(SoundManagerID id)
{
	ma_sound * const sound { SoundGetByID(id) };
	if (!sound) return 0;

	float cursor;
	return ma_sound_get_cursor_in_seconds(sound, &cursor) == MA_SUCCESS
		? static_cast<UINT32>(cursor * 1000.0f)
		: 0;
}


void SoundServiceStreams()
{
	// Garbage collect all sounds that have already finished playing.
	for (auto it{ sounds.begin() }; it != sounds.end(); )
	{
		if (ma_sound_at_end(&it->second.sound))
			it = sounds.erase(it);
		else
			++it;
	}
}
