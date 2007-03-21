#ifndef __LIGHT_EFFECTS
#define __LIGHT_EFFECTS


// Light effect types
enum
{
	NO_LIGHT_EFFECT,
	LIGHT_FLARE_MARK_1,
};


typedef struct
{
	INT16		sGridNo;          // gridno at which the tear gas cloud is centered

	UINT8		ubDuration;		       // the number of turns will remain effective
	UINT8		bRadius;	          // the current radius
	INT8		bAge;				        // the number of turns light has been around
	BOOLEAN	fAllocated;
	INT8		bType;
	INT32		iLight;
	UINT32	uiTimeOfLastUpdate;

} LIGHTEFFECT;



// Decays all light effects...
void DecayLightEffects( UINT32 uiTime );

INT32 NewLightEffect( INT16 sGridNo, INT8 bType );


BOOLEAN SaveLightEffectsToSaveGameFile( HWFILE hFile );
BOOLEAN LoadLightEffectsFromLoadGameFile( HWFILE hFile );

BOOLEAN SaveLightEffectsToMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ );
BOOLEAN LoadLightEffectsFromMapTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ );
void ResetLightEffects();

#endif
