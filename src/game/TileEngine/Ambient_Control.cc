#include "Ambient_Types.h"
#include "Directories.h"
#include "FileMan.h"
#include "Environment.h"
#include "Sound_Control.h"
#include "Game_Events.h"
#include "Ambient_Control.h"
#include "SoundMan.h"
#include "Debug.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"
#include <string_theory/format>
#include <string_theory/string>

AMBIENTDATA_STRUCT		gAmbData[ MAX_AMBIENT_SOUNDS ];
INT16									gsNumAmbData = 0;


static BOOLEAN LoadAmbientControlFile(UINT8 ubAmbientID)
try
{
	ST::string zFilename = ST::format("{}/{}.bad", AMBIENTDIR, ubAmbientID);
	AutoSGPFile hFile(GCM->openGameResForReading(zFilename));

	// READ #
	hFile->read(&gsNumAmbData, sizeof(INT16));

	// LOOP FOR OTHERS
	for (INT32 cnt = 0; cnt < gsNumAmbData; cnt++)
	{
		hFile->read(&gAmbData[cnt], sizeof(AMBIENTDATA_STRUCT));

		zFilename = ST::format("{}/{}", AMBIENTDIR, gAmbData[cnt].zFilename);
		if (zFilename.size() > SGPFILENAME_LEN) throw std::runtime_error("ambient file name too long");
		strcpy(gAmbData[cnt].zFilename, zFilename.c_str());
	}

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
	const AMBIENTDATA_STRUCT* const a   = &gAmbData[uiAmbientID];
	const UINT32                    vol = CalculateSoundEffectsVolume(a->uiVol);
	return SoundPlayRandom(a->zFilename, a->uiMinTime, a->uiMaxTime, vol, vol, MIDDLEPAN, MIDDLEPAN, 1);
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(AmbientControl, asserts)
{
	EXPECT_EQ(sizeof(AMBIENTDATA_STRUCT), 116u);
}

#endif
