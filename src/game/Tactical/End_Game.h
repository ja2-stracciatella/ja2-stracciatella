#ifndef __ENDGAME_H
#define __ENDGAME_H


BOOLEAN DoesO3SectorStatueExistHere( INT16 sGridNo );
void ChangeO3SectorStatue( BOOLEAN fFromExplosion );

void BeginHandleDeidrannaDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );

void EndQueenDeathEndgameBeginEndCimenatic(void);
void EndQueenDeathEndgame(void);

void BeginHandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );

#endif
