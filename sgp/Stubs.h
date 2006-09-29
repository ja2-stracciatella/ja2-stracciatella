#ifndef STUBS_H
#define STUBS_H

#include "Types.h"

#define AILCALL
#define AILCALLBACK
#define CALLBACK
#define DXDEC
#define FAR
#define PASCAL

#define _cdecl
#define __cdecl

typedef char CHAR;

typedef BYTE* LPBYTE;

typedef int AIL_DRIVER;
typedef int AILDOSDLS;
typedef int BOOL;
typedef int HANDLE;
typedef int HBITMAP;
typedef int HDC;
typedef int HHOOK;
typedef int HKEY;
typedef int HINSTANCE;
typedef int HRESULT;
typedef int HWND;
typedef int INT;
typedef int IO_PARMS;
typedef int LPARAM;
typedef int LRESULT;
typedef int MMRESULT;
typedef int PALETTEENTRY;
typedef int WIN32_FIND_DATA;
typedef int WPARAM;

typedef long long FILETIME;

typedef void* H3DSAMPLE;
typedef int HDIGDRIVER;
typedef void* HSAMPLE;
typedef void* HSTREAM;

typedef PALETTEENTRY* LPPALETTEENTRY;
typedef int* LPRGNDATA;

typedef unsigned int UINT;

typedef unsigned long DWORD;

typedef char* LPSTR;

typedef void* LPVOID;

typedef long LONG;

typedef struct
{
  int x;
  int y;
} POINT;

typedef struct
{
  int left;
  int top;
  int right;
  int bottom;
} RECT;

typedef RECT* LPRECT;

void GetCursorPos(POINT*);

#define FILE_ATTRIBUTE_DIRECTORY 0 /* XXX */

#define __max(a, b) ((a) > (b) ? (a) : (b))
#define __min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) __max(a, b)
#define min(a, b) __min(a, b)


#if 0
/* Timer_Control das gibt noch Kopfschmerzen... */
typedef void (*LPTIMECALLBACK)(UINT uiID, UINT uiMsg, DWORD uiUser, DWORD uiDw1, DWORD uiDw2);
typedef int TIMECAPS;
MMRESULT timeGetDevCaps(TIMECAPS*, size_t);
MMRESULT timeSetEvent(int, int, LPTIMECALLBACK, DWORD, int);
MMRESULT timeKillEvent(MMRESULT);
#define TIMERR_NOERROR 0
#define TIME_PERIODIC 0
#endif

#if 0
void GetSystemDirectory(char*, size_t);
void _splitpath(const char*, char*, char*, char*, char*);

int WritePrivateProfileString(const char*, const char*, const char*, const char*);
int GetLogicalDriveStrings(size_t, const char*);
int GetDriveType(const char*);
int GetVolumeInformation(const char*, const char*, size_t, UINT32*, UINT32*, UINT32*, const char*, size_t);
#define DRIVE_CDROM 0
#define ERROR_SUCCESS 0
#endif

size_t GetPrivateProfileString(const char* app_name, const char* key_name, const char* default_value, char* out_buf, size_t buf_size, const char* filename);


int GetLastError(void);
#define ERROR_NOT_READY 1
void ShowCursor(int);

UINT32 GetTickCount(void);

void OutputDebugString(const char*);

#define INVALID_HANDLE_VALUE 0

typedef void Smack;
typedef void SmackBuf;

#if 0
typedef void* LPDIRECTDRAW2;
typedef void* LPDIRECTDRAWPALETTE;
typedef void* LPDIRECTDRAWSURFACE2;
#endif

#ifdef __linux__
/* Gcc lacks this function
 * in linux we need to add this function
 * (or my installation is lacking it nonetheless
 * it needs to be added)
 **/
size_t wcslcpy(wchar_t *s1, const wchar_t *s2, size_t n);
#endif
#endif
