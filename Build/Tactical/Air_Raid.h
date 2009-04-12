#ifndef AIR_RAID_H
#define AIR_RAID_H

#include "Types.h"

#define AIR_RAID_BEGINNING_GAME								0x00000001
#define AIR_RAID_CAN_RANDOMIZE_TEASE_DIVES		0x00000002


struct AIR_RAID_DEFINITION
{
	INT16		sSectorX;
	INT16		sSectorY;
	INT16		sSectorZ;
	INT8		bIntensity;
	UINT32	uiFlags;
	UINT8		ubNumMinsFromCurrentTime;
	UINT8		ubFiller[8]; // XXX HACK000B
};
CASSERT(sizeof(AIR_RAID_DEFINITION) == 24);


extern BOOLEAN gfInAirRaid;

// what ari raid mode are we in?
extern UINT8 gubAirRaidMode;

enum AIR_RAID_STATES
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
};


void HandleAirRaid(void);

BOOLEAN BeginAirRaid(void);
BOOLEAN InAirRaid(void);

BOOLEAN HandleAirRaidEndTurn(UINT8 ubTeam);


//Save the air raid info to the saved game
void SaveAirRaidInfoToSaveGameFile(HWFILE);

//load the air raid info from the saved game
void LoadAirRaidInfoFromSaveGameFile(HWFILE);

void EndAirRaid(void);


#endif
