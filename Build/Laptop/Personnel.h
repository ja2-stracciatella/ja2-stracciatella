#ifndef __PERSONNEL_H
#define __PERSONNEL_H

#include "JA2Types.h"


void GameInitPersonnel(void);
void EnterPersonnel(void);
void ExitPersonnel(void);
void HandlePersonnel(void);
void RenderPersonnel(void);


// add character to:

// leaving for odd reasons
void AddCharacterToOtherList( SOLDIERTYPE *pSoldier );

// killed and removed
void AddCharacterToDeadList( SOLDIERTYPE *pSoldier );

// simply fired...but alive
void AddCharacterToFiredList( SOLDIERTYPE *pSoldier );

BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList( UINT8 ubProfile );

#endif
