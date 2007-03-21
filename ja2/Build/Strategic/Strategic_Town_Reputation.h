#ifndef _STRATEGIC_TOWN_REPUTATION_H
#define _STRATEGIC_TOWN_REPUTATION_H

#include "Types.h"
#include "Soldier_Control.h"
// header to contain functions to handle strategic town reputation


// initialize profiles for the town reputation system
void InitializeProfilesForTownReputation( void );

// post events to handle spread of town loyalty
void PostEventsForSpreadOfTownOpinion( void );

// global handling of spread
void HandleSpreadOfAllTownsOpinion( void );

#endif
