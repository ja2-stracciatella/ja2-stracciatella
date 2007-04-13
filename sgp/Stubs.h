#ifndef STUBS_H
#define STUBS_H

#include "Types.h"

typedef unsigned long DWORD;


typedef struct
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;


typedef struct
{
  int x;
  int y;
} POINT;

void GetCursorPos(POINT*);

#define FILE_ATTRIBUTE_DIRECTORY 0 /* XXX */


size_t GetPrivateProfileString(const char* app_name, const char* key_name, const char* default_value, char* out_buf, size_t buf_size, const char* filename);


UINT32 GetTickCount(void);

void OutputDebugString(const char*);

#endif
