#ifndef AIR_RAID_H
#define AIR_RAID_H

#include "Types.h"

#define AIR_RAID_BEGINNING_GAME								0x00000001
#define AIR_RAID_CAN_RANDOMIZE_TEASE_DIVES		0x00000002


typedef struct
{
	INT16		sSectorX;
	INT16		sSectorY;
	INT16		sSectorZ;
	INT8		bIntensity;
	UINT32	uiFlags;
	UINT8		ubNumMinsFromCurrentTime;
	UINT8		ubFiller[8];

} AIR_RAID_DEFINITION;


extern BOOLEAN gfInAirRaid;

// what ari raid mode are we in?
extern UINT8 gubAirRaidMode;

enum
{
	AIR_RAID_TRYING_TO_START,
	AIR_RAID_START,
	AIR_RAID_LOOK_FOR_DIVE,
	AIR_RAID_BEGIN_DIVE,
	AIR_RAID_DIVING,
	AIR_RAID_END_DIVE,
	AIR_RAID_BEGIN_BOMBING,
	AIR_RAID_BOMBING,
	AIR_RAID_END_BOMBING,
	AIR_RAID_START_END,
	AIR_RAID_END

} AIR_RAID_STATES;


void HandleAirRaid( );

BOOLEAN BeginAirRaid( );
BOOLEAN InAirRaid( );

BOOLEAN HandleAirRaidEndTurn(UINT8 ubTeam);


//Save the air raid info to the saved game
BOOLEAN SaveAirRaidInfoToSaveGameFile( HWFILE hFile );


//load the air raid info from the saved game
BOOLEAN LoadAirRaidInfoFromSaveGameFile( HWFILE hFile );

void EndAirRaid( );


#endif
