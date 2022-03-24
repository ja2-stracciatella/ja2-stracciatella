#ifndef _MAP_SCREEN_INTERFACE_MAP_H
#define _MAP_SCREEN_INTERFACE_MAP_H

#include "Types.h"
#include "UILayout.h"
struct SOLDIERTYPE;
struct VEHICLETYPE;


void InitMapScreenInterfaceMap();

// functions
void DrawMapIndexBigMap( BOOLEAN fSelectedCursorIsYellow );
//void DrawMapIndexSmallMap( BOOLEAN fSelectedCursorIsYellow );

void DrawMap(void);

void GetScreenXYFromMapXY(const SGPSector& sMap, INT16 *psX, INT16 *psY);

void InitializePalettesForMap(void);
void ShutDownPalettesForMap( void );

// plot path for helicopter
void PlotPathForHelicopter(const SGPSector& sector);

// the temp path, where the helicopter could go
void PlotATemporaryPathForHelicopter(const SGPSector& sector);

// show arrows for this char
void DisplayPathArrows(UINT16 usCharNum, HVOBJECT hMapHandle);

// build path for character
void PlotPathForCharacter(SOLDIERTYPE&, const SGPSector& sector, bool tactical_traversal);

// build temp path for character
void PlotATemporaryPathForCharacter(const SOLDIERTYPE* s, const SGPSector& sector);


// display current/temp paths
void DisplaySoldierPath( SOLDIERTYPE *pCharacter );
void DisplaySoldierTempPath(void);
void DisplayHelicopterPath( void );
void DisplayHelicopterTempPath( void );


// clear path after this sector
UINT32 ClearPathAfterThisSectorForCharacter(SOLDIERTYPE *pCharacter, const SGPSector& sMap);

// cancel path : clear the path completely and gives player feedback message that the route was canceled
void CancelPathForCharacter( SOLDIERTYPE *pCharacter );
void CancelPathForVehicle(VEHICLETYPE&, BOOLEAN fAlreadyReversed);

// check if we have waited long enought o update temp path
void DisplayThePotentialPathForHelicopter(const SGPSector& sector);

// clear out helicopter list after this sector
UINT32 ClearPathAfterThisSectorForHelicopter(const SGPSector& sMap);


// check to see if sector is highlightable
bool IsTheCursorAllowedToHighLightThisSector(INT16 x, INT16 y);

// restore background for map grids
void RestoreBackgroundForMapGrid(const SGPSector& sMap);

// clip blits to map view region
void ClipBlitsToMapViewRegion( void );
void ClipBlitsToMapViewRegionForRectangleAndABit( UINT32 uiDestPitchBYTES );

// clip blits to full screen....restore after use of ClipBlitsToMapViewRegion( )
void RestoreClipRegionToFullScreen( void );
void RestoreClipRegionToFullScreenForRectangle( UINT32 uiDestPitchBYTES );

// last sector in helicopter's path
INT16 GetLastSectorOfHelicoptersPath( void );

// display info about helicopter path
void DisplayDistancesForHelicopter( void );

// display where hei is
void DisplayPositionOfHelicopter( void );

// check for click
BOOLEAN CheckForClickOverHelicopterIcon(const SGPSector& sMap);

void LoadMapScreenInterfaceMapGraphics();
void DeleteMapScreenInterfaceMapGraphics();


// grab the total number of militia in sector
INT32 GetNumberOfMilitiaInSector(const SGPSector& sector);

// create destroy
void CreateDestroyMilitiaPopUPRegions( void );

// draw the militia box
void DrawMilitiaPopUpBox();


//Returns true if the player knows how many enemies are in the sector if that number is greater than 0.
//Returns false for all other cases.
UINT32 WhatPlayerKnowsAboutEnemiesInSector(const SGPSector& sSector);

//There is a special case flag used when players encounter enemies in a sector, then retreat.  The number of enemies
//will display on mapscreen until time is compressed.  When time is compressed, the flag is cleared, and
//a question mark is displayed to reflect that the player no longer knows.  This is the function that clears that
//flag.
void ClearAnySectorsFlashingNumberOfEnemies(void);


enum {
	ABORT_PLOTTING = 0,
	PATH_CLEARED,
	PATH_SHORTENED,
};

// what the player knows about the enemies in a given sector
enum {
	KNOWS_NOTHING = 0,
	KNOWS_THEYRE_THERE,
	KNOWS_HOW_MANY,
};


// size of squares on the map
#define MAP_GRID_X		21
#define MAP_GRID_Y		18


// map view region
#define MAP_VIEW_START_X	(STD_SCREEN_X + 270)
#define MAP_VIEW_START_Y	(STD_SCREEN_Y + 10)
#define MAP_VIEW_WIDTH		336
#define MAP_VIEW_HEIGHT		298


// number of units wide
#define WORLD_MAP_X		18

// dirty regions for the map
#define DMAP_GRID_X		( MAP_GRID_X + 1 )
#define DMAP_GRID_Y		( MAP_GRID_Y + 1 )


// Tixa position on the map
#define TIXA_SECTOR_X		9
#define TIXA_SECTOR_Y		10


// wait time until temp path is drawn, from placing cursor on a map grid
#define MIN_WAIT_TIME_FOR_TEMP_PATH 200


// the number of militia on the cursor
extern INT16 sGreensOnCursor;
extern INT16 sRegularsOnCursor;
extern INT16 sElitesOnCursor;

// highlighted sectors
extern INT16 gsHighlightSectorX;
extern INT16 gsHighlightSectorY;

// the viewable map bound region
extern SGPRect MapScreenRect;

// draw temp path
extern BOOLEAN fDrawTempHeliPath;

// selected destination char
extern INT8 bSelectedDestChar;

// current assignment character
extern INT8 bSelectedAssignChar;

// the contract char
extern INT8 bSelectedContractChar;

// has temp path for character path or helicopter been already drawn
extern BOOLEAN  fTempPathAlreadyDrawn;

// the currently selected town militia
extern INT16 sSelectedMilitiaTown;

// the selected sectors
extern UINT16	sSelMapX;
extern UINT16	sSelMapY;


void    CreateDestroyMilitiaSectorButtons(void);
BOOLEAN CanRedistributeMilitiaInSector(INT8 bClickedTownId);

#endif
