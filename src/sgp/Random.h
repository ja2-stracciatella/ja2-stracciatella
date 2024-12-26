#ifndef __RANDOM_
#define __RANDOM_

#include "Types.h"
#include <random>


extern void InitializeRandom(void);
extern UINT32 Random( UINT32 uiRange );

// Returns true 50% of the time, false 50% of the time.
bool CoinToss();

//Chance( 74 ) returns true 74% of the time.  If uiChance >= 100, then it will always return true.
extern BOOLEAN Chance( UINT32 uiChance );

//Returns a pregenerated random number.
//Used to deter Ian's tactic of shoot, miss, restore saved game :)
extern UINT32 PreRandom( UINT32 uiRange );
extern BOOLEAN PreChance( UINT32 uiChance );

//IMPORTANT:  Changing this define will invalidate the JA2 save.  If this
//						is necessary, please ifdef your own value.
#define MAX_PREGENERATED_NUMS			256
extern UINT32 guiPreRandomIndex;
extern UINT32 guiPreRandomNums[ MAX_PREGENERATED_NUMS ];

extern std::mt19937 gRandomEngine;

#endif
