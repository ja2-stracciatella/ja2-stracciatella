#ifndef __PERSONNEL_H
#define __PERSONNEL_H


#include "Types.h"
#include "Soldier Control.h"


// delay for change in ATM mode
#define DELAY_PER_MODE_CHANGE_IN_ATM 2000


void GameInitPersonnel();
void EnterPersonnel();
void ExitPersonnel();
void HandlePersonnel();
void RenderPersonnel();


// add character to:

// leaving for odd reasons
void AddCharacterToOtherList( SOLDIERTYPE *pSoldier );

// killed and removed
void AddCharacterToDeadList( SOLDIERTYPE *pSoldier );

// simply fired...but alive
void AddCharacterToFiredList( SOLDIERTYPE *pSoldier );

// get the total amt of money on this guy
INT32 GetFundsOnMerc( SOLDIERTYPE *pSoldier );

BOOLEAN TransferFundsFromMercToBank( SOLDIERTYPE *pSoldier, INT32 iCurrentBalance );
BOOLEAN TransferFundsFromBankToMerc( SOLDIERTYPE *pSoldier, INT32 iCurrentBalance );

BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList( UINT8 ubProfile );

#endif
