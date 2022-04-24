#include "Explosion_Control.h"

#include "AI.h"
#include "Action_Items.h"
#include "Animation_Control.h"
#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Debug.h"
#include "Directories.h"
#include "End_Game.h"
#include "FOV.h"
#include "FileMan.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "Handle_Doors.h"
#include "Handle_Items.h"
#include "Handle_UI.h"
#include "Interactive_Tiles.h"
#include "Interface.h"
#include "Interface_Dialogue.h"
#include "Isometric_Utils.h"
#include "Items.h"
#include "Keys.h"
#include "LightEffects.h"
#include "Lighting.h"
#include "LoadSaveData.h"
#include "LoadSaveExplosionType.h"
#include "Logger.h"
#include "Map_Information.h"
#include "MemMan.h"
#include "Message.h"
#include "Morale.h"
#include "OppList.h"
#include "Overhead.h"
#include "PathAI.h"
#include "Pits.h"
#include "Quests.h"
#include "Random.h"
#include "RenderWorld.h"
#include "Render_Fun.h"
#include "Rotting_Corpses.h"
#include "SamSiteModel.h"
#include "SaveLoadMap.h"
#include "Smell.h"
#include "SmokeEffects.h"
#include "Soldier_Add.h"
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Macros.h"
#include "Soldier_Profile.h"
#include "Soldier_Tile.h"
#include "Sound_Control.h"
#include "StrategicMap.h"
#include "Structure.h"
#include "Structure_Wrap.h"
#include "TileDat.h"
#include "TileDef.h"
#include "Tile_Animation.h"
#include "Timer_Control.h"
#include "Weapons.h"
#include "WorldDat.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "World_Items.h"

struct ExplosionInfo
{
	const char* blast_anim;
	SoundID     sound;
	UINT8       blast_speed;
	UINT8       transparent_key_frame;
	UINT8       damage_key_frame;
};

static const ExplosionInfo explosion_info[] =
{
	{ "",                           EXPLOSION_1,        0,  0,  0 },
	{ TILECACHEDIR "/zgrav_d.sti",  EXPLOSION_1,       80, 17,  3 },
	{ TILECACHEDIR "/zgrav_c.sti",  EXPLOSION_BLAST_2, 80, 28,  5 },
	{ TILECACHEDIR "/zgrav_b.sti",  EXPLOSION_BLAST_2, 80, 24,  5 },
	{ TILECACHEDIR "/shckwave.sti", EXPLOSION_1,       20,  1,  5 },
	{ TILECACHEDIR "/wat_exp.sti",  AIR_ESCAPING_1,    80,  1, 18 },
	{ TILECACHEDIR "/tear_exp.sti", AIR_ESCAPING_1,    80,  1, 18 },
	{ TILECACHEDIR "/tear_exp.sti", AIR_ESCAPING_1,    80,  1, 18 },
	{ TILECACHEDIR "/must_exp.sti", AIR_ESCAPING_1,    80,  1, 18 }
};


#define BOMB_QUEUE_DELAY (1000 + Random( 500 ) )

#define MAX_BOMB_QUEUE 40
static ExplosionQueueElement gExplosionQueue[MAX_BOMB_QUEUE];
UINT8                        gubElementsOnExplosionQueue = 0;
BOOLEAN                      gfExplosionQueueActive      = FALSE;

static BOOLEAN      gfExplosionQueueMayHaveChangedSight = FALSE;
static SOLDIERTYPE* gPersonToSetOffExplosions           = 0;

#define NUM_EXPLOSION_SLOTS 100
static EXPLOSIONTYPE gExplosionData[NUM_EXPLOSION_SLOTS];

Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT32, BOOLEAN_S*> BeforeStructureDamaged = {};
Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT8, BOOLEAN> OnStructureDamaged = {};

static EXPLOSIONTYPE* GetFreeExplosion(void)
{
	FOR_EACH(EXPLOSIONTYPE, e, gExplosionData)
	{
		if (!e->fAllocated) return e;
	}
	return NULL;
}


static void GenerateExplosionFromExplosionPointer(EXPLOSIONTYPE* pExplosion);


// GENERATE EXPLOSION
void InternalIgniteExplosion(SOLDIERTYPE* const owner, const INT16 sX, const INT16 sY, const INT16 sZ, const INT16 sGridNo, const UINT16 usItem, const BOOLEAN fLocate, const INT8 bLevel)
{
	// Double check that we are using an explosive!
	if ( !( GCM->getItem(usItem)->isExplosive() ) )
	{
		return;
	}

	// Increment attack counter...

	if (gubElementsOnExplosionQueue == 0)
	{
		// single explosion, disable sight until the end, and set flag
		// to check sight at end of attack

		gTacticalStatus.uiFlags |= (DISALLOW_SIGHT | CHECK_SIGHT_AT_END_OF_ATTACK);
	}


	gTacticalStatus.ubAttackBusyCount++;
	SLOGD("Incrementing Attack: Explosion gone off, Count now %d", gTacticalStatus.ubAttackBusyCount);

	EXPLOSIONTYPE* const e = GetFreeExplosion();
	if (e == NULL) return;

	e->owner         = owner;
	e->ubTypeID      = Explosive[GCM->getItem(usItem)->getClassIndex()].ubAnimationID;
	e->usItem        = usItem;
	e->sX            = sX;
	e->sY            = sY;
	e->sZ            = sZ;
	e->sGridNo       = sGridNo;
	e->bLevel        = bLevel;
	e->fAllocated    = TRUE;
	e->sCurrentFrame = 0;
	GenerateExplosionFromExplosionPointer(e);

	if (fLocate) LocateGridNo(sGridNo);
}


void IgniteExplosion(SOLDIERTYPE* const owner, const INT16 z, const INT16 sGridNo, const UINT16 item, const INT8 level)
{
	INT16 x;
	INT16 y;
	ConvertGridNoToCenterCellXY(sGridNo, &x, &y);
	InternalIgniteExplosion(owner, x, y, z, sGridNo, item, TRUE, level);
}


void IgniteExplosionXY(SOLDIERTYPE* const owner, const INT16 sX, const INT16 sY, const INT16 sZ, const INT16 sGridNo, const UINT16 usItem, const INT8 bLevel)
{
	InternalIgniteExplosion(owner, sX, sY, sZ, sGridNo, usItem, TRUE, bLevel);
}


static void GenerateExplosionFromExplosionPointer(EXPLOSIONTYPE* pExplosion)
{
	UINT8			ubTerrainType;

	ANITILE_PARAMS	AniParams;

	// Assign param values
	const INT16 sX      = pExplosion->sX;
	const INT16 sY      = pExplosion->sY;
	INT16       sZ      = pExplosion->sZ;
	const INT16 sGridNo = pExplosion->sGridNo;
	const INT16 bLevel  = pExplosion->bLevel;

	// If Z value given is 0 and bLevel > 0, make z heigher
	if ( sZ == 0 && bLevel > 0 )
	{
		sZ = ROOF_LEVEL_HEIGHT;
	}

	pExplosion->light = NULL;

	// OK, if we are over water.... use water explosion...
	ubTerrainType = GetTerrainType( sGridNo );

	const ExplosionInfo* inf = &explosion_info[pExplosion->ubTypeID];

	// Setup explosion!
	AniParams = ANITILE_PARAMS{};

	AniParams.sGridNo							= sGridNo;
	AniParams.ubLevelID						= ANI_TOPMOST_LEVEL;
	AniParams.sDelay              = inf->blast_speed;
	AniParams.sStartFrame					= pExplosion->sCurrentFrame;
	AniParams.uiFlags             = ANITILE_FORWARD | ANITILE_EXPLOSION;

	if ( ubTerrainType == LOW_WATER || ubTerrainType == MED_WATER || ubTerrainType == DEEP_WATER )
	{
		// Change type to water explosion...
		inf = &explosion_info[WATER_BLAST];
		AniParams.uiFlags						|= ANITILE_ALWAYS_TRANSLUCENT;
	}

	if ( sZ < WALL_HEIGHT )
	{
		AniParams.uiFlags |= ANITILE_NOZBLITTER;
	}

	AniParams.sX = sX;
	AniParams.sY = sY;
	AniParams.sZ = sZ;

	AniParams.ubKeyFrame1     = inf->transparent_key_frame;
	AniParams.uiKeyFrame1Code = ANI_KEYFRAME_BEGIN_TRANSLUCENCY;

	AniParams.ubKeyFrame2     = inf->damage_key_frame;
	AniParams.uiKeyFrame2Code = ANI_KEYFRAME_BEGIN_DAMAGE;
	AniParams.v.explosion     = pExplosion;

	AniParams.zCachedFile = inf->blast_anim;
	CreateAnimationTile( &AniParams );

	//  set light source....
	if (pExplosion->light == NULL)
	{
		// DO ONLY IF WE'RE AT A GOOD LEVEL
		if ( ubAmbientLightLevel >= MIN_AMB_LEVEL_FOR_MERC_LIGHTS )
		{
			LIGHT_SPRITE* const l = LightSpriteCreate("L-R04.LHT");
			pExplosion->light = l;
			if (l != NULL)
			{
				LightSpritePower(l, TRUE);
				LightSpritePosition(l, sX / CELL_X_SIZE, sY / CELL_Y_SIZE);
			}
		}
	}

	SoundID uiSoundID = inf->sound;
	if ( uiSoundID == EXPLOSION_1 )
	{
		// Randomize
		if ( Random( 2 ) == 0 )
		{
			uiSoundID = EXPLOSION_ALT_BLAST_1;
		}
	}

	PlayLocationJA2Sample(sGridNo, uiSoundID, HIGHVOLUME, 1);
}


void UpdateExplosionFrame(EXPLOSIONTYPE* const e, const INT16 sCurrentFrame)
{
	Assert(gExplosionData <= e && e < endof(gExplosionData));
	Assert(e->fAllocated);

	e->sCurrentFrame = sCurrentFrame;
}


void RemoveExplosionData(EXPLOSIONTYPE* const e)
{
	Assert(gExplosionData <= e && e < endof(gExplosionData));
	Assert(e->fAllocated);

	e->fAllocated = FALSE;
	if (e->light != NULL) LightSpriteDestroy(e->light);
}


static void HandleFencePartnerCheck(INT16 sStructGridNo)
{
	STRUCTURE *pFenceStructure, *pFenceBaseStructure;
	INT8		bFenceDestructionPartner = -1;

	pFenceStructure = FindStructure( sStructGridNo, STRUCTURE_FENCE );

	if ( pFenceStructure )
	{
		// How does our explosion partner look?
		if ( pFenceStructure->pDBStructureRef->pDBStructure->bDestructionPartner < 0 )
		{
			// Find level node.....
			pFenceBaseStructure = FindBaseStructure( pFenceStructure );

			// Get LEVELNODE for struct and remove!
			LEVELNODE* const pFenceNode = FindLevelNodeBasedOnStructure(pFenceBaseStructure);

			// Get type from index...
			const UINT32 uiFenceType = GetTileType(pFenceNode->usIndex);

			bFenceDestructionPartner = -1 * ( pFenceBaseStructure->pDBStructureRef->pDBStructure->bDestructionPartner );

			// Get new index
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(uiFenceType, bFenceDestructionPartner);

			ApplyMapChangesToMapTempFile app;
			// Remove it!
			RemoveStructFromLevelNode( pFenceBaseStructure->sGridNo, pFenceNode );
			// Add it!
			AddStructToHead( pFenceBaseStructure->sGridNo, (UINT16)( usTileIndex ) );
		}
	}
}


static void ReplaceWall(GridNo const grid_no, UINT8 const orientation, INT16 const sub_idx)
{
	STRUCTURE* const wall_struct = GetWallStructOfSameOrientationAtGridno(grid_no, orientation);
	if (!wall_struct || !(wall_struct->fFlags & STRUCTURE_WALL)) return;

	LEVELNODE* const node    = FindLevelNodeBasedOnStructure(wall_struct);
	UINT16     const new_idx = GetTileIndexFromTypeSubIndex(gTileDatabase[node->usIndex].fType, sub_idx);
	ApplyMapChangesToMapTempFile app;
	RemoveStructFromLevelNode(grid_no, node);
	AddWallToStructLayer(grid_no, new_idx, TRUE);
}


static STRUCTURE* RemoveOnWall(GridNo const grid_no, StructureFlags const flags, STRUCTURE* next)
{
	while (STRUCTURE* const attached = FindStructure(grid_no, flags))
	{
		STRUCTURE* const base = FindBaseStructure(attached);
		if (!base)
		{ // Error!
			SLOGW("Problems removing structure attached to wall at {}", grid_no);
			break;
		}

		while (next && next->usStructureID == base->usStructureID)
		{ // The next structure will also be deleted, so skip past it
			next = next->pNext;
		}

		LEVELNODE* const node = FindLevelNodeBasedOnStructure(base);
		ApplyMapChangesToMapTempFile app;
		RemoveStructFromLevelNode(base->sGridNo, node);
	}
	return next;
}


static bool ExplosiveDamageStructureAtGridNo(STRUCTURE* const pCurrent, STRUCTURE** const ppNextCurrent, INT16 const grid_no, INT16 const wound_amt, UINT32 const uiDist, BOOLEAN* const pfRecompileMovementCosts, BOOLEAN const only_walls, SOLDIERTYPE* const owner, INT8 const level)
{
	BOOLEAN_S skipDamage = false;
	BeforeStructureDamaged(gWorldSector.x, gWorldSector.y, gWorldSector.z, grid_no, pCurrent, uiDist, &skipDamage);
	if (skipDamage)
	{
		return true;
	}

	// ATE: Continue if we are only looking for walls
	if (only_walls && !(pCurrent->fFlags & STRUCTURE_WALLSTUFF)) return true;

	if (level > 0) return true;

	// Is this a corpse?
	if (pCurrent->fFlags & STRUCTURE_CORPSE && gGameSettings.fOptions[TOPTION_BLOOD_N_GORE] && wound_amt > 10)
	{
		// Spray corpse in a fine mist
		if (uiDist <= 1)
		{ // Remove corpse
			VaporizeCorpse(grid_no, pCurrent->usStructureID);
		}
	}
	else if (!(pCurrent->fFlags & STRUCTURE_PERSON))
	{
		// Damage structure!
		INT16 const sX = CenterX(grid_no);
		INT16 const sY = CenterY(grid_no);
		INT8 bDamageReturnVal = DamageStructure(pCurrent, wound_amt, STRUCTURE_DAMAGE_EXPLOSION, grid_no, sX, sY, NULL);
		if (bDamageReturnVal == STRUCTURE_NOT_DAMAGED) return true;

		BOOLEAN fDestroyed = (bDamageReturnVal == STRUCTURE_DESTROYED);
		OnStructureDamaged(gWorldSector.x, gWorldSector.y, gWorldSector.z, grid_no, pCurrent, wound_amt, fDestroyed);

		STRUCTURE* const base         = FindBaseStructure(pCurrent);
		GridNo     const base_grid_no = base->sGridNo;

		// if the structure is openable, destroy all items there
		if (base->fFlags & STRUCTURE_OPENABLE && !(base->fFlags & STRUCTURE_DOOR))
		{
			RemoveAllUnburiedItems(base_grid_no, level);
		}

		bool const is_explosive = pCurrent->fFlags & STRUCTURE_EXPLOSIVE;

		// Get LEVELNODE for struct and remove!
		LEVELNODE* const node = FindLevelNodeBasedOnStructure(base);

		INT8 const orig_destruction_partner = base->pDBStructureRef->pDBStructure->bDestructionPartner;
		/* ATE: if we have completely destroyed a structure, and this structure
		 * should have a in-between explosion partner, make damage code 2 - which
		 * means only damaged - the normal explosion spreading will cause it do use
		 * the proper pieces */
		if (bDamageReturnVal == STRUCTURE_DESTROYED && orig_destruction_partner < 0)
		{
			bDamageReturnVal = STRUCTURE_DAMAGED;
		}

		INT8    destruction_partner = -1;
		BOOLEAN fContinue           = FALSE;
		if (bDamageReturnVal == STRUCTURE_DESTROYED)
		{
			fContinue = TRUE;
		}
		// Check for a damaged looking graphic
		else if (bDamageReturnVal == STRUCTURE_DAMAGED)
		{
			if (orig_destruction_partner < 0)
			{
				// We swap to another graphic!
				// It's -ve and 1-based, change to +ve, 1 based
				destruction_partner = -orig_destruction_partner;
				fContinue           = 2;
			}
		}

		if (fContinue == 0) return true;

		// Remove the beast!
		while (*ppNextCurrent && (*ppNextCurrent)->usStructureID == pCurrent->usStructureID)
		{ // The next structure will also be deleted, so skip past it
			*ppNextCurrent = (*ppNextCurrent)->pNext;
		}

		// Replace with explosion debris if there are any....
		// ( and there already sin;t explosion debris there.... )
		if (orig_destruction_partner > 0)
		{
			// Alrighty add!

			// Add to every gridno structure is in
			UINT8                     const n_tiles = base->pDBStructureRef->pDBStructure->ubNumberOfTiles;
			DB_STRUCTURE_TILE* const* const ppTile  = base->pDBStructureRef->ppTile;

			destruction_partner = orig_destruction_partner;

			// OK, destrcution index is , as default, the partner, until we go over the first set of explsion
			// debris...
			UINT16 const tile_idx = destruction_partner >= 40 ?
				GetTileIndexFromTypeSubIndex(SECONDEXPLDEBRIS, destruction_partner - 40) :
				GetTileIndexFromTypeSubIndex(FIRSTEXPLDEBRIS,  destruction_partner);

			// Free all the non-base tiles; the base tile is at pointer 0
			for (UINT8 ubLoop = BASE_TILE; ubLoop != n_tiles; ++ubLoop)
			{
				if (ppTile[ubLoop]->fFlags & TILE_ON_ROOF) continue;

				/* There might be two structures in this tile, one on each level, but
				 * we just want to delete one on each pass */
				GridNo const struct_grid_no = base_grid_no + ppTile[ubLoop]->sPosRelToBase;
				if (TypeRangeExistsInObjectLayer(struct_grid_no, FIRSTEXPLDEBRIS, SECONDEXPLDEBRIS)) continue;

				ApplyMapChangesToMapTempFile app;
				AddObjectToHead(struct_grid_no, tile_idx + Random(3));
			}

			// If we are a wall, add debris for the other side
			if (pCurrent->fFlags & STRUCTURE_WALLSTUFF)
			{
				switch (pCurrent->ubWallOrientation)
				{
					case OUTSIDE_TOP_LEFT:
					case INSIDE_TOP_LEFT:
					{
						GridNo const struct_grid_no = NewGridNo(base_grid_no, DirectionInc(SOUTH));
						if (!TypeRangeExistsInObjectLayer(struct_grid_no, FIRSTEXPLDEBRIS, SECONDEXPLDEBRIS))
						{
							ApplyMapChangesToMapTempFile app;
							AddObjectToHead(struct_grid_no, tile_idx + Random(3));
						}
						break;
					}

					case OUTSIDE_TOP_RIGHT:
					case INSIDE_TOP_RIGHT:
					{
						GridNo const struct_grid_no = NewGridNo(base_grid_no, DirectionInc(EAST));
						if (!TypeRangeExistsInObjectLayer(struct_grid_no, FIRSTEXPLDEBRIS, SECONDEXPLDEBRIS))
						{
							ApplyMapChangesToMapTempFile app;
							AddObjectToHead(struct_grid_no, tile_idx + Random(3));
						}
						break;
					}
				}
			}
		}
		// Else look for fences, walk along them to change to destroyed pieces
		else if (pCurrent->fFlags & STRUCTURE_FENCE)
		{
			// walk along based on orientation
			switch (pCurrent->ubWallOrientation)
			{
				case OUTSIDE_TOP_RIGHT:
				case INSIDE_TOP_RIGHT:
					HandleFencePartnerCheck(NewGridNo(base_grid_no, DirectionInc(SOUTH)));
					HandleFencePartnerCheck(NewGridNo(base_grid_no, DirectionInc(NORTH)));
					break;

				case OUTSIDE_TOP_LEFT:
				case INSIDE_TOP_LEFT:
					HandleFencePartnerCheck(NewGridNo(base_grid_no, DirectionInc(EAST)));
					HandleFencePartnerCheck(NewGridNo(base_grid_no, DirectionInc(WEST)));
					break;
			}
		}

		// OK, Check if this is a wall, then search and change other walls based on this
		if (pCurrent->fFlags & STRUCTURE_WALLSTUFF)
		{
			/* ATE
			 * Remove any decals in tile
			 * Use tile database for this as apposed to stuct data */
			RemoveAllStructsOfTypeRange(base_grid_no, FIRSTWALLDECAL, FOURTHWALLDECAL);
			RemoveAllStructsOfTypeRange(base_grid_no, FIFTHWALLDECAL, EIGTHWALLDECAL);

			/* Based on orientation, go either x or y dir, check for wall in both _ve
			 * and -ve directions and if found, then replace */
			switch (UINT8 const orientation = pCurrent->ubWallOrientation)
			{
				case OUTSIDE_TOP_LEFT:
				case INSIDE_TOP_LEFT:
					ReplaceWall(NewGridNo(base_grid_no, DirectionInc(WEST)), orientation, orientation == OUTSIDE_TOP_LEFT ? 48 : 52);
					ReplaceWall(NewGridNo(base_grid_no, DirectionInc(EAST)), orientation, orientation == OUTSIDE_TOP_LEFT ? 49 : 53);

					// look for attached structures in same tile
					*ppNextCurrent = RemoveOnWall(base_grid_no, STRUCTURE_ON_LEFT_WALL, *ppNextCurrent);

					// Move in SOUTH, looking for attached structures to remove
					RemoveOnWall(NewGridNo(base_grid_no, DirectionInc(SOUTH)), STRUCTURE_ON_LEFT_WALL, 0);
					break;

				case OUTSIDE_TOP_RIGHT:
				case INSIDE_TOP_RIGHT:
					ReplaceWall(NewGridNo(base_grid_no, DirectionInc(NORTH)), orientation, orientation == OUTSIDE_TOP_RIGHT ? 51 : 55);
					ReplaceWall(NewGridNo(base_grid_no, DirectionInc(SOUTH)), orientation, orientation == OUTSIDE_TOP_RIGHT ? 50 : 54);

					// looking for attached structures to remove in base tile
					*ppNextCurrent = RemoveOnWall(base_grid_no, STRUCTURE_ON_RIGHT_WALL, *ppNextCurrent);

					// Move in EAST, looking for attached structures to remove
					RemoveOnWall(NewGridNo(base_grid_no, DirectionInc(EAST)), STRUCTURE_ON_RIGHT_WALL, 0);
					break;
			}
		}

		// We need to remove the water from the fountain
		// Lots of HARD CODING HERE :(
		UINT32 const tile_type = GetTileType(node->usIndex);
		// Check if we are a fountain!
		if (gTilesets[giCurrentTilesetID].zTileSurfaceFilenames[tile_type].compare("fount1.sti", ST::case_insensitive) == 0)
		{ // Remove water
			ApplyMapChangesToMapTempFile app;
			UINT16 sNewIndex;
			sNewIndex = GetTileIndexFromTypeSubIndex(tile_type, 1);
			RemoveStruct(base_grid_no, sNewIndex);
			RemoveStruct(base_grid_no, sNewIndex);
			sNewIndex = GetTileIndexFromTypeSubIndex(tile_type, 2);
			RemoveStruct(base_grid_no, sNewIndex);
			RemoveStruct(base_grid_no, sNewIndex);
			sNewIndex = GetTileIndexFromTypeSubIndex(tile_type, 3);
			RemoveStruct(base_grid_no, sNewIndex);
			RemoveStruct(base_grid_no, sNewIndex);
		}

		// Remove any interactive tiles we could be over
		BeginCurInteractiveTileCheck();

		if (pCurrent->fFlags & STRUCTURE_WALLSTUFF)
		{
			RecompileLocalMovementCostsForWall(base_grid_no, base->ubWallOrientation);
		}

		{ ApplyMapChangesToMapTempFile app;
			RemoveStructFromLevelNode(base_grid_no, node);
		}

		// If we are to swap structures, do it now
		if (fContinue == 2)
		{ // We have a levelnode, get new index for new graphic
			UINT16 const tile_idx = GetTileIndexFromTypeSubIndex(tile_type, destruction_partner);
			ApplyMapChangesToMapTempFile app;
			AddStructToHead(base_grid_no, tile_idx);
		}

		// Rerender world!
		// Reevaluate world movement costs, redundancy!
		gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
		// For the next render loop, re-evaluate redundent tiles
		InvalidateWorldRedundency();
		SetRenderFlags(RENDER_FLAG_FULL);
		// Movement costs!
		*pfRecompileMovementCosts = TRUE;

		// Make secondary explosion if eplosive....
		if (is_explosive)
		{
			InternalIgniteExplosion(owner, CenterX(base_grid_no), CenterY(base_grid_no), 0, base_grid_no, STRUCTURE_EXPLOSION, FALSE, level);
		}

		if (fContinue == 2) return false;
	}

	return true;
}


static void ExplosiveDamageGridNo(const INT16 sGridNo, const INT16 sWoundAmt, const UINT32 uiDist, BOOLEAN* const pfRecompileMovementCosts, const BOOLEAN fOnlyWalls, INT8 bMultiStructSpecialFlag, SOLDIERTYPE* const owner, const INT8 bLevel)
{
	STRUCTURE *pCurrent, *pNextCurrent, *pStructure;
	STRUCTURE *pBaseStructure;
	INT16     sDesiredLevel;
	UINT8     ubLoop, ubLoop2;
	INT16     sNewGridNo, sNewGridNo2;
	BOOLEAN   fToBreak = FALSE;
	BOOLEAN   fMultiStructure = FALSE;
	BOOLEAN   fMultiStructSpecialFlag = FALSE;
	BOOLEAN   fExplodeDamageReturn = FALSE;

	std::vector<DB_STRUCTURE_TILE*> ppTile;
	GridNo              sBaseGridNo     = NOWHERE; // XXX HACK000E
	UINT8               ubNumberOfTiles = 0;       // XXX HACK000E

	// Based on distance away, damage any struct at this gridno
	// OK, loop through structures and damage!
	pCurrent =  gpWorldLevelData[ sGridNo ].pStructureHead;
	sDesiredLevel  = STRUCTURE_ON_GROUND;

	// This code gets a little hairy because
	// (1) we might need to destroy the currently-examined structure
	while (pCurrent != NULL)
	{
		// ATE: These are for the chacks below for multi-structs....
		pBaseStructure = FindBaseStructure( pCurrent );

		if ( pBaseStructure )
		{
			sBaseGridNo = pBaseStructure->sGridNo;
			ubNumberOfTiles = pBaseStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles;
			fMultiStructure = ( ( pBaseStructure->fFlags & STRUCTURE_MULTI ) != 0 );
			ppTile.assign(pBaseStructure->pDBStructureRef->ppTile, pBaseStructure->pDBStructureRef->ppTile + ubNumberOfTiles);

			if ( bMultiStructSpecialFlag == -1 )
			{
				// Set it!
				bMultiStructSpecialFlag = ( ( pBaseStructure->fFlags & STRUCTURE_SPECIAL ) != 0 );
			}

			if ( pBaseStructure->fFlags & STRUCTURE_EXPLOSIVE )
			{
				// ATE: Set hit points to zero....
				pBaseStructure->ubHitPoints = 0;
			}
		}
		else
		{
			fMultiStructure = FALSE;
		}

		pNextCurrent = pCurrent->pNext;

		// Check level!
		if (pCurrent->sCubeOffset == sDesiredLevel )
		{
			fExplodeDamageReturn = ExplosiveDamageStructureAtGridNo(pCurrent, &pNextCurrent, sGridNo, sWoundAmt, uiDist, pfRecompileMovementCosts, fOnlyWalls, owner, bLevel);

			if ( !fExplodeDamageReturn )
			{
				fToBreak = TRUE;
			}
		}

		// OK, for multi-structs...
		// AND we took damage...
		if ( fMultiStructure && !fOnlyWalls && fExplodeDamageReturn == 0 )
		{
			// ATE: Don't after first attack...
			if ( uiDist > 1 )
			{
				return;
			}

			{

				for ( ubLoop = BASE_TILE; ubLoop < ubNumberOfTiles; ubLoop++)
				{
					sNewGridNo = sBaseGridNo + ppTile[ubLoop]->sPosRelToBase;

					// look in adjacent tiles
					for ( ubLoop2 = 0; ubLoop2 < NUM_WORLD_DIRECTIONS; ubLoop2++ )
					{
						sNewGridNo2 = NewGridNo( sNewGridNo, DirectionInc( ubLoop2 ) );
						if ( sNewGridNo2 != sNewGridNo && sNewGridNo2 != sGridNo )
						{
							pStructure = FindStructure( sNewGridNo2, STRUCTURE_MULTI );
							if ( pStructure )
							{
								fMultiStructSpecialFlag = ( ( pStructure->fFlags & STRUCTURE_SPECIAL ) != 0 );

								if ( bMultiStructSpecialFlag == fMultiStructSpecialFlag )
								{
									// If we just damaged it, use same damage value....
									ExplosiveDamageGridNo(sNewGridNo2, sWoundAmt, uiDist, pfRecompileMovementCosts, fOnlyWalls, bMultiStructSpecialFlag, owner, bLevel);

									InternalIgniteExplosion(owner, CenterX(sNewGridNo2), CenterY(sNewGridNo2), 0, sNewGridNo2, RDX, FALSE, bLevel);

									fToBreak = TRUE;
								}
							}
						}
					}
				}
			}
			if ( fToBreak )
			{
				break;
			}
		}

		if ( pBaseStructure )
		{
			ppTile.clear();
		}

		pCurrent = pNextCurrent;
	}
}


static BOOLEAN DamageSoldierFromBlast(SOLDIERTYPE* const pSoldier, SOLDIERTYPE* const owner, const INT16 sBombGridNo, const INT16 sWoundAmt, const INT16 sBreathAmt, const UINT32 uiDist, const UINT16 usItem)
{
	INT16 sNewWoundAmt = 0;
	UINT8		ubDirection;

	if (!pSoldier->bActive || !pSoldier->bInSector || !pSoldier->bLife )
		return( FALSE );

	if ( pSoldier->ubMiscSoldierFlags & SOLDIER_MISC_HURT_BY_EXPLOSION )
	{
		// don't want to damage the guy twice
		return( FALSE );
	}

	// Direction to center of explosion
	ubDirection = (UINT8)GetDirectionFromGridNo( sBombGridNo, pSoldier );

	// Increment attack counter...
	gTacticalStatus.ubAttackBusyCount++;
	SLOGD("Incrementing Attack: Explosion dishing out damage, Count now %d", gTacticalStatus.ubAttackBusyCount);

	sNewWoundAmt = sWoundAmt - std::min(int(sWoundAmt), 35) * ArmourVersusExplosivesPercent(pSoldier) / 100;
	if ( sNewWoundAmt < 0 )
	{
		sNewWoundAmt = 0;
	}
	EVENT_SoldierGotHit(pSoldier, usItem, sNewWoundAmt, sBreathAmt, ubDirection, uiDist, owner, 0, ANIM_CROUCH, sBombGridNo);

	pSoldier->ubMiscSoldierFlags |= SOLDIER_MISC_HURT_BY_EXPLOSION;

	if (owner != NULL && owner->bTeam == OUR_TEAM && pSoldier->bTeam != OUR_TEAM)
	{
		ProcessImplicationsOfPCAttack(owner, pSoldier, REASON_EXPLOSION);
	}

	return( TRUE );
}


BOOLEAN DishOutGasDamage(SOLDIERTYPE* const pSoldier, EXPLOSIVETYPE const* const pExplosive, INT16 const sSubsequent, BOOLEAN const fRecompileMovementCosts, INT16 sWoundAmt, INT16 sBreathAmt, SOLDIERTYPE* const owner)
{
	INT8 bPosOfMask = NO_SLOT;

	if (!pSoldier->bActive || !pSoldier->bInSector || !pSoldier->bLife || AM_A_ROBOT( pSoldier ) )
	{
		return( fRecompileMovementCosts );
	}

	if ( pExplosive->ubType == EXPLOSV_CREATUREGAS )
	{
		if ( pSoldier->uiStatusFlags & SOLDIER_MONSTER )
		{
			// unaffected by own gas effects
			return( fRecompileMovementCosts );
		}
		if ( sSubsequent && pSoldier->fHitByGasFlags & HIT_BY_CREATUREGAS )
		{
			// already affected by creature gas this turn
			return( fRecompileMovementCosts );
		}
	}
	else // no gas mask help from creature attacks
	// ATE/CJC: gas stuff
	{
		if ( pExplosive->ubType == EXPLOSV_TEARGAS )
		{
			if ( AM_A_ROBOT( pSoldier ) )
			{
				return( fRecompileMovementCosts );
			}

			// ignore whether subsequent or not if hit this turn
			if ( pSoldier->fHitByGasFlags & HIT_BY_TEARGAS )
			{
				// already affected by creature gas this turn
				return( fRecompileMovementCosts );
			}
		}
		else if ( pExplosive->ubType == EXPLOSV_MUSTGAS )
		{
			if ( AM_A_ROBOT( pSoldier ) )
			{
				return( fRecompileMovementCosts );
			}

			if ( sSubsequent && pSoldier->fHitByGasFlags & HIT_BY_MUSTARDGAS )
			{
				// already affected by creature gas this turn
				return( fRecompileMovementCosts );
			}

		}

		if ( pSoldier->inv[ HEAD1POS ].usItem == GASMASK && pSoldier->inv[ HEAD1POS ].bStatus[0] >= USABLE )
		{
			bPosOfMask = HEAD1POS;
		}
		else if ( pSoldier->inv[ HEAD2POS ].usItem == GASMASK && pSoldier->inv[ HEAD2POS ].bStatus[0] >= USABLE )
		{
			bPosOfMask = HEAD2POS;
		}

		if ( bPosOfMask != NO_SLOT  )
		{
			if ( pSoldier->inv[ bPosOfMask ].bStatus[0] < GASMASK_MIN_STATUS )
			{
				// GAS MASK reduces breath loss by its work% (it leaks if not at least 70%)
				sBreathAmt = ( sBreathAmt * ( 100 - pSoldier->inv[ bPosOfMask ].bStatus[0] ) ) / 100;
				if ( sBreathAmt > 500 )
				{
					// if at least 500 of breath damage got through
					// the soldier within the blast radius is gassed for at least one
					// turn, possibly more if it's tear gas (which hangs around a while)
					pSoldier->uiStatusFlags |= SOLDIER_GASSED;
				}

				if ( pSoldier->uiStatusFlags & SOLDIER_PC )
				{

					if ( sWoundAmt > 1 )
					{
						pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 4 );
						sWoundAmt = ( sWoundAmt * ( 100 -  pSoldier->inv[ bPosOfMask ].bStatus[0] ) ) / 100;
					}
					else if ( sWoundAmt == 1 )
					{
						pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 2 );
					}
				}
			}
			else
			{
				sBreathAmt = 0;
				if ( sWoundAmt > 0 )
				{
					if ( sWoundAmt == 1 )
					{
						pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 2 );
					}
					else
					{
						// use up gas mask
						pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 4 );
					}
				}
				sWoundAmt = 0;
			}

		}
	}

	if ( sWoundAmt != 0 || sBreathAmt != 0 )
	{
		switch( pExplosive->ubType )
		{
			case EXPLOSV_CREATUREGAS:
				pSoldier->fHitByGasFlags |= HIT_BY_CREATUREGAS;
				break;
			case EXPLOSV_TEARGAS:
				pSoldier->fHitByGasFlags |= HIT_BY_TEARGAS;
				break;
			case EXPLOSV_MUSTGAS:
				pSoldier->fHitByGasFlags |= HIT_BY_MUSTARDGAS;
				break;
			default:
				break;
		}
		// a gas effect, take damage directly...
		SoldierTakeDamage(pSoldier, sWoundAmt, sBreathAmt, TAKE_DAMAGE_GAS, NULL);
		if ( pSoldier->bLife >= CONSCIOUSNESS )
		{
			DoMercBattleSound(pSoldier, BATTLE_SOUND_HIT1);
		}

		if (owner != NULL && owner->bTeam == OUR_TEAM && pSoldier->bTeam != OUR_TEAM)
		{
			ProcessImplicationsOfPCAttack(owner, pSoldier, REASON_EXPLOSION);
		}
	}
	return( fRecompileMovementCosts );
}


static void HandleBuldingDestruction(INT16 sGridNo, const SOLDIERTYPE* owner);


// Spreads the effects of explosions...
static BOOLEAN ExpAffect(const INT16 sBombGridNo, const INT16 sGridNo, const UINT32 uiDist, const UINT16 usItem, SOLDIERTYPE* const owner, const INT16 sSubsequent, BOOLEAN* const pfMercHit, const INT8 bLevel, const SMOKEEFFECT* const smoke)
{
	INT16 sWoundAmt = 0, sBreathAmt = 0, sStructDmgAmt;
	BOOLEAN fRecompileMovementCosts = FALSE;
	BOOLEAN fSmokeEffect=FALSE;
	BOOLEAN fStunEffect = FALSE;
	SmokeEffectKind bSmokeEffectType = NO_SMOKE_EFFECT;
	BOOLEAN	fBlastEffect = TRUE;
	INT16		sNewGridNo;
	UINT32	uiRoll;

	if ( sSubsequent == BLOOD_SPREAD_EFFECT )
	{
		fSmokeEffect = FALSE;
		fBlastEffect = FALSE;
	}
	else
	{
		// Turn off blast effect if some types of items...
		switch( usItem )
		{
			case MUSTARD_GRENADE:
				fSmokeEffect = TRUE;
				bSmokeEffectType = MUSTARDGAS_SMOKE_EFFECT;
				fBlastEffect = FALSE;
				break;

			case TEARGAS_GRENADE:
			case GL_TEARGAS_GRENADE:
			case BIG_TEAR_GAS:
				fSmokeEffect = TRUE;
				bSmokeEffectType = TEARGAS_SMOKE_EFFECT;
				fBlastEffect = FALSE;
				break;

			case SMOKE_GRENADE:
			case GL_SMOKE_GRENADE:
				fSmokeEffect = TRUE;
				bSmokeEffectType = NORMAL_SMOKE_EFFECT;
				fBlastEffect = FALSE;
				break;

			case STUN_GRENADE:
			case GL_STUN_GRENADE:
				fStunEffect = TRUE;
				break;

			case SMALL_CREATURE_GAS:
			case LARGE_CREATURE_GAS:
			case VERY_SMALL_CREATURE_GAS:
				fSmokeEffect = TRUE;
				bSmokeEffectType = CREATURE_SMOKE_EFFECT;
				fBlastEffect = FALSE;
				break;
		}
	}


	// OK, here we:
	// Get explosive data from table
	EXPLOSIVETYPE const* const pExplosive = &Explosive[GCM->getItem(usItem)->getClassIndex()];

	uiRoll = PreRandom( 100 );

	// Calculate wound amount
	sWoundAmt = pExplosive->ubDamage + (INT16) ( (pExplosive->ubDamage * uiRoll) / 100 );

	// Calculate breath amount ( if stun damage applicable )
	sBreathAmt = ( pExplosive->ubStunDamage * 100 ) + (INT16) ( ( ( pExplosive->ubStunDamage / 2 ) * 100 * uiRoll ) / 100 ) ;

	// ATE: Make sure guys get pissed at us!
	HandleBuldingDestruction(sGridNo, owner);


	if ( fBlastEffect )
	{
		// lower effects for distance away from center of explosion
		// If radius is 3, damage % is (100)/66/33/17
		// If radius is 5, damage % is (100)/80/60/40/20/10
		// If radius is 8, damage % is (100)/88/75/63/50/37/25/13/6

		if ( pExplosive->ubRadius == 0 )
		{
			// leave as is, has to be at range 0 here
		}
		else if (uiDist < pExplosive->ubRadius)
		{
			// if radius is 5, go down by 5ths ~ 20%
			sWoundAmt -= (INT16)  (sWoundAmt * uiDist / pExplosive->ubRadius );
			sBreathAmt -= (INT16) (sBreathAmt * uiDist / pExplosive->ubRadius );
		}
		else
		{
			// at the edge of the explosion, do half the previous damage
			sWoundAmt = (INT16) ( (sWoundAmt / pExplosive->ubRadius) / 2);
			sBreathAmt = (INT16) ( (sBreathAmt / pExplosive->ubRadius) / 2);
		}

		if (sWoundAmt < 0)
			sWoundAmt = 0;

		if (sBreathAmt < 0)
			sBreathAmt = 0;

		// damage structures
		if ( uiDist <= std::max(1U, (UINT32) (pExplosive->ubDamage / 30)))
		{
			if ( GCM->getItem(usItem)->isGrenade() )
			{
				sStructDmgAmt = sWoundAmt / 3;
			}
			else // most explosives
			{
				sStructDmgAmt = sWoundAmt;
			}

			ExplosiveDamageGridNo(sGridNo, sStructDmgAmt, uiDist, &fRecompileMovementCosts, FALSE, -1, owner, bLevel);

			// ATE: Look for damage to walls ONLY for next two gridnos
			sNewGridNo = NewGridNo( sGridNo, DirectionInc( NORTH ) );

			if ( GridNoOnVisibleWorldTile( sNewGridNo ) )
			{
				ExplosiveDamageGridNo(sNewGridNo, sStructDmgAmt, uiDist, &fRecompileMovementCosts, TRUE, -1, owner, bLevel);
			}

			// ATE: Look for damage to walls ONLY for next two gridnos
			sNewGridNo = NewGridNo( sGridNo, DirectionInc( WEST ) );

			if ( GridNoOnVisibleWorldTile( sNewGridNo ) )
			{
				ExplosiveDamageGridNo(sNewGridNo, sStructDmgAmt, uiDist, &fRecompileMovementCosts, TRUE, -1, owner, bLevel);
			}
		}

		// Add burn marks to ground randomly....
		if ( Random( 50 ) < 15 && uiDist == 1 )
		{
			//if (!TypeRangeExistsInObjectLayer(sGridNo, FIRSTEXPLDEBRIS, SECONDEXPLDEBRIS))
			//{
			//	UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(SECONDEXPLDEBRIS, Random(10) + 1);
			//	AddObjectToHead( sGridNo, usTileIndex );
			//	SetRenderFlags(RENDER_FLAG_FULL);
			//}
		}

		// NB radius can be 0 so cannot divide it by 2 here
		if (!fStunEffect && (uiDist * 2 <= pExplosive->ubRadius)  )
		{
			const ITEM_POOL* pItemPool = GetItemPool(sGridNo, bLevel);

			while( pItemPool )
			{
				const ITEM_POOL* pItemPoolNext = pItemPool->pNext;

				WORLDITEM& wi = GetWorldItem(pItemPool->iItemIndex);
				if (DamageItemOnGround(&wi.o, sGridNo, bLevel, sWoundAmt * 2, owner))
				{
					// item was destroyed
					RemoveItemFromPool(wi);
				}
				pItemPool = pItemPoolNext;
			}
		}
	}
	else if ( fSmokeEffect )
	{
		// If tear gar, determine turns to spread.....
		if ( sSubsequent == ERASE_SPREAD_EFFECT )
		{
			RemoveSmokeEffectFromTile( sGridNo, bLevel );
		}
		else if ( sSubsequent != REDO_SPREAD_EFFECT )
		{
			AddSmokeEffectToTile(smoke, bSmokeEffectType, sGridNo, bLevel);
		}
	}
	else
	{
		// Drop blood ....
		// Get blood quantity....
		InternalDropBlood(sGridNo, 0, HUMAN, std::max(MAXBLOODQUANTITY - uiDist * 2 /* XXX always >= 0, because uiDist is unsigned */, 0U), 1);
	}

	if ( sSubsequent != ERASE_SPREAD_EFFECT && sSubsequent != BLOOD_SPREAD_EFFECT )
	{
		// if an explosion effect....
		if ( fBlastEffect )
		{
			// don't hurt anyone who is already dead & waiting to be removed
			SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, bLevel);
			if (tgt != NULL)
			{
				DamageSoldierFromBlast(tgt, owner, sBombGridNo, sWoundAmt, sBreathAmt, uiDist, usItem);
			}

			if ( bLevel == 1 )
			{
				SOLDIERTYPE* const tgt_below = WhoIsThere2(sGridNo, 0);
				if (tgt_below != NULL)
				{
					if ( (sWoundAmt / 2) > 20 )
					{
						// debris damage!
						const INT16 breath = sBreathAmt / 2 - 20 > 0 ? Random(sBreathAmt / 2 - 20) : 1;
						DamageSoldierFromBlast(tgt_below, owner, sBombGridNo, Random(sWoundAmt / 2 - 20), breath, uiDist, usItem);
					}
				}
			}
		}
		else
		{
			SOLDIERTYPE* const pSoldier = WhoIsThere2(sGridNo, bLevel);
			if (pSoldier == NULL) return fRecompileMovementCosts;
			// someone is here, and they're gonna get hurt

			fRecompileMovementCosts = DishOutGasDamage(pSoldier, pExplosive, sSubsequent, fRecompileMovementCosts, sWoundAmt, sBreathAmt, owner);
/*
			if (!pSoldier->bActive || !pSoldier->bInSector || !pSoldier->bLife || AM_A_ROBOT( pSoldier ) )
			{
				return( fRecompileMovementCosts );
			}

			if ( pExplosive->ubType == EXPLOSV_CREATUREGAS )
			{
				if ( pSoldier->uiStatusFlags & SOLDIER_MONSTER )
				{
					// unaffected by own gas effects
					return( fRecompileMovementCosts );
				}
				if ( sSubsequent && pSoldier->fHitByGasFlags & HIT_BY_CREATUREGAS )
				{
					// already affected by creature gas this turn
					return( fRecompileMovementCosts );
				}
			}
			else // no gas mask help from creature attacks
				// ATE/CJC: gas stuff
				{
					INT8 bPosOfMask = NO_SLOT;


				if ( pExplosive->ubType == EXPLOSV_TEARGAS )
				{
					// ignore whether subsequent or not if hit this turn
					if ( pSoldier->fHitByGasFlags & HIT_BY_TEARGAS )
					{
						// already affected by creature gas this turn
						return( fRecompileMovementCosts );
					}
				}
				else if ( pExplosive->ubType == EXPLOSV_MUSTGAS )
				{
					if ( sSubsequent && pSoldier->fHitByGasFlags & HIT_BY_MUSTARDGAS )
					{
						// already affected by creature gas this turn
						return( fRecompileMovementCosts );
					}

				}

				if ( sSubsequent && pSoldier->fHitByGasFlags & HIT_BY_CREATUREGAS )
				{
					// already affected by creature gas this turn
					return( fRecompileMovementCosts );
				}


				if ( pSoldier->inv[ HEAD1POS ].usItem == GASMASK && pSoldier->inv[ HEAD1POS ].bStatus[0] >= USABLE )
				{
					bPosOfMask = HEAD1POS;
				}
				else if ( pSoldier->inv[ HEAD2POS ].usItem == GASMASK && pSoldier->inv[ HEAD2POS ].bStatus[0] >= USABLE )
				{
					bPosOfMask = HEAD2POS;
				}

				if ( bPosOfMask != NO_SLOT  )
				{
					if ( pSoldier->inv[ bPosOfMask ].bStatus[0] < GASMASK_MIN_STATUS )
					{
						// GAS MASK reduces breath loss by its work% (it leaks if not at least 70%)
						sBreathAmt = ( sBreathAmt * ( 100 - pSoldier->inv[ bPosOfMask ].bStatus[0] ) ) / 100;
						if ( sBreathAmt > 500 )
						{
							// if at least 500 of breath damage got through
							// the soldier within the blast radius is gassed for at least one
							// turn, possibly more if it's tear gas (which hangs around a while)
							pSoldier->uiStatusFlags |= SOLDIER_GASSED;
						}

						if ( sWoundAmt > 1 )
						{
							pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 4 );
							sWoundAmt = ( sWoundAmt * ( 100 -  pSoldier->inv[ bPosOfMask ].bStatus[0] ) ) / 100;
						}
						else if ( sWoundAmt == 1 )
						{
							pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 2 );
						}
					}
					else
					{
						sBreathAmt = 0;
						if ( sWoundAmt > 0 )
						{
							if ( sWoundAmt == 1 )
							{
								pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 2 );
							}
							else
							{
								// use up gas mask
								pSoldier->inv[ bPosOfMask ].bStatus[0] -= (INT8) Random( 4 );
							}
						}
						sWoundAmt = 0;
					}

				}
			}

			if ( sWoundAmt != 0 || sBreathAmt != 0 )
			{
				switch( pExplosive->ubType )
				{
					case EXPLOSV_CREATUREGAS:
						pSoldier->fHitByGasFlags |= HIT_BY_CREATUREGAS;
						break;
					case EXPLOSV_TEARGAS:
						pSoldier->fHitByGasFlags |= HIT_BY_TEARGAS;
						break;
					case EXPLOSV_MUSTGAS:
						pSoldier->fHitByGasFlags |= HIT_BY_MUSTARDGAS;
						break;
					default:
						break;
				}
				// a gas effect, take damage directly...
				SoldierTakeDamage(pSoldier, sWoundAmt, sBreathAmt, TAKE_DAMAGE_GAS, NULL);
				if ( pSoldier->bLife >= CONSCIOUSNESS )
				{
					DoMercBattleSound(pSoldier, BATTLE_SOUND_HIT1);
				}
			}
			*/
		}

		(*pfMercHit) = TRUE;
	}

	return( fRecompileMovementCosts );
}


static void GetRayStopInfo(UINT32 uiNewSpot, UINT8 ubDir, INT8 bLevel, BOOLEAN fSmokeEffect, INT32 uiCurRange, INT32* piMaxRange, UINT8* pubKeepGoing)
{
	INT8      bStructHeight;
	UINT8     ubMovementCost;
	INT8      Blocking, BlockingTemp;
	BOOLEAN   fTravelCostObs = FALSE;
	UINT32    uiRangeReduce;
	INT16     sNewGridNo;
	STRUCTURE *pBlockingStructure;
	BOOLEAN   fBlowWindowSouth = FALSE;
	BOOLEAN   fReduceRay = TRUE;

	ubMovementCost = gubWorldMovementCosts[ uiNewSpot ][ ubDir ][ bLevel ];

	if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
	{
		ubMovementCost = DoorTravelCost( NULL, uiNewSpot, ubMovementCost, FALSE, NULL );
		// If we have hit a wall, STOP HERE
		if (ubMovementCost >= TRAVELCOST_BLOCKED)
		{
			fTravelCostObs  = TRUE;
		}
	}
	else
	{
		// If we have hit a wall, STOP HERE
		if ( ubMovementCost == TRAVELCOST_WALL )
		{
			// We have an obstacle here..
			fTravelCostObs = TRUE;
		}
	}


	Blocking = GetBlockingStructureInfo( (INT16)uiNewSpot, ubDir, 0, bLevel, &bStructHeight, &pBlockingStructure, TRUE );

	if ( pBlockingStructure )
	{
		if ( pBlockingStructure->fFlags & STRUCTURE_CAVEWALL )
		{
			// block completely!
			fTravelCostObs = TRUE;
		}
		else if ( pBlockingStructure->pDBStructureRef->pDBStructure->ubDensity <= 15 )
		{
		// not stopped
		fTravelCostObs = FALSE;
		fReduceRay = FALSE;
		}
	}

	if ( fTravelCostObs )
	{

		if ( fSmokeEffect )
		{
			if ( Blocking == BLOCKING_TOPRIGHT_OPEN_WINDOW || Blocking == BLOCKING_TOPLEFT_OPEN_WINDOW )
			{
				// If open, fTravelCostObs set to false and reduce range....
				fTravelCostObs = FALSE;
				// Range will be reduced below...
			}

			if ( fTravelCostObs )
			{
				// ATE: For windows, check to the west and north for a broken window, as movement costs
				// will override there...
				sNewGridNo = NewGridNo( (INT16)uiNewSpot, DirectionInc( WEST ) );

				BlockingTemp = GetBlockingStructureInfo( (INT16)sNewGridNo, ubDir, 0, bLevel, &bStructHeight, &pBlockingStructure, TRUE );
				if ( BlockingTemp == BLOCKING_TOPRIGHT_OPEN_WINDOW || BlockingTemp == BLOCKING_TOPLEFT_OPEN_WINDOW )
				{
					// If open, fTravelCostObs set to false and reduce range....
					fTravelCostObs = FALSE;
					// Range will be reduced below...
				}
				if ( pBlockingStructure && pBlockingStructure->pDBStructureRef->pDBStructure->ubDensity <= 15 )
				{
					fTravelCostObs = FALSE;
					fReduceRay = FALSE;
				}
			}

			if ( fTravelCostObs )
			{
				sNewGridNo = NewGridNo( (INT16)uiNewSpot, DirectionInc( NORTH ) );

				BlockingTemp = GetBlockingStructureInfo( (INT16)sNewGridNo, ubDir, 0, bLevel, &bStructHeight, &pBlockingStructure, TRUE );
				if ( BlockingTemp == BLOCKING_TOPRIGHT_OPEN_WINDOW || BlockingTemp == BLOCKING_TOPLEFT_OPEN_WINDOW )
				{
					// If open, fTravelCostObs set to false and reduce range....
					fTravelCostObs = FALSE;
					// Range will be reduced below...
				}
				if ( pBlockingStructure && pBlockingStructure->pDBStructureRef->pDBStructure->ubDensity <= 15 )
				{
					fTravelCostObs = FALSE;
					fReduceRay = FALSE;
				}
			}

		}
		else
		{
			// We are a blast effect....

			// ATE: explode windows!!!!
			if ( Blocking == BLOCKING_TOPLEFT_WINDOW || Blocking == BLOCKING_TOPRIGHT_WINDOW )
			{
				// Explode!
				if ( ubDir == SOUTH || ubDir == SOUTHEAST || ubDir == SOUTHWEST )
				{
					fBlowWindowSouth = TRUE;
				}

				if ( pBlockingStructure != NULL )
				{
					WindowHit( (INT16)uiNewSpot, pBlockingStructure->usStructureID, fBlowWindowSouth, TRUE );
				}
			}

			// ATE: For windows, check to the west and north for a broken window, as movement costs
			// will override there...
			sNewGridNo = NewGridNo( (INT16)uiNewSpot, DirectionInc( WEST ) );

			BlockingTemp = GetBlockingStructureInfo( (INT16)sNewGridNo, ubDir, 0, bLevel, &bStructHeight, &pBlockingStructure , TRUE );
			if ( pBlockingStructure && pBlockingStructure->pDBStructureRef->pDBStructure->ubDensity <= 15 )
			{
				fTravelCostObs = FALSE;
				fReduceRay = FALSE;
			}
			if ( BlockingTemp == BLOCKING_TOPRIGHT_WINDOW || BlockingTemp == BLOCKING_TOPLEFT_WINDOW )
			{
				if ( pBlockingStructure != NULL )
				{
					WindowHit( sNewGridNo, pBlockingStructure->usStructureID, FALSE, TRUE );
				}
			}

			sNewGridNo = NewGridNo( (INT16)uiNewSpot, DirectionInc( NORTH ) );
			BlockingTemp = GetBlockingStructureInfo( (INT16)sNewGridNo, ubDir, 0, bLevel, &bStructHeight, &pBlockingStructure, TRUE );

			if ( pBlockingStructure && pBlockingStructure->pDBStructureRef->pDBStructure->ubDensity <= 15 )
			{
				fTravelCostObs = FALSE;
				fReduceRay = FALSE;
			}
			if ( BlockingTemp == BLOCKING_TOPRIGHT_WINDOW || BlockingTemp == BLOCKING_TOPLEFT_WINDOW )
			{
				if ( pBlockingStructure != NULL )
				{
					WindowHit( sNewGridNo, pBlockingStructure->usStructureID, FALSE, TRUE );
				}
			}
		}
	}

	// Have we hit things like furniture, etc?
	if ( Blocking != NOTHING_BLOCKING && !fTravelCostObs )
	{
		// ATE: Tall things should blaock all; Default wall/door height is 4
		if ( bStructHeight > 4 )
		{
			(*pubKeepGoing) = FALSE;
		}
		else
		{
			// If we are smoke, reduce range variably....
			if ( fReduceRay )
			{
				if ( fSmokeEffect )
				{
					switch( bStructHeight )
					{
						case 3:
							uiRangeReduce = 2;
							break;
						case 2:
							uiRangeReduce = 1;
							break;
						default:
							uiRangeReduce = 0;
							break;
					}
				}
				else
				{
					uiRangeReduce = 2;
				}

				(*piMaxRange) -= uiRangeReduce;
			}

			if ( uiCurRange <= (*piMaxRange) )
			{
				(*pubKeepGoing) = TRUE;
			}
			else
			{
				(*pubKeepGoing) = FALSE;
			}
		}
	}
	else
	{
		if ( fTravelCostObs )
		{
			(*pubKeepGoing) = FALSE;
		}
		else
		{
			(*pubKeepGoing) = TRUE;
		}
	}

	if (bLevel == 1)
	{
		// We check for roof-level and structure to prevent smoke spreading over roof
		STRUCTURE * pStructure = FindStructure( uiNewSpot, STRUCTURE_ROOF );
		if (pStructure == NULL)
		{
			// no structure found therefore we can't spread
			(*pubKeepGoing) = FALSE;
		}
	}
}


void SpreadEffect(const INT16 sGridNo, const UINT8 ubRadius, const UINT16 usItem, SOLDIERTYPE* const owner, const BOOLEAN fSubsequent, const INT8 bLevel, const SMOKEEFFECT* const smoke)
{
	INT32   uiNewSpot, uiTempSpot, uiBranchSpot, branchCnt;
	INT32   uiTempRange, ubBranchRange;
	UINT8   ubDir,ubBranchDir, ubKeepGoing;
	INT16   sRange;
	BOOLEAN fRecompileMovement = FALSE;
	BOOLEAN fAnyMercHit = FALSE;
	BOOLEAN fSmokeEffect = FALSE;

	switch( usItem )
	{
		case MUSTARD_GRENADE:
		case TEARGAS_GRENADE:
		case GL_TEARGAS_GRENADE:
		case BIG_TEAR_GAS:
		case SMOKE_GRENADE:
		case GL_SMOKE_GRENADE:
		case SMALL_CREATURE_GAS:
		case LARGE_CREATURE_GAS:
		case VERY_SMALL_CREATURE_GAS:
			fSmokeEffect = TRUE;
			break;
	}

	// Set values for recompile region to optimize area we need to recompile for MPs
	gsRecompileAreaTop = sGridNo / WORLD_COLS;
	gsRecompileAreaLeft = sGridNo % WORLD_COLS;
	gsRecompileAreaRight = gsRecompileAreaLeft;
	gsRecompileAreaBottom = gsRecompileAreaTop;

	// multiply range by 2 so we can correctly calculate approximately round explosion regions
	sRange = ubRadius * 2;

	// first, affect main spot
	if (ExpAffect(sGridNo, sGridNo, 0, usItem, owner, fSubsequent, &fAnyMercHit, bLevel, smoke))
	{
		fRecompileMovement = TRUE;
	}


	for (ubDir = NORTH; ubDir <= NORTHWEST; ubDir++ )
	{
		uiTempSpot = sGridNo;

		uiTempRange = sRange;

		INT32 cnt;
		if (ubDir & 1)
		{
			cnt = 3;
		}
		else
		{
			cnt = 2;
		}
		while( cnt <= uiTempRange) // end of range loop
		{
			// move one tile in direction
			uiNewSpot = NewGridNo( (INT16)uiTempSpot, DirectionInc( ubDir ) );

			// see if this was a different spot & if we should be able to reach
			// this spot
			if (uiNewSpot == uiTempSpot)
			{
				ubKeepGoing = FALSE;
			}
			else
			{
				// Check if struct is a tree, etc and reduce range...
				GetRayStopInfo( uiNewSpot, ubDir, bLevel, fSmokeEffect, cnt, &uiTempRange, &ubKeepGoing );
			}

			if (ubKeepGoing)
			{
				uiTempSpot = uiNewSpot;

				SLOGD("Explosion affects %d", uiNewSpot);
				// ok, do what we do here...
				if (ExpAffect(sGridNo, uiNewSpot, cnt / 2, usItem, owner, fSubsequent, &fAnyMercHit, bLevel, smoke))
				{
					fRecompileMovement = TRUE;
				}

				// how far should we branch out here?
				ubBranchRange = (UINT8)( sRange - cnt );

				if ( ubBranchRange )
				{
					// ok, there's a branch here. Mark where we start this branch.
					uiBranchSpot = uiNewSpot;

					// figure the branch direction - which is one dir clockwise
					ubBranchDir = (ubDir + 1) % 8;

					if (ubBranchDir & 1)
					{
						branchCnt = 3;
					}
					else
					{
						branchCnt = 2;
					}

					while( branchCnt <= ubBranchRange) // end of range loop
					{
						ubKeepGoing = TRUE;
						uiNewSpot = NewGridNo( (INT16)uiBranchSpot, DirectionInc(ubBranchDir));

						if (uiNewSpot != uiBranchSpot)
						{
							// Check if struct is a tree, etc and reduce range...
							GetRayStopInfo( uiNewSpot, ubBranchDir, bLevel, fSmokeEffect, branchCnt, &ubBranchRange, &ubKeepGoing );

							if ( ubKeepGoing )
							{
								// ok, do what we do here
								SLOGD("Explosion affects %d", uiNewSpot);
								if (ExpAffect(sGridNo, uiNewSpot, (INT16)((cnt + branchCnt) / 2), usItem, owner, fSubsequent, &fAnyMercHit, bLevel, smoke))
								{
									fRecompileMovement = TRUE;
								}
								uiBranchSpot = uiNewSpot;
							}
							//else
							{
								// check if it's ANY door, and if so, affect that spot so it's damaged
							//	if (RealDoorAt(uiNewSpot))
							//	{
							//      	ExpAffect(sGridNo,uiNewSpot,cnt,ubReason,fSubsequent);
							//	}
								// blocked, break out of the the sub-branch loop
							//	break;
							}
						}

						if (ubBranchDir & 1)
						{
							branchCnt += 3;
						}
						else
						{
							branchCnt += 2;
						}

					}
				} // end of if a branch to do

			}
			else // at edge, or tile blocks further spread in that direction
			{
				break;
			}

			if (ubDir & 1)
			{
				cnt += 3;
			}
			else
			{
				cnt += 2;
			}
		}

	} // end of dir loop

	// Recompile movement costs...
	if ( fRecompileMovement )
	{
		// DO wireframes as well
		SetRecalculateWireFrameFlagRadius(sGridNo, ubRadius);
		CalculateWorldWireFrameTiles( FALSE );

		RecompileLocalMovementCostsInAreaWithFlags();
		RecompileLocalMovementCostsFromRadius( sGridNo, MAX_DISTANCE_EXPLOSIVE_CAN_DESTROY_STRUCTURES );

		// if anything has been done to change movement costs and this is a potential POW situation, check
		// paths for POWs
		if (gWorldSector.x == 13 && gWorldSector.y == MAP_ROW_I)
		{
			DoPOWPathChecks();
		}

	}

	// do sight checks if something damaged or smoke stuff involved
	if ( fRecompileMovement || fSmokeEffect )
	{
		if ( gubElementsOnExplosionQueue )
		{
			gfExplosionQueueMayHaveChangedSight	= TRUE;
		}
	}

	gsRecompileAreaTop = 0;
	gsRecompileAreaLeft = 0;
	gsRecompileAreaRight = 0;
	gsRecompileAreaBottom = 0;

	if (fAnyMercHit)
	{
		// reset explosion hit flag so we can damage mercs again
		FOR_EACH_MERC(i) (*i)->ubMiscSoldierFlags &= ~SOLDIER_MISC_HURT_BY_EXPLOSION;
	}

	if ( fSubsequent != BLOOD_SPREAD_EFFECT )
	{
		MakeNoise(NULL, sGridNo, bLevel, Explosive[GCM->getItem(usItem)->getClassIndex()].ubVolume, NOISE_EXPLOSION);
	}
}


void SpreadEffectSmoke(const SMOKEEFFECT* const s, const BOOLEAN subsequent, const INT8 level)
{
	SpreadEffect(s->sGridNo, s->ubRadius, s->usItem, s->owner, subsequent, level, s);
}


static void ToggleActionItemsByFrequency(INT8 bFrequency)
{
	// Go through all the bombs in the world, and look for remote ones
	CFOR_EACH_WORLD_BOMB(wb)
	{
		OBJECTTYPE& o = GetWorldItem(wb.iItemIndex).o;
		if (o.bDetonatorType == BOMB_REMOTE)
		{
			// Found a remote bomb, so check to see if it has the same frequency
			if (o.bFrequency == bFrequency)
			{
				// toggle its active flag
				if (o.fFlags & OBJECT_DISABLED_BOMB)
				{
					o.fFlags &= ~OBJECT_DISABLED_BOMB;
				}
				else
				{
					o.fFlags |= OBJECT_DISABLED_BOMB;
				}
			}
		}
	}
}


static void TogglePressureActionItemsInGridNo(INT16 sGridNo)
{
	// Go through all the bombs in the world, and look for remote ones
	CFOR_EACH_WORLD_BOMB(wb)
	{
		WORLDITEM& wi = GetWorldItem(wb.iItemIndex);
		if (wi.sGridNo != sGridNo) continue;

		OBJECTTYPE& o = wi.o;
		if (o.bDetonatorType == BOMB_PRESSURE)
		{
			// Found a pressure item
			// toggle its active flag
			if (o.fFlags & OBJECT_DISABLED_BOMB)
			{
				o.fFlags &= ~OBJECT_DISABLED_BOMB;
			}
			else
			{
				o.fFlags |= OBJECT_DISABLED_BOMB;
			}
		}
	}
}


static BOOLEAN HookerInRoom(UINT8 ubRoom)
{
	FOR_EACH_IN_TEAM(s, CIV_TEAM)
	{
		if (!s->bInSector)                 continue;
		if (s->bLife < OKLIFE)             continue;
		if (!s->bNeutral)                  continue;
		if (s->ubBodyType != MINICIV)      continue;
		if (GetRoom(s->sGridNo) != ubRoom) continue;
		return TRUE;
	}
	return FALSE;
}

static void PerformItemAction(INT16 sGridNo, OBJECTTYPE* pObj)
{
	STRUCTURE * pStructure;

	switch( pObj->bActionValue )
	{
		case ACTION_ITEM_OPEN_DOOR:
			pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
			if (pStructure)
			{
				if (pStructure->fFlags & STRUCTURE_OPEN)
				{
					// it's already open - this MIGHT be an error but probably not
					// because we are basically just ensuring that the door is open
				}
				else
				{
					if (pStructure->fFlags & STRUCTURE_BASE_TILE)
					{
						HandleDoorChangeFromGridNo( NULL, sGridNo, FALSE );
					}
					else
					{
						HandleDoorChangeFromGridNo( NULL, pStructure->sBaseGridNo, FALSE );
					}
					gfExplosionQueueMayHaveChangedSight = TRUE;
				}
			}
			else
			{
				// error message here
				SLOGW("Action item to open door in gridno {} but there is none!", sGridNo);
			}
			break;
		case ACTION_ITEM_CLOSE_DOOR:
			pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
			if (pStructure)
			{
				if (pStructure->fFlags & STRUCTURE_OPEN)
				{
					if (pStructure->fFlags & STRUCTURE_BASE_TILE)
					{
						HandleDoorChangeFromGridNo( NULL, sGridNo , FALSE );
					}
					else
					{
						HandleDoorChangeFromGridNo( NULL, pStructure->sBaseGridNo, FALSE );
					}
					gfExplosionQueueMayHaveChangedSight = TRUE;
				}
				else
				{
					// it's already closed - this MIGHT be an error but probably not
					// because we are basically just ensuring that the door is closed
				}
			}
			else
			{
				// error message here
				SLOGW("Action item to close door in gridno {} but there is none!", sGridNo);
			}
			break;
		case ACTION_ITEM_TOGGLE_DOOR:
			pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
			if (pStructure)
			{
				if (pStructure->fFlags & STRUCTURE_BASE_TILE)
				{
					HandleDoorChangeFromGridNo( NULL, sGridNo, FALSE );
				}
				else
				{
					HandleDoorChangeFromGridNo( NULL, pStructure->sBaseGridNo , FALSE );
				}
				gfExplosionQueueMayHaveChangedSight = TRUE;
			}
			else
			{
				// error message here
				SLOGW("Action item to toggle door in gridno {} but there is none!", sGridNo);
			}
			break;
		case ACTION_ITEM_UNLOCK_DOOR:
			{
				DOOR * pDoor;

				pDoor = FindDoorInfoAtGridNo( sGridNo );
				if ( pDoor )
				{
					pDoor->fLocked = FALSE;
				}
			}
			break;
		case ACTION_ITEM_TOGGLE_LOCK:
			{
				DOOR * pDoor;

				pDoor = FindDoorInfoAtGridNo( sGridNo );
				if ( pDoor )
				{
					if ( pDoor->fLocked )
					{
						pDoor->fLocked = FALSE;
					}
					else
					{
						pDoor->fLocked = TRUE;
					}
				}
			}
			break;
		case ACTION_ITEM_UNTRAP_DOOR:
			{
				DOOR * pDoor;

				pDoor = FindDoorInfoAtGridNo( sGridNo );
				if ( pDoor )
				{
					pDoor->ubTrapLevel = 0;
					pDoor->ubTrapID = NO_TRAP;
				}
			}
			break;
		case ACTION_ITEM_SMALL_PIT:
			Add3X3Pit( sGridNo );
			SearchForOtherMembersWithinPitRadiusAndMakeThemFall( sGridNo, 1 );
			break;
		case ACTION_ITEM_LARGE_PIT:
			Add5X5Pit( sGridNo );
			SearchForOtherMembersWithinPitRadiusAndMakeThemFall( sGridNo, 2 );
			break;
		case ACTION_ITEM_TOGGLE_ACTION1:
			ToggleActionItemsByFrequency( FIRST_MAP_PLACED_FREQUENCY + 1 );
			break;
		case ACTION_ITEM_TOGGLE_ACTION2:
			ToggleActionItemsByFrequency( FIRST_MAP_PLACED_FREQUENCY + 2 );
			break;
		case ACTION_ITEM_TOGGLE_ACTION3:
			ToggleActionItemsByFrequency( FIRST_MAP_PLACED_FREQUENCY + 3 );
			break;
		case ACTION_ITEM_TOGGLE_ACTION4:
			ToggleActionItemsByFrequency( FIRST_MAP_PLACED_FREQUENCY + 4 );
			break;
		case ACTION_ITEM_TOGGLE_PRESSURE_ITEMS:
			TogglePressureActionItemsInGridNo( sGridNo );
			break;
		case ACTION_ITEM_ENTER_BROTHEL:
			// JA2Gold: Disable brothel tracking
			/*
			if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
			{
				const SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, 0);
				if (tgt != NULL && tgt->bTeam == OUR_TEAM)
				{
					if (tgt->sOldGridNo == sGridNo + DirectionInc(SOUTH))
					{
						gMercProfiles[ MADAME ].bNPCData2++;

						SetFactTrue( FACT_PLAYER_USED_BROTHEL );
						SetFactTrue( FACT_PLAYER_PASSED_GOON );

						// If we for any reason trigger Madame's record 34 then we don't bother to do
						// anything else

						// Billy always moves back on a timer so that the player has a chance to sneak
						// someone else through

						// Madame's quote about female mercs should therefore not be made on a timer

						if ( gMercProfiles[ MADAME ].bNPCData2 > 2 )
						{
							// more than 2 entering brothel
							TriggerNPCRecord( MADAME, 35 );
							return;
						}

						if ( gMercProfiles[ MADAME ].bNPCData2 == gMercProfiles[ MADAME ].bNPCData )
						{
							// full # of mercs who paid have entered brothel
							// have Billy block the way again
							SetCustomizableTimerCallbackAndDelay( 2000, BillyBlocksDoorCallback, FALSE );
							//TriggerNPCRecord( BILLY, 6 );
						}
						else if ( gMercProfiles[ MADAME ].bNPCData2 > gMercProfiles[ MADAME ].bNPCData )
						{
							// more than full # of mercs who paid have entered brothel
							// have Billy block the way again?
							if ( CheckFact( FACT_PLAYER_FORCED_WAY_INTO_BROTHEL, 0 ) )
							{
								// player already did this once!
								TriggerNPCRecord( MADAME, 35 );
								return;
							}
							else
							{
								SetCustomizableTimerCallbackAndDelay( 2000, BillyBlocksDoorCallback, FALSE );
								SetFactTrue( FACT_PLAYER_FORCED_WAY_INTO_BROTHEL );
								TriggerNPCRecord( MADAME, 34 );
							}
						}

						if (gMercProfiles[tgt->ubProfile].bSex == FEMALE)
						{
							// woman walking into brothel
							TriggerNPCRecordImmediately( MADAME, 33 );
						}

					}
					else
					{
						// someone wants to leave the brothel
						TriggerNPCRecord( BILLY, 5 );
					}

				}

			}
			*/
			break;
		case ACTION_ITEM_EXIT_BROTHEL:
			// JA2Gold: Disable brothel tracking
			/*
			if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
			{
				const SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, 0);
				if (tgt != NULL && tgt->bTeam == OUR_TEAM && tgt->sOldGridNo == sGridNo + DirectionInc(NORTH))
				{
					gMercProfiles[ MADAME ].bNPCData2--;
					if ( gMercProfiles[ MADAME ].bNPCData2 == 0 )
					{
						// reset paid #
						gMercProfiles[ MADAME ].bNPCData = 0;
					}
					// Billy should move back to block the door again
					gsTempActionGridNo = sGridNo;
					SetCustomizableTimerCallbackAndDelay( 1000, DelayedBillyTriggerToBlockOnExit, TRUE );
				}
			}
			*/
			break;
		case ACTION_ITEM_KINGPIN_ALARM:
			PlayLocationJA2Sample(sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			CallAvailableKingpinMenTo( sGridNo );

			gTacticalStatus.fCivGroupHostile[ KINGPIN_CIV_GROUP ] = CIV_GROUP_HOSTILE;

			{
				FOR_EACH_IN_TEAM(civ, CIV_TEAM)
				{
					if (civ->bInSector && civ->ubCivilianGroup == KINGPIN_CIV_GROUP)
					{
						for (UINT8 ubID2 = gTacticalStatus.Team[OUR_TEAM].bFirstID; ubID2 <= gTacticalStatus.Team[OUR_TEAM].bLastID; ++ubID2)
						{
							if (civ->bOppList[ubID2] == SEEN_CURRENTLY)
							{
								MakeCivHostile(civ, 2);
							}
						}
					}
				}

				if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
				{
					EnterCombatMode( CIV_TEAM );
				}
			}

			// now zap this object so it won't activate again
			pObj->fFlags &= (~OBJECT_DISABLED_BOMB);
			break;
		case ACTION_ITEM_SEX:
			if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
			{
				OBJECTTYPE DoorCloser;
				INT16		sTeleportSpot;
				INT16		sDoorSpot;
				UINT8		ubDirection;

				SOLDIERTYPE* tgt = WhoIsThere2(sGridNo, 0);
				if (tgt != NULL)
					if (tgt->bTeam == OUR_TEAM)
					{
						UINT8 const room     = GetRoom(sGridNo);
						UINT8 const old_room = GetRoom(tgt->sOldGridNo);
						if (room != NO_ROOM && old_room != NO_ROOM && old_room != room)
						{
							// also require there to be a miniskirt civ in the room
							if (HookerInRoom(room))
							{

								// stop the merc...
								EVENT_StopMerc(tgt);

								switch( sGridNo )
								{
									case 13414:
										sDoorSpot = 13413;
										sTeleportSpot = 13413;
										break;
									case 11174:
										sDoorSpot = 11173;
										sTeleportSpot = 11173;
										break;
									case 12290:
										sDoorSpot = 12290;
										sTeleportSpot = 12291;
										break;

									default:

										sDoorSpot = NOWHERE;
										sTeleportSpot = NOWHERE;


								}

								if ( sDoorSpot != NOWHERE && sTeleportSpot != NOWHERE )
								{
									// close the door...
									DoorCloser.bActionValue = ACTION_ITEM_CLOSE_DOOR;
									PerformItemAction( sDoorSpot, &DoorCloser );

									// have sex
									HandleNPCDoAction( 0, NPC_ACTION_SEX, 0 );

									// move the merc outside of the room again
									sTeleportSpot = FindGridNoFromSweetSpotWithStructData(tgt, STANDING, sTeleportSpot, 2, &ubDirection, FALSE);
									ChangeSoldierState(tgt, STANDING, 0, TRUE);
									TeleportSoldier(*tgt, sTeleportSpot, false);

									HandleMoraleEvent(tgt, MORALE_SEX, gWorldSector);
									FatigueCharacter(*tgt);
									FatigueCharacter(*tgt);
									FatigueCharacter(*tgt);
									FatigueCharacter(*tgt);
									DirtyMercPanelInterface(tgt, DIRTYLEVEL1);
								}
							}

						}
						break;
					}
			}
			break;
		case ACTION_ITEM_REVEAL_ROOM:
			{
				UINT8 ubRoom;
				if ( InAHiddenRoom( sGridNo, &ubRoom ) )
				{
					RemoveRoomRoof( sGridNo, ubRoom, NULL );
				}
			}
			break;
		case ACTION_ITEM_LOCAL_ALARM:
			MakeNoise(NULL, sGridNo, 0, 30, NOISE_SILENT_ALARM);
			break;
		case ACTION_ITEM_GLOBAL_ALARM:
			CallAvailableEnemiesTo( sGridNo );
			break;
		case ACTION_ITEM_BLOODCAT_ALARM:
			CallAvailableTeamEnemiesTo( sGridNo, CREATURE_TEAM );
			break;
		case ACTION_ITEM_KLAXON:
			PlayLocationJA2Sample(sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			break;
		case ACTION_ITEM_MUSEUM_ALARM:
			PlayLocationJA2Sample(sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			CallEldinTo( sGridNo );
			break;
		default:
			// error message here
			SLOGW("Action item with invalid action in gridno {}!", sGridNo);
			break;
	}
}


static void AddBombToQueue(UINT32 const world_bomb_idx, UINT32 const timestamp)
{
	if (gubElementsOnExplosionQueue == MAX_BOMB_QUEUE) return; // XXX exception?

	ExplosionQueueElement& e = gExplosionQueue[gubElementsOnExplosionQueue++];
	e.uiWorldBombIndex = world_bomb_idx;
	e.uiTimeStamp      = timestamp;
	e.fExists          = TRUE;

	if (!gfExplosionQueueActive)
	{
		gfExplosionQueueActive   = TRUE;
		guiPendingOverrideEvent  = LU_BEGINUILOCK; // Lock UI
		gTacticalStatus.uiFlags |= DISALLOW_SIGHT; // Disable sight
	}
}


void HandleExplosionQueue()
{
	if (!gfExplosionQueueActive) return;

	UINT32 const now = GetJA2Clock();
	for (UINT32 i = 0; i != gubElementsOnExplosionQueue; ++i)
	{
		ExplosionQueueElement& e = gExplosionQueue[i];
		if (!e.fExists)          continue;
		if (now < e.uiTimeStamp) continue;

		// Set off this bomb now.
		WORLDITEM&  wi     = GetWorldItem(gWorldBombs[e.uiWorldBombIndex].iItemIndex);
		OBJECTTYPE& o      = wi.o;
		INT16 const gridno = wi.sGridNo;
		UINT8 const level  = wi.ubLevel;

		if (o.usItem == ACTION_ITEM && o.bActionValue != ACTION_ITEM_BLOW_UP)
		{
			PerformItemAction(gridno, &o);
		}
		else if (o.usBombItem == TRIP_KLAXON)
		{
			PlayLocationJA2Sample(gridno, KLAXON_ALARM, MIDVOLUME, 5);
			CallAvailableEnemiesTo(gridno);
		}
		else if (o.usBombItem == TRIP_FLARE)
		{
			NewLightEffect(gridno, LIGHT_FLARE_MARK_1);
			RemoveItemFromPool(wi);
		}
		else
		{
			gfExplosionQueueMayHaveChangedSight = TRUE;

			/* Remove the item first to prevent the explosion from detonating it a
			 * second time. */
			RemoveItemFromPool(wi);

			// Make sure no one thinks there is a bomb here any more
			UINT16& flags = gpWorldLevelData[gridno].uiFlags;
			if (flags & MAPELEMENT_PLAYER_MINE_PRESENT)
			{
				RemoveBlueFlag(gridno, level);
			}
			flags &= ~MAPELEMENT_ENEMY_MINE_PRESENT;

			// Bomb objects only store the side who placed the bomb.
			SOLDIERTYPE* const owner = o.ubBombOwner > 1 ? ID2SOLDIER(o.ubBombOwner - 2) : 0;
			IgniteExplosion(owner, 0, gridno, o.usBombItem, level);
		}

		e.fExists = FALSE;
	}

	/* See if we can reduce the # of elements on the queue that we have recorded.
	 * Easier to do it at this time rather than in the loop above. */
	while (gubElementsOnExplosionQueue > 0 && !gExplosionQueue[gubElementsOnExplosionQueue - 1].fExists)
	{
		--gubElementsOnExplosionQueue;
	}

	TacticalStatusType& ts = gTacticalStatus;
	if (gubElementsOnExplosionQueue == 0 &&
			(!gPersonToSetOffExplosions || ts.ubAttackBusyCount == 0))
	{ // Turn off explosion queue
		ts.uiFlags &= ~DISALLOW_SIGHT; // Re-enable sight

		SOLDIERTYPE* const s = gPersonToSetOffExplosions;
		if (s && !(s->uiStatusFlags & SOLDIER_PC))
		{
			FreeUpNPCFromPendingAction(s);
		}

		if (gfExplosionQueueMayHaveChangedSight)
		{
			// Set variable so we may at least have someone to resolve interrupts against
			gInterruptProvoker = s;
			AllTeamsLookForAll(TRUE);

			// call fov code
			FOR_EACH_IN_TEAM(s, OUR_TEAM)
			{
				if (s->bInSector) RevealRoofsAndItems(s, FALSE);
			}

			gfExplosionQueueMayHaveChangedSight = FALSE;
			gPersonToSetOffExplosions           = 0;
		}

		if (!(ts.uiFlags & INCOMBAT) || ts.ubCurrentTeam == OUR_TEAM)
		{ // Don't end UI lock when it's a computer turn
			guiPendingOverrideEvent = LU_ENDUILOCK;
		}

		gfExplosionQueueActive = FALSE;
	}
}


void DecayBombTimers( void )
{
	UINT32				uiWorldBombIndex;
	UINT32				uiTimeStamp;

	uiTimeStamp = GetJA2Clock();

	// Go through all the bombs in the world, and look for timed ones
	Assert(gWorldBombs.size() <= UINT32_MAX);
	for (uiWorldBombIndex = 0; uiWorldBombIndex < static_cast<UINT32>(gWorldBombs.size()); uiWorldBombIndex++)
	{
		if (gWorldBombs[uiWorldBombIndex].fExists)
		{
			OBJECTTYPE& o = GetWorldItem(gWorldBombs[uiWorldBombIndex].iItemIndex).o;
			if (o.bDetonatorType == BOMB_TIMED && !(o.fFlags & OBJECT_DISABLED_BOMB))
			{
				// Found a timed bomb, so decay its delay value and see if it goes off
				o.bDelay--;
				if (o.bDelay == 0)
				{
					// put this bomb on the queue
					AddBombToQueue( uiWorldBombIndex, uiTimeStamp );
					// ATE: CC black magic....
					if (o.ubBombOwner > 1)
					{
						gPersonToSetOffExplosions = &GetMan(o.ubBombOwner - 2);
					}
					else
					{
						gPersonToSetOffExplosions = NULL;
					}

					if (o.usItem != ACTION_ITEM || o.bActionValue == ACTION_ITEM_BLOW_UP)
					{
						uiTimeStamp += BOMB_QUEUE_DELAY;
					}
				}
			}
		}
	}
}


void SetOffBombsByFrequency(SOLDIERTYPE* const s, const INT8 bFrequency)
{
	UINT32 uiWorldBombIndex;
	UINT32 uiTimeStamp;

	uiTimeStamp = GetJA2Clock();

	// Go through all the bombs in the world, and look for remote ones
	Assert(gWorldBombs.size() <= UINT32_MAX);
	for (uiWorldBombIndex = 0; uiWorldBombIndex < static_cast<UINT32>(gWorldBombs.size()); uiWorldBombIndex++)
	{
		if (gWorldBombs[uiWorldBombIndex].fExists)
		{
			OBJECTTYPE const& o = GetWorldItem(gWorldBombs[uiWorldBombIndex].iItemIndex).o;
			if (o.bDetonatorType == BOMB_REMOTE && !(o.fFlags & OBJECT_DISABLED_BOMB))
			{
				// Found a remote bomb, so check to see if it has the same frequency
				if (o.bFrequency == bFrequency)
				{
					gPersonToSetOffExplosions = s;

					// put this bomb on the queue
					AddBombToQueue( uiWorldBombIndex, uiTimeStamp );
					if (o.usItem != ACTION_ITEM || o.bActionValue == ACTION_ITEM_BLOW_UP)
					{
						uiTimeStamp += BOMB_QUEUE_DELAY;
					}
				}
			}
		}
	}
}


void SetOffPanicBombs(SOLDIERTYPE* const s, const INT8 bPanicTrigger)
{
	// need to turn off gridnos & flags in gTacticalStatus
	gTacticalStatus.sPanicTriggerGridNo[ bPanicTrigger ] = NOWHERE;
	if ( (gTacticalStatus.sPanicTriggerGridNo[0] == NOWHERE) &&
		(gTacticalStatus.sPanicTriggerGridNo[1] == NOWHERE) &&
		(gTacticalStatus.sPanicTriggerGridNo[2] == NOWHERE) )
	{
		gTacticalStatus.fPanicFlags &= ~(PANIC_TRIGGERS_HERE);
	}

	switch( bPanicTrigger )
	{
		case 0:
			SetOffBombsByFrequency(s, PANIC_FREQUENCY);
			gTacticalStatus.fPanicFlags &= ~(PANIC_BOMBS_HERE);
			break;

		case 1: SetOffBombsByFrequency(s, PANIC_FREQUENCY_2); break;
		case 2: SetOffBombsByFrequency(s, PANIC_FREQUENCY_3); break;

		default:
			break;

	}

	if ( gTacticalStatus.fPanicFlags )
	{
		// find a new "closest one"
		MakeClosestEnemyChosenOne();
	}
}


BOOLEAN SetOffBombsInGridNo(SOLDIERTYPE* const s, const INT16 sGridNo, const BOOLEAN fAllBombs, const INT8 bLevel)
{
	UINT32  uiWorldBombIndex;
	UINT32  uiTimeStamp;
	BOOLEAN fFoundMine = FALSE;

	uiTimeStamp = GetJA2Clock();

	// Go through all the bombs in the world, and look for mines at this location
	Assert(gWorldBombs.size() <= UINT32_MAX);
	for (uiWorldBombIndex = 0; uiWorldBombIndex < static_cast<UINT32>(gWorldBombs.size()); uiWorldBombIndex++)
	{
		if (!gWorldBombs[uiWorldBombIndex].fExists) continue;

		WORLDITEM const& wi = GetWorldItem(gWorldBombs[uiWorldBombIndex].iItemIndex);
		if (wi.sGridNo != sGridNo || wi.ubLevel != bLevel) continue;

		OBJECTTYPE const& o = wi.o;
		if (!(o.fFlags & OBJECT_DISABLED_BOMB))
		{
			if (fAllBombs || o.bDetonatorType == BOMB_PRESSURE)
			{
				if (!fAllBombs && s->bTeam != OUR_TEAM)
				{
					// ignore this unless it is a mine, etc which would have to have been placed by the
					// player, seeing as how the others are all marked as known to the AI.
					if (o.usItem != MINE && o.usItem != TRIP_FLARE && o.usItem != TRIP_KLAXON)
					{
						continue;
					}
				}

				// player and militia ignore bombs set by player
				if (o.ubBombOwner > 1 &&
						(s->bTeam == OUR_TEAM || s->bTeam == MILITIA_TEAM))
				{
					continue;
				}

				if (o.usItem == SWITCH)
				{
					// send out a signal to detonate other bombs, rather than this which
					// isn't a bomb but a trigger
					SetOffBombsByFrequency(s, o.bFrequency);
				}
				else
				{
					gPersonToSetOffExplosions = s;

					// put this bomb on the queue
					AddBombToQueue( uiWorldBombIndex, uiTimeStamp );
					if (o.usItem != ACTION_ITEM || o.bActionValue == ACTION_ITEM_BLOW_UP)
					{
						uiTimeStamp += BOMB_QUEUE_DELAY;
					}

					if (o.usBombItem != NOTHING && GCM->getItem(o.usBombItem)->isExplosive())
					{
						fFoundMine = TRUE;
					}

				}
			}
		}
	}
	return( fFoundMine );
}


void ActivateSwitchInGridNo(SOLDIERTYPE* const s, const INT16 sGridNo)
{
	// Go through all the bombs in the world, and look for mines at this location
	CFOR_EACH_WORLD_BOMB(wb)
	{
		WORLDITEM const& wi = GetWorldItem(wb.iItemIndex);
		if (wi.sGridNo != sGridNo) continue;

		OBJECTTYPE const& o = wi.o;
		if (o.usItem == SWITCH && !(o.fFlags & OBJECT_DISABLED_BOMB) && o.bDetonatorType == BOMB_SWITCH)
		{
			// send out a signal to detonate other bombs, rather than this which
			// isn't a bomb but a trigger

			// first set attack busy count to 0 in case of a lingering a.b.c. problem...
			gTacticalStatus.ubAttackBusyCount = 0;

			SetOffBombsByFrequency(s, o.bFrequency);
		}
	}
}


void SaveExplosionTableToSaveGameFile(HWFILE const hFile)
{
	UINT32 uiExplosionCount=0;
	UINT32 uiCnt;


	//
	// Explosion queue Info
	//


	//Write the number of explosion queues
	hFile->write(&gubElementsOnExplosionQueue, sizeof(gubElementsOnExplosionQueue));
	hFile->seek(3, FILE_SEEK_FROM_CURRENT);

	//loop through and add all the explosions
	FOR_EACH(ExplosionQueueElement const, i, gExplosionQueue)
	{
		ExplosionQueueElement const& e = *i;
		BYTE  data[12];
		DataWriter d{data};
		INJ_U32( d, e.uiWorldBombIndex)
		INJ_U32( d, e.uiTimeStamp)
		INJ_U8(  d, e.fExists)
		INJ_SKIP(d, 3)
		Assert(d.getConsumed() == lengthof(data));
		hFile->write(data, sizeof(data));
	}

	//
	//	Explosion Data
	//

	//loop through and count all the active explosions
	uiExplosionCount = 0;
	for( uiCnt=0; uiCnt< NUM_EXPLOSION_SLOTS; uiCnt++)
	{
		if( gExplosionData[ uiCnt ].fAllocated )
		{
			uiExplosionCount++;
		}
	}

	//Save the number of explosions
	hFile->write(&uiExplosionCount, sizeof(UINT32));

	//loop through and count all the active explosions
	for( uiCnt=0; uiCnt< NUM_EXPLOSION_SLOTS; uiCnt++)
	{
		const EXPLOSIONTYPE* const e = &gExplosionData[uiCnt];
		if (e->fAllocated)
		{
			InjectExplosionTypeIntoFile(hFile, e);
		}
	}
}


void LoadExplosionTableFromSavedGameFile(HWFILE const hFile)
{
	//
	// Explosion Queue
	//

	//Read the number of explosions queue's
	hFile->read(&gubElementsOnExplosionQueue, sizeof(gubElementsOnExplosionQueue));
	hFile->seek(3, FILE_SEEK_FROM_CURRENT);

	//loop through read all the active explosions fro the file
	FOR_EACH(ExplosionQueueElement, i, gExplosionQueue)
	{
		BYTE  data[12];
		hFile->read(data, sizeof(data));
		DataReader d{data};
		ExplosionQueueElement& e = *i;
		EXTR_U32( d, e.uiWorldBombIndex)
		EXTR_U32( d, e.uiTimeStamp)
		EXTR_U8(  d, e.fExists)
		EXTR_SKIP(d, 3)
		Assert(d.getConsumed() == lengthof(data));
	}

	//
	//	Explosion Data
	//

	//Load the number of explosions
	UINT32 num_explosions;
	hFile->read(&num_explosions, sizeof(num_explosions));

	//loop through and load all the active explosions
	const EXPLOSIONTYPE* const end = gExplosionData + num_explosions;
	for (EXPLOSIONTYPE* e = gExplosionData; e != end; ++e)
	{
		ExtractExplosionTypeFromFile(hFile, e);
		GenerateExplosionFromExplosionPointer(e);
	}
}

// loop through civ team and find
// anybody who is an NPC and
// see if they get angry
static void HandleBuldingDestruction(const INT16 sGridNo, const SOLDIERTYPE* const owner)
{
	if (owner == NULL || owner->bTeam != OUR_TEAM) return;

	FOR_EACH_IN_TEAM(pSoldier, CIV_TEAM)
	{
		if (pSoldier->bInSector && pSoldier->bLife && pSoldier->bNeutral)
		{
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				// ignore if the player is fighting the enemy here and this is a good guy
				if (IsTeamActive(ENEMY_TEAM) && gMercProfiles[pSoldier->ubProfile].ubMiscFlags3 & PROFILE_MISC_FLAG3_GOODGUY)
				{
					continue;
				}

				if ( DoesNPCOwnBuilding( pSoldier, sGridNo ) )
				{
					MakeNPCGrumpyForMinorOffense(pSoldier, owner);
				}
			}
		}
	}
}


static INT32 FindActiveTimedBomb(void)
{
	// Go through all the bombs in the world, and look for timed ones
	FOR_EACH_WORLD_BOMB(wb)
	{
		OBJECTTYPE const& o = GetWorldItem(wb.iItemIndex).o;
		if (o.bDetonatorType != BOMB_TIMED || o.fFlags & OBJECT_DISABLED_BOMB) continue;

		return wb.iItemIndex;
	}
	return -1;
}


BOOLEAN ActiveTimedBombExists(void)
{
	return gfWorldLoaded && FindActiveTimedBomb() != -1;
}


void RemoveAllActiveTimedBombs(void)
{
	for (;;)
	{
		const INT32 item_idx = FindActiveTimedBomb();
		if (item_idx == -1) break;
		RemoveItemFromWorld(item_idx);
	}
}
