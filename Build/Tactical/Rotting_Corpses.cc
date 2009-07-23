#include "Font_Control.h"
#include "Lighting.h"
#include "Overhead.h"
#include "Soldier_Find.h"
#include "TileDef.h"
#include "VObject.h"
#include "Debug.h"
#include "Soldier_Control.h"
#include "Weapons.h"
#include "Handle_Items.h"
#include "WorldDef.h"
#include "Rotting_Corpses.h"
#include "Tile_Cache.h"
#include "Isometric_Utils.h"
#include "Animation_Control.h"
#include "Utilities.h"
#include "Game_Clock.h"
#include "Soldier_Create.h"
#include "RenderWorld.h"
#include "Soldier_Add.h"
#include "StrategicMap.h"
#include "LOS.h"
#include "Structure.h"
#include "Message.h"
#include "Sound_Control.h"
#include "PathAI.h"
#include "Random.h"
#include "Dialogue_Control.h"
#include "Items.h"
#include "Smell.h"
#include "World_Items.h"
#include "Explosion_Control.h"
#include "GameSettings.h"
#include "Interface_Items.h"
#include "Soldier_Profile.h"
#include "Soldier_Macros.h"
#include "Keys.h"
#include "Render_Fun.h"
#include "Strategic.h"
#include "QArray.h"
#include "Interface.h"
#include "MemMan.h"


#define CORPSE_WARNING_MAX 5
#define CORPSE_WARNING_DIST 5

#define		CORPSE_INDEX_OFFSET		10000

//#define		DELAY_UNTIL_ROTTING		( 1 * NUM_SEC_IN_DAY )
#define		DELAY_UNTIL_ROTTING		    ( 1 * NUM_SEC_IN_DAY / 60 )
#define   DELAY_UNTIL_DONE_ROTTING  ( 3 * NUM_SEC_IN_DAY / 60 )

#define		MAX_NUM_CROWS					6


// When adding a corpse, add struct data...
static const char* const zCorpseFilenames[NUM_CORPSES] =
{
	"",
	"ANIMS/CORPSES/S_D_JFK.STI",
	"ANIMS/CORPSES/S_D_BCK.STI",
	"ANIMS/CORPSES/S_D_FWD.STI",
	"ANIMS/CORPSES/S_D_DHD.STI",
	"ANIMS/CORPSES/S_D_PRN.STI",
	"ANIMS/CORPSES/S_D_WTR.STI",
	"ANIMS/CORPSES/S_D_FALL.STI",
	"ANIMS/CORPSES/S_D_FALLF.STI",

	"ANIMS/CORPSES/M_D_JFK.STI",
	"ANIMS/CORPSES/M_D_BCK.STI",
	"ANIMS/CORPSES/M_D_FWD.STI",
	"ANIMS/CORPSES/M_D_DHD.STI",
	"ANIMS/CORPSES/M_D_PRN.STI",
	"ANIMS/CORPSES/S_D_WTR.STI",
	"ANIMS/CORPSES/M_D_FALL.STI",
	"ANIMS/CORPSES/M_D_FALLF.STI",

	"ANIMS/CORPSES/F_D_JFK.STI",
	"ANIMS/CORPSES/F_D_BCK.STI",
	"ANIMS/CORPSES/F_D_FWD.STI",
	"ANIMS/CORPSES/F_D_DHD.STI",
	"ANIMS/CORPSES/F_D_PRN.STI",
	"ANIMS/CORPSES/S_D_WTR.STI",
	"ANIMS/CORPSES/F_D_FALL.STI",
	"ANIMS/CORPSES/F_D_FALLF.STI",

	// Civs....
	"ANIMS/CORPSES/M_DEAD1.STI",
	"ANIMS/CORPSES/K_DEAD2.STI",
	"ANIMS/CORPSES/H_DEAD2.STI",
	"ANIMS/CORPSES/FT_DEAD1.STI",
	"ANIMS/CORPSES/S_DEAD1.STI",
	"ANIMS/CORPSES/W_DEAD1.STI",
	"ANIMS/CORPSES/CP_DEAD1.STI",
	"ANIMS/CORPSES/M_DEAD2.STI",
	"ANIMS/CORPSES/K_DEAD1.STI",
	"ANIMS/CORPSES/H_DEAD1.STI",

	"ANIMS/CORPSES/FT_DEAD2.STI",
	"ANIMS/CORPSES/S_DEAD2.STI",
	"ANIMS/CORPSES/W_DEAD2.STI",
	"ANIMS/CORPSES/CP_DEAD2.STI",
	"ANIMS/CORPSES/CT_DEAD.STI",
	"ANIMS/CORPSES/CW_DEAD1.STI",
	"ANIMS/CORPSES/MN_DEAD2.STI",
	"ANIMS/CORPSES/I_DEAD1.STI",
	"ANIMS/CORPSES/L_DEAD1.STI",

	"ANIMS/CORPSES/P_DECOMP2.STI",
	"ANIMS/CORPSES/TK_WREK.STI",
	"ANIMS/CORPSES/TK2_WREK.STI",
  "ANIMS/CORPSES/HM_WREK.STI",
  "ANIMS/CORPSES/IC_WREK.STI",
  "ANIMS/CORPSES/QN_DEAD.STI",
  "ANIMS/CORPSES/J_DEAD.STI",
  "ANIMS/CORPSES/S_BURNT.STI",
  "ANIMS/CORPSES/S_EXPLD.STI",
};


// When adding a corpse, add struct data...
static const char* const	zNoBloodCorpseFilenames[NUM_CORPSES] =
{
	"",
	"ANIMS/CORPSES/M_D_JFK_NB.STI",
	"ANIMS/CORPSES/S_D_BCK_NB.STI",
	"ANIMS/CORPSES/S_D_FWD_NB.STI",
	"ANIMS/CORPSES/S_D_DHD_NB.STI",
	"ANIMS/CORPSES/S_D_PRN_NB.STI",
	"ANIMS/CORPSES/S_D_WTR.STI",
	"ANIMS/CORPSES/S_D_FALL_NB.STI",
	"ANIMS/CORPSES/S_D_FALLF_NB.STI",

	"ANIMS/CORPSES/M_D_JFK_NB.STI",
	"ANIMS/CORPSES/M_D_BCK_NB.STI",
	"ANIMS/CORPSES/M_D_FWD_NB.STI",
	"ANIMS/CORPSES/M_D_DHD_NB.STI",
	"ANIMS/CORPSES/M_D_PRN_NB.STI",
	"ANIMS/CORPSES/S_D_WTR.STI",
	"ANIMS/CORPSES/M_D_FALL_NB.STI",
	"ANIMS/CORPSES/M_D_FALLF_NB.STI",

	"ANIMS/CORPSES/F_D_JFK_NB.STI",
	"ANIMS/CORPSES/F_D_BCK_NB.STI",
	"ANIMS/CORPSES/F_D_FWD_NB.STI",
	"ANIMS/CORPSES/F_D_DHD_NB.STI",
	"ANIMS/CORPSES/F_D_PRN_NB.STI",
	"ANIMS/CORPSES/S_D_WTR.STI",
	"ANIMS/CORPSES/F_D_FALL_NB.STI",
	"ANIMS/CORPSES/F_D_FALLF_NB.STI",

	// Civs....
	"ANIMS/CORPSES/M_DEAD1_NB.STI",
	"ANIMS/CORPSES/K_DEAD2_NB.STI",
	"ANIMS/CORPSES/H_DEAD2_NB.STI",
	"ANIMS/CORPSES/FT_DEAD1_NB.STI",
	"ANIMS/CORPSES/S_DEAD1_NB.STI",
	"ANIMS/CORPSES/W_DEAD1_NB.STI",
	"ANIMS/CORPSES/CP_DEAD1_NB.STI",
	"ANIMS/CORPSES/M_DEAD2_NB.STI",
	"ANIMS/CORPSES/K_DEAD1_NB.STI",
	"ANIMS/CORPSES/H_DEAD1_NB.STI",

	"ANIMS/CORPSES/FT_DEAD2_NB.STI",
	"ANIMS/CORPSES/S_DEAD2_NB.STI",
	"ANIMS/CORPSES/W_DEAD2_NB.STI",
	"ANIMS/CORPSES/CP_DEAD2_NB.STI",
	"ANIMS/CORPSES/CT_DEAD.STI",
	"ANIMS/CORPSES/CW_DEAD1.STI",
	"ANIMS/CORPSES/MN_DEAD2.STI",
	"ANIMS/CORPSES/I_DEAD1.STI",
	"ANIMS/CORPSES/L_DEAD1.STI",
	"ANIMS/CORPSES/P_DECOMP2.STI",

	"ANIMS/CORPSES/TK_WREK.STI",
	"ANIMS/CORPSES/TK2_WREK.STI",
  "ANIMS/CORPSES/HM_WREK.STI",
  "ANIMS/CORPSES/IC_WREK.STI",
  "ANIMS/CORPSES/QN_DEAD.STI",
  "ANIMS/CORPSES/J_DEAD.STI",
  "ANIMS/CORPSES/S_BURNT.STI",
  "ANIMS/CORPSES/S_EXPLD.STI",
};

UINT8		gb4DirectionsFrom8[8] =
{
	7,		// NORTH
	0,		// NE
	0,		// E
	0,		// SE
	1,		// S
	0,		// SW,
	2,		// W,
	0			// NW
};


static const UINT8 gb2DirectionsFrom8[8] =
{
	0,		// NORTH
	7,		// NE
	7,		// E
	7,		// SE
	0,		// S
	7,		// SW,
	7,		// W,
	7			// NW
};


static const BOOLEAN gbCorpseValidForDecapitation[NUM_CORPSES] =
{
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,

	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,

	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,

	// Civs....
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,

	1,
	1,
	1,
	1,
	0,
	0,
	0,
  0,
  0,
  1,

  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};


static const INT8 gDecapitatedCorpse[NUM_CORPSES] =
{
	0,
	SMERC_JFK,
	SMERC_JFK,
	SMERC_JFK,
	SMERC_JFK,
	SMERC_JFK,
	SMERC_JFK,
	SMERC_JFK,
	SMERC_JFK,

	MMERC_JFK,
	MMERC_JFK,
	MMERC_JFK,
	MMERC_JFK,
	MMERC_JFK,
	MMERC_JFK,
	MMERC_JFK,
	MMERC_JFK,

	FMERC_JFK,
	FMERC_JFK,
	FMERC_JFK,
	FMERC_JFK,
	FMERC_JFK,
	FMERC_JFK,
	FMERC_JFK,
	FMERC_JFK,

	// Civs....
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	0,
	0,
	0,
	0,
  0,
  0,
  0,
  0,
  0,
  0,

	0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};



ROTTING_CORPSE	gRottingCorpse[ MAX_ROTTING_CORPSES ];
INT32						giNumRottingCorpse = 0;


static ROTTING_CORPSE* GetFreeRottingCorpse(void)
{
	for (ROTTING_CORPSE* c = gRottingCorpse; c != gRottingCorpse + giNumRottingCorpse; ++c)
	{
		if (!c->fActivated) return c;
	}
	if (giNumRottingCorpse < MAX_ROTTING_CORPSES)
	{
		return &gRottingCorpse[giNumRottingCorpse++];
	}
	return NULL;
}


UINT16 GetCorpseStructIndex(const ROTTING_CORPSE_DEFINITION* pCorpseDef, BOOLEAN fForImage)
{
	INT8		bDirection;

	switch( pCorpseDef->ubType )
	{
    case QUEEN_MONSTER_DEAD:
    case BURNT_DEAD:
    case EXPLODE_DEAD:

      bDirection = 0;
      break;

    case ICECREAM_DEAD:
    case HUMMER_DEAD:

			// OK , these have 2 directions....
			bDirection = gb2DirectionsFrom8[ pCorpseDef->bDirection ];
			if (fForImage) bDirection = OneCDirection(bDirection);
			break;

		case SMERC_FALL:
		case SMERC_FALLF:
		case MMERC_FALL:
		case MMERC_FALLF:
		case FMERC_FALL:
		case FMERC_FALLF:

			// OK , these have 4 directions....
			bDirection = gb4DirectionsFrom8[ pCorpseDef->bDirection ];

      if ( fForImage )
      {
			  bDirection = OneCDirection(bDirection);
      }
			break;

		default:

			// Uses 8
			bDirection =  pCorpseDef->bDirection;
			if (fForImage) bDirection = OneCDirection(bDirection);
			break;
	}

	return( bDirection );
}


static void CreateCorpsePalette(ROTTING_CORPSE*);


ROTTING_CORPSE* AddRottingCorpse(ROTTING_CORPSE_DEFINITION* const pCorpseDef)
try
{
	if (pCorpseDef->sGridNo == NOWHERE)   return NULL;
	if (pCorpseDef->ubType  == NO_CORPSE) return NULL;

	ROTTING_CORPSE* const c = GetFreeRottingCorpse();
	if (c == NULL) return NULL;

	// Copy elements in
	c->def = *pCorpseDef;

	// If we are a soecial type...
	AnimationFlags uiDirectionUseFlag;
	switch (pCorpseDef->ubType)
	{
		case SMERC_FALL:
		case SMERC_FALLF:
		case MMERC_FALL:
		case MMERC_FALLF:
		case FMERC_FALL:
		case FMERC_FALLF: uiDirectionUseFlag = ANITILE_USE_4DIRECTION_FOR_START_FRAME; break;
		default:          uiDirectionUseFlag = ANITILE_USE_DIRECTION_FOR_START_FRAME;  break;
	}

	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		// OK, AS WE ADD, CHECK FOR TOD AND DECAY APPROPRIATELY
		if (GetWorldTotalMin() - c->def.uiTimeOfDeath > DELAY_UNTIL_ROTTING &&
				c->def.ubType < ROTTING_STAGE2 &&
				c->def.ubType <= FMERC_FALLF)
		{
			// Rott!
			c->def.ubType = ROTTING_STAGE2;
		}

    // If time of death is a few days, now, don't add at all!
		if (GetWorldTotalMin() - c->def.uiTimeOfDeath > DELAY_UNTIL_DONE_ROTTING) return NULL;
	}

	// Check if on roof or not...
	AnimationLevel const ubLevelID = (c->def.bLevel == 0 ? ANI_STRUCT_LEVEL : ANI_ONROOF_LEVEL);

	ANITILE_PARAMS AniParams;
	memset(&AniParams, 0, sizeof(AniParams));
	AniParams.sGridNo        = c->def.sGridNo;
	AniParams.ubLevelID      = ubLevelID;
	AniParams.sDelay         = 150;
	AniParams.sStartFrame    = 0;
	AniParams.uiFlags        = ANITILE_PAUSED | ANITILE_OPTIMIZEFORSLOWMOVING | ANITILE_ANIMATE_Z | ANITILE_ERASEITEMFROMSAVEBUFFFER | uiDirectionUseFlag;
	AniParams.sX             = CenterX(c->def.sGridNo);
	AniParams.sY             = CenterY(c->def.sGridNo);
	AniParams.sZ             = c->def.sHeightAdjustment;
	AniParams.v.user.uiData3 = c->def.bDirection;

	if (!gGameSettings.fOptions[TOPTION_BLOOD_N_GORE])
	{
		AniParams.zCachedFile = zNoBloodCorpseFilenames[c->def.ubType];
	}
	else
	{
		AniParams.zCachedFile = zCorpseFilenames[c->def.ubType];
	}

	ANITILE* const ani = CreateAnimationTile(&AniParams);
	c->pAniTile = ani;

  LEVELNODE*       const n    = ani->pLevelNode;
  const LEVELNODE* const land = gpWorldLevelData[c->def.sGridNo].pLandHead;

	// Set flag and index values
	n->uiFlags             |= LEVELNODE_ROTTINGCORPSE;
	n->ubShadeLevel         = land->ubShadeLevel;
	n->ubSumLights          = land->ubSumLights;
	n->ubMaxLights          = land->ubMaxLights;
	n->ubNaturalShadeLevel  = land->ubNaturalShadeLevel;

	// Get palette and create palettes and do substitutions
	CreateCorpsePalette(c);

	c->fActivated = TRUE;
	ani->v.user.uiData = CORPSE2ID(c);

	SetRenderFlags(RENDER_FLAG_FULL);

	if (c->def.usFlags & ROTTING_CORPSE_VEHICLE)
	{
		ani->uiFlags |= ANITILE_FORWARD | ANITILE_LOOPING;
		ani->uiFlags &= ~ANITILE_PAUSED;
	}

	InvalidateWorldRedundency();

	// OK, loop through gridnos for corpse and remove blood.....

	// Get root filename... this removes path and extension
	// Used to find struct data for this corpse...
	char zFilename[150];
	GetRootName(zFilename, lengthof(zFilename), AniParams.zCachedFile);

	// Add structure data.....
	CheckForAndAddTileCacheStructInfo(n, c->def.sGridNo, ani->sCachedTileID, GetCorpseStructIndex(pCorpseDef, TRUE));

	const STRUCTURE_FILE_REF* const pStructureFileRef = GetCachedTileStructureRefFromFilename(zFilename);
	if (pStructureFileRef != NULL)
	{
		const UINT16                  usStructIndex   = GetCorpseStructIndex(pCorpseDef, TRUE);
		const DB_STRUCTURE_REF* const pDBStructureRef = &pStructureFileRef->pDBStructureRef[usStructIndex];
		for (UINT8 ubLoop = 0; ubLoop < pDBStructureRef->pDBStructure->ubNumberOfTiles; ++ubLoop)
		{
			DB_STRUCTURE_TILE* const* const ppTile      = pDBStructureRef->ppTile;
			const INT16                     sTileGridNo = pCorpseDef->sGridNo + ppTile[ubLoop]->sPosRelToBase;
			RemoveBlood(sTileGridNo, pCorpseDef->bLevel);
		}
	}

	return c;
}
catch (...) { return 0; }


static void FreeCorpsePalettes(ROTTING_CORPSE* pCorpse)
{
	INT32 cnt;

	for ( cnt = 0; cnt < NUM_CORPSE_SHADES; cnt++ )
	{
		if ( pCorpse->pShades[ cnt ] != NULL )
		{
			MemFree( pCorpse->pShades[ cnt ] );
			pCorpse->pShades[ cnt ] = NULL;
		}
	}
}


static void RemoveCorpse(ROTTING_CORPSE* c);


void RemoveCorpses( )
{
	FOR_ALL_ROTTING_CORPSES(c) RemoveCorpse(c);
	giNumRottingCorpse = 0;
}


static void RemoveCorpse(ROTTING_CORPSE* const c)
{
	Assert(gRottingCorpse <= c && c < endof(gRottingCorpse));
	Assert(c->fActivated);

	c->fActivated = FALSE;
	DeleteAniTile(c->pAniTile);
	FreeCorpsePalettes(c);
}


static void CreateCorpsePalette(ROTTING_CORPSE* const c)
{
	char const* const substitution =
		c->def.ubType == ROTTING_STAGE2                  ? ""               :
		c->def.usFlags & ROTTING_CORPSE_USE_CAMO_PALETTE ? "ANIMS/camo.COL" :
		GetBodyTypePaletteSubstitution(0, c->def.ubBodyType);

	const SGPPaletteEntry* pal;
	SGPPaletteEntry        tmp_pal[256];
	if (!substitution)
	{
		// Use palette from HVOBJECT, then use substitution for pants, etc
		memcpy(tmp_pal, gpTileCache[c->pAniTile->sCachedTileID].pImagery->vo->Palette(), sizeof(tmp_pal));
		SetPaletteReplacement(tmp_pal, c->def.HeadPal);
		SetPaletteReplacement(tmp_pal, c->def.VestPal);
		SetPaletteReplacement(tmp_pal, c->def.PantsPal);
		SetPaletteReplacement(tmp_pal, c->def.SkinPal);
		pal = tmp_pal;
	}
	else if (substitution[0] != '\0' && CreateSGPPaletteFromCOLFile(tmp_pal, substitution))
	{
		pal = tmp_pal;
	}
	else
	{
		// Use palette from hvobject
		pal = gpTileCache[c->pAniTile->sCachedTileID].pImagery->vo->Palette();
	}

	CreateBiasedShadedPalettes(c->pShades, pal);
}


BOOLEAN TurnSoldierIntoCorpse(SOLDIERTYPE& s)
{
	ROTTING_CORPSE_DEFINITION		Corpse;
	UINT8												ubType;
	UINT16											usItemFlags = 0; //WORLD_ITEM_DONTRENDER;
  UINT8                       ubNumGoo;
  INT16                       sNewGridNo;
  OBJECTTYPE                  ItemObject;


	if (s.sGridNo == NOWHERE)
	{
		return( FALSE );
	}

  // ATE: Change to fix crash when item in hand
  if (gpItemPointer && gpItemPointerSoldier == &s)
  {
    CancelItemPointer( );
  }

	// Setup some values!
	memset( &Corpse, 0, sizeof( Corpse ) );
	Corpse.ubBodyType = s.ubBodyType;
	Corpse.sGridNo    = s.sGridNo;
	Corpse.bLevel     = s.bLevel;
	Corpse.ubProfile  = s.ubProfile;

	if ( Corpse.bLevel > 0 )
	{
		Corpse.sHeightAdjustment = s.sHeightAdjustment - WALL_HEIGHT;
	}

	SET_PALETTEREP_ID(Corpse.HeadPal,  s.HeadPal);
	SET_PALETTEREP_ID(Corpse.VestPal,  s.VestPal);
	SET_PALETTEREP_ID(Corpse.SkinPal,  s.SkinPal);
	SET_PALETTEREP_ID(Corpse.PantsPal, s.PantsPal);

	if (s.bCamo != 0)
	{
		Corpse.usFlags |= ROTTING_CORPSE_USE_CAMO_PALETTE;
	}

	// Determine corpse type!
	ubType = gubAnimSurfaceCorpseID[s.ubBodyType][s.usAnimState];

	Corpse.bDirection	= s.bDirection;

	// If we are a vehicle.... only use 1 direction....
	if (s.uiStatusFlags & SOLDIER_VEHICLE)
	{
		Corpse.usFlags |= ROTTING_CORPSE_VEHICLE;

    if (s.ubBodyType != ICECREAMTRUCK && s.ubBodyType != HUMVEE)
    {
      Corpse.bDirection = 7;
    }
    else
    {
      Corpse.bDirection = gb2DirectionsFrom8[ Corpse.bDirection ];
    }
	}

  if ( ubType == QUEEN_MONSTER_DEAD || ubType == BURNT_DEAD || ubType == EXPLODE_DEAD )
  {
    Corpse.bDirection = 7;
  }


	// ATE: If bDirection, get opposite
//	if ( ubType == SMERC_FALLF || ubType == MMERC_FALLF || ubType == FMERC_FALLF )
	//{
	//	Corpse.bDirection = OppositeDirection(Corpse.bDirection);
//	}

	// Set time of death
	Corpse.uiTimeOfDeath = GetWorldTotalMin( );

	// If corpse is not valid. make items visible
	if (ubType == NO_CORPSE && s.bTeam != gbPlayerNum)
	{
		usItemFlags &= (~WORLD_ITEM_DONTRENDER );
	}


	// ATE: If the queen is killed, she should
	// make items visible because it ruins end sequence....
	Visibility const bVisible =
		s.bTeam == gbPlayerNum || s.ubProfile == QUEEN ?
			VISIBLE : INVISIBLE;

  // Not for a robot...
  if (AM_A_ROBOT(&s))
  {

  }
  else if ( ubType == QUEEN_MONSTER_DEAD )
  {
    gTacticalStatus.fLockItemLocators = FALSE;

    ubNumGoo = 6 - ( gGameOptions.ubDifficultyLevel - DIF_LEVEL_EASY );

    sNewGridNo = s.sGridNo + WORLD_COLS * 2;

    for (INT32 cnt = 0; cnt < ubNumGoo; ++cnt)
    {
			CreateItem( JAR_QUEEN_CREATURE_BLOOD, 100, &ItemObject );

		  AddItemToPool(sNewGridNo, &ItemObject, bVisible, s.bLevel, usItemFlags, -1);
    }
  }
  else
  {
	  // OK, Place what objects this guy was carrying on the ground!
	  FOR_ALL_SOLDIER_INV_SLOTS(pObj, s)
	  {
		  if ( pObj->usItem != NOTHING )
		  {
			  // Check if it's supposed to be dropped
			  if (!(pObj->fFlags & OBJECT_UNDROPPABLE) || s.bTeam == gbPlayerNum)
			  {
				  // and make sure that it really is a droppable item type
				  if ( !(Item[ pObj->usItem ].fFlags & ITEM_DEFAULT_UNDROPPABLE) )
				  {
					  ReduceAmmoDroppedByNonPlayerSoldiers(s, *pObj);
					  AddItemToPool(s.sGridNo, pObj, bVisible, s.bLevel, usItemFlags, -1);
				  }
			  }
		  }
    }

    DropKeysInKeyRing(s, s.sGridNo, s.bLevel, bVisible, false, 0, false);
	}

	// Make team look for items
	AllSoldiersLookforItems();

	// If not a player, you can completely remove soldiertype
	// otherwise, just remove their graphic
	if (s.bTeam != gbPlayerNum)
	{
		// Remove merc!
		// ATE: Remove merc slot first - will disappear if no corpse data found!
		TacticalRemoveSoldier(s);
	}
	else
	{
		RemoveSoldierFromGridNo(s);
	}

	if (ubType == NO_CORPSE)
	{
		return( TRUE );
	}

	// Set type
	Corpse.ubType	= ubType;
	ROTTING_CORPSE* const added_corpse = AddRottingCorpse(&Corpse);

	// If this is our guy......make visible...
	//if (s.bTeam == gbPlayerNum)
	{
		added_corpse->def.bVisible = 1;
	}

	return( TRUE );
}


INT16 FindNearestRottingCorpse( SOLDIERTYPE *pSoldier )
{
	INT32 uiLowestRange = 999999;
	INT16 sLowestGridNo = NOWHERE;

	// OK, loop through our current listing of bodies
	CFOR_ALL_ROTTING_CORPSES(c)
	{
		// Check rotting state
		if (c->def.ubType == ROTTING_STAGE2)
		{
			const INT32 uiRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, c->def.sGridNo);
			if (uiRange < uiLowestRange)
			{
				sLowestGridNo = c->def.sGridNo;
				uiLowestRange = uiRange;
			}
		}
	}

	return( sLowestGridNo );
}


static void AddCrowToCorpse(ROTTING_CORPSE* pCorpse)
{
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	INT8										bBodyType = CROW;

	// No crows inside :(
	if (GetRoom(pCorpse->def.sGridNo) != NO_ROOM) return;

	// Put him flying over corpse pisition
	memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
	MercCreateStruct.ubProfile		= NO_PROFILE;
	MercCreateStruct.sSectorX			= gWorldSectorX;
	MercCreateStruct.sSectorY			= gWorldSectorY;
	MercCreateStruct.bSectorZ			= gbWorldSectorZ;
	MercCreateStruct.bBodyType		= bBodyType;
	MercCreateStruct.bDirection					= SOUTH;
	MercCreateStruct.bTeam							= CIV_TEAM;
	MercCreateStruct.sInsertionGridNo		= pCorpse->def.sGridNo;
	RandomizeNewSoldierStats( &MercCreateStruct );

	SOLDIERTYPE* const pSoldier = TacticalCreateSoldier(MercCreateStruct);
	if (pSoldier != NULL)
	{
		const INT16 sGridNo = FindRandomGridNoFromSweetSpot(pSoldier, pCorpse->def.sGridNo, 2);
		if ( sGridNo != NOWHERE )
		{
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			pSoldier->usStrategicInsertionData = sGridNo;

			pSoldier->sInsertionGridNo		= sGridNo;
			pSoldier->sDesiredHeight			= 0;

			AddSoldierToSector(pSoldier);

		  // Change to fly animation
		  //sGridNo = FindRandomGridNoFromSweetSpot(pSoldier, pCorpse->def.sGridNo, 5);
		  //pSoldier->usUIMovementMode = CROW_FLY;
		  //EVENT_GetNewSoldierPath( pSoldier, sGridNo, pSoldier->usUIMovementMode );

		  // Setup action data to point back to corpse....
		  pSoldier->uiPendingActionData1 = CORPSE2ID(pCorpse);
		  pSoldier->sPendingActionData2		=	 pCorpse->def.sGridNo;

		  pCorpse->def.bNumServicingCrows++;
    }

	}

}

void HandleCrowLeave( SOLDIERTYPE *pSoldier )
{
	// Check if this crow is still referencing the same corpse...
	ROTTING_CORPSE* const pCorpse = ID2CORPSE(pSoldier->uiPendingActionData1);

	// Double check grindo...
	if ( pSoldier->sPendingActionData2 == pCorpse->def.sGridNo )
	{
		// We have a match
		// Adjust crow servicing count...
		pCorpse->def.bNumServicingCrows--;

		if ( pCorpse->def.bNumServicingCrows < 0 )
		{
			pCorpse->def.bNumServicingCrows = 0;
		}
	}
}


void HandleCrowFlyAway( SOLDIERTYPE *pSoldier )
{
	// Set desired height
	pSoldier->sDesiredHeight			= 100;

	// Change to fly animation
	const INT16 sGridNo = FindRandomGridNoFromSweetSpot(pSoldier, pSoldier->sGridNo, 5);
	pSoldier->usUIMovementMode = CROW_FLY;
	SendGetNewSoldierPathEvent(pSoldier, sGridNo);
}


void HandleRottingCorpses( )
{
	ROTTING_CORPSE		*pCorpse;
	INT8							bNumCrows = 0;
  UINT32            uiChosenCorpseID;

	// ATE: If it's too late, don't!
	if( NightTime() )
	{
		return;
	}

  if ( gbWorldSectorZ > 0 )
  {
    return;
  }

	// ATE: Check for multiple crows.....
	// Couint how many we have now...
	CFOR_ALL_IN_TEAM(s, CIV_TEAM)
	{
		if (s->bInSector &&
				s->bLife >= OKLIFE &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				s->ubBodyType == CROW)
		{
			bNumCrows++;
		}
	}

  // Once population gets down to 0, we can add more again....
  if ( bNumCrows == 0 )
  {
    gTacticalStatus.fDontAddNewCrows = FALSE;
  }

  if ( gTacticalStatus.fDontAddNewCrows )
  {
    return;
  }

	if ( bNumCrows >= gTacticalStatus.ubNumCrowsPossible )
	{
    gTacticalStatus.fDontAddNewCrows = TRUE;
		return;
	}

	if (IsTeamActive(CREATURE_TEAM))
	{
		// don't add any crows while there are predators around
		return;
	}

  // Pick one to attact a crow...
	{
    uiChosenCorpseID = Random( giNumRottingCorpse );

		pCorpse = &(gRottingCorpse[ uiChosenCorpseID ] );

    if ( pCorpse->fActivated )
    {
		  if ( !( pCorpse->def.usFlags & ROTTING_CORPSE_VEHICLE ) )
		  {
			  if ( pCorpse->def.ubType == ROTTING_STAGE2 )
			  {
	        if ( GridNoOnScreen( pCorpse->def.sGridNo ) )
	        {
		        return;
	        }

					AddCrowToCorpse( pCorpse );
					AddCrowToCorpse( pCorpse );
        }
      }
    }
  }
}


static ROTTING_CORPSE* FindCorpseBasedOnStructure(GridNo const grid_no, STRUCTURE* const structure)
{
	for (LEVELNODE const* i = gpWorldLevelData[grid_no].pStructHead; i; i = i->pNext)
	{
		if (i->pStructureData != structure) continue;
		return ID2CORPSE(i->pAniTile->v.user.uiData);
	}
	return 0;
}


void CorpseHit( INT16 sGridNo, UINT16 usStructureID )
{
#if 0
	STRUCTURE				*pStructure, *pBaseStructure;
	ROTTING_CORPSE	*pCorpse = NULL;
	INT16						sBaseGridNo;

	pStructure = FindStructureByID( sGridNo, usStructureID );

	// Get base....
	pBaseStructure = FindBaseStructure( pStructure );

	// Find base gridno...
	sBaseGridNo = pBaseStructure->sGridNo;

	// Get corpse ID.....
	pCorpse = FindCorpseBasedOnStructure( sBaseGridNo, pBaseStructure );

	if ( pCorpse == NULL )
	{
	#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Bullet hit corpse but corpse cannot be found at: %d", sBaseGridNo );
	#endif
		return;
	}

	// Twitch the bugger...
	#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Corpse hit" );
	#endif

	if ( GridNoOnScreen( sBaseGridNo ) )
	{
		// Twitch....
		// Set frame...
		SetAniTileFrame( 	pCorpse->pAniTile, 1 );

		// Go reverse...
		pCorpse->pAniTile->uiFlags |= ( ANITILE_BACKWARD | ANITILE_PAUSE_AFTER_LOOP );

		// Turn off pause...
		pCorpse->pAniTile->uiFlags &= (~ANITILE_PAUSED);
	}

	// PLay a sound....
	PlayLocationJA2Sample(sGridNo, BULLET_IMPACT_2, MIDVOLUME, 1);
#endif
}


void VaporizeCorpse( INT16 sGridNo, UINT16 usStructureID )
{
	STRUCTURE				*pStructure, *pBaseStructure;
	ROTTING_CORPSE	*pCorpse = NULL;
	INT16						sBaseGridNo;
	ANITILE_PARAMS		AniParams;

	pStructure = FindStructureByID( sGridNo, usStructureID );

	// Get base....
	pBaseStructure = FindBaseStructure( pStructure );

	// Find base gridno...
	sBaseGridNo = pBaseStructure->sGridNo;

	// Get corpse ID.....
	pCorpse = FindCorpseBasedOnStructure( sBaseGridNo, pBaseStructure );

	if ( pCorpse == NULL )
	{
	#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Vaporize corpse but corpse cannot be found at: %d", sBaseGridNo );
	#endif
		return;
	}

	if ( pCorpse->def.usFlags & ROTTING_CORPSE_VEHICLE )
	{
		return;
	}

	if ( GridNoOnScreen( sBaseGridNo ) )
	{
		// Add explosion
		memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
		AniParams.sGridNo							= sBaseGridNo;
		AniParams.ubLevelID						= ANI_STRUCT_LEVEL;
		AniParams.sDelay							= (INT16)( 80 );
		AniParams.sStartFrame					= 0;
		AniParams.uiFlags             = ANITILE_FORWARD;
		AniParams.sX									= CenterX( sBaseGridNo );
		AniParams.sY									= CenterY( sBaseGridNo );
		AniParams.sZ									= (INT16)pCorpse->def.sHeightAdjustment;
		AniParams.zCachedFile = "TILECACHE/gen_blow.sti";
		CreateAnimationTile( &AniParams );

		RemoveCorpse(pCorpse);
		SetRenderFlags( RENDER_FLAG_FULL );

		if ( pCorpse->def.bLevel == 0 )
		{
			// Set some blood......
			SpreadEffect(sBaseGridNo, 2, 0, NULL, BLOOD_SPREAD_EFFECT, 0, NULL);
		}
	}

	// PLay a sound....
	PlayLocationJA2Sample(sGridNo, BODY_EXPLODE_1, HIGHVOLUME, 1);
}


INT16 FindNearestAvailableGridNoForCorpse( ROTTING_CORPSE_DEFINITION *pDef, INT8 ubRadius )
{
	INT16  sSweetGridNo;
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2, cnt3;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT16		sLowestGridNo=0;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;
	STRUCTURE_FILE_REF * pStructureFileRef = NULL;
	UINT8						ubBestDirection=0;
	BOOLEAN	fSetDirection   = FALSE;

	cnt3 = 0;

	// Get root filename... this removes path and extension
	// USed to find struct data fo rthis corpse...
	char zFilename[150];
	GetRootName(zFilename, lengthof(zFilename), zCorpseFilenames[pDef->ubType]);

	pStructureFileRef = GetCachedTileStructureRefFromFilename( zFilename );

	sSweetGridNo = pDef->sGridNo;


	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
			if ( sGridNo >=0 && sGridNo < WORLD_MAX )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, 0 );

	uiLowestRange = 999999;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if ( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) &&
				gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( &soldier, sGridNo, TRUE, soldier.bLevel ) )
				{
					BOOLEAN fDirectionFound = FALSE;
					BOOLEAN	fCanSetDirection   = FALSE;

					// Check each struct in each direction
					if ( pStructureFileRef == NULL )
					{
						fDirectionFound = TRUE;
					}
					else
					{
						for( cnt3 = 0; cnt3 < 8; cnt3++ )
						{
							if (OkayToAddStructureToWorld(sGridNo, pDef->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(cnt3)], INVALID_STRUCTURE_ID))
							{
								fDirectionFound = TRUE;
								fCanSetDirection = TRUE;
								break;
							}
						}
					}

					if ( fDirectionFound )
					{
						uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

						if ( uiRange < uiLowestRange )
						{
							if ( fCanSetDirection )
							{
								ubBestDirection = (UINT8)cnt3;
								fSetDirection   = TRUE;
							}
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
							fFound = TRUE;
						}
					}

				}
			}
		}
	}
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
	if ( fFound )
	{
		if ( fSetDirection )
		{
			pDef->bDirection = ubBestDirection;
		}

		return sLowestGridNo;
	}
	return NOWHERE;
}


BOOLEAN IsValidDecapitationCorpse(const ROTTING_CORPSE* const c)
{
	if (c->def.fHeadTaken) return FALSE;
	return gbCorpseValidForDecapitation[c->def.ubType];
}


ROTTING_CORPSE *GetCorpseAtGridNo( INT16 sGridNo, INT8 bLevel )
{
	STRUCTURE				*pStructure, *pBaseStructure;
	INT16						sBaseGridNo;

	pStructure = FindStructure( sGridNo, STRUCTURE_CORPSE );

	if ( pStructure != NULL )
	{
		// Get base....
		pBaseStructure = FindBaseStructure( pStructure );

		// Find base gridno...
		sBaseGridNo = pBaseStructure->sGridNo;

		if ( pBaseStructure != NULL )
		{
			return( FindCorpseBasedOnStructure( sBaseGridNo, pBaseStructure ) );
		}
	}

	return( NULL );
}


void DecapitateCorpse(const INT16 sGridNo, const INT8 bLevel)
{
	OBJECTTYPE		Object;
	ROTTING_CORPSE *pCorpse;
	ROTTING_CORPSE_DEFINITION CorpseDef;

	pCorpse = GetCorpseAtGridNo( sGridNo, bLevel );

	if ( pCorpse == NULL )
	{
		return;
	}

	if ( IsValidDecapitationCorpse( pCorpse ) )
	{
		// Decapitate.....
		// Copy corpse definition...
		CorpseDef = pCorpse->def;

		// Add new one...
		CorpseDef.ubType = gDecapitatedCorpse[ CorpseDef.ubType ];

    pCorpse->def.fHeadTaken = TRUE;

		if ( CorpseDef.ubType != 0 )
		{
			RemoveCorpse(pCorpse);
			AddRottingCorpse( &CorpseDef );
		}

		// Add head item.....

		// Pick the head based on profile type...
		UINT16 head_index;
		switch (pCorpse->def.ubProfile)
		{
			case CHRIS:    head_index = HEAD_2; break;
			case T_REX:    head_index = HEAD_3; break;
			case SLAY:     head_index = HEAD_4; break;
			case DRUGGIST: head_index = HEAD_5; break;
			case ANNIE:    head_index = HEAD_6; break;
			case TIFFANY:  head_index = HEAD_7; break;
			default:       head_index = HEAD_1; break;
		}

		CreateItem(head_index, 100, &Object);
		AddItemToPool( sGridNo, &Object, INVISIBLE, 0, 0, 0 );

		// All teams lok for this...
		NotifySoldiersToLookforItems( );
	}

}


void GetBloodFromCorpse( SOLDIERTYPE *pSoldier )
{
	const ROTTING_CORPSE* const pCorpse = ID2CORPSE(pSoldier->uiPendingActionData4);
	INT8						bObjSlot;
	OBJECTTYPE			Object;

	bObjSlot = FindObj( pSoldier, JAR );

	// What kind of corpse ami I?
	switch( pCorpse->def.ubType )
	{
		case ADULTMONSTER_DEAD:
		case INFANTMONSTER_DEAD:

			// Can get creature blood....
			CreateItem( JAR_CREATURE_BLOOD, 100, &Object );
			break;

    case QUEEN_MONSTER_DEAD:
			CreateItem( JAR_QUEEN_CREATURE_BLOOD, 100, &Object );
			break;

		default:

			CreateItem( JAR_HUMAN_BLOOD, 100, &Object );
			break;

	}

	if ( bObjSlot != NO_SLOT )
	{
		SwapObjs( &(pSoldier->inv[ bObjSlot ] ), &Object );
	}
}


void ReduceAmmoDroppedByNonPlayerSoldiers(SOLDIERTYPE const& s, OBJECTTYPE& o)
{
	if (s.bTeam == gbPlayerNum) return;
	if (Item[o.usItem].usItemClass != IC_AMMO) return;

	/* Don't drop all the clips, just a random # of them between 1 and how
	 * many there are */
	o.ubNumberOfObjects = 1 + Random(o.ubNumberOfObjects);
	o.ubWeight          = CalculateObjectWeight(&o);
}


void LookForAndMayCommentOnSeeingCorpse( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubLevel )
{
	ROTTING_CORPSE *pCorpse;
  INT8            bToleranceThreshold = 0;

  if ( QuoteExp_HeadShotOnly[ pSoldier->ubProfile ] == 1 )
  {
    return;
  }

  pCorpse = GetCorpseAtGridNo( sGridNo, ubLevel );

  if ( pCorpse == NULL )
  {
    return;
  }

	if ( pCorpse->def.ubType != ROTTING_STAGE2 )
  {
    return;
  }

  // If servicing qrows, tolerance is now 1
  if ( pCorpse->def.bNumServicingCrows > 0 )
  {
    bToleranceThreshold++;
  }

  // Check tolerance
  if ( pSoldier->bCorpseQuoteTolerance <= bToleranceThreshold )
  {
    // Say quote...
		TacticalCharacterDialogue( pSoldier, QUOTE_HEADSHOT );

    BeginMultiPurposeLocator(sGridNo, ubLevel);

    // Reset values....
    pSoldier->bCorpseQuoteTolerance = (INT8)( Random(3) + 1 );

    // 50% chance of adding 1 to other mercs....
    if ( Random( 2 ) == 1 )
    {
			FOR_ALL_IN_TEAM(s, gbPlayerNum)
	    {
			  // ATE: Ok, lets check for some basic things here!
			  if (s->bLife >= OKLIFE && s->sGridNo != NOWHERE && s->bInSector)
			  {
          ++s->bCorpseQuoteTolerance;
        }
	    }
    }
  }
}


INT16 GetGridNoOfCorpseGivenProfileID(const UINT8 ubProfileID)
{
	// Loop through all corpses....
	CFOR_ALL_ROTTING_CORPSES(c)
	{
		if (c->def.ubProfile == ubProfileID) return c->def.sGridNo;
  }
  return NOWHERE;
}


void DecayRottingCorpseAIWarnings(void)
{
	FOR_ALL_ROTTING_CORPSES(c)
	{
		if (c->def.ubAIWarningValue > 0) --c->def.ubAIWarningValue;
	}
}


UINT8 GetNearestRottingCorpseAIWarning(const INT16 sGridNo)
{
	UINT8 ubHighestWarning = 0;
	CFOR_ALL_ROTTING_CORPSES(c)
	{
		if (c->def.ubAIWarningValue > 0                                    &&
				PythSpacesAway(sGridNo, c->def.sGridNo) <= CORPSE_WARNING_DIST &&
				c->def.ubAIWarningValue > ubHighestWarning)
		{
			ubHighestWarning = c->def.ubAIWarningValue;
		}
	}
	return ubHighestWarning;
}
