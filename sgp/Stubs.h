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

typedef int AIL_DRIVER;
typedef int AILDOSDLS;
typedef int BOOL;
typedef int FILETIME;
typedef int HANDLE;
typedef int HDC;
typedef int HINSTANCE;
typedef int HRESULT;
typedef int HWND;
typedef int IO_PARMS;
typedef int MMRESULT;
typedef int RECT;

typedef int* LPPALETTEENTRY;
typedef int* LPRGNDATA;

typedef unsigned int UINT;

typedef unsigned long DWORD;

typedef RECT* LPRECT;

typedef void* LPVOID;

typedef long LONG;

typedef struct
{
  int x;
  int y;
} POINT;

void GetCursorPos(POINT*);

#define FILE_ATTRIBUTE_DIRECTORY 0 /* XXX */

#define __max(a, b) ((a) > (b) ? (a) : (b))
#define __min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) __max(a, b)
#define min(a, b) __min(a, b)


/* Timer_Control das gibt noch Kopfschmerzen... */
typedef void (*LPTIMECALLBACK)(UINT uiID, UINT uiMsg, DWORD uiUser, DWORD uiDw1, DWORD uiDw2);
typedef int TIMECAPS;
MMRESULT timeGetDevCaps(TIMECAPS*, size_t);
MMRESULT timeSetEvent(int, int, LPTIMECALLBACK, DWORD, int);
MMRESULT timeKillEvent(MMRESULT);
#define TIMERR_NOERROR 0
#define TIME_PERIODIC 0

void GetSystemDirectory(char*, size_t);
void _splitpath(const char*, char*, char*, char*, char*);

int GetPrivateProfileString(const char*, const char*, const char*, const char*, size_t, const char*);
int WritePrivateProfileString(const char*, const char*, const char*, const char*); 
int GetLogicalDriveStrings(size_t, const char*);
int GetDriveType(const char*);
int GetVolumeInformation(const char*, const char*, size_t, UINT32*, UINT32*, UINT32*, const char*, size_t);
#define DRIVE_CDROM 0
#define ERROR_SUCCESS 0

int GetLastError(void);
#define ERROR_NOT_READY 1
void ShowCursor(int);

#endif
