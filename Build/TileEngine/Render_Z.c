static inline INT16 GetMapXYWorldY(INT32 WorldCellX, INT32 WorldCellY)
{
	INT16 RDistToCenterX = WorldCellX * CELL_X_SIZE - gCenterWorldX;
	INT16 RDistToCenterY = WorldCellY * CELL_Y_SIZE - gCenterWorldY;
	INT16 RScreenCenterY = RDistToCenterX + RDistToCenterY;
	return RScreenCenterY + gsCY - gsTLY;
}

#define	OnRoofZLevel( sMapX, sMapY )                \
{                                                   \
	INT16 sWorldY = GetMapXYWorldY(sMapX, sMapY);     \
                                                    \
	if ( uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE ) \
	{                                                 \
		sWorldY += ( WALL_HEIGHT + 40 );                \
	}                                                 \
	if ( uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE ) \
	{                                                 \
		sWorldY += ( WALL_HEIGHT + 40 );                \
	}                                                 \
	else                                              \
	{                                                 \
		sWorldY += WALL_HEIGHT;                         \
	}                                                 \
                                                    \
	sZLevel=(sWorldY*Z_SUBLAYERS)+ONROOF_Z_LEVEL;     \
}

#define	TopmostZLevel()    \
{                          \
	sZLevel=TOPMOST_Z_LEVEL; \
}

#define SoldierZLevel( pSoldier, sMapX, sMapY )                                        \
{                                                                                      \
	INT16 sWorldY;                                                                       \
	if ( ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE ) )                               \
	{                                                                                    \
		if ( pNode->pStructureData != NULL )                                               \
		{                                                                                  \
			sZOffsetX = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetX; \
			sZOffsetY = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetY; \
		}                                                                                  \
		sWorldY = GetMapXYWorldY(sMapX + sZOffsetX, sMapY + sZOffsetY);                    \
	}                                                                                    \
	else                                                                                 \
	{                                                                                    \
		sWorldY = GetMapXYWorldY(sMapX, sMapY);                                            \
	}                                                                                    \
                                                                                       \
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )                                     \
	{                                                                                    \
		sZLevel=(sWorldY*Z_SUBLAYERS)+STRUCT_Z_LEVEL;                                      \
	}                                                                                    \
	else                                                                                 \
	{                                                                                    \
		if ( pSoldier->dHeightAdjustment > 0 )                                             \
		{                                                                                  \
			sWorldY += ( WALL_HEIGHT + 20 );                                                 \
			sZLevel=(sWorldY*Z_SUBLAYERS)+ONROOF_Z_LEVEL;                                    \
		}                                                                                  \
		else                                                                               \
		{                                                                                  \
			sZLevel = (sWorldY * Z_SUBLAYERS) + MERC_Z_LEVEL;                                \
		}                                                                                  \
	                                                                                     \
		if ( pSoldier->sZLevelOverride != -1 )                                             \
		{                                                                                  \
			sZLevel = pSoldier->sZLevelOverride;                                             \
		}                                                                                  \
	                                                                                     \
		if ( gsForceSoldierZLevel != 0 )                                                   \
		{                                                                                  \
			sZLevel = gsForceSoldierZLevel;                                                  \
		}                                                                                  \
	}                                                                                    \
}
