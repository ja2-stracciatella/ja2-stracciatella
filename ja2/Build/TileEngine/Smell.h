#include "soldier control.h"

#define HUMAN							 0
#define CREATURE_ON_FLOOR	 0x01
#define CREATURE_ON_ROOF   0x02

#define NORMAL_HUMAN_SMELL_STRENGTH			10
#define COW_SMELL_STRENGTH							15
#define NORMAL_CREATURE_SMELL_STRENGTH	20

#define SMELL_TYPE_NUM_BITS		2
#define SMELL_TYPE( s )				(s & 0x01)
#define SMELL_STRENGTH( s )		((s & 0xFC) >> SMELL_TYPE_NUM_BITS)

#define MAXBLOODQUANTITY						7
#define BLOODDIVISOR								10

void DecaySmells( void );
void DecayBloodAndSmells( UINT32 uiTime );
void DropSmell( SOLDIERTYPE * pSoldier );
void DropBlood( SOLDIERTYPE * pSoldier, UINT8 ubStrength, INT8 bVisible );
void UpdateBloodGraphics( INT16 sGridNo, INT8 bLevel );
void RemoveBlood( INT16 sGridNo, INT8 bLevel );
void InternalDropBlood( INT16 sGridNo, INT8 bLevel, UINT8 ubType, UINT8 ubStrength, INT8 bVisible );
