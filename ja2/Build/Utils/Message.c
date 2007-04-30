#include "SGP.h"
#include "Font.h"
#include "Types.h"
#include "Font_Control.h"
#include "Message.h"
#include "Timer_Control.h"
#include "Render_Dirty.h"
#include "RenderWorld.h"
#include "Local.h"
#include "Interface.h"
#include "Map_Screen_Interface_Bottom.h"
#include "WordWrap.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "BuildDefines.h"
#include "Dialogue_Control.h"
#include "Game_Clock.h"
#include <stdarg.h>
#include "VSurface.h"
#include "MemMan.h"
#include "JAScreens.h"
#include "ScreenIDs.h"
#include "FileMan.h"


typedef struct ScrollStringSt ScrollStringSt;
struct ScrollStringSt
{
	STR16   pString16;
	INT32   iVideoOverlay;
	UINT32  uiFont;
	UINT16  usColor;
	UINT32  uiFlags;
	BOOLEAN fBeginningOfNewString;
	UINT32  uiTimeOfLastUpdate;
	ScrollStringSt* pNext;
	ScrollStringSt* pPrev;
};


typedef struct
{
	UINT32  uiFont;
	UINT32  uiTimeOfLastUpdate;
	UINT32  uiFlags;
	UINT32	uiPadding[ 3 ];
	UINT16  usColor;
	BOOLEAN fBeginningOfNewString;
} StringSaveStruct;
CASSERT(sizeof(StringSaveStruct) == 28)


#define MAX_LINE_COUNT 6
#define X_START 2
#define Y_START 330
#define MAX_AGE 10000
#define LINE_WIDTH 320
#define MAP_LINE_WIDTH 300
#define WIDTH_BETWEEN_NEW_STRINGS 5

#define BETAVERSION_COLOR FONT_ORANGE
#define TESTVERSION_COLOR FONT_GREEN
#define DEBUG_COLOR FONT_RED
#define DIALOGUE_COLOR FONT_WHITE
#define INTERFACE_COLOR FONT_YELLOW

#define MAP_SCREEN_MESSAGE_FONT TINYFONT1

UINT8 gubStartOfMapScreenMessageList = 0;
UINT8 gubEndOfMapScreenMessageList = 0;

// index of the current string we are looking at
UINT8 gubCurrentMapMessageString = 0;

// temp position for display of marker
//UINT8 ubTempPosition = 0;

// are allowed to beep on message scroll?
BOOLEAN fOkToBeepNewMessage = TRUE;


static ScrollStringSt* gpDisplayList[MAX_LINE_COUNT];
static ScrollStringSt* gMapScreenMessageList[256];
static ScrollStringSt* pStringS = NULL;

// first time adding any message to the message dialogue system
BOOLEAN fFirstTimeInMessageSystem = TRUE;
BOOLEAN fDisableJustForIan = FALSE;

BOOLEAN fScrollMessagesHidden = FALSE;
UINT32 uiStartOfPauseTime = 0;

// test extern functions
BOOLEAN RestoreExternBackgroundRectGivenID( INT32 iBack );
extern VIDEO_OVERLAY	gVideoOverlays[];

extern UINT16 gusSubtitleBoxWidth;
extern BOOLEAN gfFacePanelActive;

// region created and due to last quote box
extern BOOLEAN fTextBoxMouseRegionCreated;
extern BOOLEAN fDialogueBoxDueToLastMessage;


static void SetStringFont(ScrollStringSt* pStringSt, UINT32 uiFont)
{
	pStringSt->uiFont=uiFont;
}


static UINT32 GetStringFont(ScrollStringSt* pStringSt)
{
	return pStringSt->uiFont;
}


static void SetString(ScrollStringSt* pStringSt, STR16 pString);
static void SetStringColor(ScrollStringSt* pStringSt, UINT16 usColor);
static ScrollStringSt* SetStringNext(ScrollStringSt* pStringSt, ScrollStringSt* pNext);
static ScrollStringSt* SetStringPrev(ScrollStringSt* pStringSt, ScrollStringSt* pPrev);


static ScrollStringSt* AddString(STR16 pString, UINT16 usColor, UINT32 uiFont, BOOLEAN fStartOfNewString, UINT8 ubPriority)
{
	// add a new string to the list of strings
	ScrollStringSt* pStringSt = NULL;
	pStringSt=MemAlloc(sizeof(ScrollStringSt));

	SetString(pStringSt, pString);
	SetStringColor(pStringSt, usColor);
	pStringSt->uiFont = uiFont;
	pStringSt -> fBeginningOfNewString = fStartOfNewString;
	pStringSt -> uiFlags = ubPriority;

	SetStringNext(pStringSt, NULL);
	SetStringPrev(pStringSt, NULL);
	pStringSt->iVideoOverlay=-1;

	// now add string to map screen strings
	//AddStringToMapScreenMessageList(pString, usColor, uiFont, fStartOfNewString, ubPriority );

	return (pStringSt);
}


static void SetString(ScrollStringSt* pStringSt, STR16 pString)
{
	pStringSt->pString16 = MemAlloc(sizeof(*pStringSt->pString16) * (wcslen(pString) + 1));
	wcscpy(pStringSt->pString16, pString);
}


static void SetStringVideoOverlayPosition(ScrollStringSt* pStringSt, UINT16 usX, UINT16 usY);


static void SetStringPosition(ScrollStringSt* pStringSt, UINT16 usX, UINT16 usY)
{
	SetStringVideoOverlayPosition( pStringSt, usX, usY );
}


static void SetStringColor(ScrollStringSt* pStringSt, UINT16 usColor)
{
	pStringSt->usColor=usColor;
}


static ScrollStringSt* GetNextString(ScrollStringSt* pStringSt)
{
	// returns pointer to next string line
  if (pStringSt==NULL)
		return NULL;
	else
		return pStringSt->pNext;
}


static ScrollStringSt* GetPrevString(ScrollStringSt* pStringSt)
{
	// returns pointer to previous string line
	if (pStringSt==NULL)
		return NULL;
	else
		return pStringSt->pPrev;
}


static ScrollStringSt* SetStringNext(ScrollStringSt* pStringSt, ScrollStringSt* pNext)
{
	pStringSt->pNext=pNext;
  return pStringSt;
}


static ScrollStringSt* SetStringPrev(ScrollStringSt* pStringSt, ScrollStringSt* pPrev)
{
	pStringSt->pPrev=pPrev;
  return pStringSt;
}


static void BlitString(VIDEO_OVERLAY* pBlitter);


static BOOLEAN CreateStringVideoOverlay(ScrollStringSt* pStringSt, UINT16 usX, UINT16 usY)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	// SET VIDEO OVERLAY
	VideoOverlayDesc.sLeft			 = usX;
	VideoOverlayDesc.sTop				 = usY;
	VideoOverlayDesc.uiFontID    = pStringSt->uiFont;
	VideoOverlayDesc.ubFontBack  = FONT_MCOLOR_BLACK ;
	VideoOverlayDesc.ubFontFore  = (unsigned char)pStringSt->usColor;
	VideoOverlayDesc.sX					 = VideoOverlayDesc.sLeft;
	VideoOverlayDesc.sY					 = VideoOverlayDesc.sTop;
	swprintf( VideoOverlayDesc.pzText, lengthof(VideoOverlayDesc.pzText), pStringSt->pString16 );
	VideoOverlayDesc.BltCallback = BlitString;
	pStringSt->iVideoOverlay =  RegisterVideoOverlay( ( VOVERLAY_DIRTYBYTEXT ), &VideoOverlayDesc );

	if ( pStringSt->iVideoOverlay == -1 )
	{
		return( FALSE );
	}

	return( TRUE );
}


static void RemoveStringVideoOverlay(ScrollStringSt* pStringSt)
{
	// error check, remove one not there
	if( pStringSt->iVideoOverlay == -1 )
	{
		return;
	}


	RemoveVideoOverlay( pStringSt->iVideoOverlay );
	pStringSt->iVideoOverlay=-1;
}


static void SetStringVideoOverlayPosition(ScrollStringSt* pStringSt, UINT16 usX, UINT16 usY)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	memset( &VideoOverlayDesc, 0, sizeof( VideoOverlayDesc ) );

	// Donot update if not allocated!
	if ( pStringSt->iVideoOverlay != -1 )
	{
		VideoOverlayDesc.uiFlags    = VOVERLAY_DESC_POSITION;
		VideoOverlayDesc.sLeft			 = usX;
		VideoOverlayDesc.sTop				 = usY;
		VideoOverlayDesc.sX					 = VideoOverlayDesc.sLeft;
		VideoOverlayDesc.sY					 = VideoOverlayDesc.sTop;
		UpdateVideoOverlay( &VideoOverlayDesc, pStringSt->iVideoOverlay, FALSE );
	}
}


static void BlitString(VIDEO_OVERLAY* pBlitter)
{
	UINT8	 *pDestBuf;
	UINT32 uiDestPitchBYTES;

  //gprintfdirty(pBlitter->sX,pBlitter->sY, pBlitter->zText);
	//RestoreExternBackgroundRect(pBlitter->sX,pBlitter->sY, pBlitter->sX+StringPixLength(pBlitter->zText,pBlitter->uiFontID ), pBlitter->sY+GetFontHeight(pBlitter->uiFontID ));

	if( fScrollMessagesHidden == TRUE )
	{
		return;
	}


	pDestBuf = LockVideoSurface( pBlitter->uiDestBuff, &uiDestPitchBYTES);
	SetFont(pBlitter->uiFontID);

	SetFontBackground( pBlitter->ubFontBack );
	SetFontForeground( pBlitter->ubFontFore );
	SetFontShadow( DEFAULT_SHADOW );
	mprintf_buffer_coded(pDestBuf, uiDestPitchBYTES, pBlitter->sX, pBlitter->sY, pBlitter->zText);
	UnLockVideoSurface( pBlitter->uiDestBuff );

}


static void EnableStringVideoOverlay(ScrollStringSt* pStringSt, BOOLEAN fEnable)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	memset( &VideoOverlayDesc, 0, sizeof( VideoOverlayDesc ) );

	if ( pStringSt->iVideoOverlay != -1 )
	{
		VideoOverlayDesc.fDisabled	= !fEnable;
		VideoOverlayDesc.uiFlags    = VOVERLAY_DESC_DISABLED;
		UpdateVideoOverlay( &VideoOverlayDesc, pStringSt->iVideoOverlay, FALSE );
	}
}


void ClearDisplayedListOfTacticalStrings( void )
{
	// this function will go through list of display strings and clear them all out
	UINT32 cnt;

  for ( cnt = 0; cnt < MAX_LINE_COUNT; cnt++ )
  {
		if ( gpDisplayList[ cnt ] != NULL )
		{
			// CHECK IF WE HAVE AGED

			// Remove our sorry ass
			RemoveStringVideoOverlay( gpDisplayList[ cnt ] );
			MemFree( gpDisplayList[ cnt ]->pString16);
			MemFree( gpDisplayList[ cnt ] );

			// Free slot
			gpDisplayList[ cnt ] = NULL;
		}
  }
}


static INT32 GetMessageQueueSize(void);
static void HandleLastQuotePopUpTimer(void);
static void PlayNewMessageSound(void);


void ScrollString( )
{
	UINT32 suiTimer=0;
	UINT32 cnt;
  INT32 iNumberOfNewStrings = 0; // the count of new strings, so we can update position by WIDTH_BETWEEN_NEW_STRINGS pixels in the y
	INT32 iNumberOfMessagesOnQueue = 0;
	INT32 iMaxAge = 0;
	BOOLEAN fDitchLastMessage = FALSE;

	// UPDATE TIMER
	suiTimer=GetJA2Clock();

	// might have pop up text timer
	HandleLastQuotePopUpTimer( );

	if( guiCurrentScreen == MAP_SCREEN )
	{
		return;
	}

	// DONOT UPDATE IF WE ARE SCROLLING!
	if ( gfScrollPending || gfScrollInertia )
	{
		return;
	}

	// messages hidden
	if( fScrollMessagesHidden )
	{
		return;
	}

	iNumberOfMessagesOnQueue = GetMessageQueueSize( );
	iMaxAge =  MAX_AGE;

	if( ( iNumberOfMessagesOnQueue > 0 )&&( gpDisplayList[ MAX_LINE_COUNT - 1 ] != NULL) )
	{
		fDitchLastMessage = TRUE;
	}
	else
	{
		fDitchLastMessage = FALSE;

	}


	if( ( iNumberOfMessagesOnQueue * 1000 ) >= iMaxAge )
	{
		iNumberOfMessagesOnQueue = ( iMaxAge / 1000 );
	}
	else if( iNumberOfMessagesOnQueue < 0 )
	{
		iNumberOfMessagesOnQueue = 0;
	}

	//AGE
	for ( cnt = 0; cnt < MAX_LINE_COUNT; cnt++ )
	{
		if ( gpDisplayList[ cnt ] != NULL )
		{
			if( ( fDitchLastMessage ) && ( cnt == MAX_LINE_COUNT - 1 ) )
			{
				gpDisplayList[ cnt ]->uiTimeOfLastUpdate = iMaxAge;
			}
				// CHECK IF WE HAVE AGED
			if ( ( suiTimer - gpDisplayList[ cnt ]->uiTimeOfLastUpdate ) > ( UINT32 )( iMaxAge - ( 1000 * iNumberOfMessagesOnQueue ) ) )
			{
				// Remove our sorry ass
				RemoveStringVideoOverlay( gpDisplayList[ cnt ] );
				MemFree( gpDisplayList[ cnt ]->pString16);
				MemFree( gpDisplayList[ cnt ] );

				// Free slot
				gpDisplayList[ cnt ] = NULL;
			}
		}
	}


	// CHECK FOR FREE SPOTS AND ADD ANY STRINGS IF WE HAVE SOME TO ADD!

	// FIRST CHECK IF WE HAVE ANY IN OUR QUEUE
	if ( pStringS != NULL )
	{
		// CHECK IF WE HAVE A SLOT!
		// CHECK OUR LAST SLOT!
		if ( gpDisplayList[ MAX_LINE_COUNT - 1 ] == NULL )
		{
			// MOVE ALL UP!

		 // cpy, then move
		 for( cnt = MAX_LINE_COUNT - 1; cnt > 0; cnt-- )
		 {
				gpDisplayList[ cnt ] =  gpDisplayList[ cnt - 1 ];
		 }

			// now add in the new string
		 cnt = 0;
		 gpDisplayList[ cnt ] = pStringS;
		 CreateStringVideoOverlay( pStringS, X_START, Y_START );
		 if( pStringS -> fBeginningOfNewString == TRUE )
		 {
			 iNumberOfNewStrings++;
		 }

		 // set up age
		 pStringS->uiTimeOfLastUpdate = GetJA2Clock();

		 // now move
		 for ( cnt = 0; cnt <= MAX_LINE_COUNT - 1; cnt++ )
		 {

				// Adjust position!
				if ( gpDisplayList[ cnt  ] != NULL )
				{

					SetStringVideoOverlayPosition( gpDisplayList[ cnt ], X_START, (INT16)( ( Y_START - ( ( cnt ) * GetFontHeight( SMALLFONT1 ) ) ) - ( INT16)( WIDTH_BETWEEN_NEW_STRINGS * ( iNumberOfNewStrings ) ) ) );

					// start of new string, increment count of new strings, for spacing purposes
					if( gpDisplayList[ cnt ] -> fBeginningOfNewString == TRUE )
					{
						iNumberOfNewStrings++;
					}


				}

		 }


		 // WE NOW HAVE A FREE SPACE, INSERT!

		 // Adjust head!
		 pStringS = pStringS->pNext;
		 if( pStringS )
		 {
			 pStringS->pPrev = NULL;
		 }

		 //check if new meesage we have not seen since mapscreen..if so, beep
			if( ( fOkToBeepNewMessage == TRUE ) && ( gpDisplayList[ MAX_LINE_COUNT - 2 ] == NULL ) && ( ( guiCurrentScreen == GAME_SCREEN ) || ( guiCurrentScreen == MAP_SCREEN ) ) && ( gfFacePanelActive == FALSE ) )
			{
				PlayNewMessageSound( );
			}
		}
	}
}


void DisableScrollMessages( void )
{
	// will stop the scroll of messages in tactical and hide them during an NPC's dialogue
	// disble video overlay for tatcitcal scroll messages
	EnableDisableScrollStringVideoOverlay( FALSE );
}


void EnableScrollMessages( void )
{
	EnableDisableScrollStringVideoOverlay( TRUE );
}

void HideMessagesDuringNPCDialogue( void )
{
	// will stop the scroll of messages in tactical and hide them during an NPC's dialogue
	INT32 cnt;

	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	memset( &VideoOverlayDesc, 0, sizeof( VideoOverlayDesc ) );


	VideoOverlayDesc.fDisabled	= TRUE;
	VideoOverlayDesc.uiFlags    = VOVERLAY_DESC_DISABLED;


	fScrollMessagesHidden = TRUE;
	uiStartOfPauseTime = GetJA2Clock();

	for ( cnt = 0; cnt < MAX_LINE_COUNT; cnt++ )
	{
			if ( gpDisplayList[ cnt ] != NULL )
			{
				RestoreExternBackgroundRectGivenID( gVideoOverlays[ gpDisplayList[ cnt ] -> iVideoOverlay ].uiBackground );
				UpdateVideoOverlay( &VideoOverlayDesc, gpDisplayList[ cnt ] -> iVideoOverlay, FALSE );
			}
	}
}


void UnHideMessagesDuringNPCDialogue( void )
{

	VIDEO_OVERLAY_DESC		VideoOverlayDesc;
	INT32 cnt = 0;

	memset( &VideoOverlayDesc, 0, sizeof( VideoOverlayDesc ) );


	VideoOverlayDesc.fDisabled	= FALSE;
	VideoOverlayDesc.uiFlags    = VOVERLAY_DESC_DISABLED;
	fScrollMessagesHidden				= FALSE;

	for ( cnt = 0; cnt < MAX_LINE_COUNT; cnt++ )
	{
		if ( gpDisplayList[ cnt ] != NULL )
		{
			gpDisplayList[ cnt ]->uiTimeOfLastUpdate+= GetJA2Clock() - uiStartOfPauseTime;
			UpdateVideoOverlay( &VideoOverlayDesc, gpDisplayList[ cnt ] -> iVideoOverlay, FALSE );
		}
	}
}


static void TacticalScreenMsg(UINT16 usColor, UINT8 ubPriority, const wchar_t* pStringA, ...);


// new screen message
void ScreenMsg( UINT16 usColor, UINT8 ubPriority, const wchar_t *pStringA, ... )
{

	wchar_t	DestString[512];
	va_list argptr;

	if( fDisableJustForIan == TRUE )
	{
		if( ubPriority == MSG_BETAVERSION )
		{
			return;
		}
		else if( ubPriority == MSG_TESTVERSION )
		{
			return;
		}
		else if ( ubPriority == MSG_DEBUG )
		{
			return;
		}
	}

	if( ubPriority == MSG_DEBUG )
	{
		usColor = DEBUG_COLOR;
		#ifndef _DEBUG
			return;
		#endif
		#ifdef JA2DEMO
			return;
		#endif
	}

	if( ubPriority == MSG_BETAVERSION )
	{
		usColor = BETAVERSION_COLOR;
		#ifndef JA2BETAVERSION
			#ifndef JA2TESTVERSION
				return;
			#endif
		#endif

	}

	if( ubPriority == MSG_TESTVERSION )
	{
		usColor = TESTVERSION_COLOR;

		#ifndef JA2TESTVERSION
			 return;
		#endif

	}

	va_start(argptr, pStringA);
	vswprintf(DestString, lengthof(DestString), pStringA, argptr);
	va_end(argptr);

	// pass onto tactical message and mapscreen message
	TacticalScreenMsg(usColor, ubPriority, L"%ls", DestString);

//	if( ( ubPriority != MSG_DEBUG ) && ( ubPriority != MSG_TESTVERSION ) )
	{
		MapScreenMessage(usColor, ubPriority, L"%ls", DestString);
	}


	if( guiCurrentScreen == MAP_SCREEN )
	{
	  PlayNewMessageSound( );
	}
	else
	{
		fOkToBeepNewMessage = TRUE;
	}
}


// clear up a linked list of wrapped strings
static void ClearWrappedStrings(WRAPPED_STRING* pStringWrapperHead)
{
	WRAPPED_STRING *pNode = pStringWrapperHead;
	WRAPPED_STRING *pDeleteNode = NULL;
	// clear out a link list of wrapped string structures

	// error check, is there a node to delete?
	if( pNode == NULL )
	{
		// leave,
		return;
	}

	do
	{

		// set delete node as current node
		pDeleteNode = pNode;

		// set current node as next node
		pNode = pNode -> pNextWrappedString;

		//delete the string
		MemFree( pDeleteNode->sString );
		pDeleteNode->sString = NULL;

		// clear out delete node
		MemFree( pDeleteNode );
		pDeleteNode = NULL;

	}	while( pNode );


//	MemFree( pNode );

	pStringWrapperHead = NULL;

}


static void WriteMessageToFile(STR16 pString);


// new tactical and mapscreen message system
static void TacticalScreenMsg(UINT16 usColor, UINT8 ubPriority, const wchar_t* pStringA, ...)
{
  // this function sets up the string into several single line structures

	ScrollStringSt* pStringSt;
	UINT32 uiFont = TINYFONT1;
	//wchar_t *pString;
  va_list argptr;

  wchar_t	DestString[512], DestStringA[ 512 ];
	//wchar_t *pStringBuffer;
	ScrollStringSt* pTempStringSt = NULL;
  WRAPPED_STRING *pStringWrapper=NULL;
  WRAPPED_STRING *pStringWrapperHead=NULL;
  BOOLEAN fNewString = FALSE;
	UINT16	usLineWidthIfWordIsWiderThenWidth=0;


	if( giTimeCompressMode > TIME_COMPRESS_X1 )
	{
		return;
	}

	if( fDisableJustForIan == TRUE && ubPriority != MSG_ERROR && ubPriority != MSG_INTERFACE )
	{
		return;
	}

	if( ubPriority == MSG_BETAVERSION )
	{
		usColor = BETAVERSION_COLOR;
		#ifndef JA2BETAVERSION
			#ifndef JA2TESTVERSION
				return;
			#endif
		#endif
		WriteMessageToFile( DestString );

	}

	if( ubPriority == MSG_TESTVERSION )
	{
		usColor = TESTVERSION_COLOR;

		#ifndef JA2TESTVERSION
			 return;
		#endif

		WriteMessageToFile( DestString );

	}


	if ( fFirstTimeInMessageSystem )
	{
		// Init display array!
		memset( gpDisplayList, 0, sizeof( gpDisplayList ) );
		fFirstTimeInMessageSystem = FALSE;
	}


	pStringSt=pStringS;
	while(GetNextString(pStringSt))
		    pStringSt=GetNextString(pStringSt);

	va_start(argptr, pStringA);       	// Set up variable argument pointer
	vswprintf(DestString, lengthof(DestString), pStringA, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	if ( ubPriority == MSG_DEBUG )
	{
		#ifndef _DEBUG
			return;
		#endif
		#ifdef JA2DEMO
			return;
		#endif
		usColor = DEBUG_COLOR;
		wcscpy( DestStringA, DestString );
		swprintf(DestString, lengthof(DestString), L"Debug: %ls", DestStringA);
		WriteMessageToFile( DestStringA );
	}

	if ( ubPriority == MSG_DIALOG )
	{
		usColor = DIALOGUE_COLOR;
	}

	if ( ubPriority == MSG_INTERFACE )
	{
		usColor = INTERFACE_COLOR;
	}



	pStringWrapperHead=LineWrap(uiFont, LINE_WIDTH, &usLineWidthIfWordIsWiderThenWidth, L"%ls", DestString);
  pStringWrapper=pStringWrapperHead;
	if(!pStringWrapper)
    return;

	fNewString = TRUE;
	while(pStringWrapper->pNextWrappedString!=NULL)
	{
	 if(!pStringSt)
	 {
    pStringSt=AddString(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority );
		fNewString = FALSE;
		pStringSt->pNext=NULL;
		pStringSt->pPrev=NULL;
    pStringS=pStringSt;
	 }
	 else
	 {
	  pTempStringSt=AddString(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority);
    fNewString = FALSE;
		pTempStringSt->pPrev=pStringSt;
	  pStringSt->pNext=pTempStringSt;
	  pStringSt=pTempStringSt;
	  pTempStringSt->pNext=NULL;
	 }
   pStringWrapper=pStringWrapper->pNextWrappedString;
	}
  pTempStringSt=AddString(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority );
	if(pStringSt)
	{
	 pStringSt->pNext=pTempStringSt;
	 pTempStringSt->pPrev=pStringSt;
	 pStringSt=pTempStringSt;
	 pStringSt->pNext=NULL;
	}
  else
	{
		pStringSt=pTempStringSt;
		pStringSt->pNext=NULL;
		pStringSt->pPrev=NULL;
    pStringS=pStringSt;
	}

	// clear up list of wrapped strings
	ClearWrappedStrings( pStringWrapperHead );
}


static void AddStringToMapScreenMessageList(STR16 pString, UINT16 usColor, UINT32 uiFont, BOOLEAN fStartOfNewString, UINT8 ubPriority);


void MapScreenMessage( UINT16 usColor, UINT8 ubPriority, const wchar_t *pStringA, ... )
{
  // this function sets up the string into several single line structures

	ScrollStringSt* pStringSt;
	UINT32 uiFont = MAP_SCREEN_MESSAGE_FONT;
	//wchar_t *pString;
  va_list argptr;
  wchar_t	DestString[512], DestStringA[ 512 ];
	//wchar_t *pStringBuffer;
  WRAPPED_STRING *pStringWrapper=NULL;
  WRAPPED_STRING *pStringWrapperHead=NULL;
  BOOLEAN fNewString = FALSE;
	UINT16	usLineWidthIfWordIsWiderThenWidth;

	if( fDisableJustForIan == TRUE )
	{
		if( ubPriority == MSG_BETAVERSION )
		{
			return;
		}
		else if( ubPriority == MSG_TESTVERSION )
		{
			return;
		}
		else if ( ubPriority == MSG_DEBUG )
		{
			return;
		}
	}

	if( ubPriority == MSG_BETAVERSION )
	{
		usColor = BETAVERSION_COLOR;
		#ifndef JA2BETAVERSION
			#ifndef JA2TESTVERSION
				return;
			#endif
		#endif

		WriteMessageToFile( DestString );
	}

	if( ubPriority == MSG_TESTVERSION )
	{
		usColor = TESTVERSION_COLOR;

		#ifndef JA2TESTVERSION
			 return;
		#endif
		WriteMessageToFile( DestString );
	}
	// OK, check if we are ani imeediate feedback message, if so, do something else!
	if ( ubPriority == MSG_UI_FEEDBACK )
	{
		va_start(argptr, pStringA);       	// Set up variable argument pointer
		vswprintf(DestString, lengthof(DestString), pStringA, argptr);	// process gprintf string (get output str)
		va_end(argptr);

		BeginUIMessage( DestString );
		return;
	}

	if ( ubPriority == MSG_SKULL_UI_FEEDBACK )
	{
		va_start(argptr, pStringA);       	// Set up variable argument pointer
		vswprintf(DestString, lengthof(DestString), pStringA, argptr);	// process gprintf string (get output str)
		va_end(argptr);

		InternalBeginUIMessage( TRUE, DestString );
		return;
	}

	// check if error
	if ( ubPriority == MSG_ERROR )
	{
		va_start(argptr, pStringA);       	// Set up variable argument pointer
		vswprintf(DestString, lengthof(DestString), pStringA, argptr);	// process gprintf string (get output str)
		va_end(argptr);

		swprintf(DestStringA, lengthof(DestStringA), L"DEBUG: %ls", DestString);

		BeginUIMessage( DestStringA );
		WriteMessageToFile( DestStringA );

		return;
	}


		// OK, check if we are an immediate MAP feedback message, if so, do something else!
	if ( ( ubPriority == MSG_MAP_UI_POSITION_UPPER  ) ||
			 ( ubPriority == MSG_MAP_UI_POSITION_MIDDLE ) ||
			 ( ubPriority == MSG_MAP_UI_POSITION_LOWER  ) )
	{
		va_start(argptr, pStringA);       	// Set up variable argument pointer
		vswprintf(DestString, lengthof(DestString), pStringA, argptr);	// process gprintf string (get output str)
		va_end(argptr);

		BeginMapUIMessage( ubPriority, DestString );
		return;
	}


	if ( fFirstTimeInMessageSystem )
	{
		// Init display array!
		memset( gpDisplayList, 0, sizeof( gpDisplayList ) );
		fFirstTimeInMessageSystem = FALSE;
	}


	pStringSt=pStringS;
	while(GetNextString(pStringSt))
		    pStringSt=GetNextString(pStringSt);

	va_start(argptr, pStringA);       	// Set up variable argument pointer
	vswprintf(DestString, lengthof(DestString), pStringA, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	if ( ubPriority == MSG_DEBUG )
	{
		#ifndef _DEBUG
			return;
		#endif
		#ifdef JA2DEMO
			return;
		#endif
		usColor = DEBUG_COLOR;
		wcscpy( DestStringA, DestString );
		swprintf(DestString, lengthof(DestString), L"Debug: %ls", DestStringA);
	}

	if ( ubPriority == MSG_DIALOG )
	{
		usColor = DIALOGUE_COLOR;
	}

	if ( ubPriority == MSG_INTERFACE )
	{
		usColor = INTERFACE_COLOR;
	}

	pStringWrapperHead=LineWrap(uiFont, MAP_LINE_WIDTH, &usLineWidthIfWordIsWiderThenWidth, DestString);
  pStringWrapper=pStringWrapperHead;
	if(!pStringWrapper)
    return;

	fNewString = TRUE;

	while(pStringWrapper->pNextWrappedString!=NULL)
	{
		AddStringToMapScreenMessageList(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority );
		fNewString = FALSE;

		pStringWrapper=pStringWrapper->pNextWrappedString;
	}

  AddStringToMapScreenMessageList(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority );


	// clear up list of wrapped strings
	ClearWrappedStrings( pStringWrapperHead );

	// play new message beep
	//PlayNewMessageSound( );

	MoveToEndOfMapScreenMessageList( );
}


// add string to the map screen message list
static void AddStringToMapScreenMessageList(STR16 pString, UINT16 usColor, UINT32 uiFont, BOOLEAN fStartOfNewString, UINT8 ubPriority)
{
	ScrollStringSt* pStringSt = NULL;


  pStringSt = MemAlloc(sizeof(ScrollStringSt));

	SetString(pStringSt, pString);
  SetStringColor(pStringSt, usColor);
	pStringSt->uiFont = uiFont;
	pStringSt->fBeginningOfNewString = fStartOfNewString;
	pStringSt->uiFlags = ubPriority;
  pStringSt->iVideoOverlay = -1;

	// next/previous are not used, it's strictly a wraparound queue
  SetStringNext(pStringSt, NULL);
  SetStringPrev(pStringSt, NULL);


	// Figure out which queue slot index we're going to use to store this
	// If queue isn't full, this is easy, if is is full, we'll re-use the oldest slot
	// Must always keep the wraparound in mind, although this is easy enough with a static, fixed-size queue.


	// always store the new message at the END index

	// check if slot is being used, if so, clear it up
	if( gMapScreenMessageList[ gubEndOfMapScreenMessageList ] != NULL )
	{
		MemFree( gMapScreenMessageList[ gubEndOfMapScreenMessageList ]->pString16 );
		MemFree( gMapScreenMessageList[ gubEndOfMapScreenMessageList ] );
	}

	// store the new message there
	gMapScreenMessageList[ gubEndOfMapScreenMessageList ] = pStringSt;

	// increment the end
	gubEndOfMapScreenMessageList = ( gubEndOfMapScreenMessageList + 1 ) % 256;

	// if queue is full, end will now match the start
	if ( gubEndOfMapScreenMessageList == gubStartOfMapScreenMessageList )
	{
		// if that's so, increment the start
		gubStartOfMapScreenMessageList = ( gubStartOfMapScreenMessageList + 1 ) % 256;
	}
}


void DisplayStringsInMapScreenMessageList( void )
{
	UINT8 ubCurrentStringIndex;
	UINT8	ubLinesPrinted;
	INT16 sY;
	UINT16 usSpacing;


	SetFontDestBuffer( FRAME_BUFFER, 17, 360 + 6, 407, 360 + 101, FALSE );

	SetFont( MAP_SCREEN_MESSAGE_FONT );		// no longer supports variable fonts
	SetFontBackground( FONT_BLACK );
	SetFontShadow( DEFAULT_SHADOW );

	ubCurrentStringIndex = gubCurrentMapMessageString;

	sY = 377;
	usSpacing = GetFontHeight( MAP_SCREEN_MESSAGE_FONT );

	for ( ubLinesPrinted = 0; ubLinesPrinted < MAX_MESSAGES_ON_MAP_BOTTOM; ubLinesPrinted++ )
	{
		// reached the end of the list?
		if ( ubCurrentStringIndex == gubEndOfMapScreenMessageList )
		{
			break;
		}

		// nothing stored there?
		if ( gMapScreenMessageList[ ubCurrentStringIndex ] == NULL )
		{
			break;
		}

		// set font color
		SetFontForeground( ( UINT8 )( gMapScreenMessageList[ ubCurrentStringIndex ]->usColor ) );

		// print this line
		mprintf_coded( 20, sY, gMapScreenMessageList[ ubCurrentStringIndex ]->pString16 );

		sY += usSpacing;

		// next message index to print (may wrap around)
		ubCurrentStringIndex = ( ubCurrentStringIndex + 1 ) % 256;
	}

	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );
}


void EnableDisableScrollStringVideoOverlay( BOOLEAN fEnable )
{
	// will go through the list of video overlays for the tactical scroll message system, and enable/disable
	// video overlays depending on fEnable
	INT8 bCounter = 0;

	for( bCounter = 0; bCounter < MAX_LINE_COUNT; bCounter++ )
	{

		// if valid, enable/disable
		if( gpDisplayList[ bCounter ] != NULL )
		{
			 EnableVideoOverlay( fEnable ,gpDisplayList[ bCounter ] -> iVideoOverlay );
		}
	}
}


// play beep when new message is added
static void PlayNewMessageSound(void)
{
	// play a new message sound, if there is one playing, do nothing
	static UINT32 uiSoundId = NO_SAMPLE;

	if( uiSoundId != NO_SAMPLE )
	{
		// is sound playing?..don't play new one
		if( SoundIsPlaying( uiSoundId ) == TRUE )
		{
			return;
		}
	}

	// otherwise no sound playing, play one
	uiSoundId = PlayJA2SampleFromFile("Sounds/newbeep.wav", MIDVOLUME, 1, MIDDLEPAN);
}


BOOLEAN SaveMapScreenMessagesToSaveGameFile( HWFILE hFile )
{
	UINT32	uiCount;
	UINT32	uiSizeOfString;
	StringSaveStruct StringSave;


	//	write to the begining of the message list
	if (!FileWrite(hFile, &gubEndOfMapScreenMessageList, sizeof(UINT8))) return FALSE;

	if (!FileWrite(hFile, &gubStartOfMapScreenMessageList, sizeof(UINT8))) return FALSE;

	//	write the current message string
	if (!FileWrite(hFile, &gubCurrentMapMessageString, sizeof(UINT8))) return FALSE;

	//Loopthrough all the messages
	for( uiCount=0; uiCount<256; uiCount++)
	{
		if( gMapScreenMessageList[ uiCount ] )
		{
			uiSizeOfString = (wcslen(gMapScreenMessageList[uiCount]->pString16) + 1) * sizeof(*gMapScreenMessageList[uiCount]->pString16);
		}
		else
			uiSizeOfString = 0;

		//	write to the file the size of the message
		if (!FileWrite(hFile, &uiSizeOfString, sizeof(UINT32))) return FALSE;

		//if there is a message
		if( uiSizeOfString )
		{
			//	write the message to the file
			if (!FileWrite(hFile, gMapScreenMessageList[uiCount]->pString16, uiSizeOfString)) return FALSE;

			// Create  the saved string struct
			StringSave.uiFont = gMapScreenMessageList[ uiCount ]->uiFont;
			StringSave.usColor = gMapScreenMessageList[ uiCount ]->usColor;
			StringSave.fBeginningOfNewString = gMapScreenMessageList[ uiCount ]->fBeginningOfNewString;
			StringSave.uiTimeOfLastUpdate = gMapScreenMessageList[ uiCount ]->uiTimeOfLastUpdate;
			StringSave.uiFlags= gMapScreenMessageList[ uiCount ]->uiFlags;


			//Write the rest of the message information to the saved game file
			if (!FileWrite(hFile, &StringSave, sizeof(StringSaveStruct))) return FALSE;
		}

	}

	return( TRUE );
}


BOOLEAN LoadMapScreenMessagesFromSaveGameFile( HWFILE hFile )
{
	UINT32	uiCount;
	UINT32	uiSizeOfString;
	StringSaveStruct StringSave;
	CHAR16	SavedString[ 512 ];

	// clear tactical message queue
	ClearTacticalMessageQueue( );

	gubEndOfMapScreenMessageList = 0;
	gubStartOfMapScreenMessageList = 0;
	gubCurrentMapMessageString = 0;

	//	Read to the begining of the message list
	if (!FileRead(hFile, &gubEndOfMapScreenMessageList, sizeof(UINT8))) return FALSE;

	//	Read the current message string
	if (!FileRead(hFile, &gubStartOfMapScreenMessageList, sizeof(UINT8))) return FALSE;

	//	Read the current message string
	if (!FileRead(hFile, &gubCurrentMapMessageString, sizeof(UINT8))) return FALSE;

	//Loopthrough all the messages
	for( uiCount=0; uiCount<256; uiCount++)
	{
		//	Read to the file the size of the message
		if (!FileRead(hFile, &uiSizeOfString, sizeof(UINT32))) return FALSE;

		//if there is a message
		if( uiSizeOfString )
		{
			//	Read the message from the file
			if (!FileRead(hFile, SavedString, uiSizeOfString)) return FALSE;

			//if there is an existing string,delete it
			if( gMapScreenMessageList[ uiCount ] )
			{
				if( gMapScreenMessageList[ uiCount ]->pString16 )
				{
					MemFree( gMapScreenMessageList[ uiCount ]->pString16 );
					gMapScreenMessageList[ uiCount ]->pString16 = NULL;
				}
			}
			else
			{
				// There is now message here, add one
				ScrollStringSt	*sScroll;


				sScroll = MemAlloc( sizeof( ScrollStringSt ) );
				if( sScroll == NULL )
					return( FALSE );

				memset( sScroll, 0, sizeof( ScrollStringSt ) );

				gMapScreenMessageList[ uiCount ] = sScroll;
			}

			//allocate space for the new string
			gMapScreenMessageList[ uiCount ]->pString16 = MemAlloc( uiSizeOfString );
			if( gMapScreenMessageList[ uiCount ]->pString16 == NULL )
				return( FALSE );

			memset( gMapScreenMessageList[ uiCount ]->pString16, 0, uiSizeOfString);

			//copy the string over
			wcscpy( gMapScreenMessageList[ uiCount ]->pString16, SavedString );


			//Read the rest of the message information to the saved game file
			if (!FileRead(hFile, &StringSave, sizeof(StringSaveStruct))) return FALSE;

			// Create  the saved string struct
			gMapScreenMessageList[ uiCount ]->uiFont = StringSave.uiFont;
			gMapScreenMessageList[ uiCount ]->usColor = StringSave.usColor;
			gMapScreenMessageList[ uiCount ]->uiFlags = StringSave.uiFlags;
			gMapScreenMessageList[ uiCount ]->fBeginningOfNewString = StringSave.fBeginningOfNewString;
			gMapScreenMessageList[ uiCount ]->uiTimeOfLastUpdate = StringSave.uiTimeOfLastUpdate;
		}
		else
			gMapScreenMessageList[ uiCount ] = NULL;

	}


	// this will set a valid value for gubFirstMapscreenMessageIndex, which isn't being saved/restored
	MoveToEndOfMapScreenMessageList();

	return( TRUE );
}


static void HandleLastQuotePopUpTimer(void)
{
	if( ( fTextBoxMouseRegionCreated == FALSE ) || ( fDialogueBoxDueToLastMessage == FALSE ) )
	{
		return;
	}

	// check if timed out
	if( GetJA2Clock() - guiDialogueLastQuoteTime >  guiDialogueLastQuoteDelay )
	{
		// done clear up
		ShutDownLastQuoteTacticalTextBox( );
		guiDialogueLastQuoteTime = 0;
		guiDialogueLastQuoteDelay = 0;

	}
}


static ScrollStringSt* MoveToBeginningOfMessageQueue(void)
{
	ScrollStringSt* pStringSt = pStringS;

	if( pStringSt == NULL )
	{
		return( NULL );
	}

	while( pStringSt->pPrev )
	{
		pStringSt = pStringSt->pPrev;
	}

	return( pStringSt );
}


static INT32 GetMessageQueueSize(void)
{
	ScrollStringSt* pStringSt = pStringS;
	INT32 iCounter = 0;

	pStringSt = MoveToBeginningOfMessageQueue( );

	while( pStringSt )
	{
		iCounter++;
		pStringSt = pStringSt->pNext;
	}

	return( iCounter );
}



void ClearTacticalMessageQueue( void )
{
	ScrollStringSt* pStringSt = pStringS;
	ScrollStringSt* pOtherStringSt = pStringS;

	ClearDisplayedListOfTacticalStrings( );

	// now run through all the tactical messages
	while( pStringSt )
	{
		pOtherStringSt = pStringSt;
		pStringSt = pStringSt->pNext;
		MemFree( pOtherStringSt-> pString16 );
		MemFree( pOtherStringSt );
	}

	pStringS = NULL;
}


static void WriteMessageToFile(STR16 pString)
{
#ifdef JA2BETAVERSION

	FILE *fp;

	fp = fopen( "DebugMessage.txt", "a" );

	if( fp == NULL )
	{
		return;
	}

	fprintf( fp, "%ls\n", pString );
	fclose( fp );

#endif
}



void InitGlobalMessageList( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		gMapScreenMessageList[ iCounter ] = NULL;
	}

	gubEndOfMapScreenMessageList = 0;
	gubStartOfMapScreenMessageList = 0;
	gubCurrentMapMessageString = 0;
//	ubTempPosition = 0;
}


void FreeGlobalMessageList( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		// check if next unit is empty, if not...clear it up
		if( gMapScreenMessageList[ iCounter ] != NULL )
		{
			MemFree( gMapScreenMessageList[ iCounter ]->pString16 );
			MemFree( gMapScreenMessageList[ iCounter ] );
		}
	}

	InitGlobalMessageList( );
}


UINT8 GetRangeOfMapScreenMessages( void )
{
	UINT8 ubRange = 0;

	// NOTE: End is non-inclusive, so start/end 0/0 means no messages, 0/1 means 1 message, etc.
	if( gubStartOfMapScreenMessageList <= gubEndOfMapScreenMessageList )
	{
		ubRange = gubEndOfMapScreenMessageList - gubStartOfMapScreenMessageList;
	}
	else
	{
		// this should always be 255 now, since this only happens when queue fills up, and we never remove any messages
		ubRange = ( gubEndOfMapScreenMessageList + 256 ) - gubStartOfMapScreenMessageList;
	}

	return ( ubRange );
}
