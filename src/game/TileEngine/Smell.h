#pragma once

#include "JA2Types.h"


enum BloodKind
{
	HUMAN             = 0,
	CREATURE_ON_FLOOR = 1,
	CREATURE_ON_ROOF  = 2
};

#define NORMAL_HUMAN_SMELL_STRENGTH			10
#define COW_SMELL_STRENGTH							15
#define NORMAL_CREATURE_SMELL_STRENGTH	20

#define SMELL_TYPE_NUM_BITS		2
#define SMELL_TYPE( s )				(s & 0x01)
#define SMELL_STRENGTH( s )		((s & 0xFC) >> SMELL_TYPE_NUM_BITS)

#define MAXBLOODQUANTITY						7
#define BLOODDIVISOR								10

void DecaySmells();
void DecayBloodAndSmells( UINT32 uiTime );
void DropSmell(SOLDIERTYPE&);
void DropBlood(SOLDIERTYPE const&, UINT8 strength);
void UpdateBloodGraphics(GridNo, INT8 level);
void RemoveBlood(GridNo, INT8 level);
void InternalDropBlood(GridNo, INT8 level, BloodKind, UINT8 strength, INT8 visible);
