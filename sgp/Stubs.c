#include "Stubs.h"
#include <SDL.h>


void GetCursorPos(POINT* pos)
{
	int x;
	int y;

	SDL_GetMouseState(&x, &y);
	pos->x = x;
	pos->y = y;
}


UINT32 GetTickCount(void)
{
	return SDL_GetTicks();
}


size_t GetPrivateProfileString(const char* app_name, const char* key_name, const char* default_value, char* out_buf, size_t buf_size, const char* filename)
{
	FIXME
	strlcpy(out_buf, default_value, buf_size);
	return strlen(out_buf);
}


void OutputDebugString(const char* str)
{
	fputs(str, stderr);
}
