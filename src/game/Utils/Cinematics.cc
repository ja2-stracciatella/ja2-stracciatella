//------------------------------------------------------------------------------
// Cinematics Module
//
//
//	Stolen from Nemesis by Derek Beland.
//	Originally by Derek Beland and Bret Rowden.
//
//------------------------------------------------------------------------------

#include <cmath>
#include <vector>

#include <SDL.h>
extern "C" {
#include "smacker.h"
}

#include "ContentManager.h"
#include "GameInstance.h"
#include "Cinematics.h"
#include "Debug.h"
#include "HImage.h"
#include "VObject.h"
#include "VSurface.h"
#include "SoundMan.h"

struct SMKFLIC
{
	unsigned char* file_in_memory;
	smk smacker; // object pointer type for libsmacker
	UINT32 sounds[7];
	UINT32 flags;
	UINT32 left;
	UINT32 top;
	UINT32 start_tick;
	UINT32 frame_no;
	double milliseconds_per_frame;
	signed char status;
};


// SMKFLIC flags
#define SMK_FLIC_OPEN      0x00000001 // Flic is open
#define SMK_FLIC_PLAYING   0x00000002 // Flic is playing
#define SMK_FLIC_AUTOCLOSE 0x00000008 // Close when done


static SMKFLIC gSmkList[4];


static SMKFLIC* SmkOpenFlic(const char* filename);
static SMKFLIC* SmkGetFreeFlic(void);
static void SmkSkipFrames(SMKFLIC* sf);
static void SmkBlitVideoFrame(SMKFLIC* const sf, SGPVSurface* surface);


BOOLEAN SmkPollFlics(void)
{
	BOOLEAN is_playing = false;
	FOR_EACH(SMKFLIC, sf, gSmkList)
	{
		if (!(sf->flags & SMK_FLIC_PLAYING)) continue;

		SmkSkipFrames(sf);

		if (sf->status == SMK_DONE || sf->status == SMK_ERROR)
		{
			if (sf->flags & SMK_FLIC_AUTOCLOSE)
			{
				SmkCloseFlic(sf);
			}
			else
			{
				sf->status &= ~SMK_FLIC_PLAYING;
			}
		}
		else
		{
			is_playing = true;
			SmkBlitVideoFrame(sf, FRAME_BUFFER);
		}
	}

	return is_playing;
}


void SmkInitialize(void)
{
	// assume the list contains garbage
	FOR_EACH(SMKFLIC, sf, gSmkList)
	{
		sf->file_in_memory = nullptr;
		sf->smacker = nullptr;
		FOR_EACH(UINT32, sound, sf->sounds)
		{
			*sound = NO_SAMPLE;
		}
		sf->flags = 0;
	}
}


void SmkShutdown(void)
{
	FOR_EACH(SMKFLIC, sf, gSmkList)
	{
		SmkCloseFlic(sf);
	}
}


SMKFLIC* SmkPlayFlic(const char* const filename, const UINT32 left, const UINT32 top, const BOOLEAN auto_close)
{
	char status;
	SMKFLIC* const sf = SmkOpenFlic(filename);
	if (sf == nullptr) return nullptr;

	// Set the blitting position on the screen
	sf->left = left;
	sf->top  = top;

	// Get all the audio data
	unsigned char audio_tracks;
	unsigned char audio_channels[7];
	unsigned char audio_depth[7];
	unsigned long audio_rate[7];
	std::vector<UINT8> audio[7];
	if (smk_info_audio(sf->smacker, &audio_tracks, audio_channels, audio_depth, audio_rate) == 0 && audio_tracks != 0)
	{
		status = smk_enable_all(sf->smacker, audio_tracks);
		Assert(status == 0);

		for (sf->status = smk_first(sf->smacker); sf->status != SMK_DONE; sf->status = smk_next(sf->smacker))
		{
			if (sf->status == SMK_ERROR)
			{
				SLOGW("smacker failed to decode audio data");
				break;
			}
			for (uint8_t i = 0; i < 7; i++)
			{
				if (!(audio_tracks & (1 << i))) continue;
				unsigned long audio_size = smk_get_audio_size(sf->smacker, i);
				if (audio_size > 0)
				{
					unsigned char* audio_data = smk_get_audio(sf->smacker, i);
					audio[i].insert(audio[i].end(), audio_data, audio_data + audio_size);
				}
			}
		}
	}

	// get play speed
	// the video is too slow using microsecond resolution, speed it up by rounding down to milliseconds
	double microseconds_per_frame;
	status = smk_info_all(sf->smacker, nullptr, nullptr, &microseconds_per_frame);
	Assert(status == 0);
	sf->milliseconds_per_frame = microseconds_per_frame / 1000.0;


	// Start playing
	status = smk_enable_all(sf->smacker, SMK_VIDEO_TRACK);
	Assert(status == 0);
	sf->status = smk_first(sf->smacker);
	for (uint8_t i = 0; i < 7; i++)
	{
		if (audio[i].empty() || !IsSoundEnabled())
		{
			sf->sounds[i] = NO_SAMPLE;
		}
		else
		{
			ST::string name = ST::format("{}@{}", i, filename);
			sf->sounds[i] = SoundPlayFromSmackBuff(name.c_str(), audio_channels[i], audio_depth[i], audio_rate[i], audio[i], MAXVOLUME, 64, 1, nullptr, nullptr);
		}
	}

	// We have started to play the flick, so set start time and frame number
	sf->start_tick = SDL_GetTicks();
	sf->frame_no = 0;
	// We're now playing, flag the flic for the poller to update
	sf->flags |= SMK_FLIC_PLAYING;
	if (auto_close) sf->flags |= SMK_FLIC_AUTOCLOSE;

	return sf;
}


static SMKFLIC* SmkOpenFlic(const char* const filename)
{
	Assert(filename != nullptr);
	SMKFLIC* const sf = SmkGetFreeFlic();

	try
	{
		if (!sf) throw std::runtime_error("no free slots");

		Assert(sf->file_in_memory == nullptr);
		Assert(sf->smacker == nullptr);

		// read file to memory
		AutoSGPFile file(GCM->openGameResForReading(filename));
		unsigned long bytes = file->size();
		if (bytes == 0) throw std::runtime_error("empty file");
		sf->file_in_memory = new unsigned char[bytes]{};
		file->read(sf->file_in_memory, bytes);

		// open with smacker
		sf->smacker = smk_open_memory(sf->file_in_memory, bytes);
		if (sf->smacker == nullptr) throw std::runtime_error("smk_open_memory failed");
		sf->flags |= SMK_FLIC_OPEN;
		return sf;
	}
	catch (const std::runtime_error& ex)
	{

		SLOGE("Failed to open '{}': {}", filename, ex.what());
		if (sf != nullptr) SmkCloseFlic(sf);
		return nullptr;
	}
	return sf;
}


void SmkCloseFlic(SMKFLIC* const sf)
{
	Assert(sf != nullptr);
	FOR_EACH(UINT32, sound, sf->sounds)
	{
		if (*sound != NO_SAMPLE)
		{
			SoundStop(*sound);
			*sound = NO_SAMPLE;
		}
	}
	if (sf->smacker != nullptr)
	{
		smk_close(sf->smacker);
		sf->smacker = nullptr;
	}
	if (sf->file_in_memory != nullptr)
	{
		delete[] sf->file_in_memory;
		sf->file_in_memory = nullptr;
	}
	sf->flags = 0;
}


static SMKFLIC* SmkGetFreeFlic(void)
{
	FOR_EACH(SMKFLIC, sf, gSmkList)
	{
		if (!(sf->flags & SMK_FLIC_OPEN)) return sf;
	}
	return nullptr;
}


static void SmkSkipFrames(SMKFLIC* sf)
{
	// get target frame
	UINT32 milliseconds = SDL_GetTicks() - sf->start_tick;
	UINT32 frame_no = static_cast<UINT32>(milliseconds / sf->milliseconds_per_frame);

	// skip until the target frame (video repeats if there is a ring frame)
	while (sf->status != SMK_ERROR && sf->status != SMK_DONE && sf->frame_no != frame_no)
	{
		sf->status = smk_next(sf->smacker);
		sf->frame_no++;
	}
}


static void SmkBlitVideoFrame(SMKFLIC* const sf, SGPVSurface* surface)
{
	// get frame (source)
	// TODO handle flags SMK_FLAG_Y_* (I need a sample of each case)
	unsigned char* src;
	unsigned char* src_palette;
	unsigned long src_width;
	unsigned long src_height;
	src = smk_get_video(sf->smacker);
	if (src == nullptr) return;
	src_palette = smk_get_palette(sf->smacker);
	if (src_palette == nullptr) return;
	if (static_cast<signed char>(smk_info_video(sf->smacker, &src_width, &src_height, nullptr)) < 0) return;

	// convert palette
	UINT16 palette[256];
	for (int i = 0; i < 256; i++)
	{
		unsigned char* rgb = src_palette + i * 3;
		palette[i] = Get16BPPColor(FROMRGB(rgb[0], rgb[1], rgb[2]));
	}

	// get surface (destination)
	SGPVSurface::Lock lock(surface);
	UINT16* dst = lock.Buffer<UINT16>();
	Assert(lock.Pitch() % 2 == 0);
	Assert(surface->BPP() == 16);
	UINT32 dst_pitch = lock.Pitch() / 2; // pitch in pixels
	UINT16 dst_height = surface->Height();

	// blit the intersection
	unsigned long y_end = sf->top >= dst_height ? 0 : std::min(src_height, (unsigned long) (dst_height - sf->top));
	unsigned long x_end = sf->left >= dst_pitch ? 0 : std::min(src_width, (unsigned long) (dst_pitch - sf->left));
	dst += sf->left + sf->top * dst_pitch;
	for (unsigned long y = 0; y < y_end; y++)
	{
		for (unsigned long x = 0; x < x_end; x++)
		{
			dst[x] = palette[src[x]];
		}
		dst += dst_pitch;
		src += src_width;
	}
}
