#include "BuildDefines.h"

#ifdef JA2EDITOR

#include "Button_System.h"
#include "Font.h"
#include "Input.h"
#include "TileDef.h"
#include "VSurface.h"
#include "WorldDat.h"
#include "Random.h"
#include "SysUtil.h"
#include "Font_Control.h"
#include "SelectWin.h"
#include "EditorDefines.h"
#include "Editor_Taskbar_Utils.h"
#include "MemMan.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "WorldDef.h"


// defines for DisplaySpec.ubType
#define DISPLAY_TEXT    1
#define DISPLAY_GRAPHIC 2

#define ONE_COLUMN       0x0001
#define ONE_ROW          0x0002
#define CLEAR_BACKGROUND 0x0004

#define DISPLAY_ALL_OBJECTS 0xFFFF


typedef struct DisplaySpec
{
	UINT8 ubType;
	union
	{
		struct
		{
			HVOBJECT hVObject;
			UINT16   usStart;
			UINT16   usEnd;
			UINT32   uiObjIndx;
		};
		struct
		{
			UINT16* pString;
		};
	};
} DisplaySpec;


typedef struct DisplayList DisplayList;
struct DisplayList
{
	HVOBJECT     hObj;
	UINT16       uiIndex;
	INT16        iX;
	INT16        iY;
	INT16        iWidth;
	INT16        iHeight;
	UINT32       uiObjIndx;
	BOOLEAN      fChosen;
	DisplayList* pNext;
};


extern BOOLEAN gfOverheadMapDirty;


extern BOOLEAN fDontUseRandom;


static BOOLEAN gfRenderSquareArea = FALSE;
static INT16 iStartClickX;
static INT16 iStartClickY;
static INT16 iEndClickX;
static INT16 iEndClickY;


#define CANCEL_ICON		0
#define UP_ICON				1
#define DOWN_ICON			2
#define OK_ICON				3

static INT32 iButtonIcons[4];
static INT32 iSelectWin;
static INT32 iCancelWin;
static INT32 iScrollUp;
static INT32 iScrollDown;
static INT32 iOkWin;

BOOLEAN fAllDone=FALSE;
static BOOLEAN fButtonsPresent = FALSE;

SGPPoint SelWinSpacing, SelWinStartPoint, SelWinEndPoint;

//These definitions help define the start and end of the various wall indices.
//This needs to be maintained if the walls change.
#define WALL_LAST_WALL_OFFSET						 30
#define	WALL_FIRST_AFRAME_OFFSET				 31
#define WALL_LAST_AFRAME_OFFSET					 34
#define	WALL_FIRST_WINDOW_OFFSET				 35
#define WALL_LAST_WINDOW_OFFSET					 46
#define WALL_FIRST_BROKEN_WALL_OFFSET		 47
#define WALL_LAST_BROKEN_WALL_OFFSET		 54
#define WALL_FIRST_APPENDED_WALL_OFFSET	 55
#define WALL_LAST_APPENDED_WALL_OFFSET	 56
#define WALL_FIRST_WEATHERED_WALL_OFFSET 57
#define WALL_LAST_WEATHERED_WALL_OFFSET  64

//I've added these definitions to add readability, and minimize conversion time for changes
//incase there are new values, etc.
#define OSTRUCTS_NUMELEMENTS		( LASTOSTRUCT - FIRSTFULLSTRUCT + 22 )
#define OSTRUCTS1_NUMELEMENTS					5
#define OSTRUCTS2_NUMELEMENTS					12
#define BANKSLIST_NUMELEMENTS					5
#define ROADSLIST_NUMELEMENTS					1
#define DEBRISLIST_NUMELEMENTS				( LASTDEBRIS - DEBRISROCKS + 2 + 1 ) //+1 for ANOTHERDEBRIS

#define SINGLEWALL_NUMELEMENTS				( (LASTWALL-FIRSTWALL+1) * 2 )
#define SINGLEDOOR_NUMELEMENTS				(( LASTDOOR - FIRSTDOOR + 1 ) * 5)
#define SINGLEWINDOW_NUMELEMENTS			( LASTWALL - FIRSTWALL + 1 )
#define SINGLEROOF_NUMELEMENTS				( (LASTROOF-FIRSTROOF+1) + (LASTSLANTROOF-FIRSTSLANTROOF+1) + \
																				(LASTWALL-FIRSTWALL+1) + (SECONDONROOF-FIRSTONROOF+1) )
#define SINGLENEWROOF_NUMELEMENTS			(LASTROOF-FIRSTROOF+1)
#define SINGLEBROKENWALL_NUMELEMENTS	( (LASTDECORATIONS-FIRSTDECORATIONS+1) + (LASTWALL-FIRSTWALL+1)*2 )
#define SINGLEDECOR_NUMELEMENTS				( LASTISTRUCT - FIRSTISTRUCT + 1 )
#define SINGLEDECAL_NUMELEMENTS				( LASTWALLDECAL - FIRSTWALLDECAL + EIGTHWALLDECAL - FIFTHWALLDECAL + 3 )
#define SINGLEFLOOR_NUMELEMENTS				( LASTFLOOR - FIRSTFLOOR + 1 )
#define SINGLETOILET_NUMELEMENTS			( EIGHTISTRUCT - FIFTHISTRUCT + 1 )
//#define ROOM_NUMELEMENTS							( (LASTWALL-FIRSTWALL+1) + (LASTFLOOR-FIRSTFLOOR+1) + \
//																				(LASTROOF-FIRSTROOF+1) + (LASTSLANTROOF-FIRSTSLANTROOF+1) )
#define ROOM_NUMELEMENTS							( (LASTWALL-FIRSTWALL+1) + (LASTFLOOR-FIRSTFLOOR+1) + \
																				(LASTROOF-FIRSTROOF+1) + (2 ))


//This is a special case for trees which may have varying numbers.  There was a problem
//in which we loaded a new tileset which had one less tree in it.  When we called BuildSelectionWindow(),
//it would crash because it thought there was an extra tree which was now invalid.
static UINT16 gusNumOStructs = 0;

// List of objects to display in the selection window
static DisplaySpec OStructs[OSTRUCTS_NUMELEMENTS];
static DisplaySpec OStructs1[OSTRUCTS1_NUMELEMENTS];
static DisplaySpec OStructs2[OSTRUCTS2_NUMELEMENTS];
static DisplaySpec BanksList[BANKSLIST_NUMELEMENTS];
static DisplaySpec RoadsList[ROADSLIST_NUMELEMENTS];
static DisplaySpec DebrisList[DEBRISLIST_NUMELEMENTS];
static DisplaySpec SingleWall[SINGLEWALL_NUMELEMENTS];
static DisplaySpec SingleDoor[SINGLEDOOR_NUMELEMENTS];
static DisplaySpec SingleWindow[SINGLEWINDOW_NUMELEMENTS];
static DisplaySpec SingleRoof[SINGLEROOF_NUMELEMENTS];
static DisplaySpec SingleNewRoof[SINGLENEWROOF_NUMELEMENTS];
static DisplaySpec SingleBrokenWall[SINGLEBROKENWALL_NUMELEMENTS];
static DisplaySpec SingleDecor[SINGLEDECOR_NUMELEMENTS];
static DisplaySpec SingleDecal[SINGLEDECAL_NUMELEMENTS];
static DisplaySpec SingleFloor[SINGLEFLOOR_NUMELEMENTS];
static DisplaySpec SingleToilet[SINGLETOILET_NUMELEMENTS];
static DisplaySpec Room[ROOM_NUMELEMENTS];

//These are all of the different selection lists.  Changing the max_selections will
//change the number of selections values you can have at a time.  This is Bret's gay code,
//though I've cleaned it up a lot.
Selections	SelOStructs[MAX_SELECTIONS]				=		{ {FIRSTFULLSTRUCT, 0, 1} };	// Default selections
Selections	SelOStructs1[MAX_SELECTIONS]				=		{ {FOURTHOSTRUCT, 0, 1} };	// Default selections
Selections	SelOStructs2[MAX_SELECTIONS]				=		{ {THIRDOSTRUCT, 0, 1} };	// Default selections
Selections	SelBanks[MAX_SELECTIONS]					=		{ {FIRSTCLIFF, 0, 1} };
Selections	SelRoads[MAX_SELECTIONS]					=		{ {FIRSTROAD, 0, 1} };
Selections	SelDebris[MAX_SELECTIONS]					=		{ {DEBRISROCKS, 0, 1} };
Selections	SelSingleWall[MAX_SELECTIONS]			=		{ {FIRSTWALL, 0, 1} };
Selections	SelSingleDoor[MAX_SELECTIONS]			=		{ {FIRSTDOOR, 0, 1} };
Selections	SelSingleWindow[MAX_SELECTIONS]		=		{ {FIRSTWALL, 44, 1} };
Selections  SelSingleRoof[MAX_SELECTIONS] =				{ {FIRSTROOF, 0, 1} };
Selections  SelSingleNewRoof[MAX_SELECTIONS] =		{ {FIRSTROOF, 0, 1} };
Selections  SelSingleBrokenWall[MAX_SELECTIONS] = { {FIRSTDECORATIONS, 0, 1} };
Selections	SelSingleDecor[MAX_SELECTIONS]=				{ {FIRSTISTRUCT, 0, 1} };
Selections	SelSingleDecal[MAX_SELECTIONS]	=		{ {FIRSTWALLDECAL, 0, 1} };
Selections  SelSingleFloor[MAX_SELECTIONS] =			{ {FIRSTFLOOR, 0, 1} };
Selections  SelSingleToilet[MAX_SELECTIONS] =			{ {FIFTHISTRUCT, 0, 1} };
Selections	SelRoom[MAX_SELECTIONS]			=					{ {FIRSTWALL, 0, 1} };

// Number of objects currently in the selection list
INT32				iNumOStructsSelected = 1;
INT32				iNumOStructs1Selected = 1;
INT32				iNumOStructs2Selected = 1;
INT32				iNumBanksSelected = 1;
INT32				iNumRoadsSelected = 1;
INT32				iNumDebrisSelected = 1;

INT32				iNumWallsSelected = 1;
INT32				iNumDoorsSelected = 1;
INT32				iNumWindowsSelected = 1;
INT32				iNumDecorSelected = 1;
INT32				iNumDecalsSelected = 1;
INT32				iNumBrokenWallsSelected = 1;
INT32				iNumFloorsSelected = 1;
INT32       iNumToiletsSelected = 1;
INT32				iNumRoofsSelected = 1;
INT32				iNumNewRoofsSelected = 1;
INT32				iNumRoomsSelected = 1;

// Holds the previous selection list when a selection window is up. Used for canceling the selection window
Selections	OldSelList[MAX_SELECTIONS];
INT32				iOldNumSelList;

// Global pointers for selection list
Selections  *pSelList;
INT32				*pNumSelList;

// Global used to indicate which selection to use (changes with the PGUP/PGDWN keys in editor)
INT32				iCurBank = 0;

static DisplayList* pDispList;
static INT16 iTopWinCutOff;
static INT16 iBotWinCutOff;

UINT16 SelWinFillColor = 0x0000;					// Black
UINT16 SelWinHilightFillColor = 0x000d;		// a kind of medium dark blue


static BOOLEAN BuildDisplayWindow(DisplaySpec* pDisplaySpecs, UINT16 usNumSpecs, DisplayList** pDisplayList, SGPPoint* pUpperLeft, SGPPoint* pBottomRight, SGPPoint* pSpacing, UINT16 fFlags);
static void CnclClkCallback(GUI_BUTTON* button, INT32 reason);
static void DwnClkCallback(GUI_BUTTON* button, INT32 reason);
static void OkClkCallback(GUI_BUTTON* button, INT32 reason);
static void SelWinClkCallback(GUI_BUTTON* button, INT32 reason);
static void UpClkCallback(GUI_BUTTON* button, INT32 reason);


static INT32 MakeButton(UINT idx, const char* gfx, INT16 y, INT16 h, GUI_CALLBACK click, const wchar_t* help)
{
	INT32 img = LoadGenericButtonIcon(gfx);
	iButtonIcons[idx] = img;
	INT32 btn = CreateIconButton(img, 0, 600, y, 40, h, MSYS_PRIORITY_HIGH, click);
	SetButtonFastHelpText(btn, help);
	return btn;
}


//----------------------------------------------------------------------------------------------
//	CreateJA2SelectionWindow
//
//	Creates a selection window of the given type.
//
void CreateJA2SelectionWindow( INT16 sWhat )
{
	DisplaySpec *pDSpec;
	UINT16 usNSpecs;

	fAllDone = FALSE;

	DisableEditorTaskbar( );

	iSelectWin = CreateHotSpot(0, 0, 600, 360, MSYS_PRIORITY_HIGH, SelWinClkCallback);

	iCancelWin  = MakeButton(CANCEL_ICON, "EDITOR/bigX.sti",         40,  40, CnclClkCallback, L"Cancel selections");
	iOkWin      = MakeButton(OK_ICON,     "EDITOR/checkmark.sti",     0,  40, OkClkCallback,   L"Accept selections");
	iScrollUp   = MakeButton(UP_ICON,     "EDITOR/lgUpArrow.sti",    80, 160, UpClkCallback,   L"Scroll window up");
	iScrollDown = MakeButton(DOWN_ICON,   "EDITOR/lgDownArrow.sti", 240, 160, DwnClkCallback,  L"Scroll window down");

	fButtonsPresent = TRUE;

	SelWinSpacing.iX = 2;
	SelWinSpacing.iY = 2;

	SelWinStartPoint.iX = 1;
	SelWinStartPoint.iY = 15;

	iTopWinCutOff = 15;

	SelWinEndPoint.iX = 599;
	SelWinEndPoint.iY = 359;

	iBotWinCutOff = 359;

	switch( sWhat )
	{
		case SELWIN_OSTRUCTS:
			pDSpec = OStructs;
			usNSpecs = gusNumOStructs;//OSTRUCTS_NUMELEMENTS;
			pSelList = SelOStructs;
			pNumSelList = &iNumOStructsSelected;
			break;

		case SELWIN_OSTRUCTS1:
			pDSpec = OStructs1;
			usNSpecs = OSTRUCTS1_NUMELEMENTS;
			pSelList = SelOStructs1;
			pNumSelList = &iNumOStructs1Selected;
			break;

		case SELWIN_OSTRUCTS2:
			pDSpec = OStructs2;
			usNSpecs = OSTRUCTS2_NUMELEMENTS;
			pSelList = SelOStructs2;
			pNumSelList = &iNumOStructs2Selected;
			break;

		case SELWIN_BANKS:
			pDSpec = BanksList;
			usNSpecs = BANKSLIST_NUMELEMENTS;
			pSelList = SelBanks;
			pNumSelList = &iNumBanksSelected;
			break;

		case SELWIN_ROADS:
			pDSpec = RoadsList;
			usNSpecs = ROADSLIST_NUMELEMENTS;
			pSelList = SelRoads;
			pNumSelList = &iNumRoadsSelected;
			break;

		case SELWIN_DEBRIS:
			pDSpec = DebrisList;
			usNSpecs = DEBRISLIST_NUMELEMENTS;
			pSelList = SelDebris;
			pNumSelList = &iNumDebrisSelected;
			break;

		case SELWIN_SINGLEWALL:
			pDSpec = SingleWall;
			usNSpecs = SINGLEWALL_NUMELEMENTS;
			pSelList = SelSingleWall;
			pNumSelList = &iNumWallsSelected;
			break;
		case SELWIN_SINGLEDOOR:
			pDSpec = SingleDoor;
			usNSpecs = SINGLEDOOR_NUMELEMENTS;
			pSelList = SelSingleDoor;
			pNumSelList = &iNumDoorsSelected;
			break;
		case SELWIN_SINGLEWINDOW:
			pDSpec = SingleWindow;
			usNSpecs = SINGLEWINDOW_NUMELEMENTS;
			pSelList = SelSingleWindow;
			pNumSelList = &iNumWindowsSelected;
			break;
		case SELWIN_SINGLEROOF:
			pDSpec = SingleRoof;
			usNSpecs = SINGLEROOF_NUMELEMENTS;
			pSelList = SelSingleRoof;
			pNumSelList = &iNumRoofsSelected;
			break;
		case SELWIN_SINGLENEWROOF:
			pDSpec = SingleNewRoof;
			usNSpecs = SINGLENEWROOF_NUMELEMENTS;
			pSelList = SelSingleNewRoof;
			pNumSelList = &iNumNewRoofsSelected;
			break;
		case SELWIN_SINGLEBROKENWALL:
			pDSpec = SingleBrokenWall;
			usNSpecs = SINGLEBROKENWALL_NUMELEMENTS;
			pSelList = SelSingleBrokenWall;
			pNumSelList = &iNumBrokenWallsSelected;
			break;
		case SELWIN_SINGLEDECOR:
			pDSpec = SingleDecor;
			usNSpecs = SINGLEDECOR_NUMELEMENTS;
			pSelList = SelSingleDecor;
			pNumSelList = &iNumDecorSelected;
			break;
		case SELWIN_SINGLEDECAL:
			pDSpec = SingleDecal;
			usNSpecs = SINGLEDECAL_NUMELEMENTS;
			pSelList = SelSingleDecal;
			pNumSelList = &iNumDecalsSelected;
			break;
		case SELWIN_SINGLEFLOOR:
			pDSpec = SingleFloor;
			usNSpecs = SINGLEFLOOR_NUMELEMENTS;
			pSelList = SelSingleFloor;
			pNumSelList = &iNumFloorsSelected;
			break;
		case SELWIN_SINGLETOILET:
			pDSpec = SingleToilet;
			usNSpecs = SINGLETOILET_NUMELEMENTS;
			pSelList = SelSingleToilet;
			pNumSelList = &iNumToiletsSelected;
			break;
		case SELWIN_ROOM:
			pDSpec = Room;
			usNSpecs = ROOM_NUMELEMENTS;
			pSelList = SelRoom;
			pNumSelList = &iNumRoomsSelected;
			break;
	}

	BuildDisplayWindow( pDSpec, usNSpecs, &pDispList, &SelWinStartPoint, &SelWinEndPoint,
										&SelWinSpacing, CLEAR_BACKGROUND);
}


//The selection window method is initialized here.  This is where all the graphics for all
//the categories are organized and loaded.  If you wish to move things around, then this is
//where the initialization part is done.  I have also changed this from previously being loaded
//every single time you go into a selection window which was redundant and CPU consuming.
void InitJA2SelectionWindow( void )
{
	INT32 iCount;
	INT32 iCount2;
	INT32 iCount3;

	UINT16			usETRLEObjects;
	HVOBJECT		hVObject;


	pDispList = NULL;

	// Init the display spec lists for the types of selection windows

	// Trees & bushes (The tree button in the "terrain" toolbar)
	for ( iCount3 = 0, iCount = 0; iCount < (LASTOSTRUCT - FIRSTFULLSTRUCT + 1); iCount++ )
	{
		hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTFULLSTRUCT + iCount]].hTileSurface;
		usETRLEObjects = hVObject->usNumberOfObjects;


		for ( iCount2 = 0; iCount2 < usETRLEObjects; iCount2 += 3, iCount3++)
		{
			OStructs[ iCount3 ].ubType = DISPLAY_GRAPHIC;
			OStructs[ iCount3 ].hVObject = hVObject;
			OStructs[ iCount3 ].usStart = (UINT16)iCount2;
			OStructs[ iCount3 ].usEnd = (UINT16)iCount2;
			OStructs[ iCount3 ].uiObjIndx = (FIRSTFULLSTRUCT + iCount);
		}
	}

	OStructs[ iCount3 ].ubType = DISPLAY_GRAPHIC;
	OStructs[ iCount3 ].hVObject = gTileDatabase[gTileTypeStartIndex[SIXTHOSTRUCT]].hTileSurface;
	OStructs[ iCount3 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs[ iCount3 ].usEnd = 0;
	OStructs[ iCount3 ].uiObjIndx = SIXTHOSTRUCT;

	gusNumOStructs = (UINT16)iCount3 + 1;

	// Rocks & barrels! (the "1" button in the "terrain" toolbar)
	OStructs1[ 0 ].ubType = DISPLAY_GRAPHIC;
	OStructs1[ 0 ].hVObject = gTileDatabase[gTileTypeStartIndex[FOURTHOSTRUCT]].hTileSurface;
	OStructs1[ 0 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs1[ 0 ].usEnd = 0;
	OStructs1[ 0 ].uiObjIndx = FOURTHOSTRUCT;

	for ( iCount = 0; iCount < (THIRDOSTRUCT - FIRSTOSTRUCT); iCount++ )
	{
		OStructs1[ iCount + 1 ].ubType = DISPLAY_GRAPHIC;
		OStructs1[ iCount + 1 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTOSTRUCT + iCount]].hTileSurface;
		OStructs1[ iCount + 1 ].usStart = DISPLAY_ALL_OBJECTS;
		OStructs1[ iCount + 1 ].usEnd = 0;
		OStructs1[ iCount + 1 ].uiObjIndx = FIRSTOSTRUCT + iCount;
	}


	// Other junk! (the "2" button in the "terrain" toolbar)
	OStructs2[ 0 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 0 ].hVObject = gTileDatabase[gTileTypeStartIndex[THIRDOSTRUCT]].hTileSurface;
	OStructs2[ 0 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 0 ].usEnd = 0;
	OStructs2[ 0 ].uiObjIndx = THIRDOSTRUCT;

	OStructs2[ 1 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 1 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIFTHOSTRUCT]].hTileSurface;
	OStructs2[ 1 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 1 ].usEnd = 0;
	OStructs2[ 1 ].uiObjIndx = FIFTHOSTRUCT;

	OStructs2[ 2 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 2 ].hVObject = gTileDatabase[gTileTypeStartIndex[SEVENTHOSTRUCT]].hTileSurface;
	OStructs2[ 2 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 2 ].usEnd = 0;
	OStructs2[ 2 ].uiObjIndx = SEVENTHOSTRUCT;

	OStructs2[ 3 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 3 ].hVObject = gTileDatabase[gTileTypeStartIndex[EIGHTOSTRUCT]].hTileSurface;
	OStructs2[ 3 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 3 ].usEnd = 0;
	OStructs2[ 3 ].uiObjIndx = EIGHTOSTRUCT;

	OStructs2[ 4 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 4 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTVEHICLE]].hTileSurface;
	OStructs2[ 4 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 4 ].usEnd = 0;
	OStructs2[ 4 ].uiObjIndx = FIRSTVEHICLE;

	OStructs2[ 5 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 5 ].hVObject = gTileDatabase[gTileTypeStartIndex[SECONDVEHICLE]].hTileSurface;
	OStructs2[ 5 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 5 ].usEnd = 0;
	OStructs2[ 5 ].uiObjIndx = SECONDVEHICLE;

	OStructs2[ 6 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 6 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDEBRISSTRUCT]].hTileSurface;
	OStructs2[ 6 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 6 ].usEnd = 0;
	OStructs2[ 6 ].uiObjIndx = FIRSTDEBRISSTRUCT;

	OStructs2[ 7 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 7 ].hVObject = gTileDatabase[gTileTypeStartIndex[SECONDDEBRISSTRUCT]].hTileSurface;
	OStructs2[ 7 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 7 ].usEnd = 0;
	OStructs2[ 7 ].uiObjIndx = SECONDDEBRISSTRUCT;

	OStructs2[ 8 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 8 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTLARGEEXPDEBRIS]].hTileSurface;
	OStructs2[ 8 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 8 ].usEnd = 0;
	OStructs2[ 8 ].uiObjIndx = FIRSTLARGEEXPDEBRIS;

	OStructs2[ 9 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 9 ].hVObject = gTileDatabase[gTileTypeStartIndex[SECONDLARGEEXPDEBRIS]].hTileSurface;
	OStructs2[ 9 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 9 ].usEnd = 0;
	OStructs2[ 9 ].uiObjIndx = SECONDLARGEEXPDEBRIS;

	OStructs2[ 10 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 10 ].hVObject = gTileDatabase[gTileTypeStartIndex[NINTHOSTRUCT]].hTileSurface;
	OStructs2[ 10 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 10 ].usEnd = 0;
	OStructs2[ 10 ].uiObjIndx = NINTHOSTRUCT;

	OStructs2[ 11 ].ubType = DISPLAY_GRAPHIC;
	OStructs2[ 11 ].hVObject = gTileDatabase[gTileTypeStartIndex[TENTHOSTRUCT]].hTileSurface;
	OStructs2[ 11 ].usStart = DISPLAY_ALL_OBJECTS;
	OStructs2[ 11 ].usEnd = 0;
	OStructs2[ 11 ].uiObjIndx = TENTHOSTRUCT;

	// River banks and cliffs (the "river" button on the "terrain" toolbar)
	BanksList[0].ubType = DISPLAY_GRAPHIC;
	BanksList[0].hVObject = gTileDatabase[gTileTypeStartIndex[ANIOSTRUCT]].hTileSurface;
	BanksList[0].usStart = DISPLAY_ALL_OBJECTS;
	BanksList[0].usEnd = 0;
	BanksList[0].uiObjIndx = ANIOSTRUCT;

	BanksList[1].ubType = DISPLAY_GRAPHIC;
	BanksList[1].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTCLIFF]].hTileSurface;
	BanksList[1].usStart = DISPLAY_ALL_OBJECTS;
	BanksList[1].usEnd = 0;
	BanksList[1].uiObjIndx = FIRSTCLIFF;

	BanksList[2].ubType = DISPLAY_GRAPHIC;
	BanksList[2].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTCLIFFHANG]].hTileSurface;
	BanksList[2].usStart = DISPLAY_ALL_OBJECTS;
	BanksList[2].usEnd = 0;
	BanksList[2].uiObjIndx = FIRSTCLIFFHANG;

	BanksList[3].ubType = DISPLAY_GRAPHIC;
	BanksList[3].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTROAD]].hTileSurface;
	BanksList[3].usStart = DISPLAY_ALL_OBJECTS;
	BanksList[3].usEnd = 0;
	BanksList[3].uiObjIndx = FIRSTROAD;

	BanksList[4].ubType = DISPLAY_GRAPHIC;
	BanksList[4].hVObject = gTileDatabase[gTileTypeStartIndex[FENCESTRUCT]].hTileSurface;
	BanksList[4].usStart = DISPLAY_ALL_OBJECTS;
	BanksList[4].usEnd = 0;
	BanksList[4].uiObjIndx = FENCESTRUCT;

	RoadsList[0].ubType = DISPLAY_GRAPHIC;
	RoadsList[0].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTROAD]].hTileSurface;
	RoadsList[0].usStart = DISPLAY_ALL_OBJECTS;
	RoadsList[0].usEnd = 0;
	RoadsList[0].uiObjIndx = FIRSTROAD;

	// Debris (the "bent can" button on the "terrain", and "buildings" toolbars)
	for ( iCount = 0; iCount < (LASTDEBRIS - DEBRISROCKS + 1); iCount++ )
	{
		DebrisList[ iCount ].ubType = DISPLAY_GRAPHIC;
		DebrisList[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[DEBRISROCKS + iCount]].hTileSurface;
		DebrisList[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		DebrisList[ iCount ].usEnd = 0;
		DebrisList[ iCount ].uiObjIndx = DEBRISROCKS + iCount;
	}
	// Add one more for new misc debris
	DebrisList[ iCount ].ubType = DISPLAY_GRAPHIC;
	DebrisList[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[DEBRIS2MISC ] ].hTileSurface;
	DebrisList[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
	DebrisList[ iCount ].usEnd = 0;
	DebrisList[ iCount ].uiObjIndx = DEBRIS2MISC;
	//Add yet another one...
	iCount++;
	DebrisList[ iCount ].ubType = DISPLAY_GRAPHIC;
	DebrisList[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[ANOTHERDEBRIS ] ].hTileSurface;
	DebrisList[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
	DebrisList[ iCount ].usEnd = 0;
	DebrisList[ iCount ].uiObjIndx = ANOTHERDEBRIS;

	// Rooms
	for ( iCount = 0; iCount < (LASTWALL - FIRSTWALL + 1); iCount++ )
	{
		Room[ iCount ].ubType = DISPLAY_GRAPHIC;
		Room[iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount]].hTileSurface;
		Room[ iCount ].usStart = 0;
		Room[ iCount ].usEnd = 0;
		Room[ iCount ].uiObjIndx = FIRSTWALL + iCount;
	}
	for ( iCount2 = 0; iCount2 < (LASTFLOOR - FIRSTFLOOR + 1); iCount2++, iCount++ )
	{
		Room[ iCount ].ubType = DISPLAY_GRAPHIC;
		Room[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTFLOOR + iCount2]].hTileSurface;
		Room[ iCount ].usStart = 0;
		Room[ iCount ].usEnd = 0;
		Room[ iCount ].uiObjIndx = FIRSTFLOOR + iCount2;
	}
	for ( iCount2 = 0; iCount2 < (LASTROOF - FIRSTROOF + 1); iCount2++, iCount++ )
	{
		Room[ iCount ].ubType = DISPLAY_GRAPHIC;
		Room[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTROOF + iCount2]].hTileSurface;
		Room[ iCount ].usStart = 0;
		Room[ iCount ].usEnd = 0;
		Room[ iCount ].uiObjIndx = FIRSTROOF + iCount2;
	}
	for ( iCount2 = 0; iCount2 < 2/*(LASTSLANTROOF - FIRSTSLANTROOF + 1)*/; iCount2++, iCount++ )
	{
		Room[ iCount ].ubType = DISPLAY_GRAPHIC;
		Room[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTSLANTROOF + iCount2]].hTileSurface;
		Room[ iCount ].usStart = 0;
		Room[ iCount ].usEnd = 0;
		Room[ iCount ].uiObjIndx = FIRSTSLANTROOF + iCount2;
	}

	//Walls
	for ( iCount = 0, iCount2 = 0; iCount < (LASTWALL - FIRSTWALL + 1); iCount++, iCount2+=2 )
	{
		SingleWall[ iCount2 ].ubType = DISPLAY_GRAPHIC;
		SingleWall[ iCount2 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount]].hTileSurface;
		SingleWall[ iCount2 ].usStart = 0;
		SingleWall[ iCount2 ].usEnd = WALL_LAST_WALL_OFFSET;
		SingleWall[ iCount2 ].uiObjIndx = FIRSTWALL + iCount;
		//New appended walls
		SingleWall[ iCount2+1 ].ubType = DISPLAY_GRAPHIC;
		SingleWall[ iCount2+1 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount]].hTileSurface;
		SingleWall[ iCount2+1 ].usStart = WALL_FIRST_APPENDED_WALL_OFFSET;
		SingleWall[ iCount2+1 ].usEnd = WALL_LAST_APPENDED_WALL_OFFSET;
		SingleWall[ iCount2+1 ].uiObjIndx = FIRSTWALL + iCount;
	}

	//Doors
	for ( iCount = 0, iCount2 = 0; iCount < (LASTDOOR - FIRSTDOOR + 1); iCount++, iCount2+=5)
	{
		//closed
		SingleDoor[ iCount2 ].ubType = DISPLAY_GRAPHIC;
		SingleDoor[ iCount2 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDOOR + iCount]].hTileSurface;
		SingleDoor[ iCount2 ].usStart = 0;
		SingleDoor[ iCount2 ].usEnd = 0;
		SingleDoor[ iCount2 ].uiObjIndx = FIRSTDOOR + iCount;
		//open, closed
		SingleDoor[ iCount2+1 ].ubType = DISPLAY_GRAPHIC;
		SingleDoor[ iCount2+1 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDOOR + iCount]].hTileSurface;
		SingleDoor[ iCount2+1 ].usStart = 4;
		SingleDoor[ iCount2+1 ].usEnd = 5;
		SingleDoor[ iCount2+1 ].uiObjIndx = FIRSTDOOR + iCount;
		//open, closed
		SingleDoor[ iCount2+2 ].ubType = DISPLAY_GRAPHIC;
		SingleDoor[ iCount2+2 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDOOR + iCount]].hTileSurface;
		SingleDoor[ iCount2+2 ].usStart = 9;
		SingleDoor[ iCount2+2 ].usEnd = 10;
		SingleDoor[ iCount2+2 ].uiObjIndx = FIRSTDOOR + iCount;
		//open, closed
		SingleDoor[ iCount2+3 ].ubType = DISPLAY_GRAPHIC;
		SingleDoor[ iCount2+3 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDOOR + iCount]].hTileSurface;
		SingleDoor[ iCount2+3 ].usStart = 14;
		SingleDoor[ iCount2+3 ].usEnd = 15;
		SingleDoor[ iCount2+3 ].uiObjIndx = FIRSTDOOR + iCount;
		//open
		SingleDoor[ iCount2+4 ].ubType = DISPLAY_GRAPHIC;
		SingleDoor[ iCount2+4 ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDOOR + iCount]].hTileSurface;
		SingleDoor[ iCount2+4 ].usStart = 19;
		SingleDoor[ iCount2+4 ].usEnd = 19;
		SingleDoor[ iCount2+4 ].uiObjIndx = FIRSTDOOR + iCount;
	}
	//Windows
	for ( iCount = 0; iCount < (LASTWALL - FIRSTWALL + 1); iCount++ )
	{
		SingleWindow[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleWindow[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount]].hTileSurface;
		SingleWindow[ iCount ].usStart = WALL_FIRST_WINDOW_OFFSET;
		SingleWindow[ iCount ].usEnd = WALL_LAST_WINDOW_OFFSET;
		SingleWindow[ iCount ].uiObjIndx = FIRSTWALL + iCount;
	}
	//Roofs and slant roofs
	for ( iCount = 0; iCount < (LASTROOF - FIRSTROOF + 1); iCount++ )
	{	//Flat roofs
		SingleRoof[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleRoof[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTROOF + iCount]].hTileSurface;
		SingleRoof[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleRoof[ iCount ].usEnd = 0;
		SingleRoof[ iCount ].uiObjIndx = FIRSTROOF + iCount;
	}
	for ( iCount2 = 0; iCount2 < (LASTSLANTROOF - FIRSTSLANTROOF + 1); iCount2++, iCount++ )
	{	//Slanted roofs
		SingleRoof[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleRoof[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTSLANTROOF + iCount2]].hTileSurface;
		SingleRoof[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleRoof[ iCount ].usEnd = 0;
		SingleRoof[ iCount ].uiObjIndx = FIRSTSLANTROOF + iCount2;
	}
	for( iCount2 = 0; iCount2 < (LASTWALL - FIRSTWALL + 1); iCount2++, iCount++ )
	{	//A-Frames
		SingleRoof[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleRoof[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount2]].hTileSurface;
		SingleRoof[ iCount ].usStart = WALL_FIRST_AFRAME_OFFSET;
		SingleRoof[ iCount ].usEnd = WALL_LAST_AFRAME_OFFSET;
		SingleRoof[ iCount ].uiObjIndx = FIRSTWALL + iCount2;
	}
	for( iCount2 = 0; iCount2 < (SECONDONROOF - FIRSTONROOF + 1); iCount2++, iCount++ )
	{	//On roofs
		SingleRoof[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleRoof[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTONROOF + iCount2]].hTileSurface;
		SingleRoof[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleRoof[ iCount ].usEnd = 0;
		SingleRoof[ iCount ].uiObjIndx = FIRSTONROOF + iCount2;
	}

	//New replacement roofs
	for ( iCount = 0; iCount < (LASTROOF - FIRSTROOF + 1); iCount++ )
	{	//Flat roofs
		SingleNewRoof[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleNewRoof[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTROOF + iCount]].hTileSurface;
		SingleNewRoof[ iCount ].usStart = 9;
		SingleNewRoof[ iCount ].usEnd = 9;
		SingleNewRoof[ iCount ].uiObjIndx = FIRSTROOF + iCount;
	}

	//Broken walls
	for ( iCount = 0; iCount < (LASTDECORATIONS - FIRSTDECORATIONS + 1); iCount++ )
	{	//Old obsolete wall decals, but should be replaced with multitiled decals such as banners, etc.
		SingleBrokenWall[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleBrokenWall[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTDECORATIONS + iCount]].hTileSurface;
		SingleBrokenWall[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleBrokenWall[ iCount ].usEnd = 0;
		SingleBrokenWall[ iCount ].uiObjIndx = FIRSTDECORATIONS + iCount;
	}
	for( iCount2 = 0; iCount2 < (LASTWALL - FIRSTWALL + 1); iCount2++, iCount++ )
	{	//Broken walls
		SingleBrokenWall[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleBrokenWall[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount2]].hTileSurface;
		SingleBrokenWall[ iCount ].usStart = WALL_FIRST_BROKEN_WALL_OFFSET;
		SingleBrokenWall[ iCount ].usEnd = WALL_LAST_BROKEN_WALL_OFFSET;
		SingleBrokenWall[ iCount ].uiObjIndx = FIRSTWALL + iCount2;
	}
	for( iCount2 = 0; iCount2 < (LASTWALL - FIRSTWALL + 1); iCount2++, iCount++ )
	{	//Cracked and smudged walls
		SingleBrokenWall[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleBrokenWall[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALL + iCount2]].hTileSurface;
		SingleBrokenWall[ iCount ].usStart = WALL_FIRST_WEATHERED_WALL_OFFSET;
		SingleBrokenWall[ iCount ].usEnd = WALL_LAST_WEATHERED_WALL_OFFSET;
		SingleBrokenWall[ iCount ].uiObjIndx = FIRSTWALL + iCount2;
	}

	// Decorations
	for ( iCount = 0; iCount < (LASTISTRUCT - FIRSTISTRUCT + 1); iCount++ )
	{
		SingleDecor[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleDecor[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTISTRUCT + iCount]].hTileSurface;
		SingleDecor[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleDecor[ iCount ].usEnd = 0;
		SingleDecor[ iCount ].uiObjIndx = FIRSTISTRUCT + iCount;
	}

	// Wall decals
	for ( iCount = 0; iCount < (LASTWALLDECAL - FIRSTWALLDECAL + 1); iCount++ )
	{
		SingleDecal[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleDecal[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTWALLDECAL + iCount]].hTileSurface;
		SingleDecal[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleDecal[ iCount ].usEnd = 0;
		SingleDecal[ iCount ].uiObjIndx = FIRSTWALLDECAL + iCount;
	}
	for ( iCount2 = 0; iCount2 < (EIGTHWALLDECAL - FIFTHWALLDECAL + 1); iCount++, iCount2++ )
	{
		SingleDecal[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleDecal[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIFTHWALLDECAL + iCount2]].hTileSurface;
		SingleDecal[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleDecal[ iCount ].usEnd = 0;
		SingleDecal[ iCount ].uiObjIndx = FIFTHWALLDECAL + iCount2;
	}
	SingleDecal[ iCount ].ubType = DISPLAY_GRAPHIC;
	SingleDecal[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTSWITCHES]].hTileSurface;
	SingleDecal[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
	SingleDecal[ iCount ].usEnd = 0;
	SingleDecal[ iCount ].uiObjIndx = FIRSTSWITCHES;

	//Floors
	for ( iCount = 0; iCount < (LASTFLOOR - FIRSTFLOOR + 1); iCount++ )
	{
		SingleFloor[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleFloor[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIRSTFLOOR + iCount]].hTileSurface;
		SingleFloor[ iCount ].usStart = 0;
		SingleFloor[ iCount ].usEnd = 7;
		SingleFloor[ iCount ].uiObjIndx = FIRSTFLOOR + iCount;
	}

	//Toilets
	for ( iCount = 0; iCount < (EIGHTISTRUCT - FIFTHISTRUCT + 1); iCount++ )
	{
		SingleToilet[ iCount ].ubType = DISPLAY_GRAPHIC;
		SingleToilet[ iCount ].hVObject = gTileDatabase[gTileTypeStartIndex[FIFTHISTRUCT + iCount]].hTileSurface;
		SingleToilet[ iCount ].usStart = DISPLAY_ALL_OBJECTS;
		SingleToilet[ iCount ].usEnd = 0;
		SingleToilet[ iCount ].uiObjIndx = FIFTHISTRUCT + iCount;
	}

}


static DisplayList* TrashList(DisplayList* pNode);


//----------------------------------------------------------------------------------------------
//	ShutdownJA2SelectionWindow
//
//	Unloads selection window button images and makes sure any display list that may remain in memory
//	is removed.
//
void ShutdownJA2SelectionWindow( void )
{
	INT16 x;

	for (x = 0; x < 4; x++)
		UnloadGenericButtonIcon( (INT16)iButtonIcons[x] );

	if (pDispList != NULL)
	{
		pDispList = TrashList(pDispList);
	}
	gfRenderWorld = TRUE;
}


//----------------------------------------------------------------------------------------------
//	RemoveJA2SelectionWindow
//
//	Removes the selection window from the screen.
//
void RemoveJA2SelectionWindow( void )
{
	RemoveButton(iSelectWin);
	RemoveButton(iCancelWin);
	RemoveButton(iScrollUp);
	RemoveButton(iScrollDown);
	RemoveButton(iOkWin);

	gfRenderSquareArea = FALSE;

	if (pDispList != NULL)
	{
		pDispList = TrashList(pDispList);
	}
	gfRenderTaskbar = TRUE;

	gfOverheadMapDirty = TRUE;
	EnableEditorTaskbar( );
}


//	Free the current display list for the selection window.
static DisplayList* TrashList(DisplayList* pNode)
{
	if (pNode == NULL)
		return(NULL);

	if (pNode->pNext != NULL)
		pNode->pNext = TrashList(pNode->pNext);

	if (pNode->pNext == NULL)
		MemFree(pNode);

	return(NULL);
}


static void DrawSelections(void);


//----------------------------------------------------------------------------------------------
//	RenderSelectionWindow
//
//	Displays the current selection window
//
void RenderSelectionWindow( void )
{
	GUI_BUTTON *button;
	INT32 iSX,iSY,iEX,iEY;
	UINT16 usFillColor;
	static UINT8 usFillGreen = 0;
	static UINT8 usDir = 5;


	if (!fButtonsPresent)
		return;

	ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, 600, 400, GetGenericButtonFillColor());
	DrawSelections( );
	MarkButtonsDirty();
	RenderButtons( );

	if ( gfRenderSquareArea )
	{
		button = ButtonList[iSelectWin];
		if ( button == NULL )
			return;

		if ( (abs( iStartClickX - button->Area.MouseXPos ) > 9) ||
			   (abs( iStartClickY - (button->Area.MouseYPos + iTopWinCutOff - (INT16)SelWinStartPoint.iY)) > 9) )
		{
//			iSX = (INT32)iStartClickX;
//			iEX = (INT32)button->Area.MouseXPos;
//			iSY = (INT32)iStartClickY;
//			iEY = (INT32)(button->Area.MouseYPos + iTopWinCutOff - (INT16)SelWinStartPoint.iY);

			iSX = iStartClickX;
			iSY = iStartClickY - iTopWinCutOff + SelWinStartPoint.iY;
			iEX = gusMouseXPos;
			iEY = gusMouseYPos;


			if (iEX < iSX)
			{
				iEX ^= iSX;
				iSX ^= iEX;
				iEX ^= iSX;
			}

			if (iEY < iSY)
			{
				iEY ^= iSY;
				iSY ^= iEY;
				iEY ^= iSY;
			}

			iEX = min( iEX, 600 );
			iSY = max( SelWinStartPoint.iY, iSY );
			iEY = min( 359, iEY );
			iEY = max( SelWinStartPoint.iY, iEY );

			usFillColor = Get16BPPColor(FROMRGB(255, usFillGreen, 0));
			usFillGreen += usDir;
			if( usFillGreen > 250 )
				usDir = 251;
			else if( usFillGreen < 5 )
				usDir = 5;

			ColorFillVideoSurfaceArea(FRAME_BUFFER, iSX, iSY, iEX, iSY+1, usFillColor );
			ColorFillVideoSurfaceArea(FRAME_BUFFER, iSX, iEY, iEX, iEY+1, usFillColor );
			ColorFillVideoSurfaceArea(FRAME_BUFFER, iSX, iSY, iSX+1, iEY, usFillColor );
			ColorFillVideoSurfaceArea(FRAME_BUFFER, iEX, iSY, iEX+1, iEY, usFillColor );
		}
	}
}


static void AddToSelectionList(DisplayList* pNode);
static BOOLEAN RemoveFromSelectionList(DisplayList* pNode);


//	Button callback function for the main selection window. Checks if user clicked on an image,
//	if so selects or de-selects that object. Also handles the multi-object selection (left-click
//	and drag to get the selection rectangle)
static void SelWinClkCallback(GUI_BUTTON* button, INT32 reason)
{
	DisplayList *pNode;
	BOOLEAN fDone;
	INT16 iClickX,iClickY, iYInc, iXInc;


	if (!(button->uiFlags & BUTTON_ENABLED))
		return;

	iClickX = button->Area.MouseXPos;
	iClickY = button->Area.MouseYPos + iTopWinCutOff - (INT16)SelWinStartPoint.iY;

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		button->uiFlags |= BUTTON_CLICKED_ON;
		iStartClickX = iClickX;
		iStartClickY = iClickY;
		gfRenderSquareArea = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		button->uiFlags |= BUTTON_CLICKED_ON;

		if ( gfRenderSquareArea )
		{
			gfRenderSquareArea = FALSE;
			return;
		}

		// Code to figure out what image user wants goes here
		pNode = pDispList;

		fDone = FALSE;
		while( (pNode != NULL) && !fDone )
		{
			if ( (iClickX >= pNode->iX) && (iClickX < (pNode->iX + pNode->iWidth)) &&
					(iClickY >= pNode->iY) && (iClickY < (pNode->iY + pNode->iHeight)) )
			{
				fDone = TRUE;
				if ( RemoveFromSelectionList(pNode) )
					pNode->fChosen = FALSE;
			}
			else
				pNode = pNode->pNext;
		}
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		button->uiFlags &= (~BUTTON_CLICKED_ON);
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		button->uiFlags &= (~BUTTON_CLICKED_ON);

		if( !gfRenderSquareArea )
			return;

		iEndClickX = iClickX;
		iEndClickY = iClickY;

		gfRenderSquareArea = FALSE;

		if( iEndClickX < iStartClickX )
		{
			iStartClickX ^= iEndClickX;
			iEndClickX ^= iStartClickX;
			iStartClickX ^= iEndClickX;
		}

		if( iEndClickY < iStartClickY )
		{
			iStartClickY ^= iEndClickY;
			iEndClickY ^= iStartClickY;
			iStartClickY ^= iEndClickY;
		}

		iXInc = iYInc = 1;
		for( iClickY = iStartClickY; iClickY <= iEndClickY; iClickY += iYInc )
		{
			iYInc = 1;
			for( iClickX = iStartClickX; iClickX <= iEndClickX; iClickX += iXInc )
			{
				iXInc = 1;
				// Code to figure out what image user wants goes here
				pNode = pDispList;

				fDone = FALSE;
				while( (pNode != NULL) && !fDone )
				{
					if ( (iClickX >= pNode->iX) && (iClickX < (pNode->iX + pNode->iWidth)) &&
							(iClickY >= pNode->iY) && (iClickY < (pNode->iY + pNode->iHeight)) )
					{
						fDone = TRUE;
						AddToSelectionList(pNode);
						pNode->fChosen = TRUE;
						iXInc = (pNode->iX + pNode->iWidth) - iClickX;
						if ( iYInc < ((pNode->iY + pNode->iHeight) - iClickY) )
							iYInc = (pNode->iY + pNode->iHeight) - iClickY;
					}
					else
						pNode = pNode->pNext;
				}
			}
		}
	}
}

//When a selection window is up, the file information of the picture will display
//at the top of the screen.
void DisplaySelectionWindowGraphicalInformation()
{
	DisplayList *pNode;
	BOOLEAN fDone;
	//UINT16 usObjIndex, usIndex;
	UINT16 y;
	//Determine if there is a valid picture at cursor position.
	//iRelX = gusMouseXPos;
	//iRelY = gusMouseYPos + iTopWinCutOff - (INT16)SelWinStartPoint.iY;

	y = gusMouseYPos + iTopWinCutOff - (UINT16)SelWinStartPoint.iY;
	pNode = pDispList;
	fDone = FALSE;
	while( (pNode != NULL) && !fDone )
	{
		if ( (gusMouseXPos >= pNode->iX) && (gusMouseXPos < (pNode->iX + pNode->iWidth)) &&
				(y >= pNode->iY) && (y < (pNode->iY + pNode->iHeight)) )
		{
			fDone = TRUE;
			//pNode->fChosen = TRUE;
			//iXInc = (pNode->iX + pNode->iWidth) - iClickX;
			//if ( iYInc < ((pNode->iY + pNode->iHeight) - iClickY) )
			//	iYInc = (pNode->iY + pNode->iHeight) - iClickY;
		}
		else
			pNode = pNode->pNext;
	}
	SetFont( FONT12POINT1 );
	SetFontForeground( FONT_WHITE );
	if( pNode )
	{
		//usObjIndex = (UINT16)pNode->uiObjIndx;
		//usIndex = pNode->uiIndex;
		if( !gTilesets[ giCurrentTilesetID].TileSurfaceFilenames[ pNode->uiObjIndx ][0] )
		{
			mprintf(2, 2, L"%hs[%d] is from default tileset %ls (%hs)",
				gTilesets[0].TileSurfaceFilenames[ pNode->uiObjIndx ],
				pNode->uiIndex, gTilesets[0].zName,
				gTileSurfaceName[ pNode->uiObjIndx ] );
		}
		else
		{
			mprintf(2, 2, L"File:  %hs, subindex:  %d (%hs)",
				gTilesets[ giCurrentTilesetID ].TileSurfaceFilenames[ pNode->uiObjIndx ],
				pNode->uiIndex, gTileSurfaceName[ pNode->uiObjIndx ] );
		}
	}
	mprintf(350, 2, L"Current Tileset:  %ls", gTilesets[giCurrentTilesetID].zName);
}

//----------------------------------------------------------------------------------------------
//	AddToSelectionList
//
//	Add an object in the display list to the selection list. If the object already exists in the
//	selection list, then it's count is incremented.
//
static void AddToSelectionList(DisplayList* pNode)
{
	INT32 iIndex, iUseIndex;
	BOOLEAN fDone;

	fDone = FALSE;
	for (iIndex = 0; iIndex < (*pNumSelList) && !fDone; iIndex++ )
	{
		if ( pNode->uiObjIndx == pSelList[ iIndex ].uiObject &&
				 pNode->uiIndex == pSelList[ iIndex ].usIndex )
		{
			fDone = TRUE;
			iUseIndex = iIndex;
		}
	}

	if ( fDone )
	{
		// Was already in the list, so bump up the count
		pSelList[ iUseIndex ].sCount++;
	}
	else
	{
		// Wasn't in the list, so add to end (if space available)
		if ( (*pNumSelList) < MAX_SELECTIONS )
		{
			pSelList[ (*pNumSelList) ].uiObject = pNode->uiObjIndx;
			pSelList[ (*pNumSelList) ].usIndex = pNode->uiIndex;
			pSelList[ (*pNumSelList) ].sCount = 1;

			(*pNumSelList)++;
		}
	}
}



//----------------------------------------------------------------------------------------------
//	ClearSelectionList
//
//	Removes everything from the current selection list
//
BOOLEAN ClearSelectionList( void )
{
	INT32 iIndex;
	DisplayList *pNode;


	if (pNumSelList == NULL )
		return( FALSE );

	pNode = pDispList;
	while ( pNode != NULL )
	{
		pNode->fChosen = FALSE;
		pNode = pNode->pNext;
	}

	for (iIndex = 0; iIndex < (*pNumSelList); iIndex++ )
		pSelList[ iIndex ].sCount = 0;

	(*pNumSelList) = 0;
	return( TRUE );
}


//	Removes the object given n a display list from the selection list. If the objects count is
//	greater than one, then the count is decremented and the object remains in the list.
static BOOLEAN RemoveFromSelectionList(DisplayList* pNode)
{
	INT32 iIndex, iUseIndex;
	BOOLEAN fDone, fRemoved;

	// Abort if no entries in list (pretend we removed a node)
	if ( (*pNumSelList) <= 0 )
		return( TRUE );

	fRemoved = FALSE;
	fDone = FALSE;
	for (iIndex = 0; iIndex < (*pNumSelList) && !fDone; iIndex++ )
	{
		if ( pNode->uiObjIndx == pSelList[ iIndex ].uiObject &&
				 pNode->uiIndex == pSelList[ iIndex ].usIndex )
		{
			fDone = TRUE;
			iUseIndex = iIndex;
		}
	}

	if ( fDone )
	{
		// Was already in the list, so bump up the count
		pSelList[ iUseIndex ].sCount--;

		if ( pSelList[ iUseIndex ].sCount <= 0 )
		{
			// Squash the list to remove old entry
			for ( iIndex = iUseIndex; iIndex < ((*pNumSelList) - 1); iIndex++ )
				pSelList[ iIndex ] = pSelList[ iIndex + 1 ];

			(*pNumSelList)--;
			fRemoved = TRUE;
		}
	}

	return ( fRemoved );
}


//----------------------------------------------------------------------------------------------
//	GetRandomSelection
//
//	Randomly selects an item in the selection list. The object counts are taken into account so
//	that objects with higher counts will be chosen more often.
//
INT32 GetRandomSelection( void )
{
	INT32 iRandNum, iTotalCounts;
	INT32 iIndex, iSelectedIndex, iNextCount;

	if ( fDontUseRandom )
	{
		fDontUseRandom = FALSE;
		return( iCurBank );
	}

	iTotalCounts = 0;
	for (iIndex = 0; iIndex < (*pNumSelList); iIndex++ )
		iTotalCounts += (INT32)pSelList[ iIndex ].sCount;

	iRandNum = Random( iTotalCounts );

	iSelectedIndex = -1;
	iNextCount = 0;
	for (iIndex = 0; iIndex < (*pNumSelList) && iSelectedIndex == -1; iIndex++ )
	{
		iNextCount += (INT32)pSelList[ iIndex ].sCount;
		if ( iRandNum < iNextCount )
			iSelectedIndex = iIndex;
	}

	return ( iSelectedIndex );
}


//	Verifies if a particular display list object exists in the current selection list.
static BOOLEAN IsInSelectionList(const DisplayList* pNode)
{
	for (INT32 iIndex = 0; iIndex < *pNumSelList; iIndex++)
	{
		if (pNode->uiObjIndx == pSelList[iIndex].uiObject &&
				pNode->uiIndex   == pSelList[iIndex].usIndex)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//	Finds an occurance of a particular display list object in the current selection list.
//	if found, returns the selection list's index where it can be found. otherwise it
//	returns -1
static INT32 FindInSelectionList(DisplayList* pNode)
{
	INT32 iIndex,iUseIndex;
	BOOLEAN fFound;

	fFound = FALSE;
	iUseIndex = -1;
	for (iIndex = 0; iIndex < (*pNumSelList) && !fFound; iIndex++ )
	{
		if ( pNode->uiObjIndx == pSelList[ iIndex ].uiObject &&
				 pNode->uiIndex == pSelList[ iIndex ].usIndex )
		{
			fFound = TRUE;
			iUseIndex = iIndex;
		}
	}

	return ( iUseIndex );
}


//	Copies the current selection list to a save buffer. Used in case we want to cancel a
//	selection window.
static void SaveSelectionList(void)
{
	INT32 iIndex;

	for (iIndex = 0; iIndex < MAX_SELECTIONS; iIndex++ )
		OldSelList[ iIndex ] = pSelList[ iIndex ];

	iOldNumSelList = (*pNumSelList);
}


//----------------------------------------------------------------------------------------------
//	RestoreSelectionList
//
//	Copies the selection list in the save buffer back to the current selection list.
//
void RestoreSelectionList( void )
{
	INT32 iIndex;

	for (iIndex = 0; iIndex < MAX_SELECTIONS; iIndex++ )
		pSelList[ iIndex ] = OldSelList[ iIndex ];

	(*pNumSelList) = iOldNumSelList;
}


//	Button callback function for the selection window's OK button
static void OkClkCallback(GUI_BUTTON* button, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fAllDone = TRUE;
	}
}


//----------------------------------------------------------------------------------------------
//	CnclClkCallback
//
//	Button callback function for the selection window's CANCEL button
//
static void CnclClkCallback(GUI_BUTTON* button, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fAllDone = TRUE;
		RestoreSelectionList();
	}
}


//	Button callback function for scrolling the selection window up
static void UpClkCallback(GUI_BUTTON* button, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		button->uiFlags |= BUTTON_CLICKED_ON;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		button->uiFlags &= (~BUTTON_CLICKED_ON );
		ScrollSelWinUp();
	}
}


//----------------------------------------------------------------------------------------------
//	ScrollSelWinUp
//
//	Performs the calculations required to actually scroll a selection window up by one line.
//
void ScrollSelWinUp(void)
{
	DisplayList *pNode;
	INT16 iCutOff;
	BOOLEAN fDone;

	// Code to scroll window up!
	pNode = pDispList;
	iCutOff = iTopWinCutOff;

	fDone = FALSE;
	while( (pNode != NULL) && !fDone )
	{
		if (pNode->iY >= iTopWinCutOff)
		{
			iCutOff = pNode->iY;
			pNode = pNode->pNext;
		}
		else
		{
			iCutOff = pNode->iY;
			fDone = TRUE;
		}
	}

	iTopWinCutOff = iCutOff;
}


//----------------------------------------------------------------------------------------------
//	ScrollSelWinDown
//
//	Performs the actual calculations for scrolling a selection window down.
//
void ScrollSelWinDown(void)
{
	DisplayList *pNode;
	INT16 iCutOff;
	BOOLEAN fDone;

	pNode = pDispList;
	iCutOff = iTopWinCutOff;

	fDone = FALSE;
	while( (pNode != NULL) && !fDone )
	{
		if (pNode->iY > iTopWinCutOff)
		{
			iCutOff = pNode->iY;
			pNode = pNode->pNext;
		}
		else
			fDone = TRUE;
	}

	iTopWinCutOff = iCutOff;
}


//	Button callback function to scroll the selection window down.
static void DwnClkCallback(GUI_BUTTON* button, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		button->uiFlags |= BUTTON_CLICKED_ON;
	}
	else if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		button->uiFlags &= (~BUTTON_CLICKED_ON);
		ScrollSelWinDown();
	}
}


static BOOLEAN DisplayWindowFunc(DisplayList* pNode, INT16 iTopCutOff, INT16 iBottomCutOff, SGPPoint* pUpperLeft, UINT16 fFlags);


//	Displays the objects in the display list to the selection window.
static void DrawSelections(void)
{
	SGPRect					ClipRect, NewRect;

	NewRect.iLeft = SelWinStartPoint.iX;
	NewRect.iTop = SelWinStartPoint.iY;
	NewRect.iRight = SelWinEndPoint.iX;
	NewRect.iBottom = SelWinEndPoint.iY;

	GetClippingRect(&ClipRect);
	SetClippingRect(&NewRect);

	SetFont( gpLargeFontType1 );
	SetFontShade(LARGEFONT1, 0);

	DisplayWindowFunc( pDispList, iTopWinCutOff, iBotWinCutOff, &SelWinStartPoint, CLEAR_BACKGROUND );

	SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);

	SetClippingRect(&ClipRect);
}


//----------------------------------------------------------------------------------------------
//	BuildDisplayWindow
//
//	Creates a display list from a display specification list. It also sets variables up for
//	properly scrolling the window etc.
//
static BOOLEAN BuildDisplayWindow(DisplaySpec* pDisplaySpecs, UINT16 usNumSpecs, DisplayList** pDisplayList, SGPPoint* pUpperLeft, SGPPoint* pBottomRight, SGPPoint* pSpacing, UINT16 fFlags)
{
	INT32						iCurrX = pUpperLeft->iX;
	INT32						iCurrY = pUpperLeft->iY;
	UINT16					usGreatestHeightInRow = 0;
	UINT16					usSpecLoop;
	UINT16					usETRLELoop;
	UINT16					usETRLEStart;
	UINT16					usETRLEEnd;
	DisplaySpec *		pDisplaySpec;
	ETRLEObject *		pETRLEObject;

	SaveSelectionList();

	for (usSpecLoop = 0; usSpecLoop < usNumSpecs; usSpecLoop++)
	{
		pDisplaySpec = &(pDisplaySpecs[usSpecLoop]);
		if (pDisplaySpec->ubType == DISPLAY_GRAPHIC)
		{
			if( !pDisplaySpec->hVObject )
				return FALSE;
			usETRLEStart = pDisplaySpec->usStart;
			usETRLEEnd = pDisplaySpec->usEnd;

			if ( usETRLEStart == DISPLAY_ALL_OBJECTS )
			{
				usETRLEStart = 0;
				usETRLEEnd = pDisplaySpec->hVObject->usNumberOfObjects - 1;
			}

			if( usETRLEStart > usETRLEEnd )
				return FALSE;
			if( usETRLEEnd >= pDisplaySpec->hVObject->usNumberOfObjects )
				return FALSE;

			for( usETRLELoop = usETRLEStart; usETRLELoop <= usETRLEEnd; usETRLELoop++)
			{
				pETRLEObject = &(pDisplaySpec->hVObject->pETRLEObject[usETRLELoop]);

				if ((iCurrX + pETRLEObject->usWidth > pBottomRight->iX) || (fFlags & ONE_COLUMN))
				{
					if (fFlags & ONE_ROW)
					{
						break;
					}
					iCurrX = pUpperLeft->iX;
					iCurrY += usGreatestHeightInRow + pSpacing->iY;
					usGreatestHeightInRow = 0;
				}

				DisplayList* pCurNode = MemAlloc(sizeof(*pCurNode));
				if (pCurNode == NULL) return FALSE;

				pCurNode->hObj      = pDisplaySpec->hVObject;
				pCurNode->uiIndex   = usETRLELoop;
				pCurNode->iX        = iCurrX;
				pCurNode->iY        = iCurrY;
				pCurNode->iWidth    = pETRLEObject->usWidth;
				pCurNode->iHeight   = pETRLEObject->usHeight;
				pCurNode->pNext     = *pDisplayList;
				pCurNode->uiObjIndx = pDisplaySpec->uiObjIndx;
				pCurNode->fChosen   = IsInSelectionList(pCurNode);

				*pDisplayList = pCurNode;

				if (pETRLEObject->usHeight > usGreatestHeightInRow)
				{
					usGreatestHeightInRow = pETRLEObject->usHeight;
				}

				iCurrX += pETRLEObject->usWidth + pSpacing->iX;
			}
		}
	}

	return( TRUE );
}


//----------------------------------------------------------------------------------------------
//	DisplayWindowFunc
//
//	Blits the actual object images in the display list on the selection window. The objects that
//	have been selected (in the selection list) are highlighted and the count placed in the upper
//	left corner of the image.
//
static BOOLEAN DisplayWindowFunc(DisplayList* pNode, INT16 iTopCutOff, INT16 iBottomCutOff, SGPPoint* pUpperLeft, UINT16 fFlags)
{
	INT16						iCurrY;
	INT16						sTempOffsetX;
	INT16						sTempOffsetY;
	BOOLEAN					fReturnVal;
	ETRLEObject *		pETRLEObject;
	UINT16					usFillColor;
	INT16						sCount;

	if ( pNode == NULL )
		return(TRUE);

	if ( pNode->iY < iTopCutOff )
		return(TRUE);

	fReturnVal = FALSE;
	if (DisplayWindowFunc(pNode->pNext, iTopCutOff, iBottomCutOff, pUpperLeft, fFlags))
	{
		iCurrY = pUpperLeft->iY + pNode->iY - iTopCutOff;

		if ( iCurrY > iBottomCutOff )
			return(TRUE);

		pETRLEObject = &(pNode->hObj->pETRLEObject[pNode->uiIndex]);

		// We have to store the offset data in temp variables before zeroing them and blitting
		sTempOffsetX = pETRLEObject->sOffsetX;
		sTempOffsetY = pETRLEObject->sOffsetY;

		// Set the offsets used for blitting to 0
		pETRLEObject->sOffsetX = 0;
		pETRLEObject->sOffsetY = 0;


		if (fFlags & CLEAR_BACKGROUND)
		{
			usFillColor = SelWinFillColor;
			if (pNode->fChosen)
				usFillColor = SelWinHilightFillColor;

			ColorFillVideoSurfaceArea( FRAME_BUFFER, pNode->iX, iCurrY,
												pNode->iX + pNode->iWidth,
												iCurrY + pNode->iHeight, usFillColor);
		}

		sCount = 0;
		if (pNode->fChosen)
			sCount = pSelList[ FindInSelectionList( pNode ) ].sCount;

		SetObjectShade( pNode->hObj, DEFAULT_SHADE_LEVEL );
		fReturnVal = BltVideoObject(FRAME_BUFFER, pNode->hObj, pNode->uiIndex, (UINT16)pNode->iX, (UINT16)iCurrY);

		if ( sCount != 0)
		{
			gprintf( pNode->iX, iCurrY, L"%d", sCount );
		}

		pETRLEObject->sOffsetX = sTempOffsetX;
		pETRLEObject->sOffsetY = sTempOffsetY;
	}

	return(fReturnVal);
}





#endif
