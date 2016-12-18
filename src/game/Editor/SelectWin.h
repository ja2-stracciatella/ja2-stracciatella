#ifndef _SELECTION_WIN_H_
#define _SELECTION_WIN_H_

#include "sgp/Types.h"


enum SelectWindow
{
	SELWIN_SINGLEWALL,
	SELWIN_SINGLEDOOR,
	SELWIN_SINGLEWINDOW,
	SELWIN_SINGLEROOF,
	SELWIN_SINGLENEWROOF,
	SELWIN_SINGLEBROKENWALL,
	SELWIN_SINGLEDECOR,
	SELWIN_SINGLEDECAL,
	SELWIN_SINGLEFLOOR,
	SELWIN_SINGLETOILET,

	SELWIN_ROOM,
	SELWIN_BANKS,
	SELWIN_ROADS,
	SELWIN_DEBRIS,
	SELWIN_OSTRUCTS,
	SELWIN_OSTRUCTS1,
	SELWIN_OSTRUCTS2
};

extern BOOLEAN fAllDone;

void CreateJA2SelectionWindow(SelectWindow);
extern void InitJA2SelectionWindow( void );
extern void ShutdownJA2SelectionWindow( void );
extern void RemoveJA2SelectionWindow( void );
extern void RenderSelectionWindow( void );


extern void ScrollSelWinUp(void);
extern void ScrollSelWinDown(void);

#define MAX_SELECTIONS		120

struct Selections
{
	UINT32		uiObject;
	UINT16		usIndex;
	INT16			sCount;
};


INT32 GetRandomSelection( void );
void RestoreSelectionList( void );
BOOLEAN ClearSelectionList( void );

void DisplaySelectionWindowGraphicalInformation(void);


extern INT32				iCurBank;
extern Selections		*pSelList;
extern INT32				*pNumSelList;

extern Selections		SelOStructs[ MAX_SELECTIONS ];
extern Selections		SelOStructs1[ MAX_SELECTIONS ];
extern Selections		SelOStructs2[ MAX_SELECTIONS ];
extern Selections		SelBanks[ MAX_SELECTIONS ];
extern Selections		SelRoads[ MAX_SELECTIONS ];
extern Selections		SelDebris[ MAX_SELECTIONS ];

extern Selections		SelSingleWall[ MAX_SELECTIONS ];
extern Selections		SelSingleDoor[ MAX_SELECTIONS ];
extern Selections		SelSingleWindow[ MAX_SELECTIONS ];
extern Selections		SelSingleRoof[ MAX_SELECTIONS ];
extern Selections		SelSingleNewRoof[ MAX_SELECTIONS ];
extern Selections   SelSingleBrokenWall[ MAX_SELECTIONS ];
extern Selections		SelSingleDecor[ MAX_SELECTIONS ];
extern Selections		SelSingleDecal[ MAX_SELECTIONS ];
extern Selections		SelSingleFloor[ MAX_SELECTIONS ];
extern Selections   SelSingleToilet[ MAX_SELECTIONS ];

extern Selections		SelRoom[ MAX_SELECTIONS ];

extern INT32				iNumOStructsSelected;
extern INT32				iNumOStructs1Selected;
extern INT32				iNumOStructs2Selected;
extern INT32				iNumBanksSelected;
extern INT32				iNumRoadsSelected;
extern INT32				iNumDebrisSelected;
extern INT32				iNumWallsSelected;
extern INT32				iNumDoorsSelected;
extern INT32				iNumWindowsSelected;
extern INT32				iNumDecorSelected;
extern INT32				iNumDecalsSelected;
extern INT32				iNumBrokenWallsSelected;
extern INT32				iNumFloorsSelected;
extern INT32				iNumToiletsSelected;
extern INT32				iNumRoofsSelected;
extern INT32				iNumNewRoofsSelected;
extern INT32				iNumRoomsSelected;

extern INT32 iDrawMode;

#endif
