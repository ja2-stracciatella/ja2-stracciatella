// Because we're in a library, define SGP_DEBUG here - the client may not always
// use the code to write text, because the header switches on the define
#define SGP_DEBUG


#include "Types.h"
#include <stdio.h>
#include "Debug.h"
#include "WCheck.h"
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

// Had to move these outside the ifdef SGP_DEBUG below, because
// they are required for the String() function, which is NOT a
// debug-mode only function, it's used in release-mode as well! -- DB

UINT8 gubAssertString[128];

#define MAX_MSG_LENGTH2 512
static UINT8		gbTmpDebugString[8][MAX_MSG_LENGTH2];
static UINT8		gubStringIndex = 0;

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

	DbgClearAllTopics();

	gfRecordToFile = TRUE;
	gfRecordToDebugger = TRUE;
	gubAssertString[0] = '\0';

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


void DbgClearAllTopics( void )
{
	UINT16 usIndex;

	for( usIndex = 0; usIndex < MAX_TOPICS_ALLOTED; usIndex++)
	{
		if ( gpDbgTopicPtrs[ usIndex ] != NULL )
		{
			*gpDbgTopicPtrs[usIndex] = INVALID_TOPIC;
			gpDbgTopicPtrs[usIndex] = NULL;
		}
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


///////////////////////////////////////////////////////////////////////////////////////////////////
// Wiz8 compatible debug messaging

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


extern HVOBJECT FontObjs[25];

#ifdef JA2 //JAGGED ALLIANCE 2 VERSION ONLY
void _FailMessage(const char *pString, UINT32 uiLineNum, const char *pSourceFile)
{
	/*MSG Message;*/
	UINT8 ubOutputString[512];
	//Build the output strings
	sprintf( ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]\n", GetTickCount(), uiLineNum, pSourceFile );
	strlcpy(gubAssertString, pString != NULL ? pString : "", lengthof(gubAssertString));

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

#if 0
	if( !FontObjs[0] )
	{ //Font manager hasn't yet been initialized so use the windows error system
		sprintf( gubErrorText, "Assertion Failure -- Line %d in %s", uiLineNum, pSourceFile );
		MessageBox( NULL, gubErrorText, "Jagged Alliance 2", MB_OK );
		gfProgramIsRunning = FALSE;
		return;
	}
#endif

	//Kris:
	//NASTY HACK, THE GAME IS GOING TO DIE ANYWAY, SO WHO CARES WHAT WE DO.
	//This will actually bring up a screen that prints out the assert message
	//until the user hits esc or alt-x.
	sprintf( gubErrorText, "Assertion Failure -- Line %d in %s", uiLineNum, pSourceFile );
	SetPendingNewScreen( ERROR_SCREEN );
	SetCurrentScreen( ERROR_SCREEN );
	/* Using windows-COM-model - I don't think it is really necessary,
	 * so I comment it out for the moment to make the file 
	 * compile. We can add our own way of doing this later
	 * - Wolf
	 *
	 * TODO: Repair this function and add something of our own
	 * (if we need this at all)
	 */
#if 0
	while (gfProgramIsRunning)
	{
		if (PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE))
		{ // We have a message on the WIN95 queue, let's get it
			if (!GetMessage(&Message, NULL, 0, 0))
			{ // It's quitting time
				continue;
			}
			// Ok, now that we have the message, let's handle it
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{ // Windows hasn't processed any messages, therefore we handle the rest
			GameLoop();
		}
	}
#endif
#endif
	abort();
}

#else //NOT JAGGED ALLIANCE 2

void _FailMessage(UINT8 *pString, UINT32 uiLineNum, UINT8 *pSourceFile)
{
	UINT8 ubOutputString[512];
	BOOLEAN fDone = FALSE;

	// Build the output string
	sprintf( ubOutputString, "{ %ld } Assertion Failure: %s [Line %d in %s]\n", GetTickCount(), pString, uiLineNum, pSourceFile );
	if( pString )
		strlcpy(gubAssertString, pString, lengthof(gubAssertString));
	// Output to debugger
	if (gfRecordToDebugger)
	{
		OutputDebugString( ubOutputString );
		if( pString )
		{ //tag on the assert message
			OutputDebugString( gubAssertString );
		}
	}
	// Record to file if required
#ifndef _NO_DEBUG_TXT
	if (gfRecordToFile)
	{
		FILE* DebugFile = fopen(gpcDebugLogFileName, "a+");
		if (DebugFile != NULL)
		{
			fputs( ubOutputString, DebugFile );
			if (pString != NULL)
			{ //tag on the assert message
				fputs(gubAssertString, DebugFile);
			}
			fclose(DebugFile);
		}
	}
#endif
	exit( 0 );
}

#endif

#endif

// This is NOT a _DEBUG only function! It is also needed in
// release mode builds. -- DB
UINT8 *String(const char *String, ...)
{

  va_list  ArgPtr;
  UINT8    usIndex;

  // Record string index. This index is used since we live in a multitasking environment.
  // It is still not bulletproof, but it's better than a single string
  usIndex = gubStringIndex++;
  if (gubStringIndex == 8)
  { // reset string pointer
    gubStringIndex = 0;
  }

  va_start(ArgPtr, String);
  vsprintf(gbTmpDebugString[usIndex], String, ArgPtr);
  va_end(ArgPtr);

  return gbTmpDebugString[usIndex];
}
