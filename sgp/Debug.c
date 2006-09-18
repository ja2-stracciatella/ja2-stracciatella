
// JA2

//**************************************************************************
//
// Filename :	debug.c
//
//	Purpose :	debug manager implementation
//
// Modification history :
//
//		xxxxx96:LH				- Creation
//		xxnov96:HJH				- made it work
//
//**************************************************************************

// Because we're in a library, define SGP_DEBUG here - the client may not always
// use the code to write text, because the header switches on the define
#define SGP_DEBUG


#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	//#include <ddeml.h>
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
		#include "ScreenIDs.h"
		#include "Sys_Globals.h"
		#include "JAScreens.h"
		#include "Gameloop.h"
		#include "Input.h"
	#endif

	// CJC added
	#ifndef _NO_DEBUG_TXT
		#include "FileMan.h"
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN gfRecordToFile     = FALSE;
BOOLEAN gfRecordToDebugger = TRUE;

// moved from header file: 24mar98:HJH
UINT32	guiProfileStart, guiExecutions, guiProfileTime;
INT32		giProfileCount;

// Had to move these outside the ifdef SGP_DEBUG below, because
// they are required for the String() function, which is NOT a
// debug-mode only function, it's used in release-mode as well! -- DB

UINT8 gubAssertString[128];

#define MAX_MSG_LENGTH2 512
UINT8		gbTmpDebugString[8][MAX_MSG_LENGTH2];
UINT8		gubStringIndex = 0;

#ifdef SGP_DEBUG

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#define BUFSIZE        100
#define TIMER_TIMEOUT  1000

//**************************************************************************
//
//				Variables
//
//**************************************************************************

UINT16 TOPIC_MEMORY_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_FILE_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_DATABASE_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_GAME = INVALID_TOPIC;
UINT16 TOPIC_SGP = INVALID_TOPIC;
UINT16 TOPIC_VIDEO = INVALID_TOPIC;
UINT16 TOPIC_INPUT = INVALID_TOPIC;
UINT16 TOPIC_LIST_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_QUEUE_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_PRILIST_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_HIMAGE = INVALID_TOPIC;
UINT16 TOPIC_3DENGINE = INVALID_TOPIC;
UINT16 TOPIC_VIDEOOBJECT = INVALID_TOPIC;
UINT16 TOPIC_FONT_HANDLER = INVALID_TOPIC;
UINT16 TOPIC_VIDEOSURFACE = INVALID_TOPIC;
UINT16 TOPIC_MOUSE_SYSTEM = INVALID_TOPIC;
UINT16 TOPIC_BUTTON_HANDLER = INVALID_TOPIC;
UINT16 TOPIC_JA2 = INVALID_TOPIC;
UINT16 TOPIC_BLIT_QUEUE = INVALID_TOPIC;
UINT16 TOPIC_JA2OPPLIST = INVALID_TOPIC;
UINT16 TOPIC_JA2AI = INVALID_TOPIC;


UINT32	guiTimerID = 0;
UINT8		guiDebugLevels[NUM_TOPIC_IDS];	// don't change this, Luis!!!!

BOOLEAN		gfDebugTopics[MAX_TOPICS_ALLOTED];
UINT16 		*gpDbgTopicPtrs[MAX_TOPICS_ALLOTED];


// remove debug .txt file
void RemoveDebugText( void );

STRING512 gpcDebugLogFileName;

#ifdef __cplusplus
}
#endif

//**************************************************************************
//
//				Functions
//
//**************************************************************************


//**************************************************************************
//
// DbgGetLogFileName
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		xxjun98:CJC		-> creation
//
//**************************************************************************
BOOLEAN DbgGetLogFileName( STRING512 pcName )
{
	// use the provided buffer to get the directory name, then tack on
	// "\debug.txt"
#ifndef _NO_DEBUG_TXT
	if ( ! GetExecutableDirectory( pcName ) )
	{
		return( FALSE );
	}

	if ( strlen( pcName ) > (512 - strlen( "\\debug.txt" ) - 1 ) )
	{
		// no room!
		return( FALSE );
	}

	strcat( pcName, "\\debug.txt" );
#endif

	return( TRUE );
}


//**************************************************************************
//
// DbgInitialize
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		xxnov96:HJH		-> creation
//
//**************************************************************************

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
	if (! DbgGetLogFileName( gpcDebugLogFileName ) )
	{
		return( FALSE );
	}
	// clear debug text file out
	RemoveDebugText( );
#endif

	return(TRUE);
}

//**************************************************************************
//
// DbgShutdown
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		xxnov96:HJH		-> creation
//
//**************************************************************************

void DbgShutdown(void)
{
	DbgMessageReal( (UINT16)(-1), CLIENT_SHUTDOWN, 0, "SGP Going Down" );
}


//**************************************************************************
//
// DbgTopicRegistration
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		June 97: BR		-> creation
//
//**************************************************************************

void DbgTopicRegistration( UINT8 ubCmd, UINT16 *usTopicID, CHAR8 *zMessage )
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
			if ( !gfDebugTopics[usIndex] )
			{
				fFound = TRUE;
				usUse = usIndex;
			}
		}

		gfDebugTopics[ usUse ] = TRUE;
		*usTopicID = usUse;
		gpDbgTopicPtrs[usUse] = usTopicID;
		DbgMessageReal(usUse, TOPIC_MESSAGE, DBG_LEVEL_0, zMessage );
	}
	else if( ubCmd == TOPIC_UNREGISTER )
	{
		if ( *usTopicID >= MAX_TOPICS_ALLOTED )
			return;

		DbgMessageReal( *usTopicID, TOPIC_MESSAGE, DBG_LEVEL_0, zMessage );
		gfDebugTopics[ *usTopicID ] = FALSE;


		if (gpDbgTopicPtrs[ *usTopicID ] != NULL )
		{
			gpDbgTopicPtrs[ *usTopicID ] = NULL;
		}

		*usTopicID = INVALID_TOPIC;
	}
}


// *************************************************************************
// Clear the debug txt file out to prevent it from getting huge
//
//
// *************************************************************************

void RemoveDebugText( void )
{
	FileDelete( gpcDebugLogFileName );
}


//**************************************************************************
//
// DbgClearAllTopics
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		June 97: BR		-> creation
//
//**************************************************************************

void DbgClearAllTopics( void )
{
	UINT16 usIndex;

	for( usIndex = 0; usIndex < MAX_TOPICS_ALLOTED; usIndex++)
	{
		gfDebugTopics[ usIndex ] = FALSE;
		if ( gpDbgTopicPtrs[ usIndex ] != NULL )
		{
			*gpDbgTopicPtrs[usIndex] = INVALID_TOPIC;
			gpDbgTopicPtrs[usIndex] = NULL;
		}
	}
}


//**************************************************************************
//
// DbgMessageReal
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		xxnov96:HJH		-> creation
//
//**************************************************************************

void DbgMessageReal(UINT16 uiTopicId, UINT8 uiCommand, UINT8 uiDebugLevel, const char *strMessage)
{
#ifndef _NO_DEBUG_TXT
  FILE      *OutFile;
#endif

	// Check for a registered topic ID
	if ( uiTopicId < MAX_TOPICS_ALLOTED && gfDebugTopics[uiTopicId] )
	{
		OutputDebugString ( strMessage );
		OutputDebugString ( "\n" );

//add _NO_DEBUG_TXT to your SGP preprocessor definitions to avoid this f**king huge file from
//slowly growing behind the scenes!!!!
#ifndef _NO_DEBUG_TXT
		if ((OutFile = fopen(gpcDebugLogFileName, "a+t")) != NULL)
		{
			fprintf(OutFile, "%s\n", strMessage);
			fclose(OutFile);
		}
#endif
	}

}

//**************************************************************************
//
// DbgSetDebugLevel
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		11nov96:HJH		-> creation
//
//**************************************************************************

BOOLEAN DbgSetDebugLevel(UINT16 uiTopicId, UINT8 uiDebugLevel)
{
	return(TRUE);
}

//**************************************************************************
//
// DbgFailedAssertion
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		xxnov96:HJH		-> creation
//
//**************************************************************************

void DbgFailedAssertion( BOOLEAN fExpression, char *szFile, int nLine )
{
#ifndef _NO_DEBUG_TXT
  FILE *OutFile;

	if ( fExpression == FALSE )
	{
		if ((OutFile = fopen(gpcDebugLogFileName, "a+t")) != NULL)
		{
		  fprintf(OutFile, "Assertion Failed at:\n    line %i\n    %s\n", nLine, szFile);
	    fclose(OutFile);
		}
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void			_DebugRecordToFile(BOOLEAN gfState)
{
	gfRecordToFile = gfState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void			_DebugRecordToDebugger(BOOLEAN gfState)
{
	gfRecordToDebugger = gfState;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Wiz8 compatible debug messaging

void			_DebugMessage(UINT8 *pString, UINT32 uiLineNum, UINT8 *pSourceFile)
{
	UINT8 ubOutputString[512];
#ifndef _NO_DEBUG_TXT
	FILE *DebugFile;
#endif

	//
	// Build the output string
	//

	sprintf( ubOutputString, "{ %ld } %s [Line %d in %s]\n", GetTickCount(), pString, uiLineNum, pSourceFile );

	//
	// Output to debugger
	//

	if (gfRecordToDebugger)
	{
		OutputDebugString( ubOutputString );
	}

	//
	// Record to file if required
	//

#ifndef _NO_DEBUG_TXT
	if (gfRecordToFile)
	{
		if ((DebugFile = fopen( gpcDebugLogFileName, "a+t" )) != NULL)
		{
			fputs( ubOutputString, DebugFile );
			fclose( DebugFile );
		}
	}
#endif
}



//////////////////////////////////////////////////////////////////////
// This func is used by Assert()
void _Null(void)
{
}

extern HVOBJECT FontObjs[25];

#ifdef JA2 //JAGGED ALLIANCE 2 VERSION ONLY
void _FailMessage( UINT8 *pString, UINT32 uiLineNum, UINT8 *pSourceFile )
{
	/*MSG Message;*/
	UINT8 ubOutputString[512];
#ifndef _NO_DEBUG_TXT
	FILE *DebugFile;
#endif
	BOOLEAN fDone = FALSE;
	//Build the output strings
	sprintf( ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]\n", GetTickCount(), uiLineNum, pSourceFile );
	if( pString )
		sprintf( gubAssertString, pString );
	else
		sprintf( gubAssertString, "" );

	//Output to debugger
	if (gfRecordToDebugger)
		OutputDebugString( ubOutputString );

	//Record to file if required
#ifndef _NO_DEBUG_TXT
	if (gfRecordToFile)
	{
		if ((DebugFile = fopen( gpcDebugLogFileName, "a+t" )) != NULL)
		{
			fputs( ubOutputString, DebugFile );
			fclose( DebugFile );
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
			gfSGPInputReceived  =  FALSE;
		}
	}
#endif
#endif
	exit(0);
}

#else //NOT JAGGED ALLIANCE 2

void _FailMessage(UINT8 *pString, UINT32 uiLineNum, UINT8 *pSourceFile)
{
	UINT8 ubOutputString[512];
	BOOLEAN fDone = FALSE;

#ifndef _NO_DEBUG_TXT
	FILE *DebugFile;
#endif


	// Build the output string
	sprintf( ubOutputString, "{ %ld } Assertion Failure: %s [Line %d in %s]\n", GetTickCount(), pString, uiLineNum, pSourceFile );
	if( pString )
		sprintf( gubAssertString, pString );
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
		if ((DebugFile = fopen( gpcDebugLogFileName, "a+t" )) != NULL)
		{
			fputs( ubOutputString, DebugFile );
			if( pString )
			{ //tag on the assert message
				fputs( gubAssertString, DebugFile );
			}
			fclose( DebugFile );
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
