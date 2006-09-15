#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Mutex_Manager.h"
	#include "Debug.h"
	#include "Local.h"
	#include <SDL.h>
#endif
//#define __MUTEX_TYPE
/* __MUTEX_TYPE not ported, but else-case ported 
 * TODO: Port complex case, using SDL_semaphore
 */
#ifdef __MUTEX_TYPE

//
// Use defines to allocate slots in the mutex manager. Put these defines in LOCAL.H
//

HANDLE MutexTable[MAX_MUTEX_HANDLES];

BOOLEAN InitializeMutexManager(void)
{
  UINT32 uiIndex;

  //
  // Register the Mutex Manager debug topic
  //

  RegisterDebugTopic(TOPIC_MUTEX, "Mutex Manager");
  DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "Initializing the Mutex Manager");

  //
  // Initialize the table of mutex handles to NULL
  //

  for (uiIndex = 0; uiIndex < MAX_MUTEX_HANDLES; uiIndex++)
  {
    MutexTable[uiIndex] = NULL;
  }

  return TRUE;
}

void ShutdownMutexManager(void)
{
  UINT32 uiIndex;

  DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "Shutting down the Mutex Manager");

  //
  // Make sure all mutex handles are closed
  //

  for (uiIndex = 0; uiIndex < MAX_MUTEX_HANDLES; uiIndex++)
  {
    if (MutexTable[uiIndex] != NULL)
    {
      CloseHandle(MutexTable[uiIndex]);
      MutexTable[uiIndex] = NULL;
    }
  }

  UnRegisterDebugTopic(TOPIC_MUTEX, "Mutex Manager");
}

BOOLEAN InitializeMutex(UINT32 uiMutexIndex, UINT8 *ubMutexName)
{
  MutexTable[uiMutexIndex] = CreateMutex(NULL, FALSE, ubMutexName);
  if (MutexTable[uiMutexIndex] == NULL)
  {
    //
    // Mutex creation has failed.
    //
    DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "ERROR : Mutex initialization has failed.");
    return FALSE;
  }

  return TRUE;
}

BOOLEAN DeleteMutex(UINT32 uiMutexIndex)
{
  if (MutexTable[uiMutexIndex] == NULL)
  {
    //
    // Hum ?? We just tried to initialize a mutex entry which doesn't have a reserved slot
    //

    DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "ERROR : Mutex cannot be deleted since it does not exit");
    return FALSE;
  }

  if (CloseHandle(MutexTable[uiMutexIndex]) == FALSE)
  {
    //
    // Hum, the mutex deletion has failed
    //

    DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "ERROR : Mutex cannot be deleted since it does not exit");
    return FALSE;
  }

  MutexTable[uiMutexIndex] = NULL;

  return TRUE;
}

BOOLEAN EnterMutex(UINT32 uiMutexIndex, INT32 nLine, char *szFilename)
{
  switch (WaitForSingleObject(MutexTable[uiMutexIndex], INFINITE))
  {
    case WAIT_OBJECT_0
    : return TRUE;
    case WAIT_TIMEOUT
    : DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "ERROR : Possible infinite loop detected due to enter mutex timeout");
      return FALSE;
    case WAIT_ABANDONED
    : DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "ERROR : Abandoned mutex has been found");
      return FALSE;
  }
}

BOOLEAN EnterMutexWithTimeout(UINT32 uiMutexIndex, UINT32 uiTimeout, INT32 nLine, char *szFilename)
{
  switch (WaitForSingleObject(MutexTable[uiMutexIndex], uiTimeout))
  {
    case WAIT_OBJECT_0
    : return TRUE;
    case WAIT_TIMEOUT
    : return FALSE;
    case WAIT_ABANDONED
    : return FALSE;
  }
  return TRUE;
}

BOOLEAN LeaveMutex(UINT32 uiMutexIndex, INT32 nLine, char *szFilename)
{
  if (ReleaseMutex(MutexTable[uiMutexIndex]) == FALSE)
  {
    DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "ERROR : Failed to leave mutex");
    return FALSE;
  }

  return TRUE;
}

#else

//
// Use defines to allocate slots in the mutex manager. Put these defines in LOCAL.H
//

SDL_mutex *  MutexTable[MAX_MUTEX_HANDLES];

BOOLEAN InitializeMutexManager(void)
{
  UINT32 uiIndex;

  //
  // Make sure all mutex handles are opened
  //

  for (uiIndex = 0; uiIndex < MAX_MUTEX_HANDLES; uiIndex++)
  {
    MutexTable[uiIndex] = SDL_CreateMutex();
  }

  RegisterDebugTopic(TOPIC_MUTEX, "Mutex Manager");

  return TRUE;
}

void ShutdownMutexManager(void)
{
  UINT32 uiIndex;

  DbgMessage(TOPIC_MUTEX, DBG_LEVEL_0, "Shutting down the Mutex Manager");

  //
  // Make sure all mutex handles are closed
  //

  for (uiIndex = 0; uiIndex < MAX_MUTEX_HANDLES; uiIndex++)
  {
    SDL_DestroyMutex(MutexTable[uiIndex]);
  }

  UnRegisterDebugTopic(TOPIC_MUTEX, "Mutex Manager");
}

BOOLEAN InitializeMutex(UINT32 uiMutexIndex, UINT8 *ubMutexName)
{
  //InitializeCriticalSection(&MutexTable[uiMutexIndex]);

  return TRUE;
}

BOOLEAN DeleteMutex(UINT32 uiMutexIndex)
{
  //DeleteCriticalSection(&MutexTable[uiMutexIndex]);

  return TRUE;
}

BOOLEAN EnterMutex(UINT32 uiMutexIndex, INT32 nLine, char *szFilename)
{
  SDL_mutexP(MutexTable[uiMutexIndex]);
  return TRUE;
}

BOOLEAN EnterMutexWithTimeout(UINT32 uiMutexIndex, UINT32 uiTimeout, INT32 nLine, char *szFilename)
{
  SDL_mutexP(MutexTable[uiMutexIndex]);
  return TRUE;
}

BOOLEAN LeaveMutex(UINT32 uiMutexIndex, INT32 nLine, char *szFilename)
{
  SDL_mutexV(MutexTable[uiMutexIndex]);

  return TRUE;
}

#endif
