#ifndef __LIGHT_EFFECTS
#define __LIGHT_EFFECTS

#include "JA2Types.h"


struct LIGHTEFFECT
{
	INT16        sGridNo; // gridno at which the tear gas cloud is centered

	UINT8        ubDuration; // the number of turns will remain effective
	UINT8        decayStage; // light template countdown to deactivation
	INT8         bAge; // the number of turns light has been around
	BOOLEAN      fAllocated;
	INT8         bType;
	LIGHT_SPRITE *light;
	UINT32       uiTimeOfLastUpdate;
};


// Decays all light effects...
void DecayLightEffects(UINT32 uiTime, bool updateSightings);

LIGHTEFFECT* NewLightEffect(INT16 sGridNo, UINT8 decayStage, UINT8 duration);

// Update light coverage after opening/closing doors or structure destruction
void UpdateLightCoverageOnStructureChange(GridNo structGridNo, UINT8 rangeExtension = 0);

void LoadLightEffectsFromLoadGameFile(HWFILE);

void SaveLightEffectsToMapTempFile(const SGPSector& sector);
void LoadLightEffectsFromMapTempFile(const SGPSector& sector);
void ResetLightEffects(void);

#endif
