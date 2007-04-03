#include <stdio.h>
#include <string.h>
#include "WCheck.h"
#include "stdlib.h"
#include "Debug.h"
#include "Soldier_Control.h"
#include "Weapons.h"
#include "Handle_Items.h"
#include "WorldDef.h"
#include "WorldMan.h"
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
#include "OppList.h"
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
#include "Stubs.h" // XXX


#define CORPSE_WARNING_MAX 5
#define CORPSE_WARNING_DIST 5

#define		CORPSE_INDEX_OFFSET		10000

//#define		DELAY_UNTIL_ROTTING		( 1 * NUM_SEC_IN_DAY )
#define		DELAY_UNTIL_ROTTING		    ( 1 * NUM_SEC_IN_DAY / 60 )
#define   DELAY_UNTIL_DONE_ROTTING  ( 3 * NUM_SEC_IN_DAY / 60 )

#define		MAX_NUM_CROWS					6

// From lighting
extern		SGPPaletteEntry	gpLightColors[3];
extern		UINT16 gusShadeLevels[16][3];


static void MakeCorpseVisible(const SOLDIERTYPE* pSoldier, ROTTING_CORPSE* pCorpse);


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


void ReduceAmmoDroppedByNonPlayerSoldiers( SOLDIERTYPE *pSoldier, INT32 iInvSlot );


static INT32 GetFreeRottingCorpse(void)
{
	INT32 iCount;

	for(iCount=0; iCount < giNumRottingCorpse; iCount++)
	{
		if(( gRottingCorpse[iCount].fActivated == FALSE ) )
			return((INT32)iCount);
	}

	if(giNumRottingCorpse < MAX_ROTTING_CORPSES )
		return((INT32)giNumRottingCorpse++);

	return(-1);
}


static void RecountRottingCorpses(void)
{
INT32 uiCount;

	if ( giNumRottingCorpse > 0 )
	{
		for(uiCount=giNumRottingCorpse-1; (uiCount >=0) ; uiCount--)
		{
			if( ( gRottingCorpse[uiCount].fActivated == FALSE ) )
			{
				giNumRottingCorpse=(UINT32)(uiCount+1);
				break;
			}
		}
	}
}


UINT16 GetCorpseStructIndex( ROTTING_CORPSE_DEFINITION *pCorpseDef, BOOLEAN fForImage )
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
      if ( fForImage )
      {
			  bDirection = gOneCDirection[ bDirection ];
      }
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
			  bDirection = gOneCDirection[ bDirection ];
      }
			break;

		default:

			// Uses 8
			bDirection =  pCorpseDef->bDirection;

      if ( fForImage )
      {
			  bDirection = gOneCDirection[ bDirection ];
      }
			break;
	}

	return( bDirection );
}


static BOOLEAN CreateCorpsePalette(ROTTING_CORPSE* pCorpse);


INT32	AddRottingCorpse( ROTTING_CORPSE_DEFINITION *pCorpseDef )
{
	INT32							iIndex;
	ROTTING_CORPSE		*pCorpse;
	ANITILE_PARAMS		AniParams;
	UINT8							ubLevelID;
	STRUCTURE_FILE_REF * pStructureFileRef = NULL;
	INT8						zFilename[150];
	DB_STRUCTURE_REF	 *pDBStructureRef;
	UINT8									ubLoop;
	INT16							sTileGridNo;
	DB_STRUCTURE_TILE	**	ppTile;
	UINT16						usStructIndex;
	UINT32						uiDirectionUseFlag;

  if ( pCorpseDef->sGridNo == NOWHERE )
  {
    return( -1 );
  }

  if ( pCorpseDef->ubType == NO_CORPSE )
  {
    return( -1 );
  }

	if( ( iIndex = GetFreeRottingCorpse() )==(-1) )
		return(-1);

	pCorpse = &gRottingCorpse[ iIndex ];

	// Copy elements in
	pCorpse->def = *pCorpseDef;

	uiDirectionUseFlag = ANITILE_USE_DIRECTION_FOR_START_FRAME;

	// If we are a soecial type...
	switch( pCorpseDef->ubType )
	{
		case SMERC_FALL:
		case SMERC_FALLF:
		case MMERC_FALL:
		case MMERC_FALLF:
		case FMERC_FALL:
		case FMERC_FALLF:

			uiDirectionUseFlag = ANITILE_USE_4DIRECTION_FOR_START_FRAME;
	}


	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// OK, AS WE ADD, CHECK FOR TOD AND DECAY APPROPRIATELY
		if ( ( ( GetWorldTotalMin( ) - pCorpse->def.uiTimeOfDeath ) > DELAY_UNTIL_ROTTING ) && ( pCorpse->def.ubType < ROTTING_STAGE2 ) )
		{
      if ( pCorpse->def.ubType <= FMERC_FALLF )
      {
			  // Rott!
			  pCorpse->def.ubType = ROTTING_STAGE2;
      }
		}

    // If time of death is a few days, now, don't add at all!
		if ( ( ( GetWorldTotalMin( ) - pCorpse->def.uiTimeOfDeath ) > DELAY_UNTIL_DONE_ROTTING ) )
    {

      return( -1 );
    }
	}

	// Check if on roof or not...
	if ( pCorpse->def.bLevel == 0 )
	{
		//ubLevelID = ANI_OBJECT_LEVEL;
		ubLevelID = ANI_STRUCT_LEVEL;
	}
	else
	{
		ubLevelID = ANI_ONROOF_LEVEL;
	}


	memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
	AniParams.sGridNo							= pCorpse->def.sGridNo;
	AniParams.ubLevelID						= ubLevelID;
	AniParams.sDelay							= (INT16)( 150 );
	AniParams.sStartFrame					= 0;
	AniParams.uiFlags							= ANITILE_CACHEDTILE | ANITILE_PAUSED | ANITILE_OPTIMIZEFORSLOWMOVING | ANITILE_ANIMATE_Z | ANITILE_ERASEITEMFROMSAVEBUFFFER | uiDirectionUseFlag;
	AniParams.sX									= CenterX( pCorpse->def.sGridNo );
	AniParams.sY									= CenterY( pCorpse->def.sGridNo );
	AniParams.sZ									= (INT16)pCorpse->def.sHeightAdjustment;
	AniParams.uiUserData3					= pCorpse->def.bDirection;


	if ( !gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ] )
	{
		strcpy( AniParams.zCachedFile, zNoBloodCorpseFilenames[ pCorpse->def.ubType ] );
	}
	else
	{
		strcpy( AniParams.zCachedFile, zCorpseFilenames[ pCorpse->def.ubType ] );
	}

	pCorpse->pAniTile = CreateAnimationTile( &AniParams );

  if ( pCorpse->pAniTile == NULL )
  {
  	pCorpse->fActivated = FALSE;
    return( -1 );
  }

	// Set flag and index values
	pCorpse->pAniTile->pLevelNode->uiFlags		 |= ( LEVELNODE_ROTTINGCORPSE );

	pCorpse->pAniTile->pLevelNode->ubShadeLevel					=	gpWorldLevelData[ pCorpse->def.sGridNo ].pLandHead->ubShadeLevel;
	pCorpse->pAniTile->pLevelNode->ubSumLights					=	gpWorldLevelData[ pCorpse->def.sGridNo ].pLandHead->ubSumLights;
	pCorpse->pAniTile->pLevelNode->ubMaxLights					=	gpWorldLevelData[ pCorpse->def.sGridNo ].pLandHead->ubMaxLights;
	pCorpse->pAniTile->pLevelNode->ubNaturalShadeLevel	=	gpWorldLevelData[ pCorpse->def.sGridNo ].pLandHead->ubNaturalShadeLevel;

	pCorpse->pAniTile->uiUserData								= iIndex;
	pCorpse->iID																= iIndex;

	pCorpse->fActivated = TRUE;

	if ( Random( 100 ) > 50 )
	{
		pCorpse->fAttractCrowsOnlyWhenOnScreen = TRUE;
	}
	else
	{
		pCorpse->fAttractCrowsOnlyWhenOnScreen = FALSE;
	}

	pCorpse->iCachedTileID = pCorpse->pAniTile->sCachedTileID;

	if ( pCorpse->iCachedTileID == -1 )
	{
		DeleteAniTile( pCorpse->pAniTile );
  	pCorpse->fActivated = FALSE;
		return( -1 );
	}

	// Get palette and create palettes and do substitutions
	if ( !CreateCorpsePalette( pCorpse ) )
	{
		DeleteAniTile( pCorpse->pAniTile );
  	pCorpse->fActivated = FALSE;
		return( -1 );
	}

	SetRenderFlags(RENDER_FLAG_FULL);

	if ( pCorpse->def.usFlags & ROTTING_CORPSE_VEHICLE )
	{
		pCorpse->pAniTile->uiFlags |= ( ANITILE_FORWARD | ANITILE_LOOPING );

		// Turn off pause...
		pCorpse->pAniTile->uiFlags &= (~ANITILE_PAUSED);
	}

	InvalidateWorldRedundency( );

	// OK, loop through gridnos for corpse and remove blood.....

	// Get root filename... this removes path and extension
	// USed to find struct data fo rthis corpse...
	GetRootName( zFilename, AniParams.zCachedFile );

	// Add structure data.....
	CheckForAndAddTileCacheStructInfo( pCorpse->pAniTile->pLevelNode, pCorpse->def.sGridNo, ( UINT16 ) ( pCorpse->iCachedTileID ), GetCorpseStructIndex( pCorpseDef, TRUE ) );

	pStructureFileRef = GetCachedTileStructureRefFromFilename( zFilename );

	if ( pStructureFileRef != NULL )
	{
		usStructIndex = GetCorpseStructIndex( pCorpseDef, TRUE );

		pDBStructureRef = &(pStructureFileRef->pDBStructureRef[ usStructIndex ] );

		for (ubLoop = 0; ubLoop < pDBStructureRef->pDBStructure->ubNumberOfTiles; ubLoop++)
		{
			ppTile = pDBStructureRef->ppTile;

			sTileGridNo = pCorpseDef->sGridNo + ppTile[ ubLoop ]->sPosRelToBase;

			//Remove blood
			RemoveBlood( sTileGridNo, pCorpseDef->bLevel );
		}
	}

	// OK, we're done!
	return( iIndex );
}


static void FreeCorpsePalettes(ROTTING_CORPSE* pCorpse)
{
	INT32 cnt;

	// Free palettes
	MemFree( pCorpse->p8BPPPalette );

	for ( cnt = 0; cnt < NUM_CORPSE_SHADES; cnt++ )
	{
		if ( pCorpse->pShades[ cnt ] != NULL )
		{
			MemFree( pCorpse->pShades[ cnt ] );
			pCorpse->pShades[ cnt ] = NULL;
		}
	}
}


static void RemoveCorpse(INT32 iCorpseID);


void RemoveCorpses( )
{
	INT32 iCount;

	for(iCount=0; iCount < giNumRottingCorpse; iCount++)
	{
		if( ( gRottingCorpse[iCount].fActivated ) )
		{
			RemoveCorpse( iCount );
		}
	}

	giNumRottingCorpse = 0;
}


static void RemoveCorpse(INT32 iCorpseID)
{
	// Remove!
	gRottingCorpse[ iCorpseID ].fActivated = FALSE;

	DeleteAniTile( gRottingCorpse[ iCorpseID ].pAniTile );

	FreeCorpsePalettes( &( gRottingCorpse[ iCorpseID ] ) );

}


static UINT16 CreateCorpsePaletteTables(ROTTING_CORPSE* pCorpse);


static BOOLEAN CreateCorpsePalette(ROTTING_CORPSE* pCorpse)
{
	CHAR8	zColFilename[ 100 ];
	INT8	bBodyTypePalette;
	SGPPaletteEntry							Temp8BPPPalette[ 256 ];

	pCorpse->p8BPPPalette = MemAlloc( sizeof( SGPPaletteEntry ) * 256 );

	CHECKF( pCorpse->p8BPPPalette != NULL );

	bBodyTypePalette = GetBodyTypePaletteSubstitutionCode( NULL, pCorpse->def.ubBodyType, zColFilename );

	// If this corpse has cammo,
  if ( pCorpse->def.ubType == ROTTING_STAGE2 )
  {
		bBodyTypePalette = 0;
  }
  else if ( pCorpse->def.usFlags & ROTTING_CORPSE_USE_CAMMO_PALETTE )
	{
		strcpy( zColFilename, "ANIMS/camo.COL" );
		bBodyTypePalette = 1;
	}

	if ( bBodyTypePalette == -1  )
	{
		// Use palette from HVOBJECT, then use substitution for pants, etc
		memcpy( pCorpse->p8BPPPalette, gpTileCache[ pCorpse->iCachedTileID ].pImagery->vo->pPaletteEntry, sizeof( pCorpse->p8BPPPalette ) * 256 );

		// Substitute based on head, etc
		SetPaletteReplacement( pCorpse->p8BPPPalette, pCorpse->def.HeadPal );
		SetPaletteReplacement( pCorpse->p8BPPPalette, pCorpse->def.VestPal );
		SetPaletteReplacement( pCorpse->p8BPPPalette, pCorpse->def.PantsPal );
		SetPaletteReplacement( pCorpse->p8BPPPalette, pCorpse->def.SkinPal );
	}
	else if ( bBodyTypePalette == 0 )
	{
		// Use palette from hvobject
		memcpy( pCorpse->p8BPPPalette, gpTileCache[ pCorpse->iCachedTileID ].pImagery->vo->pPaletteEntry, sizeof( pCorpse->p8BPPPalette ) * 256 );
	}
	else
	{
		// Use col file
		if ( CreateSGPPaletteFromCOLFile( Temp8BPPPalette, zColFilename ) )
		{
			// Copy into palette
			memcpy( pCorpse->p8BPPPalette,		Temp8BPPPalette, sizeof( pCorpse->p8BPPPalette ) * 256 );
		}
		else
		{
			// Use palette from hvobject
			memcpy( pCorpse->p8BPPPalette, gpTileCache[ pCorpse->iCachedTileID ].pImagery->vo->pPaletteEntry, sizeof( pCorpse->p8BPPPalette ) * 256 );
		}
	}

	CreateCorpsePaletteTables( pCorpse );

	return( TRUE );
}


BOOLEAN TurnSoldierIntoCorpse( SOLDIERTYPE *pSoldier, BOOLEAN fRemoveMerc, BOOLEAN fCheckForLOS )
{
	ROTTING_CORPSE_DEFINITION		Corpse;
	UINT8												ubType;
	INT32												cnt;
	UINT16											usItemFlags = 0; //WORLD_ITEM_DONTRENDER;
	INT32												iCorpseID;
	INT8												bVisible = -1;
	OBJECTTYPE									*pObj;
  UINT8                       ubNumGoo;
  INT16                       sNewGridNo;
  OBJECTTYPE                  ItemObject;


	if ( pSoldier->sGridNo == NOWHERE )
	{
		return( FALSE );
	}

  // ATE: Change to fix crash when item in hand
  if ( gpItemPointer != NULL && gpItemPointerSoldier == pSoldier )
  {
    CancelItemPointer( );
  }

	// Setup some values!
	memset( &Corpse, 0, sizeof( Corpse ) );
	Corpse.ubBodyType							= pSoldier->ubBodyType;
	Corpse.sGridNo								= pSoldier->sGridNo;
	Corpse.dXPos									= pSoldier->dXPos;
	Corpse.dYPos									= pSoldier->dYPos;
	Corpse.bLevel									= pSoldier->bLevel;
	Corpse.ubProfile							= pSoldier->ubProfile;

	if ( Corpse.bLevel > 0 )
	{
		Corpse.sHeightAdjustment			= (INT16)( pSoldier->sHeightAdjustment - WALL_HEIGHT );
	}

	SET_PALETTEREP_ID ( Corpse.HeadPal,		pSoldier->HeadPal );
	SET_PALETTEREP_ID ( Corpse.VestPal,		pSoldier->VestPal );
	SET_PALETTEREP_ID ( Corpse.SkinPal,		pSoldier->SkinPal );
	SET_PALETTEREP_ID ( Corpse.PantsPal,   pSoldier->PantsPal );

	if ( pSoldier->bCamo != 0 )
	{
		Corpse.usFlags |= ROTTING_CORPSE_USE_CAMMO_PALETTE;
	}

	// Determine corpse type!
	ubType = (UINT8)gubAnimSurfaceCorpseID[ pSoldier->ubBodyType][ pSoldier->usAnimState ];

	Corpse.bDirection	= pSoldier->bDirection;

	// If we are a vehicle.... only use 1 direction....
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		Corpse.usFlags |= ROTTING_CORPSE_VEHICLE;

    if ( pSoldier->ubBodyType != ICECREAMTRUCK && pSoldier->ubBodyType != HUMVEE )
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
	//	Corpse.bDirection = gOppositeDirection[ Corpse.bDirection ];
//	}

	// Set time of death
	Corpse.uiTimeOfDeath = GetWorldTotalMin( );

	// If corpse is not valid. make items visible
	if ( ubType == NO_CORPSE && pSoldier->bTeam != gbPlayerNum )
	{
		usItemFlags &= (~WORLD_ITEM_DONTRENDER );
	}


	// ATE: If the queen is killed, she should
	// make items visible because it ruins end sequence....
	if ( pSoldier->ubProfile == QUEEN || pSoldier->bTeam == gbPlayerNum )
	{
		bVisible = 1;
	}

  // Not for a robot...
  if ( AM_A_ROBOT( pSoldier ) )
  {

  }
  else if ( ubType == QUEEN_MONSTER_DEAD )
  {
    gTacticalStatus.fLockItemLocators = FALSE;

    ubNumGoo = 6 - ( gGameOptions.ubDifficultyLevel - DIF_LEVEL_EASY );

    sNewGridNo = pSoldier->sGridNo + ( WORLD_COLS * 2 );

    for ( cnt = 0; cnt < ubNumGoo; cnt++ )
    {
			CreateItem( JAR_QUEEN_CREATURE_BLOOD, 100, &ItemObject );

		  AddItemToPool( sNewGridNo, &ItemObject, bVisible , pSoldier->bLevel, usItemFlags, -1 );
    }
  }
  else
  {
	  // OK, Place what objects this guy was carrying on the ground!
	  for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
	  {
		  pObj = &( pSoldier->inv[ cnt ] );

		  if ( pObj->usItem != NOTHING )
		  {
			  // Check if it's supposed to be dropped
			  if ( !( pObj->fFlags & OBJECT_UNDROPPABLE ) || pSoldier->bTeam == gbPlayerNum )
			  {
				  // and make sure that it really is a droppable item type
				  if ( !(Item[ pObj->usItem ].fFlags & ITEM_DEFAULT_UNDROPPABLE) )
				  {
					  ReduceAmmoDroppedByNonPlayerSoldiers( pSoldier, cnt );
					  AddItemToPool( pSoldier->sGridNo, pObj, bVisible , pSoldier->bLevel, usItemFlags, -1 );
				  }
			  }
		  }
    }

    DropKeysInKeyRing( pSoldier, pSoldier->sGridNo, pSoldier->bLevel, bVisible, FALSE, 0, FALSE );
	}

	// Make team look for items
	AllSoldiersLookforItems( TRUE );

	//if we are to call TacticalRemoveSoldier after adding the corpse
	if( fRemoveMerc )
	{
		// If not a player, you can completely remove soldiertype
		// otherwise, just remove their graphic
		if ( pSoldier->bTeam != gbPlayerNum )
		{
			// Remove merc!
			// ATE: Remove merc slot first - will disappear if no corpse data found!
			TacticalRemoveSoldier( pSoldier->ubID );
		}
		else
		{
			RemoveSoldierFromGridNo( pSoldier );
		}

		if ( ubType == NO_CORPSE )
		{
			return( TRUE );
		}

		// Set type
		Corpse.ubType	= ubType;


		// Add corpse!
		iCorpseID = AddRottingCorpse( &Corpse );
	}
	else
	{
		if ( ubType == NO_CORPSE )
		{
			return( TRUE );
		}

		// Set type
		Corpse.ubType	= ubType;

		// Add corpse!
		iCorpseID = AddRottingCorpse( &Corpse );
	}

	// If this is our guy......make visible...
	//if ( pSoldier->bTeam == gbPlayerNum )
	{
		MakeCorpseVisible( pSoldier, &( gRottingCorpse[ iCorpseID ] ) );
	}

	return( TRUE );
}


INT16 FindNearestRottingCorpse( SOLDIERTYPE *pSoldier )
{
	INT32		uiRange, uiLowestRange = 999999;
	INT16		sLowestGridNo = NOWHERE;
	INT32		cnt;
	ROTTING_CORPSE		*pCorpse;

	// OK, loop through our current listing of bodies
	for ( cnt = 0; cnt < giNumRottingCorpse; cnt++ )
	{
		pCorpse = &(gRottingCorpse[ cnt ] );

		if ( pCorpse->fActivated )
		{
				// Check rotting state
				if ( pCorpse->def.ubType == ROTTING_STAGE2 )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pCorpse->def.sGridNo );

					if ( uiRange < uiLowestRange )
					{
						sLowestGridNo = pCorpse->def.sGridNo;
						uiLowestRange = uiRange;
					}
				}
		}

	}

	return( sLowestGridNo );
}


static void AddCrowToCorpse(ROTTING_CORPSE* pCorpse)
{
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	INT8										bBodyType = CROW;
	UINT8										iNewIndex;
	INT16										sGridNo;
	UINT8										ubDirection;
	SOLDIERTYPE							*pSoldier;
	UINT8										ubRoomNum;

	// No crows inside :(
	if ( InARoom( pCorpse->def.sGridNo, &ubRoomNum ) )
	{
		return;
	}

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

	if ( TacticalCreateSoldier( &MercCreateStruct, &iNewIndex ) != NULL )
	{
		pSoldier = MercPtrs[ iNewIndex ];

		sGridNo =  FindRandomGridNoFromSweetSpot( pSoldier, pCorpse->def.sGridNo, 2, &ubDirection );

		if ( sGridNo != NOWHERE )
		{
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			pSoldier->usStrategicInsertionData = sGridNo;

			pSoldier->sInsertionGridNo		= sGridNo;
			pSoldier->sDesiredHeight			= 0;

		  // Add to sector
		  AddSoldierToSector( iNewIndex );

		  // Change to fly animation
		  //sGridNo =  FindRandomGridNoFromSweetSpot( pSoldier, pCorpse->def.sGridNo, 5, &ubDirection );
		  //pSoldier->usUIMovementMode = CROW_FLY;
		  //EVENT_GetNewSoldierPath( pSoldier, sGridNo, pSoldier->usUIMovementMode );

		  // Setup action data to point back to corpse....
		  pSoldier->uiPendingActionData1	=  pCorpse->iID;
		  pSoldier->sPendingActionData2		=	 pCorpse->def.sGridNo;

		  pCorpse->def.bNumServicingCrows++;
    }

	}

}

void HandleCrowLeave( SOLDIERTYPE *pSoldier )
{
	ROTTING_CORPSE		*pCorpse;

	// Check if this crow is still referencing the same corpse...
	pCorpse = &(gRottingCorpse[ pSoldier->uiPendingActionData1 ] );

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
	UINT8 ubDirection;
	INT16	sGridNo;

	// Set desired height
	pSoldier->sDesiredHeight			= 100;

	// Change to fly animation
	sGridNo =  FindRandomGridNoFromSweetSpot( pSoldier, pSoldier->sGridNo, 5, &ubDirection );
	pSoldier->usUIMovementMode = CROW_FLY;
	SendGetNewSoldierPathEvent( pSoldier, sGridNo, pSoldier->usUIMovementMode );
}


void HandleRottingCorpses( )
{
	ROTTING_CORPSE		*pCorpse;
	INT8							bNumCrows = 0;
  UINT32            uiChosenCorpseID;


	// Don't allow crows here if flags not set
	if ( !gTacticalStatus.fGoodToAllowCrows )
	{
		return;
	}

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
	{
		UINT8 bLoop;
		SOLDIERTYPE * pSoldier;

		for (bLoop=gTacticalStatus.Team[ CIV_TEAM ].bFirstID, pSoldier=MercPtrs[bLoop]; bLoop <= gTacticalStatus.Team[ CIV_TEAM ].bLastID; bLoop++, pSoldier++)
		{
			if (pSoldier->bActive && pSoldier->bInSector && (pSoldier->bLife >= OKLIFE) && !( pSoldier->uiStatusFlags & SOLDIER_GASSED ) )
			{
				if ( pSoldier->ubBodyType == CROW )
				{
					bNumCrows++;
				}
			}
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

	if ( gTacticalStatus.Team[ CREATURE_TEAM ].bTeamActive )
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


static void MakeCorpseVisible(const SOLDIERTYPE* pSoldier, ROTTING_CORPSE* pCorpse)
{
	pCorpse->def.bVisible = 1;
	SetRenderFlags( RENDER_FLAG_FULL );
}


static void AllMercsOnTeamLookForCorpse(ROTTING_CORPSE* pCorpse, INT8 bTeam)
{
	INT32                    cnt;
	SOLDIERTYPE							 *pSoldier;
	INT16										 sDistVisible;
	INT16										 sGridNo;

	// If this cump is already visible, return
	if ( pCorpse->def.bVisible == 1 )
	{
		return;
	}

	if ( !pCorpse->fActivated )
	{
		return;
	}

	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	cnt = gTacticalStatus.Team[ bTeam ].bFirstID;

	sGridNo = pCorpse->def.sGridNo;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ bTeam ].bLastID; cnt++,pSoldier++ )
	{
			// ATE: Ok, lets check for some basic things here!
			if ( pSoldier->bLife >= OKLIFE && pSoldier->sGridNo != NOWHERE && pSoldier->bActive && pSoldier->bInSector )
			{
				// is he close enough to see that gridno if he turns his head?
				sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, pCorpse->def.bLevel );

				if (PythSpacesAway( pSoldier->sGridNo, sGridNo ) <= sDistVisible )
				{
					// and we can trace a line of sight to his x,y coordinates?
					// (taking into account we are definitely aware of this guy now)
					if ( SoldierTo3DLocationLineOfSightTest( pSoldier, sGridNo, pCorpse->def.bLevel, 3, (UINT8) sDistVisible, TRUE ) )
					{
						 MakeCorpseVisible( pSoldier, pCorpse );
						 return;
					}
				}
			}
	}
}


static void MercLooksForCorpses(SOLDIERTYPE* pSoldier)
{
	INT32                    cnt;
	INT16										 sDistVisible;
	INT16										 sGridNo;
	ROTTING_CORPSE					 *pCorpse;

	// Should we say disgust quote?
	if ( ( pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_ROTTINGCORPSE ) )
	{
     return;
  }

  if ( pSoldier->ubProfile == NO_PROFILE )
  {
    return;
  }

  if ( AM_AN_EPC( pSoldier ) )
  {
    return;
  }

  if ( QuoteExp_HeadShotOnly[ pSoldier->ubProfile ] == 1 )
  {
    return;
  }

  // Every so often... do a corpse quote...
  if ( Random( 400 ) <= 2 )
  {
	  // Loop through all corpses....
	  for ( cnt = 0; cnt < giNumRottingCorpse; cnt++ )
	  {
		  pCorpse = &(gRottingCorpse[ cnt ] );

		  if ( !pCorpse->fActivated )
		  {
			  continue;
		  }

      // Has this corpse rotted enough?
	    if ( pCorpse->def.ubType == ROTTING_STAGE2 )
	    {
		    sGridNo = pCorpse->def.sGridNo;

		    // is he close enough to see that gridno if he turns his head?
		    sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, pCorpse->def.bLevel );

		    if (PythSpacesAway( pSoldier->sGridNo, sGridNo ) <= sDistVisible )
		    {
			    // and we can trace a line of sight to his x,y coordinates?
			    // (taking into account we are definitely aware of this guy now)
			    if ( SoldierTo3DLocationLineOfSightTest( pSoldier, sGridNo, pCorpse->def.bLevel, 3, (UINT8) sDistVisible, TRUE ) )
			    {
				    TacticalCharacterDialogue( pSoldier, QUOTE_HEADSHOT );

				    pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_ROTTINGCORPSE;

            BeginMultiPurposeLocator( sGridNo, pCorpse->def.bLevel, FALSE );

            // Slide to...
            SlideToLocation( 0, sGridNo );

            return;
			    }
        }
		  }
    }
	}
}


void RebuildAllCorpseShadeTables( )
{
	INT32                    cnt;
	ROTTING_CORPSE					 *pCorpse;

	// Loop through all corpses....
	for ( cnt = 0; cnt < giNumRottingCorpse; cnt++ )
	{
		pCorpse = &(gRottingCorpse[ cnt ] );

		// If this cump is already visible, continue
		if ( pCorpse->def.bVisible == 1 )
		{
			continue;
		}

		if ( !pCorpse->fActivated )
		{
			continue;
		}

		// Rebuild shades....

	}
}


static BOOLEAN CreateCorpseShadedPalette(ROTTING_CORPSE* pCorpse, UINT32 uiBase, SGPPaletteEntry* pShadePal);


static UINT16 CreateCorpsePaletteTables(ROTTING_CORPSE* pCorpse)
{
	SGPPaletteEntry LightPal[256];
	UINT32 uiCount;

	// create the basic shade table
	for(uiCount=0; uiCount < 256; uiCount++)
	{
		// combine the rgb of the light color with the object's palette
		LightPal[uiCount].peRed=(UINT8)(__min((UINT16)pCorpse->p8BPPPalette[uiCount].peRed+(UINT16)gpLightColors[0].peRed, 255));
		LightPal[uiCount].peGreen=(UINT8)(__min((UINT16)pCorpse->p8BPPPalette[uiCount].peGreen+(UINT16)gpLightColors[0].peGreen, 255));
		LightPal[uiCount].peBlue=(UINT8)(__min((UINT16)pCorpse->p8BPPPalette[uiCount].peBlue+(UINT16)gpLightColors[0].peBlue, 255));
	}
	// build the shade tables
	CreateCorpseShadedPalette( pCorpse, 0, LightPal );

	// build neutral palette as well!
	// Set current shade table to neutral color

	return(TRUE);

}


static BOOLEAN CreateCorpseShadedPalette(ROTTING_CORPSE* pCorpse, UINT32 uiBase, SGPPaletteEntry* pShadePal)
{
	UINT32 uiCount;

	pCorpse->pShades[uiBase] = Create16BPPPaletteShaded( pShadePal, gusShadeLevels[0][0],
																															gusShadeLevels[0][1],
																															gusShadeLevels[0][2], TRUE);

	for(uiCount=1; uiCount < 16; uiCount++)
	{
		pCorpse->pShades[uiBase+uiCount]=Create16BPPPaletteShaded( pShadePal, gusShadeLevels[uiCount][0],
																																				gusShadeLevels[uiCount][1],
																																				gusShadeLevels[uiCount][2], FALSE);
	}

	return(TRUE);
}


static ROTTING_CORPSE* FindCorpseBasedOnStructure(INT16 sGridNo, STRUCTURE* pStructure)
{
	LEVELNODE					*pLevelNode;
	ROTTING_CORPSE		*pCorpse = NULL;

	pLevelNode = gpWorldLevelData[ sGridNo ].pStructHead;
	while( pLevelNode != NULL )
	{
		if (pLevelNode->pStructureData == pStructure )
		{
			break;
		}
		pLevelNode = pLevelNode->pNext;
	}

	if ( pLevelNode != NULL )
	{
		// Get our corpse....
		pCorpse = &( gRottingCorpse[ pLevelNode->pAniTile->uiUserData ] );
	}

	return( pCorpse );
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
	PlayJA2Sample(BULLET_IMPACT_2, SoundVolume(MIDVOLUME, sGridNo), 1, SoundDir(sGridNo));

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
		AniParams.uiFlags							= ANITILE_CACHEDTILE | ANITILE_FORWARD;
		AniParams.sX									= CenterX( sBaseGridNo );
		AniParams.sY									= CenterY( sBaseGridNo );
		AniParams.sZ									= (INT16)pCorpse->def.sHeightAdjustment;

		strcpy( AniParams.zCachedFile, "TILECACHE/gen_blow.sti" );
		CreateAnimationTile( &AniParams );

		// Remove....
		RemoveCorpse( pCorpse->iID );
		SetRenderFlags( RENDER_FLAG_FULL );

		if ( pCorpse->def.bLevel == 0 )
		{
			// Set some blood......
			SpreadEffect( sBaseGridNo, (UINT8)( ( 2 ) ), 0, NOBODY, BLOOD_SPREAD_EFFECT, 0, -1 );
		}
	}

	// PLay a sound....
	PlayJA2Sample(BODY_EXPLODE_1, SoundVolume(HIGHVOLUME, sGridNo), 1, SoundDir(sGridNo));

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
	INT8						zFilename[150];
	UINT8						ubBestDirection=0;
	BOOLEAN	fSetDirection   = FALSE;

	cnt3 = 0;

	// Get root filename... this removes path and extension
	// USed to find struct data fo rthis corpse...
	GetRootName( zFilename, zCorpseFilenames[pDef->ubType ] );

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
							if ( OkayToAddStructureToWorld( (INT16)sGridNo, pDef->bLevel, &(pStructureFileRef->pDBStructureRef[gOneCDirection[ cnt3 ]]), INVALID_STRUCTURE_ID ) )
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


BOOLEAN IsValidDecapitationCorpse( ROTTING_CORPSE *pCorpse )
{
  if ( pCorpse->def.fHeadTaken )
  {
    return( FALSE );
  }

	return( gbCorpseValidForDecapitation[ pCorpse->def.ubType ] );
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


void DecapitateCorpse( SOLDIERTYPE *pSoldier, INT16 sGridNo,  INT8 bLevel )
{
	OBJECTTYPE		Object;
	ROTTING_CORPSE *pCorpse;
	ROTTING_CORPSE_DEFINITION CorpseDef;
	UINT16 usHeadIndex = HEAD_1;


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
			// Remove old one...
			RemoveCorpse( pCorpse->iID );

			AddRottingCorpse( &CorpseDef );
		}

		// Add head item.....

		// Pick the head based on profile type...
		switch( pCorpse->def.ubProfile )
		{
			case 83:
				usHeadIndex = HEAD_2;
				break;

			case 111:
				usHeadIndex = HEAD_3;
				break;

			case 64:
				usHeadIndex = HEAD_4;
				break;

			case 112:
				usHeadIndex = HEAD_5;
				break;

			case 82:
				usHeadIndex = HEAD_6;
				break;

			case 110:
				usHeadIndex = HEAD_7;
				break;

		}

		CreateItem( usHeadIndex, 100, &Object );
		AddItemToPool( sGridNo, &Object, INVISIBLE, 0, 0, 0 );

		// All teams lok for this...
		NotifySoldiersToLookforItems( );
	}

}


void GetBloodFromCorpse( SOLDIERTYPE *pSoldier )
{
	ROTTING_CORPSE *pCorpse;
	INT8						bObjSlot;
	OBJECTTYPE			Object;

	// OK, get corpse
	pCorpse = &( gRottingCorpse[ pSoldier->uiPendingActionData4 ] );

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



void ReduceAmmoDroppedByNonPlayerSoldiers( SOLDIERTYPE *pSoldier, INT32 iInvSlot )
{
	OBJECTTYPE *pObj;

	Assert( pSoldier );
	Assert( ( iInvSlot >= 0 ) && ( iInvSlot < NUM_INV_SLOTS ) );

	pObj = &( pSoldier->inv[ iInvSlot ] );

	// if not a player soldier
	if ( pSoldier->bTeam != gbPlayerNum )
	{
		// if it's ammo
		if ( Item[ pObj->usItem ].usItemClass == IC_AMMO )
		{
			//don't drop all the clips, just a random # of them between 1 and how many there are
			pObj->ubNumberOfObjects = ( UINT8 ) ( 1 + Random( pObj->ubNumberOfObjects ) );
			// recalculate the weight
			pObj->ubWeight = CalculateObjectWeight( pObj );
		}
	}
}


void LookForAndMayCommentOnSeeingCorpse( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubLevel )
{
	ROTTING_CORPSE *pCorpse;
  INT8            bToleranceThreshold = 0;
	INT32           cnt;
	SOLDIERTYPE		  *pTeamSoldier;


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

    BeginMultiPurposeLocator( sGridNo, ubLevel, FALSE );

    // Reset values....
    pSoldier->bCorpseQuoteTolerance = (INT8)( Random(3) + 1 );

    // 50% chance of adding 1 to other mercs....
    if ( Random( 2 ) == 1 )
    {
	    // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	    cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	    // look for all mercs on the same team,
	    for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	    {
			  // ATE: Ok, lets check for some basic things here!
			  if ( pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->sGridNo != NOWHERE && pTeamSoldier->bActive && pTeamSoldier->bInSector )
			  {
          pTeamSoldier->bCorpseQuoteTolerance++;
        }
	    }
    }
  }
}


INT16 GetGridNoOfCorpseGivenProfileID( UINT8 ubProfileID )
{
	INT32                    cnt;
	ROTTING_CORPSE					 *pCorpse;

	// Loop through all corpses....
	for ( cnt = 0; cnt < giNumRottingCorpse; cnt++ )
	{
		pCorpse = &(gRottingCorpse[ cnt ] );

		if ( pCorpse->fActivated )
		{
      if ( pCorpse->def.ubProfile == ubProfileID )
      {
        return( pCorpse->def.sGridNo );
      }
    }
  }

  return( NOWHERE );
}

void DecayRottingCorpseAIWarnings( void )
{
	INT32				cnt;
	ROTTING_CORPSE *	pCorpse;

	for ( cnt = 0; cnt < giNumRottingCorpse; cnt++ )
	{
		pCorpse = &(gRottingCorpse[ cnt ] );

		if ( pCorpse->fActivated && pCorpse->def.ubAIWarningValue > 0 )
		{
			pCorpse->def.ubAIWarningValue--;
		}
	}

}

UINT8 GetNearestRottingCorpseAIWarning( INT16 sGridNo )
{
	INT32				cnt;
	ROTTING_CORPSE *	pCorpse;
	UINT8				ubHighestWarning = 0;

	for ( cnt = 0; cnt < giNumRottingCorpse; cnt++ )
	{
		pCorpse = &(gRottingCorpse[ cnt ] );

		if ( pCorpse->fActivated && pCorpse->def.ubAIWarningValue > 0 )
		{
			if ( PythSpacesAway( sGridNo, pCorpse->def.sGridNo ) <= CORPSE_WARNING_DIST )
			{
				if ( pCorpse->def.ubAIWarningValue > ubHighestWarning )
				{
					ubHighestWarning = pCorpse->def.ubAIWarningValue;
				}
			}
		}
	}

	return( ubHighestWarning );
}
