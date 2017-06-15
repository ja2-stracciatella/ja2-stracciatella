#ifndef __LIGHT_EFFECTS
#define __LIGHT_EFFECTS

#include "JA2Types.h"


// Light effect types
enum
{
	NO_LIGHT_EFFECT,
	LIGHT_FLARE_MARK_1,
};


struct LIGHTEFFECT
{
	INT16        sGridNo; // gridno at which the tear gas cloud is centered

	UINT8        ubDuration; // the number of turns will remain effective
	UINT8        bRadius; // the current radius
	INT8         bAge; // the number of turns light has been around
	BOOLEAN      fAllocated;
	INT8         bType;
	LIGHT_SPRITE *light;
	UINT32       uiTimeOfLastUpdate;
};


// Decays all light effects...
void DecayLightEffects( UINT32 uiTime );

LIGHTEFFECT* NewLightEffect(INT16 sGridNo, INT8 bType);


void LoadLightEffectsFromLoadGameFile(HWFILE);

void SaveLightEffectsToMapTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
void LoadLightEffectsFromMapTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
void ResetLightEffects(void);

#endif
