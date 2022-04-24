#include "HImage.h"
#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Smooth.h"
#include "NewSmooth.h"
#include "Edit_Sys.h"
#include "EditScreen.h"
#include "SelectWin.h"
#include "EditorTerrain.h"
#include "EditorBuildings.h"
#include "EditorMercs.h"
#include "EditorDefines.h"
#include "Smoothing_Utils.h"
#include "Cursor_Modes.h"
#include "Render_Fun.h"
#include "Isometric_Utils.h"
#include "Editor_Undo.h"
#include "Exit_Grids.h"
#include "Environment.h"
#include "Simple_Render_Utils.h"
#include "Road_Smoothing.h"
#include "MemMan.h"
#include "Logger.h"

#include <vector>

UINT16			CurrentPaste = NO_TILE;


//---------------------------------------------------------------------------------------------------------------
//	QuickEraseMapTile
//
//	Performs ersing operation when the DEL key is hit in the editor
//
void QuickEraseMapTile( UINT32 iMapIndex )
{
	if ( iMapIndex >= GRIDSIZE )
		return;
	AddToUndoList( iMapIndex );
	DeleteStuffFromMapTile( iMapIndex );
	MarkWorldDirty();
}


//---------------------------------------------------------------------------------------------------------------
//	DeleteStuffFromMapTile
//
//	Common delete function for both QuickEraseMapTile and EraseMapTile
//
void DeleteStuffFromMapTile( UINT32 iMapIndex )
{
	//UINT16		usUseIndex;
	//UINT16		usType;
	//UINT16		usDummy;

	//const UINT32 uiCheckType = GetTileType(gpWorldLevelData[iMapIndex].pLandHead->usIndex);
	//RemoveLand( iMapIndex, gpWorldLevelData[ iMapIndex ].pLandHead->usIndex );
	//SmoothTerrainRadius( iMapIndex, uiCheckType, 1, TRUE );

	RemoveExitGridFromWorld( iMapIndex );
	RemoveAllStructsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	RemoveAllObjectsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	RemoveAllLandsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	RemoveAllRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	RemoveAllOnRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	RemoveAllTopmostsOfTypeRange( iMapIndex, FIRSTTEXTURE, WIREFRAMES );
	PasteRoomNumber( iMapIndex, 0 );
}


//---------------------------------------------------------------------------------------------------------------
//	EraseMapTile
//
//	Generic tile erasing function. Erases things from the world depending on the current drawing mode
//
void EraseMapTile( UINT32 iMapIndex )
{
	INT32			iEraseMode;
	if ( iMapIndex >= GRIDSIZE )
		return;

	// Figure out what it is we are trying to erase
	iEraseMode = iDrawMode - DRAW_MODE_ERASE;

	switch ( iEraseMode )
	{
		case DRAW_MODE_NORTHPOINT:
		case DRAW_MODE_WESTPOINT:
		case DRAW_MODE_EASTPOINT:
		case DRAW_MODE_SOUTHPOINT:
		case DRAW_MODE_CENTERPOINT:
		case DRAW_MODE_ISOLATEDPOINT:
			SpecifyEntryPoint( iMapIndex );
			break;
		case DRAW_MODE_EXITGRID:
			AddToUndoList( iMapIndex );
			RemoveExitGridFromWorld( iMapIndex );
			RemoveTopmost( (UINT16)iMapIndex, FIRSTPOINTERS8 );
			break;
		case DRAW_MODE_GROUND:
		{
			// Is there ground on this tile? if not, get out o here
			if ( gpWorldLevelData[ iMapIndex ].pLandHead == NULL )
				break;

			// is there only 1 ground tile here? if so, get out o here
			if ( gpWorldLevelData[ iMapIndex ].pLandHead->pNext == NULL )
				break;
			AddToUndoList( iMapIndex );
			const UINT32 uiCheckType = GetTileType(gpWorldLevelData[iMapIndex].pLandHead->usIndex);
			RemoveLand( iMapIndex, gpWorldLevelData[ iMapIndex ].pLandHead->usIndex );
			SmoothTerrainRadius( iMapIndex, uiCheckType, 1, TRUE );
			break;
		}

		case DRAW_MODE_OSTRUCTS:
		case DRAW_MODE_OSTRUCTS1:
		case DRAW_MODE_OSTRUCTS2:
			AddToUndoList( iMapIndex );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTOSTRUCT, LASTOSTRUCT );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTVEHICLE, SECONDVEHICLE );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTDEBRISSTRUCT, SECONDDEBRISSTRUCT );
			RemoveAllStructsOfTypeRange( iMapIndex, NINTHOSTRUCT, TENTHOSTRUCT );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTLARGEEXPDEBRIS, SECONDLARGEEXPDEBRIS );
			RemoveAllObjectsOfTypeRange( iMapIndex, DEBRIS2MISC, DEBRIS2MISC );
			RemoveAllObjectsOfTypeRange( iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS );
			break;
		case DRAW_MODE_DEBRIS:
			AddToUndoList( iMapIndex );
			RemoveAllObjectsOfTypeRange( iMapIndex, DEBRISROCKS, LASTDEBRIS );
			RemoveAllObjectsOfTypeRange( iMapIndex, DEBRIS2MISC, DEBRIS2MISC );
			RemoveAllObjectsOfTypeRange( iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS );
			break;
		case DRAW_MODE_BANKS:
			AddToUndoList( iMapIndex );
			RemoveAllObjectsOfTypeRange( iMapIndex, FIRSTROAD, LASTROAD );
			// Note, for this routine, cliffs are considered a subset of banks
			RemoveAllStructsOfTypeRange( iMapIndex, ANIOSTRUCT, ANIOSTRUCT );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTCLIFF, LASTBANKS );
			RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTCLIFFSHADOW, LASTCLIFFSHADOW );
			RemoveAllObjectsOfTypeRange( iMapIndex, FIRSTCLIFFHANG, LASTCLIFFHANG );
			RemoveAllStructsOfTypeRange( iMapIndex, FENCESTRUCT, FENCESTRUCT );
			RemoveAllShadowsOfTypeRange( iMapIndex, FENCESHADOW, FENCESHADOW );
			break;
		case DRAW_MODE_FLOORS:
			AddToUndoList( iMapIndex );
			RemoveAllLandsOfTypeRange( iMapIndex, FIRSTFLOOR, LASTFLOOR );
			break;
		case DRAW_MODE_ROOFS:
		case DRAW_MODE_NEWROOF:
			AddToUndoList( iMapIndex );
			RemoveAllRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, LASTITEM );
			RemoveAllOnRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, LASTITEM );
			break;
		case DRAW_MODE_WALLS:
		case DRAW_MODE_DOORS:
		case DRAW_MODE_WINDOWS:
		case DRAW_MODE_BROKEN_WALLS:
			AddToUndoList( iMapIndex );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
			RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTDOOR, LASTDOOR );
			RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTDOORSHADOW, LASTDOORSHADOW );
			break;
		case DRAW_MODE_DECOR:
		case DRAW_MODE_DECALS:
		case DRAW_MODE_ROOM:
		case DRAW_MODE_TOILET:
			AddToUndoList( iMapIndex );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTWALLDECAL, LASTWALLDECAL );
			RemoveAllStructsOfTypeRange( iMapIndex, FIFTHWALLDECAL, EIGTHWALLDECAL );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTDECORATIONS, LASTDECORATIONS );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTISTRUCT, LASTISTRUCT );
			RemoveAllStructsOfTypeRange( iMapIndex, FIFTHISTRUCT, EIGHTISTRUCT );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTSWITCHES, FIRSTSWITCHES );
			break;
		case DRAW_MODE_CAVES:
			AddToUndoList( iMapIndex );
			RemoveAllStructsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
			break;
		case DRAW_MODE_ROOMNUM:
			PasteRoomNumber( iMapIndex, 0 );
			break;
		case DRAW_MODE_ROADS:
			RemoveAllObjectsOfTypeRange( iMapIndex, ROADPIECES, ROADPIECES );
			break;
		default:
			//DeleteStuffFromMapTile( iMapIndex );
			break;
	}
}


//---------------------------------------------------------------------------------------------------------------
//	PasteDebris
//
//	Place some "debris" on the map at the current mouse coordinates. This function is called repeatedly if
//	the current brush size is larger than 1 tile.
//
void PasteDebris( UINT32 iMapIndex )
{
	UINT16				usUseIndex;
	UINT16				usUseObjIndex;
	INT32					iRandSelIndex;

	// Get selection list for debris
	pSelList = SelDebris;
	pNumSelList = &iNumDebrisSelected;

	if ( iMapIndex < GRIDSIZE )
	{
		AddToUndoList( iMapIndex );

		// Remove any debris that is currently at this map location
		if ( gpWorldLevelData[ iMapIndex ].pObjectHead != NULL )
		{
			RemoveAllObjectsOfTypeRange( iMapIndex, ANOTHERDEBRIS, FIRSTPOINTERS - 1 );
		}

		// Get a random debris from current selection
		iRandSelIndex = GetRandomSelection( );
		if ( iRandSelIndex != -1 )
		{
			// Add debris to the world
			usUseIndex = pSelList[ iRandSelIndex ].usIndex;
			usUseObjIndex = (UINT16)pSelList[ iRandSelIndex ].uiObject;

			AddObjectToTail( iMapIndex, (UINT16)(gTileTypeStartIndex[ usUseObjIndex ] + usUseIndex) );
		}
	}
}


static void PasteSingleWallCommon(UINT32 map_idx, Selections* sel_list, INT32& n_sel_list);


void PasteSingleWall( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleWall, iNumWallsSelected);
}

void PasteSingleDoor( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleDoor, iNumDoorsSelected);
}

void PasteSingleWindow( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleWindow, iNumWindowsSelected);
}

void PasteSingleRoof( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleRoof, iNumRoofsSelected);
}

void PasteRoomNumber( UINT32 iMapIndex, UINT8 ubRoomNumber )
{
	if( iMapIndex < lengthof(gubWorldRoomInfo) && gubWorldRoomInfo[ iMapIndex ] != ubRoomNumber )
	{
		AddToUndoList( iMapIndex );
		gubWorldRoomInfo[ iMapIndex ] = ubRoomNumber;
	}
}


void PasteSingleBrokenWall( UINT32 iMapIndex )
{
	Selections* const sel_list = SelSingleBrokenWall;
	UINT16 usIndex           = sel_list[iCurBank].usIndex;
	UINT16 usObjIndex        = sel_list[iCurBank].uiObject;
	UINT16 usTileIndex       = GetTileIndexFromTypeSubIndex(usObjIndex, usIndex);
	UINT16 usWallOrientation = GetWallOrientation(usTileIndex);
	if( usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == INSIDE_TOP_RIGHT )
		EraseHorizontalWall( iMapIndex );
	else
		EraseVerticalWall( iMapIndex );

	PasteSingleWallCommon(iMapIndex, sel_list, iNumBrokenWallsSelected);
}

void PasteSingleDecoration( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleDecor, iNumDecorSelected);
}

void PasteSingleDecal( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleDecal, iNumDecalsSelected);
}

void PasteSingleFloor( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleFloor, iNumFloorsSelected);
}

void PasteSingleToilet( UINT32 iMapIndex )
{
	PasteSingleWallCommon(iMapIndex, SelSingleToilet, iNumToiletsSelected);
}


/* Common paste routine for PasteSingleWall, PasteSingleDoor,
 * PasteSingleDecoration, and PasteSingleDecor (above). */
static void PasteSingleWallCommon(UINT32 const map_idx, Selections* const sel_list, INT32& n_sel_list)
try
{
	pSelList    = sel_list;
	pNumSelList = &n_sel_list;

	if (map_idx >= GRIDSIZE) return;

	AddToUndoList(map_idx);

	UINT16 const use_idx     = sel_list[iCurBank].usIndex;
	UINT16 const use_obj_idx = sel_list[iCurBank].uiObject;
	UINT16 const idx         = gTileTypeStartIndex[use_obj_idx] + use_idx;

	// Temp stuff for onroof things
	if (FIRSTONROOF <= use_obj_idx && use_obj_idx <= SECONDONROOF)
	{
		// Add to onroof section
		AddOnRoofToTail(map_idx, idx);

		INT16 const buddy_num = gTileDatabase[idx].sBuddyNum;
		if (buddy_num != -1) AddOnRoofToTail(map_idx, buddy_num);
		return;
	}

	// Make sure A-frames are on roof level
	if (WALL_AFRAME_START <= use_idx && use_idx <= WALL_AFRAME_END)
	{
		AddRoofToTail(map_idx, idx);
		return;
	}

	if (FIRSTDOOR <= use_obj_idx && use_obj_idx <= LASTDOOR)
	{
		// Place shadow for doors
		if (!gfBasement)
			AddExclusiveShadow(map_idx, gTileTypeStartIndex[use_obj_idx - FIRSTDOOR + FIRSTDOORSHADOW] + use_idx);
	}

	// Is it a wall?
	if (FIRSTWALL <= use_obj_idx && use_obj_idx <= LASTWALL)
	{
		// ATE: If it is a wall shadow, place differenty!
		if (use_idx == 29 || use_idx == 30)
		{
			if (!gfBasement) AddExclusiveShadow(map_idx, idx);
		}
		else
		{ // Slap down wall/window/door/decoration (no smoothing)
			AddWallToStructLayer(map_idx, idx, TRUE);
		}
	}
	else if ((FIRSTDOOR <= use_obj_idx && use_obj_idx <= LASTDOOR) ||
			(FIRSTDECORATIONS <= use_obj_idx && use_obj_idx <= LASTDECORATIONS))
	{ // Slap down wall/window/door/decoration (no smoothing)
		AddWallToStructLayer(map_idx, idx, TRUE);
	}
	else if ((FIRSTROOF <= use_obj_idx && use_obj_idx <= LASTROOF) ||
			(FIRSTSLANTROOF <= use_obj_idx && use_obj_idx <= LASTSLANTROOF))
	{ // Put a roof on this tile (even if nothing else is here)
		RemoveAllRoofsOfTypeRange(map_idx, FIRSTROOF, LASTROOF);
		AddRoofToTail(map_idx, idx);
	}
	else if (FIRSTFLOOR <= use_obj_idx && use_obj_idx <= LASTFLOOR)
	{ // Drop a floor on this tile
		if (LEVELNODE const* const land = FindTypeInLandLayer(map_idx, use_obj_idx))
		{
			RemoveLand(map_idx, land->usIndex);
		}
		AddLandToHead(map_idx, idx);
	}
	else if ((FIRSTWALLDECAL <= use_obj_idx && use_obj_idx <= LASTWALLDECAL) ||
			((FIFTHWALLDECAL <= use_obj_idx && use_obj_idx <= EIGTHWALLDECAL)))
	{ // Plop a decal here
		RemoveAllStructsOfTypeRange(map_idx, FIRSTWALLDECAL, LASTWALLDECAL);
		RemoveAllStructsOfTypeRange(map_idx, FIFTHWALLDECAL, EIGTHWALLDECAL);
		AddStructToTail(map_idx, idx);
	}
	else if ((FIRSTISTRUCT <= use_obj_idx && use_obj_idx <= LASTISTRUCT) ||
			((FIFTHISTRUCT <= use_obj_idx && use_obj_idx <= EIGHTISTRUCT)))
	{
		AddStructToHead(map_idx, idx);
	}
	else if (use_obj_idx == FIRSTSWITCHES)
	{
		AddStructToTail(map_idx, idx);
	}
}
catch (FailedToAddNode const&) { /* XXX TODO0021 ignore */ }


//---------------------------------------------------------------------------------------------------------------
//	GetRandomIndexByRange
//
//	Returns a randomly picked object index given the current selection list, and the type or types of objects we want
//	from that list. If no such objects are in the list, we return 0xffff (-1).
UINT16 GetRandomIndexByRange( UINT16 usRangeStart, UINT16 usRangeEnd )
{
	UINT16	usPickList[50];
	UINT16	usNumInPickList;
	UINT16	usWhich;
	UINT16	usObject;
	// Get a list of valid object to select from
	usNumInPickList = 0;
	for ( usWhich = 0; usWhich < *pNumSelList; usWhich++ )
	{
		usObject = (UINT16)pSelList[ usWhich ].uiObject;
		if ( (usObject >= usRangeStart) && (usObject <= usRangeEnd) )
		{
			usPickList[ usNumInPickList ] = usObject;
			usNumInPickList++;
		}
	}
	return ( usNumInPickList ) ? usPickList[ Random(usNumInPickList) ] : 0xffff;
}


static void PasteStructureCommon(UINT32 iMapIndex);


//---------------------------------------------------------------------------------------------------------------
//	PasteStructure			(See also PasteStructure1, PasteStructure2, and PasteStructureCommon)
//
//	Puts a structure (trees, trucks, etc.) into the world
//
void PasteStructure( UINT32 iMapIndex )
{
	pSelList = SelOStructs;
	pNumSelList = &iNumOStructsSelected;

	PasteStructureCommon( iMapIndex );
}

//---------------------------------------------------------------------------------------------------------------
//	PasteStructure1			(See also PasteStructure, PasteStructure2, and PasteStructureCommon)
//
//	Puts a structure (trees, trucks, etc.) into the world
//
void PasteStructure1( UINT32 iMapIndex )
{
	pSelList = SelOStructs1;
	pNumSelList = &iNumOStructs1Selected;

	PasteStructureCommon( iMapIndex );
}

//---------------------------------------------------------------------------------------------------------------
//	PasteStructure2			(See also PasteStructure, PasteStructure1, and PasteStructureCommon)
//
//	Puts a structure (trees, trucks, etc.) into the world
//
void PasteStructure2( UINT32 iMapIndex )
{
	pSelList = SelOStructs2;
	pNumSelList = &iNumOStructs2Selected;

	PasteStructureCommon( iMapIndex );
}


//	This is the main (common) structure pasting function. The above three wrappers are only required because they
//	each use different selection lists. Other than that, they are COMPLETELY identical.
static void PasteStructureCommon(const UINT32 iMapIndex)
{
	if (iMapIndex >= GRIDSIZE) return;

	const INT32 iRandSelIndex = GetRandomSelection();
	if (iRandSelIndex == -1) return;

	AddToUndoList(iMapIndex);

	const UINT16 usUseIndex    = pSelList[iRandSelIndex].usIndex;
	const UINT16 usUseObjIndex = pSelList[iRandSelIndex].uiObject;

	// Check with Structure Database (aka ODB) if we can put the object here!
	const DB_STRUCTURE_REF* const sr = gTileDatabase[gTileTypeStartIndex[usUseObjIndex] + usUseIndex].pDBStructureRef;
	if (!OkayToAddStructureToWorld(iMapIndex, 0, sr, INVALID_STRUCTURE_ID) && sr != NULL) return;

	// Actual structure info is added by the functions below
	AddStructToHead(iMapIndex, gTileTypeStartIndex[usUseObjIndex] + usUseIndex);
}


//---------------------------------------------------------------------------------------------------------------
//	PasteBanks
//
//	Places a river bank or cliff into the world
//
void PasteBanks(UINT32 const iMapIndex, BOOLEAN const fReplace)
{
	BOOLEAN				fDoPaste = FALSE;
	UINT16				usUseIndex;
	UINT16				usUseObjIndex;

	pSelList = SelBanks;
	pNumSelList = &iNumBanksSelected;

	usUseIndex = pSelList[ iCurBank ].usIndex;
	usUseObjIndex = (UINT16)pSelList[ iCurBank ].uiObject;

	if ( iMapIndex < GRIDSIZE )
	{
		fDoPaste = TRUE;

		if (  gpWorldLevelData[ iMapIndex ].pStructHead != NULL )
		{
				// CHECK IF THE SAME TILE IS HERE
				if ( gpWorldLevelData[ iMapIndex ].pStructHead->usIndex == (UINT16)( gTileTypeStartIndex[ usUseObjIndex ] + usUseIndex ) )
				{
					fDoPaste = FALSE;
				}
		}
		else
		{
			// Nothing is here, paste
			fDoPaste = TRUE;
		}

		if ( fDoPaste )
		{
			AddToUndoList( iMapIndex );

			{
					if ( usUseObjIndex == FIRSTROAD )
					{
						AddObjectToHead( iMapIndex, (UINT16)( gTileTypeStartIndex[ usUseObjIndex ] + usUseIndex ) );
					}
					else
					{
						AddStructToHead( iMapIndex, (UINT16)( gTileTypeStartIndex[ usUseObjIndex ] + usUseIndex ) );
						// Add shadows
						if ( !gfBasement && usUseObjIndex == FIRSTCLIFF )
						{
							//AddShadowToHead( iMapIndex, (UINT16)( gTileTypeStartIndex[ usUseObjIndex - FIRSTCLIFF + FIRSTCLIFFSHADOW ] + usUseIndex ) );
							AddObjectToHead( iMapIndex, (UINT16)( gTileTypeStartIndex[ usUseObjIndex - FIRSTCLIFF + FIRSTCLIFFHANG ] + usUseIndex ) );
						}
					}
			}
		}
	}
}

void PasteRoads( UINT32 iMapIndex )
{
	UINT16				usUseIndex;

	pSelList = SelRoads;
	pNumSelList = &iNumRoadsSelected;

	usUseIndex = pSelList[ iCurBank ].usIndex;

	PlaceRoadMacroAtGridNo( iMapIndex, usUseIndex );
}

//---------------------------------------------------------------------------------------------------------------
//	PasteTexture
//
//	Puts a ground texture in the world. Ground textures are then "smoothed" in order to blend the edges with one
//	another. The current drawing brush also affects this function.
//
void PasteTexture( UINT32 iMapIndex )
{
	ChooseWeightedTerrainTile(); //Kris
	PasteTextureCommon( iMapIndex );
}


static void PasteHigherTexture(UINT32 iMapIndex, UINT32 fNewType);
static void PasteTextureEx(GridNo, UINT16 type);


/* PasteTexture() calls this one to actually put a ground tile down. If the
 * brush size is larger than one tile, then the above function will call this
 * one and indicate that they should all be placed into the undo stack as the
 * same undo command. */
void PasteTextureCommon(UINT32 const map_idx)
{
	UINT16 const paste = CurrentPaste;
	if (paste   == NO_TILE) return;
	if (map_idx >= GRIDSIZE)  return;

	// Set undo, then set new
	AddToUndoList(map_idx);

	if (paste == DEEPWATERTEXTURE)
	{ // If we are pasting deep water and we are not over water, ignore!
		LEVELNODE const* const land = FindTypeInLandLayer(map_idx, REGWATERTEXTURE);
		if (!land || !gTileDatabase[land->usIndex].ubFullTile) return;
	}

	// Don't draw over floors
	if (TypeRangeExistsInLandLayer(map_idx, FIRSTFLOOR, FOURTHFLOOR)) return;

	// Compare heights and do appropriate action
	UINT8 last_high_level;
	if (AnyHeigherLand(map_idx, paste, &last_high_level))
	{
		/* Here we do the following:
		 * - Remove old type from layer
		 * - Smooth World with old type
		 * - Add a 3 by 3 square of new type at head
		 * - Smooth World with new type */
		PasteHigherTexture(map_idx, paste);
	}
	else
	{
		PasteTextureEx(map_idx, paste);
		SmoothTerrainRadius(map_idx, paste, 1, TRUE);
	}
}


static BOOLEAN SetLowerLandIndexWithRadius(INT32 iMapIndex, UINT32 uiNewType, UINT8 ubRadius, BOOLEAN fReplace);


//	Some ground textures should be placed "above" others. That is, grass needs to be placed "above" sand etc.
//	This function performs the appropriate actions.
static void PasteHigherTexture(UINT32 iMapIndex, UINT32 fNewType)
{
	UINT8  ubLastHighLevel;
	std::vector<UINT32> deletedTypes;

	// Here we do the following:
	// - Remove old type from layer
	// - Smooth World with old type
	// - Add a 3 by 3 square of new type at head
	// - Smooth World with new type

	//if (iMapIndex < GRIDSIZE && TypeRangeExistsInLandLayer(iMapIndex, FIRSTFLOOR, LASTFLOOR))
	//ATE: DONOT DO THIS!!!!!!! - I know what was intended - not to draw over floors - this
	// I don't know is the right way to do it!
		//return;


	if ( iMapIndex < GRIDSIZE && AnyHeigherLand( iMapIndex, fNewType, &ubLastHighLevel ))
	{
		AddToUndoList( iMapIndex );

		// - For all heigher level, remove
		RemoveHigherLandLevels(iMapIndex, fNewType, deletedTypes);

		// Set with a radius of 1 and smooth according to height difference
		SetLowerLandIndexWithRadius( iMapIndex, fNewType, 1 , TRUE );

		// Smooth all deleted levels
		for (UINT32 deletedType : deletedTypes)
		{
			SmoothTerrainRadius( iMapIndex, deletedType, 1, TRUE );
		}
	}
	else if ( iMapIndex < GRIDSIZE )
	{
		AddToUndoList( iMapIndex );

		UINT16 NewTile = GetTileIndexFromTypeSubIndex(fNewType, REQUIRES_SMOOTHING_TILE);
		SetLandIndex(iMapIndex, NewTile, fNewType);

		// Smooth item then adding here
		SmoothTerrain( iMapIndex, fNewType, &NewTile, FALSE );

		if ( NewTile != NO_TILE )
		{
			// Change tile
			SetLandIndex(iMapIndex, NewTile, fNewType);
		}
	}
}


static BOOLEAN PasteExistingTexture(UINT32 iMapIndex, UINT16 usIndex)
{
	UINT16					usNewIndex;

	// If here, we want to make, esentially, what is a type in
	// a level other than TOP-MOST the TOP-MOST level.
	// We should:
	// - remove what was top-most
	// - re-adjust the world to reflect missing top-most peice

	if ( iMapIndex >= GRIDSIZE )
		return ( FALSE );

	//if (TypeRangeExistsInLandLayer(iMapIndex, FIRSTFLOOR, LASTFLOOR))
	//	return( FALSE );

	// Get top tile index
	// Remove all land peices except
	const UINT32 uiNewType = GetTileType(usIndex);

	DeleteAllLandLayers( iMapIndex );

	// ADD BASE LAND AT LEAST!
	usNewIndex = (UINT16)Random(10);

	// Adjust for type
	usNewIndex += gTileTypeStartIndex[ gCurrentBackground ];

	// Set land index
	AddLandToHead( iMapIndex, usNewIndex );

	SetLandIndex(iMapIndex, usIndex, uiNewType);

	// ATE: Set this land peice to require smoothing again!
	SmoothAllTerrainTypeRadius( iMapIndex, 2, TRUE );

	return( TRUE );
}


//	Puts a land index "under" an existing ground texture. Affects a radial area.
static BOOLEAN SetLowerLandIndexWithRadius(INT32 iMapIndex, UINT32 uiNewType, UINT8 ubRadius, BOOLEAN fReplace)
{
	INT16   sTop, sBottom;
	INT16   sLeft, sRight;
	INT16   cnt1, cnt2;
	INT32   iNewIndex;
	BOOLEAN fDoPaste = FALSE;
	INT32   leftmost;
	UINT8   ubLastHighLevel;
	std::vector<UINT32> smoothTiles;
	UINT16  usTemp;

	// Determine start end end indicies and num rows
	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	if ( iMapIndex >= GRIDSIZE )
		return ( FALSE );

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{

		leftmost = ( ( iMapIndex + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			iNewIndex = iMapIndex + ( WORLD_COLS * cnt1 ) + cnt2;

			if ( iNewIndex >=0 && iNewIndex < WORLD_MAX &&
				iNewIndex >= leftmost && iNewIndex < ( leftmost + WORLD_COLS ) )
			{

				if ( fReplace )
				{
					fDoPaste = TRUE;
				}
				else
				{
					if (FindTypeInLandLayer(iNewIndex, uiNewType))
					{
						fDoPaste = TRUE;
					}
				}

				//if (fDoPaste && !TypeRangeExistsInLandLayer(iMapIndex, FIRSTFLOOR, LASTFLOOR))
				if ( fDoPaste  )
				{
					if ( iMapIndex == iNewIndex )
					{
						AddToUndoList( iMapIndex );

						// Force middle one to NOT smooth, and set to random 'full' tile
						usTemp = Random(10) + 1;
						UINT16 NewTile = GetTileIndexFromTypeSubIndex(uiNewType, usTemp);
						SetLandIndex(iNewIndex, NewTile, uiNewType);
					}
					else if ( AnyHeigherLand( iNewIndex, uiNewType, &ubLastHighLevel ) )
					{
						AddToUndoList( iMapIndex );

						// Force middle one to NOT smooth, and set to random 'full' tile
						usTemp = Random(10) + 1;
						UINT16 NewTile = GetTileIndexFromTypeSubIndex(uiNewType, usTemp);
						SetLandIndex(iNewIndex, NewTile, uiNewType);
					}
					else
					{
						AddToUndoList( iMapIndex );

						// Set tile to 'smooth target' tile
						UINT16 NewTile = GetTileIndexFromTypeSubIndex(uiNewType, REQUIRES_SMOOTHING_TILE);
						SetLandIndex(iNewIndex, NewTile, uiNewType);

						// If we are top-most, add to smooth list
						smoothTiles.push_back(iNewIndex);
					}
				}
			}
		}
	}

	// Once here, smooth any tiles that need it
	for (UINT32 smoothTile : smoothTiles)
	{
		SmoothTerrainRadius(smoothTile, uiNewType, 10, FALSE);
	}

	return( TRUE );
}


// ATE FIXES
static void PasteTextureEx(GridNo const grid_no, UINT16 const type)
{
	// Check if this texture exists
	if (LEVELNODE const* const land = FindTypeInLandLayer(grid_no, type))
	{
		UINT8 type_level;
		if (GetTypeLandLevel(grid_no, type, &type_level))
		{
			// If top-land, do not change
			if (type_level != LANDHEAD)
			{
				PasteExistingTexture(grid_no, land->usIndex);
			}
		}
	}
	else
	{
		// Fill with just first tile, smoothworld() will pick proper piece later
		UINT16 const new_tile = GetTileIndexFromTypeSubIndex(type, REQUIRES_SMOOTHING_TILE);
		SetLandIndex(grid_no, new_tile, type);
	}
}


// FUNCTION TO GIVE NEAREST GRIDNO OF A CLIFF
#define LAND_DROP_1 FIRSTCLIFF1
#define LAND_DROP_2 FIRSTCLIFF11
#define LAND_DROP_3 FIRSTCLIFF12
#define LAND_DROP_4 FIRSTCLIFF15
#define LAND_DROP_5 FIRSTCLIFF8
void RaiseWorldLand( )
{
	INT32					cnt;
	UINT32				sTempGridNo;
	LEVELNODE			*pStruct;
	TILE_ELEMENT	*pTileElement;
	BOOLEAN fRaiseSet;
	BOOLEAN fSomethingRaised = FALSE;
	UINT8 ubLoop;
	INT32 iStartNumberOfRaises = 0;
	INT32 iNumberOfRaises = 0;
	BOOLEAN fAboutToRaise = FALSE;

	fRaiseSet=FALSE;

	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags &= ~MAPELEMENT_RAISE_LAND_START;
		i->uiFlags &= ~MAPELEMENT_RAISE_LAND_END;
	}

	UINT16 usIndex = (UINT16)-1; // XXX HACK000E
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{

		// Get Structure levelnode
		pStruct = gpWorldLevelData[ cnt ].pStructHead;
		gpWorldLevelData[cnt].sHeight=0;

		while( pStruct )
		{
			pTileElement = &(gTileDatabase[ pStruct->usIndex ]);
			if (pTileElement->fType==FIRSTCLIFF)
			{
				fSomethingRaised = TRUE;
				SLOGD("Cliff found at count={}", cnt);
				if( pTileElement->ubNumberOfTiles > 1 )
				{
					SLOGD("Cliff has {} children", pTileElement->ubNumberOfTiles);
					for (ubLoop = 0; ubLoop < pTileElement->ubNumberOfTiles; ubLoop++)
					{
						usIndex=pStruct->usIndex;
						// need means to turn land raising on and off based on the tile ID and the offset in the
						// tile database when reading into the mapsystem
						// turning off of land raising can only be done
						// presently by CLIFF object/tileset 1
						// so simply detect this tile set and turn off instead of on
						// element 1 is 12 tiles and is unique

						sTempGridNo = cnt + pTileElement->pTileLocData[ubLoop].bTileOffsetX + pTileElement->pTileLocData[ubLoop].bTileOffsetY * WORLD_COLS;
						// Check for valid gridno
						if ( OutOfBounds( (INT16)cnt, (INT16)sTempGridNo ) )
						{
							continue;
						}
						//if (pTileElement->ubNumberOfTiles==10)
						if((usIndex==LAND_DROP_1)||(usIndex==LAND_DROP_2) ||(usIndex==LAND_DROP_4))
						{
							gpWorldLevelData[sTempGridNo].uiFlags &= (~MAPELEMENT_RAISE_LAND_START );
							gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_END;
						}
						else if( ( usIndex==LAND_DROP_5 ) && ( ubLoop == 4 ) )
						{
							gpWorldLevelData[sTempGridNo].uiFlags &= (~MAPELEMENT_RAISE_LAND_START );
							gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_END;
							if( !( gpWorldLevelData[sTempGridNo + 1 ].uiFlags & MAPELEMENT_RAISE_LAND_START ) )
							{
								gpWorldLevelData[sTempGridNo + 1].uiFlags |= MAPELEMENT_RAISE_LAND_START;
							}
						}
						else if( ( usIndex== LAND_DROP_3 ) && ( ( ubLoop == 0 ) || ( ubLoop == 1 ) || ( ubLoop == 2 )) )
						{
							gpWorldLevelData[sTempGridNo].uiFlags &= (~MAPELEMENT_RAISE_LAND_START );
							gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_END;
						}
						else
						{
							gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_START;
						}
					}
				}
				else
				{
					if( usIndex==LAND_DROP_3 )
					{
						gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RAISE_LAND_START );
						gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RAISE_LAND_END;
					}
					else
					{
						//if (pTileElement->ubNumberOfTiles==10)
						if(usIndex==LAND_DROP_1)
						{
							gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RAISE_LAND_START );
							gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RAISE_LAND_END;
						}
						else
							gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RAISE_LAND_START;
					}
				}
			}
			pStruct=pStruct->pNext;
		}
	}

	// no cliffs?
	if (!fSomethingRaised) return;

	// run through again, this pass is for placing raiselandstart in rows that have raiseland end but no raiselandstart
	for (cnt=WORLD_MAX-1; cnt >=0 ; cnt--)
	{
		if(cnt%WORLD_ROWS==WORLD_ROWS-1)
		{
		// start of new row
		fRaiseSet=FALSE;
		}
		if (gpWorldLevelData[cnt].uiFlags&MAPELEMENT_RAISE_LAND_START)
		{
			fRaiseSet=TRUE;
		}
		else if((gpWorldLevelData[cnt].uiFlags&MAPELEMENT_RAISE_LAND_END)&&(!fRaiseSet))
		{
			// there is a dropoff without a rise.
			// back up and set beginning to raiseland start
			gpWorldLevelData[cnt+((WORLD_ROWS-1)-(cnt%WORLD_ROWS))].uiFlags &= (~MAPELEMENT_RAISE_LAND_END );
			gpWorldLevelData[cnt+((WORLD_ROWS-1)-(cnt%WORLD_ROWS))].uiFlags|=MAPELEMENT_RAISE_LAND_START;
			if(cnt+((WORLD_ROWS-1)-(cnt%WORLD_ROWS))-WORLD_ROWS >0)
			{
				gpWorldLevelData[cnt+((WORLD_ROWS-1)-(cnt%WORLD_ROWS))-WORLD_ROWS].uiFlags &= (~MAPELEMENT_RAISE_LAND_END );
				gpWorldLevelData[cnt+((WORLD_ROWS-1)-(cnt%WORLD_ROWS))-WORLD_ROWS].uiFlags|=MAPELEMENT_RAISE_LAND_START;
			}
			fRaiseSet=TRUE;
		}
	}
	fRaiseSet=FALSE;
	// Look for a cliff face that is along either the lower edge or the right edge of the map, this is used for a special case fill
	// start at y=159, x= 80 and go to x=159, y=80

	// now check along x=159, y=80 to x=80, y=0
	for (cnt=( ( WORLD_COLS * WORLD_ROWS ) - ( WORLD_ROWS / 2 ) * ( WORLD_ROWS - 2 ) - 1 ); cnt >WORLD_ROWS-1 ; cnt-=(WORLD_ROWS+1))
	{
		if (fAboutToRaise)
		{
			fRaiseSet=TRUE;
			fAboutToRaise = FALSE;
		}

		if ((gpWorldLevelData[cnt].uiFlags&MAPELEMENT_RAISE_LAND_START)||(gpWorldLevelData[cnt-1].uiFlags&MAPELEMENT_RAISE_LAND_START)||(gpWorldLevelData[ cnt + 1 ].uiFlags&MAPELEMENT_RAISE_LAND_START))
		{
			fAboutToRaise = TRUE;
			fRaiseSet = FALSE;
		}
		else if((gpWorldLevelData[cnt].uiFlags&MAPELEMENT_RAISE_LAND_END)||(gpWorldLevelData[cnt-1].uiFlags&MAPELEMENT_RAISE_LAND_END) || ( gpWorldLevelData[cnt+1].uiFlags&MAPELEMENT_RAISE_LAND_END) )
		{
			fRaiseSet=FALSE;
		}
		if (fRaiseSet)
		{
			gpWorldLevelData[cnt+((WORLD_ROWS-1)-(cnt%WORLD_ROWS))].uiFlags|=MAPELEMENT_RAISE_LAND_START;
			//gpWorldLevelData[cnt].uiFlags|=MAPELEMENT_RAISE_LAND_START;
			//gpWorldLevelData[cnt-1].uiFlags|=MAPELEMENT_RAISE_LAND_START;
			SLOGD("Land Raise start at count: {} is raised (maybe count : {})", cnt, cnt - 1);
		}
	}

	//fRaiseSet = FALSE;

	// Now go through the world, starting at x=max(x) and y=max(y) and work backwards
	// if a cliff is found turn raise flag on, if the end of a screen is found, turn off
	// the system uses world_cord=x+y*(row_size)


	for (cnt=WORLD_MAX-1; cnt >= 0;  cnt--)
	{
		//  reset the RAISE to FALSE
		// End of the row
		if ( !(cnt % WORLD_ROWS) )
		{
			iNumberOfRaises = 0;
			iStartNumberOfRaises = 0;
		}

		if( ( gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_END ) )
		{
			if( cnt > 1 )
			{
				if( ( !( gpWorldLevelData[ cnt - 1 ].uiFlags & MAPELEMENT_RAISE_LAND_END) && !( gpWorldLevelData[ cnt - 2 ].uiFlags & MAPELEMENT_RAISE_LAND_END ) ) )
				{
					iNumberOfRaises--;
				}
			}
		}
		else if( gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_START )
		{
			// check tile before and after, if either are raise land flagged, then don't increment number of
			// raises
			if( cnt < WORLD_MAX - 2 )
			{
				if( ( !( gpWorldLevelData[ cnt + 1 ].uiFlags & MAPELEMENT_RAISE_LAND_START) && !( gpWorldLevelData[ cnt + 2 ].uiFlags & MAPELEMENT_RAISE_LAND_START ) )  )
				{
					iNumberOfRaises++;
				}
			}
		}

		// look at number of raises.. if negative, then we have more downs than ups, restart row with raises + 1;
		// now raise land of any tile while the flag is on
		if( iNumberOfRaises < 0 )
		{
			// something wrong, reset cnt
			iStartNumberOfRaises++;
			cnt += WORLD_ROWS - cnt % WORLD_ROWS;
			iNumberOfRaises = iStartNumberOfRaises;
			continue;
		}

		if( iNumberOfRaises >= 0 )
		{
			SLOGD("Land Raise start at count: {} is raised", cnt);
			gpWorldLevelData[cnt].sHeight=iNumberOfRaises * WORLD_CLIFF_HEIGHT;
		}
	}

	for (cnt=WORLD_MAX-1; cnt >= 0;  cnt--)
	{
		if( ( cnt < WORLD_MAX - WORLD_ROWS ) && ( cnt > WORLD_ROWS ) )
		{
			if( ( gpWorldLevelData[cnt + WORLD_ROWS ].sHeight == gpWorldLevelData[cnt - WORLD_ROWS ].sHeight ) && ( gpWorldLevelData[ cnt  ].sHeight!= gpWorldLevelData[cnt - WORLD_ROWS ].sHeight ) )
			{
				gpWorldLevelData[cnt].sHeight = gpWorldLevelData[cnt + WORLD_ROWS ].sHeight;
			}
			else if( ( gpWorldLevelData[ cnt ].sHeight > gpWorldLevelData[cnt - WORLD_ROWS ].sHeight )&& ( gpWorldLevelData[cnt + WORLD_ROWS ].sHeight != gpWorldLevelData[cnt - WORLD_ROWS ].sHeight ) && ( gpWorldLevelData[cnt ].sHeight > gpWorldLevelData[cnt + WORLD_ROWS ].sHeight ) )
			{
				if( gpWorldLevelData[cnt - WORLD_ROWS ].sHeight > gpWorldLevelData[cnt + WORLD_ROWS ].sHeight )
				{
					gpWorldLevelData[ cnt ].sHeight = gpWorldLevelData[cnt - WORLD_ROWS ].sHeight;
				}
				else
				{
					gpWorldLevelData[ cnt ].sHeight = gpWorldLevelData[cnt + WORLD_ROWS ].sHeight;
				}
			}
		}
	}

//*/

}

