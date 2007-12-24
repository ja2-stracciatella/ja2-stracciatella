#include "Font.h"
#include "Input.h"
#include "Local.h"
#include "Types.h"
#include "Credits.h"
#include "Timer_Control.h"
#include "VSurface.h"
#include "Text.h"
#include "MouseSystem.h"
#include "WordWrap.h"
#include "Video.h"
#include "Render_Dirty.h"
#include "WCheck.h"
#include "Font_Control.h"
#include "Cursors.h"
#include "SysUtil.h"
#include "English.h"
#include "Encrypted_File.h"
#include "Random.h"
#include "MemMan.h"
#include "Button_System.h"
#include "Debug.h"
#include "ScreenIDs.h"


enum
{
	CRDT_RENDER_NONE,
	CRDT_RENDER_ALL,
};


typedef struct	_CRDT_NODE
{
	CHAR16	*pString;		//string for the node if the node contains a string

	INT16		sPosY;

	INT16		sHeightOfString;		//The height of the displayed string

	BOOLEAN	fDelete;		//Delete this loop

	UINT32	uiLastTime;	// The last time the node was udated

	UINT32	uiVideoSurfaceImage;

	struct _CRDT_NODE	*pPrev;
	struct _CRDT_NODE *pNext;
} 	CRDT_NODE;

//flags for the credits
//Flags:
#define		CRDT_FLAG__TITLE							0x00000001
#define		CRDT_FLAG__START_SECTION			0x00000002
#define		CRDT_FLAG__END_SECTION				0x00000004


#define		CRDT_NAME_OF_CREDIT_FILE				"BINARYDATA/Credits.edt"

#define CREDITS_LINESIZE 80


//
// Code tokens
//
//new codes:
#define CRDT_START_CODE      L'@'
#define CRDT_SEPARATION_CODE L','
#define CRDT_END_CODE        L';'

#define		CRDT_DELAY_BN_STRINGS_CODE			'D'
#define		CRDT_DELAY_BN_SECTIONS_CODE			'B'
#define		CRDT_SCROLL_SPEED								'S'
#define		CRDT_FONT_JUSTIFICATION					'J'
#define		CRDT_TITLE_FONT_COLOR						'C'
#define		CRDT_ACTIVE_FONT_COLOR					'R'

//Flags:
#define		CRDT_TITLE											'T'
#define		CRDT_START_OF_SECTION						'{'
#define		CRDT_END_OF_SECTION							'}'


#define		CRDT_NAME_LOC_X										375
#define		CRDT_NAME_LOC_Y										420
#define		CRDT_NAME_TITLE_LOC_Y							435
#define		CRDT_NAME_FUNNY_LOC_Y							450
#define		CRDT_NAME_LOC_WIDTH								260
#define		CRDT_NAME_LOC_HEIGHT							( CRDT_NAME_FUNNY_LOC_Y - CRDT_NAME_LOC_Y + GetFontHeight( CRDT_NAME_FONT ) )

#define		CRDT_NAME_FONT										FONT12ARIAL

#define		CRDT_LINE_NODE_DISAPPEARS_AT			0//20


#define		CRDT_WIDTH_OF_TEXT_AREA					210
#define		CRDT_TEXT_START_LOC							10


#define		CRDT_SCROLL_PIXEL_AMOUNT				1
#define		CRDT_NODE_DELAY_AMOUNT					25

#define		CRDT_SPACE_BN_SECTIONS					50
#define		CRDT_SPACE_BN_NODES							12

#define CRDT_START_POS_Y        (SCREEN_HEIGHT - 1)

#define		CRDT_EYE_WIDTH									30
#define		CRDT_EYE_HEIGHT									12

#define		CRDT_EYES_CLOSED_TIME						150


typedef struct
{
	INT16		sX;
	INT16		sY;
	INT16		sWidth;
	INT16		sHeight;

	INT16		sEyeX;
	INT16		sEyeY;

	INT16		sMouthX;
	INT16		sMouthY;

	INT16		sBlinkFreq;
	UINT32	uiLastBlinkTime;
	UINT32	uiEyesClosedTime;

} CDRT_FACE;

static CDRT_FACE gCreditFaces[] =
{
	{ 298, 137, 37, 49, 310, 157, 304, 170, 2500, 0, 0 }, // Camfield
	{ 348, 137, 43, 47, 354, 153, 354, 153, 3700, 0, 0 }, // Shawn
	{ 407, 132, 30, 50, 408, 151, 410, 164, 3000, 0, 0 }, // Kris
	{ 443, 131, 30, 50, 447, 151, 446, 161, 4000, 0, 0 }, // Ian
	{ 487, 136, 43, 50, 493, 155, 493, 155, 3500, 0, 0 }, // Linda
	{ 529, 145, 43, 50, 536, 164, 536, 164, 4000, 0, 0 }, // Eric
	{ 581, 132, 43, 48, 584, 150, 583, 161, 3500, 0, 0 }, // Lynn
	{ 278, 211, 36, 51, 283, 232, 283, 241, 3700, 0, 0 }, // Norm
	{ 319, 210, 34, 49, 323, 227, 320, 339, 4000, 0, 0 }, // George
	{ 358, 211, 38, 49, 364, 226, 361, 239, 3600, 0, 0 }, // Andrew Stacey
	{ 396, 200, 42, 50, 406, 220, 403, 230, 4600, 0, 0 }, // Scott
	{ 444, 202, 43, 51, 452, 220, 452, 231, 2800, 0, 0 }, // Emmons
	{ 493, 188, 36, 51, 501, 207, 499, 217, 4500, 0, 0 }, // Dave
	{ 531, 199, 47, 56, 541, 221, 540, 232, 4000, 0, 0 }, // Alex
	{ 585, 196, 39, 49, 593, 218, 593, 228, 3500, 0, 0 }  // Joey
};


static MOUSE_REGION gCrdtMouseRegions[NUM_PEOPLE_IN_CREDITS];


static UINT32  guiCreditBackGroundImage;
static UINT32  guiCreditFaces;
static BOOLEAN gfCreditsScreenEntry = TRUE;
static BOOLEAN gfCreditsScreenExit  = FALSE;
static UINT32  guiCreditsExitScreen;

static UINT8 gubCreditScreenRenderFlags = CRDT_RENDER_ALL;

static CRDT_NODE* gCrdtRootNode      = NULL;
static CRDT_NODE* gCrdtLastAddedNode = NULL;

static BOOLEAN gfCrdtHaveRenderedFirstFrameToSaveBuffer; // need to render background image to save buffer once

static INT32 giCurrentlySelectedFace = -1;


static UINT32 guiCreditScreenActiveFont;  // the font that is used
static UINT32 guiCreditScreenTitleFont;   // the font that is used
static UINT8  gubCreditScreenActiveColor; // color of the font
static UINT8  gubCreditScreenTitleColor;  // color of a Title node

static UINT32 guiCrdtNodeScrollSpeed      = CRDT_NODE_DELAY_AMOUNT; // speed credits go up at
static UINT32 guiCrdtLastTimeUpdatingNode = 0;                      // the last time a node was read from the file
static UINT8  gubCrdtJustification        = CENTER_JUSTIFIED;       // the current justification

static UINT32 guiGapBetweenCreditSections = CRDT_SPACE_BN_SECTIONS;
static UINT32 guiGapBetweenCreditNodes    = CRDT_SPACE_BN_NODES;
static UINT32 guiGapTillReadNextCredit    = CRDT_SPACE_BN_NODES;

static UINT32  guiCurrentCreditRecord = 0;
static BOOLEAN gfPauseCreditScreen    = FALSE;


UINT32	CreditScreenInit( void )
{
	gfCreditsScreenEntry = TRUE;
	return( 1 );
}


static BOOLEAN EnterCreditsScreen(void);
static BOOLEAN ExitCreditScreen(void);
static void GetCreditScreenUserInput(void);
static void HandleCreditScreen(void);


UINT32	CreditScreenHandle( void )
{
	StartFrameBufferRender();

	if( gfCreditsScreenEntry )
	{
		if( !EnterCreditsScreen() )
		{
			gfCreditsScreenEntry = FALSE;
			gfCreditsScreenExit = TRUE;
		}
		else
		{
			gfCreditsScreenEntry = FALSE;
			gfCreditsScreenExit = FALSE;
		}
		gubCreditScreenRenderFlags = CRDT_RENDER_ALL;
	}

	GetCreditScreenUserInput();

	HandleCreditScreen();

	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();


	if( gfCreditsScreenExit )
	{
		ExitCreditScreen();
		gfCreditsScreenEntry = TRUE;
		gfCreditsScreenExit = FALSE;
	}


	return( guiCreditsExitScreen );
}


UINT32	CreditScreenShutdown( void )
{
	return( 1 );
}


static void InitCreditEyeBlinking(void);
static BOOLEAN InitCreditNode(void);
static void SelectCreditFaceMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static BOOLEAN EnterCreditsScreen(void)
{
	UINT32 uiCnt;

	guiCreditBackGroundImage = AddVideoObjectFromFile("INTERFACE/Credits.sti");
	CHECKF(guiCreditBackGroundImage != NO_VOBJECT);
	guiCreditFaces = AddVideoObjectFromFile("INTERFACE/Credit Faces.sti");
	CHECKF(guiCreditFaces != NO_VOBJECT);

	//Initialize the root credit node
	InitCreditNode( );

	guiCreditsExitScreen = CREDIT_SCREEN;
	gfCrdtHaveRenderedFirstFrameToSaveBuffer = FALSE;

	guiCreditScreenActiveFont = FONT12ARIAL;
	gubCreditScreenActiveColor = FONT_MCOLOR_DKWHITE;
	guiCreditScreenTitleFont = FONT14ARIAL;
	gubCreditScreenTitleColor = FONT_MCOLOR_RED;
	guiCrdtNodeScrollSpeed = CRDT_NODE_DELAY_AMOUNT;
	gubCrdtJustification = CENTER_JUSTIFIED;
	guiCurrentCreditRecord = 0;

	guiCrdtLastTimeUpdatingNode = GetJA2Clock();


	guiGapBetweenCreditSections = CRDT_SPACE_BN_SECTIONS;
	guiGapBetweenCreditNodes = CRDT_SPACE_BN_NODES;
	guiGapTillReadNextCredit = CRDT_SPACE_BN_NODES;


	for( uiCnt=0; uiCnt < NUM_PEOPLE_IN_CREDITS; uiCnt++)
	{
		// Make a mouse region
		MSYS_DefineRegion( &gCrdtMouseRegions[uiCnt], gCreditFaces[uiCnt].sX, gCreditFaces[uiCnt].sY, (INT16)(gCreditFaces[uiCnt].sX + gCreditFaces[uiCnt].sWidth), (INT16)(gCreditFaces[uiCnt].sY + gCreditFaces[uiCnt].sHeight), MSYS_PRIORITY_NORMAL,
							 CURSOR_WWW, SelectCreditFaceMovementRegionCallBack, NULL);

		MSYS_SetRegionUserData( &gCrdtMouseRegions[uiCnt], 0, uiCnt );
	}

	giCurrentlySelectedFace = -1;
	gfPauseCreditScreen = FALSE;

	InitCreditEyeBlinking();

	return( TRUE );
}


static BOOLEAN ShutDownCreditList(void);


static BOOLEAN ExitCreditScreen(void)
{
	UINT32	uiCnt;

	DeleteVideoObjectFromIndex( guiCreditBackGroundImage );

	DeleteVideoObjectFromIndex( guiCreditFaces );


	//ShutDown Credit link list
	ShutDownCreditList();

	for( uiCnt=0; uiCnt < NUM_PEOPLE_IN_CREDITS; uiCnt++)
	{
		MSYS_RemoveRegion( &gCrdtMouseRegions[uiCnt] );
	}

	return( TRUE );
}


static BOOLEAN GetNextCreditFromTextFile(void);
static void HandleCreditEyeBlinking(void);
static void HandleCreditNodes(void);
static BOOLEAN RenderCreditScreen(void);
static void SetCreditsExitScreen(UINT32 uiScreenToGoTo);


static void HandleCreditScreen(void)
{
	if( gubCreditScreenRenderFlags == CRDT_RENDER_ALL )
	{
		RenderCreditScreen();
		gubCreditScreenRenderFlags = CRDT_RENDER_NONE;
	}


	//Handle the Credit linked list
	HandleCreditNodes();

	//Handle the blinkng eyes
	HandleCreditEyeBlinking();


	//is it time to get a new node
	if( gCrdtLastAddedNode == NULL || ( CRDT_START_POS_Y - ( gCrdtLastAddedNode->sPosY + gCrdtLastAddedNode->sHeightOfString - 16 ) ) >= (INT16)guiGapTillReadNextCredit )
	{
		//if there are no more credits in the file
		if( !GetNextCreditFromTextFile() && gCrdtLastAddedNode == NULL )
		{
			SetCreditsExitScreen( MAINMENU_SCREEN );
		}
	}



	RestoreExternBackgroundRect( CRDT_NAME_LOC_X, CRDT_NAME_LOC_Y, CRDT_NAME_LOC_WIDTH, (INT16)CRDT_NAME_LOC_HEIGHT );

	if( giCurrentlySelectedFace != -1 )
	{
		DrawTextToScreen(gzCreditNames[giCurrentlySelectedFace],     CRDT_NAME_LOC_X, CRDT_NAME_LOC_Y,       CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, INVALIDATE_TEXT | CENTER_JUSTIFIED);
		DrawTextToScreen(gzCreditNameTitle[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_TITLE_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, INVALIDATE_TEXT | CENTER_JUSTIFIED);
		DrawTextToScreen(gzCreditNameFunny[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_FUNNY_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, INVALIDATE_TEXT | CENTER_JUSTIFIED);
	}
}


static BOOLEAN RenderCreditScreen(void)
{
  BltVideoObjectFromIndex(FRAME_BUFFER, guiCreditBackGroundImage, 0, 0, 0);
	if( !gfCrdtHaveRenderedFirstFrameToSaveBuffer )
	{
		gfCrdtHaveRenderedFirstFrameToSaveBuffer = TRUE;

		//blit everything to the save buffer ( cause the save buffer can bleed through )
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);

		UnmarkButtonsDirty( );
	}

	InvalidateScreen( );
	return( TRUE );
}


static void GetCreditScreenUserInput(void)
{
	InputAtom Event;

	while( DequeueEvent( &Event ) )
	{
		if( Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case SDLK_ESCAPE: SetCreditsExitScreen(MAINMENU_SCREEN); break;

#ifdef JA2TESTVERSION
				case 'r':
					gubCreditScreenRenderFlags = CRDT_RENDER_ALL;
					break;

				case 'i':
					InvalidateScreen();
					break;

				case SDLK_UP:
					guiCrdtNodeScrollSpeed+=5;
					break;
				case SDLK_DOWN:
					if( guiCrdtNodeScrollSpeed > 5 )
						guiCrdtNodeScrollSpeed-=5;
					break;

				case SDLK_PAUSE:
				case SDLK_SPACE:
					if( gfPauseCreditScreen )
						gfPauseCreditScreen = FALSE;
					else
						gfPauseCreditScreen = TRUE;
					break;
#endif
			}
		}
	}
}


static void SetCreditsExitScreen(UINT32 uiScreenToGoTo)
{
	gfCreditsScreenExit = TRUE;

	guiCreditsExitScreen = uiScreenToGoTo;
}


static BOOLEAN InitCreditNode(void)
{
	if( gCrdtRootNode != NULL )
		Assert( 0 );

	gCrdtRootNode = NULL;

	return( TRUE );
}


static BOOLEAN DeleteNode(CRDT_NODE* pNodeToDelete);


static BOOLEAN ShutDownCreditList(void)
{
	CRDT_NODE	*pNodeToDelete=NULL;
	CRDT_NODE	*pTemp=NULL;

	pNodeToDelete = gCrdtRootNode;


	while( pNodeToDelete!= NULL )
	{
		pTemp = pNodeToDelete;

		pNodeToDelete = pNodeToDelete->pNext;

		//Delete the current node
		DeleteNode( pTemp );
	}

	return( TRUE );
}


static BOOLEAN DeleteNode(CRDT_NODE* pNodeToDelete)
{
	CRDT_NODE	*pTempNode;

	pTempNode = pNodeToDelete;

	if( gCrdtLastAddedNode == pNodeToDelete )
	{
		gCrdtLastAddedNode = NULL;
	}

	//if its Not the first node
	if( pNodeToDelete->pPrev != NULL )
		pNodeToDelete->pPrev = pNodeToDelete->pNext;
	else
	{
		if( gCrdtRootNode->pNext != NULL )
		{
			gCrdtRootNode = gCrdtRootNode->pNext;
			gCrdtRootNode->pPrev = NULL;
		}
	}

	//if its the last node in the list
	if( pNodeToDelete->pNext == NULL && pNodeToDelete->pPrev != NULL )
		pNodeToDelete->pPrev->pNext = NULL;

	//iof the node that is being deleted is the first node
	if( pTempNode == gCrdtRootNode )
		gCrdtRootNode = NULL;

	//Free the string
	if( pTempNode->pString != NULL )
	{
		MemFree( pTempNode->pString );
		pTempNode->pString = NULL;
	}

	DeleteVideoSurfaceFromIndex(pTempNode->uiVideoSurfaceImage);
	pTempNode->uiVideoSurfaceImage = 0;

	//Free the node
	MemFree( pTempNode );
	pTempNode = NULL;

	return( TRUE );
}


static BOOLEAN AddCreditNode(UINT32 uiFlags, const wchar_t* pString)
{
	CRDT_NODE	*pNodeToAdd=NULL;
	CRDT_NODE	*pTemp=NULL;
	UINT32	uiFontToUse;
	UINT8		uiColorToUse;

	pNodeToAdd = MemAlloc( sizeof( CRDT_NODE) );
	if( pNodeToAdd == NULL )
	{
		return( FALSE );
	}
	memset( pNodeToAdd, 0, sizeof( CRDT_NODE) );


	//Determine the font and the color to use
	if( uiFlags & CRDT_FLAG__TITLE )
	{
		uiFontToUse = guiCreditScreenTitleFont;
		uiColorToUse = gubCreditScreenTitleColor;
	}
	else
	{
		uiFontToUse = guiCreditScreenActiveFont;
		uiColorToUse = gubCreditScreenActiveColor;
	}

	//
	// Set some default data
	//

	//Allocate memory for the string
	pNodeToAdd->pString = MemAlloc(sizeof(*pNodeToAdd->pString) * (wcslen(pString) + 1));
	if( pNodeToAdd->pString == NULL )
		return( FALSE );

	//copy the string into the node
	wcscpy( pNodeToAdd->pString, pString );

	//Calculate the height of the string
	pNodeToAdd->sHeightOfString = DisplayWrappedString(0, 0, CRDT_WIDTH_OF_TEXT_AREA, 2, uiFontToUse, uiColorToUse, pNodeToAdd->pString, 0, DONT_DISPLAY_TEXT) + 1;

	//starting y position on the screen
	pNodeToAdd->sPosY = CRDT_START_POS_Y;

//	pNodeToAdd->uiLastTime = GetJA2Clock();

	pNodeToAdd->uiVideoSurfaceImage = AddVideoSurface(CRDT_WIDTH_OF_TEXT_AREA, pNodeToAdd->sHeightOfString, PIXEL_DEPTH);
	if (pNodeToAdd->uiVideoSurfaceImage == NO_VSURFACE) return FALSE;

	//Set transparency
	SetVideoSurfaceTransparency(pNodeToAdd->uiVideoSurfaceImage, 0);

	//fill the surface with a transparent color
	FillSurface(pNodeToAdd->uiVideoSurfaceImage, 0);

	//set the font dest buffer to be the surface
	SetFontDestBuffer(pNodeToAdd->uiVideoSurfaceImage, 0, 0, CRDT_WIDTH_OF_TEXT_AREA, pNodeToAdd->sHeightOfString);

	//write the string onto the surface
	DisplayWrappedString(0, 1, CRDT_WIDTH_OF_TEXT_AREA, 2, uiFontToUse, uiColorToUse, pNodeToAdd->pString, 0, gubCrdtJustification);

	//reset the font dest buffer
	SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	//
	// Insert the node into the list
	//

	//if its the first node to add
	if( gCrdtRootNode == NULL )
	{
		//make the new node the root node
		gCrdtRootNode = pNodeToAdd;

		gCrdtRootNode->pNext = NULL;
		gCrdtRootNode->pPrev = NULL;
	}
	else
	{
		pTemp = gCrdtRootNode;

		while( pTemp->pNext != NULL )
		{
			pTemp = pTemp->pNext;
		}

		//Add the new node to the list
		pTemp->pNext = pNodeToAdd;

		//Assign the prev node
		pNodeToAdd->pPrev = pTemp;
	}

	gCrdtLastAddedNode = pNodeToAdd;

	return( TRUE );
}


static void HandleNode_Default(CRDT_NODE* pCurrent);


static void HandleCreditNodes(void)
{
	CRDT_NODE	*pCurrent=NULL;
	CRDT_NODE	*pTemp=NULL;

	if( gCrdtRootNode == NULL )
		return;

	//if the screen is paused, exit
	if( gfPauseCreditScreen )
		return;


	pCurrent = gCrdtRootNode;

	if( !( GetJA2Clock() - guiCrdtLastTimeUpdatingNode > guiCrdtNodeScrollSpeed ) )
		return;

	//loop through all the nodes
	while( pCurrent != NULL )
	{
		pTemp = pCurrent;

		pCurrent = pCurrent->pNext;

		HandleNode_Default(pTemp);

		//if the node is to be deleted
		if( pTemp->fDelete )
		{
			//delete the node
			DeleteNode( pTemp );
		}
	}

	guiCrdtLastTimeUpdatingNode = GetJA2Clock();
}


static BOOLEAN DisplayCreditNode(CRDT_NODE* pCurrent);


static void HandleNode_Default(CRDT_NODE* pCurrent)
{
	//Display the Current Node
	DisplayCreditNode(pCurrent);

	//Move the current node up
	pCurrent->sPosY -= CRDT_SCROLL_PIXEL_AMOUNT;

	//if the node is entirely off the screen
	if (pCurrent->sPosY + pCurrent->sHeightOfString < CRDT_LINE_NODE_DISAPPEARS_AT)
	{
		//mark the node to be deleted this frame
		pCurrent->fDelete = TRUE;
	}

	pCurrent->uiLastTime = GetJA2Clock();
}


static BOOLEAN DisplayCreditNode(CRDT_NODE* pCurrent)
{
	//Currently, we have no need to display a node that doesnt have a string
	if( pCurrent->pString == NULL )
		return( FALSE );


	//if the node is new and we havent displayed it yet
	if( pCurrent->uiLastTime == 0 )
	{

	}

	//else we have to restore were the string was
	else
	{
		//Restore the background before blitting the text back on
		INT16 y = pCurrent->sPosY + CRDT_SCROLL_PIXEL_AMOUNT;
		INT16 h = pCurrent->sHeightOfString;
		/* Clip to the screen area */
		if (y - CRDT_LINE_NODE_DISAPPEARS_AT < 0)
		{
			h += y - CRDT_LINE_NODE_DISAPPEARS_AT;
			y = 0;
		}
		else if (y + pCurrent->sHeightOfString - SCREEN_HEIGHT > 0)
		{
			h -= y + pCurrent->sHeightOfString - SCREEN_HEIGHT;
		}
		RestoreExternBackgroundRect(CRDT_TEXT_START_LOC, y, CRDT_WIDTH_OF_TEXT_AREA, h);
	}

	BltVideoSurface(FRAME_BUFFER, pCurrent->uiVideoSurfaceImage, CRDT_TEXT_START_LOC, pCurrent->sPosY, NULL);

	return( TRUE );
}


static UINT32 GetNumber(const wchar_t* const string)
{
	unsigned int v = 0;
	swscanf(string, L"%u", &v);
	return v;
}


static void HandleCreditFlags(UINT32 uiFlags);


static BOOLEAN GetNextCreditFromTextFile(void)
{
	wchar_t text[CREDITS_LINESIZE];
	const UINT32 pos = CREDITS_LINESIZE * guiCurrentCreditRecord++;
	if (!LoadEncryptedDataFromFile(CRDT_NAME_OF_CREDIT_FILE, text, pos, CREDITS_LINESIZE))
	{
		return FALSE;
	}

	UINT32         flags = 0;
	const wchar_t* s     = text;
	if (*s == CRDT_START_CODE)
	{
		for (;;)
		{
			++s;
			/* process code */
			switch (*s++)
			{
				case CRDT_DELAY_BN_STRINGS_CODE:  guiGapBetweenCreditNodes    = GetNumber(s); break;
				case CRDT_DELAY_BN_SECTIONS_CODE: guiGapBetweenCreditSections = GetNumber(s); break;
				case CRDT_SCROLL_SPEED:           guiCrdtNodeScrollSpeed      = GetNumber(s); break;
				case CRDT_TITLE_FONT_COLOR:       gubCreditScreenTitleColor   = GetNumber(s); break;
				case CRDT_ACTIVE_FONT_COLOR:      gubCreditScreenActiveColor  = GetNumber(s); break;

				case CRDT_FONT_JUSTIFICATION:
					switch (GetNumber(s))
					{
						case 0:  gubCrdtJustification = LEFT_JUSTIFIED;   break;
						case 1:  gubCrdtJustification = CENTER_JUSTIFIED; break;
						case 2:  gubCrdtJustification = RIGHT_JUSTIFIED;  break;
						default: Assert(0);                               break;
					}
					break;

				case CRDT_TITLE:            flags |= CRDT_FLAG__TITLE;         break;
				case CRDT_START_OF_SECTION: flags |= CRDT_FLAG__START_SECTION; break;
				case CRDT_END_OF_SECTION:   flags |= CRDT_FLAG__END_SECTION;   break;

				default: Assert(0); break;
			}

			/* skip till the next code or end of codes */
			while (*s != CRDT_SEPARATION_CODE)
			{
				switch (*s)
				{
					case CRDT_END_CODE:  ++s; goto handle_text;
					case L'\0':               goto handle_text;
					default:             ++s; break;
				}
			}
		}
	}

handle_text:
	if (*s != L'\0') AddCreditNode(flags, s);
	HandleCreditFlags(flags);
	return TRUE;
}


static void HandleCreditFlags(UINT32 uiFlags)
{
	if( uiFlags & CRDT_FLAG__START_SECTION )
	{
		guiGapTillReadNextCredit = guiGapBetweenCreditNodes;
	}

	if( uiFlags & CRDT_FLAG__END_SECTION )
	{
		guiGapTillReadNextCredit = guiGapBetweenCreditSections;
	}
}


static void SelectCreditFaceMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giCurrentlySelectedFace = -1;
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		giCurrentlySelectedFace = MSYS_GetRegionUserData( pRegion,0 );
	}
}


static void InitCreditEyeBlinking(void)
{
	UINT8 ubCnt;

	for( ubCnt=0; ubCnt<NUM_PEOPLE_IN_CREDITS; ubCnt++ )
	{
		gCreditFaces[ubCnt].uiLastBlinkTime = GetJA2Clock() + Random( gCreditFaces[ubCnt].sBlinkFreq * 2 );
	}
}


static void HandleCreditEyeBlinking(void)
{
	UINT8 ubCnt;

	HVOBJECT hPixHandle = GetVideoObject(guiCreditFaces);

	for( ubCnt=0; ubCnt<NUM_PEOPLE_IN_CREDITS; ubCnt++ )
	{
		if( ( GetJA2Clock() - gCreditFaces[ubCnt].uiLastBlinkTime ) > (UINT32)gCreditFaces[ubCnt].sBlinkFreq )
		{
		  BltVideoObject( FRAME_BUFFER, hPixHandle, (UINT8)(ubCnt*3), gCreditFaces[ubCnt].sEyeX, gCreditFaces[ubCnt].sEyeY);
			InvalidateRegion( gCreditFaces[ubCnt].sEyeX, gCreditFaces[ubCnt].sEyeY, gCreditFaces[ubCnt].sEyeX + CRDT_EYE_WIDTH, gCreditFaces[ubCnt].sEyeY + CRDT_EYE_HEIGHT );

			gCreditFaces[ubCnt].uiLastBlinkTime = GetJA2Clock();

			gCreditFaces[ubCnt].uiEyesClosedTime = GetJA2Clock() + CRDT_EYES_CLOSED_TIME + Random( CRDT_EYES_CLOSED_TIME );
		}
		else if( GetJA2Clock() > gCreditFaces[ubCnt].uiEyesClosedTime )
		{
			gCreditFaces[ubCnt].uiEyesClosedTime = 0;

			RestoreExternBackgroundRect( gCreditFaces[ubCnt].sEyeX, gCreditFaces[ubCnt].sEyeY, CRDT_EYE_WIDTH, CRDT_EYE_HEIGHT );
		}
	}
}
