#ifndef __SMOKE_EFFECTS
#define __SMOKE_EFFECTS

#include "JA2Types.h"

// Smoke effect types
enum SmokeEffectKind
{
	NO_SMOKE_EFFECT,
	NORMAL_SMOKE_EFFECT,
	TEARGAS_SMOKE_EFFECT,
	MUSTARDGAS_SMOKE_EFFECT,
	CREATURE_SMOKE_EFFECT,
};

#define SMOKE_EFFECT_INDOORS          0x01
#define SMOKE_EFFECT_ON_ROOF          0x02
#define SMOKE_EFFECT_MARK_FOR_UPDATE  0x04


struct SMOKEEFFECT
{
	INT16		sGridNo;          // gridno at which the tear gas cloud is centered

	UINT8		ubDuration;        // the number of turns gas will remain effective
	UINT8		ubRadius;          // the current radius of the cloud in map tiles
	UINT8   bFlags;            // 0 - outdoors (fast spread), 1 - indoors (slow)
	INT8		bAge;             // the number of turns gas has been around
	BOOLEAN	fAllocated;
	INT8		bType;
	UINT16	usItem;
	SOLDIERTYPE* owner;
	UINT32	uiTimeOfLastUpdate;
};


// Returns NO_SMOKE_EFFECT if none there...
SmokeEffectKind GetSmokeEffectOnTile(INT16 sGridNo, INT8 bLevel);

// Decays all smoke effects...
void DecaySmokeEffects( UINT32 uiTime );

// Add smoke to gridno
// ( Replacement algorithm uses distance away )
void AddSmokeEffectToTile(SMOKEEFFECT const*, SmokeEffectKind, INT16 sGridNo, INT8 bLevel);

void RemoveSmokeEffectFromTile( INT16 sGridNo, INT8 bLevel );

void NewSmokeEffect(INT16 sGridNo, UINT16 usItem, INT8 bLevel, SOLDIERTYPE* owner);


void LoadSmokeEffectsFromLoadGameFile(HWFILE, UINT32 savegame_version);


void SaveSmokeEffectsToMapTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
void LoadSmokeEffectsFromMapTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);

void ResetSmokeEffects(void);

void UpdateSmokeEffectGraphics(void);


#endif
