#ifndef SMACK_STUB_H
#define SMACK_STUB_H

#include "Types.h"
#include "SDL.h"
extern "C" {
#include "smacker.h"
}

enum
  {
    DISABLE = 0,
    ENABLE
  };

enum
{
	SMACKBUFFER555 = 0x80000000,
	SMACKBUFFER565 = 0xC0000000
};

enum
{
	SMACKAUTOBLIT = 0
};

enum
{
	SMACKFILEHANDLE = 0x01000,
	SMACKTRACK1     = 0x02000,
	SMACKTRACK2     = 0x04000,
	SMACKTRACK3     = 0x08000,
	SMACKTRACK4     = 0x10000,
	SMACKTRACK5     = 0x20000,
	SMACKTRACK6     = 0x40000,
	SMACKTRACK7     = 0x80000,
	SMACKTRACKS     = SMACKTRACK1 | SMACKTRACK2 | SMACKTRACK3 | SMACKTRACK4 | SMACKTRACK5 | SMACKTRACK6 | SMACKTRACK7,

	SMACKAUTOEXTRA = 0xFFFFFFFF
};

struct Smack
{
  smk Smacker; //object pointer type for libsmacker
  //SDL_AudioSpec smkaudiotrack;
  //union smk_read_t fp;
  char* audiobuffer;
  UINT32 Height;
  UINT32 Frames;
  UINT32 FrameNum;
  UINT32 FramesPerSecond;
  UINT32 LastTick;
  BOOLEAN VideoDelayed;
};


typedef void SmackBuf;

Smack* SmackOpen(const char* Name, UINT32 Flags, UINT32 ExtraBuf);
UINT32 SmackDoFrame(Smack* Smk);
CHAR8 SmackNextFrame(Smack* Smk);

UINT32 SmackWait(Smack* Smk);
void SmackClose(Smack* Smk);

void SmackToBuffer(Smack* Smk, UINT32 Left, UINT32 Top, UINT32 Pitch, UINT32 DestHeight, void* Buf, UINT32 Flags);

//SDL_Surface* SmackBufferOpen(UINT32 BlitType, UINT32 Width, UINT32 Height, UINT32 ZoomW, UINT32 ZoomH);
//void SmackBufferClose(SmackBuf* SBuf);

UINT32 SmackUseMMX(UINT32 Flag);

#endif
