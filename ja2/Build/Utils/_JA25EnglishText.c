#include "Language_Defines.h"
#ifdef ENGLISH
#	include "FileMan.h"
#endif


#if defined(DUTCH) || defined(ENGLISH) || defined(FRENCH) || defined(ITALIAN) || defined(POLISH) || defined(RUSSIAN)

#include "Text.h"

// VERY TRUNCATED FILE COPIED FROM JA2.5 FOR ITS FEATURES FOR JA2 GOLD

const wchar_t *zNewTacticalMessages[]=
{
	L"Range to target: %d tiles",
	L"Attaching the transmitter to your laptop computer.",
	L"You cannot afford to hire %s",
	L"For a limited time, the above fee covers the cost of the entire mission and includes the equipment listed below.",
	L"Hire %s now and take advantage of our unprecedented 'one fee covers all' pricing.  Also included in this unbelievable offer is the mercenary's personal equipment at no charge.",
	L"Fee",
	L"There is someone else in the sector...",
	L"Gun Range: %d tiles, Range to target: %d tiles",
	L"Display Cover",
	L"Line of Sight",
	L"New Recruits cannot arrive there.",
	L"Since your laptop has no transmitter, you won't be able to hire new team members.  Perhaps this would be a good time to load a saved game or start over!",
	L"%s hears the sound of crumpling metal coming from underneath Jerry's body.  It sounds disturbingly like your laptop antenna being crushed.",  //the %s is the name of a merc.  @@@  Modified
	L"After scanning the note left behind by Deputy Commander Morris, %s senses an oppurtinity.  The note contains the coordinates for launching missiles against different towns in Arulco.  It also gives the coodinates of the origin - the missile facility.",
	L"Noticing the control panel, %s figures the numbers can be reveresed, so that the missile might destroy this very facility.  %s needs to find an escape route.  The elevator appears to offer the fastest solution...",
	L"This is an IRON MAN game and you cannot save when enemies are around.",	//	@@@  new text
	L"(Cannot save during combat)", //@@@@ new text
	L"The current campaign name is greater than 30 characters.",							// @@@ new text
	L"The current campaign cannot be found.",																	// @@@ new text
	L"Campaign: Default ( %ls )",																							// @@@ new text
	L"Campaign: %ls",																													// @@@ new text
	L"You have selected the campaign %ls. This campaign is a player-modified version of the original Unfinished Business campaign. Are you sure you wish to play the %ls campaign?",			// @@@ new text
	L"In order to use the editor, please select a campaign other than the default.",		///@@new
};

//@@@:  New string as of March 3, 2000.
const wchar_t *gzIronManModeWarningText[]=
{
	L"You have chosen IRON MAN mode. This setting makes the game considerably more challenging as you will not be able to save your game when in a sector occupied by enemies. This setting will affect the entire course of the game.  Are you sure want to play in IRON MAN mode?",
};


#endif
