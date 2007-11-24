#ifndef __SMOKE_EFFECTS
#define __SMOKE_EFFECTS


// Smoke effect types
enum
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


typedef struct TAG_SMOKE_EFFECT
{
	INT16		sGridNo;          // gridno at which the tear gas cloud is centered

	UINT8		ubDuration;        // the number of turns gas will remain effective
	UINT8		ubRadius;          // the current radius of the cloud in map tiles
	UINT8   bFlags;            // 0 - outdoors (fast spread), 1 - indoors (slow)
	INT8		bAge;             // the number of turns gas has been around
	BOOLEAN	fAllocated;
	INT8		bType;
	UINT16	usItem;
  UINT8   ubOwner;
  UINT8   ubPadding;
	UINT32	uiTimeOfLastUpdate;
} SMOKEEFFECT;
CASSERT(sizeof(SMOKEEFFECT) == 16)


// Returns NO_SMOKE_EFFECT if none there...
INT8 GetSmokeEffectOnTile( INT16 sGridNo, INT8 bLevel );

// Decays all smoke effects...
void DecaySmokeEffects( UINT32 uiTime );

// Add smoke to gridno
// ( Replacement algorithm uses distance away )
void AddSmokeEffectToTile(const SMOKEEFFECT* smoke, INT8 bType, INT16 sGridNo, INT8 bLevel);

void RemoveSmokeEffectFromTile( INT16 sGridNo, INT8 bLevel );

INT32 NewSmokeEffect(INT16 sGridNo, UINT16 usItem, INT8 bLevel, SOLDIERTYPE* owner);


BOOLEAN SaveSmokeEffectsToSaveGameFile( HWFILE hFile );
BOOLEAN LoadSmokeEffectsFromLoadGameFile( HWFILE hFile );


BOOLEAN SaveSmokeEffectsToMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ );
BOOLEAN LoadSmokeEffectsFromMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ );

void ResetSmokeEffects();

void UpdateSmokeEffectGraphics( );


#endif
