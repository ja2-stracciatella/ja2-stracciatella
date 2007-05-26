/****************************************************************************************
* JA2 Lighting Module
*
*		Tile-based, ray-casted lighting system.
*
*		Lights are precalculated into linked lists containing offsets from 0,0, and a light
* level to add at that tile. Lists are constructed by casting a ray from the origin of
* the light, and each tile stopped at is stored as a node in the list. To draw the light
* during runtime, you traverse the list, checking at each tile that it isn't of the type
* that can obscure light. If it is, you keep traversing the list until you hit a node
* with a marker LIGHT_NEW_RAY, which means you're back at the origin, and have skipped
* the remainder of the last ray.
*
* Written by Derek Beland, April 14, 1997
*
***************************************************************************************/

#include "Overhead.h"
#include "math.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "VSurface.h"
#include "Input.h"
#include "SysUtil.h"
#include "Debug.h"
#include "WCheck.h"
#include "Edit_Sys.h"
#include "Isometric_Utils.h"
#include "Line.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Timer_Control.h"
#include "Radar_Screen.h"
#include "Render_Dirty.h"
#include "Sys_Globals.h"
#include "TileDef.h"
#include "Lighting.h"
#include "Structure_Internals.h"
#include "Structure_Wrap.h"
#include "Shade_Table_Util.h"
#include "Rotting_Corpses.h"
#include "FileMan.h"
#include "Environment.h"
#include "PathAI.h"
#include "MemMan.h"


#define LVL1_L1_PER			(50)
#define LVL1_L2_PER			(50)

#define LVL2_L1_PER			(30)
#define LVL2_L2_PER			(70)


#define LIGHT_TREE_REVEAL		5						// width of rect


// Top node of linked lists, NULL = FREE
LIGHT_NODE	 *pLightList[MAX_LIGHT_TEMPLATES];
UINT16				usTemplateSize[MAX_LIGHT_TEMPLATES];
UINT16			 *pLightRayList[MAX_LIGHT_TEMPLATES];
UINT16				usRaySize[MAX_LIGHT_TEMPLATES];
INT16					LightHeight[MAX_LIGHT_TEMPLATES];
INT16					LightWidth[MAX_LIGHT_TEMPLATES];
INT16					LightXOffset[MAX_LIGHT_TEMPLATES];
INT16					LightYOffset[MAX_LIGHT_TEMPLATES];
INT16					LightMapLeft[MAX_LIGHT_TEMPLATES];
INT16					LightMapTop[MAX_LIGHT_TEMPLATES];
INT16					LightMapRight[MAX_LIGHT_TEMPLATES];
INT16					LightMapBottom[MAX_LIGHT_TEMPLATES];
STR						pLightNames[MAX_LIGHT_TEMPLATES];

// Sprite data
LIGHT_SPRITE	LightSprites[MAX_LIGHT_SPRITES];

// Lighting system general data
UINT8						ubAmbientLightLevel=DEFAULT_SHADE_LEVEL;
UINT8						gubNumLightColors=1;

// Externed in Rotting Corpses.c
SGPPaletteEntry	gpLightColors[3]={{0,0,0,0}, {0,0,255,0}, {0,0,0,0}};

SGPPaletteEntry gpOrigLights[2]={{0,0,0,0}, {0,0,255,0}};


/*
UINT16 gusShadeLevels[16][3]={{500, 500, 500},				// green table
															{450, 450, 450},		// bright
															{350, 350, 350},
															{300, 300, 300},
															{255, 255, 255},		// normal
															{227, 227, 227},
															{198, 198, 198},
															{171, 171, 171},
															{143, 143, 143},
															{115, 115, 160},				// darkening
															{87, 87, 176},
															{60, 60, 160},
															{48, 48, 192},
															{36, 36, 208},
															{18, 18, 224},
															{48, 222, 48}};
*/
// Externed in Rotting Corpses.c
//Kris' attempt at blue night lights
/*
UINT16 gusShadeLevels[16][3]={{500, 500, 500},				// green table
															{450, 450, 450},		// bright
															{350, 350, 350},
															{300, 300, 300},
															{255, 255, 255},		// normal
															{215, 215, 227},
															{179, 179, 179},
															{149, 149, 149},
															{125, 125, 128},
															{104, 104, 128},				// darkening
															{86, 86, 128},
															{72, 72, 128},
															{60, 60, 128},
															{36, 36, 208},
															{18, 18, 224},
															{48, 222, 48}};
*/

/*
//Linda's final version

UINT16 gusShadeLevels[16][3] =
{
	500, 500, 500,
	450, 450, 450,	//bright
	350, 350, 350,
	300, 300, 300,
	255, 255, 255,	//normal
	222, 200, 200,
	174, 167, 167,
	150, 137, 137,
	122, 116, 116,	//darkening
	96, 96, 96,
	77, 77, 84,
	58, 58, 69,
	44, 44, 66,			//night
	36, 36, 244,
	18, 18, 224,
	48, 222, 48,
};
*/

// JA2 Gold:
static UINT16 gusShadeLevels[16][3] =
{
	500, 500, 500,
	450, 450, 450,	//bright
	350, 350, 350,
	300, 300, 300,
	255, 255, 255,	//normal
	231, 199, 199,
	209, 185, 185,
	187, 171, 171,
	165, 157, 157,	//darkening
	143, 143, 143,
	121, 121, 129,
	99, 99, 115 ,
	77, 77, 101,			//night
	36, 36, 244,
	18, 18, 224,
	48, 222, 48,
};

//Set this true if you want the shadetables to be loaded from the text file.
BOOLEAN gfLoadShadeTablesFromTextFile =		FALSE;

void LoadShadeTablesFromTextFile()
{
	FILE *fp;
	INT32 i, j;
	INT32 num;
	if( gfLoadShadeTablesFromTextFile )
	{
		fp = fopen( "ShadeTables.txt", "r" );
		Assert( fp );
		if( fp )
		{
			for( i = 0; i < 16; i++ )
			{
				for( j = 0; j < 3; j++ )
				{
					char str[10];
					fscanf( fp, "%s", str );
					sscanf( str, "%d", &num );
					gusShadeLevels[i][j] = (UINT16)num;
				}
			}
			fclose( fp );
		}
	}
}

// Debug variable
UINT32	gNodesAdded=0;


static INT32 LightLoad(const char* pFilename);


/****************************************************************************************
 InitLightingSystem

	Initializes the lighting system.

***************************************************************************************/
BOOLEAN InitLightingSystem(void)
{
UINT32 uiCount;

	LoadShadeTablesFromTextFile();

	// init all light lists
	for(uiCount=0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
	{
		pLightList[uiCount]=NULL;
		pLightNames[uiCount]=NULL;
		pLightRayList[uiCount]=NULL;
		usTemplateSize[uiCount]=0;
		usRaySize[uiCount]=0;
	}

	// init all light sprites
	for(uiCount=0; uiCount < MAX_LIGHT_SPRITES; uiCount++)
		memset(&LightSprites[uiCount], 0, sizeof(LIGHT_SPRITE));

	if(LightLoad("TRANSLUC.LHT")!=0)
	{
		DebugMsg(TOPIC_GAME, DBG_LEVEL_0, "Failed to load translucency template");
		return(FALSE);
	}

	return(TRUE);
}

// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
BOOLEAN SetDefaultWorldLightingColors(void)
{
	SGPPaletteEntry pPal[2];

	pPal[0].peRed=0;
	pPal[0].peGreen=0;
	pPal[0].peBlue=0;
	pPal[1].peRed=0;
	pPal[1].peGreen=0;
	pPal[1].peBlue=128;

	LightSetColors(&pPal[0], 1);

	return(TRUE);
}


static BOOLEAN LightDelete(INT32 iLight);


/****************************************************************************************
 ShutdownLightingSystem

	Closes down the lighting system. Any lights that were created are destroyed, and the
	memory attached to them freed up.

***************************************************************************************/
BOOLEAN ShutdownLightingSystem(void)
{
UINT32 uiCount;

	// free up all allocated light nodes
	for(uiCount=0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
		if(pLightList[uiCount]!=NULL)
			LightDelete(uiCount);

	return(TRUE);
}

/****************************************************************************************
 LightReset

	Removes all currently active lights, without undrawing them.

***************************************************************************************/
BOOLEAN LightReset(void)
{
UINT32 uiCount;

	// reset all light lists
	for(uiCount=0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
		if(pLightList[uiCount]!=NULL)
			LightDelete(uiCount);

	// init all light sprites
	for(uiCount=0; uiCount < MAX_LIGHT_SPRITES; uiCount++)
		memset(&LightSprites[uiCount], 0, sizeof(LIGHT_SPRITE));

	if(LightLoad("TRANSLUC.LHT")!=0)
	{
		DebugMsg(TOPIC_GAME, DBG_LEVEL_0, "Failed to load translucency template");
		return(FALSE);
	}

	// Loop through mercs and reset light value
	for ( uiCount = 0; uiCount < MAX_NUM_SOLDIERS; uiCount++ )
	{
		MercPtrs[ uiCount ]->iLight = -1;
	}

	return(TRUE);
}


/* Creates a new node, and appends it to the template list. The index into the
 * list is returned. */
static UINT16 LightCreateTemplateNode(INT32 iLight, INT16 iX, INT16 iY, UINT8 ubLight)
{
UINT16 usNumNodes;


	// create a new list
	if(pLightList[iLight]==NULL)
	{
		if((pLightList[iLight]=MemAlloc(sizeof(LIGHT_NODE)))==NULL)
			return(65535);

		pLightList[iLight]->iDX=iX;
		pLightList[iLight]->iDY=iY;
		pLightList[iLight]->ubLight=ubLight;
		pLightList[iLight]->uiFlags=0;

		usTemplateSize[iLight]=1;
		return(0);
	}
	else
	{
		usNumNodes=usTemplateSize[iLight];
		pLightList[iLight]=MemRealloc(pLightList[iLight], (usNumNodes+1)*sizeof(LIGHT_NODE));
		(pLightList[iLight]+usNumNodes)->iDX=iX;
		(pLightList[iLight]+usNumNodes)->iDY=iY;
		(pLightList[iLight]+usNumNodes)->ubLight=ubLight;
		(pLightList[iLight]+usNumNodes)->uiFlags=0;
		usTemplateSize[iLight]=usNumNodes+1;
		return(usNumNodes);
	}

}


/* Adds a node to the template list. If the node does not exist, it creates a
 * new one.  Returns the index into the list. */
static UINT16 LightAddTemplateNode(INT32 iLight, INT16 iX, INT16 iY, UINT8 ubLight)
{
UINT16 usCount;

		for(usCount=0; usCount < usTemplateSize[iLight]; usCount++)
		{
			if(((pLightList[iLight]+usCount)->iDX==iX) &&
				((pLightList[iLight]+usCount)->iDY==iY))
			{
				return(usCount);
			}
		}

		return(LightCreateTemplateNode(iLight, iX, iY, ubLight));
}


// Adds a node to the ray casting list.
static UINT16 LightAddRayNode(INT32 iLight, INT16 iX, INT16 iY, UINT8 ubLight, UINT16 usFlags)
{
UINT16 usNumNodes;


	// create a new list
	if(pLightRayList[iLight]==NULL)
	{
		if((pLightRayList[iLight]=MemAlloc(sizeof(UINT16)))==NULL)
			return(65535);

		*pLightRayList[iLight]=(LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);

		usRaySize[iLight]=1;
		return(0);
	}
	else
	{
		usNumNodes=usRaySize[iLight];
		pLightRayList[iLight]=MemRealloc(pLightRayList[iLight], (usNumNodes+1)*sizeof(UINT16));
		*(pLightRayList[iLight]+usNumNodes)=(LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);
		usRaySize[iLight]=usNumNodes+1;
		return(usNumNodes);
	}
}


// Adds a node to the ray casting list.
static UINT16 LightInsertRayNode(INT32 iLight, UINT16 usIndex, INT16 iX, INT16 iY, UINT8 ubLight, UINT16 usFlags)
{
UINT16 usNumNodes;

	// create a new list
	if(pLightRayList[iLight]==NULL)
	{
		if((pLightRayList[iLight]=MemAlloc(sizeof(UINT16)))==NULL)
			return(65535);

		*pLightRayList[iLight]=(LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);

		usRaySize[iLight]=1;
		return(0);
	}
	else
	{
		usNumNodes=usRaySize[iLight];
		pLightRayList[iLight]=MemRealloc(pLightRayList[iLight], (usNumNodes+1)*sizeof(UINT16));

		if(usIndex < usRaySize[iLight])
		{
			memmove(pLightRayList[iLight]+usIndex+1,
							pLightRayList[iLight]+usIndex,
							(usRaySize[iLight]-usIndex)*sizeof(UINT16));
		}

		*(pLightRayList[iLight]+usIndex)=(LightAddTemplateNode(iLight, iX, iY, ubLight) | usFlags);
		usRaySize[iLight]=usNumNodes+1;
		return(usNumNodes);
	}
}


static BOOLEAN LightTileHasWall(INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY);


// Returns TRUE/FALSE if the tile at the specified tile number can block light.
static BOOLEAN LightTileBlocked(INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY)
{
UINT16 usTileNo, usSrcTileNo;

	Assert(gpWorldLevelData!=NULL);

	usTileNo=MAPROWCOLTOPOS(iY, iX);
	usSrcTileNo=MAPROWCOLTOPOS(iSrcY, iSrcX);

	if ( usTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	if ( usSrcTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	if(gpWorldLevelData[ usTileNo ].sHeight > gpWorldLevelData[ usSrcTileNo ].sHeight)
		return(TRUE);
	{
		UINT16 usTileNo;
		LEVELNODE *pStruct;

		usTileNo=MAPROWCOLTOPOS(iY, iX);

		pStruct = gpWorldLevelData[ usTileNo ].pStructHead;
		if ( pStruct != NULL )
		{
			// IF WE ARE A WINDOW, DO NOT BLOCK!
			if ( FindStructure( usTileNo, STRUCTURE_WALLNWINDOW ) != NULL )
			{
				return( FALSE );
			}
		}
	}

	return(LightTileHasWall( iSrcX, iSrcY, iX, iY));
}


// Returns TRUE/FALSE if the tile at the specified coordinates contains a wall.
static BOOLEAN LightTileHasWall(INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY)
{
//LEVELNODE *pStruct;
//UINT32 uiType;
UINT16 usTileNo;
UINT16 usSrcTileNo;
INT8		bDirection;
UINT8		ubTravelCost;
//INT8		bWallCount = 0;
//UINT16	usWallOrientation;

	Assert(gpWorldLevelData!=NULL);

	usTileNo=MAPROWCOLTOPOS(iY, iX);
	usSrcTileNo=MAPROWCOLTOPOS(iSrcY, iSrcX);

	if ( usTileNo == usSrcTileNo )
	{
		return( FALSE );
	}

	//if ( usTileNo == 10125 || usTileNo == 10126 )
	//{
	//	int i = 0;
	//}

	if ( usTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	if ( usSrcTileNo >= NOWHERE )
	{
		return( FALSE );
	}

	// Get direction
	//bDirection = atan8( iX, iY, iSrcX, iSrcY );
	bDirection = atan8( iSrcX, iSrcY, iX, iY );

	ubTravelCost = gubWorldMovementCosts[ usTileNo ][ bDirection ][ 0 ];

	if ( ubTravelCost == TRAVELCOST_WALL  )
	{
		return( TRUE );
	}

	if ( IS_TRAVELCOST_DOOR( ubTravelCost ) )
	{
		ubTravelCost = DoorTravelCost( NULL, usTileNo, ubTravelCost, TRUE, NULL );

		if ( ubTravelCost == TRAVELCOST_OBSTACLE || ubTravelCost == TRAVELCOST_DOOR )
		{
			return( TRUE );
		}
	}

#if 0
	pStruct = gpWorldLevelData[ usTileNo ].pStructHead;
	while ( pStruct != NULL )
	{
		if ( pStruct->usIndex < NUMBEROFTILES )
		{
			GetTileType( pStruct->usIndex, &uiType );

			// ATE: Changed to use last decordations rather than last decal
			// Could maybe check orientation value? Depends on our
			// use of the orientation value flags..
			if((uiType >= FIRSTWALL) && (uiType <=LASTDECORATIONS ))
			{
				GetWallOrientation(pStruct->usIndex, &usWallOrientation);

				bWallCount++;
			}
		}

		pStruct=pStruct->pNext;
	}

	if ( bWallCount )
	{
		// ATE: If TWO or more - assume it's BLOCKED and return TRUE
		if ( bWallCount != 1 )
		{
			return( TRUE );
		}

		switch(usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				return( iSrcX < iX );

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				return( iSrcY < iY );

		}
	}

#endif

	return(FALSE);
}


// Removes a light template from the list, and frees up the associated node memory.
static BOOLEAN LightDelete(INT32 iLight)
{
		if(pLightList[iLight]!=NULL)
		{
			if(pLightList[iLight]!=NULL)
			{
				MemFree(pLightList[iLight]);
				pLightList[iLight]=NULL;
			}

			if(pLightRayList[iLight]!=NULL)
			{
				MemFree(pLightRayList[iLight]);
				pLightRayList[iLight]=NULL;
			}

			if(pLightNames[iLight]!=NULL)
			{
				MemFree(pLightNames[iLight]);
				pLightNames[iLight]=NULL;
			}

			usTemplateSize[iLight]=0;
			usRaySize[iLight]=0;

			return(TRUE);
		}
		else
			return(FALSE);
}


/* Returns an available slot for a new light template. If none are available,
 * (-1) is returned. */
static INT32 LightGetFree(void)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < MAX_LIGHT_TEMPLATES; uiCount++)
		if(pLightList[uiCount]==NULL)
			return(uiCount);

	return(-1);
}


// Calculates the 2D linear distance between two points.
static INT32 LinearDistance(INT16 iX1, INT16 iY1, INT16 iX2, INT16 iY2)
{
INT32 iDx, iDy;

	iDx=abs(iX1-iX2);
	iDx*=iDx;
	iDy=abs(iY1-iY2);
	iDy*=iDy;

	return((INT32)sqrt((DOUBLE)(iDx+iDy)));
}


/* Calculates the 2D linear distance between two points. Returns the result in
 * a DOUBLE for greater accuracy. */
static DOUBLE LinearDistanceDouble(INT16 iX1, INT16 iY1, INT16 iX2, INT16 iY2)
{
INT32 iDx, iDy;

	iDx=abs(iX1-iX2);
	iDx*=iDx;
	iDy=abs(iY1-iY2);
	iDy*=iDy;

	return(sqrt((DOUBLE)(iDx+iDy)));
}

/****************************************************************************************
	LightTrueLevel

		Returns the light level at a particular level without fake lights

***************************************************************************************/
UINT8 LightTrueLevel( INT16 sGridNo, INT8 bLevel )
{
	LEVELNODE * pNode;
	INT32 iSum;

	if (bLevel == 0)
	{
		pNode = gpWorldLevelData[sGridNo].pLandHead;
	}
	else
	{
		pNode = gpWorldLevelData[sGridNo].pRoofHead;
	}

	if (pNode == NULL)
	{
		return( ubAmbientLightLevel );
	}
	else
	{
		iSum=pNode->ubNaturalShadeLevel - (pNode->ubSumLights - pNode->ubFakeShadeLevel );

		iSum=__min(SHADE_MIN, iSum);
		iSum=__max(SHADE_MAX, iSum);
		return( (UINT8) iSum );
	}
}


// Does the addition of light values to individual LEVELNODEs in the world tile list.
static void LightAddTileNode(LEVELNODE* pNode, UINT32 uiLightType, UINT8 ubShadeAdd, BOOLEAN fFake)
{
INT16 sSum;

	pNode->ubSumLights += ubShadeAdd;
	if (fFake)
	{
		pNode->ubFakeShadeLevel += ubShadeAdd;
	}

	// Now set max
	pNode->ubMaxLights = __max( pNode->ubMaxLights, ubShadeAdd );

	sSum=pNode->ubNaturalShadeLevel - pNode->ubMaxLights;

	sSum=__min(SHADE_MIN, sSum);
	sSum=__max(SHADE_MAX, sSum);

	pNode->ubShadeLevel=(UINT8)sSum;
}


// Does the subtraction of light values to individual LEVELNODEs in the world tile list.
static void LightSubtractTileNode(LEVELNODE* pNode, UINT32 uiLightType, UINT8 ubShadeSubtract, BOOLEAN fFake)
{
INT16 sSum;

	if (ubShadeSubtract > pNode->ubSumLights )
	{
		pNode->ubSumLights = 0;
	}
	else
	{
		pNode->ubSumLights -= ubShadeSubtract;
	}
	if (fFake)
	{
		if (ubShadeSubtract > pNode->ubFakeShadeLevel)
		{
			pNode->ubFakeShadeLevel = 0;
		}
		else
		{
			pNode->ubFakeShadeLevel -= ubShadeSubtract;
		}
	}

	// Now set max
	pNode->ubMaxLights = __min( pNode->ubMaxLights, pNode->ubSumLights );


	sSum=pNode->ubNaturalShadeLevel - pNode->ubMaxLights;

	sSum=__min(SHADE_MIN, sSum);
	sSum=__max(SHADE_MAX, sSum);

	pNode->ubShadeLevel=(UINT8)sSum;
}


static BOOLEAN LightIlluminateWall(INT16 iSourceX, INT16 iSourceY, INT16 iTileX, INT16 iTileY, LEVELNODE* pStruct);


// Adds a specified amount of light to all objects on a given tile.
static BOOLEAN LightAddTile(UINT32 uiLightType, INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY, UINT8 ubShade, UINT32 uiFlags, BOOLEAN fOnlyWalls)
{
LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;
UINT8 ubShadeAdd;
UINT32 uiTile;
BOOLEAN fLitWall=FALSE;
BOOLEAN fFake;

	Assert(gpWorldLevelData!=NULL);

	uiTile= MAPROWCOLTOPOS( iY, iX );

	if ( uiTile >= NOWHERE )
	{
		return( FALSE );
	}

	gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;

	//if((uiFlags&LIGHT_BACKLIGHT) && !(uiFlags&LIGHT_ROOF_ONLY))
	//	ubShadeAdd=ubShade*7/10;
	//else
		ubShadeAdd=ubShade;


	if (uiFlags&LIGHT_FAKE)
	{
		fFake = TRUE;
	}
	else
	{
		fFake = FALSE;
	}

	if(!(uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pStruct = gpWorldLevelData[uiTile].pStructHead;
		while(pStruct!=NULL)
		{
			if ( pStruct->usIndex < NUMBEROFTILES )
			{
				if((gTileDatabase[ pStruct->usIndex ].fType != FIRSTCLIFFHANG) || (uiFlags&LIGHT_EVERYTHING))
				{
					if( (uiFlags&LIGHT_IGNORE_WALLS ) || gfCaves )
						LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
					else if(LightIlluminateWall(iSrcX, iSrcY, iX, iY, pStruct))
					{
						if(LightTileHasWall(iSrcX, iSrcY, iX, iY))
							fLitWall=TRUE;

						// ATE: Limit shade for walls if in caves
						if ( fLitWall && gfCaves )
						{
							LightAddTileNode(pStruct, uiLightType, ( UINT8 ) __min( ubShadeAdd, ( SHADE_MAX + 5 ) ), FALSE);
						}
						else if ( fLitWall )
						{
							LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
						}
						else if ( !fOnlyWalls )
						{
							LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
						}
					}
				}
			}
			else
			{
				LightAddTileNode(pStruct, uiLightType, ubShadeAdd, FALSE);
			}
			pStruct=pStruct->pNext;
		}

		ubShadeAdd=ubShade;

    if ( !fOnlyWalls )
    {
		  pLand = gpWorldLevelData[uiTile].pLandHead;

		  while( pLand )
		  {
			  if( gfCaves || !fLitWall )
			  {
				  LightAddTileNode(pLand, uiLightType, ubShadeAdd, fFake);
			  }
			  pLand=pLand->pNext;
		  }

		  pObject = gpWorldLevelData[uiTile].pObjectHead;
		  while(pObject!=NULL)
		  {
			  if ( pObject->usIndex < NUMBEROFTILES )
			  {
				  LightAddTileNode(pObject, uiLightType, ubShadeAdd, FALSE);
			  }
			  pObject=pObject->pNext;
		  }

		  if(uiFlags&LIGHT_BACKLIGHT)
			  ubShadeAdd=(INT16)ubShade*7/10;

		  pMerc = gpWorldLevelData[uiTile].pMercHead;
		  while(pMerc!=NULL)
		  {
			  LightAddTileNode(pMerc, uiLightType, ubShadeAdd, FALSE);
			  pMerc=pMerc->pNext;
		  }
	  }
  }

	if((uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pRoof = gpWorldLevelData[uiTile].pRoofHead;
		while(pRoof!=NULL)
		{
			if ( pRoof->usIndex < NUMBEROFTILES )
			{
				LightAddTileNode(pRoof, uiLightType, ubShadeAdd, fFake);
			}
			pRoof=pRoof->pNext;
		}

		pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
		while(pOnRoof!=NULL)
		{
			LightAddTileNode(pOnRoof, uiLightType, ubShadeAdd, FALSE);

			pOnRoof=pOnRoof->pNext;
		}
	}
	return(TRUE);
}


// Subtracts a specified amount of light to a given tile.
static BOOLEAN LightSubtractTile(UINT32 uiLightType, INT16 iSrcX, INT16 iSrcY, INT16 iX, INT16 iY, UINT8 ubShade, UINT32 uiFlags, BOOLEAN fOnlyWalls)
{
LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;
UINT8 ubShadeSubtract;
UINT32 uiTile;
BOOLEAN fLitWall=FALSE;
BOOLEAN fFake; // only passed in to land and roof layers; others get fed FALSE

	Assert(gpWorldLevelData!=NULL);

	uiTile= MAPROWCOLTOPOS( iY, iX );

	if ( uiTile >= NOWHERE )
	{
		return( FALSE );
	}


	gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;

//	if((uiFlags&LIGHT_BACKLIGHT) && !(uiFlags&LIGHT_ROOF_ONLY))
//		ubShadeSubtract=ubShade*7/10;
//	else
		ubShadeSubtract=ubShade;

	if (uiFlags&LIGHT_FAKE)
	{
		fFake = TRUE;
	}
	else
	{
		fFake = FALSE;
	}

	if(!(uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pStruct = gpWorldLevelData[uiTile].pStructHead;
		while(pStruct!=NULL)
		{
			if ( pStruct->usIndex < NUMBEROFTILES )
			{
				if((gTileDatabase[ pStruct->usIndex ].fType != FIRSTCLIFFHANG) || (uiFlags&LIGHT_EVERYTHING))
				{
					if( (uiFlags&LIGHT_IGNORE_WALLS ) || gfCaves )
						LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
					else if(LightIlluminateWall(iSrcX, iSrcY, iX, iY, pStruct))
					{
						if(LightTileHasWall( iSrcX, iSrcY, iX, iY))
							fLitWall=TRUE;

						// ATE: Limit shade for walls if in caves
						if ( fLitWall && gfCaves )
						{
							LightSubtractTileNode(pStruct, uiLightType, ( UINT8 ) __max( ( ubShadeSubtract - 5 ), 0 ), FALSE);
						}
						else if ( fLitWall )
						{
							LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
						}
            else if ( !fOnlyWalls )
            {
						  LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
            }
					}
				}
			}
      else
      {
				LightSubtractTileNode(pStruct, uiLightType, ubShadeSubtract, FALSE);
      }
			pStruct=pStruct->pNext;
		}

		ubShadeSubtract=ubShade;

    if ( !fOnlyWalls )
    {
		  pLand = gpWorldLevelData[uiTile].pLandHead;

		  while( pLand )
		  {
			  if( gfCaves || !fLitWall )
			  {
				  LightSubtractTileNode(pLand, uiLightType, ubShadeSubtract, fFake);
			  }
			  pLand=pLand->pNext;
		  }

		  pObject = gpWorldLevelData[uiTile].pObjectHead;
		  while(pObject!=NULL)
		  {
			  if ( pObject->usIndex < NUMBEROFTILES )
			  {
				  LightSubtractTileNode(pObject, uiLightType, ubShadeSubtract, FALSE);
			  }
			  pObject=pObject->pNext;
		  }

		  if(uiFlags&LIGHT_BACKLIGHT)
			  ubShadeSubtract=(INT16)ubShade*7/10;

		  pMerc = gpWorldLevelData[uiTile].pMercHead;
		  while(pMerc!=NULL)
		  {
			  LightSubtractTileNode(pMerc, uiLightType, ubShadeSubtract, FALSE);
			  pMerc=pMerc->pNext;
		  }
	  }
  }

	if((uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pRoof = gpWorldLevelData[uiTile].pRoofHead;
		while(pRoof!=NULL)
		{
			if ( pRoof->usIndex < NUMBEROFTILES )
			{
				LightSubtractTileNode(pRoof, uiLightType, ubShadeSubtract, fFake);
			}
			pRoof=pRoof->pNext;
		}

		pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
		while(pOnRoof!=NULL)
		{
			if ( pOnRoof->usIndex < NUMBEROFTILES )
			{
				LightSubtractTileNode(pOnRoof, uiLightType, ubShadeSubtract, FALSE);
			}
			pOnRoof=pOnRoof->pNext;
		}
	}

	return(TRUE);
}


// Sets the natural light level (as well as the current) on individual LEVELNODEs.
static void LightSetNaturalTileNode(LEVELNODE* pNode, UINT8 ubShade)
{
		Assert(pNode!=NULL);

		pNode->ubSumLights=0;
		pNode->ubMaxLights=0;
		pNode->ubNaturalShadeLevel = ubShade;
		pNode->ubShadeLevel = ubShade;
		//LightAddTileNode(pNode, 0, (INT16)(SHADE_MIN-ubShade));
}


/* Sets the natural light value of all objects on a given tile to the specified
 * value.  This is the light value a tile has with no artificial lighting
 * affecting it. */
static BOOLEAN LightSetNaturalTile(INT16 iX, INT16 iY, UINT8 ubShade)
{
LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pMerc;
UINT32 uiIndex;

	CHECKF(gpWorldLevelData!=NULL);

	uiIndex = MAPROWCOLTOPOS( iY, iX );

	Assert(uiIndex!=0xffff);

	ubShade=__max(SHADE_MAX, ubShade);
	ubShade=__min(SHADE_MIN, ubShade);

	pLand = gpWorldLevelData[ uiIndex ].pLandHead;

	while(pLand!=NULL)
	{
		LightSetNaturalTileNode(pLand, ubShade);
		pLand=pLand->pNext;
	}

	pStruct = gpWorldLevelData[ uiIndex ].pStructHead;

	while(pStruct!=NULL)
	{
		LightSetNaturalTileNode(pStruct, ubShade);
		pStruct=pStruct->pNext;
	}

	pObject = gpWorldLevelData[ uiIndex ].pObjectHead;
	while(pObject!=NULL)
	{
		LightSetNaturalTileNode(pObject, ubShade);
		pObject=pObject->pNext;
	}

	pRoof = gpWorldLevelData[ uiIndex ].pRoofHead;
	while(pRoof!=NULL)
	{
		LightSetNaturalTileNode(pRoof, ubShade);
		pRoof=pRoof->pNext;
	}

	pOnRoof = gpWorldLevelData[ uiIndex ].pOnRoofHead;
	while(pOnRoof!=NULL)
	{
		LightSetNaturalTileNode(pOnRoof, ubShade);
		pOnRoof=pOnRoof->pNext;
	}

	pTopmost = gpWorldLevelData[ uiIndex ].pTopmostHead;
	while(pTopmost!=NULL)
	{
		LightSetNaturalTileNode(pTopmost, ubShade);
		pTopmost=pTopmost->pNext;
	}

	pMerc = gpWorldLevelData[ uiIndex ].pMercHead;
	while(pMerc!=NULL)
	{
		LightSetNaturalTileNode(pMerc, ubShade);
		pMerc=pMerc->pNext;
	}
	return(TRUE);
}


/* Resets the light level of individual LEVELNODEs to the value contained in
 * the natural light level. */
static void LightResetTileNode(LEVELNODE* pNode)
{
	pNode->ubSumLights=0;
	pNode->ubMaxLights=0;
	pNode->ubShadeLevel = pNode->ubNaturalShadeLevel;
	pNode->ubFakeShadeLevel = 0;
}


/* Resets the light values of all objects on a given tile to the "natural"
 * light level for that tile. */
static BOOLEAN LightResetTile(INT16 iX, INT16 iY)
{
LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pMerc;
UINT32 uiTile;

	CHECKF(gpWorldLevelData!=NULL);

	uiTile = MAPROWCOLTOPOS( iY, iX );

	CHECKF(uiTile!=0xffff);

	pLand = gpWorldLevelData[uiTile].pLandHead;

	while(pLand!=NULL)
	{
		LightResetTileNode(pLand);
		pLand=pLand->pNext;
	}

	pStruct = gpWorldLevelData[ uiTile ].pStructHead;

	while(pStruct!=NULL)
	{
		LightResetTileNode(pStruct);
		pStruct=pStruct->pNext;
	}

	pObject = gpWorldLevelData[ uiTile ].pObjectHead;
	while(pObject!=NULL)
	{
		LightResetTileNode(pObject);
		pObject=pObject->pNext;
	}

	pRoof = gpWorldLevelData[ uiTile ].pRoofHead;
	while(pRoof!=NULL)
	{
		LightResetTileNode(pRoof);
		pRoof=pRoof->pNext;
	}

	pOnRoof = gpWorldLevelData[ uiTile ].pOnRoofHead;
	while(pOnRoof!=NULL)
	{
		LightResetTileNode(pOnRoof);
		pOnRoof=pOnRoof->pNext;
	}

	pTopmost = gpWorldLevelData[ uiTile ].pTopmostHead;
	while(pTopmost!=NULL)
	{
		LightResetTileNode(pTopmost);
		pTopmost=pTopmost->pNext;
	}

	pMerc = gpWorldLevelData[ uiTile ].pMercHead;
	while(pMerc!=NULL)
	{
		LightResetTileNode(pMerc);
		pMerc=pMerc->pNext;
	}

	return(TRUE);
}


// Resets all tiles on the map to their baseline values.
static BOOLEAN LightResetAllTiles(void)
{
INT16 iCountY, iCountX;

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightResetTile(iCountX, iCountY);

	return(TRUE);
}


// Sets the current light value of all objects on a given tile to the specified value.
static BOOLEAN LightSetTile(INT16 iX, INT16 iY, UINT8 ubShade, UINT32 uiLightType)
{
/*LEVELNODE *pLand, *pStruct, *pObject, *pRoof, *pOnRoof, *pTopmost, *pFog;
UINT32 uiIndex;

	ubShade=__max(SHADE_MAX, ubShade);
	ubShade=__min(SHADE_MIN, ubShade);

	uiIndex = MAPROWCOLTOPOS( iY, iX );
	pLand = gpWorldLevelData[ uiIndex ].pLandHead;

	while(pLand!=NULL)
	{
		pLand->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pLand->ubShadeLevel = ubShade;
		pLand=pLand->pNext;
	}

	pStruct = gpWorldLevelData[ uiIndex ].pStructHead;

	while(pStruct!=NULL)
	{
		pStruct->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pStruct->ubShadeLevel = ubShade;
		pStruct=pStruct->pNext;
	}

	pObject = gpWorldLevelData[ uiIndex ].pObjectHead;
	while(pObject!=NULL)
	{
		pObject->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pObject->ubShadeLevel = ubShade;
		pObject=pObject->pNext;
	}

	pRoof = gpWorldLevelData[ uiIndex ].pRoofHead;
	while(pRoof!=NULL)
	{
		pRoof->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pRoof->ubShadeLevel = ubShade;
		pRoof=pRoof->pNext;
	}

	pOnRoof = gpWorldLevelData[ uiIndex ].pOnRoofHead;
	while(pOnRoof!=NULL)
	{
		pOnRoof->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pOnRoof->ubShadeLevel = ubShade;
		pOnRoof=pOnRoof->pNext;
	}

	pFog = gpWorldLevelData[ uiIndex ].pFogHead;
	while(pFog!=NULL)
	{
		pFog->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pFog->ubShadeLevel = ubShade;
		pFog=pFog->pNext;
	}

	pTopmost = gpWorldLevelData[ uiIndex ].pTopmostHead;
	while(pTopmost!=NULL)
	{
		pTopmost->sSumLights[uiLightType]= ubShade-SHADE_MAX;
		pTopmost->ubShadeLevel = ubShade;
		pTopmost=pTopmost->pNext;
	} */
	return(TRUE);
}


/* Returns a pointer to the last node in a light list. If the list is empty,
 * NULL is returned. */
static UINT16 LightGetLastNode(INT32 iLight)
{
		return(usRaySize[iLight]);
}


// Creates a new node, and adds it to the end of a light list.
static BOOLEAN LightAddNode(INT32 iLight, INT16 iHotSpotX, INT16 iHotSpotY, INT16 iX, INT16 iY, UINT8 ubIntensity, UINT16 uiFlags)
{
DOUBLE dDistance;
UINT8 ubShade;
INT32 iLightDecay;

	dDistance=LinearDistanceDouble(iX, iY, iHotSpotX, iHotSpotY);
	dDistance/=DISTANCE_SCALE;

	iLightDecay=(INT32)(dDistance*LIGHT_DECAY);

	if((iLightDecay >= (INT32)ubIntensity))
		ubShade=0;
	else
		ubShade=ubIntensity-(UINT8)iLightDecay;

	iX/=DISTANCE_SCALE;
	iY/=DISTANCE_SCALE;

	LightAddRayNode(iLight, iX, iY, ubShade, uiFlags);
	return(TRUE);
}


// Creates a new node, and inserts it after the specified node.
static BOOLEAN LightInsertNode(INT32 iLight, UINT16 usLightIns, INT16 iHotSpotX, INT16 iHotSpotY, INT16 iX, INT16 iY, UINT8 ubIntensity, UINT16 uiFlags)
{
DOUBLE dDistance;
UINT8 ubShade;
INT32 iLightDecay;

	dDistance=LinearDistanceDouble(iX, iY, iHotSpotX, iHotSpotY);
	dDistance/=DISTANCE_SCALE;

	iLightDecay=(INT32)(dDistance*LIGHT_DECAY);

	if((iLightDecay >= (INT32)ubIntensity))
		ubShade=0;
	else
		ubShade=ubIntensity-(UINT8)iLightDecay;

	iX/=DISTANCE_SCALE;
	iY/=DISTANCE_SCALE;

	LightInsertRayNode(iLight, usLightIns, iX, iY, ubShade, uiFlags);

	return(TRUE);
}


/* Traverses the linked list until a node with the LIGHT_NEW_RAY marker, and
 * returns the pointer. If the end of list is reached, NULL is returned. */
static UINT16 LightFindNextRay(INT32 iLight, UINT16 usIndex)
{
UINT16 usNodeIndex;

	usNodeIndex=usIndex;
	while((usNodeIndex < usRaySize[iLight]) && !(*(pLightRayList[iLight]+usNodeIndex)&LIGHT_NEW_RAY))
		usNodeIndex++;

	return(usNodeIndex);
}


/* Casts a ray from an origin to an end point, creating nodes and adding them
 * to the light list. */
static BOOLEAN LightCastRay(INT32 iLight, INT16 iStartX, INT16 iStartY, INT16 iEndPointX, INT16 iEndPointY, UINT8 ubStartIntens, UINT8 ubEndIntens)
{
INT16 AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
INT32 WholeStep, InitialPixelCount, FinalPixelCount, i, j, RunLength;
INT16 iXPos, iYPos, iEndY, iEndX;
UINT16 usCurNode=0, usFlags=0;
BOOLEAN fInsertNodes=FALSE;

		if((iEndPointX > 0) && (iEndPointY > 0))
			usFlags=LIGHT_BACKLIGHT;

   /* We'll always draw top to bottom, to reduce the number of cases we have to
   handle, and to make lines between the same endpoints draw the same pixels */
		if (iStartY > iEndPointY)
		{
			iXPos=iEndPointX;
			iEndX=iStartX;
			iYPos=iEndPointY;
			iEndY=iStartY;
			fInsertNodes=TRUE;
		}
		else
		{
			iXPos=iStartX;
			iEndX=iEndPointX;
			iYPos=iStartY;
			iEndY=iEndPointY;
		}

	 /* Figure out whether we're going left or right, and how far we're
      going horizontally */
   if ((XDelta = (iEndX - iXPos)) < 0)
   {
      XAdvance = -1;
      XDelta = -XDelta;
   }
   else
   {
      XAdvance = 1;
   }
   /* Figure out how far we're going vertically */
   YDelta = iEndY - iYPos;

	 // Check for 0 length ray
	 if((XDelta==0) && (YDelta==0))
		 return(FALSE);

	//DebugMsg(TOPIC_GAME, DBG_LEVEL_0, String("Drawing (%d,%d) to (%d,%d)", iXPos, iYPos, iEndX, iEndY));
	 LightAddNode(iLight, 32767, 32767, 32767, 32767, 0, LIGHT_NEW_RAY);
		if(fInsertNodes)
			usCurNode=LightGetLastNode(iLight);


		/* Special-case horizontal, vertical, and diagonal lines, for speed
      and to avoid nasty boundary conditions and division by 0 */
   if (XDelta == 0)
   {
     /* Vertical line */
		 if(fInsertNodes)
		 {
				for (i=0; i<=YDelta; i++)
				{
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
		 }
		 else
		 {
				for (i=0; i<=YDelta; i++)
				{
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
		 }
     return(TRUE);
   }
   if (YDelta == 0)
   {
      /* Horizontal line */
		 if(fInsertNodes)
		 {
				for (i=0; i<=XDelta; i++)
				{
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
			  }
		 }
		 else
		 {
				for (i=0; i<=XDelta; i++)
				{
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
			  }
		 }
      return(TRUE);
   }
   if (XDelta == YDelta)
   {
      /* Diagonal line */
		 if(fInsertNodes)
		 {
	      for (i=0; i<=XDelta; i++)
		    {
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
					iYPos++;
				}
		 }
		 else
		 {
				for (i=0; i<=XDelta; i++)
				{
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
					iYPos++;
				}
		 }
      return(TRUE);
   }

   /* Determine whether the line is X or Y major, and handle accordingly */
   if (XDelta >= YDelta)
   {
			/* X major line */
      /* Minimum # of pixels in a run in this line */
      WholeStep = XDelta / YDelta;

      /* Error term adjust each time Y steps by 1; used to tell when one
         extra pixel should be drawn as part of a run, to account for
         fractional steps along the X axis per 1-pixel steps along Y */
      AdjUp = (XDelta % YDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the X step made at that time */
      AdjDown = YDelta * 2;

      /* Initial error term; reflects an initial step of 0.5 along the Y
         axis */
      ErrorTerm = (XDelta % YDelta) - (YDelta * 2);

      /* The initial and last runs are partial, because Y advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      InitialPixelCount = (WholeStep / 2) + 1;
      FinalPixelCount = InitialPixelCount;

      /* If the basic run length is even and there's no fractional
         advance, we have one pixel that could go to either the initial
         or last partial run, which we'll arbitrarily allocate to the
         last run */
      if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
      {
         InitialPixelCount--;
      }
      /* If there're an odd number of pixels per run, we have 1 pixel that can't
         be allocated to either the initial or last partial run, so we'll add 0.5
         to error term so this pixel will be handled by the normal full-run loop */
      if ((WholeStep & 0x01) != 0)
      {
         ErrorTerm += YDelta;
      }
      /* Draw the first, partial run of pixels */
      //DrawHorizontalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);
			if(fInsertNodes)
			{
				for (i=0; i<InitialPixelCount; i++)
				{
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			else
			{
				for (i=0; i<InitialPixelCount; i++)
		    {
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			iYPos++;

      /* Draw all full runs */
      for (j=0; j<(YDelta-1); j++)
      {
         RunLength = WholeStep;  /* run is at least this long */
         /* Advance the error term and add an extra pixel if the error
            term so indicates */
         if ((ErrorTerm += AdjUp) > 0)
         {
            RunLength++;
            ErrorTerm -= AdjDown;   /* reset the error term */
         }
         /* Draw this scan line's run */
         //DrawHorizontalRun(&ScreenPtr, XAdvance, RunLength, Color);
				 if(fInsertNodes)
				 {
						for (i=0; i<RunLength; i++)
						{
							LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
							iXPos+=XAdvance;
						}
				 }
				 else
				 {
						for (i=0; i<RunLength; i++)
						{
							LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
							iXPos+=XAdvance;
						}
				 }
				iYPos++;
      }
      /* Draw the final run of pixels */
      //DrawHorizontalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
      if(fInsertNodes)
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			else
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			iYPos++;
   }
   else
   {
		 /* Y major line */

      /* Minimum # of pixels in a run in this line */
      WholeStep = YDelta / XDelta;

      /* Error term adjust each time X steps by 1; used to tell when 1 extra
         pixel should be drawn as part of a run, to account for
         fractional steps along the Y axis per 1-pixel steps along X */
      AdjUp = (YDelta % XDelta) * 2;

      /* Error term adjust when the error term turns over, used to factor
         out the Y step made at that time */
      AdjDown = XDelta * 2;

      /* Initial error term; reflects initial step of 0.5 along the X axis */
      ErrorTerm = (YDelta % XDelta) - (XDelta * 2);

      /* The initial and last runs are partial, because X advances only 0.5
         for these runs, rather than 1. Divide one full run, plus the
         initial pixel, between the initial and last runs */
      InitialPixelCount = (WholeStep / 2) + 1;
      FinalPixelCount = InitialPixelCount;

      /* If the basic run length is even and there's no fractional advance, we
         have 1 pixel that could go to either the initial or last partial run,
         which we'll arbitrarily allocate to the last run */
      if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
      {
         InitialPixelCount--;
      }
      /* If there are an odd number of pixels per run, we have one pixel
         that can't be allocated to either the initial or last partial
         run, so we'll add 0.5 to the error term so this pixel will be
         handled by the normal full-run loop */
      if ((WholeStep & 0x01) != 0)
      {
         ErrorTerm += XDelta;
      }
      /* Draw the first, partial run of pixels */
			if(fInsertNodes)
			{
				for (i=0; i<InitialPixelCount; i++)
				{
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			else
			{
				for (i=0; i<InitialPixelCount; i++)
				{
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			iXPos+=XAdvance;
      //DrawVerticalRun(&ScreenPtr, XAdvance, InitialPixelCount, Color);

      /* Draw all full runs */
      for (j=0; j<(XDelta-1); j++)
      {
         RunLength = WholeStep;  /* run is at least this long */
         /* Advance the error term and add an extra pixel if the error
            term so indicates */
         if ((ErrorTerm += AdjUp) > 0)
         {
            RunLength++;
            ErrorTerm -= AdjDown;   /* reset the error term */
         }
         /* Draw this scan line's run */
         //DrawVerticalRun(&ScreenPtr, XAdvance, RunLength, Color);
				if(fInsertNodes)
				{
					for (i=0; i<RunLength; i++)
					{
						LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
						iYPos++;
					}
				}
				else
				{
					for (i=0; i<RunLength; i++)
					{
						LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
						iYPos++;
					}
				}
				iXPos+=XAdvance;
      }
      /* Draw the final run of pixels */
      //DrawVerticalRun(&ScreenPtr, XAdvance, FinalPixelCount, Color);
			if(fInsertNodes)
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightInsertNode(iLight, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			else
			{
				for (i=0; i<FinalPixelCount; i++)
				{
					LightAddNode(iLight, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			iXPos+=XAdvance;
   }
	 return(TRUE);
}


// Creates an elliptical light, taking two radii.
static BOOLEAN LightGenerateElliptical(INT32 iLight, UINT8 iIntensity, INT16 iA, INT16 iB)
{
INT16 iX, iY;
INT32 WorkingX, WorkingY;
DOUBLE ASquared;
DOUBLE BSquared;
DOUBLE Temp;

	iX=0;
	iY=0;
	ASquared = (DOUBLE) iA * iA;
	BSquared = (DOUBLE) iB * iB;

   /* Draw the four symmetric arcs for which X advances faster (that is,
      for which X is the major axis) */
   /* Draw the initial top & bottom points */
   LightCastRay(iLight, iX, iY, (INT16)iX, (INT16)(iY+iB), iIntensity, 1);
   LightCastRay(iLight, iX, iY, (INT16)iX, (INT16)(iY-iB), iIntensity, 1);

   /* Draw the four arcs */
   for (WorkingX = 0; ; )
	 {
      /* Advance one pixel along the X axis */
      WorkingX++;

      /* Calculate the corresponding point along the Y axis. Guard
         against floating-point roundoff making the intermediate term
         less than 0 */
      Temp = BSquared - (BSquared * WorkingX * WorkingX / ASquared);

			if(Temp >= 0)
				WorkingY= (INT32)(sqrt(Temp)+0.5);
			else
				WorkingY=0;

      /* Stop if X is no longer the major axis (the arc has passed the
         45-degree point) */
      if(((DOUBLE)WorkingY/BSquared) <= ((DOUBLE)WorkingX/ASquared))
         break;

      /* Draw the 4 symmetries of the current point */
			LightCastRay(iLight, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
      LightCastRay(iLight, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
      LightCastRay(iLight, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
      LightCastRay(iLight, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
   }

   /* Draw the four symmetric arcs for which Y advances faster (that is,
      for which Y is the major axis) */
   /* Draw the initial left & right points */
   LightCastRay(iLight, iX, iY, (INT16)(iX+iA), iY, iIntensity, 1);
   LightCastRay(iLight, iX, iY, (INT16)(iX-iA), iY, iIntensity, 1);

   /* Draw the four arcs */
   for (WorkingY = 0; ; )
	 {
      /* Advance one pixel along the Y axis */
      WorkingY++;

      /* Calculate the corresponding point along the X axis. Guard
         against floating-point roundoff making the intermediate term
         less than 0 */
      Temp = ASquared - (ASquared * WorkingY * WorkingY / BSquared);

			if(Temp >= 0)
				WorkingX = (INT32)(sqrt(Temp)+0.5);
			else
				WorkingX = 0;

      /* Stop if Y is no longer the major axis (the arc has passed the
         45-degree point) */
      if (((DOUBLE)WorkingX/ASquared) < ((DOUBLE)WorkingY/BSquared))
         break;

      /* Draw the 4 symmetries of the current point */
      LightCastRay(iLight, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
      LightCastRay(iLight, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
      LightCastRay(iLight, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
      LightCastRay(iLight, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
   }

	return(TRUE);
}


// Creates an square light, taking two radii.
static BOOLEAN LightGenerateSquare(INT32 iLight, UINT8 iIntensity, INT16 iA, INT16 iB)
{
INT16 iX, iY;

	for(iX=0-iA; iX <= 0+iA; iX++)
		LightCastRay(iLight, 0, 0, iX, (INT16)(0-iB), iIntensity, 1);

	for(iX=0-iA; iX <= 0+iA; iX++)
		LightCastRay(iLight, 0, 0, iX, (INT16)(0+iB), iIntensity, 1);

	for(iY=0-iB; iY <= 0+iB; iY++)
		LightCastRay(iLight, 0, 0, (INT16)(0-iA), iY, iIntensity, 1);

	for(iY=0-iB; iY <= 0+iB; iY++)
		LightCastRay(iLight, 0, 0, (INT16)(0+iA), iY, iIntensity, 1);


	/*for(iY=0-iB; iY <= 0+iB; iY++)
		LightCastRay(iLight, 0, iY, (INT16)(0+iA), iY, iIntensity, 1);

	for(iY=0+iB; iY >= 0-iB; iY--)
		LightCastRay(iLight, 0, iY, (INT16)(0-iA), iY, iIntensity, 1);

	for(iX=0-iA; iX <= 0+iA; iX++)
		LightCastRay(iLight, iX, 0, iX, (INT16)(0+iB), iIntensity, 1);

	for(iX=0+iA; iX >= 0-iA; iX--)
		LightCastRay(iLight, iX, 0, iX, (INT16)(0-iB), iIntensity, 1); */

	return(TRUE);
}


/****************************************************************************************
	LightSetBaseLevel

		Sets the current and natural light settings for all tiles in the world.

***************************************************************************************/
BOOLEAN LightSetBaseLevel(UINT8 iIntensity)
{
	INT16 iCountY, iCountX;
	UINT32	cnt;
	SOLDIERTYPE *pSoldier;

	ubAmbientLightLevel=iIntensity;

	if( !gfEditMode )
	{
		// Loop for all good guys in tactical map and add a light if required
		for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
		{
			pSoldier = MercSlots[ cnt ];

			if ( pSoldier != NULL )
			{
				if ( pSoldier->bTeam == gbPlayerNum )
				{
					// Re-create soldier lights
					ReCreateSoldierLight( pSoldier );
				}
			}
		}
	}

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightSetNaturalTile(iCountX, iCountY, iIntensity);

	LightSpriteRenderAll();

	if(iIntensity >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);


	return(TRUE);
}

/****************************************************************************************
	LightAddBaseLevel

		Adds a light value to all the tiles in the world

***************************************************************************************/
BOOLEAN LightAddBaseLevel(UINT32 uiLightType, UINT8 iIntensity)
{
INT16 iCountY, iCountX;

	ubAmbientLightLevel=__max(SHADE_MAX, ubAmbientLightLevel-iIntensity);

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightAddTile(uiLightType, iCountX, iCountY, iCountX, iCountY, iIntensity, LIGHT_IGNORE_WALLS|LIGHT_EVERYTHING, FALSE );

	if(ubAmbientLightLevel >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);

	return(TRUE);
}

/****************************************************************************************
	LightSubtractBaseLevel

		Sets the current and natural light settings for all tiles in the world.

***************************************************************************************/
BOOLEAN LightSubtractBaseLevel(UINT32 uiLightType, UINT8 iIntensity)
{
INT16 iCountY, iCountX;

	ubAmbientLightLevel=__min(SHADE_MIN, ubAmbientLightLevel+iIntensity);

	for(iCountY=0; iCountY < WORLD_ROWS; iCountY++)
		for(iCountX=0; iCountX < WORLD_COLS; iCountX++)
			LightSubtractTile(uiLightType, iCountX, iCountY, iCountX, iCountY, iIntensity, LIGHT_IGNORE_WALLS|LIGHT_EVERYTHING, FALSE );

	if(ubAmbientLightLevel >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);

	return(TRUE);
}

/****************************************************************************************
	LightCreateOmni

		Creates a circular light.

***************************************************************************************/
INT32 LightCreateOmni(UINT8 ubIntensity, INT16 iRadius)
{
INT32 iLight;

	iLight=LightGetFree();
	if(iLight!=(-1))
	{
		LightGenerateElliptical(iLight, ubIntensity, (INT16)(iRadius*DISTANCE_SCALE), (INT16)(iRadius*DISTANCE_SCALE));
	}

	char usName[14];
	sprintf(usName, "LTO%d.LHT", iRadius);
	pLightNames[iLight]=MemAlloc(strlen(usName)+1);
	strcpy(pLightNames[iLight], usName);

	return(iLight);
}


// Creates a square light
static INT32 LightCreateSquare(UINT8 ubIntensity, INT16 iRadius1, INT16 iRadius2)
{
INT32 iLight;

	iLight=LightGetFree();
	if(iLight!=(-1))
	{
		LightGenerateSquare(iLight, ubIntensity, (INT16)(iRadius1*DISTANCE_SCALE), (INT16)(iRadius2*DISTANCE_SCALE));
	}

	char usName[14];
	sprintf(usName, "LTS%d-%d.LHT", iRadius1, iRadius2);
	pLightNames[iLight]=MemAlloc(strlen(usName)+1);
	strcpy(pLightNames[iLight], usName);

	return(iLight);
}


// Creates an elliptical light (two separate radii)
static INT32 LightCreateElliptical(UINT8 ubIntensity, INT16 iRadius1, INT16 iRadius2)
{
INT32 iLight;

	iLight=LightGetFree();
	if(iLight!=(-1))
		LightGenerateElliptical(iLight, ubIntensity, (INT16)(iRadius1*DISTANCE_SCALE), (INT16)(iRadius2*DISTANCE_SCALE));

	char usName[14];
	sprintf(usName, "LTE%d-%d.LHT", iRadius1, iRadius2);
	pLightNames[iLight]=MemAlloc(strlen(usName)+1);
	strcpy(pLightNames[iLight], usName);

	return(iLight);
}


// Renders a light template at the specified X,Y coordinates.
static BOOLEAN LightIlluminateWall(INT16 iSourceX, INT16 iSourceY, INT16 iTileX, INT16 iTileY, LEVELNODE* pStruct)
{
//	return( LightTileHasWall( iSourceX, iSourceY, iTileX, iTileY ) );

#if 0
UINT16 usWallOrientation;

	GetWallOrientation(pStruct->usIndex, &usWallOrientation);

	switch(usWallOrientation)
	{
		case NO_ORIENTATION:
			return(TRUE);

		case INSIDE_TOP_RIGHT:
		case OUTSIDE_TOP_RIGHT:
			return(iSourceX >= iTileX);

		case INSIDE_TOP_LEFT:
		case OUTSIDE_TOP_LEFT:
			return(iSourceY >= iTileY);

	}
	return(FALSE);

#endif

  return( TRUE );
}

/****************************************************************************************
	LightDraw

		Renders a light template at the specified X,Y coordinates.

***************************************************************************************/
BOOLEAN LightDraw(UINT32 uiLightType, INT32 iLight, INT16 iX, INT16 iY, UINT32 uiSprite)
{
LIGHT_NODE *pLight;
UINT16 uiCount;
UINT16 usNodeIndex;
UINT32 uiFlags;
INT32		iOldX, iOldY;
BOOLEAN	fBlocked = FALSE;
BOOLEAN fOnlyWalls;

//MAP_ELEMENT * pMapElement;

	if(pLightList[iLight]==NULL)
		return(FALSE);

	// clear out all the flags
	for(uiCount=0; uiCount < usTemplateSize[iLight]; uiCount++)
	{
		pLight=pLightList[iLight]+uiCount;
		pLight->uiFlags&=(~LIGHT_NODE_DRAWN);
	}

/*
	if (!(LightSprites[uiSprite].uiFlags & MERC_LIGHT))
	{
		uiFlags |= LIGHT_FAKE

		pMapElement = &(gpWorldLevelData[]);
		if (pMapElement->pLand != NULL)
		{
			// only do this for visible locations
			// adjust tile's recorded light level
			pMapElement->sSumRealLight1+=sShadeAdd;

			sSum=pMapElement->pLand->ubNaturalShadeLevel - __max( pMapElement->sSumRealLights[0], pMapElement->pLand->sSumLights[1]);

			sSum=__min(SHADE_MIN, sSum);
			sSum=__max(SHADE_MAX, sSum);

			gpWorldLevelData[ ].ubRealShadeLevel = (UINT8) sSum;

		}

	}
*/

	iOldX = iX;
	iOldY = iY;

	for(uiCount=0; uiCount < usRaySize[iLight]; uiCount++)
	{
		usNodeIndex=*(pLightRayList[iLight]+uiCount);

		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			fBlocked = FALSE;
      fOnlyWalls = FALSE;

			pLight=pLightList[iLight]+(usNodeIndex&(~LIGHT_BACKLIGHT));

			if(!(LightSprites[uiSprite].uiFlags&LIGHT_SPR_ONROOF))
			{
				if(LightTileBlocked( (INT16)iOldX, (INT16)iOldY, (INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY)))
				{
					uiCount=LightFindNextRay(iLight, uiCount);

          fOnlyWalls = TRUE;
					fBlocked = TRUE;
				}
			}

			if(!(pLight->uiFlags&LIGHT_NODE_DRAWN) && (pLight->ubLight) )
			{
				uiFlags=(UINT32)(usNodeIndex&LIGHT_BACKLIGHT);
				if (LightSprites[uiSprite].uiFlags & MERC_LIGHT)
					uiFlags |= LIGHT_FAKE;
				if(LightSprites[uiSprite].uiFlags&LIGHT_SPR_ONROOF)
					uiFlags|=LIGHT_ROOF_ONLY;

				LightAddTile(uiLightType, (INT16)iOldX, (INT16)iOldY, (INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY), pLight->ubLight, uiFlags, fOnlyWalls );

				pLight->uiFlags|=LIGHT_NODE_DRAWN;

			}

			if ( fBlocked )
			{
				iOldX = iX;
				iOldY = iY;
			}
			else
			{
				iOldX = iX+pLight->iDX;
				iOldY = iY+pLight->iDY;
			}

		}
		else
		{
			iOldX = iX;
			iOldY = iY;
		}
	}

	return(TRUE);
}


static BOOLEAN LightRevealWall(INT16 sX, INT16 sY, INT16 sSrcX, INT16 sSrcY)
{
	LEVELNODE *pStruct;
	UINT32 uiTile;
	BOOLEAN fRerender=FALSE, fHitWall=FALSE, fDoRightWalls=TRUE, fDoLeftWalls=TRUE;
	TILE_ELEMENT *TileElem;

	Assert(gpWorldLevelData!=NULL);

	uiTile=MAPROWCOLTOPOS(sY, sX);

	if(sX < sSrcX)
		fDoRightWalls=FALSE;

	if(sY < sSrcY)
		fDoLeftWalls=FALSE;

	// IF A FENCE, RETURN FALSE
	if ( IsFencePresentAtGridno( (INT16)uiTile ) )
	{
		return( FALSE );
	}

	pStruct=gpWorldLevelData[uiTile].pStructHead;
	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				if(!fDoRightWalls)
					fDoLeftWalls=FALSE;
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				if(!fDoLeftWalls)
					fDoRightWalls=FALSE;
				break;
		}
		pStruct=pStruct->pNext;
	}

	pStruct=gpWorldLevelData[uiTile].pStructHead;
	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case NO_ORIENTATION:
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall=TRUE;
				if((fDoRightWalls) && (sX >= sSrcX))
				{
					pStruct->uiFlags|=LEVELNODE_REVEAL;
					fRerender=TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall=TRUE;
				if((fDoLeftWalls) && (sY >= sSrcY))
				{
					pStruct->uiFlags|=LEVELNODE_REVEAL;
					fRerender=TRUE;
				}
				break;
		}
		pStruct=pStruct->pNext;
	}

	if(fRerender)
		SetRenderFlags(RENDER_FLAG_FULL);

	return(fHitWall);
}


static BOOLEAN LightHideWall(INT16 sX, INT16 sY, INT16 sSrcX, INT16 sSrcY)
{
LEVELNODE *pStruct;
UINT32 uiTile;
BOOLEAN fRerender=FALSE, fHitWall=FALSE, fDoRightWalls=TRUE, fDoLeftWalls=TRUE;
TILE_ELEMENT *TileElem;

	Assert(gpWorldLevelData!=NULL);

	uiTile=MAPROWCOLTOPOS(sY, sX);

	if(sX < sSrcX)
		fDoRightWalls=FALSE;

	if(sY < sSrcY)
		fDoLeftWalls=FALSE;

	pStruct=gpWorldLevelData[uiTile].pStructHead;
	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				if(!fDoRightWalls)
					fDoLeftWalls=FALSE;
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				if(!fDoLeftWalls)
					fDoRightWalls=FALSE;
				break;
		}
		pStruct=pStruct->pNext;
	}

	pStruct=gpWorldLevelData[uiTile].pStructHead;
	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case NO_ORIENTATION:
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall=TRUE;
				if((fDoRightWalls) && (sX >= sSrcX))
				{
					pStruct->uiFlags&=(~LEVELNODE_REVEAL);
					fRerender=TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall=TRUE;
				if((fDoLeftWalls) && (sY >= sSrcY))
				{
					pStruct->uiFlags&=(~LEVELNODE_REVEAL);
					fRerender=TRUE;
				}
				break;
		}
		pStruct=pStruct->pNext;
	}

	if(fRerender)
		SetRenderFlags(RENDER_FLAG_FULL);

	return(fHitWall);
}


// Tags walls as being translucent using a light template.
static BOOLEAN CalcTranslucentWalls(INT16 iX, INT16 iY)
{
	LIGHT_NODE *pLight;
	UINT16 uiCount;
	UINT16 usNodeIndex;
	if(pLightList[0]==NULL)
		return(FALSE);
	for(uiCount=0; uiCount < usRaySize[0]; uiCount++)
	{
		usNodeIndex=*(pLightRayList[0]+uiCount);

		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			pLight=pLightList[0]+(usNodeIndex&(~LIGHT_BACKLIGHT));

			//Kris:  added map boundary checking!!!
			if(LightRevealWall(
				 (INT16)min(max((iX+pLight->iDX),0),WORLD_COLS-1),
				 (INT16)min(max((iY+pLight->iDY),0),WORLD_ROWS-1),
				 (INT16)min(max(iX,0),WORLD_COLS-1),
				 (INT16)min(max(iY,0),WORLD_ROWS-1)
				))
			{
				uiCount=LightFindNextRay(0, uiCount);
				SetRenderFlags(RENDER_FLAG_FULL);
			}
		}
	}

	return(TRUE);
}


static BOOLEAN LightGreenTile(INT16 sX, INT16 sY, INT16 sSrcX, INT16 sSrcY)
{
LEVELNODE *pStruct, *pLand;
UINT32 uiTile;
BOOLEAN fRerender=FALSE, fHitWall=FALSE, fThroughWall=FALSE;
TILE_ELEMENT *TileElem;

	Assert(gpWorldLevelData!=NULL);

	uiTile=MAPROWCOLTOPOS(sY, sX);
	pStruct=gpWorldLevelData[uiTile].pStructHead;

	if((sX < sSrcX) || (sY < sSrcY))
		fThroughWall=TRUE;

	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case NO_ORIENTATION:
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall=TRUE;
				if(sX >= sSrcX)
				{
					pStruct->uiFlags|=LEVELNODE_REVEAL;
					fRerender=TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall=TRUE;
				if(sY >= sSrcY)
				{
					pStruct->uiFlags|=LEVELNODE_REVEAL;
					fRerender=TRUE;
				}
				break;
		}
		pStruct=pStruct->pNext;
	}

	//if(fRerender)
	//{
		pLand=gpWorldLevelData[uiTile].pLandHead;
		while(pLand!=NULL)
		{
			pLand->ubShadeLevel=0;
			pLand=pLand->pNext;
		}

		gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;
		SetRenderFlags(RENDER_FLAG_MARKED);
	//}

	return(fHitWall);
}

/****************************************************************************************
	LightShowRays

		Draws a template by making the ground tiles green. Must be polled once for
each tile drawn to facilitate animating the drawing process for debugging.

***************************************************************************************/
BOOLEAN LightShowRays(INT16 iX, INT16 iY, BOOLEAN fReset)
{
LIGHT_NODE *pLight;
static UINT16 uiCount=0;
UINT16 usNodeIndex;

	if(fReset)
		uiCount=0;

	if(pLightList[0]==NULL)
		return(FALSE);

	if(uiCount < usRaySize[0])
	{
		usNodeIndex=*(pLightRayList[0]+uiCount);

		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			pLight=pLightList[0]+(usNodeIndex&(~LIGHT_BACKLIGHT));

			if(LightGreenTile((INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY), iX, iY))
			{
				uiCount=LightFindNextRay(0, uiCount);
				SetRenderFlags(RENDER_FLAG_MARKED);
			}
		}

		uiCount++;
		return(TRUE);
	}
	else
		return(FALSE);
}


// Removes the green from the tiles that was drawn to show the path of the rays.
static BOOLEAN LightHideGreen(INT16 sX, INT16 sY, INT16 sSrcX, INT16 sSrcY)
{
LEVELNODE *pStruct, *pLand;
UINT32 uiTile;
BOOLEAN fRerender=FALSE, fHitWall=FALSE;
TILE_ELEMENT *TileElem;

	Assert(gpWorldLevelData!=NULL);

	uiTile=MAPROWCOLTOPOS(sY, sX);
	pStruct=gpWorldLevelData[uiTile].pStructHead;

	while(pStruct!=NULL)
	{
		TileElem = &(gTileDatabase[pStruct->usIndex]);
		switch(TileElem->usWallOrientation)
		{
			case NO_ORIENTATION:
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:
				fHitWall=TRUE;
				if(sX >= sSrcX)
				{
					pStruct->uiFlags&=(~LEVELNODE_REVEAL);
					fRerender=TRUE;
				}
				break;

			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:
				fHitWall=TRUE;
				if(sY >= sSrcY)
				{
					pStruct->uiFlags&=(~LEVELNODE_REVEAL);
					fRerender=TRUE;
				}
				break;
		}
		pStruct=pStruct->pNext;
	}

	//if(fRerender)
	//{
		pLand=gpWorldLevelData[uiTile].pLandHead;
		while(pLand!=NULL)
		{
			pLand->ubShadeLevel=pLand->ubNaturalShadeLevel;
			pLand=pLand->pNext;
		}

		gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;
		SetRenderFlags(RENDER_FLAG_MARKED);
	//}

	return(fHitWall);
}

/****************************************************************************************
	LightHideRays

		Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN LightHideRays(INT16 iX, INT16 iY)
{
LIGHT_NODE *pLight;
UINT16 uiCount;
UINT16 usNodeIndex;

	if(pLightList[0]==NULL)
		return(FALSE);

	for(uiCount=0; uiCount < usRaySize[0]; uiCount++)
	{
		usNodeIndex=*(pLightRayList[0]+uiCount);

		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			pLight=pLightList[0]+(usNodeIndex&(~LIGHT_BACKLIGHT));

			if(LightHideWall((INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY), iX, iY))
			{
				uiCount=LightFindNextRay(0, uiCount);
				SetRenderFlags(RENDER_FLAG_MARKED);
			}
		}
	}

	return(TRUE);
}

/****************************************************************************************
	ApplyTranslucencyToWalls

		Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN ApplyTranslucencyToWalls(INT16 iX, INT16 iY)
{
LIGHT_NODE *pLight;
UINT16 uiCount;
UINT16 usNodeIndex;

	if(pLightList[0]==NULL)
		return(FALSE);

	for(uiCount=0; uiCount < usRaySize[0]; uiCount++)
	{
		usNodeIndex=*(pLightRayList[0]+uiCount);

		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			pLight=pLightList[0]+(usNodeIndex&(~LIGHT_BACKLIGHT));
			//Kris:  added map boundary checking!!!
			if(LightHideWall(
				 (INT16)min(max((iX+pLight->iDX),0),WORLD_COLS-1),
				 (INT16)min(max((iY+pLight->iDY),0),WORLD_ROWS-1),
				 (INT16)min(max(iX,0),WORLD_COLS-1),
				 (INT16)min(max(iY,0),WORLD_ROWS-1)
				))
			{
				uiCount=LightFindNextRay(0, uiCount);
				SetRenderFlags(RENDER_FLAG_FULL);
			}
		}
	}

	return(TRUE);
}


// Makes all the near-side trees around a given coordinate translucent.
static BOOLEAN LightTranslucentTrees(INT16 iX, INT16 iY)
{
INT32 iCountX, iCountY;
UINT32 uiTile;
LEVELNODE *pNode;
BOOLEAN fRerender=FALSE;
UINT32	fTileFlags;

	for(iCountY=iY; iCountY < (INT16)(iY+LIGHT_TREE_REVEAL); iCountY++)
		for(iCountX=iX; iCountX < (INT16)(iX+LIGHT_TREE_REVEAL); iCountX++)
		{
			uiTile=MAPROWCOLTOPOS(iCountY, iCountX);
			pNode=gpWorldLevelData[uiTile].pStructHead;
			while(pNode!=NULL)
			{
				GetTileFlags( pNode->usIndex, &fTileFlags );

				if ( fTileFlags & FULL3D_TILE )
				{
					if ( !( pNode->uiFlags & LEVELNODE_REVEALTREES ) )
					{
						//pNode->uiFlags |= ( LEVELNODE_REVEALTREES | LEVELNODE_ERASEZ );
						pNode->uiFlags |= ( LEVELNODE_REVEALTREES  );
						gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;
					}

					fRerender=TRUE;
				}
				pNode=pNode->pNext;
			}
		}

		if(fRerender)
		{
			//SetRenderFlags(RENDER_FLAG_MARKED);
			SetRenderFlags(RENDER_FLAG_FULL);
			return(TRUE);
		}
		else
			return(FALSE);
}


// Removes the translucency from any trees in the area.
static BOOLEAN LightHideTrees(INT16 iX, INT16 iY)
{
INT32 iCountX, iCountY;
UINT32 uiTile;
LEVELNODE *pNode;
BOOLEAN fRerender=FALSE;
UINT32	fTileFlags;

	//Kris:  added map boundary checking!!!
	for(iCountY=(INT16)__max(iY-LIGHT_TREE_REVEAL,0); iCountY < (INT16)__min(iY+LIGHT_TREE_REVEAL,WORLD_ROWS-1); iCountY++)
		for(iCountX=(INT16)__max(iX-LIGHT_TREE_REVEAL,0); iCountX < (INT16)__min(iX+LIGHT_TREE_REVEAL,WORLD_COLS-1); iCountX++)
		{
			uiTile=MAPROWCOLTOPOS(iCountY, iCountX);
			pNode=gpWorldLevelData[uiTile].pStructHead;
			while(pNode!=NULL)
			{
				GetTileFlags( pNode->usIndex, &fTileFlags );

				if ( fTileFlags & FULL3D_TILE )
				{

					if ( ( pNode->uiFlags & LEVELNODE_REVEALTREES ) )
					{
						//pNode->uiFlags  &=(~( LEVELNODE_REVEALTREES | LEVELNODE_ERASEZ ) );
						pNode->uiFlags  &=(~( LEVELNODE_REVEALTREES ) );
						gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REDRAW;
					}

					fRerender=TRUE;
				}
				pNode=pNode->pNext;
			}
		}

		if(fRerender)
		{
			//SetRenderFlags(RENDER_FLAG_MARKED);
			SetRenderFlags(RENDER_FLAG_FULL );
			return(TRUE);
		}
		else
			return(FALSE);
}


// Reverts all tiles a given light affects to their natural light levels.
static BOOLEAN LightErase(UINT32 uiLightType, INT32 iLight, INT16 iX, INT16 iY, UINT32 uiSprite)
{
LIGHT_NODE *pLight;
UINT16 uiCount;
UINT16 usNodeIndex;
UINT32 uiFlags;
INT32		iOldX, iOldY;
BOOLEAN	fBlocked = FALSE;
BOOLEAN fOnlyWalls;


	if(pLightList[iLight]==NULL)
		return(FALSE);

	// clear out all the flags
	for(uiCount=0; uiCount < usTemplateSize[iLight]; uiCount++)
	{
		pLight=pLightList[iLight]+uiCount;
		pLight->uiFlags&=(~LIGHT_NODE_DRAWN);
	}

	iOldX = iX;
	iOldY = iY;

	for(uiCount=0; uiCount < usRaySize[iLight]; uiCount++)
	{
		usNodeIndex=*(pLightRayList[iLight]+uiCount);
		if(!(usNodeIndex&LIGHT_NEW_RAY))
		{
			fBlocked = FALSE;
      fOnlyWalls = FALSE;

			pLight=pLightList[iLight]+(usNodeIndex&(~LIGHT_BACKLIGHT));

			if(!(LightSprites[uiSprite].uiFlags&LIGHT_SPR_ONROOF))
			{
				if(LightTileBlocked( (INT16)iOldX, (INT16)iOldY, (INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY)))
				{
					uiCount=LightFindNextRay(iLight, uiCount);

          fOnlyWalls = TRUE;
					fBlocked = TRUE;
				}
			}

			if(!(pLight->uiFlags&LIGHT_NODE_DRAWN) && (pLight->ubLight) )
			{
				uiFlags=(UINT32)(usNodeIndex&LIGHT_BACKLIGHT);
				if (LightSprites[uiSprite].uiFlags & MERC_LIGHT)
					uiFlags |= LIGHT_FAKE;
				if(LightSprites[uiSprite].uiFlags&LIGHT_SPR_ONROOF)
					uiFlags|=LIGHT_ROOF_ONLY;

				LightSubtractTile(uiLightType, (INT16)iOldX, (INT16)iOldY, (INT16)(iX+pLight->iDX), (INT16)(iY+pLight->iDY), pLight->ubLight, uiFlags, fOnlyWalls );
				pLight->uiFlags|=LIGHT_NODE_DRAWN;
			}

			if ( fBlocked )
			{
				iOldX = iX;
				iOldY = iY;
			}
			else
			{
				iOldX = iX+pLight->iDX;
				iOldY = iY+pLight->iDY;
			}
		}
		else
		{
			iOldX = iX;
			iOldY = iY;
		}
	}

	return(TRUE);
}


/* Calculates the rect size of a given light, used in dirtying the screen after
 * updating a light. */
static BOOLEAN LightCalcRect(INT32 iLight)
{
SGPRect MaxRect;
INT16 sXValue, sYValue, sDummy;
UINT32 uiCount;
LIGHT_NODE *pLight;

	if(pLightList[iLight]==NULL)
		return(FALSE);

	pLight=pLightList[iLight];

	MaxRect.iLeft=99999;
	MaxRect.iRight=-99999;
	MaxRect.iTop=99999;
	MaxRect.iBottom=-99999;

	for(uiCount=0; uiCount < usTemplateSize[iLight]; uiCount++)
	{
		pLight=pLightList[iLight]+uiCount;
		if(pLight->ubLight)
		{
			MaxRect.iLeft=__min(MaxRect.iLeft, pLight->iDX);
			MaxRect.iRight=__max(MaxRect.iRight, pLight->iDX);
			MaxRect.iTop=__min(MaxRect.iTop, pLight->iDY);
			MaxRect.iBottom=__max(MaxRect.iBottom, pLight->iDY);
		}
	}

	FromCellToScreenCoordinates((INT16)(MaxRect.iLeft*CELL_X_SIZE),
																	(INT16)(MaxRect.iTop*CELL_Y_SIZE),
																	&sDummy, &sYValue);

	LightMapLeft[iLight]=(INT16)MaxRect.iLeft;
	LightMapTop[iLight]=(INT16)MaxRect.iTop;
	LightMapRight[iLight]=(INT16)MaxRect.iRight;
	LightMapBottom[iLight]=(INT16)MaxRect.iBottom;

	LightHeight[iLight]=-sYValue;
	LightYOffset[iLight]=sYValue;

	FromCellToScreenCoordinates((INT16)(MaxRect.iRight*CELL_X_SIZE),
																	(INT16)(MaxRect.iBottom*CELL_Y_SIZE),
																	&sDummy, &sYValue);
	LightHeight[iLight]+=sYValue;

	FromCellToScreenCoordinates((INT16)(MaxRect.iLeft*CELL_X_SIZE),
																	(INT16)(MaxRect.iBottom*CELL_Y_SIZE),
																	&sXValue, &sDummy);
	LightWidth[iLight]=-sXValue;
	LightXOffset[iLight]=sXValue;

	FromCellToScreenCoordinates((INT16)(MaxRect.iRight*CELL_X_SIZE),
																	(INT16)(MaxRect.iTop*CELL_Y_SIZE),
																	&sXValue, &sDummy);
	LightWidth[iLight]+=sXValue;

	LightHeight[iLight]+=WORLD_TILE_X*2;
	LightWidth[iLight]+=WORLD_TILE_Y*3;
	LightXOffset[iLight]-=WORLD_TILE_X*2;
	LightYOffset[iLight]-=WORLD_TILE_Y*2;

	return(TRUE);
}


/****************************************************************************************
	LightSave

		Saves the light list of a given template to a file. Passing in NULL for the
	filename forces the system to save the light with the internal filename (recommended).

***************************************************************************************/
BOOLEAN LightSave(INT32 iLight, STR pFilename)
{
	HWFILE hFile;
	STR pName;

	if(pLightList[iLight]==NULL)
		return(FALSE);
	else
	{
		if(pFilename==NULL)
			pName=pLightNames[iLight];
		else
			pName=pFilename;

		hFile = FileOpen(pName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
		if (hFile != 0)
		{
			FileWrite(hFile, &usTemplateSize[iLight], sizeof(UINT16));
			FileWrite(hFile, pLightList[iLight],      sizeof(LIGHT_NODE) * usTemplateSize[iLight]);
			FileWrite(hFile, &usRaySize[iLight],      sizeof(UINT16));
			FileWrite(hFile, pLightRayList[iLight],  sizeof(UINT16) * usRaySize[iLight]);

			FileClose(hFile);
		}
		else
			return(FALSE);
	}
	return(TRUE);
}


/* Loads a light template from disk. The light template number is returned, or
 * (-1) if the file wasn't loaded. */
static INT32 LightLoad(const char* pFilename)
{
HWFILE hFile;
INT32 iLight;

	if((iLight=LightGetFree())==(-1))
		return(-1);
	else
	{
		hFile = FileOpen(pFilename, FILE_ACCESS_READ);
		if (hFile != 0)
		{
			FileRead(hFile, &usTemplateSize[iLight], sizeof(UINT16));
			if((pLightList[iLight]=MemAlloc(usTemplateSize[iLight]*sizeof(LIGHT_NODE)))==NULL)
			{
				usTemplateSize[iLight]=0;
				return(-1);
			}
			FileRead(hFile, pLightList[iLight], sizeof(LIGHT_NODE) * usTemplateSize[iLight]);

			FileRead(hFile, &usRaySize[iLight], sizeof(UINT16));
			if((pLightRayList[iLight]=MemAlloc(usRaySize[iLight]*sizeof(UINT16)))==NULL)
			{
				usTemplateSize[iLight]=0;
				usRaySize[iLight]=0;
				MemFree(pLightList[iLight]);
				return(-1);
			}
			FileRead(hFile, pLightRayList[iLight], sizeof(UINT16) * usRaySize[iLight]);

			FileClose(hFile);

			pLightNames[iLight]=MemAlloc(strlen(pFilename)+1);
			strcpy(pLightNames[iLight], pFilename);
		}
		else
			return(-1);
	}
	LightCalcRect(iLight);
	return(iLight);
}


/* Figures out whether a light template is already in memory, or needs to be
 * loaded from disk. Returns the index of the template, or (-1) if it couldn't
 * be loaded. */
static INT32 LightLoadCachedTemplate(const char* pFilename)
{
INT32 iCount;

	for(iCount=0; iCount < MAX_LIGHT_TEMPLATES; iCount++)
	{
		if (pLightNames[iCount] != NULL && strcasecmp(pFilename, pLightNames[iCount]) == 0)
			return(iCount);
	}

	return(LightLoad(pFilename));
}

UINT8 LightGetColors(SGPPaletteEntry *pPal)
{
	if(pPal!=NULL)
		memcpy(pPal, &gpOrigLights[0], sizeof(SGPPaletteEntry)*gubNumLightColors);

	return(gubNumLightColors);
}

/****************************************************************************************
	LightSetColors

	Sets the number of light colors, and the RGB value for each.

***************************************************************************************/
#ifdef JA2EDITOR
BOOLEAN gfEditorForceRebuildAllColors = FALSE;
#endif

extern void SetAllNewTileSurfacesLoaded( BOOLEAN fNew );

BOOLEAN LightSetColors(SGPPaletteEntry *pPal, UINT8 ubNumColors)
{
	INT16 sRed, sGreen, sBlue;

	Assert( ubNumColors >=1 && ubNumColors <=2 );
	Assert( pPal );

	if( pPal[0].peRed != gpLightColors[0].peRed ||
			pPal[0].peGreen != gpLightColors[0].peGreen ||
			pPal[0].peBlue != gpLightColors[0].peBlue )
	{	//Set the entire tileset database so that it reloads everything.  It has to because the
		//colors have changed.
		SetAllNewTileSurfacesLoaded( TRUE );
	}

	// before doing anything, get rid of all the old palettes
	DestroyTileShadeTables( );

	// we will have at least one light color
	memcpy(&gpLightColors[0], &pPal[0], sizeof(SGPPaletteEntry));
	memcpy(&gpOrigLights[0], &pPal[0], sizeof(SGPPaletteEntry)*2);

	gubNumLightColors=ubNumColors;

	// if there are two colors, calculate a third palette that is a mix of the two
	if(ubNumColors==2)
	{
		sRed=__min((((INT16)pPal[0].peRed)*LVL1_L1_PER/100 + ((INT16)pPal[1].peRed)*LVL1_L2_PER/100), 255);
		sGreen=__min((((INT16)pPal[0].peGreen)*LVL1_L1_PER/100 + ((INT16)pPal[1].peGreen)*LVL1_L2_PER/100), 255);
		sBlue=__min((((INT16)pPal[0].peBlue)*LVL1_L1_PER/100 + ((INT16)pPal[1].peBlue)*LVL1_L2_PER/100), 255);

		gpLightColors[1].peRed=(UINT8)(sRed);
		gpLightColors[1].peGreen=(UINT8)(sGreen);
		gpLightColors[1].peBlue=(UINT8)(sBlue);

		sRed=__min((((INT16)pPal[0].peRed)*LVL2_L1_PER/100 + ((INT16)pPal[1].peRed)*LVL2_L2_PER/100), 255);
		sGreen=__min((((INT16)pPal[0].peGreen)*LVL2_L1_PER/100 + ((INT16)pPal[1].peGreen)*LVL2_L2_PER/100), 255);
		sBlue=__min((((INT16)pPal[0].peBlue)*LVL2_L1_PER/100 + ((INT16)pPal[1].peBlue)*LVL2_L2_PER/100), 255);

		gpLightColors[2].peRed=(UINT8)(sRed);
		gpLightColors[2].peGreen=(UINT8)(sGreen);
		gpLightColors[2].peBlue=(UINT8)(sBlue);

	}

	BuildTileShadeTables( );

	// Build all palettes for all soldiers in the world
	// ( THIS FUNCTION WILL ERASE THEM IF THEY EXIST )
	RebuildAllSoldierShadeTables( );

	SetRenderFlags(RENDER_FLAG_FULL);

	return(TRUE);
}

//---------------------------------------------------------------------------------------
// Light Manipulation Layer
//---------------------------------------------------------------------------------------


// Returns the index of the next available sprite.
static INT32 LightSpriteGetFree(void)
{
INT32 iCount;

	for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
	{
		if(!(LightSprites[iCount].uiFlags&LIGHT_SPR_ACTIVE))
			return(iCount);
	}

	return(-1);
}

/********************************************************************************
* LightSpriteCreate
*
*		Creates an instance of a light. The template is loaded if it isn't already.
* If this function fails (out of sprites, or bad template name) it returns (-1).
*
********************************************************************************/
INT32 LightSpriteCreate(const char *pName, UINT32 uiLightType)
{
INT32 iSprite;

	if((iSprite=LightSpriteGetFree())!=(-1))
	{
		memset(&LightSprites[iSprite], 0, sizeof(LIGHT_SPRITE));
		LightSprites[iSprite].iX=WORLD_COLS+1;
		LightSprites[iSprite].iY=WORLD_ROWS+1;
		LightSprites[iSprite].iOldX=WORLD_COLS+1;
		LightSprites[iSprite].iOldY=WORLD_ROWS+1;
		LightSprites[iSprite].uiLightType=uiLightType;

		if((LightSprites[iSprite].iTemplate=LightLoadCachedTemplate(pName))==(-1))
			return(-1);

		LightSprites[iSprite].uiFlags|=LIGHT_SPR_ACTIVE;
	}

	return(iSprite);
}


/********************************************************************************
* LightSpriteFake
*
*		Sets the flag of a light sprite to "fake" (in game for merc navig purposes)
*
********************************************************************************/
BOOLEAN LightSpriteFake(INT32 iSprite)
{
	if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ACTIVE)
	{
		LightSprites[iSprite].uiFlags|=MERC_LIGHT;
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static BOOLEAN LightSpriteDirty(INT32 iSprite);


/********************************************************************************
* LightSpriteDestroy
*
*		Removes an instance of a light. If it was on, it is erased from the scene.
*
********************************************************************************/
BOOLEAN LightSpriteDestroy(INT32 iSprite)
{
	if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ACTIVE)
	{
		if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ERASE)
		{
			if((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS))
			{
				LightErase(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate, LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
				LightSpriteDirty(iSprite);
			}
			LightSprites[iSprite].uiFlags&=(~LIGHT_SPR_ERASE);
		}

		LightSprites[iSprite].uiFlags&=(~LIGHT_SPR_ACTIVE);
		return(TRUE);
	}

	return(FALSE);
}


// Updates any change in position in lights
static BOOLEAN LightSpriteRender(void)
{
//INT32 iCount;
//BOOLEAN fRenderLights=FALSE;

	return(FALSE);

/*	for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
	{
		if(LightSprites[iCount].uiFlags&LIGHT_SPR_ACTIVE)
		{
			if((LightSprites[iCount].iX!=LightSprites[iCount].iOldX)
				|| (LightSprites[iCount].iY!=LightSprites[iCount].iOldY)
				|| (LightSprites[iCount].uiFlags&LIGHT_SPR_REDRAW))
			{
				if(LightSprites[iCount].iOldX < WORLD_COLS)
				{
					fRenderLights=TRUE;
					LightSpriteDirty(iCount);
				}

				LightSprites[iCount].iOldX=LightSprites[iCount].iX;
				LightSprites[iCount].iOldY=LightSprites[iCount].iY;

				if(LightSprites[iCount].uiFlags&LIGHT_SPR_ON)
				{
						LightSpriteDirty(iCount);
						fRenderLights=TRUE;
				}

				LightSprites[iCount].uiFlags&=(~LIGHT_SPR_REDRAW);
			}
		}
	}

	if(fRenderLights)
	{
		LightResetAllTiles();
		for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
			if((LightSprites[iCount].uiFlags&LIGHT_SPR_ACTIVE) && (LightSprites[iCount].uiFlags&LIGHT_SPR_ON))
				LightDraw(LightSprites[iCount].iTemplate, LightSprites[iCount].iX, LightSprites[iCount].iY, iCount);
		return(TRUE);
	}

	return(FALSE); */
}

/********************************************************************************
* LightSpriteRenderAll
*
*		Resets all tiles in the world to the ambient light level, and redraws all
* active lights.
*
********************************************************************************/
BOOLEAN LightSpriteRenderAll(void)
{
INT32 iCount;

	LightResetAllTiles();
	for(iCount=0; iCount < MAX_LIGHT_SPRITES; iCount++)
	{
		LightSprites[iCount].uiFlags&=(~LIGHT_SPR_ERASE);

		if((LightSprites[iCount].uiFlags&LIGHT_SPR_ACTIVE) && (LightSprites[iCount].uiFlags&LIGHT_SPR_ON))
		{
			LightDraw(LightSprites[iCount].uiLightType, LightSprites[iCount].iTemplate, LightSprites[iCount].iX, LightSprites[iCount].iY, iCount);
			LightSprites[iCount].uiFlags|=LIGHT_SPR_ERASE;
			LightSpriteDirty(iCount);
		}

		LightSprites[iCount].iOldX=LightSprites[iCount].iX;
		LightSprites[iCount].iOldY=LightSprites[iCount].iY;
	}

	return(TRUE);
}

/********************************************************************************
* LightSpritePosition
*
*		Sets the X,Y position (IN TILES) of a light instance.
*
********************************************************************************/
BOOLEAN LightSpritePosition(INT32 iSprite, INT16 iX, INT16 iY)
{
	if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ACTIVE)
	{
		if((LightSprites[iSprite].iX==iX) && (LightSprites[iSprite].iY==iY))
			return(TRUE);

		if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ERASE)
		{
			if((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS))
			{
				LightErase(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate, LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
				LightSpriteDirty(iSprite);
			}
		}

		//LightSprites[iSprite].iOldX=LightSprites[iSprite].iX;
		//LightSprites[iSprite].iOldY=LightSprites[iSprite].iY;

		LightSprites[iSprite].iX=iX;
		LightSprites[iSprite].iY=iY;

		if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ON)
		{
			if((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS))
			{
				LightDraw(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate, iX, iY, iSprite);
				LightSprites[iSprite].uiFlags|=LIGHT_SPR_ERASE;
				LightSpriteDirty(iSprite);
			}
		}
	}
	else
		return(FALSE);

	return(TRUE);
}

/********************************************************************************
* LightSpriteRoofStatus
*
*		Determines whether a light is on a roof or not.
*
********************************************************************************/
BOOLEAN LightSpriteRoofStatus(INT32 iSprite, BOOLEAN fOnRoof)
{
	if(fOnRoof && (LightSprites[iSprite].uiFlags&LIGHT_SPR_ONROOF))
		return(FALSE);

	if(!fOnRoof && !(LightSprites[iSprite].uiFlags&LIGHT_SPR_ONROOF))
		return(FALSE);

	if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ACTIVE)
	{
		if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ERASE)
		{
			if((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS))
			{
				LightErase(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate, LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
				LightSpriteDirty(iSprite);
			}
		}

		if(fOnRoof)
			LightSprites[iSprite].uiFlags|=LIGHT_SPR_ONROOF;
		else
			LightSprites[iSprite].uiFlags&=(~LIGHT_SPR_ONROOF);


		if(LightSprites[iSprite].uiFlags&LIGHT_SPR_ON)
		{
			if((LightSprites[iSprite].iX < WORLD_COLS) && (LightSprites[iSprite].iY < WORLD_ROWS))
			{
				LightDraw(LightSprites[iSprite].uiLightType, LightSprites[iSprite].iTemplate, LightSprites[iSprite].iX, LightSprites[iSprite].iY, iSprite);
				LightSprites[iSprite].uiFlags|=LIGHT_SPR_ERASE;
				LightSpriteDirty(iSprite);
			}
		}
	}
	else
		return(FALSE);

	return(TRUE);
}

/********************************************************************************
* LightSpritePower
*
*		Turns on or off a light, based on the BOOLEAN.
*
********************************************************************************/
BOOLEAN LightSpritePower(INT32 iSprite, BOOLEAN fOn)
{
	if(fOn)
	{
		LightSprites[iSprite].uiFlags|=(LIGHT_SPR_ON|LIGHT_SPR_REDRAW);
		LightSprites[iSprite].iOldX=WORLD_COLS;
	}
	else
		LightSprites[iSprite].uiFlags&=(~LIGHT_SPR_ON);

	return(TRUE);

}


// Sets the flag for the renderer to draw all marked tiles.
static BOOLEAN LightSpriteDirty(INT32 iSprite)
{
//INT16 iLeft_s, iTop_s;
//INT16 iMapLeft, iMapTop, iMapRight, iMapBottom;

	//CellXYToScreenXY((INT16)(LightSprites[iSprite].iX*CELL_X_SIZE),
	//								(INT16)(LightSprites[iSprite].iY*CELL_Y_SIZE),  &iLeft_s, &iTop_s);

	//iLeft_s+=LightXOffset[LightSprites[iSprite].iTemplate];
	//iTop_s+=LightYOffset[LightSprites[iSprite].iTemplate];

	//iMapLeft=LightSprites[iSprite].iX+LightMapLeft[LightSprites[iSprite].iTemplate];
	//iMapTop=LightSprites[iSprite].iY+LightMapTop[LightSprites[iSprite].iTemplate];
	//iMapRight=LightSprites[iSprite].iX+LightMapRight[LightSprites[iSprite].iTemplate];
	//iMapBottom=LightSprites[iSprite].iY+LightMapBottom[LightSprites[iSprite].iTemplate];

	//UpdateSaveBuffer();
	//AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y );
	//AddBaseDirtyRect(iLeft_s, iTop_s,
	//								(INT16)(iLeft_s+LightWidth[LightSprites[iSprite].iTemplate]),
	//								(INT16)(iTop_s+LightHeight[LightSprites[iSprite].iTemplate]));

	SetRenderFlags(RENDER_FLAG_MARKED);

	return(TRUE);
}


void CreateShadedPalettes(UINT16* Shades[16], const SGPPaletteEntry ShadePal[256])
{
	UINT16* sl0 = gusShadeLevels[0];
	Shades[0] = Create16BPPPaletteShaded(ShadePal, sl0[0], sl0[1], sl0[2], TRUE);
	for (UINT i = 1; i < 16; i++)
	{
		const UINT16* sl = gusShadeLevels[i];
		Shades[i] = Create16BPPPaletteShaded(ShadePal, sl[0], sl[1], sl[2], FALSE);
	}
}


static BOOLEAN CreateObjectPalette(HVOBJECT pObj, UINT32 uiBase, const SGPPaletteEntry* pShadePal)
{
	CreateShadedPalettes(pObj->pShades + uiBase, pShadePal);
	return(TRUE);
}


static BOOLEAN CreateSoldierShadedPalette(SOLDIERTYPE* pSoldier, UINT32 uiBase, const SGPPaletteEntry* pShadePal)
{
	CreateShadedPalettes(pSoldier->pShades + uiBase, pShadePal);
	return(TRUE);
}


void AddSaturatePalette(SGPPaletteEntry Dst[256], const SGPPaletteEntry Src[256], const SGPPaletteEntry* Bias)
{
	UINT8 r = Bias->peRed;
	UINT8 g = Bias->peGreen;
	UINT8 b = Bias->peBlue;
	for (UINT i = 0; i < 256; i++)
	{
		Dst[i].peRed   = __min(Src[i].peRed   + r, 255);
		Dst[i].peGreen = __min(Src[i].peGreen + g, 255);
		Dst[i].peBlue  = __min(Src[i].peBlue  + b, 255);
	}
}


/**********************************************************************************************
 CreateObjectPaletteTables

		Creates the shading tables for 8-bit brushes. One highlight table is created, based on
	the object-type, 3 brightening tables, 1 normal, and 11 darkening tables. The entries are
	created iteratively, rather than in a loop to allow hand-tweaking of the values. If you
	change the HVOBJECT_SHADE_TABLES symbol, remember to add/delete entries here, it won't
	adjust automagically.

**********************************************************************************************/
#ifdef JA2TESTVERSION
	extern UINT32 uiNumTablesSaved;
#endif

UINT16 CreateTilePaletteTables(HVOBJECT pObj, UINT32 uiTileIndex, BOOLEAN fForce )
{
		SGPPaletteEntry LightPal[256];
		BOOLEAN fLoaded = FALSE;

		Assert(pObj!=NULL);

		// create the basic shade table
		if( !gfForceBuildShadeTables && !fForce )
		{ //The overwhelming majority of maps use the neutral 0,0,0 light for outdoors.  These shadetables
			//are extremely time consuming to generate, so we will attempt to load them.  If we do, then
			//we skip the generation process altogether.
			if( LoadShadeTable( pObj, uiTileIndex ) )
			{
				fLoaded = TRUE;
			}
		}
		if( !fLoaded )
		{ //This is expensive as hell to call!
			AddSaturatePalette(LightPal, pObj->pPaletteEntry, &gpLightColors[0]);
			// build the shade tables
			CreateObjectPalette(pObj, 0, LightPal);

			//We paid to generate the shade table, so now save it, so we don't have to regenerate it ever
			//again!
			if( !gfForceBuildShadeTables && !gpLightColors[0].peRed && !gpLightColors[0].peGreen && !gpLightColors[0].peBlue )
			{
				SaveShadeTable( pObj, uiTileIndex );
			}
			#ifdef JA2TESTVERSION
			else
				uiNumTablesSaved++;
			#endif
		}

		// if two lights are active
		if(gubNumLightColors==2)
		{
			// build the second light's palette and table
			AddSaturatePalette(LightPal, pObj->pPaletteEntry, &gpLightColors[1]);
			CreateObjectPalette(pObj, 16, LightPal);

			// build a table that is a mix of the first two
			AddSaturatePalette(LightPal, pObj->pPaletteEntry, &gpLightColors[2]);
			CreateObjectPalette(pObj, 32, LightPal);
		}

		// build neutral palette as well!
		// Set current shade table to neutral color
		pObj->pShadeCurrent=pObj->pShades[4];

		return(TRUE);
}


UINT16 CreateSoldierPaletteTables(SOLDIERTYPE *pSoldier)
{
	SGPPaletteEntry LightPal[256];

	// create the basic shade table
	AddSaturatePalette(LightPal, pSoldier->p8BPPPalette, &gpLightColors[0]);
	// build the shade tables
	CreateSoldierShadedPalette(pSoldier, 0, LightPal);

	// if two lights are active
	if(gubNumLightColors==2)
	{
		// build the second light's palette and table
		AddSaturatePalette(LightPal, pSoldier->p8BPPPalette, &gpLightColors[1]);
		CreateSoldierShadedPalette(pSoldier, 16, LightPal);

		// build a table that is a mix of the first two
		AddSaturatePalette(LightPal, pSoldier->p8BPPPalette, &gpLightColors[2]);
		CreateSoldierShadedPalette(pSoldier, 32, LightPal);
	}

	return(TRUE);

}
