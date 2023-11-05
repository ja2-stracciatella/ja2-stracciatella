#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "Font_Control.h"
#include "Types.h"

#include <string_theory/string>


extern UINT8 gubCurrentMapMessageString;

#define MSG_INTERFACE		0
#define MSG_DIALOG		1
#define MSG_CHAT		2
#define MSG_DEBUG		3
#define MSG_UI_FEEDBACK	4
#define MSG_SKULL_UI_FEEDBACK	11


// These defines correlate to defines in font.h
#define MSG_FONT_RED		FONT_MCOLOR_RED
#define MSG_FONT_YELLOW	FONT_MCOLOR_LTYELLOW
#define MSG_FONT_WHITE		FONT_MCOLOR_WHITE

// are we allowed to beep on message scroll in tactical
extern BOOLEAN fOkToBeepNewMessage;



void ScreenMsg(UINT32 usColor, UINT8 ubPriority, const ST::string& str);

// same as screen message, but only display to mapscreen message system, not tactical
void MapScreenMessage(UINT32 usColor, UINT8 ubPriority, const ST::string& str);

void ScrollString( void );
void DisplayStringsInMapScreenMessageList( void );

void FreeGlobalMessageList( void );

UINT8 GetRangeOfMapScreenMessages( void );

void EnableDisableScrollStringVideoOverlay( BOOLEAN fEnable );

// will go and clear all displayed strings off the screen
void ClearDisplayedListOfTacticalStrings( void );

// clear ALL strings in the tactical Message Queue
void ClearTacticalMessageQueue( void );

void LoadMapScreenMessagesFromSaveGameFile(const HWFILE hFile, bool stracLinuxFormat, const UINT32 version);
void SaveMapScreenMessagesToSaveGameFile(HWFILE);

// use these if you are not Kris
void HideMessagesDuringNPCDialogue( void );
void UnHideMessagesDuringNPCDialogue( void );

// disable and enable scroll string, only to be used by Kris
void DisableScrollMessages( void );
void EnableScrollMessages( void );

extern UINT8 gubStartOfMapScreenMessageList;

#endif
