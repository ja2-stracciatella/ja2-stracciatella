#if defined DUTCH || defined ENGLISH || defined FRENCH || defined ITALIAN || defined POLISH

#include "_JA25EnglishText.h"

// VERY TRUNCATED FILE COPIED FROM JA2.5 FOR ITS FEATURES FOR JA2 GOLD

const wchar_t* const zNewTacticalMessages[] =
{
	L"Range to target: %d tiles",
	L"Gun Range: %d tiles, Range to target: %d tiles",
	L"Display Cover",
	L"Line of Sight",
	L"This is an IRON MAN game and you cannot save when enemies are around.",	//	@@@  new text
	L"(Cannot save during combat)", //@@@@ new text
};

//@@@:  New string as of March 3, 2000.
const wchar_t* const gzIronManModeWarningText[] =
{
	L"You have chosen IRON MAN mode. This setting makes the game considerably more challenging as you will not be able to save your game when in a sector occupied by enemies. This setting will affect the entire course of the game.  Are you sure want to play in IRON MAN mode?",
};

#endif
