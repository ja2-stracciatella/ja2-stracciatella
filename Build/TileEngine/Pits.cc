#include "TileDat.h"
#include "Types.h"
#include "Pits.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Simple_Render_Utils.h"
#include "World_Items.h"
#include "SaveLoadMap.h"
#include "Exit_Grids.h"
#include "Sys_Globals.h"
#include "StrategicMap.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "Sound_Control.h"
#include "Animation_Control.h"
#include "Strategic.h"
#include "Action_Items.h"


//used by editor
BOOLEAN gfShowPits = FALSE;


void Add3X3Pit( INT32 iMapIndex )
{
	EXITGRID ExitGrid;
	if( !gfEditMode )
		ApplyMapChangesToMapTempFile( TRUE );
	AddObjectToTail( iMapIndex + 159, REGWATERTEXTURE1 );
	AddObjectToTail( iMapIndex -   1,	REGWATERTEXTURE2 );
	AddObjectToTail( iMapIndex - 161, REGWATERTEXTURE3 );
	AddObjectToTail( iMapIndex + 160, REGWATERTEXTURE4 );
	AddObjectToTail( iMapIndex,				REGWATERTEXTURE5 );
	AddObjectToTail( iMapIndex - 160, REGWATERTEXTURE6 );
	AddObjectToTail( iMapIndex + 161, REGWATERTEXTURE7 );
	AddObjectToTail( iMapIndex +   1, REGWATERTEXTURE8 );
	AddObjectToTail( iMapIndex - 159, REGWATERTEXTURE9 );
	if( !gfEditMode )
	{ //Add the exitgrids associated with the pit.
		ExitGrid.ubGotoSectorX = (UINT8)gWorldSectorX;
		ExitGrid.ubGotoSectorY = (UINT8)gWorldSectorY;
		ExitGrid.ubGotoSectorZ = (UINT8)(gbWorldSectorZ+1);
		ExitGrid.usGridNo = (UINT16)iMapIndex;
		AddExitGridToWorld( iMapIndex + 159, &ExitGrid );
		AddExitGridToWorld( iMapIndex -   1, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 161, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 160, &ExitGrid );
		AddExitGridToWorld( iMapIndex,			 &ExitGrid );
		AddExitGridToWorld( iMapIndex - 160, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 161, &ExitGrid );
		AddExitGridToWorld( iMapIndex +   1, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 159, &ExitGrid );
		RecompileLocalMovementCostsFromRadius( (INT16)iMapIndex, 2 );
	}

	MarkWorldDirty();
	if( !gfEditMode )
		ApplyMapChangesToMapTempFile( FALSE );
}

void Add5X5Pit( INT32 iMapIndex )
{
	EXITGRID ExitGrid;
	if( !gfEditMode )
		ApplyMapChangesToMapTempFile( TRUE );
	AddObjectToTail( iMapIndex + 318, REGWATERTEXTURE10 );
	AddObjectToTail( iMapIndex + 158, REGWATERTEXTURE11 );
	AddObjectToTail( iMapIndex -   2, REGWATERTEXTURE12 );
	AddObjectToTail( iMapIndex - 162, REGWATERTEXTURE13 );
	AddObjectToTail( iMapIndex - 322, REGWATERTEXTURE14 );
	AddObjectToTail( iMapIndex + 319, REGWATERTEXTURE15 );
	AddObjectToTail( iMapIndex + 159, REGWATERTEXTURE16 );
	AddObjectToTail( iMapIndex -   1, REGWATERTEXTURE17 );
	AddObjectToTail( iMapIndex - 161, REGWATERTEXTURE18 );
	AddObjectToTail( iMapIndex - 321, REGWATERTEXTURE19 );
	AddObjectToTail( iMapIndex + 320, REGWATERTEXTURE20 );
	AddObjectToTail( iMapIndex + 160, REGWATERTEXTURE21 );
	AddObjectToTail( iMapIndex,				REGWATERTEXTURE22 );
	AddObjectToTail( iMapIndex - 160, REGWATERTEXTURE23 );
	AddObjectToTail( iMapIndex - 320, REGWATERTEXTURE24 );
	AddObjectToTail( iMapIndex + 321, REGWATERTEXTURE25 );
	AddObjectToTail( iMapIndex + 161, REGWATERTEXTURE26 );
	AddObjectToTail( iMapIndex +   1, REGWATERTEXTURE27 );
	AddObjectToTail( iMapIndex - 159, REGWATERTEXTURE28 );
	AddObjectToTail( iMapIndex - 319, REGWATERTEXTURE29 );
	AddObjectToTail( iMapIndex + 322, REGWATERTEXTURE30 );
	AddObjectToTail( iMapIndex + 162, REGWATERTEXTURE31 );
	AddObjectToTail( iMapIndex +   2, REGWATERTEXTURE32 );
	AddObjectToTail( iMapIndex - 158, REGWATERTEXTURE33 );
	AddObjectToTail( iMapIndex - 318, REGWATERTEXTURE34 );
	if( !gfEditMode )
	{ //Add the exitgrids associated with the pit.
		ExitGrid.ubGotoSectorX = (UINT8)gWorldSectorX;
		ExitGrid.ubGotoSectorY = (UINT8)gWorldSectorY;
		ExitGrid.ubGotoSectorZ = (UINT8)(gbWorldSectorZ+1);
		ExitGrid.usGridNo = (UINT16)iMapIndex;
		AddExitGridToWorld( iMapIndex + 318, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 158, &ExitGrid );
		AddExitGridToWorld( iMapIndex -   2, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 162, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 322, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 319, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 159, &ExitGrid );
		AddExitGridToWorld( iMapIndex -   1, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 161, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 321, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 320, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 160, &ExitGrid );
		AddExitGridToWorld( iMapIndex,			 &ExitGrid );
		AddExitGridToWorld( iMapIndex - 160, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 320, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 321, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 161, &ExitGrid );
		AddExitGridToWorld( iMapIndex +   1, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 159, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 319, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 322, &ExitGrid );
		AddExitGridToWorld( iMapIndex + 162, &ExitGrid );
		AddExitGridToWorld( iMapIndex +   2, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 158, &ExitGrid );
		AddExitGridToWorld( iMapIndex - 318, &ExitGrid );
		RecompileLocalMovementCostsFromRadius( (INT16)iMapIndex, 3 );
	}
	MarkWorldDirty();
	if( !gfEditMode )
		ApplyMapChangesToMapTempFile( FALSE );
}

void Remove3X3Pit( INT32 iMapIndex )
{
	RemoveAllObjectsOfTypeRange( iMapIndex + 159, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex -   1,	REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 161, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 160, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex,				REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 160, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 161, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex +   1, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 159, REGWATERTEXTURE, REGWATERTEXTURE );
	MarkWorldDirty();
}

void Remove5X5Pit( INT32 iMapIndex )
{
	RemoveAllObjectsOfTypeRange( iMapIndex + 318, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 158, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex -   2, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 162, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 322, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 319, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 159, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex -   1, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 161, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 321, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 320, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 160, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex,				REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 160, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 320, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 321, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 161, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex +   1, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 159, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 319, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 322, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex + 162, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex +   2, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 158, REGWATERTEXTURE, REGWATERTEXTURE );
	RemoveAllObjectsOfTypeRange( iMapIndex - 318, REGWATERTEXTURE, REGWATERTEXTURE );
	MarkWorldDirty();
}

void AddAllPits()
{
	UINT32 i;
	for( i = 0; i < guiNumWorldItems; i++ )
	{
		const WORLDITEM* const wi = GetWorldItem(i);
		if (wi->o.usItem != ACTION_ITEM) continue;

		switch (wi->o.bActionValue)
		{
			case ACTION_ITEM_SMALL_PIT: Add3X3Pit(wi->sGridNo); break;
			case ACTION_ITEM_LARGE_PIT: Add5X5Pit(wi->sGridNo); break;
		}
	}
}

void RemoveAllPits()
{
	UINT32 i;
	for( i = 0; i < guiNumWorldItems; i++ )
	{
		const WORLDITEM* const wi = GetWorldItem(i);
		if (wi->o.usItem != ACTION_ITEM) continue;

		switch (wi->o.bActionValue)
		{
			case ACTION_ITEM_SMALL_PIT: Remove3X3Pit(wi->sGridNo); break;
			case ACTION_ITEM_LARGE_PIT: Remove5X5Pit(wi->sGridNo); break;
		}
	}
}

void SearchForOtherMembersWithinPitRadiusAndMakeThemFall( INT16 sGridNo, INT16 sRadius )
{
	INT16 x, y, sNewGridNo;

	PlayLocationJA2Sample(sGridNo, CAVE_COLLAPSE, HIGHVOLUME, 1);
	for( y = -sRadius; y <= sRadius; y++ ) for( x = -sRadius; x <= sRadius; x++ )
	{
		sNewGridNo = sGridNo + y * WORLD_COLS + x;
		//Validate gridno location, and check if there are any mercs here.  If there are
		//any mercs, we want them to fall below.  The exitgrid already exists at this location
		if( GridNoOnVisibleWorldTile( sNewGridNo ) )
		{

			// Check if buddy exists here.....
			SOLDIERTYPE* const pSoldier = WhoIsThere2(sNewGridNo, 0);
			if (pSoldier != NULL)
			{
				// OK, make guy fall...
				// Set data to look for exit grid....
				pSoldier->uiPendingActionData4 = sNewGridNo;

				EVENT_InitNewSoldierAnim( pSoldier, FALL_INTO_PIT, 0 , FALSE );
			}
		}
	}
}


void HandleFallIntoPitFromAnimation(SOLDIERTYPE* const pSoldier)
{
	EXITGRID ExitGrid;
	INT16 sPitGridNo;
	// OK, get exit grid...

	sPitGridNo = (INT16)pSoldier->uiPendingActionData4;

	GetExitGrid( sPitGridNo, &ExitGrid );

	// Given exit grid, make buddy move to next sector....
	pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
	pSoldier->usStrategicInsertionData = ExitGrid.usGridNo;

	pSoldier->sSectorX = ExitGrid.ubGotoSectorX;
	pSoldier->sSectorY = ExitGrid.ubGotoSectorY;
	pSoldier->bSectorZ = ExitGrid.ubGotoSectorZ;

	// Remove from world......
	RemoveSoldierFromTacticalSector(pSoldier);

	HandleSoldierLeavingSectorByThemSelf( pSoldier );

	SetSoldierHeight( pSoldier, 0 );

}
