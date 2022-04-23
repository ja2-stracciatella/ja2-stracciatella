#ifndef _TOWN_MILITIA_H
#define _TOWN_MILITIA_H

// header for town militia strategic control module

#include "JA2Types.h"

#include <string_theory/string>


// how many militia of all ranks can be in any one sector at once
#define MAX_ALLOWABLE_MILITIA_PER_SECTOR 20

// how many new green militia civilians are trained at a time
#define MILITIA_TRAINING_SQUAD_SIZE		10		// was 6

// cost of starting a new militia training assignment
#define MILITIA_TRAINING_COST		750

// minimum loyalty rating before training is allowed in a town
#define MIN_RATING_TO_TRAIN_TOWN 20



// this handles what happens when a new militia unit is finishes getting trained
void TownMilitiaTrainingCompleted(SOLDIERTYPE *pTrainer, const SGPSector& sector);

// Given a SOLDIER_CLASS_ returns a _MITILIA rank or -1 if it is not militia
INT8 SoldierClassToMilitiaRank(UINT8 soldier_class);

// remove militias of a certain rank
void StrategicRemoveMilitiaFromSector(const SGPSector& sMap, UINT8 ubRank, UINT8 ubHowMany);

// Check for promotions and handle them
UINT8 CheckOneMilitiaForPromotion(const SGPSector& sMap, UINT8 &current_rank, UINT8 kill_points);

ST::string BuildMilitiaPromotionsString();

UINT8 CountAllMilitiaInSector(const SGPSector& sMap);
UINT8 MilitiaInSectorOfRank(const SGPSector& sMap, UINT8 ubRank);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in combat mode
BOOLEAN SectorOursAndPeaceful(const SGPSector& sector);

// tell player how much it will cost
void HandleInterfaceMessageForCostOfTrainingMilitia( SOLDIERTYPE *pSoldier );

// call this when the sector changes...
void HandleMilitiaStatusInCurrentMapBeforeLoadingNewMap( void );

// Is there a town with militia here or nearby?
bool CanNearbyMilitiaScoutThisSector(const SGPSector& sMap);

// Is the town or SAM site here full of milita?
BOOLEAN IsAreaFullOfMilitia(const SGPSector& sector);

// now that town training is complete, handle the continue boxes
void HandleContinueOfTownTraining( void );

// clear the list of training completed sectors
void ClearSectorListForCompletedTrainingOfMilitia();

BOOLEAN MilitiaTrainingAllowedInSector(const SGPSector& sMap);
BOOLEAN MilitiaTrainingAllowedInTown( INT8 bTownId );

void AddSectorForSoldierToListOfSectorsThatCompletedMilitiaTraining(SOLDIERTYPE* pSoldier);

extern SOLDIERTYPE* pMilitiaTrainerSoldier;

#endif
