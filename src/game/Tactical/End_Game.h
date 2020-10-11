#ifndef __ENDGAME_H
#define __ENDGAME_H

void HandleStatueDamaged(INT16 sectorX, INT16 sectorY, INT8 sectorZ, INT16 sGridNo, STRUCTURE *s, UINT32 uiDist, BOOLEAN_S *skipDamage);
void ChangeO3SectorStatue( BOOLEAN fFromExplosion );

void BeginHandleDeidrannaDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );

void EndQueenDeathEndgameBeginEndCimenatic(void);
void EndQueenDeathEndgame(void);

void BeginHandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel );

#endif
