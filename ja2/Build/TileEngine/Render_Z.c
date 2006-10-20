#include "WorldDef.h"
#include "Isometric_Utils.h"

#define GetMapXYWorldY( sWorldCellX, sWorldCellY, sWorldY )\
{\
\
	INT16 gsRDistToCenterX = (sWorldCellX * CELL_X_SIZE) - gCenterWorldX;\
	INT16 gsRDistToCenterY = (sWorldCellY * CELL_Y_SIZE) - gCenterWorldY;\
\
	INT16 gsRScreenCenterY = gsRDistToCenterX + gsRDistToCenterY;\
\
	sWorldY = gsRScreenCenterY + gsCY - gsTLY;\
}


#define	LandZLevel( sMapX, sMapY )\
{\
	sZLevel = LAND_Z_LEVEL;\
}


#define ObjectZLevel( TileElem, pNode, sMapX, sMapY )\
{\
	GetMapXYWorldY( sMapX, sMapY, sWorldY );\
\
	if ( uiTileElemFlags & CLIFFHANG_TILE )\
	{\
		sZLevel=LAND_Z_LEVEL;\
	}\
	else if ( uiTileElemFlags & OBJECTLAYER_USEZHEIGHT )\
	{\
		sZLevel=( ( sWorldY ) *Z_SUBLAYERS)+LAND_Z_LEVEL;\
	}\
	else\
	{\
		sZLevel=OBJECT_Z_LEVEL;\
	}\
}


#define	StructZLevel( sMapX, sMapY )\
{\
\
	GetMapXYWorldY( sMapX, sMapY, sWorldY );\
\
	if ( ( uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE ) )\
	{\
		if ( pCorpse->def.usFlags & ROTTING_CORPSE_VEHICLE )\
		{\
\
			if ( pNode->pStructureData != NULL )\
			{\
				sZOffsetX = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetX;\
				sZOffsetY = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetY;\
			}\
	\
			GetMapXYWorldY( (sMapX + sZOffsetX), (sMapY + sZOffsetY), sWorldY );\
	\
			GetMapXYWorldY( (sMapX + sZOffsetX), (sMapY + sZOffsetY), sWorldY );\
	\
			sZLevel=( ( sWorldY ) * Z_SUBLAYERS)+STRUCT_Z_LEVEL;\
	\
		}\
		else\
		{\
			sZOffsetX = -1;\
			sZOffsetY = -1;\
			GetMapXYWorldY( (sMapX + sZOffsetX), (sMapY + sZOffsetY), sWorldY );\
	\
      sWorldY += 20;\
\
			sZLevel=( ( sWorldY ) * Z_SUBLAYERS)+LAND_Z_LEVEL;\
		}\
	}\
	else if ( uiLevelNodeFlags & LEVELNODE_PHYSICSOBJECT )\
	{\
		sWorldY += pNode->sRelativeZ;\
 \
		sZLevel=(sWorldY*Z_SUBLAYERS)+ONROOF_Z_LEVEL;\
\
	}\
	else if ( uiLevelNodeFlags & LEVELNODE_ITEM )\
	{\
		if ( pNode->pItemPool->bRenderZHeightAboveLevel > 0 )\
		{\
			sZLevel=(sWorldY*Z_SUBLAYERS)+STRUCT_Z_LEVEL;\
			sZLevel+=( pNode->pItemPool->bRenderZHeightAboveLevel );\
		}\
		else\
		{\
			sZLevel=(sWorldY*Z_SUBLAYERS)+OBJECT_Z_LEVEL;\
		}\
	}\
  else if ( uiAniTileFlags & ANITILE_SMOKE_EFFECT )\
  {\
		sZLevel=(sWorldY*Z_SUBLAYERS)+OBJECT_Z_LEVEL;\
  }\
	else if ( ( uiLevelNodeFlags & LEVELNODE_USEZ ) )\
	{\
\
		if ( ( uiLevelNodeFlags & LEVELNODE_NOZBLITTER ) )\
		{\
			sWorldY += 40;\
		}\
		else\
		{\
			sWorldY += pNode->sRelativeZ;\
		}\
\
		sZLevel=(sWorldY*Z_SUBLAYERS)+ONROOF_Z_LEVEL;\
	}\
	else\
	{\
		sZLevel=(sWorldY*Z_SUBLAYERS)+STRUCT_Z_LEVEL;\
	}\
\
}

#define	RoofZLevel( sMapX, sMapY )\
{\
\
	GetMapXYWorldY( sMapX, sMapY, sWorldY );\
\
	sWorldY += WALL_HEIGHT;\
\
	sZLevel=(sWorldY*Z_SUBLAYERS)+ROOF_Z_LEVEL;\
\
}

#define	OnRoofZLevel( sMapX, sMapY )\
{\
\
	GetMapXYWorldY( sMapX, sMapY, sWorldY );\
\
	if ( uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE )\
	{\
		sWorldY += ( WALL_HEIGHT + 40 );\
	}\
	if ( uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE )\
	{\
		sWorldY += ( WALL_HEIGHT + 40 );\
	}\
	else\
	{\
		sWorldY += WALL_HEIGHT;\
	}\
\
	sZLevel=(sWorldY*Z_SUBLAYERS)+ONROOF_Z_LEVEL;\
\
}

#define	TopmostZLevel( sMapX, sMapY )\
{\
\
	GetMapXYWorldY( sMapX, sMapY, sWorldY );\
\
	sZLevel=TOPMOST_Z_LEVEL;\
\
}

#define	ShadowZLevel( sMapX, sMapY )\
{\
\
	GetMapXYWorldY( sMapX, sMapY, sWorldY );\
\
	sZLevel=__max( ( (sWorldY - 80 ) *Z_SUBLAYERS )+SHADOW_Z_LEVEL, 0 );\
\
}

#define SoldierZLevel( pSoldier, sMapX, sMapY )\
{\
\
	if ( ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE ) )\
	{\
\
		if ( pNode->pStructureData != NULL )\
		{\
			sZOffsetX = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetX;\
			sZOffsetY = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetY;\
		}\
\
		GetMapXYWorldY( (sMapX + sZOffsetX), (sMapY + sZOffsetY), sWorldY );\
\
	}\
	else\
	{\
		GetMapXYWorldY( sMapX, sMapY, sWorldY );\
	}\
\
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )\
	{\
		sZLevel=(sWorldY*Z_SUBLAYERS)+STRUCT_Z_LEVEL;\
	}\
	else\
	{\
		if ( pSoldier->dHeightAdjustment > 0 )\
		{\
			sWorldY += ( WALL_HEIGHT + 20 );\
\
			sZLevel=(sWorldY*Z_SUBLAYERS)+ONROOF_Z_LEVEL;\
		}\
		else\
		{\
			if ( ( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )\
			{\
				sZLevel=(sWorldY*Z_SUBLAYERS)+MERC_Z_LEVEL;\
			}\
			else\
			{\
				sZLevel=(sWorldY*Z_SUBLAYERS)+MERC_Z_LEVEL;\
			}\
		}\
	\
		if ( pSoldier->sZLevelOverride != -1 )\
		{\
			sZLevel = pSoldier->sZLevelOverride;\
		}\
	\
		if ( gsForceSoldierZLevel != 0 )\
		{\
			sZLevel = gsForceSoldierZLevel;\
		}\
	}\
\
}
