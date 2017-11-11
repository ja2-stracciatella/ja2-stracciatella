
#if !defined(_MSC_VER)
	#include <strings.h>
#endif
#include "Smack_Stub.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "FileMan.h"

extern "C" {
#include "smacker.h"
}

#define SMKTRACK 0

typedef unsigned char   UCHAR8;

BOOLEAN SmackCheckStatus(CHAR8 smkstatus)
{
	if (smkstatus <0)
	{
		printf ("SmackLibrary returned an error!\n");
		return TRUE;
	}
	return FALSE;
}

BOOLEAN SmkVideoSwitch(smk SmkObj, BOOLEAN sw)
{
	return (SmackCheckStatus(smk_enable_video(SmkObj, sw)));
} 

BOOLEAN SmkAudioSwitch(smk SmkObj, BOOLEAN sw)
{
	return SmackCheckStatus(smk_enable_audio(SmkObj, SMKTRACK, sw));
}

void SmackPrintFlickInfo(unsigned long width, unsigned long height, UCHAR8 scale, unsigned long framecount, 
			DOUBLE usf, UCHAR8 a_channels, UCHAR8 a_depth, UCHAR8 a_rate)
{
	printf ("Video -- Frames: %lu Width: %lu Height: %lu Scale: %d \n", framecount, width, height, scale);
	printf ("Audio -- FPS: %2.2f  Channels: %d Depth: %d Rate %d\n", usf/1000, a_channels, a_depth, a_rate);
}

// read all smackaudio and convert it to 22050Hz on the fly (44100 originally)
UINT32 SmackGetAudio(const smk SmkObj, const INT16* audiobuffer)
{
	UINT32 n_samples = 0;
	UINT32 i, index;
	INT16 *paudio = (INT16*) audiobuffer;
	if (!audiobuffer)
		return 0;
	SmkAudioSwitch(SmkObj, ENABLE);
	smk_first(SmkObj);
	do
	{
		const UINT32 smacklen = smk_get_audio_size (SmkObj, SMKTRACK);
		const INT16 *smackaudio = (INT16*) smk_get_audio (SmkObj, SMKTRACK);

		memcpy(paudio, smackaudio, smacklen);
		paudio += smacklen / 2;

		n_samples += smacklen / 2;
	}
	while(smk_next(SmkObj) != SMK_DONE);
	SmkAudioSwitch(SmkObj, DISABLE);
	return n_samples;
}

void SmackWriteAudio(INT16* abuffer, UINT32 size)
{
	FILE* fp = fopen("/tmp/smk.raw", "wb");
	fwrite(abuffer, size, 1, fp);
	fclose(fp);
}

UCHAR8* SmackToMemory(SGPFile* File)
{
	UCHAR8* smacktomemory;
	UINT32 FileSize=FileGetSize(File);

	smacktomemory = (UCHAR8*) malloc (FileSize);
	if(!smacktomemory)
		return NULL;
	FileRead(File, smacktomemory, FileSize);
	return smacktomemory;
}


Smack* SmackOpen(SGPFile* FileHandle, UINT32 Flags, UINT32 ExtraFlag)
{
	Smack* flickinfo;
	// smacklib info types
	unsigned long frame, framecount, width, height;
	UCHAR8 scale;
	DOUBLE usf;
	CHAR8 smkstatus;
	/* arrays for audio track metadata */
	UCHAR8 a_depth[7], a_channels[7];
	unsigned long a_rate[7];
	unsigned long audiolen, audiosamples;
	INT16* audiobuffer;
	UCHAR8* smackloaded;
	UINT32 smacksize = FileGetSize(FileHandle);
	flickinfo = (Smack*) malloc(sizeof(Smack));
	if(!flickinfo)
		return NULL;

	smackloaded = SmackToMemory (FileHandle);
	if (!smackloaded)
	{
		free(flickinfo);
		return NULL;
	}
	flickinfo->SmackerInMemory = smackloaded;
	flickinfo->Smacker = smk_open_memory(smackloaded, smacksize);
	//open file with given filehandle DISK/MEMORY mode
	//flickinfo->Smacker = smk_open_generic(1, fp, 0, SMK_MODE_DISK);
	//flickinfo->Smacker = smk_open_generic(1, fp, 0, SMK_MODE_MEMORY);
	if(!flickinfo->Smacker)
	{
		free(flickinfo);
		return NULL;
	}

	smkstatus = smk_info_video(flickinfo->Smacker, &width, &height, &scale);
	SmackCheckStatus(smkstatus);
	smkstatus = smk_info_all(flickinfo->Smacker, &frame, &framecount, &usf);
	SmackCheckStatus(smkstatus);
	smkstatus = smk_info_audio(flickinfo->Smacker, NULL, a_channels, a_depth, a_rate);
	SmackCheckStatus(smkstatus);

	SmkVideoSwitch(flickinfo->Smacker, DISABLE);

	flickinfo->Frames=framecount;
	flickinfo->Height=height;
	flickinfo->Width=width;
	flickinfo->FramesPerSecond = usf;
	// calculated audio memory for downsampling 44100->22050
	audiosamples = ((flickinfo->Frames / (usf/1000)) * (a_rate[SMKTRACK]) * 16 *  a_channels[SMKTRACK]);
	audiobuffer = (INT16*) malloc(audiosamples);
	if (!audiobuffer)
	{
		free(flickinfo);
		return NULL;
	}
	audiolen = SmackGetAudio(flickinfo->Smacker, audiobuffer);
	//SmackWriteAudio( audiobuffer, audiolen); // are getting right audio data?
	// shoot and forget... audiobuffer should be freed by SoundMan
	if (audiolen > 0)
		flickinfo->SoundTag = SoundPlayFromBuffer(audiobuffer, audiolen, MAXVOLUME, 64, 1, NULL, NULL);
	else free(audiobuffer), audiobuffer = NULL;
	SmkVideoSwitch  (flickinfo->Smacker, ENABLE);
	if ((smk_first(flickinfo->Smacker) < 0))
	{
		printf("First Failed!");
		return NULL;
	}
	smk_first(flickinfo->Smacker);
	flickinfo->LastTick = SDL_GetTicks();
	return flickinfo;
}


UINT32 SmackDoFrame(Smack* Smk)
{
	UINT32 i=0;
	// wait for FPS milliseconds
	UINT16 millisecondspassed = SDL_GetTicks() - Smk->LastTick;
	UINT16 skiptime;
	UINT16 delay, skipframes = 0;
	DOUBLE framerate = Smk->FramesPerSecond/1000;

	if(framerate > millisecondspassed)
	{
		delay = framerate-millisecondspassed;
	}
	else // video is delayed - so skip frames according to delay but take fps into account
	{
		skipframes = millisecondspassed / (UINT16)framerate;
		delay = millisecondspassed % (UINT16)framerate;
		//bigskiptime:
		skiptime = SDL_GetTicks();
		millisecondspassed = skiptime - Smk->LastTick;
		while (skipframes > 0)
		{
			SmackNextFrame(Smk);
			skipframes--;
			i++;
		}
		skiptime = SDL_GetTicks() - skiptime;
		if(skiptime+delay <= i*framerate)
		{
			delay =  i*(framerate)-skiptime-delay;
		}
		else
		{
			delay =  0;
			// need to find a nice way to compensate for lagging video
			//Smk->LastTick = SDL_GetTicks();
			//skipframes = skiptime+delay / (UINT16)framerate;
			//delay = (skiptime+delay) % (UINT16)framerate;
			//i=0;
			//goto bigskiptime; // skiptime was big.. so just go on skipping frames
		}
	}
	SDL_Delay(delay);
	Smk->LastTick = SDL_GetTicks();
	return 0;
}



CHAR8 SmackNextFrame(Smack* Smk)
{
	CHAR8 smkstatus;
	smkstatus = smk_next(Smk->Smacker);
	return smkstatus;
}

UINT32 SmackWait(Smack* Smk)
{
	return 0;
}


void SmackClose(Smack* Smk)
{
	if(!SoundStop(Smk->SoundTag))
		printf("Error in SmackClose SoundStop\n");
	free(Smk->SmackerInMemory);
	smk_close(Smk->Smacker); // closes and frees Smacker Object and file
}

void SmackToBuffer(Smack* Smk, UINT32 Left, UINT32 Top, UINT32 Pitch, UINT32 DestHeight, UINT32 DestWidth, void* Buf, UINT32 Flags)
{
	unsigned char* smackframe, *pframe;
	unsigned char* smackpal;
	UINT16 i,j,pixel,*buf;
	UINT8 *rgb;
	UINT32 halfpitch = Pitch / 2;
	smackframe = smk_get_video(Smk->Smacker);
	smackpal = smk_get_palette (Smk->Smacker);
	// dump_bmp (smackpal, smackframe, 640, 480, Smk->FrameNum);
	buf=(UINT16*)Buf;
	pframe=smackframe;
	// hardcoded copy to frambuffer without taking sdl methods into account.
	// maybe need to find a way to blit it later
	if (Flags == SMACKBUFFER565)
	{
		buf += Left + Top*halfpitch;
		for (i =0; i < DestHeight ; i++)
		{
			for (j = 0; j <DestWidth; j++)
			{
				rgb = &smackpal[*pframe++*3] ;
				*buf++ = (rgb[0]>>3)<<11 | (rgb[1]>>2)<<5 | rgb[2]>>3;
			}
			buf+=halfpitch-DestWidth;
		}
	}
	else // SMACKBUFFER555
	{
		for (i =0; i < DestHeight ; i++)
		{
			for (j = 0; j < DestWidth; j++)
			{
				// get rgb offset of palette
				rgb = &smackpal[pframe[0]*3] ;
				// convert from rbg to rgb555 0=red 1=green 2=blue
				pixel = (rgb[0]>>3)<<10 | (rgb[1]>>2)<<5 | rgb[2]>>3;
				buf[(j+Top)+(i+Left)*halfpitch] = pixel;
				pframe++;
			}
		}
	}
}

// not needed for now... 
/*
SDL_Surface* SmackBufferOpen(UINT32 BlitType, UINT32 Width, UINT32 Height, UINT32 ZoomW, UINT32 ZoomH)
{
	SDL_Surface* buffer;
	buffer = SDL_CreateRGBSurface(BlitType, Width, Height, 16, 0, 0, 0, 0);
	if(!buffer)
	{
		fprintf(stderr, "CreateRGBSurface failed: %s       ", SDL_GetError());
		exit (1);
	}
	return buffer;
}
*/

/*
void SmackBufferClose(SmackBuf* SBuf)
{
	free (SBuf);
}
*/


UINT32 SmackUseMMX(UINT32 Flag)
{
	return 0;
}
