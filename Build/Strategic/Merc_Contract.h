#ifndef _MERC_CONTRACT_H_
#define _MERC_CONTRACT_H_

#include "JA2Types.h"


//enums used for extending contract, etc.
enum
{
	CONTRACT_EXTEND_1_DAY,
	CONTRACT_EXTEND_1_WEEK,
	CONTRACT_EXTEND_2_WEEK,
};


extern BOOLEAN									gfContractRenewalSquenceOn;
extern BOOLEAN									gfInContractMenuFromRenewSequence;


/*

//list of quotes used in renewing a mercs contract
enum
{
	LAME_REFUSAL_DOING_SOMETHING_ELSE = 73,
	DEPARTING_COMMENT_AFTER_48_HOURS	= 75,
	CONTRACTS_OVER_U_EXTENDING = 79,
	ACCEPT_CONTRACT_RENEWAL = 80,
	REFUSAL_TO_RENEW_POOP_MORALE = 85,
	DEPARTING_COMMENT_BEFORE_48_HOURS=88,
	DEATH_RATE_REFUSAL=89,
	HATE_MERC_1_ON_TEAM,
	HATE_MERC_2_ON_TEAM,
	LEARNED_TO_HATE_MERC_ON_TEAM,
	JOING_CAUSE_BUDDY_1_ON_TEAM,
	JOING_CAUSE_BUDDY_2_ON_TEAM,
	JOING_CAUSE_LEARNED_TO_LIKE_BUDDY_ON_TEAM,
	PRECEDENT_TO_REPEATING_ONESELF,
	REFUSAL_DUE_TO_LACK_OF_FUNDS,
};
*/

BOOLEAN	MercContractHandling( SOLDIERTYPE	*pSoldier, UINT8 ubDesiredAction );

void StrategicRemoveMerc(SOLDIERTYPE* pSoldier);
void BeginStrategicRemoveMerc(SOLDIERTYPE* pSoldier, BOOLEAN fAddRehireButton);


BOOLEAN WillMercRenew( SOLDIERTYPE	*pSoldier, BOOLEAN fSayQuote );
void CheckIfMercGetsAnotherContract( SOLDIERTYPE *pSoldier );
void FindOutIfAnyMercAboutToLeaveIsGonnaRenew( void );

void BeginContractRenewalSequence(void);
void HandleContractRenewalSequence(void);

UINT32 GetHourWhenContractDone( SOLDIERTYPE *pSoldier );

void LoadContractRenewalDataFromSaveGameFile(HWFILE);
void SaveContractRenewalDataToSaveGameFile(HWFILE);


// rehiring of mercs from leave equipment pop up
extern BOOLEAN	fEnterMapDueToContract;
extern SOLDIERTYPE *pContractReHireSoldier;

#endif
