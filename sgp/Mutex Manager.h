#ifndef __MUTEX_
#define __MUTEX_

#include <process.h>
#include "Types.h"
#include "Local.h"

extern BOOLEAN InitializeMutexManager(void);
extern void    ShutdownMutexManager(void);
extern BOOLEAN InitializeMutex(UINT32 uiMutexIndex, UINT8 *ubMutexName);
extern BOOLEAN DeleteMutex(UINT32 uiMutexIndex);
extern BOOLEAN EnterMutex(UINT32 uiMutexIndex, INT32 nLine, char *szFilename);
extern BOOLEAN EnterMutexWithTimeout(UINT32 uiMutexIndex, UINT32 uiTimeout, INT32 nLine, char *szFilename);
extern BOOLEAN LeaveMutex(UINT32 uiMutexIndex, INT32 nLine, char *szFilename);

//
// Use defines to allocate slots in the mutex manager. Put these defines in LOCAL.H
//

#endif
