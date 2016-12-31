#include "Random.h"
#include <stdlib.h>
#include <time.h>

UINT32 guiPreRandomIndex = 0;
UINT32 guiPreRandomNums[ MAX_PREGENERATED_NUMS ];

void InitializeRandom(void)
{
	// Seed the random-number generator with current time so that
	// the numbers will be different every time we run.
	srand( (unsigned) time(NULL) );
	//Pregenerate all of the random numbers.
	for( guiPreRandomIndex = 0; guiPreRandomIndex < MAX_PREGENERATED_NUMS; guiPreRandomIndex++ )
	{
		guiPreRandomNums[ guiPreRandomIndex ] = rand();
	}
	guiPreRandomIndex = 0;
}

// Returns a pseudo-random integer between 0 and uiRange
UINT32 Random(UINT32 uiRange)
{
	UINT32 x;
	// Always return 0, if no range given (it's not an error)
	if (uiRange == 0)
		return(0);
	/* Ensures a correct average value by actually limiting the possible
	 * set of values to the largest multiple of uiRange and
	 * discarding [largest multiple of uiRange beneath RAND_MAX,RAND_MAX].
	 * The rather complex limitation ensures a correct behaviour even
	 * for very large (close to RAND_MAX) values of uiRange.
	 */
	do { x = rand(); } while ( x >= (((RAND_MAX - uiRange + 1)/uiRange+1)*uiRange));
	return x % uiRange;
}

BOOLEAN Chance( UINT32 uiChance )
{
	return Random(100) < uiChance;
}

UINT32 PreRandom( UINT32 uiRange )
{
	UINT32 uiNum;
	if( !uiRange )
		return 0;
	//Extract the current pregenerated number
	/* HACK0007 Stop PreRandom always returning 0 or 1
	 * without ensuring an equal distribution, which
	 * would be a rather complex task with pregenerated randoms
	 */
	uiNum = guiPreRandomNums[ guiPreRandomIndex ] % uiRange;
	//Replace the current pregenerated number with a new one.

	//This was removed in the name of optimization.  Uncomment if you hate recycling.
	//guiPreRandomNums[ guiPreRandomIndex ] = rand();

	//Go to the next index.
	guiPreRandomIndex++;
	if( guiPreRandomIndex >= (UINT32)MAX_PREGENERATED_NUMS )
		guiPreRandomIndex = 0;
	return uiNum;
}

BOOLEAN PreChance( UINT32 uiChance )
{
	return PreRandom(100) < uiChance;
}
