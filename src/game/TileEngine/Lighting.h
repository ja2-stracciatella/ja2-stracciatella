#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include "JA2Types.h"


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

#define DISTANCE_SCALE			4
#define LIGHT_DUSK_CUTOFF		8
#define LIGHT_DECAY			0.9 // shade level decay per tile distance

// lightlist node flags
#define LIGHT_NODE_DRAWN		0x00000001 // light node duplicate marker
#define LIGHT_ROOF_ONLY			0x00001000 // light only rooftops
#define LIGHT_IGNORE_WALLS		0x00002000 // doesn't take walls into account
#define LIGHT_BACKLIGHT			0x00004000 // light does not light objs, trees
#define LIGHT_NEW_RAY			0x00008000 // start of new ray in linked list
#define LIGHT_EVERYTHING		0x00010000 // light up everything
#define LIGHT_FAKE			0x10000000 // "fake" light for display only

// standard light file symbols

#define LIGHT_OMNI_R1			"LTO1.LHT"
#define LIGHT_OMNI_R2			"LTO2.LHT"
#define LIGHT_OMNI_R3			"LTO3.LHT"
#define LIGHT_OMNI_R4			"LTO4.LHT"
#define LIGHT_OMNI_R5			"LTO5.LHT"
#define LIGHT_OMNI_R6			"LTO6.LHT"
#define LIGHT_OMNI_R7			"LTO7.LHT"
#define LIGHT_OMNI_R8			"LTO8.LHT"

#define MAX_LIGHT_SPRITES		256 // maximum number of light types
#define SHADE_MIN			15 // DARKEST shade value
#define SHADE_MAX			1 // LIGHTEST shade value


// light sprite flags
#define LIGHT_SPR_ACTIVE		0x0001
#define LIGHT_SPR_ON			0x0002
#define LIGHT_SPR_ERASE			0x0008
#define LIGHT_SPR_ONROOF		0x0020
#define MERC_LIGHT			0x0040
#define LIGHT_PRIMETIME			0x0080 // light turns goes on in evening, turns off at bedtime.
#define LIGHT_NIGHTTIME			0x0100 // light stays on when dark outside


struct LightTemplate;


// structure of light instance, or sprite (a copy of the template)
struct LIGHT_SPRITE
{
	INT16         iX, iY;
	LightTemplate *light_template;
	UINT32        uiFlags;
};


// Initializes the lighting system
void InitLightingSystem(void);
// Shuts down, and frees up all lights/memory
void ShutdownLightingSystem(void);
// Resets all light sprites and deallocates templates
void LightReset(void);

// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
void SetDefaultWorldLightingColors(void);

// Low-Level Template Interface

// Sets the normal light level for all tiles in the world
void LightSetBaseLevel(UINT8 iIntensity);
// Adds a light value to all tiles in the world
void LightAddBaseLevel(UINT8 iIntensity);
// Subtracts a light value from all tiles
void LightSubtractBaseLevel(UINT8 iIntensity);
// Creates an omni (circular) light
LightTemplate* LightCreateOmni(UINT8 ubIntensity, INT16 iRadius);
// Draws a light into the scene at X,Y
BOOLEAN LightDraw(const LIGHT_SPRITE* l);
// Save a light list into a file
void LightSave(LightTemplate const*, char const* filename);

// Sets the light color
void LightSetColor(const SGPPaletteEntry* pPal);
// Returns the active color
const SGPPaletteEntry* LightGetColor(void);


// High-Level Sprite Interface

/* Creates an instance of a light. The template is loaded if it isn't already.
	* If this function fails (out of sprites, or bad template name) it returns
	* NULL. */
LIGHT_SPRITE* LightSpriteCreate(const char* const pName);
// Removes an instance of a light. If it was on, it is erased from the scene.
BOOLEAN LightSpriteDestroy(LIGHT_SPRITE* l);
// Sets the X,Y position (IN TILES) of a light instance.
void LightSpritePosition(LIGHT_SPRITE* l, INT16 iX, INT16 iY);
// Sets the flag of a light sprite to "fake" (in game for merc navig purposes)
BOOLEAN LightSpriteFake(LIGHT_SPRITE* l);

/* Reset all tiles in the world to the ambient light level and redraw all active
	* lights. */
void LightSpriteRenderAll();

// Turns on/off power to a light
void LightSpritePower(LIGHT_SPRITE* l, BOOLEAN fOn);
// Moves light to/from roof position
BOOLEAN LightSpriteRoofStatus(LIGHT_SPRITE* l, BOOLEAN fOnRoof);

BOOLEAN ApplyTranslucencyToWalls(INT16 iX, INT16 iY);


// makes the 16-bit palettes
void CreateTilePaletteTables(HVOBJECT pObj);

// returns the true light value at a tile (ignoring fake/merc lights)
UINT8 LightTrueLevel( INT16 sGridNo, INT8 bLevel );

// Sprite data
extern LIGHT_SPRITE LightSprites[MAX_LIGHT_SPRITES];

#define BASE_FOR_EACH_LIGHT_SPRITE(type, iter) \
	FOR_EACH(type, iter, LightSprites) \
		if (!(iter->uiFlags & LIGHT_SPR_ACTIVE)) continue; else
#define FOR_EACH_LIGHT_SPRITE( iter) BASE_FOR_EACH_LIGHT_SPRITE(      LIGHT_SPRITE, iter)
#define CFOR_EACH_LIGHT_SPRITE(iter) BASE_FOR_EACH_LIGHT_SPRITE(const LIGHT_SPRITE, iter)


// Lighting system general data
extern UINT8 ubAmbientLightLevel;

// Lighting color
extern SGPPaletteEntry g_light_color;


// macros
#define LightGetAmbient() (ubAmbientLightLevel)

const char* LightSpriteGetTypeName(const LIGHT_SPRITE*);

void CreateBiasedShadedPalettes(UINT16* Shades[16], const SGPPaletteEntry ShadePal[256]);

#endif
