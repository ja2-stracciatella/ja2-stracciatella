#ifndef _STRATEGIC_TOWN_REPUTATION_H
#define _STRATEGIC_TOWN_REPUTATION_H

#include "Types.h"
#include "Soldier_Control.h"
// header to contain functions to handle strategic town reputation


// initialize profiles for the town reputation system
void InitializeProfilesForTownReputation( void );

// post events to handle spread of town loyalty
void PostEventsForSpreadOfTownOpinion( void );

// get the towns opinion of this merc...indexed by profile type
UINT8 GetTownOpinionOfMerc( UINT8 ubProfileId, UINT8 ubTownId );
UINT8 GetTownOpinionOfMercForSoldier( SOLDIERTYPE *pSoldier, UINT8 ubTownId );

// update merc reputation for this town by this amount
void UpdateTownOpinionOfThisMerc( UINT8 ubProfileId, UINT8 ubTownId, INT8 bAmount );
void UpdateTownOpinionOfThisMercForSoldier( SOLDIERTYPE *pSoldier, UINT8 ubTownId, INT8 bAmount );

// global handling of spread
void HandleSpreadOfAllTownsOpinion( void );
// handles the spread of town opinion for one merc
void HandleSpreadOfTownOpinionForMerc( UINT8 ubProfileId );
// handles the opinion spread between these two towns about this merc at the distance between them
void HandleOpinionOfTownsAboutSoldier( INT8 bTownA, INT8 bTownB, INT32 iDistanceBetweenThem, UINT8 ubProfile );

/*
// handle only for mercs on players team
void HandleSpreadOfTownsOpinionForCurrentMercs( void );
void HandleSpreadOfTownOpinionForMercForSoldier( SOLDIERTYPE *pSoldier );
*/

#endif
