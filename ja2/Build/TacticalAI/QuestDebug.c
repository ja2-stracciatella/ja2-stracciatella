#ifdef PRECOMPILEDHEADERS
	#include "AI All.h"
#else
	#include "FileMan.h"
	#include "QuestDebug.h"
	#include "stdarg.h"
	#include "stdio.h"
	#include "Debug.h"
	#include "Message.h"
#endif

#define	QUEST_DEBUG_FILE			"QuestDebug.txt"


//enums used for the current output level flags
enum
{
	QD_OUTPUT_NONE,				// no output
	QD_OUTPUT_LEVEL_1,		// Highest level
	QD_OUTPUT_LEVEL_2,
	QD_OUTPUT_LEVEL_3,
	QD_OUTPUT_LEVEL_4,
	QD_OUTPUT_LEVEL_ALL,	// any message
};




//Mode and priority for the Quest Debug Messages
UINT8	gubQuestDebugOutPutLevel = QD_OUTPUT_LEVEL_ALL;

//Mode and priority for the NPC interaction Debug Messages
UINT8	gubNPCDebugOutPutLevel = QD_OUTPUT_LEVEL_ALL;




//set the current output mode for either the NPC or the quest output
void ToggleQuestDebugModes( UINT8 ubType )
{
	wchar_t	sType[16];
	UINT8 ubLevel;

	if( ubType == QD_NPC_MSG )
	{
		wcscpy( sType, L"NPC Debug:");

		//check to see if its out of range
		if( gubNPCDebugOutPutLevel <= QD_OUTPUT_NONE+1 )
			gubNPCDebugOutPutLevel = QD_OUTPUT_LEVEL_ALL;
		else
		{
			//decrement
			gubNPCDebugOutPutLevel--;
		}

		ubLevel = gubNPCDebugOutPutLevel;
	}
	else
	{
		wcscpy( sType, L"QUEST Debug:");

		//check to see if its out of range
		if( gubQuestDebugOutPutLevel <= QD_OUTPUT_NONE )
			gubQuestDebugOutPutLevel = QD_OUTPUT_LEVEL_ALL;
		else
		{
			//decrement
			gubQuestDebugOutPutLevel--;
		}

		ubLevel = gubQuestDebugOutPutLevel;
	}

	//Display a messasge to the screen
	if( ubLevel == QD_OUTPUT_NONE )
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"%s No Output", sType );
	else if( ubLevel == QD_OUTPUT_LEVEL_ALL )
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"%s All messages", sType );
	else
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"%s Level %d", sType, ubLevel );
}





void QuestDebugFileMsg( UINT8 ubQuoteType, UINT8 ubPriority, STR pStringA, ...)
{
	static BOOLEAN	fFirstTimeIn = TRUE;
	static UINT32		uiLineNumber = 1;
	HWFILE		hFile;
	UINT32		uiByteWritten;
  va_list		argptr;
  char			TempString[1024];
  char			DestString[1024];


	TempString[0] = '\0';
	DestString[0] = '\0';

	va_start(argptr, pStringA);       	// Set up variable argument pointer
	vsprintf(TempString, pStringA, argptr);	// process gprintf string (get output str)
	va_end(argptr);


	//
	//determine if we should display the message
	//

	// if its a quest message
	if( ubQuoteType == QD_QUEST_MSG )
	{
		//if the message is a lower priority then the current output mode, dont display it
		if( ubPriority > gubQuestDebugOutPutLevel )
			return;
	}
	else
	{
		//if the message is a lower priority then the current output mode, dont display it
		if( ubPriority > gubNPCDebugOutPutLevel )
			return;
	}


	//if its the first time in the game
	if( fFirstTimeIn )
	{
		//open a new file for writing

		//if the file exists
		if( FileExists( QUEST_DEBUG_FILE ) )
		{
			//delete the file
			if( !FileDelete( QUEST_DEBUG_FILE ) )
			{
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to delete %s file", QUEST_DEBUG_FILE) );
				return;
			}
		}
		fFirstTimeIn = FALSE;
	}


	//open the file
	hFile = FileOpen( QUEST_DEBUG_FILE, FILE_ACCESS_WRITE, FALSE);
	if( !hFile )
	{
		FileClose(hFile);
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to open Quest Debug File %s", QUEST_DEBUG_FILE) );
		return;
	}


	sprintf( DestString, "#%5d. P%d:\n\t%s\n\n", uiLineNumber, ubPriority, TempString );

	//open the file and append to it
	if( !FileWrite( hFile, DestString, strlen(DestString), &uiByteWritten ) )
	{
		FileClose(hFile);
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to write to %s", QUEST_DEBUG_FILE) );
		return;
	}

	//increment the line number
	uiLineNumber++;
}
