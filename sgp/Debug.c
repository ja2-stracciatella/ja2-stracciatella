// Because we're in a library, define SGP_DEBUG here - the client may not always
// use the code to write text, because the header switches on the define
#define SGP_DEBUG

#include <SDL.h>
#include "Types.h"
#include <stdio.h>
#include "Debug.h"
#include "TopicIDs.h"
#include "TopicOps.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
//Kris addition
#ifdef JA2
#	include "ScreenIDs.h"
#	include "Sys_Globals.h"
#	include "JAScreens.h"
#	include "Gameloop.h"
#	include "Input.h"
#endif
#include "VObject.h"
#include "Stubs.h" // XXX


#ifdef __cplusplus
extern "C" {
#endif

static BOOLEAN gfRecordToFile     = FALSE;
static BOOLEAN gfRecordToDebugger = TRUE;

#ifdef SGP_DEBUG


UINT16 TOPIC_MEMORY_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_FILE_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_GAME = INVALID_TOPIC;
UINT16 TOPIC_SGP = INVALID_TOPIC;
UINT16 TOPIC_VIDEO = INVALID_TOPIC;
UINT16 TOPIC_INPUT = INVALID_TOPIC;
UINT16 TOPIC_LIST_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_QUEUE_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_HIMAGE = INVALID_TOPIC;
UINT16 TOPIC_VIDEOOBJECT = INVALID_TOPIC;
UINT16 TOPIC_FONT_HANDLER = INVALID_TOPIC;
UINT16 TOPIC_VIDEOSURFACE = INVALID_TOPIC;
UINT16 TOPIC_MOUSE_SYSTEM = INVALID_TOPIC;
UINT16 TOPIC_BUTTON_HANDLER = INVALID_TOPIC;
UINT16 TOPIC_JA2 = INVALID_TOPIC;
UINT16 TOPIC_JA2OPPLIST = INVALID_TOPIC;
UINT16 TOPIC_JA2AI = INVALID_TOPIC;


static UINT16* gpDbgTopicPtrs[MAX_TOPICS_ALLOTED];


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


BOOLEAN DbgInitialize(void)
{
	INT32 iX;

	for( iX = 0; iX < MAX_TOPICS_ALLOTED; iX++ )
	{
		gpDbgTopicPtrs[iX] = NULL;
	}

	gfRecordToFile = TRUE;
	gfRecordToDebugger = TRUE;

#ifndef _NO_DEBUG_TXT
	if (!DbgGetLogFileName(gpcDebugLogFileName)) return FALSE;
	remove(gpcDebugLogFileName);
#endif

	return(TRUE);
}


void DbgShutdown(void)
{
	DbgMessageReal((UINT16)(-1), 0, "SGP Going Down");
}


void DbgTopicRegistration(UINT8 ubCmd, UINT16 *usTopicID, const char *zMessage)
{
	UINT16 usIndex,usUse;
	BOOLEAN fFound;

	if ( usTopicID == NULL )
		return;

	if( ubCmd == TOPIC_REGISTER )
	{
		usUse = INVALID_TOPIC;
		fFound = FALSE;
		for( usIndex = 0; usIndex < MAX_TOPICS_ALLOTED && !fFound; usIndex++)
		{
			if (gpDbgTopicPtrs[usIndex] == NULL)
			{
				fFound = TRUE;
				usUse = usIndex;
			}
		}

		*usTopicID = usUse;
		gpDbgTopicPtrs[usUse] = usTopicID;
		DbgMessageReal(usUse, DBG_LEVEL_0, zMessage);
	}
	else if( ubCmd == TOPIC_UNREGISTER )
	{
		if ( *usTopicID >= MAX_TOPICS_ALLOTED )
			return;

		DbgMessageReal(*usTopicID, DBG_LEVEL_0, zMessage);

		gpDbgTopicPtrs[*usTopicID] = NULL;
		*usTopicID = INVALID_TOPIC;
	}
}


void DbgMessageReal(UINT16 uiTopicId, UINT8 uiDebugLevel, const char* strMessage)
{
	// Check for a registered topic ID
	if (uiTopicId < MAX_TOPICS_ALLOTED && gpDbgTopicPtrs[uiTopicId] != NULL)
	{
		OutputDebugString ( strMessage );
		OutputDebugString ( "\n" );

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
	UINT8 ubOutputString[512];

	sprintf( ubOutputString, "{ %ld } %s [Line %d in %s]\n", GetTickCount(), pString, uiLineNum, pSourceFile );

	if (gfRecordToDebugger)
	{
		OutputDebugString( ubOutputString );
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
		sprintf(ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]: %s\n", GetTickCount(), uiLineNum, pSourceFile, pString);
	else
		sprintf(ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]\n", GetTickCount(), uiLineNum, pSourceFile);

	//Output to debugger
	if (gfRecordToDebugger)
		OutputDebugString( ubOutputString );

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
