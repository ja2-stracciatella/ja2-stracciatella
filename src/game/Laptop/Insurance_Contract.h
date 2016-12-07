#ifndef __INSURANCE_CONTRACT_H
#define __INSURANCE_CONTRACT_H

#include "JA2Types.h"


void EnterInsuranceContract(void);
void ExitInsuranceContract(void);
void HandleInsuranceContract(void);
void RenderInsuranceContract(void);

extern	INT16		gsCurrentInsuranceMercIndex;

// determines if a merc will run out of there insurance contract
void DailyUpdateOfInsuredMercs(void);

//void InsuranceContractPayLifeInsuranceForDeadMerc( LIFE_INSURANCE_PAYOUT *pPayoutStruct );

void AddLifeInsurancePayout(SOLDIERTYPE*);
void InsuranceContractPayLifeInsuranceForDeadMerc( UINT8	ubPayoutID );
void StartInsuranceInvestigation( UINT8	ubPayoutID );
void EndInsuranceInvestigation( UINT8	ubPayoutID );

INT32	CalculateInsuranceContractCost( INT32 iLength, UINT8 ubMercID );

void InsuranceContractEndGameShutDown(void);

void PurchaseOrExtendInsuranceForSoldier( SOLDIERTYPE *pSoldier, UINT32 uiInsuranceLength );

#endif
