#ifndef _STRATEGIC_MERC_HANDLER_H_
#define _STRATEGIC_MERC_HANDLER_H_

#include "JA2Types.h"

void StrategicHandlePlayerTeamMercDeath(SOLDIERTYPE&);
void MercDailyUpdate(void);
void MercsContractIsFinished(SOLDIERTYPE* s);
void RPCWhineAboutNoPay(SOLDIERTYPE&);
void MercComplainAboutEquipment(ProfileID);
BOOLEAN SoldierHasWorseEquipmentThanUsedTo( SOLDIERTYPE *pSoldier );
void UpdateBuddyAndHatedCounters( void );
void HourlyCamouflageUpdate();
#endif
