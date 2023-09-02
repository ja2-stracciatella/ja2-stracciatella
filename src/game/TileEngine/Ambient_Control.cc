#include "Ambient_Types.h"
#include "Directories.h"
#include "Environment.h"
#include "Game_Event_Hook.h"
#include "Sound_Control.h"
#include "Ambient_Control.h"
#include "SGPFile.h"
#include "SoundMan.h"
#include "Debug.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"
#include <string_theory/format>
#include <string_theory/string>


static BOOLEAN LoadAmbientControlFile(UINT8 ubAmbientID)
try
{
	AutoSGPFile hFile{GCM->openGameResForReading(
		ST::format("{}/{}.bad", AMBIENTDIR, ubAmbientID))};

	// READ #
	hFile->read(&gsNumAmbData, sizeof(INT16));
	Assert(gsNumAmbData <= MAX_AMBIENT_SOUNDS && gsNumAmbData >= 0);

	// Read all ambient data
	hFile->read(gAmbData, sizeof(AMBIENTDATA_STRUCT) * gsNumAmbData);

	return TRUE;
}
catch (...) { return FALSE; }


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
			SLOGW("Cannot load Ambient data for tileset");
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
	if (static_cast<INT16>(uiAmbientID) >= gsNumAmbData)
	{
		return NO_SAMPLE;
	}

	const AMBIENTDATA_STRUCT* const a   = &gAmbData[uiAmbientID];
	const UINT32                    vol = CalculateSoundEffectsVolume(a->uiVol);
	const auto filename = ST::format("{}/{}", AMBIENTDIR, a->zFilename);

	return SoundPlayRandom(filename.c_str(), a->uiMinTime, a->uiMaxTime, vol, vol, MIDDLEPAN, MIDDLEPAN, 1);
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(AmbientControl, asserts)
{
	EXPECT_EQ(sizeof(AMBIENTDATA_STRUCT), 116u);
}

#endif
