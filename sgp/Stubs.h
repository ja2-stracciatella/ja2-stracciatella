#ifndef STUBS_H
#define STUBS_H

#include "Types.h"

typedef int WIN32_FIND_DATA;

typedef unsigned long DWORD;

typedef long LONG;


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


#if 0
void _splitpath(const char*, char*, char*, char*, char*);

int WritePrivateProfileString(const char*, const char*, const char*, const char*);
int GetLogicalDriveStrings(size_t, const char*);
int GetDriveType(const char*);
#define DRIVE_CDROM 0
#endif

size_t GetPrivateProfileString(const char* app_name, const char* key_name, const char* default_value, char* out_buf, size_t buf_size, const char* filename);


UINT32 GetTickCount(void);

void OutputDebugString(const char*);

#define INVALID_HANDLE_VALUE 0

#ifdef __linux__
/* Gcc lacks this function
 * in linux we need to add this function
 * (or my installation is lacking it nonetheless
 * it needs to be added)
 **/
size_t wcslcpy(wchar_t *s1, const wchar_t *s2, size_t n);

/* Another function which isn't present in glibc
 */
size_t strlcpy(char*,const char*,size_t);
#endif
#endif
