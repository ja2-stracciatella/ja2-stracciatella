#ifndef __ENDGAME_H
#define __ENDGAME_H


BOOLEAN DoesO3SectorStatueExistHere( INT16 sGridNo );
void ChangeO3SectorStatue( BOOLEAN fFromExplosion );

void HandleDeidrannaDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );
void BeginHandleDeidrannaDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );

void HandleDoneLastKilledQueenQuote( );

void EndQueenDeathEndgameBeginEndCimenatic( );
void EndQueenDeathEndgame( );

void HandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );
void BeginHandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );

void HandleDoneLastEndGameQuote( );


#endif
