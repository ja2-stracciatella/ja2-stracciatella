#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include "Soldier Control.h"

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

#define		DISTANCE_SCALE			4
#define		LIGHT_DUSK_CUTOFF		8
#define		LIGHT_DECAY					0.9	// shade level decay per tile distance

// lightlist node flags
#define		LIGHT_NODE_DRAWN		0x00000001		// light node duplicate marker
#define   LIGHT_ROOF_ONLY			0x00001000		// light only rooftops
#define		LIGHT_IGNORE_WALLS	0x00002000		// doesn't take walls into account
#define		LIGHT_BACKLIGHT			0x00004000		// light does not light objs, trees
#define		LIGHT_NEW_RAY				0x00008000		// start of new ray in linked list
#define		LIGHT_EVERYTHING		0x00010000		// light up everything
#define		LIGHT_FAKE					0x10000000		// "fake" light	for display only

// standard light file symbols

#define		LIGHT_OMNI_R1				"LTO1.LHT"
#define		LIGHT_OMNI_R2				"LTO2.LHT"
#define		LIGHT_OMNI_R3				"LTO3.LHT"
#define		LIGHT_OMNI_R4				"LTO4.LHT"
#define		LIGHT_OMNI_R5				"LTO5.LHT"
#define		LIGHT_OMNI_R6				"LTO6.LHT"
#define		LIGHT_OMNI_R7				"LTO7.LHT"
#define		LIGHT_OMNI_R8				"LTO8.LHT"

#define		MAX_LIGHT_TEMPLATES	32				// maximum number of light types
#define		MAX_LIGHT_SPRITES		256				// maximum number of light types
#define		SHADE_MIN						15				// DARKEST shade value
#define		SHADE_MAX						1					// LIGHTEST shade value


// light sprite flags
#define		LIGHT_SPR_ACTIVE		0x0001
#define		LIGHT_SPR_ON				0x0002
#define		LIGHT_SPR_ANIMATE		0x0004
#define		LIGHT_SPR_ERASE			0x0008
#define		LIGHT_SPR_REDRAW		0x0010
#define		LIGHT_SPR_ONROOF		0x0020
#define   MERC_LIGHT		      0x0040
#define		LIGHT_PRIMETIME			0x0080		// light turns goes on in evening, turns off at bedtime.
#define		LIGHT_NIGHTTIME			0x0100		// light stays on when dark outside


#define		COLOR_RED						162
#define		COLOR_BLUE					203
#define		COLOR_YELLOW				144
#define		COLOR_GREEN					184
#define		COLOR_LTGREY				134
#define		COLOR_DKGREY				136
#define		COLOR_BROWN					80
#define		COLOR_PURPLE				160
#define		COLOR_ORANGE				76
#define		COLOR_WHITE					208
#define		COLOR_BLACK					72


// stucture of node in linked list for lights
typedef struct light_p {
					INT16		iDX, iDY;
					UINT8		uiFlags;
					UINT8		ubLight;
			} LIGHT_NODE;

// structure of light instance, or sprite (a copy of the template)
typedef struct {
					INT16		iX, iY;
					INT16		iOldX, iOldY;
					INT16		iAnimSpeed;
					INT32		iTemplate;
					UINT32	uiFlags;
					UINT32	uiLightType;
			} LIGHT_SPRITE;


// Initializes the lighting system
BOOLEAN		InitLightingSystem(void);
// Shuts down, and frees up all lights/memory
BOOLEAN		ShutdownLightingSystem(void);
// Resets all light sprites and deallocates templates
BOOLEAN		LightReset(void);

// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
BOOLEAN SetDefaultWorldLightingColors(void);

// Low-Level Template Interface

// Sets the normal light level for all tiles in the world
BOOLEAN		LightSetBaseLevel(UINT8 iIntensity);
// Adds a light value to all tiles
BOOLEAN		LightAddBaseLevel(UINT32 uiLightType, UINT8 iIntensity);
// Subtracts a light value from all tiles
BOOLEAN		LightSubtractBaseLevel(UINT32 uiLightType, UINT8 iIntensity);
// Creates an omni (circular) light
INT32			LightCreateOmni( UINT8 ubIntensity, INT16 iRadius);
// Creates an oval-shaped light (two separate radii)
INT32			LightCreateElliptical(UINT8 ubIntensity, INT16 iRadius1, INT16 iRadius2);
// Creates a square light
INT32			LightCreateSquare(UINT8 ubIntensity, INT16 iRadius1, INT16 iRadius2);
// Draws a light into the scene at X,Y
BOOLEAN		LightDraw(UINT32 uiLightType, INT32 iLight, INT16 iX, INT16 iY, UINT32 uiSprite);
// Reverts the tiles a light has affected back to normal
BOOLEAN		LightErase(UINT32 uiLightType, INT32 iLight, INT16 iX, INT16 iY, UINT32 uiSprite);
// Save a light list into a file
BOOLEAN		LightSave(INT32 uiLight, STR pFilename);
// Load a light list from a file
INT32			LightLoad(STR pFilename);

// Sets the RGB values and number of light colors (1/2)
BOOLEAN LightSetColors(SGPPaletteEntry *pPal, UINT8 ubNumColors);
// Returns the number of colors active (1/2) and the palette colors
UINT8 LightGetColors(SGPPaletteEntry *pPal);


// High-Level Sprite Interface

// Creates a new light sprite from a given filename/predefined symbol
INT32			LightSpriteCreate(STR pName, UINT32 uiLightType);
// Destroys the instance of that light
BOOLEAN		LightSpriteDestroy(INT32 iSprite);
// Sets the tile position of the light instance
BOOLEAN		LightSpritePosition(INT32 iSprite, INT16 iX, INT16 iY);
// Makes a light "fake"
BOOLEAN LightSpriteFake(INT32 iSprite);
// Updates any change in position in lights
BOOLEAN		LightSpriteRender();
// Renders all lights
BOOLEAN		LightSpriteRenderAll(void);
// Turns on/off power to a light
BOOLEAN		LightSpritePower(INT32 iSprite, BOOLEAN fOn);
// Moves light to/from roof position
BOOLEAN		LightSpriteRoofStatus(INT32 iSprite, BOOLEAN fOnRoof);

// Reveals translucent walls
BOOLEAN		CalcTranslucentWalls(INT16 iX, INT16 iY);
BOOLEAN		ApplyTranslucencyToWalls(INT16 iX, INT16 iY);
// Makes trees translucent
BOOLEAN		LightTranslucentTrees(INT16 iX, INT16 iY);
BOOLEAN		LightHideTrees(INT16 iX, INT16 iY);
BOOLEAN		LightShowRays(INT16 iX, INT16 iY, BOOLEAN fReset);
BOOLEAN		LightHideRays(INT16 iX, INT16 iY);


// makes the 16-bit palettes
UINT16		CreateTilePaletteTables(HVOBJECT pObj, UINT32 uiType, BOOLEAN fForce );
BOOLEAN		CreateSoldierShadedPalette( SOLDIERTYPE *pSoldier, UINT32 uiBase, SGPPaletteEntry *pShadePal);
UINT16		CreateSoldierPaletteTables(SOLDIERTYPE *pSoldier, UINT32 uiType);

// returns the true light value at a tile (ignoring fake/merc lights)
UINT8 LightTrueLevel( INT16 sGridNo, INT8 bLevel );

// system variables
extern LIGHT_NODE				*pLightList[MAX_LIGHT_TEMPLATES];
extern UINT16						usTemplateSize[MAX_LIGHT_TEMPLATES];
extern UINT16						*pLightRayList[MAX_LIGHT_TEMPLATES];
extern UINT16						usRaySize[MAX_LIGHT_TEMPLATES];
extern INT16						LightHeight[MAX_LIGHT_TEMPLATES];
extern INT16						LightWidth[MAX_LIGHT_TEMPLATES];
extern INT16						LightXOffset[MAX_LIGHT_TEMPLATES];
extern INT16						LightYOffset[MAX_LIGHT_TEMPLATES];
extern INT16						LightMapLeft[MAX_LIGHT_TEMPLATES];
extern INT16						LightMapTop[MAX_LIGHT_TEMPLATES];
extern INT16						LightMapRight[MAX_LIGHT_TEMPLATES];
extern INT16						LightMapBottom[MAX_LIGHT_TEMPLATES];
extern STR							pLightNames[MAX_LIGHT_TEMPLATES];

// Sprite data
extern LIGHT_SPRITE			LightSprites[MAX_LIGHT_SPRITES];

// Lighting system general data
extern UINT8						ubAmbientLightLevel;
extern UINT8						gubNumLightColors;


// Lighting colors
extern UINT8						gubNumLightColors;
extern SGPPaletteEntry	gpLightColors[3];


// macros
#define LightSpriteGetType(x)				(LightSprites[x].uiLightType)
#define LightSpriteGetTypeName(x)		(pLightNames[LightSprites[x].iTemplate])
#define LightGetAmbient()						(ubAmbientLightLevel)

#endif
