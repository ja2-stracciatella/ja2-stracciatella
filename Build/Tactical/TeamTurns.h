#ifndef TEAMTURNS_H
#define TEAMTURNS_H

#include "JA2Types.h"


extern UINT8 gubOutOfTurnPersons;
extern BOOLEAN gfHiddenInterrupt;
extern BOOLEAN gfHiddenTurnbased;

#define INTERRUPT_QUEUED (gubOutOfTurnPersons > 0)

BOOLEAN StandardInterruptConditionsMet(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pOpponent, INT8 bOldOppList);
INT8 CalcInterruptDuelPts(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* opponent, BOOLEAN fUseWatchSpots);
extern void EndAITurn( void );
extern void DisplayHiddenInterrupt( SOLDIERTYPE * pSoldier );
extern BOOLEAN InterruptDuel( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pOpponent);
void AddToIntList(SOLDIERTYPE* s, BOOLEAN fGainControl, BOOLEAN fCommunicate);
void DoneAddingToIntList(void);

void ClearIntList( void );

void SaveTeamTurnsToTheSaveGameFile(HWFILE);

void LoadTeamTurnsFromTheSavedGameFile(HWFILE);

void EndAllAITurns( void );

BOOLEAN NPCFirstDraw( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pTargetSoldier );

void SayCloseCallQuotes(void);

void DisplayHiddenTurnbased(SOLDIERTYPE* pActingSoldier);

#endif
