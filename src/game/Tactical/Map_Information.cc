#include "FileMan.h"
#include "Types.h"
#include "Map_Information.h"
#include "Soldier_Init_List.h"
#include "Random.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "EditorMercs.h"
#include "Exit_Grids.h"
#include "Isometric_Utils.h"
#include "Animation_Data.h"
#include "Road_Smoothing.h"
#include "Weapons.h"
#include "Environment.h"
#include "Lighting.h"
#include "Animated_ProgressBar.h"
#include "Debug.h"
#include "World_Items.h"


BOOLEAN gfWorldLoaded;

struct MAPCREATE_STRUCT;
MAPCREATE_STRUCT gMapInformation;

//Current minor map version updater.
#define MINOR_MAP_VERSION 25
UINT8 gubMinorMapVersion = MINOR_MAP_VERSION;

// MINOR_MAP_VERSION Log -- Created by Kris Morness, November 14, 1997
// Version 0 -- Kris -- obsolete November 14, 1997
//   The newly created soldier information had a couple bugs regarding initialization
//   Bug 1)  Soldier placements without detailed slots weren't initialized.
//   Bug 2)  The attitude variable was accidentally being generated like attributes
//           which usually put it completely out of range.
// Version 1 -- Kris -- obsolete January 7, 1998
//   Bug 3)  New changes to the wall decals, made the currently existing ones in the world
//           unpredictable, and are automatically removed, so new ones can be used.
// Version 2 -- Kris -- obsolete February 3, 1998
//   Bug 4)  Enemy mercs now have a color code assignment which defaults to army.
// Version 3 -- Kris -- obsolete February 9, 1998
//   Bug 5)  Move entry points down if they are too high.
// Version 4 -- Kris -- obsolete February 25, 1998
//   Bug 6)  Change all doors to FIRSTDOOR
// Version 5 -- Kris -- obsolete March 4, 1998
//   Bug 7)  Remove all exit grids (the format has changed)
// Version 6 -- Kris -- obsolete March 9, 1998
//   Bug 8)  Change droppable status of merc items so that they are all undroppable.
// Version 7 -- Kris -- obsolete April 14, 1998
//   Bug 9)  Priority placements have been dropped in favor of splitting it into two categories.
//           The first is Detailed placements, and the second is priority existance.  So, all
//           current detailed placements will also have priority existance.
// Version 8 -- Kris -- obsolete April 16, 1998
//   MAJOR CONFLICT RESULTING IN A MAJOR VERSION UPDATE 2.00!
//   Bug 10) Padding on detailed placements is uninitialized.  Clear it.
// Version 9 -- Kris -- obsolete April 26, 1998
//   This version requires no auto updating, but external code has adjusted the size of the mapedgepoint
//   arraysize from UINT8 to UINT16.  See Map Edgepoints.c.
//   Bug 11) Convert all wheelchaired placement bodytypes to cows.  Result of change in the animation database.
// Version 11 -- Kris -- obsolete May 2, 1998
//   Added new center entry point.  Need to initialize the original padding to -1.


//EntryPoints can't be placed on the top two gridnos in a map.  So all we do in this case
//is return the closest gridno.  Returns TRUE if the mapindex changes.
BOOLEAN ValidateEntryPointGridNo( INT16 *sGridNo )
{
	INT16 sWorldX, sWorldY;
	INT16 sTopLimit, sBottomLimit;

	if( *sGridNo < 0 )
		return FALSE; //entry point is non-existant

	sTopLimit = 80;
	sBottomLimit = gsBottomY - gsTopY - 40;

	//Get screen coordinates for current gridno
	GetAbsoluteScreenXYFromMapPos(*sGridNo, &sWorldX, &sWorldY);

	if( sWorldY < sTopLimit )
	{
		*sGridNo = GetMapPosFromAbsoluteScreenXY(sWorldX, sTopLimit);
	}
	else if( sWorldY > sBottomLimit )
	{
		*sGridNo = GetMapPosFromAbsoluteScreenXY(sWorldX, sBottomLimit);
	}
	else
	{
		return FALSE; //already valid
	}

	return TRUE; //modified
}


void SaveMapInformation( HWFILE fp )
{
	gMapInformation.ubMapVersion = MINOR_MAP_VERSION;
	fp->write(&gMapInformation, sizeof(MAPCREATE_STRUCT));
}


void LoadMapInformation(HWFILE const f)
{
	f->read(&gMapInformation, sizeof(gMapInformation));

	// ATE: OK, do some handling here for basement level scroll restrictions
	// Calcuate world scrolling restrictions
	InitRenderParams( gMapInformation.ubRestrictedScrollID );
}


//This will automatically update obsolete map versions to the new ones.  This will even
//work in the game itself, but would require conversion to happen every time.  This is completely
//transparent to the rest of the game, but in the editor, obsolete versions will be updated upon
//loading and won't be permanently updated until the map is saved, regardless of changes.
static void UpdateOldVersionMap(void)
{
	if( gMapInformation.ubMapVersion < 16 )
	{
		gMapInformation.ubMapVersion = 16;
		gMapInformation.sIsolatedGridNo = -1;
	}
	if( gMapInformation.ubMapVersion < 17 )
	{
		gMapInformation.ubMapVersion = 17;
	}
	if( gMapInformation.ubMapVersion < 18 )
	{
		// replace useless crowbars with proper ones
		gMapInformation.ubMapVersion = 18;
		FOR_EACH_WORLD_ITEM(wi)
		{
			OBJECTTYPE& o = wi.o;
			if (o.usItem == JAR_ELIXIR) o.usItem = CROWBAR;
		}
	}
	if( gMapInformation.ubMapVersion < 19 )
	{
		//Do nothing, this is used to force regenerate the map edgepoints in map edgepoints.c
		gMapInformation.ubMapVersion = 19;
	}
	if( gMapInformation.ubMapVersion < 20 )
	{
		//validate the map entry points as the world boundaries have changed.
		gMapInformation.ubMapVersion = 20;
		ValidateEntryPointGridNo( &gMapInformation.sNorthGridNo );
		ValidateEntryPointGridNo( &gMapInformation.sEastGridNo );
		ValidateEntryPointGridNo( &gMapInformation.sSouthGridNo );
		ValidateEntryPointGridNo( &gMapInformation.sWestGridNo );
		ValidateEntryPointGridNo( &gMapInformation.sCenterGridNo );
		ValidateEntryPointGridNo( &gMapInformation.sIsolatedGridNo );
	}
	if( gMapInformation.ubMapVersion < 21 )
	{
		//override any item slots being locked if there is no item in that slot.
		//Laymen terms:  If any items slots are locked to be empty, make them empty but available
		//for random item generation.
		gMapInformation.ubMapVersion = 21;
		CFOR_EACH_SOLDIERINITNODE(curr)
		{
			if( curr->pDetailedPlacement )
			{
				INT32 i;
				for( i = 0; i < NUM_INV_SLOTS; i++ )
				{
					if( !curr->pDetailedPlacement->Inv[ i ].usItem )
					{
						if( curr->pDetailedPlacement->Inv[ i ].fFlags & OBJECT_UNDROPPABLE )
						{
							if( curr->pDetailedPlacement->Inv[ i ].fFlags & OBJECT_NO_OVERWRITE )
							{
								curr->pDetailedPlacement->Inv[ i ].fFlags &= ~OBJECT_NO_OVERWRITE;
							}
						}
					}
				}
			}
		}
	}
	if( gMapInformation.ubMapVersion < 22 )
	{ //Allow map edgepoints to be regenerated as new system has been reenabled.
		gMapInformation.ubMapVersion = 22;
	}
	if( gMapInformation.ubMapVersion < 23 )
	{ //Allow map edgepoints to be regenerated as new system has been reenabled.
		gMapInformation.ubMapVersion = 23;
		if (giCurrentTilesetID == CAVES_1) //cave/mine tileset only
		{ //convert all civilians to miners which use uniforms and more masculine body types.
			CFOR_EACH_SOLDIERINITNODE(curr)
			{
				if( curr->pBasicPlacement->bTeam == CIV_TEAM && !curr->pDetailedPlacement )
				{
					curr->pBasicPlacement->ubSoldierClass = SOLDIER_CLASS_MINER;
					curr->pBasicPlacement->bBodyType      = BODY_RANDOM;
				}
			}
		}
	}
	if( gMapInformation.ubMapVersion < 25 )
	{
		gMapInformation.ubMapVersion = 25;
		if( gfCaves )
		{
			LightSetBaseLevel( 13 );
		}
	}
}


static void AutoCalculateItemNoOverwriteStatus(void)
{
	INT32 i;
	OBJECTTYPE *pItem;

	//Recalculate the "no overwrite" status flag on all items.  There are two different cases:
	//1)  If detailed placement has item, the item "no overwrite" flag is set
	//2)  If detailed placement doesn't have item, but item is set to drop (forced empty slot),
	//    the "no overwrite" flag is set.
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pDetailedPlacement )
		{
			for( i = 0; i < NUM_INV_SLOTS; i++ )
			{
				pItem = &curr->pDetailedPlacement->Inv[ i ];
				if( pItem->usItem != NONE )
				{
					//case 1 (see above)
					pItem->fFlags |= OBJECT_NO_OVERWRITE;
				}
				else if( !(pItem->fFlags & OBJECT_UNDROPPABLE) )
				{
					//case 2 (see above)
					pItem->fFlags |= OBJECT_NO_OVERWRITE;
				}
			}
		}
	}
}

void ValidateAndUpdateMapVersionIfNecessary()
{
	//Older versions of mercs may require updating due to past bug fixes, new changes, etc.
	if( gMapInformation.ubMapVersion < MINOR_MAP_VERSION )
	{
		SetRelativeStartAndEndPercentage( 0, 92, 93, L"Updating older map version..." );
		RenderProgressBar( 0, 0 );
		UpdateOldVersionMap();
	}
	else if( gMapInformation.ubMapVersion > MINOR_MAP_VERSION )
	{
		//we may have a problem...
		SLOGW("Map version is greater than the current version (old ja2.exe?)");
	}
	AutoCalculateItemNoOverwriteStatus() ;
}

#include "Summary_Info.h"
//This function is used to avoid conflicts between minor version updates and sector summary info.
//By updating the summary info in conjunction with minor version updates, we can avoid these conflicts
//and really prevent major map updates.
void UpdateSummaryInfo( SUMMARYFILE *pSummary )
{
	if( pSummary->MapInfo.ubMapVersion == MINOR_MAP_VERSION )
		return;
	if( pSummary->MapInfo.ubMapVersion < 9 )
	{
		//See bug 10
		pSummary->ubCivSchedules = 0;
	}
	if( pSummary->MapInfo.ubMapVersion < 12 )
	{
		pSummary->MapInfo.sCenterGridNo = -1;
	}
	if( pSummary->MapInfo.ubMapVersion < 16 )
	{
		pSummary->MapInfo.sIsolatedGridNo = -1;
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(MapInformation, asserts)
{
	EXPECT_EQ(sizeof(MAPCREATE_STRUCT), 100u);
}

#endif
