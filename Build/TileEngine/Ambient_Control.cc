#include "Ambient_Types.h"
#include "FileMan.h"
#include "Environment.h"
#include "Sound_Control.h"
#include "Game_Events.h"
#include "Ambient_Control.h"
#include "SoundMan.h"
#include "Debug.h"


AMBIENTDATA_STRUCT		gAmbData[ MAX_AMBIENT_SOUNDS ];
INT16									gsNumAmbData = 0;


static BOOLEAN LoadAmbientControlFile(UINT8 ubAmbientID)
{
	BOOLEAN Ret = FALSE;

	SGPFILENAME zFilename;
	sprintf(zFilename, "AMBIENT/%d.bad", ubAmbientID);

	HWFILE hFile = FileOpen(zFilename, FILE_ACCESS_READ);
	if (!hFile) goto ret_only;

	// READ #
	if (!FileRead(hFile, &gsNumAmbData, sizeof(INT16))) goto ret_close;

	// LOOP FOR OTHERS
	for (INT32 cnt = 0; cnt < gsNumAmbData; cnt++)
	{
		if (!FileRead(hFile, &gAmbData[cnt], sizeof(AMBIENTDATA_STRUCT))) goto ret_close;

		sprintf(zFilename, "AMBIENT/%s", gAmbData[cnt].zFilename);
		strcpy(gAmbData[cnt].zFilename, zFilename);
	}

	Ret = TRUE;

ret_close:
	FileClose(hFile);
ret_only:
	return Ret;
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
			DebugMsg(TOPIC_JA2, DBG_LEVEL_0, "Cannot load Ambient data for tileset");
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
