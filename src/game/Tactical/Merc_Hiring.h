#ifndef _MERC_HIRING_H_
#define _MERC_HIRING_H_

#include "JA2Types.h"
#include "Item_Types.h"


//
// Used with the HireMerc function
//
#define MERC_HIRE_OVER_20_MERCS_HIRED		-1
#define MERC_HIRE_FAILED			0
#define MERC_HIRE_OK				1

#define MERC_ARRIVE_TIME_SLOT_1		( 7 * 60 + 30 ) // 7:30 a.m.
#define MERC_ARRIVE_TIME_SLOT_2		( 13 * 60 + 30 ) // 1:30 pm
#define MERC_ARRIVE_TIME_SLOT_3		( 19 * 60 + 30 ) // 7:30 pm


// ATE: This define has been moved to be a function so that
// we pick the most appropriate time of day to use...
//#define MERC_ARRIVAL_TIME_OF_DAY		(7 * 60 + 30) // 7:30 am


struct MERC_HIRE_STRUCT
{
	UINT8   ubProfileID;
	SGPSector sSector;
	INT16   iTotalContractLength;
	BOOLEAN fCopyProfileItemsOver;
	UINT32  uiTimeTillMercArrives;
	UINT8   ubInsertionCode;
	UINT16  usInsertionData;
	BOOLEAN fUseLandingZoneForArrival;
	INT8    bWhatKindOfMerc = -1;
};

// ATE: Global that dictates where the mercs will land once being hired
// Default to start sector
// Saved in general saved game structure
extern SGPSector g_merc_arrive_sector;


void CreateSpecialItem(SOLDIERTYPE* const, ItemId index);
INT8 HireMerc(MERC_HIRE_STRUCT&);
void MercArrivesCallback(SOLDIERTYPE&);
bool IsMercHireable(MERCPROFILESTRUCT const&);
bool IsMercDead(MERCPROFILESTRUCT const&);
void HandleMercArrivesQuotes(SOLDIERTYPE&);
void UpdateAnyInTransitMercsWithGlobalArrivalSector(void);


UINT32 GetMercArrivalTimeOfDay(void);

#endif
