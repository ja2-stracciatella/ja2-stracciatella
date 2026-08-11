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
#include "HImage.h"
#include "Overhead.h"
#include "math.h"
#include "Structure.h"
#include "VObject.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "Debug.h"
#include "Isometric_Utils.h"
#include "Sys_Globals.h"
#include "TileDef.h"
#include "Lighting.h"
#include "FileMan.h"
#include "Environment.h"
#include "PathAI.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <string_theory/iostream>
#include <string_theory/string>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#define MAX_LIGHT_TEMPLATES 32 // maximum number of light types

#define MIN_BOX_COLUMNS 8 // minimum number of columns to consider a structure light-blocking

// lightlist node flags
#define LIGHT_NODE_DRAWN_FULLY	 0x00000001 // light node duplicate marker
#define LIGHT_NODE_DRAWN_NORTH	 0x00000002 // light was cast only from one side
#define LIGHT_NODE_DRAWN_WEST	 0x00000004 // light was cast only from one side
#define LIGHT_ROOF_ONLY			 0x00001000 // light only rooftops
#define LIGHT_IGNORE_WALLS		 0x00002000 // doesn't take walls into account
#define LIGHT_BACKLIGHT			 0x00004000 // light does not light objs, trees
#define LIGHT_NEW_RAY			 0x00008000 // start of new ray in linked list
#define LIGHT_EVERYTHING		 0x00010000 // light up everything
#define LIGHT_FAKE				 0x10000000 // "fake" light for display only


// stucture of node in linked list for lights
struct LIGHT_NODE
{
	INT16 iDX;
	INT16 iDY;
	UINT8 uiFlags;
	UINT8 ubLight;
};


struct LightTemplate
{
	std::vector<LIGHT_NODE> lights;
	std::vector<UINT16> rays;
	ST::string name;
};

static LightTemplate g_light_templates[MAX_LIGHT_TEMPLATES];

#define FOR_EACH_LIGHT_TEMPLATE_SLOT(iter) \
	FOR_EACH(LightTemplate, iter, g_light_templates)

#define FOR_EACH_LIGHT_TEMPLATE(iter) \
	FOR_EACH_LIGHT_TEMPLATE_SLOT(iter) \
		if (iter->lights.empty()) continue; else


// Sprite data
LIGHT_SPRITE	LightSprites[MAX_LIGHT_SPRITES];

// Lighting system general data
UINT8 ubAmbientLightLevel = DEFAULT_SHADE_LEVEL;

SGPPaletteEntry g_light_color = { 0, 0, 0, 0 };

static SGPPaletteEntry gpOrigLight = { 0, 0, 0, 0 };


/*
UINT16 gusShadeLevels[16][3]={
	{500, 500, 500}, // green table
	{450, 450, 450}, // bright
	{350, 350, 350},
	{300, 300, 300},
	{255, 255, 255}, // normal
	{227, 227, 227},
	{198, 198, 198},
	{171, 171, 171},
	{143, 143, 143},
	{115, 115, 160}, // darkening
	{87, 87, 176},
	{60, 60, 160},
	{48, 48, 192},
	{36, 36, 208},
	{18, 18, 224},
	{48, 222, 48}
};
*/
// Externed in Rotting Corpses.c
//Kris' attempt at blue night lights
/*
UINT16 gusShadeLevels[16][3]={
	{500, 500, 500}, // green table
	{450, 450, 450}, // bright
	{350, 350, 350},
	{300, 300, 300},
	{255, 255, 255}, // normal
	{215, 215, 227},
	{179, 179, 179},
	{149, 149, 149},
	{125, 125, 128},
	{104, 104, 128}, // darkening
	{86, 86, 128},
	{72, 72, 128},
	{60, 60, 128},
	{36, 36, 208},
	{18, 18, 224},
	{48, 222, 48}
};
*/

/*
//Linda's final version

UINT16 gusShadeLevels[16][3] =
{
	500, 500, 500,
	450, 450, 450, //bright
	350, 350, 350,
	300, 300, 300,
	255, 255, 255, //normal
	222, 200, 200,
	174, 167, 167,
	150, 137, 137,
	122, 116, 116, //darkening
	96, 96, 96,
	77, 77, 84,
	58, 58, 69,
	44, 44, 66, //night
	36, 36, 244,
	18, 18, 224,
	48, 222, 48,
};
*/

// JA2 Gold:
static UINT16 gusShadeLevels[16][3] =
{
	{ 500, 500, 500 },
	{ 450, 450, 450 }, // bright
	{ 350, 350, 350 },
	{ 300, 300, 300 },
	{ 255, 255, 255 }, // normal
	{ 231, 199, 199 },
	{ 209, 185, 185 },
	{ 187, 171, 171 },
	{ 165, 157, 157 }, // darkening
	{ 143, 143, 143 },
	{ 121, 121, 129 },
	{  99,  99, 115 },
	{  77,  77, 101 }, // night
	{  36,  36, 244 },
	{  18,  18, 224 },
	{  48, 222,  48 }
};

static LightTemplate* LightLoad(const ST::string& pFilename);


/****************************************************************************************
InitLightingSystem

	Initializes the lighting system.

***************************************************************************************/
void InitLightingSystem(void)
{
	// init all light lists
	std::fill(std::begin(g_light_templates), std::end(g_light_templates), LightTemplate{});

	// init all light sprites
	std::fill(std::begin(LightSprites), std::end(LightSprites), LIGHT_SPRITE{});

	LightLoad("TRANSLUC.LHT");
}


// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
void SetDefaultWorldLightingColors(void)
{
	static const SGPPaletteEntry pPal = { 0, 0, 0, 0 };
	LightSetColor(&pPal);
}


static BOOLEAN LightDelete(LightTemplate*);


/****************************************************************************************
ShutdownLightingSystem

	Closes down the lighting system. Any lights that were created are destroyed, and the
	memory attached to them freed up.

***************************************************************************************/
void ShutdownLightingSystem(void)
{
	// free up all allocated light nodes
	FOR_EACH_LIGHT_TEMPLATE(t)
	{
		LightDelete(t);
	}
}


/****************************************************************************************
LightReset

	Removes all currently active lights, without undrawing them.

***************************************************************************************/
void LightReset(void)
{
	// reset all light lists
	FOR_EACH_LIGHT_TEMPLATE(t)
	{
		LightDelete(t);
	}

	// init all light sprites
	std::fill(std::begin(LightSprites), std::end(LightSprites), LIGHT_SPRITE{});

	LightLoad("TRANSLUC.LHT");

	// Loop through mercs and reset light value
	FOR_EACH_SOLDIER(s) s->light = 0;
}


/* Creates a new node, and appends it to the template list. The index into the
	* list is returned. */
static UINT16 LightCreateTemplateNode(LightTemplate* const t, const INT16 iX, const INT16 iY, const UINT8 ubLight)
{
	Assert(t->lights.size() <= UINT16_MAX);
	const UINT16 idx = static_cast<UINT16>(t->lights.size());

	t->lights.push_back(LIGHT_NODE{});

	LIGHT_NODE* const l = &t->lights.back();
	l->iDX     = iX;
	l->iDY     = iY;
	l->ubLight = ubLight;
	l->uiFlags = 0;

	return idx;
}


/* Adds a node to the template list. If the node does not exist, it creates a
	* new one.  Returns the index into the list. */
static UINT16 LightAddTemplateNode(LightTemplate* const t, const INT16 iX, const INT16 iY, const UINT8 ubLight)
{
	Assert(t->lights.size() <= UINT16_MAX);
	for (UINT16 i = 0; i < static_cast<UINT16>(t->lights.size()); ++i)
	{
		if (t->lights[i].iDX == iX && t->lights[i].iDY == iY) return i;
	}
	return LightCreateTemplateNode(t, iX, iY, ubLight);
}


// Adds a node to the ray casting list.
static UINT16 LightAddRayNode(LightTemplate* const t, const INT16 iX, const INT16 iY, const UINT8 ubLight, const UINT16 usFlags)
{
	Assert(t->rays.size() <= UINT16_MAX);
	const UINT16 usIndex = static_cast<UINT16>(t->rays.size());
	UINT16 usNodeIndex = LightAddTemplateNode(t, iX, iY, ubLight) | usFlags;
	t->rays.push_back(usNodeIndex);
	return usIndex;
}


// Adds a node to the ray casting list.
static void LightInsertRayNode(LightTemplate* const t, const UINT16 usIndex, const INT16 iX, const INT16 iY, const UINT8 ubLight, const UINT16 usFlags)
{
	UINT16 usNodeIndex = LightAddTemplateNode(t, iX, iY, ubLight) | usFlags;
	t->rays.insert(t->rays.begin() + usIndex, 1, usNodeIndex);
}


struct IlluminationFilter
{
	bool            blocked{};                           // Light propagation can be blocked, but some structures still might need illumination
	bool            illuminateNothing{};
	bool            illuminateOrientedBlocksOnly{};      // Illuminate only oriented light-blocking structures (like walls and doors)
	Orientation     allowedOrients{ ORIENT_NONE };
	WorldDirections srcToDstDir{ DIRECTION_IRRELEVANT }; // Direction from source to destination tile
	int32_t         baseGridNo{};                        // E.g. open door slab and its frame (base) can be in different tiles

	IlluminationFilter(INT32 iSrcX, INT32 iSrcY, INT32 iX, INT32 iY)
	{
		UINT16 dstTileNo = MAPROWCOLTOPOS(iY, iX);
		UINT16 srcTileNo = MAPROWCOLTOPOS(iSrcY, iSrcX);

		if (dstTileNo >= GRIDSIZE || srcTileNo >= GRIDSIZE)
		{
			blocked = true;
			illuminateNothing = true;
			return;
		}

		if (gpWorldLevelData[dstTileNo].sHeight > gpWorldLevelData[srcTileNo].sHeight)
		{
			return;
		}

		srcToDstDir = static_cast<WorldDirections>(atan8(iSrcX, iSrcY, iX, iY));

		UINT8 ubTravelCost = gubWorldMovementCosts[dstTileNo][srcToDstDir][0];

		GridNo windowTileNo = srcToDstDir == NORTH || srcToDstDir == WEST ? dstTileNo : srcTileNo;

		if (gpWorldLevelData[windowTileNo].pStructHead)
		{
			// IF WE ARE A WINDOW, DO NOT BLOCK!
			if (ubTravelCost == TRAVELCOST_WALL && FindStructure(windowTileNo, STRUCTURE_WALLNWINDOW))
			{
				if (windowTileNo == dstTileNo)
				{
					if (srcToDstDir == NORTH)
					{
						allowedOrients = ORIENT_LEFT;
					}
					else if (srcToDstDir == WEST)
					{
						allowedOrients = ORIENT_RIGHT;
					}
				}
				return;
			}
		}

		if (dstTileNo == srcTileNo)
		{
			return;
		}

		if (ubTravelCost == TRAVELCOST_WALL)
		{
			blocked = true;
			if (srcToDstDir == NORTH)
			{
				illuminateOrientedBlocksOnly = true;
				allowedOrients = ORIENT_LEFT;
			}
			else if (srcToDstDir == WEST)
			{
				illuminateOrientedBlocksOnly = true;
				allowedOrients = ORIENT_RIGHT;
			}
			else
			{
				illuminateNothing = true;
			}
			return;
		}

		if (IS_TRAVELCOST_DOOR(ubTravelCost))
		{
			ubTravelCost = DoorTravelCost(NULL, dstTileNo, ubTravelCost, TRUE, &baseGridNo);

			if (IS_TRAVELCOST_OPEN_DOOR(ubTravelCost) && baseGridNo != srcTileNo)
			{
				blocked = true;
				illuminateOrientedBlocksOnly = true;
				if (srcToDstDir == NORTH)
				{
					allowedOrients = ORIENT_RIGHT;
				}
				else if (srcToDstDir == WEST)
				{
					allowedOrients = ORIENT_LEFT;
				}
				else
				{	// this includes diagonal directions
					illuminateNothing = true;
				}
				return;
			}
			else
			{
				baseGridNo = 0;
			}

			if (ubTravelCost == TRAVELCOST_DOOR || IS_TRAVELCOST_DOOR(ubTravelCost))
			{
				blocked = true;
				illuminateOrientedBlocksOnly = true;
				if (srcToDstDir == NORTH)
				{
					allowedOrients = ORIENT_LEFT;
				}
				else if (srcToDstDir == WEST)
				{
					allowedOrients = ORIENT_RIGHT;
				}
				else
				{	// this includes diagonal directions
					illuminateNothing = true;
				}
				return;
			}
		}

		if (ubTravelCost == TRAVELCOST_OBSTACLE)
		{
			STRUCTURE* pStruct = gpWorldLevelData[dstTileNo].pStructureHead;
			while (pStruct)
			{
				if (pStruct->fFlags & (STRUCTURE_PASSABLE | STRUCTURE_ANYDOOR | STRUCTURE_WALLSTUFF | STRUCTURE_TREE) ||
					pStruct->ubStructureHeight < STANDING_CUBES || pStruct->sCubeOffset == PROFILE_Z_SIZE)
				{
					pStruct = pStruct->pNext;
					continue;
				}

				UINT8 boxColumnCount = 0;

				for (auto& row : *pStruct->pShape)
				{
					for (auto& elem : row)
					{	// count columns that are at least 3 boxes high
						if (elem == 0b0111 || elem == 0b1111)
						{
							boxColumnCount++;
						}
					}
				}

				if (boxColumnCount >= MIN_BOX_COLUMNS)
				{
					blocked = true;
					return;
				}

				pStruct = pStruct->pNext;
			}
		}

		if (ubTravelCost == TRAVELCOST_CAVEWALL)
		{
			blocked = true;
			return;
		}
	}
};

// Removes a light template from the list, and frees up the associated node memory.
static BOOLEAN LightDelete(LightTemplate* const t)
{
	if (t->lights.empty()) return FALSE;

	t->lights.clear();
	t->rays.clear();
	t->name = ""; // clear() before ST 3.4

	return TRUE;
}


/* Returns an available slot for a new light template. */
static LightTemplate* LightGetFree(void)
{
	FOR_EACH_LIGHT_TEMPLATE_SLOT(t)
	{
		if (t->lights.empty()) return t;
	}
	throw std::runtime_error("Out of light template slots");
}


/* Calculates the 2D linear distance between two points. Returns the result in
	* a DOUBLE for greater accuracy. */
static DOUBLE LinearDistanceDouble(INT16 iX1, INT16 iY1, INT16 iX2, INT16 iY2)
{
	INT32 iDx, iDy;

	iDx = std::abs(iX1 - iX2);
	iDx*=iDx;
	iDy = std::abs(iY1 - iY2);
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
		int iSum = pNode->ubNaturalShadeLevel - (pNode->ubSumLights - pNode->ubFakeShadeLevel);
		iSum = std::clamp(iSum, SHADE_MAX, SHADE_MIN); // looks wrong because min and max have inverted values
		return( (UINT8) iSum );
	}
}


// Does the addition of light values to individual LEVELNODEs in the world tile list.
static void LightAddTileNode(LEVELNODE* const pNode, const UINT8 ubShadeAdd, const BOOLEAN fFake)
{
	pNode->ubSumLights += ubShadeAdd;
	if (fFake)
	{
		pNode->ubFakeShadeLevel += ubShadeAdd;
	}

	// Now set max
	pNode->ubMaxLights = std::max(pNode->ubMaxLights, ubShadeAdd);

	int sSum = pNode->ubNaturalShadeLevel - pNode->ubMaxLights;
	sSum = std::clamp(sSum, SHADE_MAX, SHADE_MIN);

	pNode->ubShadeLevel=(UINT8)sSum;
}


// Does the subtraction of light values to individual LEVELNODEs in the world tile list.
static void LightSubtractTileNode(LEVELNODE* const pNode, const UINT8 ubShadeSubtract, const BOOLEAN fFake)
{
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
	pNode->ubMaxLights = std::min(pNode->ubMaxLights, pNode->ubSumLights);

	int sSum = pNode->ubNaturalShadeLevel - pNode->ubMaxLights;
	sSum = std::clamp(sSum, SHADE_MAX, SHADE_MIN);
	pNode->ubShadeLevel=(UINT8)sSum;
}


// Adds a specified amount of light to all objects on a given tile.
static BOOLEAN LightAddTile(const INT16 iX, const INT16 iY, const UINT8 ubShade, const UINT32 uiFlags, const IlluminationFilter& filter)
{
	LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;

	UINT32 uiTile = static_cast<UINT32>(MAPROWCOLTOPOS(iY, iX));

	if ( uiTile >= GRIDSIZE )
	{
		return( FALSE );
	}

	gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;

	UINT8 ubShadeAdd = ubShade;

	bool fFake = uiFlags & LIGHT_FAKE ? true : false;

	if (!(uiFlags & LIGHT_ROOF_ONLY) || (uiFlags & LIGHT_EVERYTHING))
	{
		pStruct = gpWorldLevelData[uiTile].pStructHead;
		while(pStruct)
		{
			TileTypeDefines tileType = static_cast<TileTypeDefines>(gTileDatabase[pStruct->usIndex].fType);
			bool isOrientedBlock{}, isCaveWall{};
			Orientation wallOrient{};

			if (pStruct->pStructureData)
			{
				isOrientedBlock = pStruct->pStructureData->fFlags & (STRUCTURE_WALLSTUFF | STRUCTURE_ANYDOOR);
				wallOrient = pStruct->pStructureData->ubWallOrientation;
				isCaveWall = pStruct->pStructureData->fFlags & STRUCTURE_CAVEWALL;
			}
			else if (filter.illuminateOrientedBlocksOnly && tileType >= FIRSTWALLDECAL && tileType <= EIGTHWALLDECAL)
			{
				isOrientedBlock = true;
				wallOrient = filter.allowedOrients;
			}

			bool isCliff = tileType == FIRSTCLIFFHANG;
			if ( (filter.illuminateOrientedBlocksOnly && (!isOrientedBlock || !(wallOrient & filter.allowedOrients))) ||
				 (isOrientedBlock && filter.allowedOrients == ORIENT_NONE) ||
				  isCliff )
			{
				pStruct = pStruct->pNext;
				continue;
			}

			LightAddTileNode(pStruct, ubShadeAdd, FALSE);

			pStruct=pStruct->pNext;
		}

		ubShadeAdd = ubShade;

		if ( !filter.illuminateOrientedBlocksOnly )
		{
			pLand = gpWorldLevelData[uiTile].pLandHead;

			while( pLand )
			{
				LightAddTileNode(pLand, ubShadeAdd, fFake);
				pLand = pLand->pNext;
			}

			pObject = gpWorldLevelData[uiTile].pObjectHead;
			while(pObject != NULL)
			{
				LightAddTileNode(pObject, ubShadeAdd, FALSE);
				pObject = pObject->pNext;
			}

			if(uiFlags&LIGHT_BACKLIGHT)
				ubShadeAdd = (INT16)ubShade*7/10;

			pMerc = gpWorldLevelData[uiTile].pMercHead;
			while(pMerc != NULL)
			{
				LightAddTileNode(pMerc, ubShadeAdd, FALSE);
				pMerc = pMerc->pNext;
			}
		}
	}

	if((uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pRoof = gpWorldLevelData[uiTile].pRoofHead;
		while(pRoof!=NULL)
		{
			LightAddTileNode(pRoof, ubShadeAdd, fFake);
			pRoof=pRoof->pNext;
		}

		pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
		while(pOnRoof!=NULL)
		{
			LightAddTileNode(pOnRoof, ubShadeAdd, FALSE);

			pOnRoof=pOnRoof->pNext;
		}
	}
	return(TRUE);
}


// Subtracts a specified amount of light to a given tile.
static BOOLEAN LightSubtractTile(const INT16 iX, const INT16 iY, const UINT8 ubShade, const UINT32 uiFlags, const IlluminationFilter& filter)
{
	LEVELNODE *pLand, *pStruct, *pObject, *pMerc, *pRoof, *pOnRoof;

	UINT32 uiTile = static_cast<UINT32>(MAPROWCOLTOPOS(iY, iX));

	if ( uiTile >= GRIDSIZE )
	{
		return( FALSE );
	}

	gpWorldLevelData[uiTile].uiFlags|=MAPELEMENT_REDRAW;

	UINT8 ubShadeSubstract = ubShade;

	bool fFake = uiFlags & LIGHT_FAKE ? true : false;

	if (!(uiFlags & LIGHT_ROOF_ONLY) || (uiFlags & LIGHT_EVERYTHING))
	{
		pStruct = gpWorldLevelData[uiTile].pStructHead;
		while(pStruct)
		{
			bool isCliff = gTileDatabase[pStruct->usIndex].fType == FIRSTCLIFFHANG;
			bool isOrientedBlock{}, isCaveWall{};
			Orientation wallOrient{};
			if (pStruct->pStructureData)
			{
				isOrientedBlock = pStruct->pStructureData->fFlags & (STRUCTURE_WALLSTUFF | STRUCTURE_ANYDOOR);
				wallOrient = pStruct->pStructureData->ubWallOrientation;
				isCaveWall = pStruct->pStructureData->fFlags & STRUCTURE_CAVEWALL;
			}

			if ( (filter.illuminateOrientedBlocksOnly && (!isOrientedBlock || !(wallOrient & filter.allowedOrients))) ||
				 (isOrientedBlock && filter.allowedOrients == ORIENT_NONE) ||
				  isCliff )
			{
				pStruct = pStruct->pNext;
				continue;
			}

			LightSubtractTileNode(pStruct, ubShadeSubstract, FALSE);

			pStruct=pStruct->pNext;
		}

		ubShadeSubstract = ubShade;

		if ( !filter.illuminateOrientedBlocksOnly )
		{
			pLand = gpWorldLevelData[uiTile].pLandHead;

			while( pLand )
			{
				LightSubtractTileNode(pLand, ubShadeSubstract, fFake);
				pLand = pLand->pNext;
			}

			pObject = gpWorldLevelData[uiTile].pObjectHead;
			while(pObject!=NULL)
			{
				LightSubtractTileNode(pObject, ubShadeSubstract, FALSE);
				pObject = pObject->pNext;
			}

			if(uiFlags&LIGHT_BACKLIGHT)
				ubShadeSubstract = (INT16)ubShade * 7 / 10;

			pMerc = gpWorldLevelData[uiTile].pMercHead;
			while(pMerc!=NULL)
			{
				LightSubtractTileNode(pMerc, ubShadeSubstract, FALSE);
				pMerc = pMerc->pNext;
			}
		}
	}

	if((uiFlags&LIGHT_ROOF_ONLY) || (uiFlags&LIGHT_EVERYTHING))
	{
		pRoof = gpWorldLevelData[uiTile].pRoofHead;
		while(pRoof!=NULL)
		{
			LightSubtractTileNode(pRoof, ubShadeSubstract, fFake);
			pRoof=pRoof->pNext;
		}

		pOnRoof = gpWorldLevelData[uiTile].pOnRoofHead;
		while(pOnRoof!=NULL)
		{
			LightSubtractTileNode(pOnRoof, ubShadeSubstract, FALSE);

			pOnRoof=pOnRoof->pNext;
		}
	}

	return(TRUE);
}


/* Set the natural light level (as well as the current) on all LEVELNODEs on a
	* level. */
static void LightSetNaturalLevel(LEVELNODE* n, UINT8 const shade)
{
	for (; n; n = n->pNext)
	{
		n->ubSumLights         = 0;
		n->ubMaxLights         = 0;
		n->ubNaturalShadeLevel = shade;
		n->ubShadeLevel        = shade;
	}
}


/* Set the natural light value of all objects on a given tile to the specified
	* value.  This is the light value a tile has with no artificial lighting
	* affecting it. */
static void LightSetNaturalTile(MAP_ELEMENT const& e, UINT8 shade)
{
	LightSetNaturalLevel(e.pLandHead,    shade);
	LightSetNaturalLevel(e.pObjectHead,  shade);
	LightSetNaturalLevel(e.pStructHead,  shade);
	LightSetNaturalLevel(e.pMercHead,    shade);
	LightSetNaturalLevel(e.pRoofHead,    shade);
	LightSetNaturalLevel(e.pOnRoofHead,  shade);
	LightSetNaturalLevel(e.pTopmostHead, shade);
}


/* Reset the light level of all LEVELNODEs on a level to the value contained in
	* the natural light level. */
static void LightResetLevel(LEVELNODE* n)
{
	for (; n; n = n->pNext)
	{
		n->ubSumLights      = 0;
		n->ubMaxLights      = 0;
		n->ubShadeLevel     = n->ubNaturalShadeLevel;
		n->ubFakeShadeLevel = 0;
	}
}


// Reset all tiles on the map to their baseline values.
static void LightResetAllTiles(void)
{
	FOR_EACH_WORLD_TILE(i)
	{
		LightResetLevel(i->pLandHead);
		LightResetLevel(i->pObjectHead);
		LightResetLevel(i->pStructHead);
		LightResetLevel(i->pMercHead);
		LightResetLevel(i->pRoofHead);
		LightResetLevel(i->pOnRoofHead);
		LightResetLevel(i->pTopmostHead);
	}
}


// Creates a new node, and adds it to the end of a light list.
static void LightAddNode(LightTemplate* const t, const INT16 iHotSpotX, const INT16 iHotSpotY, INT16 iX, INT16 iY, const UINT8 ubIntensity, const UINT16 uiFlags)
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

	LightAddRayNode(t, iX, iY, ubShade, uiFlags);
}


// Creates a new node, and inserts it after the specified node.
static void LightInsertNode(LightTemplate* const t, const UINT16 usLightIns, const INT16 iHotSpotX, const INT16 iHotSpotY, INT16 iX, INT16 iY, const UINT8 ubIntensity, const UINT16 uiFlags)
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

	LightInsertRayNode(t, usLightIns, iX, iY, ubShade, uiFlags);
}


/* Traverses the linked list until a node with the LIGHT_NEW_RAY marker, and
	* returns the pointer. If the end of list is reached, NULL is returned. */
static UINT16 LightFindNextRay(const LightTemplate* const t, const UINT16 usIndex)
{
	UINT16 usNodeIndex = usIndex;
	Assert(t->rays.size() <= UINT16_MAX);
	while ((usNodeIndex < static_cast<UINT16>(t->rays.size())) && !(t->rays[usNodeIndex] & LIGHT_NEW_RAY))
		usNodeIndex++;

	return(usNodeIndex);
}


/* Casts a ray from an origin to an end point, creating nodes and adding them
	* to the light list. */
static BOOLEAN LightCastRay(LightTemplate* const t, const INT16 iStartX, const INT16 iStartY, const INT16 iEndPointX, const INT16 iEndPointY, const UINT8 ubStartIntens, const UINT8 ubEndIntens)
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

	SLOGD("Drawing ({},{}) to ({},{})", iXPos, iYPos, iEndX, iEndY);
	LightAddNode(t, 32767, 32767, 32767, 32767, 0, LIGHT_NEW_RAY);
	Assert(t->rays.size() <= UINT16_MAX);
	if (fInsertNodes) usCurNode = static_cast<UINT16>(t->rays.size());
	/* Special-case horizontal, vertical, and diagonal lines, for speed
		and to avoid nasty boundary conditions and division by 0 */
	if (XDelta == 0)
	{
		/* Vertical line */
		if(fInsertNodes)
		{
			for (i=0; i<=YDelta; i++)
			{
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iYPos++;
			}
		}
		else
		{
			for (i=0; i<=YDelta; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iXPos+=XAdvance;
			}
		}
		else
		{
			for (i=0; i<=XDelta; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iXPos+=XAdvance;
				iYPos++;
			}
		}
		else
		{
			for (i=0; i<=XDelta; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iXPos+=XAdvance;
			}
		}
		else
		{
			for (i=0; i<InitialPixelCount; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iXPos+=XAdvance;
				}
			}
			else
			{
				for (i=0; i<RunLength; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iXPos+=XAdvance;
			}
		}
		else
		{
			for (i=0; i<FinalPixelCount; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iYPos++;
			}
		}
		else
		{
			for (i=0; i<InitialPixelCount; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
					LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
					iYPos++;
				}
			}
			else
			{
				for (i=0; i<RunLength; i++)
				{
					LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
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
				LightInsertNode(t, usCurNode, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iYPos++;
			}
		}
		else
		{
			for (i=0; i<FinalPixelCount; i++)
			{
				LightAddNode(t, iStartX, iStartY, iXPos, iYPos, ubStartIntens, usFlags);
				iYPos++;
			}
		}
		iXPos+=XAdvance;
	}
	return(TRUE);
}


// Creates an elliptical light, taking two radii.
static void LightGenerateElliptical(LightTemplate* const t, const UINT8 iIntensity, const INT16 iA, const INT16 iB)
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
	LightCastRay(t, iX, iY, (INT16)iX, (INT16)(iY+iB), iIntensity, 1);
	LightCastRay(t, iX, iY, (INT16)iX, (INT16)(iY-iB), iIntensity, 1);

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
		LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
		LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
		LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
		LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
	}

	/* Draw the four symmetric arcs for which Y advances faster (that is,
		for which Y is the major axis) */
	/* Draw the initial left & right points */
	LightCastRay(t, iX, iY, (INT16)(iX+iA), iY, iIntensity, 1);
	LightCastRay(t, iX, iY, (INT16)(iX-iA), iY, iIntensity, 1);

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
		LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
		LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY-WorkingY), iIntensity, 1);
		LightCastRay(t, iX, iY, (INT16)(iX+WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
		LightCastRay(t, iX, iY, (INT16)(iX-WorkingX), (INT16)(iY+WorkingY), iIntensity, 1);
	}
}

/****************************************************************************************
	LightSetBaseLevel

		Sets the current and natural light settings for all tiles in the world.

***************************************************************************************/
void LightSetBaseLevel(UINT8 iIntensity)
{
	ubAmbientLightLevel=iIntensity;

	if( !gfEditMode )
	{
		// Loop for all good guys in tactical map and add a light if required
		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE* const s = *i;
			if (s->bTeam == OUR_TEAM) ReCreateSoldierLight(s);
		}
	}

	UINT16 shade = std::clamp(int(iIntensity), SHADE_MAX, SHADE_MIN);
	FOR_EACH_WORLD_TILE(i)
	{
		LightSetNaturalTile(*i, shade);
	}

	LightSpriteRenderAll();

	if(iIntensity >= LIGHT_DUSK_CUTOFF)
		RenderSetShadows(FALSE);
	else
		RenderSetShadows(TRUE);
}


LightTemplate* LightCreateOmni(const UINT8 ubIntensity, const INT16 iRadius)
{
	LightTemplate* const t = LightGetFree();

	LightGenerateElliptical(t, ubIntensity, iRadius * DISTANCE_SCALE, iRadius * DISTANCE_SCALE);

	t->name = ST::format("LTO{}.LHT", iRadius);

	return t;
}


BOOLEAN LightDraw(const LIGHT_SPRITE* const l)
{
	LightTemplate* const t = l->light_template;
	if (t->lights.empty()) return FALSE;

	// clear out all the flags
	for (LIGHT_NODE& light : t->lights)
	{
		light.uiFlags = 0;
	}

	const INT16 centerX = l->iX;
	const INT16 centerY = l->iY;

	INT32 srcX = centerX;
	INT32 srcY = centerY;

	Assert(t->rays.size() <= UINT16_MAX);
	for (UINT16 uiCount = 0; uiCount < static_cast<UINT16>(t->rays.size()); ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (usNodeIndex & LIGHT_NEW_RAY)
		{
			srcX = centerX;
			srcY = centerY;
			continue;
		}

		LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];
		const INT16 dstX = centerX + pLight->iDX;
		const INT16 dstY = centerY + pLight->iDY;

		IlluminationFilter filter{ srcX, srcY, dstX, dstY };

		if (filter.blocked)
		{
			uiCount = LightFindNextRay(t, uiCount);
		}

		if (!filter.illuminateNothing && !(pLight->uiFlags & LIGHT_NODE_DRAWN_FULLY) && pLight->ubLight)
		{
			UINT32 uiFlags = (UINT32)(usNodeIndex & LIGHT_BACKLIGHT);
			if (l->uiFlags & MERC_LIGHT)         uiFlags |= LIGHT_FAKE;
			if (l->uiFlags & LIGHT_SPR_ONROOF)   uiFlags |= LIGHT_ROOF_ONLY;
			if (filter.srcToDstDir == NORTH)     uiFlags |= LIGHT_NODE_DRAWN_NORTH;
			else if (filter.srcToDstDir == WEST) uiFlags |= LIGHT_NODE_DRAWN_WEST;

			if (filter.baseGridNo)
			{ // make a detour to the structure's base tile
				INT16 baseX, baseY;
				ConvertGridNoToXY(filter.baseGridNo, &baseX, &baseY);
				LightAddTile(baseX, baseY, pLight->ubLight, uiFlags, filter);
			}
			else
			{
				LightAddTile(dstX, dstY, pLight->ubLight, uiFlags, filter);
			}

			bool isCenterToDstDirCardinal = centerX == dstX || centerY == dstY;
			if (isCenterToDstDirCardinal || (pLight->uiFlags & (LIGHT_NODE_DRAWN_NORTH | LIGHT_NODE_DRAWN_WEST)))
			{
				pLight->uiFlags |= LIGHT_NODE_DRAWN_FULLY;
			}
		}

		srcX = centerX;
		srcY = centerY;

		if (!filter.blocked)
		{
			srcX += pLight->iDX;
			srcY += pLight->iDY;
		}
	}

	return(TRUE);
}

static BOOLEAN LightHideWall(const INT16 sX, const INT16 sY, const INT16 sSrcX, const INT16 sSrcY)
{
	UINT32     const uiTile = MAPROWCOLTOPOS(sY, sX);
	LEVELNODE* const head   = gpWorldLevelData[uiTile].pStructHead;

	BOOLEAN fDoRightWalls = (sX >= sSrcX);
	BOOLEAN fDoLeftWalls  = (sY >= sSrcY);

	for (const LEVELNODE* i = head; i != NULL; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;

		const TILE_ELEMENT* const te = &gTileDatabase[i->usIndex];
		if (te->usWallOrientation & ORIENT_RIGHT)
		{
			if (!fDoRightWalls) fDoLeftWalls = FALSE;
		}
		else if (te->usWallOrientation & ORIENT_LEFT)
		{
			if (!fDoLeftWalls) fDoRightWalls = FALSE;
		}
	}

	BOOLEAN fHitWall  = FALSE;
	BOOLEAN fRerender = FALSE;
	for (LEVELNODE* i = head; i != NULL; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;

		const TILE_ELEMENT* const te = &gTileDatabase[i->usIndex];
		if (te->usWallOrientation & ORIENT_RIGHT)
		{
			fHitWall = TRUE;
			if (fDoRightWalls && sX >= sSrcX)
			{
				i->uiFlags &= ~LEVELNODE_REVEAL;
				fRerender = TRUE;
			}
		}
		else if (te->usWallOrientation & ORIENT_LEFT)
		{
			fHitWall = TRUE;
			if (fDoLeftWalls && sY >= sSrcY)
			{
				i->uiFlags &= ~LEVELNODE_REVEAL;
				fRerender = TRUE;
			}
		}
	}

	if (fRerender) SetRenderFlags(RENDER_FLAG_FULL);
	return fHitWall;
}

/****************************************************************************************
	ApplyTranslucencyToWalls

		Hides walls that were revealed by CalcTranslucentWalls.

***************************************************************************************/
BOOLEAN ApplyTranslucencyToWalls(INT16 iX, INT16 iY)
{
	LightTemplate* const t = &g_light_templates[0];
	if (t->lights.empty()) return FALSE;

	Assert(t->rays.size() <= UINT16_MAX);
	for (UINT16 uiCount = 0; uiCount < static_cast<UINT16>(t->rays.size()); ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (!(usNodeIndex & LIGHT_NEW_RAY))
		{
			const LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];
			//Kris:  added map boundary checking!!!
			if(LightHideWall(
				(INT16) std::clamp(int(iX + pLight->iDX), 0, WORLD_COLS - 1),
				(INT16) std::clamp(int(iY + pLight->iDY), 0, WORLD_ROWS - 1),
				(INT16) std::clamp(int(iX), 0, WORLD_COLS - 1),
				(INT16) std::clamp(int(iY), 0, WORLD_ROWS - 1)
			))
			{
				uiCount = LightFindNextRay(t, uiCount);
				SetRenderFlags(RENDER_FLAG_FULL);
			}
		}
	}

	return(TRUE);
}


// Reverts all tiles a given light affects to their natural light levels.
static BOOLEAN LightErase(const LIGHT_SPRITE* const l)
{
	LightTemplate* const t = l->light_template;
	if (t->lights.empty()) return FALSE;

	// clear out all the flags
	for (LIGHT_NODE& light : t->lights)
	{
		light.uiFlags = 0;
	}

	const INT16 centerX = l->iX;
	const INT16 centerY = l->iY;

	INT32 srcX = centerX;
	INT32 srcY = centerY;

	Assert(t->rays.size() <= UINT16_MAX);
	for (UINT16 uiCount = 0; uiCount < static_cast<UINT16>(t->rays.size()); ++uiCount)
	{
		const UINT16 usNodeIndex = t->rays[uiCount];
		if (usNodeIndex & LIGHT_NEW_RAY)
		{
			srcX = centerX;
			srcY = centerY;
			continue;
		}

		LIGHT_NODE* const pLight = &t->lights[usNodeIndex & ~LIGHT_BACKLIGHT];
		const INT16 dstX = centerX + pLight->iDX;
		const INT16 dstY = centerY + pLight->iDY;

		IlluminationFilter filter{ srcX, srcY, dstX, dstY };

		if (filter.blocked)
		{
			uiCount = LightFindNextRay(t, uiCount);
		}

		if (!filter.illuminateNothing && !(pLight->uiFlags & LIGHT_NODE_DRAWN_FULLY) && pLight->ubLight)
		{
			UINT32 uiFlags = (UINT32)(usNodeIndex & LIGHT_BACKLIGHT);
			if (l->uiFlags & MERC_LIGHT)         uiFlags |= LIGHT_FAKE;
			if (l->uiFlags & LIGHT_SPR_ONROOF)   uiFlags |= LIGHT_ROOF_ONLY;
			if (filter.srcToDstDir == NORTH)     uiFlags |= LIGHT_NODE_DRAWN_NORTH;
			else if (filter.srcToDstDir == WEST) uiFlags |= LIGHT_NODE_DRAWN_WEST;

			if (filter.baseGridNo)
			{ // make a detour to the structure's base tile
				INT16 baseX, baseY;
				ConvertGridNoToXY(filter.baseGridNo, &baseX, &baseY);
				LightSubtractTile(baseX, baseY, pLight->ubLight, uiFlags, filter);
			}
			else
			{
				LightSubtractTile(dstX, dstY, pLight->ubLight, uiFlags, filter);
			}

			bool isCenterToDstDirCardinal = centerX == dstX || centerY == dstY;
			if (isCenterToDstDirCardinal || (pLight->uiFlags & (LIGHT_NODE_DRAWN_NORTH | LIGHT_NODE_DRAWN_WEST)))
			{
				pLight->uiFlags |= LIGHT_NODE_DRAWN_FULLY;
			}
		}

		srcX = centerX;
		srcY = centerY;

		if (!filter.blocked)
		{
			srcX += pLight->iDX;
			srcY += pLight->iDY;
		}
	}

	return(TRUE);
}


/****************************************************************************************
LightSave

	Saves the light list of a given template to a file. Passing in NULL for the
	filename forces the system to save the light with the internal filename (recommended).

***************************************************************************************/
void LightSave(LightTemplate const* const t, const ST::string& pFilename)
{
	if (t->lights.empty()) throw std::logic_error("Tried to save invalid light template");

	const ST::string& pName = (pFilename.empty() ? t->name : pFilename);
	AutoSGPFile f(FileMan::openForWriting(pName));
	Assert(t->lights.size() <= UINT16_MAX);
	UINT16 numLights = static_cast<UINT16>(t->lights.size());
	f->writeArray(numLights, t->lights.data());
	Assert(t->rays.size() <= UINT16_MAX);
	UINT16 numRays = static_cast<UINT16>(t->rays.size());
	f->writeArray(numRays, t->rays.data());
}


/* Loads a light template from disk. The light template is returned, or NULL if
	* the file wasn't loaded. */
static LightTemplate* LightLoad(const ST::string& pFilename)
{
	AutoSGPFile hFile(GCM->openGameResForReading(pFilename));

	UINT16 numLights;
	hFile->read(&numLights, sizeof(UINT16));
	std::vector<LIGHT_NODE> lights;
	lights.assign(numLights, LIGHT_NODE{});
	hFile->read(lights.data(), sizeof(LIGHT_NODE) * numLights);

	UINT16 numRays;
	hFile->read(&numRays, sizeof(UINT16));
	std::vector<UINT16> rays;
	rays.assign(numRays, 0);
	hFile->read(rays.data(), sizeof(UINT16) * numRays);

	LightTemplate* const t = LightGetFree();
	t->lights   = std::move(lights);
	t->rays     = std::move(rays);
	t->name     = pFilename;
	return t;
}


/* Figures out whether a light template is already in memory, or needs to be
	* loaded from disk. */
static LightTemplate* LightLoadCachedTemplate(const ST::string& pFilename)
{
	FOR_EACH_LIGHT_TEMPLATE(t)
	{
		if (pFilename.compare_i(t->name) == 0) return t;
	}
	return LightLoad(pFilename);
}


const SGPPaletteEntry* LightGetColor(void)
{
	return &gpOrigLight;
}


void LightSetColor(const SGPPaletteEntry* const pPal)
{
	Assert(pPal != NULL);

	if (pPal->r != g_light_color.r ||
			pPal->g != g_light_color.g ||
			pPal->b != g_light_color.b)
	{	//Set the entire tileset database so that it reloads everything.  It has to because the
		//colors have changed.
		SetAllNewTileSurfacesLoaded( TRUE );
	}

	// before doing anything, get rid of all the old palettes
	DestroyTileShadeTables( );

	g_light_color = *pPal;
	gpOrigLight   = *pPal;

	BuildTileShadeTables( );

	// Build all palettes for all soldiers in the world
	// ( THIS FUNCTION WILL ERASE THEM IF THEY EXIST )
	RebuildAllSoldierShadeTables( );

	SetRenderFlags(RENDER_FLAG_FULL);
}

//---------------------------------------------------------------------------------------
// Light Manipulation Layer
//---------------------------------------------------------------------------------------


// Returns the next available sprite.
static LIGHT_SPRITE* LightSpriteGetFree(void)
{
	FOR_EACH(LIGHT_SPRITE, l, LightSprites)
	{
		if (!(l->uiFlags & LIGHT_SPR_ACTIVE)) return l;
	}
	throw std::runtime_error("Out of light sprite slots");
}


LIGHT_SPRITE* LightSpriteCreate(const ST::string& pName)
try
{
	LIGHT_SPRITE* const l = LightSpriteGetFree();

	*l = LIGHT_SPRITE{};
	l->iX          = WORLD_COLS + 1;
	l->iY          = WORLD_ROWS + 1;

	l->light_template = LightLoadCachedTemplate(pName);

	l->uiFlags |= LIGHT_SPR_ACTIVE;
	return l;
}
catch (const std::runtime_error& err) {
	SLOGE("Failed to create light sprite {}: {}", pName, err.what());
	return nullptr;
}


BOOLEAN LightSpriteFake(LIGHT_SPRITE* const l)
{
	if (l->uiFlags & LIGHT_SPR_ACTIVE)
	{
		l->uiFlags |= MERC_LIGHT;
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static void LightSpriteDirty(const LIGHT_SPRITE* l);


BOOLEAN LightSpriteDestroy(LIGHT_SPRITE* const l)
{
	if (l->uiFlags & LIGHT_SPR_ACTIVE)
	{
		if (l->uiFlags & LIGHT_SPR_ERASE)
		{
			if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
			{
				LightErase(l);
				LightSpriteDirty(l);
			}
			l->uiFlags &= ~LIGHT_SPR_ERASE;
		}

		l->uiFlags &= ~LIGHT_SPR_ACTIVE;
		return(TRUE);
	}

	return(FALSE);
}


void LightSpriteRenderAll()
{
	LightResetAllTiles();
	FOR_EACH(LIGHT_SPRITE, i, LightSprites)
	{
		LIGHT_SPRITE& l = *i;
		l.uiFlags &= ~LIGHT_SPR_ERASE;
		if (!(l.uiFlags & LIGHT_SPR_ACTIVE)) continue;
		if (!(l.uiFlags & LIGHT_SPR_ON))     continue;
		LightDraw(&l);
		l.uiFlags |= LIGHT_SPR_ERASE;
		LightSpriteDirty(&l);
	}
}


void LightSpritePosition(LIGHT_SPRITE* const l, const INT16 iX, const INT16 iY)
{
	Assert(l->uiFlags & LIGHT_SPR_ACTIVE);

	if (l->iX == iX && l->iY == iY) return;

	if (l->uiFlags & LIGHT_SPR_ERASE)
	{
		if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
		{
			LightErase(l);
			LightSpriteDirty(l);
		}
	}

	l->iX = iX;
	l->iY = iY;

	if (l->uiFlags & LIGHT_SPR_ON)
	{
		if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
		{
			LightDraw(l);
			l->uiFlags |= LIGHT_SPR_ERASE;
			LightSpriteDirty(l);
		}
	}
}


BOOLEAN LightSpriteRoofStatus(LIGHT_SPRITE* const l, BOOLEAN fOnRoof)
{
	if ( fOnRoof &&  (l->uiFlags & LIGHT_SPR_ONROOF)) return FALSE;
	if (!fOnRoof && !(l->uiFlags & LIGHT_SPR_ONROOF)) return FALSE;

	if (l->uiFlags & LIGHT_SPR_ACTIVE)
	{
		if (l->uiFlags & LIGHT_SPR_ERASE)
		{
			if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
			{
				LightErase(l);
				LightSpriteDirty(l);
			}
		}

		if (fOnRoof)
		{
			l->uiFlags |= LIGHT_SPR_ONROOF;
		}
		else
		{
			l->uiFlags &= ~LIGHT_SPR_ONROOF;
		}


		if (l->uiFlags & LIGHT_SPR_ON)
		{
			if (l->iX < WORLD_COLS && l->iY < WORLD_ROWS)
			{
				LightDraw(l);
				l->uiFlags |= LIGHT_SPR_ERASE;
				LightSpriteDirty(l);
			}
		}
	}
	else
		return(FALSE);

	return(TRUE);
}


void LightSpritePower(LIGHT_SPRITE* const l, const BOOLEAN fOn)
{
	l->uiFlags = (l->uiFlags & ~LIGHT_SPR_ON) | (fOn ? LIGHT_SPR_ON : 0);
}


// Sets the flag for the renderer to draw all marked tiles.
static void LightSpriteDirty(LIGHT_SPRITE const* const l)
{
#if 0 // XXX was commented out
	INT16 iLeft_s;
	INT16 iTop_s;
	CellXYToScreenXY(l->iX * CELL_X_SIZE, l->iY * CELL_Y_SIZE, &iLeft_s, &iTop_s);

	const LightTemplate* const t = &g_light_templates[l->iTemplate];

	iLeft_s += t->x_off;
	iTop_s  += t->y_off;

	const INT16 iMapLeft   = l->iX + t->map_left;
	const INT16 iMapTop    = l->iY + t->map_top;
	const INT16 iMapRight  = l->iX + t->map_right;
	const INT16 iMapBottom = l->iY + t->map_bottom;

	UpdateSaveBuffer();
	AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);
	AddBaseDirtyRect(iLeft_s, iTop_s, (INT16)(iLeft_s + t->width), (INT16)(iTop_s + t->height));
#endif

	SetRenderFlags(RENDER_FLAG_MARKED);
}


static void AddSaturatePalette(SGPPaletteEntry Dst[256], const SGPPaletteEntry Src[256], const SGPPaletteEntry* Bias)
{
	UINT8 r = Bias->r;
	UINT8 g = Bias->g;
	UINT8 b = Bias->b;
	for (UINT i = 0; i < 256; i++)
	{
		Dst[i].r = std::min(Src[i].r + r, 255);
		Dst[i].g = std::min(Src[i].g + g, 255);
		Dst[i].b = std::min(Src[i].b + b, 255);
	}
}


static void CreateShadedPalettes(UINT16* Shades[16], const SGPPaletteEntry ShadePal[256])
{
	const UINT16* sl0 = gusShadeLevels[0];
	Shades[0] = Create16BPPPaletteShaded(ShadePal, sl0[0], sl0[1], sl0[2], TRUE);
	for (UINT i = 1; i < 16; i++)
	{
		const UINT16* sl = gusShadeLevels[i];
		Shades[i] = Create16BPPPaletteShaded(ShadePal, sl[0], sl[1], sl[2], FALSE);
	}
}


void CreateBiasedShadedPalettes(UINT16* Shades[16], const SGPPaletteEntry ShadePal[256])
{
	SGPPaletteEntry LightPal[256];
	AddSaturatePalette(LightPal, ShadePal, &g_light_color);
	CreateShadedPalettes(Shades, LightPal);
}


/**********************************************************************************************
CreateObjectPaletteTables

	Creates the shading tables for 8-bit brushes. One highlight table is created, based on
	the object-type, 3 brightening tables, 1 normal, and 11 darkening tables. The entries are
	created iteratively, rather than in a loop to allow hand-tweaking of the values. If you
	change the HVOBJECT_SHADE_TABLES symbol, remember to add/delete entries here, it won't
	adjust automagically.

**********************************************************************************************/
void CreateTilePaletteTables(const HVOBJECT pObj)
{
	Assert(pObj != NULL);

	// build the shade tables
	CreateBiasedShadedPalettes(pObj->pShades, pObj->Palette());

	// build neutral palette as well!
	// Set current shade table to neutral color
	pObj->CurrentShade(4);
}


const char* LightSpriteGetTypeName(const LIGHT_SPRITE* const l)
{
	return l->light_template->name.c_str();
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Lighting, asserts)
{
	EXPECT_EQ(sizeof(LIGHT_NODE), 6u);
}

#endif
