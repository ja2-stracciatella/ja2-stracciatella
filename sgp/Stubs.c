#include "Stubs.h"
#include <SDL.h>
#include <stdio.h>


void GetCursorPos(POINT* pos)
{
	UNIMPLEMENTED();
}


UINT32 GetTickCount(void)
{
	return SDL_GetTicks();
}


void OutputDebugString(const char* str)
{
	fprintf(stderr, "%s: %s\n", __func__, str);
}
