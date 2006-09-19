#include "Stubs.h"
#include <SDL.h>
#include <stdio.h>


void GetCursorPos(POINT* pos)
{
	FIXME
	pos->x = 0;
	pos->y = 0;
}


UINT32 GetTickCount(void)
{
	return SDL_GetTicks();
}


void OutputDebugString(const char* str)
{
	fprintf(stderr, "%s: %s\n", __func__, str);
}
