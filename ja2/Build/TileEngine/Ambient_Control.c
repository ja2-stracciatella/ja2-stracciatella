#ifdef PRECOMPILEDHEADERS
	#include "TileEngine All.h"
#else
	#include "stdio.h"
	#include "SGP.h"
	#include "Ambient_Types.h"
	#include "FileMan.h"
	#include "Environment.h"
	#include "Sound_Control.h"
	#include "Game_Events.h"
	#include "Ambient_Control.h"
	#include <string.h>
	#include "Lighting.h"
	#include "Random.h"
	#include "WorldMan.h"
	#include "SoundMan.h"
	#include "Debug.h"
#endif

AMBIENTDATA_STRUCT		gAmbData[ MAX_AMBIENT_SOUNDS ];
INT16									gsNumAmbData = 0;


UINT8                   gubCurrentSteadyStateAmbience = SSA_NONE;
UINT8                   gubCurrentSteadyStateSound    = 0;
UINT32                  guiCurrentSteadyStateSoundHandle = NO_SAMPLE;
STEADY_STATE_AMBIENCE   gSteadyStateAmbientTable[ NUM_STEADY_STATE_AMBIENCES ] =
{
   // NONE
   "",
   "",
   "",
   "",
   // NIGHT
   "",
   "",
   "",
   "",
  // COUNTRYSIZE
  // DAY
  "SOUNDS/SSA/insects Day 01.wav",
  "",
  "",
  "",
  // NIGHT
  "SOUNDS/SSA/night_crickets_01D.wav",
  "SOUNDS/SSA/night_crickets_01B.wav",
  "SOUNDS/SSA/night_crickets_01C.wav",
  "SOUNDS/SSA/night_crickets_01A.wav",
  // NEAR WATER
  // DAY
  "SOUNDS/SSA/swamp_day_01a.wav",
  "SOUNDS/SSA/swamp_day_01b.wav",
  "SOUNDS/SSA/swamp_day_01c.wav",
  "SOUNDS/SSA/swamp_day_01d.wav",
  //NIGHT
  "SOUNDS/SSA/marsh_at_night_01a.wav",
  "SOUNDS/SSA/marsh_at_night_01b.wav",
  "SOUNDS/SSA/marsh_at_night_01c.wav",
  "SOUNDS/SSA/marsh_at_night_01d.wav",
  //INWATER
  //DAY
  "SOUNDS/SSA/middle_of_water_01d.wav",
  "SOUNDS/SSA/middle_of_water_01c.wav",
  "SOUNDS/SSA/middle_of_water_01b.wav",
  "SOUNDS/SSA/middle_of_water_01a.wav",
  // night
  "SOUNDS/SSA/middle_of_water_01d.wav",
  "SOUNDS/SSA/middle_of_water_01c.wav",
  "SOUNDS/SSA/middle_of_water_01b.wav",
  "SOUNDS/SSA/middle_of_water_01a.wav",
  // HEAVY FOREST
  // day
  "SOUNDS/SSA/JUNGLE_DAY_01a.wav",
  "SOUNDS/SSA/JUNGLE_DAY_01b.wav",
  "SOUNDS/SSA/JUNGLE_DAY_01c.wav",
  "SOUNDS/SSA/JUNGLE_DAY_01d.wav",
  // night
  "SOUNDS/SSA/night_crickets_03a.wav",
  "SOUNDS/SSA/night_crickets_03b.wav",
  "SOUNDS/SSA/night_crickets_03c.wav",
  "SOUNDS/SSA/night_crickets_03d.wav",
  // PINE FOREST
  // DAY
  "SOUNDS/SSA/pine_forest_01a.wav",
  "SOUNDS/SSA/pine_forest_01b.wav",
  "SOUNDS/SSA/pine_forest_01c.wav",
  "SOUNDS/SSA/pine_forest_01d.wav",
  // NIGHT
  "SOUNDS/SSA/night_crickets_02a.wav",
  "SOUNDS/SSA/night_crickets_02b.wav",
  "SOUNDS/SSA/night_crickets_02c.wav",
  "SOUNDS/SSA/night_crickets_02d.wav",
   // ABANDANDED
   // DAY
   "SOUNDS/SSA/metal_wind_01a.wav",
   "SOUNDS/SSA/metal_wind_01b.wav",
   "SOUNDS/SSA/metal_wind_01c.wav",
   "SOUNDS/SSA/metal_wind_01d.wav",
    // NIGHT
   "SOUNDS/SSA/night_insects_01a.wav",
   "SOUNDS/SSA/night_insects_01b.wav",
   "SOUNDS/SSA/night_insects_01c.wav",
   "SOUNDS/SSA/night_insects_01d.wav",
   // AIRPORT
   // DAY
   "SOUNDS/SSA/rotating radar dish.wav",
   "",
   "",
   "",
   // NIGHT
   "SOUNDS/SSA/rotating radar dish.wav",
   "",
   "",
   "",
   // WASTE LAND
   // DAY
   "SOUNDS/SSA/gentle_wind.wav",
   "",
   "",
   "",
   // NIGHT
   "SOUNDS/SSA/insects_at_night_04.wav",
   "",
   "",
   "",
   // UNDERGROUND
   // DAY
   "SOUNDS/SSA/low ominous ambience.wav",
   "",
   "",
   "",
   // NIGHT
   "SOUNDS/SSA/low ominous ambience.wav",
   "",
   "",
   "",
   // OCEAN
   // DAY
   "SOUNDS/SSA/sea_01a.wav",
   "SOUNDS/SSA/sea_01b.wav",
   "SOUNDS/SSA/sea_01c.wav",
   "SOUNDS/SSA/sea_01d.wav",
   // NIGHT
   "SOUNDS/SSA/ocean_waves_01a.wav",
   "SOUNDS/SSA/ocean_waves_01b.wav",
   "SOUNDS/SSA/ocean_waves_01c.wav",
   "SOUNDS/SSA/ocean_waves_01d.wav",
};


BOOLEAN LoadAmbientControlFile( UINT8 ubAmbientID )
{
	SGPFILENAME						zFilename;
	HWFILE								hFile;
	INT32								cnt;



	// BUILD FILENAME
	sprintf( zFilename, "AMBIENT/%d.bad", ubAmbientID );

	// OPEN, LOAD
	hFile = FileOpen(zFilename, FILE_ACCESS_READ);
	if ( !hFile )
	{
		return( FALSE );
	}

	// READ #
	if( !FileRead( hFile, &gsNumAmbData, sizeof( INT16 ), NULL ) )
	{
		return( FALSE );
	}

	// LOOP FOR OTHERS
	for ( cnt = 0; cnt < gsNumAmbData; cnt++ )
	{
		if( !FileRead( hFile, &(gAmbData[ cnt ]), sizeof( AMBIENTDATA_STRUCT ), NULL ) )
		{
			return( FALSE );
		}

		sprintf( zFilename, "AMBIENT/%s", gAmbData[ cnt ].zFilename );
		strcpy( gAmbData[ cnt ].zFilename, zFilename );
	}

	FileClose( hFile );

	return( TRUE );
}

void GetAmbientDataPtr( AMBIENTDATA_STRUCT **ppAmbData, UINT16 *pusNumData )
{
	*ppAmbData		= gAmbData;
	*pusNumData		= gsNumAmbData;
}


void StopAmbients( )
{
	SoundStopAllRandom( );
}

void HandleNewSectorAmbience( UINT8 ubAmbientID )
{
	// OK, we could have just loaded a sector, erase all ambient sounds from queue, shutdown all ambient groupings
	SoundStopAllRandom( );

	DeleteAllStrategicEventsOfType( EVENT_AMBIENT );

	if( !gfBasement && !gfCaves )
	{
		if(	LoadAmbientControlFile( ubAmbientID ) )
		{
			// OK, load them up!
			BuildDayAmbientSounds( );
		}
		else
		{
			DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Cannot load Ambient data for tileset" ) );
		}
	}
}

void DeleteAllAmbients()
{
	// JA2Gold: it seems that ambient sounds don't get unloaded when we exit a sector!?
	SoundStopAllRandom();
	DeleteAllStrategicEventsOfType( EVENT_AMBIENT );
}

UINT32 SetupNewAmbientSound( UINT32 uiAmbientID )
{
	RANDOMPARMS rpParms;

	memset(&rpParms, 0xff, sizeof(RANDOMPARMS));

	rpParms.uiTimeMin		=	gAmbData[ uiAmbientID ].uiMinTime;
	rpParms.uiTimeMax		=	gAmbData[ uiAmbientID ].uiMaxTime;
	rpParms.uiVolMin		= CalculateSoundEffectsVolume( gAmbData[ uiAmbientID ].uiVol );
	rpParms.uiVolMax		= CalculateSoundEffectsVolume( gAmbData[ uiAmbientID ].uiVol );

	return SoundPlayRandom( gAmbData[ uiAmbientID ].zFilename, &rpParms );
}


UINT32 StartSteadyStateAmbient( UINT32 ubVolume, UINT32 ubLoops)
{
SOUNDPARMS spParms;

	memset(&spParms, 0xff, sizeof(SOUNDPARMS));

	spParms.uiVolume = CalculateSoundEffectsVolume( ubVolume );
	spParms.uiLoop = ubLoops;

	return(SoundPlay( gSteadyStateAmbientTable[ gubCurrentSteadyStateAmbience ].zSoundNames[ gubCurrentSteadyStateSound ], &spParms ) );
}



BOOLEAN SetSteadyStateAmbience( UINT8 ubAmbience )
{
  BOOLEAN fInNight = FALSE;
  INT32     cnt;
  UINT8     ubNumSounds = 0;
  UINT8     ubChosenSound;

  // Stop all ambients...
	if ( guiCurrentSteadyStateSoundHandle != NO_SAMPLE )
	{
		SoundStop( guiCurrentSteadyStateSoundHandle );
		guiCurrentSteadyStateSoundHandle = NO_SAMPLE;
	}

  // Determine what time of day we are in ( day/night)
	if( gubEnvLightValue >= LIGHT_DUSK_CUTOFF)
	{
    fInNight = TRUE;
  }

  // loop through listing to get num sounds...
  for ( cnt = ( fInNight * 4 ); cnt < ( NUM_SOUNDS_PER_TIMEFRAME / 2 ); cnt++ )
  {
    if ( gSteadyStateAmbientTable[ ubAmbience ].zSoundNames[ cnt ][ 0 ] == 0 )
    {
      break;
    }

    ubNumSounds++;
  }

  if ( ubNumSounds == 0 )
  {
    return( FALSE );
  }

  // Pick one
  ubChosenSound = (UINT8) Random( ubNumSounds );

  // Set!
  gubCurrentSteadyStateAmbience = ubAmbience;
  gubCurrentSteadyStateSound    = ubChosenSound;

  guiCurrentSteadyStateSoundHandle =  StartSteadyStateAmbient( LOWVOLUME, 0 );

  return( TRUE );
}
