#ifndef __LIGHT_EFFECTS
#define __LIGHT_EFFECTS

#include "JA2Types.h"


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
void DecayLightEffects(UINT32 uiTime, bool updateSightings);

LIGHTEFFECT* NewLightEffect(INT16 sGridNo, UINT8 radius, UINT8 duration);


void LoadLightEffectsFromLoadGameFile(HWFILE);

void SaveLightEffectsToMapTempFile(const SGPSector& sector);
void LoadLightEffectsFromMapTempFile(const SGPSector& sector);
void ResetLightEffects(void);

#endif
