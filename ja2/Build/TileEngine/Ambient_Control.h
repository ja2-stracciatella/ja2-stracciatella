#ifndef __AMBIENT_CONTROL
#define __AMBIENT_CONTROL

#include "Ambient_Types.h"

BOOLEAN LoadAmbientControlFile( UINT8 ubAmbientID );

void HandleNewSectorAmbience( UINT8 ubAmbientID );
UINT32 SetupNewAmbientSound( UINT32 uiAmbientID );

void StopAmbients( );
void DeleteAllAmbients();

extern AMBIENTDATA_STRUCT		gAmbData[ MAX_AMBIENT_SOUNDS ];
extern INT16								gsNumAmbData;

BOOLEAN SetSteadyStateAmbience( UINT8 ubAmbience );


#define               SOUND_NAME_SIZE           256
#define               NUM_SOUNDS_PER_TIMEFRAME    8

enum
{
  SSA_NONE,
  SSA_COUNTRYSIZE,
  SSA_NEAR_WATER,
  SSA_IN_WATER,
  SSA_HEAVY_FOREST,
  SSA_PINE_FOREST,
  SSA_ABANDONED,
  SSA_AIRPORT,
  SSA_WASTELAND,
  SSA_UNDERGROUND,
  SSA_OCEAN,
  NUM_STEADY_STATE_AMBIENCES
};


typedef struct
{
  CHAR8 zSoundNames[ NUM_SOUNDS_PER_TIMEFRAME ][ SOUND_NAME_SIZE ];

} STEADY_STATE_AMBIENCE;



#endif
