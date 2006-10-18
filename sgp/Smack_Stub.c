#include "Smack_Stub.h"


Smack* SmackOpen(const char* Name, UINT32 Flags, UINT32 ExtraBuf)
{
	return NULL;
}


UINT32 SmackDoFrame(Smack* Smk)
{
	return 0;
}


void SmackNextFrame(Smack* Smk)
{
}


UINT32 SmackWait(Smack* Smk)
{
	return 0;
}


void SmackClose(Smack* Smk)
{
}


void SmackToBuffer(Smack* Smk, UINT32 Left, UINT32 Top, UINT32 Pitch, UINT32 DestHeight, const void* Buf, UINT32 Flags)
{
}


void SmackGoto(Smack* Smk, UINT32 Frame)
{
}


SmackBuf* SmackBufferOpen(HWND Wnd, UINT32 BlitType, UINT32 Width, UINT32 Height, UINT32 ZoomW, UINT32 ZoomH)
{
	return NULL;
}


void SmackBufferClose(SmackBuf* SBuf)
{
}


UINT32 SmackUseMMX(UINT32 Flag)
{
	return 0;
}


UINT8 SmackSoundUseMSS(void* DigDriver)
{
	return 0;
}
