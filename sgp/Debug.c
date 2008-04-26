// Because we're in a library, define SGP_DEBUG here - the client may not always
// use the code to write text, because the header switches on the define
#ifndef SGP_DEBUG
#	define SGP_DEBUG
#endif

#include <SDL.h>
#include "Types.h"
#include "Debug.h"
#include <stdarg.h>
//Kris addition
#ifdef JA2
#	include "ScreenIDs.h"
#	include "Gameloop.h"
#	include "Input.h"
#endif
#include "Timer.h"


#ifdef __cplusplus
extern "C" {
#endif

static BOOLEAN gfRecordToFile     = FALSE;
static BOOLEAN gfRecordToDebugger = TRUE;

#ifdef SGP_DEBUG


static STRING512 gpcDebugLogFileName;

#ifdef __cplusplus
}
#endif


#ifndef _NO_DEBUG_TXT
static BOOLEAN DbgGetLogFileName(STRING512 pcName)
{
	strcpy(pcName, "debug.txt");
	return TRUE;
}
#endif


BOOLEAN InitializeDebugManager(void)
{
	gfRecordToFile = TRUE;
	gfRecordToDebugger = TRUE;

#ifndef _NO_DEBUG_TXT
	if (!DbgGetLogFileName(gpcDebugLogFileName)) return FALSE;
	remove(gpcDebugLogFileName);
#endif

	return(TRUE);
}


void DebugMsg(TopicID uiTopicId, DebugLevel uiDebugLevel, const char* strMessage)
{
	fprintf(stderr, "%s\n", strMessage);

//add _NO_DEBUG_TXT to your SGP preprocessor definitions to avoid this f**king huge file from
//slowly growing behind the scenes!!!!
#ifndef _NO_DEBUG_TXT
	FILE* OutFile = fopen(gpcDebugLogFileName, "a+");
	if (OutFile != NULL)
	{
		fprintf(OutFile, "%s\n", strMessage);
		fclose(OutFile);
	}
#endif
}


static void _DebugRecordToFile(BOOLEAN gfState)
{
	gfRecordToFile = gfState;
}


static void _DebugRecordToDebugger(BOOLEAN gfState)
{
	gfRecordToDebugger = gfState;
}


void _DebugMessage(const char* pString, UINT32 uiLineNum, const char* pSourceFile)
{
	char ubOutputString[512];
	sprintf(ubOutputString, "{ %ld } %s [Line %d in %s]\n", GetClock(), pString, uiLineNum, pSourceFile);

	if (gfRecordToDebugger)
	{
		fputs(ubOutputString, stderr);
	}

#ifndef _NO_DEBUG_TXT
	if (gfRecordToFile)
	{
		FILE* DebugFile = fopen(gpcDebugLogFileName, "a+");
		if (DebugFile != NULL)
		{
			fputs(ubOutputString, DebugFile);
			fclose(DebugFile);
		}
	}
#endif
}


void _FailMessage(const char *pString, UINT32 uiLineNum, const char *pSourceFile)
{
	char ubOutputString[512];
	if (pString != NULL)
		sprintf(ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]: %s\n", GetClock(), uiLineNum, pSourceFile, pString);
	else
		sprintf(ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]\n", GetClock(), uiLineNum, pSourceFile);

	//Output to debugger
	if (gfRecordToDebugger)
		fputs(ubOutputString, stderr);

	//Record to file if required
#ifndef _NO_DEBUG_TXT
	if (gfRecordToFile)
	{
		FILE* DebugFile = fopen(gpcDebugLogFileName, "a+");
		if (DebugFile != NULL)
		{
			fputs(ubOutputString, DebugFile);
			fclose(DebugFile);
		}
	}
#endif

	SDL_Quit();
	abort();
}

#endif

// This is NOT a _DEBUG only function! It is also needed in
// release mode builds. -- DB
const char* String(const char* String, ...)
{
	static char TmpDebugString[8][512];
	static UINT StringIndex = 0;

	// Record string index. This index is used since we live in a multitasking environment.
	// It is still not bulletproof, but it's better than a single string
	char* ResultString = TmpDebugString[StringIndex];
	StringIndex = (StringIndex + 1) % lengthof(TmpDebugString);

	va_list ArgPtr;
	va_start(ArgPtr, String);
	vsprintf(ResultString, String, ArgPtr);
	va_end(ArgPtr);

	return ResultString;
}
