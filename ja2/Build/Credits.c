#include "Font.h"
#include "Local.h"
#include "Types.h"
#include "Credits.h"
#include "VSurface.h"
#include "Text.h"
#include "MouseSystem.h"
#include "WordWrap.h"
#include "Video.h"
#include "Render_Dirty.h"
#include "Utilities.h"
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
	UINT32	uiType;			//the type of node

	CHAR16	*pString;		//string for the node if the node contains a string

	UINT32	uiFlags;		//various flags

	INT16		sPosX;			//position of the node on the screen if the node is displaying stuff
	INT16		sPosY;

	INT16		sOldPosX;			//position of the node on the screen if the node is displaying stuff
	INT16		sOldPosY;

	INT16		sHeightOfString;		//The height of the displayed string

	BOOLEAN	fDelete;		//Delete this loop

	UINT32	uiLastTime;	// The last time the node was udated

	UINT32	uiVideoSurfaceImage;

	struct _CRDT_NODE	*pPrev;
	struct _CRDT_NODE *pNext;
} 	CRDT_NODE;

//type of credits
enum
{
	CRDT_NODE_NONE,
	CRDT_NODE_DEFAULT,		// scrolls up and off the screen
};

//flags for the credits
//Flags:
#define		CRDT_FLAG__TITLE							0x00000001
#define		CRDT_FLAG__START_SECTION			0x00000002
#define		CRDT_FLAG__END_SECTION				0x00000004


//#define		CRDT_NAME_OF_CREDIT_FILE				"BINARYDATA/Credits.txt"
#define		CRDT_NAME_OF_CREDIT_FILE				"BINARYDATA/Credits.edt"

#define		CREDITS_LINESIZE								80 * 2


//
// Code tokens
//
//new codes:
#define		CRDT_START_CODE									'@'
#define		CRDT_SEPARATION_CODE						L","
#define		CRDT_END_CODE										L";"

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

/*
//new codes:
enum
{
	CRDT_ERROR,
	CRDT_CODE_DELAY_BN_STRINGS,
	CRDT_CODE_SCROLL_SPEED,
	CRDT_CODE_FONT_JUSIFICATION,
	CRDT_CODE_FONT_COLOR,

	CRDT_NUM_CODES,
};
*/


#define		CRDT_WIDTH_OF_TEXT_AREA					210
#define		CRDT_TEXT_START_LOC							10


#define		CRDT_SCROLL_PIXEL_AMOUNT				1
#define		CRDT_NODE_DELAY_AMOUNT					25
#define		CRDT_DELAY_BN_NODES							750
#define		CRDT_DELAY_BN_SECTIONS					2500

#define		CRDT_SPACE_BN_SECTIONS					50
#define		CRDT_SPACE_BN_NODES							12

#define		CRDT_START_POS_Y								479

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

CDRT_FACE		gCreditFaces[] =
{
//  x		y				w		h
	298, 137,			37, 49, 310, 157,		304, 170,	2500, 0, 0,											//Camfield
	348, 137,			43, 47, 354, 153,		354, 153,	3700, 0, 0,											//Shawn
	407, 132,			30, 50, 408, 151,		410, 164,	3000, 0, 0,											//Kris
	443, 131,			30, 50, 447, 151,		446, 161,	4000, 0, 0,											//Ian
	487, 136,			43, 50, 493, 155,		493, 155,	3500, 0, 0,											//Linda
	529, 145,			43, 50, 536, 164,		536, 164,	4000, 0, 0,											//Eric
	581, 132,			43, 48, 584, 150,		583, 161,	3500, 0, 0,											//Lynn
	278, 211,			36,	51, 283, 232,		283, 241,	3700, 0, 0,											//Norm
	319, 210,			34, 49, 323, 227,		320, 339,	4000, 0, 0,											//George
	358, 211,			38, 49, 364, 226,		361, 239,	3600, 0, 0,											//Andrew Stacey
	396, 200,			42, 50, 406, 220,		403, 230,	4600, 0, 0,											//Scott
	444, 202,			43, 51, 452, 220,		452, 231,	2800, 0, 0,											//Emmons
	493, 188,			36, 51, 501, 207,		499, 217,	4500, 0, 0,											//Dave
	531, 199,			47, 56, 541, 221,		540, 232,	4000, 0, 0,											//Alex
	585, 196,			39, 49, 593, 218,		593, 228,	3500, 0, 0,											//Joey
};

/*
enum
{
	CRDT_CAMFIELD,
	CRDT_SHAWN,
	CRDT_KRIS,
	CRDT_IAN,
	CRDT_LINDA,
	CRDT_ERIC,
	CRDT_LYNN,
	CRDT_NORM,
	CRDT_GEORGE,
	CRDT_STACEY,
	CRDT_SCOTT,
	CRDT_EMMONS,
	CRDT_DAVE,
	CRDT_ALEX,
	CRDT_JOEY,

	NUM_PEOPLE_IN_CREDITS,
};

STR16	gzCreditNames[]=
{
	L"Chris Camfield",
	L"Shaun Lyng",
	L"Kris Märnes",
	L"Ian Currie",
	L"Linda Currie",
	L"Eric \"WTF\" Cheng",
	L"Lynn Holowka",
	L"Norman \"NRG\" Olsen",
	L"George Brooks",
	L"Andrew Stacey",
	L"Scot Loving",
	L"Andrew \"Big Cheese\" Emmons",
	L"Dave \"The Feral\" French",
	L"Alex Meduna",
	L"Joey \"Joeker\" Whelan",
};


STR16	gzCreditNameTitle[]=
{
	L"Game Internals Programmer", 			// Chris Camfield
	L"Co-designer/Writer",							// Shaun Lyng
	L"Strategic Systems & Editor Programmer",					//Kris \"The Cow Rape Man\" Marnes
	L"Producer/Co-designer",						// Ian Currie
	L"Co-designer/Map Designer",				// Linda Currie
	L"Artist",													// Eric \"WTF\" Cheng
	L"Beta Coordinator, Support",				// Lynn Holowka
	L"Artist Extraordinaire",						// Norman \"NRG\" Olsen
	L"Sound Guru",											// George Brooks
	L"Screen Designer/Artist",					// Andrew Stacey
	L"Lead Artist/Animator",						// Scot Loving
	L"Lead Programmer",									// Andrew \"Big Cheese Doddle\" Emmons
	L"Programmer",											// Dave French
	L"Strategic Systems & Game Balance Programmer",					// Alex Meduna
	L"Portraits Artist",								// Joey \"Joeker\" Whelan",
};

STR16	gzCreditNameFunny[]=
{
	L"", 																			// Chris Camfield
	L"(still learning punctuation)",					// Shaun Lyng
	L"(\"It's done. I'm just fixing it\")",	//Kris \"The Cow Rape Man\" Marnes
	L"(getting much too old for this)",				// Ian Currie
	L"(and working on Wizardry 8)",						// Linda Currie
	L"(forced at gunpoint to also do QA)",			// Eric \"WTF\" Cheng
	L"(Left us for the CFSA - go figure...)",	// Lynn Holowka
	L"",																			// Norman \"NRG\" Olsen
	L"",																			// George Brooks
	L"(Dead Head and jazz lover)",						// Andrew Stacey
	L"(his real name is Robert)",							// Scot Loving
	L"(the only responsible person)",					// Andrew \"Big Cheese Doddle\" Emmons
	L"(can now get back to motocrossing)",	// Dave French
	L"(stolen from Wizardry 8)",							// Alex Meduna
	L"(did items and loading screens too!)",	// Joey \"Joeker\" Whelan",
};

*/


MOUSE_REGION	gCrdtMouseRegions[ NUM_PEOPLE_IN_CREDITS ];


UINT32		guiCreditBackGroundImage;
UINT32		guiCreditFaces;
BOOLEAN		gfCreditsScreenEntry = TRUE;
BOOLEAN		gfCreditsScreenExit = FALSE;
UINT32		guiCreditsExitScreen;

UINT8			gubCreditScreenRenderFlags = CRDT_RENDER_ALL;

CRDT_NODE	*gCrdtRootNode=NULL;
CRDT_NODE	*gCrdtLastAddedNode=NULL;

BOOLEAN		gfCrdtHaveRenderedFirstFrameToSaveBuffer;		// need to render background image to save buffer once

INT32			giCurrentlySelectedFace = -1;

//
//VAriables needed for processing of the nodes:
//


UINT32		guiCreditScreenActiveFont;				// the font that is used
UINT32		guiCreditScreenTitleFont;				// the font that is used
UINT8			gubCreditScreenActiveColor;				// color of the font
UINT8			gubCreditScreenTitleColor;				// color of a Title node
//UINT32		guiCreditScreenActiveDisplayFlags;	//

UINT32		guiCrdtNodeScrollSpeed = CRDT_NODE_DELAY_AMOUNT;	//speed credits go up at
//UINT32		guiCrdtTimeTillReadNextCredit = CRDT_DELAY_BN_SECTIONS;		//the delay before reading the next credit ( normall = guiCrdtDelayBetweenCreditSection or guiCrdtDelayBetweenNodes )
//UINT32		guiCrdtDelayBetweenCreditSection = CRDT_DELAY_BN_SECTIONS;		//delay between major credits sections ( programming and art ) appearing on the screen
//UINT32		guiCrdtDelayBetweenNodes = CRDT_DELAY_BN_NODES;		//delay between credits appearing on the screen
UINT32		guiCrdtLastTimeUpdatingNode=0;											// the last time a node was read from the file
UINT8			gubCrdtJustification = CENTER_JUSTIFIED;					// the current justification

UINT32		guiGapBetweenCreditSections = CRDT_SPACE_BN_SECTIONS;
UINT32		guiGapBetweenCreditNodes = CRDT_SPACE_BN_NODES;
UINT32		guiGapTillReadNextCredit = CRDT_SPACE_BN_NODES;

UINT32		guiCurrentCreditRecord = 0;
BOOLEAN		gfPauseCreditScreen = FALSE;

HWFILE		ghFile;


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

	// render buttons marked dirty
//	MarkButtonsDirty( );
//	RenderButtons( );

	// render help
//	RenderFastHelp( );
//	RenderButtonsFastHelp( );


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
//	guiCreditScreenActiveDisplayFlags = LEFT_JUSTIFIED;
	guiCrdtNodeScrollSpeed = CRDT_NODE_DELAY_AMOUNT;
	gubCrdtJustification = CENTER_JUSTIFIED;
	guiCurrentCreditRecord = 0;

//	guiCrdtTimeTillReadNextCredit = CRDT_DELAY_BN_SECTIONS;
//	guiCrdtDelayBetweenCreditSection = CRDT_DELAY_BN_SECTIONS;
//	guiCrdtDelayBetweenNodes = CRDT_DELAY_BN_NODES;

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


	//Test Node
	{
//		AddCreditNode( CRDT_NODE_DEFAULT, L"This is a test" );
	}

/*
	//open the credit text file
	ghFile = FileOpen( CRDT_NAME_OF_CREDIT_FILE, FILE_ACCESS_READ | FILE_OPEN_EXISTING);
	if( !ghFile )
	{
		return( FALSE );
	}
*/

	giCurrentlySelectedFace = -1;
	gfPauseCreditScreen = FALSE;

	InitCreditEyeBlinking();

	return( TRUE );
}


static BOOLEAN ShutDownCreditList(void);


static BOOLEAN ExitCreditScreen(void)
{
	UINT32	uiCnt;


	//Blit the background image
//	DeleteVideoSurfaceFromIndex( guiCreditBackGroundImage );
	DeleteVideoObjectFromIndex( guiCreditBackGroundImage );

	DeleteVideoObjectFromIndex( guiCreditFaces );


	//ShutDown Credit link list
	ShutDownCreditList();

	for( uiCnt=0; uiCnt < NUM_PEOPLE_IN_CREDITS; uiCnt++)
	{
		MSYS_RemoveRegion( &gCrdtMouseRegions[uiCnt] );
	}

/*
	//close the text file
	FileClose( ghFile );
	ghFile = 0;
*/

	return( TRUE );
}


static BOOLEAN GetNextCreditFromTextFile(void);
static void HandleCreditEyeBlinking(void);
static void HandleCreditNodes(void);
static BOOLEAN RenderCreditScreen(void);
static void SetCreditsExitScreen(UINT32 uiScreenToGoTo);


static void HandleCreditScreen(void)
{
//	UINT32	uiTime = GetJA2Clock();

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
		DrawTextToScreen( gzCreditNames[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, FALSE, INVALIDATE_TEXT | CENTER_JUSTIFIED );
		DrawTextToScreen( gzCreditNameTitle[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_TITLE_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, FALSE, INVALIDATE_TEXT | CENTER_JUSTIFIED );
		DrawTextToScreen( gzCreditNameFunny[giCurrentlySelectedFace], CRDT_NAME_LOC_X, CRDT_NAME_FUNNY_LOC_Y, CRDT_NAME_LOC_WIDTH, CRDT_NAME_FONT, FONT_MCOLOR_WHITE, 0, FALSE, INVALIDATE_TEXT | CENTER_JUSTIFIED );
	}
}


static BOOLEAN RenderCreditScreen(void)
{
  BltVideoObjectFromIndex(FRAME_BUFFER, guiCreditBackGroundImage, 0, 0, 0);
	if( !gfCrdtHaveRenderedFirstFrameToSaveBuffer )
	{
		gfCrdtHaveRenderedFirstFrameToSaveBuffer = TRUE;

		//blit everything to the save buffer ( cause the save buffer can bleed through )
		BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480);

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
					InvalidateRegion( 0, 0, 640, 480 );
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


	//if the node had something to display, delete a surface for it
	if( pTempNode->uiType == CRDT_NODE_DEFAULT )
	{
		DeleteVideoSurfaceFromIndex( pTempNode->uiVideoSurfaceImage );
		pTempNode->uiVideoSurfaceImage = 0;
	}


	//Free the node
	MemFree( pTempNode );
	pTempNode = NULL;

	return( TRUE );
}


static BOOLEAN AddCreditNode(UINT32 uiType, UINT32 uiFlags, STR16 pString)
{
	CRDT_NODE	*pNodeToAdd=NULL;
	CRDT_NODE	*pTemp=NULL;
	UINT32	uiFontToUse;
	UINT8		uiColorToUse;

	//if
	if( uiType == CRDT_NODE_NONE)
	{
		//Assert( 0 );
		return( TRUE );
	}

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
/*
	else if ( uiFlags & CRDT_FLAG__START_SECTION )
	{
		uiFontToUse = ;
		uiColorToUse = ;
	}
	else if ( uiFlags & CRDT_FLAG__END_SECTION )
	{
		uiFontToUse = ;
		uiColorToUse = ;
	}
*/
	else
	{
		uiFontToUse = guiCreditScreenActiveFont;
		uiColorToUse = gubCreditScreenActiveColor;
	}

	//
	// Set some default data
	//

	//the type of the node
	pNodeToAdd->uiType = uiType;

	//any flags that are added
	pNodeToAdd->uiFlags = uiFlags;

	//the starting left position for the it
	pNodeToAdd->sPosX = CRDT_TEXT_START_LOC;

	//Allocate memory for the string
	pNodeToAdd->pString = MemAlloc(sizeof(*pNodeToAdd->pString) * (wcslen(pString) + 1));
	if( pNodeToAdd->pString == NULL )
		return( FALSE );

	//copy the string into the node
	wcscpy( pNodeToAdd->pString, pString );

	//Calculate the height of the string
	pNodeToAdd->sHeightOfString = 	DisplayWrappedString( 0, 0, CRDT_WIDTH_OF_TEXT_AREA, 2, uiFontToUse, uiColorToUse, pNodeToAdd->pString, 0, FALSE, DONT_DISPLAY_TEXT ) + 1;

	//starting y position on the screen
	pNodeToAdd->sPosY = CRDT_START_POS_Y;

//	pNodeToAdd->uiLastTime = GetJA2Clock();

	//if the node can have something to display, Create a surface for it
	if( pNodeToAdd->uiType == CRDT_NODE_DEFAULT )
	{
		pNodeToAdd->uiVideoSurfaceImage = AddVideoSurface(CRDT_WIDTH_OF_TEXT_AREA, pNodeToAdd->sHeightOfString, PIXEL_DEPTH);
		if (pNodeToAdd->uiVideoSurfaceImage == NO_VSURFACE) return FALSE;

		//Set transparency
		SetVideoSurfaceTransparency( pNodeToAdd->uiVideoSurfaceImage, 0 );

		//fill the surface with a transparent color
		ColorFillVideoSurfaceArea(pNodeToAdd->uiVideoSurfaceImage, 0, 0, CRDT_WIDTH_OF_TEXT_AREA,	pNodeToAdd->sHeightOfString, 0 );

		//set the font dest buffer to be the surface
		SetFontDestBuffer( pNodeToAdd->uiVideoSurfaceImage, 0, 0, CRDT_WIDTH_OF_TEXT_AREA, pNodeToAdd->sHeightOfString, FALSE );

		//write the string onto the surface
		DisplayWrappedString( 0, 1, CRDT_WIDTH_OF_TEXT_AREA, 2, uiFontToUse, uiColorToUse, pNodeToAdd->pString, 0, FALSE, gubCrdtJustification );

		//reset the font dest buffer
		SetFontDestBuffer(FRAME_BUFFER, 0,0,640,480, FALSE);
	}




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


static void HandleCurrentCreditNode(CRDT_NODE* pCurrent);


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

		//Handle the current node
		HandleCurrentCreditNode( pTemp );

		//if the node is to be deleted
		if( pTemp->fDelete )
		{
			//delete the node
			DeleteNode( pTemp );
		}
	}

//	RestoreExternBackgroundRect( CRDT_TEXT_START_LOC, 0, CRDT_WIDTH_OF_TEXT_AREA, CRDT_LINE_NODE_DISAPPEARS_AT );

	guiCrdtLastTimeUpdatingNode = GetJA2Clock();
}


static void HandleNode_Default(CRDT_NODE* pCurrent);


static void HandleCurrentCreditNode(CRDT_NODE* pCurrent)
{
	//switch on the type of node
	switch( pCurrent->uiType )
	{
//new codes:
		case CRDT_NODE_DEFAULT:
			HandleNode_Default( pCurrent );
			break;

		default:
			Assert( 0 );
			break;
	}
}


static BOOLEAN DisplayCreditNode(CRDT_NODE* pCurrent);


static void HandleNode_Default(CRDT_NODE* pCurrent)
{
	UINT32	uiCurrentTime =GetJA2Clock();

	//if it is time to update the current node
//	if( ( uiCurrentTime - pCurrent->uiLastTime ) > guiCrdtNodeScrollSpeed )
	{
		//Display the Current Node
		DisplayCreditNode( pCurrent );

		//Save the old position
		pCurrent->sOldPosX = pCurrent->sPosX;
		pCurrent->sOldPosY = pCurrent->sPosY;

		//Move the current node up
		pCurrent->sPosY -= CRDT_SCROLL_PIXEL_AMOUNT;

		//if the node is entirely off the screen
		if( ( pCurrent->sPosY + pCurrent->sHeightOfString ) < CRDT_LINE_NODE_DISAPPEARS_AT )
		{
			//mark the node to be deleted this frame
			pCurrent->fDelete = TRUE;
		}

		//Update the last time to be the current time
//		pCurrent->uiLastTime = uiCurrentTime + ( uiCurrentTime - ( pCurrent->uiLastTime + guiCrdtNodeScrollSpeed ) );

//		pCurrent->uiLastTime = ( uiCurrentTime - ( uiCurrentTime - pCurrent->uiLastTime - guiCrdtNodeScrollSpeed) );

		pCurrent->uiLastTime = GetJA2Clock();
	}
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
		//
		//Restore the background before blitting the text back on
		//

		//if the surface is at the bottom of the screen
		if( pCurrent->sOldPosY + pCurrent->sHeightOfString > CRDT_START_POS_Y )
		{
			INT16	sHeight = 480 - pCurrent->sOldPosY;
			RestoreExternBackgroundRect( pCurrent->sOldPosX, pCurrent->sOldPosY, CRDT_WIDTH_OF_TEXT_AREA, sHeight );
		}
		else if( pCurrent->sOldPosY > CRDT_LINE_NODE_DISAPPEARS_AT )
		{
			RestoreExternBackgroundRect( pCurrent->sOldPosX, pCurrent->sOldPosY, CRDT_WIDTH_OF_TEXT_AREA, pCurrent->sHeightOfString );
		}

		//if the surface is at the top of the screen
		else
		{
			INT16	sHeight = pCurrent->sOldPosY + pCurrent->sHeightOfString;

			RestoreExternBackgroundRect( pCurrent->sOldPosX, CRDT_LINE_NODE_DISAPPEARS_AT, CRDT_WIDTH_OF_TEXT_AREA, sHeight );
		}
	}

	BltVideoSurface(FRAME_BUFFER, pCurrent->uiVideoSurfaceImage, pCurrent->sPosX, pCurrent->sPosY, NULL);

	return( TRUE );
}


static UINT32 GetAndHandleCreditCodeFromCodeString(STR16 pzCode);
static STR16 GetNextCreditCode(STR16 pString, UINT32* pSizeOfCode);
static void HandleCreditFlags(UINT32 uiFlags);


//return false from this function when there are no more items in the text file
static BOOLEAN GetNextCreditFromTextFile(void)
{
	CHAR16	zOriginalString[512];
	CHAR16	zString[512];
	CHAR16	zCodes[512];
	STR16		pzNewCode=NULL;
	UINT32	uiNodeType = 0;
	UINT32	uiStartLoc = 0;
	UINT32	uiFlags=0;


	//Get the current Credit record
	uiStartLoc = CREDITS_LINESIZE * guiCurrentCreditRecord;
	if( !LoadEncryptedDataFromFile( CRDT_NAME_OF_CREDIT_FILE, zOriginalString, uiStartLoc, CREDITS_LINESIZE ) )
	{
		//there are no more credits
		return( FALSE );
	}

	//Increment to the next crdit record
	guiCurrentCreditRecord++;


	//if there are no codes in the string
	if( zOriginalString[0] != CRDT_START_CODE )
	{
		//copy the string
		wcscpy( zString, zOriginalString );
		uiNodeType = CRDT_NODE_DEFAULT;
	}
	else
	{
		UINT32	uiSizeOfCodes = 0;
		UINT32	uiSizeOfSubCode = 0;
		STR16		pzEndCode = NULL;
		UINT32	uiDistanceIntoCodes = 0;

		//Retrive all the codes from the string
		pzEndCode = wcsstr( zOriginalString, CRDT_END_CODE );


		//Make a string for the codes
		wcscpy( zCodes, zOriginalString );

		//end the setence after the codes
		zCodes[ pzEndCode - zOriginalString + 1 ] = '\0';

		//Get the size of the codes
		uiSizeOfCodes = pzEndCode - zOriginalString + 1;

		//
		//check to see if there is a string, or just codes
		//

		//if the string is the same size as the codes
		if( wcslen( zOriginalString ) == uiSizeOfCodes )
		{
			//there is no string, just codes
			uiNodeType = CRDT_NODE_NONE;
		}

		//else there is a string aswell
		else
		{
			//copy the main string
			wcscpy( zString, &zOriginalString[ uiSizeOfCodes ] );

			uiNodeType = CRDT_NODE_DEFAULT;
		}

		//get rid of the start code delimeter
		uiDistanceIntoCodes = 1;

		uiFlags = 0;

		//loop through the string of codes to get all the control codes out
		while( uiDistanceIntoCodes < uiSizeOfCodes )
		{
			//Determine what kind of code it is, and handle it
			uiFlags |= GetAndHandleCreditCodeFromCodeString( &zCodes[ uiDistanceIntoCodes ] );

			//get the next code from the string of codes, returns NULL when done
			pzNewCode = GetNextCreditCode( &zCodes[ uiDistanceIntoCodes ], &uiSizeOfSubCode );

			//if we are done getting the sub codes
			if( pzNewCode == NULL )
			{
				uiDistanceIntoCodes = uiSizeOfCodes;
			}
			else
			{
				//else increment by the size of the code
				uiDistanceIntoCodes += uiSizeOfSubCode;
			}
		}
	}

	if( uiNodeType != CRDT_NODE_NONE )
	{
		//add the node to the list
		AddCreditNode( uiNodeType, uiFlags, zString );
	}

	//if any processing of the flags need to be done
	HandleCreditFlags( uiFlags );

	return( TRUE );
}


//return any flags that need to be set in the node
static UINT32 GetAndHandleCreditCodeFromCodeString(STR16 pzCode)
{
//new codes:


	//if the code is to change the delay between strings
	if( pzCode[0] == CRDT_DELAY_BN_STRINGS_CODE )
	{
		UINT32	uiNewDelay=0;

		//Get the delay from the string
		swscanf( &pzCode[1], L"%d%*s", &uiNewDelay );

//		guiCrdtDelayBetweenNodes = uiNewDelay;
		guiGapBetweenCreditNodes  = uiNewDelay;

		return( CRDT_NODE_NONE );
	}

	//if the code is to change the delay between sections strings
	else if( pzCode[0] == CRDT_DELAY_BN_SECTIONS_CODE )
	{
		UINT32	uiNewDelay=0;

		//Get the delay from the string
		swscanf( &pzCode[1], L"%d%*s", &uiNewDelay );

//		guiCrdtDelayBetweenCreditSection = uiNewDelay;
		guiGapBetweenCreditSections = uiNewDelay;

		return( CRDT_NODE_NONE );
	}


	else if( pzCode[0] == CRDT_SCROLL_SPEED )
	{
		UINT32	uiScrollSpeed=0;

		//Get the delay from the string
		swscanf( &pzCode[1], L"%d%*s", &uiScrollSpeed );

		guiCrdtNodeScrollSpeed = uiScrollSpeed;

		return( CRDT_NODE_NONE );
	}

	else if( pzCode[0] == CRDT_FONT_JUSTIFICATION )
	{
		UINT32	uiJustification=0;

		//Get the delay from the string
		swscanf( &pzCode[1], L"%d%*s", &uiJustification );

		//get the justification
		switch( uiJustification )
		{
			case 0:
				gubCrdtJustification = LEFT_JUSTIFIED;
				break;
			case 1:
				gubCrdtJustification = CENTER_JUSTIFIED;
				break;
			case 2:
				gubCrdtJustification = RIGHT_JUSTIFIED;
				break;
			default:
				Assert( 0 );
		}

		return( CRDT_NODE_NONE );
	}

	else if( pzCode[0] == CRDT_TITLE_FONT_COLOR )
	{
		//Get the new color for the title
		swscanf( &pzCode[1], L"%d%*s", &gubCreditScreenTitleColor );

		return( CRDT_NODE_NONE );
	}

	else if( pzCode[0] == CRDT_ACTIVE_FONT_COLOR )
	{
		//Get the new color for the active text
		swscanf( &pzCode[1], L"%d%*s", &gubCreditScreenActiveColor );

		return( CRDT_NODE_NONE );
	}


	//else its the title code
	else if( pzCode[0] == CRDT_TITLE )
	{
		return( CRDT_FLAG__TITLE );
	}

	//else its the title code
	else if( pzCode[0] == CRDT_START_OF_SECTION )
	{
		return( CRDT_FLAG__START_SECTION );
	}

	//else its the title code
	else if( pzCode[0] == CRDT_END_OF_SECTION )
	{
		return( CRDT_FLAG__END_SECTION );
	}

	//else its an error
	else
	{
		Assert( 0 );
	}

	return( CRDT_NODE_NONE );
}


static UINT32 CountNumberOfCreditNodes(void)
{
	UINT32	uiNumNodes = 0;
	CRDT_NODE	*pTempNode = gCrdtRootNode;

	while( pTempNode )
	{
		uiNumNodes++;

		pTempNode = pTempNode->pNext;
	}

	return( uiNumNodes );
}


static STR16 GetNextCreditCode(STR16 pString, UINT32* pSizeOfCode)
{
	STR16	pzNewCode=NULL;
	UINT32 uiSizeOfCode = 0;

	//get the new subcode out
	pzNewCode = wcsstr( pString, CRDT_SEPARATION_CODE );

	//if there is no separation code, then there must be an end code
	if( pzNewCode == NULL )
	{
		//pzNewCode = wcsstr( pString, CRDT_END_CODE );

		//we are done
		pzNewCode = NULL;
	}
	else
	{
		//get rid of separeation code
		pzNewCode++;


		//calc size of sub string
		uiSizeOfCode = pzNewCode - pString;
	}

	*pSizeOfCode = uiSizeOfCode;
	return( pzNewCode );
}


static void HandleCreditFlags(UINT32 uiFlags)
{
	if( uiFlags & CRDT_FLAG__TITLE )
	{
	}

	if( uiFlags & CRDT_FLAG__START_SECTION )
	{
//		guiCrdtTimeTillReadNextCredit = guiCrdtDelayBetweenNodes;
		guiGapTillReadNextCredit = guiGapBetweenCreditNodes;
	}

	if( uiFlags & CRDT_FLAG__END_SECTION )
	{
//		guiCrdtTimeTillReadNextCredit = guiCrdtDelayBetweenCreditSection;
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
