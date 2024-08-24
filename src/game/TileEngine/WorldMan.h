#ifndef __WORLDMAN_H_
#define __WORLDMAN_H_

#include "JA2Types.h"
#include "Types.h"
#include "WorldDef.h"

#include <exception>
#include <vector>


class FailedToAddNode : public std::exception
{
	public:
		virtual char const* what() const throw()
		{
			return "Failed to add node to world";
		}
};


// Object manipulation functions
BOOLEAN RemoveObject( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE *AddObjectToTail( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE* AddObjectToHead(UINT32 iMapIndex, UINT16 usIndex);
LEVELNODE* FindTypeInObjectLayer(UINT32 map_idx, UINT32 type);
BOOLEAN RemoveAllObjectsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
LEVELNODE* TypeRangeExistsInObjectLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType);


// Roof manipulation functions
BOOLEAN RemoveRoof( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE  *AddRoofToTail( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE* AddRoofToHead(UINT32 iMapIndex, UINT16 usIndex);
LEVELNODE* FindTypeInRoofLayer(UINT32 map_idx, UINT32 type);
BOOLEAN RemoveAllRoofsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
void RemoveRoofIndexFlagsFromTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, LevelnodeFlags);
void SetRoofIndexFlagsFromTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, LevelnodeFlags);
LEVELNODE* TypeRangeExistsInRoofLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType);
void SetWallLevelnodeFlags(UINT16 sGridNo, LevelnodeFlags);
void RemoveWallLevelnodeFlags(UINT16 sGridNo, LevelnodeFlags);
BOOLEAN IndexExistsInRoofLayer( INT16 sGridNo, UINT16 usIndex );


// OnRoof manipulation functions
BOOLEAN RemoveOnRoof( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE *AddOnRoofToTail( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE* AddOnRoofToHead(UINT32 iMapIndex, UINT16 usIndex);
BOOLEAN RemoveAllOnRoofsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
BOOLEAN RemoveOnRoofFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode );


// Land manipulation functions
void       RemoveLand(UINT32 map_idx, UINT16 idx);
LEVELNODE *AddLandToTail( UINT32 iMapIndex, UINT16 usIndex );
void AddLandToHead(UINT32 iMapIndex, UINT16 usIndex);
LEVELNODE* FindTypeInLandLayer(UINT32 map_idx, UINT32 type);
BOOLEAN RemoveAllLandsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
LEVELNODE * TypeRangeExistsInLandLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType);
void ReplaceLandIndex(UINT32 iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex);
void DeleteAllLandLayers(UINT32 iMapIndex);
void InsertLandIndexAtLevel(UINT32 iMapIndex, UINT16 usIndex, UINT8 ubLevel);
void RemoveHigherLandLevels(UINT32 map_idx, UINT32 src_type, std::vector<UINT32>& higher_types);

UINT8 GetTerrainType(GridNo);
bool  Water(GridNo);
bool  DeepWater(GridNo);
bool  WaterTooDeepForAttacks(GridNo);

// Structure manipulation routines
void       RemoveStruct(UINT32 map_idx, UINT16 usIndex);
LEVELNODE *AddStructToTail( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE *ForceStructToTail( UINT32 iMapIndex, UINT16 usIndex );

void    AddStructToHead(UINT32 iMapIndex, UINT16 usIndex);
BOOLEAN RemoveAllStructsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
bool    AddWallToStructLayer(INT32 map_idx, UINT16 idx, bool replace);
BOOLEAN ReplaceStructIndex( UINT32 iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex );
void    HideStructOfGivenType(UINT32 iMapIndex, UINT32 fType, BOOLEAN fHide);
void    SetStructAframeFlags(UINT32 iMapIndex, LevelnodeFlags);
void    RemoveStructFromLevelNode(UINT32 map_idx, LEVELNODE*);
BOOLEAN IndexExistsInStructLayer(GridNo, UINT16 tile_index);

void ForceRemoveStructFromTail(UINT32 iMapIndex);

// Shadow manipulation routines
void AddShadowToTail(UINT32 iMapIndex, UINT16 usIndex);
LEVELNODE* AddShadowToHead(UINT32 iMapIndex, UINT16 usIndex);
void AddExclusiveShadow( UINT32 iMapIndex, UINT16 usIndex );
BOOLEAN RemoveAllShadowsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
BOOLEAN	RemoveAllShadows( UINT32 iMapIndex );
BOOLEAN RemoveShadowFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode );


// Merc manipulation routines
// #################################################################

LEVELNODE* AddMercToHead(UINT32 iMapIndex, SOLDIERTYPE&, BOOLEAN fAddStructInfo);
void         RemoveMerc(UINT32 map_idx, SOLDIERTYPE&, bool placeholder);
SOLDIERTYPE* WhoIsThere2(INT16 sGridNo, INT8 bLevel);
BOOLEAN AddMercStructureInfoFromAnimSurface( INT16 sGridNo, SOLDIERTYPE *pSoldier, UINT16 usAnimSurface, UINT16 usAnimState );
BOOLEAN UpdateMercStructureInfo( SOLDIERTYPE *pSoldier );
BOOLEAN OKToAddMercToWorld( SOLDIERTYPE *pSoldier, INT8 bDirection );


// TOPMOST manipulation functions
LEVELNODE  *AddTopmostToTail( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE* AddTopmostToHead(UINT32 iMapIndex, UINT16 usIndex);
BOOLEAN RemoveTopmost( UINT32 iMapIndex, UINT16 usIndex );
LEVELNODE* FindTypeInTopmostLayer(UINT32 map_idx, UINT32 type);
BOOLEAN RemoveAllTopmostsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType );
LEVELNODE* AddUIElem(UINT32 iMapIndex, UINT16 usIndex, INT8 sRelativeX, INT8 sRelativeY);
BOOLEAN RemoveTopmostFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode );


BOOLEAN IsHeigherLevel( INT16 sGridNo );
BOOLEAN IsRoofVisible( INT16 sMapPos );
BOOLEAN IsRoofVisible2( INT16 sMapPos );


LEVELNODE* FindLevelNodeBasedOnStructure(STRUCTURE const*);
LEVELNODE * FindShadow( INT16 sGridNo, UINT16 usStructIndex );

void WorldHideTrees(void);
void WorldShowTrees(void);


//this is found in editscreen.c
//Andrew, you had worldman.c checked out at the time, so I stuck it here.
//The best thing to do is toast it here, and include editscreen.h in worldman.c.
extern UINT32	gCurrentBackground;

void SetTreeTopStateForMap(void);

void DebugLevelNodePage(void);

#endif
