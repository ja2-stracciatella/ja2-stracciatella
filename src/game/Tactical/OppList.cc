#include "Font.h"
#include "AI.h"
#include "Debug_Pages.h"
#include "Isometric_Utils.h"
#include "ItemModel.h"
#include "Overhead.h"
#include "Event_Pump.h"
#include "Random.h"
#include "Overhead_Types.h"
#include "OppList.h"
#include "AI.h"
#include "Animation_Control.h"
#include "LOS.h"
#include "FOV.h"
#include "Dialogue_Control.h"
#include "Lighting.h"
#include "Environment.h"
#include "Interface_Dialogue.h"
#include "Message.h"
#include "Soldier_Profile.h"
#include "Structure.h"
#include "TeamTurns.h"
#include "Interactive_Tiles.h"
#include "Render_Fun.h"
#include "Text.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "Soldier_Macros.h"
#include "Soldier_Functions.h"
#include "Handle_UI.h"
#include "Keys.h"
#include "Campaign.h"
#include "Soldier_Init_List.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Meanwhile.h"
#include "WorldMan.h"
#include "SkillCheck.h"
#include "SmokeEffects.h"
#include "Smell.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "Civ_Quotes.h"
#include "Sound_Control.h"
#include "Drugs_And_Alcohol.h"
#include "Debug.h"
#include "Items.h"
#include "GameRes.h"
#include "MercProfile.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>


static const BOOLEAN gbShowEnemies = FALSE; // XXX seems to be a debug switch

// for ManLooksForMan()
#define MANLOOKSFOROTHERTEAMS			0
#define OTHERTEAMSLOOKFORMAN			1
#define VERIFYANDDECAYOPPLIST			2
#define HANDLESTEPPEDLOOKAT			3
#define LOOKANDLISTEN				4
#define UPDATEPUBLIC				5
#define CALLER_UNKNOWN				6

// this variable is a flag used in HandleSight to determine whether (while in non-combat RT)
// someone has just been seen, EITHER THE MOVER OR SOMEONE THE MOVER SEES
static BOOLEAN gfPlayerTeamSawCreatures = FALSE;
BOOLEAN gfPlayerTeamSawJoey = FALSE;
BOOLEAN gfMikeShouldSayHi = FALSE;

static SOLDIERTYPE* gBestToMakeSighting[BEST_SIGHTING_ARRAY_SIZE];
UINT8 gubBestToMakeSightingSize = 0;
//BOOLEAN gfHumanSawSomeoneInRealtime;

static BOOLEAN gfDelayResolvingBestSightingDueToDoor = FALSE;

#define SHOULD_BECOME_HOSTILE_SIZE 32

static SOLDIERTYPE* gShouldBecomeHostileOrSayQuote[SHOULD_BECOME_HOSTILE_SIZE];
static UINT8 gubNumShouldBecomeHostileOrSayQuote;

// NB this ID is set for someone opening a door
SOLDIERTYPE* gInterruptProvoker = NULL;

INT8 gbPublicOpplist[MAXTEAMS][TOTAL_SOLDIERS];
INT8 gbSeenOpponents[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
INT16 gsLastKnownOppLoc[TOTAL_SOLDIERS][TOTAL_SOLDIERS]; // merc vs. merc
INT8 gbLastKnownOppLevel[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
INT16 gsPublicLastKnownOppLoc[MAXTEAMS][TOTAL_SOLDIERS]; // team vs. merc
INT8 gbPublicLastKnownOppLevel[MAXTEAMS][TOTAL_SOLDIERS];
UINT8 gubPublicNoiseVolume[MAXTEAMS];
INT16 gsPublicNoiseGridno[MAXTEAMS];
INT8	gbPublicNoiseLevel[MAXTEAMS];

UINT8 gubKnowledgeValue[10][10] =
{
	//   P E R S O N A L   O P P L I S T  //
	// -4   -3   -2   -1   0   1   2   3   4   5   //
	{   0,   1,   2,   3,  0,  5,  4,  3,  2,  1}, // -4
	{   0,   0,   1,   2,  0,  4,  3,  2,  1,  0}, // -3    O
	{   0,   0,   0,   1,  0,  3,  2,  1,  0,  0}, // -2  P P
	{   0,   0,   0,   0,  0,  2,  1,  0,  0,  0}, // -1  U P
	{   0,   1,   2,   3,  0,  5,  4,  3,  2,  1}, //  0  B L
	{   0,   0,   0,   0,  0,  0,  0,  0,  0,  0}, //  1  L I
	{   0,   0,   0,   0,  0,  1,  0,  0,  0,  0}, //  2  I S
	{   0,   0,   0,   1,  0,  2,  1,  0,  0,  0}, //  3  C T
	{   0,   0,   1,   2,  0,  3,  2,  1,  0,  0}, //  4
	{   0,   1,   2,   3,  0,  4,  3,  2,  1,  0}  //  5

	/*
	//   P E R S O N A L   O P P L I S T  //
	// -3   -2   -1   0   1   2   3   4   //
	{   0,   1,   2,  0,  4,  3,  2,  1   }, // -3    O
	{   0,   0,   1,  0,  3,  2,  1,  0   }, // -2  P P
	{   0,   0,   0,  0,  2,  1,  0,  0   }, // -1  U P
	{   1,   2,   3,  0,  5,  4,  3,  2   }, //  0  B L
	{   0,   0,   0,  0,  0,  0,  0,  0   }, //  1  L I
	{   0,   0,   0,  0,  1,  0,  0,  0   }, //  2  I S
	{   0,   0,   1,  0,  2,  1,  0,  0   }, //  3  C T
	{   0,   1,   2,  0,  3,  2,  1,  0   }  //  4
	*/
};

#define MAX_WATCHED_LOC_POINTS			4
#define WATCHED_LOC_RADIUS			1

INT16 gsWatchedLoc[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
INT8 gbWatchedLocLevel[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
UINT8 gubWatchedLocPoints[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
BOOLEAN gfWatchedLocReset[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
static BOOLEAN gfWatchedLocHasBeenIncremented[TOTAL_SOLDIERS][NUM_WATCHED_LOCS];

static const INT8 gbLookDistance[8][8] =
{
	//  LOOKER DIR       LOOKEE DIR
	//                 NORTH  | NORTHEAST |  EAST   | SOUTHEAST |  SOUTH  | SOUTHWEST |  WEST   | NORTHWEST
	/* NORTH     */ { STRAIGHT,      ANGLE,     SIDE,    SBEHIND,   BEHIND,    SBEHIND,     SIDE,     ANGLE },
	/* NORTHEAST */ { ANGLE,      STRAIGHT,    ANGLE,       SIDE,  SBEHIND,     BEHIND,  SBEHIND,      SIDE },
	/* EAST      */ { SIDE,          ANGLE, STRAIGHT,      ANGLE,     SIDE,    SBEHIND,   BEHIND,   SBEHIND },
	/* SOUTHEAST */ { SBEHIND,        SIDE,    ANGLE,   STRAIGHT,    ANGLE,       SIDE,  SBEHIND,    BEHIND },
	/* SOUTH     */ { BEHIND,      SBEHIND,     SIDE,      ANGLE, STRAIGHT,      ANGLE,     SIDE,   SBEHIND },
	/* SOUTHWEST */ { SBEHIND,      BEHIND,  SBEHIND,       SIDE,    ANGLE,   STRAIGHT,    ANGLE,      SIDE },
	/* WEST      */ { SIDE,        SBEHIND,   BEHIND,    SBEHIND,     SIDE,      ANGLE, STRAIGHT,     ANGLE },
	/* NORTHWEST */ { ANGLE,          SIDE,  SBEHIND,     BEHIND,  SBEHIND,       SIDE,    ANGLE,  STRAIGHT }
};

const SOLDIERTYPE* gWhoThrewRock = NULL;

#define NIGHTSIGHTGOGGLES_BONUS		2
#define UVGOGGLES_BONUS			4

// % values of sighting distance at various light levels

INT8 gbLightSighting[1][16] =
{{
	// human
	80, // brightest
	86,
	93,
	100, // normal daylight, 3
	94,
	88,
	82,
	76,
	70, // mid-dawn, 8
	64,
	58,
	51,
	43, // normal nighttime, 12 (11 tiles)
	30,
	17,
	9
}};
/*
{{
	// human
	80, // brightest
	86,
	93,
	100, // normal daylight, 3
	93,
	86,
	79,
	72,
	65, // mid-dawn, 8
	58,
	53,
	43, // normal nighttime, 11  (11 tiles)
	35,
	26,
	17,
	9
}};*/

SightFlags gubSightFlags;


// Returns whether the new value was out of the acceptable range.
static bool DecayOppListValue(int8_t& value)
{
	// if this person has been SEEN recently, but is not currently visible
	if (value >= SEEN_THIS_TURN)
	{
		++value; // increment how long it's been
	}
	// if this person has been only HEARD recently
	else if (value <= HEARD_THIS_TURN)
	{
		--value; // increment how long it's been
	}

	// if it's been longer than the maximum we care to remember
	if (value < OLDEST_HEARD_VALUE || value > OLDEST_SEEN_VALUE)
	{
		value = NOT_HEARD_OR_SEEN;
		return true;
	}

	return false;
}


INT16 AdjustMaxSightRangeForEnvEffects(INT8 bLightLevel, INT16 sDistVisible)
{
	INT16 sNewDist = 0;

	sNewDist = sDistVisible * gbLightSighting[ 0 ][ bLightLevel ] / 100;

	// Adjust it based on weather...
	if ( guiEnvWeather & ( WEATHER_FORECAST_SHOWERS | WEATHER_FORECAST_THUNDERSHOWERS ) )
	{
		sNewDist = sNewDist * 70 / 100;
	}

	return( sNewDist );
}


static void SwapBestSightingPositions(INT8 bPos1, INT8 bPos2)
{
	SOLDIERTYPE* const temp = gBestToMakeSighting[bPos1];
	gBestToMakeSighting[bPos1] = gBestToMakeSighting[bPos2];
	gBestToMakeSighting[bPos2] = temp;
}


static void ReevaluateBestSightingPosition(SOLDIERTYPE* pSoldier, INT8 bInterruptDuelPts)
{
	UINT8 ubLoop, ubLoop2;
	BOOLEAN fFound = FALSE;
	BOOLEAN fPointsGotLower = FALSE;

	if ( bInterruptDuelPts == NO_INTERRUPT )
	{
		return;
	}

	if ( !( pSoldier->uiStatusFlags & SOLDIER_MONSTER ) )
	{
		//gfHumanSawSomeoneInRealtime = TRUE;
	}

	if ( (pSoldier->bInterruptDuelPts != NO_INTERRUPT) && (bInterruptDuelPts < pSoldier->bInterruptDuelPts) )
	{
		fPointsGotLower = TRUE;
	}

	if ( fPointsGotLower )
	{
		// loop to end of array less 1 entry since we can't swap the last entry out of the array
		for ( ubLoop = 0; ubLoop < gubBestToMakeSightingSize - 1; ubLoop++ )
		{
			if (pSoldier == gBestToMakeSighting[ubLoop])
			{
				fFound = TRUE;
				break;
			}
		}

		// this guy has fewer interrupt pts vs another enemy!  reduce position unless in last place
		if (fFound)
		{
			// set new points
			SLOGD("RBSP: reducing points for {} to {}", pSoldier->ubID, bInterruptDuelPts);
			pSoldier->bInterruptDuelPts = bInterruptDuelPts;

			// must percolate him down
			for ( ubLoop2 = ubLoop + 1; ubLoop2 < gubBestToMakeSightingSize; ubLoop2++ )
			{
				if (gBestToMakeSighting[ubLoop2] != NULL && gBestToMakeSighting[ubLoop2 - 1]->bInterruptDuelPts < gBestToMakeSighting[ubLoop2]->bInterruptDuelPts)
				{
					SwapBestSightingPositions( (UINT8) (ubLoop2 - 1), ubLoop2 );
				}
				else
				{
					break;
				}
			}
		}
		else if (pSoldier == gBestToMakeSighting[gubBestToMakeSightingSize - 1])
		{
			// in list but can't be bumped down... set his new points
			SLOGD("RBSP: reduced points for last individual {} to {}",
				pSoldier->ubID, bInterruptDuelPts);
			pSoldier->bInterruptDuelPts = bInterruptDuelPts;
		}
	}
	else
	{
		// loop through whole array
		for ( ubLoop = 0; ubLoop < gubBestToMakeSightingSize; ubLoop++ )
		{
			if (pSoldier == gBestToMakeSighting[ubLoop])
			{
				fFound = TRUE;
				break;
			}
		}

		if (!fFound)
		{
			for ( ubLoop = 0; ubLoop < gubBestToMakeSightingSize; ubLoop++ )
			{
				if (gBestToMakeSighting[ubLoop] == NULL || bInterruptDuelPts > gBestToMakeSighting[ubLoop]->bInterruptDuelPts)
				{
					if (gBestToMakeSighting[gubBestToMakeSightingSize - 1] !=  NULL)
					{
						gBestToMakeSighting[gubBestToMakeSightingSize - 1]->bInterruptDuelPts = NO_INTERRUPT;
						SLOGD("RBSP: resetting points for {} to zilch", pSoldier->ubID);
					}

					// set new points
					SLOGD("RBSP: setting points for {} to {}", pSoldier->ubID, bInterruptDuelPts);
					pSoldier->bInterruptDuelPts = bInterruptDuelPts;

					// insert here!
					for ( ubLoop2 = gubBestToMakeSightingSize - 1; ubLoop2 > ubLoop; ubLoop2-- )
					{
						gBestToMakeSighting[ubLoop2] = gBestToMakeSighting[ubLoop2 - 1];
					}
					gBestToMakeSighting[ubLoop] = pSoldier;
					break;
				}
			}
		}
		// else points didn't get lower, so do nothing (because we want to leave each merc with
		// as low int points as possible)
	}

	for ( ubLoop = 0; ubLoop < BEST_SIGHTING_ARRAY_SIZE; ubLoop++ )
	{
		if (gBestToMakeSighting[ubLoop] != NULL)
		{
			SLOGD("RBSP entry {}: {} ({} pts)",
				ubLoop, gBestToMakeSighting[ubLoop]->ubID,
				gBestToMakeSighting[ubLoop]->bInterruptDuelPts);
		}
	}
}


static void HandleBestSightingPositionInRealtime(void)
{
	// This function is called for handling interrupts when opening a door in non-combat or
	// just sighting in non-combat, deciding who gets the first turn

	if ( gfDelayResolvingBestSightingDueToDoor )
	{
		SLOGD("HBSPIR: skipping due to door flag" );
		return;
	}

	if (gBestToMakeSighting[0] != NULL)
	{
		SLOGD("HBSPIR called and there is someone in the list" );

		//if (gfHumanSawSomeoneInRealtime)
		{
			if (gBestToMakeSighting[1] == NULL)
			{
				// award turn
				EnterCombatMode(gBestToMakeSighting[0]->bTeam);
			}
			else
			{
				// if 1st and 2nd on same team, or 1st and 3rd on same team, or there IS no 3rd, award turn to 1st
				if (gBestToMakeSighting[0]->bTeam == gBestToMakeSighting[1]->bTeam ||
					gBestToMakeSighting[2] == NULL ||
					gBestToMakeSighting[0]->bTeam == gBestToMakeSighting[2]->bTeam)
				{
					EnterCombatMode(gBestToMakeSighting[0]->bTeam);
				}
				else // give turn to 2nd best but interrupt to 1st
				{
					SLOGD("Entering combat mode: turn for 2nd best, int for best");

					EnterCombatMode(gBestToMakeSighting[1]->bTeam);
					// 2nd guy loses control
					AddToIntList(gBestToMakeSighting[1], FALSE, TRUE);
					// 1st guy gains control
					AddToIntList(gBestToMakeSighting[0], TRUE,  TRUE);
					DoneAddingToIntList();
				}
			}
		}

		for (UINT8 ubLoop = 0; ubLoop < BEST_SIGHTING_ARRAY_SIZE; ++ubLoop)
		{
			if (gBestToMakeSighting[ubLoop] != NULL)
			{
				gBestToMakeSighting[ubLoop]->bInterruptDuelPts = NO_INTERRUPT;
				SLOGD("RBSP: done, resetting points for {} to zilch",
					gBestToMakeSighting[ubLoop]->ubID);
			}
		}

#if defined FORCE_ASSERTS_ON
		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE* const s = *i;
			AssertMsg(s->bInterruptDuelPts == NO_INTERRUPT,
				ST::format("{} ({}) still has interrupt pts!", s->name, s->ubID));
		}
#endif
	}
}


static void HandleBestSightingPositionInTurnbased(void)
{
	// This function is called for handling interrupts when opening a door in turnbased
	BOOLEAN fOk = FALSE;

	if (gBestToMakeSighting[0] != NULL)
	{
		if (gBestToMakeSighting[0]->bTeam != gTacticalStatus.ubCurrentTeam)
		{
			// interrupt!
			UINT8 ubLoop;
			for (ubLoop = 0; ubLoop < gubBestToMakeSightingSize; ++ubLoop)
			{
				if (gBestToMakeSighting[ubLoop] == NULL)
				{
					// If nobody, do nothing (for now) - abort!
					if (gInterruptProvoker == NULL) return;

					// use this guy as the "interrupted" fellow
					gBestToMakeSighting[ubLoop] = gInterruptProvoker;
					fOk = TRUE;
					break;
				}
				else if (gBestToMakeSighting[ubLoop]->bTeam == gTacticalStatus.ubCurrentTeam)
				{
					fOk = TRUE;
					break;
				}
			}

			if ( fOk )
			{
				// this is the guy who gets "interrupted"; all else before him interrupted him
				AddToIntList(gBestToMakeSighting[ubLoop], FALSE, TRUE);
				for (UINT8 ubLoop2 = 0; ubLoop2 < ubLoop; ++ubLoop2)
				{
					AddToIntList(gBestToMakeSighting[ubLoop2], TRUE, TRUE);
				}
				DoneAddingToIntList();
			}

		}
		for (UINT8 ubLoop = 0; ubLoop < BEST_SIGHTING_ARRAY_SIZE; ++ubLoop)
		{
			if (gBestToMakeSighting[ubLoop] != NULL)
			{
				gBestToMakeSighting[ubLoop]->bInterruptDuelPts = NO_INTERRUPT;
				SLOGD("RBSP (TB): done, resetting points for {} to zilch",
							gBestToMakeSighting[ubLoop]->ubID);
			}
		}

#if defined FORCE_ASSERTS_ON
		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE* const s = *i;
			AssertMsg(s->bInterruptDuelPts == NO_INTERRUPT, ST::format("{} ({}) still has interrupt pts!", s->name, s->ubID));
		}
#endif
	}
}


static void InitSightArrays()
{
	FOR_EACH(SOLDIERTYPE*, i, gBestToMakeSighting) *i = 0;
}


void AddToShouldBecomeHostileOrSayQuoteList(SOLDIERTYPE* const s)
{
	UINT8 ubLoop;

	Assert( gubNumShouldBecomeHostileOrSayQuote < SHOULD_BECOME_HOSTILE_SIZE );

	if (s->bLife < OKLIFE) return;

	// make sure not already in list
	for ( ubLoop = 0; ubLoop < gubNumShouldBecomeHostileOrSayQuote; ubLoop++ )
	{
		if (gShouldBecomeHostileOrSayQuote[ubLoop] == s) return;
	}

	gShouldBecomeHostileOrSayQuote[gubNumShouldBecomeHostileOrSayQuote++] = s;
}


static SOLDIERTYPE* SelectSpeakerFromHostileOrSayQuoteList(void)
{
	SOLDIERTYPE* speaker_list[SHOULD_BECOME_HOSTILE_SIZE];
	UINT8 ubLoop, ubNumProfiles = 0;

	for ( ubLoop = 0; ubLoop < gubNumShouldBecomeHostileOrSayQuote; ubLoop++ )
	{
		SOLDIERTYPE* const pSoldier = gShouldBecomeHostileOrSayQuote[ubLoop];
		if ( pSoldier->ubProfile != NO_PROFILE )
		{

			// make sure person can say quote!!!!
			gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_NEEDS_TO_SAY_HOSTILE_QUOTE;

			if ( NPCHasUnusedHostileRecord( pSoldier->ubProfile, APPROACH_DECLARATION_OF_HOSTILITY ) )
			{
				speaker_list[ubNumProfiles++] = pSoldier;
			}
			else
			{
				// turn flag off again
				gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 &= ~PROFILE_MISC_FLAG2_NEEDS_TO_SAY_HOSTILE_QUOTE;
			}

		}
	}

	return ubNumProfiles == 0 ? NULL : speaker_list[Random(ubNumProfiles)];
}


void CheckHostileOrSayQuoteList( void )
{
	if ( gubNumShouldBecomeHostileOrSayQuote == 0 || !DialogueQueueIsEmpty() ||
		gfInTalkPanel || gfWaitingForTriggerTimer )
	{
		return;
	}
	else
	{
		UINT8 ubLoop;

		SOLDIERTYPE* const speaker = SelectSpeakerFromHostileOrSayQuoteList();
		if (speaker == NULL)
		{
			// make sure everyone on this list is hostile
			for ( ubLoop = 0; ubLoop < gubNumShouldBecomeHostileOrSayQuote; ubLoop++ )
			{
				SOLDIERTYPE* const pSoldier = gShouldBecomeHostileOrSayQuote[ubLoop];
				if ( pSoldier->bNeutral )
				{
					MakeCivHostile( pSoldier, 2 );
					// make civ group, if any, hostile
					if ( pSoldier->bTeam == CIV_TEAM && pSoldier->ubCivilianGroup != NON_CIV_GROUP && gTacticalStatus.fCivGroupHostile[ pSoldier->ubCivilianGroup ] == CIV_GROUP_WILL_BECOME_HOSTILE )
					{
						gTacticalStatus.fCivGroupHostile[ pSoldier->ubCivilianGroup ] = CIV_GROUP_HOSTILE;
					}
				}
			}

			// unpause all AI
			UnPauseAI();
			// reset the list
			std::fill(std::begin(gShouldBecomeHostileOrSayQuote), std::end(gShouldBecomeHostileOrSayQuote), nullptr);
			gubNumShouldBecomeHostileOrSayQuote = 0;
			//and return/go into combat
			if ( !(gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				EnterCombatMode( CIV_TEAM );
			}
		}
		else
		{
			// pause all AI
			PauseAIUntilManuallyUnpaused();
			// stop everyone?

			// We want to make this guy visible to the player.
			if (speaker->bVisible != TRUE)
			{
				gbPublicOpplist[OUR_TEAM][speaker->ubID] = HEARD_THIS_TURN;
				HandleSight(*speaker, SIGHT_LOOK | SIGHT_RADIO);
			}
			// trigger hater
			TriggerNPCWithIHateYouQuote(speaker->ubProfile);
		}
	}
}


static void ManLooksForOtherTeams(SOLDIERTYPE* pSoldier);
static void OurTeamRadiosRandomlyAbout(SOLDIERTYPE* about);
static void OtherTeamsLookForMan(SOLDIERTYPE* pOpponent);


static bool IsSoldierValidForSightings(SOLDIERTYPE const& s)
{
	extern INT32 iHelicopterVehicleId; 	// I don't want to include a map screen header file for this
	return !(!s.bActive
	      || !s.bInSector
	      || s.sGridNo == NOWHERE
	      || s.uiStatusFlags & SOLDIER_DEAD
	      || s.bLife <= 0 // should be redundant, but who knows?
	      || (s.bAssignment == VEHICLE && s.iVehicleId == iHelicopterVehicleId));
}


void HandleSight(SOLDIERTYPE& s, SightFlags const sight_flags)
{
	if (!IsSoldierValidForSightings(s)) return;

	gubSightFlags = sight_flags;

	if (gubBestToMakeSightingSize != BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL)
	{
		// If this is not being called as a result of all teams look for all, reset
		// array size
		if (gTacticalStatus.uiFlags & INCOMBAT)
		{
			// NB the incombat size is 0
			gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_INCOMBAT;
		}
		else
		{
			gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_NONCOMBAT;
		}

		InitSightArrays();
	}

	for (UINT32 i = 0; i != NUM_WATCHED_LOCS; ++i)
	{
		gfWatchedLocHasBeenIncremented[s.ubID][i] = FALSE;
	}

	gfPlayerTeamSawCreatures = FALSE;

	// store new situation value
	INT8 const temp_new_situation = s.bNewSituation;
	s.bNewSituation = FALSE;

	// If we've been told to make this soldier look (& others look back at him)
	if (sight_flags & SIGHT_LOOK)
	{
		// If this soldier's under our control and well enough to look
		if (s.bLife >= OKLIFE)
		{
			// He looks for all other soldiers not on his own team
			ManLooksForOtherTeams(&s);
		}

		// All soldiers under our control but not on ptr's team look for him
		OtherTeamsLookForMan(&s);
	}

	// If we've been told that interrupts are possible as a result of sighting
	if ((gTacticalStatus.uiFlags & INCOMBAT) &&
		sight_flags & SIGHT_INTERRUPT)
	{
		ResolveInterruptsVs(&s, SIGHTINTERRUPT);
	}

	if (gubBestToMakeSightingSize == BEST_SIGHTING_ARRAY_SIZE_NONCOMBAT)
	{
		HandleBestSightingPositionInRealtime();
	}

	if (s.bNewSituation && !(s.uiStatusFlags & SOLDIER_PC))
	{
		HaultSoldierFromSighting(&s, TRUE);
	}
	s.bNewSituation = std::max(s.bNewSituation, temp_new_situation);

	// If we've been told to radio the results
	if (sight_flags & SIGHT_RADIO)
	{
		if (s.uiStatusFlags & SOLDIER_PC)
		{
			// update our team's public knowledge
			RadioSightings(&s, EVERYBODY, s.bTeam);
			// Our team cooperates with the militia
			RadioSightings(&s, EVERYBODY, MILITIA_TEAM);

			// If it's our local player's merc, revealing roofs and looking for items
			// is handled here, too
			if (IsOnOurTeam(s)) RevealRoofsAndItems(&s, TRUE);
		}
		else if (gGameOptions.ubDifficultyLevel >= DIF_LEVEL_MEDIUM)
		{
			// Unless in easy mode allow alerted enemies to radio
			// Don't allow admins to radio
			if (s.bTeam == ENEMY_TEAM &&
				gTacticalStatus.Team[ENEMY_TEAM].bAwareOfOpposition &&
				s.ubSoldierClass != SOLDIER_CLASS_ADMINISTRATOR)
			{
				RadioSightings(&s, EVERYBODY, s.bTeam);
			}
		}

		s.bNewOppCnt = 0;

		// If this soldier's NOT on our team (MAY be under our control, though!)
		if (!IsOnOurTeam(s)) OurTeamRadiosRandomlyAbout(&s); // radio about him only

		// All non-humans under our control would now radio, if they were allowed to
		// radio automatically (but they're not). So just nuke new opp cnt.
		// NEW: under LOCALOPPLIST, humans on other teams now also radio in here
		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE& them = **i;
			if (!IsSoldierValidForSightings(them) || them.bLife < OKLIFE) continue;

			// if this merc is on the same team as the target soldier
			if (them.bTeam == s.bTeam) continue; // he doesn't look (he ALWAYS knows about him)

			// Other human team's merc report sightings to their teams now
			if (them.uiStatusFlags & SOLDIER_PC)
			{
				// Temporary for opplist synching - disable random order radioing
				// Exclude our own team, we've already done them, randomly
				if (them.bTeam != OUR_TEAM)
					RadioSightings(&them, &s, them.bTeam);
			}
			else if (gGameOptions.ubDifficultyLevel >= DIF_LEVEL_MEDIUM)
			{
				// Unless in easy mode allow alerted enemies to radio
				// Don't allow admins to radio
				if (them.bTeam == ENEMY_TEAM &&
					gTacticalStatus.Team[ENEMY_TEAM].bAwareOfOpposition &&
					them.ubSoldierClass != SOLDIER_CLASS_ADMINISTRATOR)
				{
					RadioSightings(&them, EVERYBODY, them.bTeam);
				}
			}

			them.bNewOppCnt = 0;
		}
	}

	// CJC August 13 2002: At the end of handling sight, reset sight flags to
	// allow interrupts in case an audio cue should cause someone to see an enemy
	gubSightFlags |= SIGHT_INTERRUPT;
}


// All mercs on our local team check if they should radio about him
static void OurTeamRadiosRandomlyAbout(SOLDIERTYPE* const about)
{
	// make a list of all of our team's mercs
	UINT radio_cnt = 0;
	SOLDIERTYPE* radio_men[20];
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// if this merc is in this sector, and well enough to look, then put him on
		// our list
		if (IsSoldierValidForSightings(*s) && s->bLife >= OKLIFE)
			radio_men[radio_cnt++] = s;
	}

	// Now RANDOMLY handle each of the mercs on our list, until none remain (this
	// is all being done ONLY so that the mercs in the earliest merc slots do not
	// arbitrarily get the bulk of the sighting speech quote action, while the
	// later ones almost never pipe up, and is NOT strictly necessary, but a nice
	// improvement over original JA)
	for (; radio_cnt > 0; --radio_cnt)
	{
		// Pick a merc from one of the remaining slots at random
		const UINT chosen_idx = Random(radio_cnt);
		SOLDIERTYPE* const chosen = radio_men[chosen_idx];

		// Handle radioing for that merc
		RadioSightings(chosen, about, chosen->bTeam);
		chosen->bNewOppCnt = 0;

		// Move the contents of the last slot into the one just handled
		radio_men[chosen_idx] = radio_men[radio_cnt - 1];
	}
}


static bool TeamNoLongerSeesMan(const UINT8 ubTeam, SOLDIERTYPE* const pOpponent, const SOLDIERTYPE* const exclude, const INT8 bIteration)
{
	// look for all mercs on the same team, check opplists for this soldier
	CFOR_EACH_IN_TEAM(pMate, ubTeam)
	{
		// if this "teammate" is me, myself, or I (whom we want to exclude)
		if (pMate == exclude)
			continue; // skip to next teammate, I KNOW I don't see him...

		// if this merc is not on the same team
		if (pMate->bTeam != ubTeam)
			continue; // skip him, he's no teammate at all!

		// if this merc is at base, on assignment, dead, unconscious
		if (!IsSoldierValidForSightings(*pMate) || pMate->bLife < OKLIFE)
			continue; // next merc

		// if this teammate currently sees this opponent
		if (pMate->bOppList[pOpponent->ubID] == SEEN_CURRENTLY)
			return false; // that's all I need to know, get out of here
	}

	if ( bIteration == 0 )
	{
		if (ubTeam == OUR_TEAM && IsTeamActive(MILITIA_TEAM))
		{
			// check militia team as well
			return TeamNoLongerSeesMan(MILITIA_TEAM, pOpponent, exclude, 1);
		}
		else if (ubTeam == MILITIA_TEAM && IsTeamActive(OUR_TEAM))
		{
			// check player team as well
			return TeamNoLongerSeesMan(OUR_TEAM, pOpponent, exclude, 1);
		}
	}

	// none of my friends is currently seeing the guy, so return success
	return true;
}


static INT16 DistanceSmellable(const SOLDIERTYPE* const pSubject)
{
	INT16 sDistVisible = STRAIGHT; // as a base

	sDistVisible *= 2;
	//In hypothetical non-turnbased mode:
	//sDistVisible += 3;

	if (pSubject)
	{
		if (pSubject->uiStatusFlags & SOLDIER_MONSTER)
		{
			// trying to smell a friend; change nothing
		}
		else
		{
			// smelling a human or animal; if they are coated with monster smell, distance shrinks
			sDistVisible = sDistVisible * (pSubject->bNormalSmell - pSubject->bMonsterSmell) /
					NORMAL_HUMAN_SMELL_STRENGTH;
			if (sDistVisible < 0)
			{
				sDistVisible = 0;
			}
		}
	}
	return( sDistVisible );
}

INT16 MaxDistanceVisible( void )
{
	return( STRAIGHT * 2 );
}


INT16 DistanceVisible(const SOLDIERTYPE* pSoldier, INT8 bFacingDir, INT8 bSubjectDir, INT16 sSubjectGridNo, INT8 bLevel)
{
	int sDistVisible;

	// IMPORTANT! WhoIsThere2 can return a null-pointer for grid calcs
	const SOLDIERTYPE* const pSubject = WhoIsThere2(sSubjectGridNo, bLevel);
	// muzzleflash can only considered if there is actually a soldier on the grid_no
	BOOLEAN const hasMuzzleFlash = pSubject && pSubject->fMuzzleFlash;

	if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
	{
		if ( !pSubject )
		{
			return( FALSE );
		}
		return DistanceSmellable(pSubject);
	}

	if (pSoldier->bBlindedCounter > 0)
	{
		// we're bliiiiiiiiind!!!
		return( 0 );
	}

	if ( bFacingDir == DIRECTION_IRRELEVANT && TANK( pSoldier ) )
	{
		// always calculate direction for tanks so we have something to work with
		bFacingDir = pSoldier->bDesiredDirection;
		bSubjectDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, sSubjectGridNo );
		//bSubjectDir = atan8(pSoldier->sX,pSoldier->sY,pOpponent->sX,pOpponent->sY);
	}

	if ( !TANK( pSoldier ) && ( bFacingDir == DIRECTION_IRRELEVANT || (pSoldier->uiStatusFlags & SOLDIER_ROBOT) || hasMuzzleFlash ) )
	{
		sDistVisible = MaxDistanceVisible();
	}
	else
	{

		if (pSoldier->sGridNo == sSubjectGridNo)
		{
			// looking up or down or two people accidentally in same tile... don't want it to be 0!
			sDistVisible = MaxDistanceVisible();
		}
		else
		{
			sDistVisible = gbLookDistance[bFacingDir][bSubjectDir];

			if ( sDistVisible == ANGLE && (pSoldier->bTeam == OUR_TEAM ||
				pSoldier->bAlertStatus >= STATUS_RED ) )
			{
				sDistVisible = STRAIGHT;
			}

			sDistVisible *= 2;

			if ( pSoldier->usAnimState == RUNNING )
			{
				if ( gbLookDistance[bFacingDir][bSubjectDir] != STRAIGHT )
				{
					// reduce sight when we're not looking in that direction...
					// (20%?)
					sDistVisible = (sDistVisible * 8) / 10;
				}
			}
		}
	}

	if (pSoldier->bLevel != bLevel)
	{
		// add two tiles distance to visibility to/from roofs
		sDistVisible += 2;
	}

	// now reduce based on light level; SHADE_MIN is the define for the
	// highest number the light can be
	int bLightLevel = LightTrueLevel(sSubjectGridNo, bLevel);

	if (hasMuzzleFlash)
		bLightLevel = std::min(bLightLevel, NORMAL_LIGHTLEVEL_DAY);
	sDistVisible = AdjustMaxSightRangeForEnvEffects(bLightLevel, sDistVisible);

	// if we wanted to simulate desert-blindness, we'd bump up the light level
	// under certain conditions (daytime in the desert, for instance)
	if (bLightLevel < NORMAL_LIGHTLEVEL_DAY)
	{
		// greater than normal daylight level; check for sun goggles
		if (IsWearingHeadGear(*pSoldier, SUNGOGGLES))
		{
			// increase sighting distance by up to 2 tiles
			sDistVisible++;
			if (bLightLevel < NORMAL_LIGHTLEVEL_DAY - 1)
			{
				sDistVisible++;
			}
		}
	}
	else if (bLightLevel > NORMAL_LIGHTLEVEL_DAY + 5)
	{
		if (IsWearingHeadGear(*pSoldier, NIGHTGOGGLES) || IsWearingHeadGear(*pSoldier, UVGOGGLES) ||
			pSoldier->ubBodyType == BLOODCAT || AM_A_ROBOT(pSoldier))
		{
			if (IsWearingHeadGear(*pSoldier, NIGHTGOGGLES) || AM_A_ROBOT(pSoldier))
			{
				if (bLightLevel > NORMAL_LIGHTLEVEL_NIGHT)
				{
					// when it gets really dark, light-intensification goggles become less effective
					if ( bLightLevel < NORMAL_LIGHTLEVEL_NIGHT + 3 )
					{
						sDistVisible += (NIGHTSIGHTGOGGLES_BONUS / 2);
					}
					// else no help at all!
				}
				else
				{
					sDistVisible += NIGHTSIGHTGOGGLES_BONUS;
				}

			}
			// UV goggles only function above ground... ditto for bloodcats
			else if (gWorldSector.z == 0)
			{
				sDistVisible += UVGOGGLES_BONUS;
			}

		}

		// give one step better vision for people with nightops
		sDistVisible += 1 * NUM_SKILL_TRAITS(pSoldier, NIGHTOPS);
	}


	// let tanks see and be seen further (at night)
	if ( (TANK( pSoldier ) && sDistVisible > 0) || (pSubject && TANK( pSubject ) ) )
	{
		sDistVisible = std::max(sDistVisible + 5, int(MaxDistanceVisible()));
	}

	auto smokeEffectID = GetSmokeEffectOnTile(pSoldier->sGridNo, bLevel);
	if ( smokeEffectID != SmokeEffectID::NOTHING )
	{
		auto smokeEffect = GCM->getSmokeEffect(smokeEffectID);
		auto maxVisibilityWhenAffected = smokeEffect->getMaxVisibilityWhenAffected();
		if (maxVisibilityWhenAffected != 0 && (!IsWearingHeadGear(*pSoldier, GASMASK) || smokeEffect->getIgnoresGasMask()))
		{
			// in gas without a gas mask; reduce max distance visible to 2 tiles at most
			sDistVisible = std::min(2, sDistVisible);
		}
	}

	return(sDistVisible);
}


static void HandleManNoLongerSeen(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, INT8* pPersOL, INT8* pbPublOL);
static void DecideTrueVisibility(SOLDIERTYPE* pSoldier);


void EndMuzzleFlash( SOLDIERTYPE * pSoldier )
{
	pSoldier->fMuzzleFlash = FALSE;

	if (!IsOnOurOrMilitiaTeam(*pSoldier))
	{
		pSoldier->bVisible = 0; // indeterminate state
	}

	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOtherSoldier = *i;
		if ( pOtherSoldier->bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY )
		{
			if ( pOtherSoldier->sGridNo != NOWHERE )
			{
				if ( PythSpacesAway( pOtherSoldier->sGridNo, pSoldier->sGridNo ) > DistanceVisible( pOtherSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, pSoldier->sGridNo, pSoldier->bLevel ) )
				{
					// if this guy can no longer see us, change to seen this turn
					HandleManNoLongerSeen(pOtherSoldier, pSoldier,
								&(pOtherSoldier->bOppList[pSoldier->ubID]),
								&(gbPublicOpplist[pOtherSoldier->bTeam][pSoldier->ubID]));
				}
				// else this person is still seen, if the looker is on our side or the militia the person should stay visible
				else if (IsOnOurOrMilitiaTeam(*pOtherSoldier))
				{
					pSoldier->bVisible = TRUE; // yes, still seen
				}
			}
		}
	}
	DecideTrueVisibility(pSoldier);
}


void TurnOffEveryonesMuzzleFlashes(void)
{
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const s = *i;
		if (s->fMuzzleFlash) EndMuzzleFlash(s);
	}
}


void TurnOffTeamsMuzzleFlashes( UINT8 ubTeam )
{
	FOR_EACH_IN_TEAM(s, ubTeam)
	{
		if (s->fMuzzleFlash) EndMuzzleFlash(s);
	}
}


static INT8 DecideHearing(const SOLDIERTYPE* pSoldier)
{
	// calculate the hearing value for the merc...

	INT8 bSlot;
	INT8 bHearing;

	if ( TANK( pSoldier ) )
	{
		return( -5 );
	}
	else if ( pSoldier->uiStatusFlags & SOLDIER_MONSTER )
	{
		return( -10 );
	}

	bHearing = 0;

	if (pSoldier->bExpLevel > 3)
	{
		bHearing++;
	}

	// sharper hearing generally
	bHearing += 1 * NUM_SKILL_TRAITS(pSoldier, NIGHTOPS);

	bSlot = FindObj( pSoldier, EXTENDEDEAR );
	if ( bSlot == HEAD1POS || bSlot == HEAD2POS)
	{
		// at 81-100% adds +5, at 61-80% adds +4, at 41-60% adds +3, etc.
		bHearing += pSoldier->inv[bSlot].bStatus[0] / 20 + 1;
	}

	// adjust for dark conditions
	switch ( ubAmbientLightLevel )
	{
		case 8:
		case 9:
			bHearing += 1;
			break;
		case 10:
			bHearing += 2;
			break;
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			bHearing += 3;
			// yet another bonus for nighttime
			bHearing += 1 * NUM_SKILL_TRAITS(pSoldier, NIGHTOPS);
			break;
		default:
			break;
	}

	return( bHearing );
}

void InitOpplistForDoorOpening( void )
{
	// this is called before generating a noise for opening a door so that
	// the results of hearing the noise are lumped in with the results from AllTeamsLookForAll
	gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL;
	gfDelayResolvingBestSightingDueToDoor = TRUE; // will be turned off in allteamslookforall
	SLOGD("HBSPIR: setting door flag on" );
	// must init sight arrays here
	InitSightArrays();
}


void AllTeamsLookForAll(UINT8 ubAllowInterrupts)
{
	if( ( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		return;
	}

	if (ubAllowInterrupts || !(gTacticalStatus.uiFlags & INCOMBAT) )
	{
		gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL;
		if ( gfDelayResolvingBestSightingDueToDoor )
		{
			// turn off flag now, and skip init of sight arrays
			SLOGD("HBSPIR: turning door flag off" );
			gfDelayResolvingBestSightingDueToDoor = FALSE;
		}
		else
		{
			InitSightArrays();
		}
	}

	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE& s = **i;
		if (s.sGridNo == NOWHERE) continue;
		if (s.bLife >= OKLIFE) HandleSight(s, SIGHT_LOOK); // no radio or interrupts yet
	}

	// the player team now radios about all sightings
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		HandleSight(*i, SIGHT_RADIO); // looking was done above
	}

	if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
	{
		// decide who should get first turn
		HandleBestSightingPositionInRealtime();
		// this could have made us switch to combat mode
		if ( (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_INCOMBAT;
		}
		else
		{
			gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_NONCOMBAT;
		}
	}
	else if ( ubAllowInterrupts )
	{
		HandleBestSightingPositionInTurnbased();
		// reset sighting size to 0
		gubBestToMakeSightingSize = BEST_SIGHTING_ARRAY_SIZE_INCOMBAT;
	}

	// reset interrupt only guynum which may have been used
	gInterruptProvoker = NULL;
}


static INT16 ManLooksForMan(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubCaller);


static void ManLooksForOtherTeams(SOLDIERTYPE* pSoldier)
{
	SLOGD("MANLOOKSFOROTHERTEAMS ID {}({}) team {} side {}",
		pSoldier->ubID, pSoldier->name, pSoldier->bTeam, pSoldier->bSide);

	// one soldier (pSoldier) looks for every soldier on another team (pOpponent)
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;
		if (IsSoldierValidForSightings(*pOpponent))
		{
			// and if he's on another team...
			if (pSoldier->bTeam != pOpponent->bTeam)
			{

				// use both sides actual x,y co-ordinates (neither side's moving)
				// if he sees this opponent...
				ManLooksForMan(pSoldier,pOpponent,MANLOOKSFOROTHERTEAMS);

				// OK, We now want to , if in non-combat, set visiblity to 0 if not visible still....
				// This allows us to walk away from buddy and have them disappear instantly
				if (!(gTacticalStatus.uiFlags & INCOMBAT))
				{
					if ( pOpponent->bVisible == 0)
					{
						pOpponent->bVisible = -1;
					}
				}

			}
		}
	}
}


static void RemoveOneOpponent(SOLDIERTYPE* pSoldier);


static void HandleManNoLongerSeen(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, INT8* pPersOL, INT8* pbPublOL)
{
	// if neither side is neutral AND
	// if this soldier is an opponent (fights for different side)
	if (!CONSIDERED_NEUTRAL( pOpponent, pSoldier ) && !CONSIDERED_NEUTRAL( pSoldier, pOpponent ) &&
		(pSoldier->bSide != pOpponent->bSide))
	{
		RemoveOneOpponent(pSoldier);
	}

	// change personal opplist to indicate "seen this turn"
	// don't use UpdatePersonal() here, because we're changing to a *lower*
	// opplist value (which UpdatePersonal ignores) and we're not updating
	// the lastKnown gridno at all, we're keeping it at its previous value

	*pPersOL = SEEN_THIS_TURN;

	if ( (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP) && (pOpponent->bTeam == OUR_TEAM ) )
	{
		UINT8 const ubRoom = GetRoom(pOpponent->sGridNo);
		if (IN_BROTHEL(ubRoom) && IN_BROTHEL_GUARD_ROOM(ubRoom))
		{
			// unauthorized!
			// make guard run to block guard room
			CancelAIAction(pSoldier);
			RESETTIMECOUNTER( pSoldier->AICounter, 0 );
			pSoldier->bNextAction = AI_ACTION_RUN;
			pSoldier->usNextActionData = 13250;
		}
	}

	// if opponent was seen publicly last time
	if (*pbPublOL == SEEN_CURRENTLY)
	{
		// check if I was the only one who was seeing this guy (exlude ourselves)
		// THIS MUST HAPPEN EVEN FOR ENEMIES, TO MAKE THEIR PUBLIC opplist DECAY!
		if (TeamNoLongerSeesMan(pSoldier->bTeam, pOpponent, pSoldier, 0))
		{
			SLOGD("TeamNoLongerSeesMan: ID {}({}) to ID {}", pSoldier->ubID, pSoldier->name, pOpponent->ubID);

			// don't use UpdatePublic() here, because we're changing to a *lower*
			// opplist value (which UpdatePublic ignores) and we're not updating
			// the lastKnown gridno at all, we're keeping it at its previous value
			*pbPublOL = SEEN_THIS_TURN;

			// ATE: Set visiblity to 0
			if (IsOnOurOrMilitiaTeam(*pSoldier) && !IsOnOurOrMilitiaTeam(*pOpponent))
			{
				pOpponent->bVisible = 0;
			}
		}
	}
	else
		SLOGD("ManLooksForMan: ID {}({}) to ID {} Personally seen, public {}",
			pSoldier->ubID, pSoldier->name, pOpponent->ubID, *pbPublOL);

	// if we had only seen the guy for an instant and now lost sight of him
	if (gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] == -1)
		// we can't leave it -1, because InterruptDuel() uses the special -1
		// value to know if we're only JUST seen the guy and screw up otherwise
		// it's enough to know we have seen him before
		gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] = TRUE;

}


static void ManSeesMan(SOLDIERTYPE& s, SOLDIERTYPE& opponent, UINT8 caller2);


static INT16 ManLooksForMan(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubCaller)
{
	INT8 bDir,bAware = FALSE,bSuccess = FALSE;
	INT16 sDistVisible,sDistAway;
	INT8  *pPersOL,*pbPublOL;


	/*
	if (ptr->guynum >= MAX_NUM_SOLDIERS)
	{
		return(success);
	}

	if (oppPtr->guynum >= MAX_NUM_SOLDIERS)
	{
		return(success);
	}*/

	if (pSoldier == pOpponent)
	{
		SLOGD("ManLooksForMan - Looking for self - ID {}({})", pSoldier->ubID, pSoldier->name);
		return(FALSE);
	}

	// if we're somehow looking while inactive, at base, dead or dying
	if (!IsSoldierValidForSightings(*pSoldier) || pSoldier->bLife < OKLIFE)
	{
		SLOGD("ManLooksForMan - WE are inactive/dead etc ID {}({})to ID {}",
			pSoldier->ubID, pSoldier->name, pOpponent->ubID);
		return(FALSE);
	}



	// if we're somehow looking for a guy who is inactive, at base, or already dead
	if (!IsSoldierValidForSightings(*pOpponent) || pOpponent->sGridNo == NOWHERE)
	{
		return(FALSE);
	}


	// if he's looking for a guy who is on the same team
	if (pSoldier->bTeam == pOpponent->bTeam)
	{
		SLOGD("ManLooksForMan - SAME TEAM ID {}({})to ID {}",
			pSoldier->ubID, pSoldier->name, pOpponent->ubID);
		return(FALSE);
	}

	if (pSoldier->bLife < OKLIFE || pSoldier->fMercAsleep)
	{
		return( FALSE );
	}

	// NEED TO CHANGE THIS
	/*
	// don't allow unconscious persons to look, but COLLAPSED, etc. is OK
	if (ptr->anitype[ptr->anim] == UNCONSCIOUS)
		return(success);*/

	if (pSoldier->ubBodyType == LARVAE_MONSTER || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE &&
		pSoldier->bTeam == OUR_TEAM))
	{
		// don't do sight for these
		return( FALSE );
	}


	/*
	if (ptrProjected)
	{
		// use looker's PROJECTED x,y co-ordinates (those of his next gridno)
		fromX = ptr->destx;
		fromY = ptr->desty;
		fromGridno = ExtMen[ptr->guynum].nextGridno;
	}
	else
	{
		// use looker's ACTUAL x,y co-ordinates (those of gridno he's in now)
		fromX = ptr->x;
		fromY = ptr->y;
		fromGridno = ptr->sGridNo;
	}


	if (oppPtrProjected)
	{
		// use target's PROJECTED x,y co-ordinates (those of his next gridno)
		toX = oppPtr->destx;
		toY = oppPtr->desty;
		toGridno = ExtMen[oppPtr->guynum].nextGridno;
	}
	else
	{
		// use target's ACTUAL x,y co-ordinates (those of gridno he's in now)
		toX = oppPtr->x;
		toY = oppPtr->y;
		toGridno = oppPtr->gridno;
	}*/

	pPersOL = &(pSoldier->bOppList[pOpponent->ubID]);
	pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID]);


	// if soldier is known about (SEEN or HEARD within last few turns)
	if (*pPersOL || *pbPublOL)
	{
		bAware = TRUE;

		// then we look for him full viewing distance in EVERY direction
		sDistVisible = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, 0, pOpponent->sGridNo,
						pOpponent->bLevel );
	}
	else // soldier is not currently known about
	{
		// distance we "see" then depends on the direction he is located from us
		bDir = atan8(pSoldier->sX,pSoldier->sY,pOpponent->sX,pOpponent->sY);
		// BIG NOTE: must use desdir instead of direction, since in a projected
		// situation, the direction may still be changing if it's one of the first
		// few animation steps when this guy's turn to do his stepped look comes up
		sDistVisible = DistanceVisible(pSoldier,pSoldier->bDesiredDirection,bDir, pOpponent->sGridNo,
						pOpponent->bLevel);
	}

	// calculate how many spaces away soldier is (using Pythagoras' theorem)
	sDistAway = PythSpacesAway(pSoldier->sGridNo,pOpponent->sGridNo);
	SLOGD("MANLOOKSFORMAN: ID {}({}) to ID {}: sDistAway {} sDistVisible {}",
		pSoldier->ubID, pSoldier->name, pOpponent->ubID, sDistAway, sDistVisible);

	// if we see close enough to see the soldier
	if (sDistAway <= sDistVisible)
	{
		// and we can trace a line of sight to his x,y coordinates
		// must use the REAL opplist value here since we may or may not know of him
		if (SoldierToSoldierLineOfSightTest(pSoldier,pOpponent,(UINT8)sDistVisible,bAware))
		{
			ManSeesMan(*pSoldier, *pOpponent, ubCaller);
			bSuccess = TRUE;
		}
		else
		{
			SLOGD("FAILED LINEOFSIGHT: ID {} ({})to ID {} Personally {}, public {}",
				pSoldier->ubID, pSoldier->name, pOpponent->ubID, *pPersOL, *pbPublOL);
		}
	}

	// if soldier seen personally LAST time could not be seen THIS time
	if (!bSuccess && (*pPersOL == SEEN_CURRENTLY))
	{
		HandleManNoLongerSeen( pSoldier, pOpponent, pPersOL, pbPublOL );
	}
	else
	{
		if (!bSuccess)
		{
			SLOGD("NO LONGER VISIBLE ID {} ({})to ID {} Personally {}, public {} success: {}",
				pSoldier->ubID, pSoldier->name, pOpponent->ubID, *pPersOL, *pbPublOL, bSuccess);


			// we didn't see the opponent, but since we didn't last time, we should be
			//if (*pbPublOL)
				//pOpponent->bVisible = TRUE;
		}
		else
		{
			SLOGD("COOL. STILL VISIBLE ID {} ({})to ID {} Personally {}, public {} success: {}",
				pSoldier->ubID, pSoldier->name, pOpponent->ubID, *pPersOL, *pbPublOL, bSuccess);
		}
	}
	return(bSuccess);
}


static void IncrementWatchedLoc(const SOLDIERTYPE* watcher, INT16 sGridNo, INT8 bLevel);
static void SetWatchedLocAsUsed(UINT8 ubID, INT16 sGridNo, INT8 bLevel);
static void MakeBloodcatsHostile(void);
static void AddOneOpponent(SOLDIERTYPE* pSoldier);
static void UpdatePersonal(SOLDIERTYPE* pSoldier, UINT8 ubID, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel);


static void ManSeesMan(SOLDIERTYPE& s, SOLDIERTYPE& opponent, UINT8 const caller2)
{
	if (s.ubID        >= MAX_NUM_SOLDIERS) return;
	if (opponent.ubID >= MAX_NUM_SOLDIERS) return;

	// if we're somehow looking while inactive, at base, dying or already dead
	if (!IsSoldierValidForSightings(s)) return;
	if (s.bLife < OKLIFE) return;

	// if we're somehow seeing a guy who is inactive, at base, or already dead
	if (!IsSoldierValidForSightings(opponent)) return;

	// If we're somehow seeing a guy who is on the same team
	if (s.bTeam == opponent.bTeam) return;

	INT8   const old_opp_list = s.bOppList[opponent.ubID];
	GridNo const opp_gridno   = opponent.sGridNo;
	INT8   const opp_level    = opponent.bLevel;

	bool new_opponent = false;
	// If we're seeing a guy we didn't see on our last chance to look for him
	if (s.bOppList[opponent.ubID] != SEEN_CURRENTLY)
	{
		if (opponent.bTeam == OUR_TEAM)
		{
			if (s.ubProfile != NO_PROFILE)
			{
				if (s.bTeam == CIV_TEAM)
				{
					bool not_added_to_list = true;
					// If this person doing the sighting is a member of a civ group that
					// hates us but this fact hasn't been revealed, change the side of
					// these people now. This will make them non-neutral so AddOneOpponent
					// will be called, and the guy will say his "I hate you" quote
					if (s.bNeutral)
					{
						if (s.ubCivilianGroup != NON_CIV_GROUP &&
							gTacticalStatus.fCivGroupHostile[s.ubCivilianGroup] >= CIV_GROUP_WILL_BECOME_HOSTILE)
						{
							AddToShouldBecomeHostileOrSayQuoteList(&s);
							not_added_to_list = false;
						}
					}
					else if (NPCHasUnusedRecordWithGivenApproach(s.ubProfile, APPROACH_DECLARATION_OF_HOSTILITY))
					{
						// only add if have something to say
						AddToShouldBecomeHostileOrSayQuoteList(&s);
						not_added_to_list = false;
					}

					if (not_added_to_list)
					{
						switch (s.ubProfile)
						{
							case CARMEN:
								// Carmen goes to war (against Slay)
								if (opponent.ubProfile == SLAY && s.bNeutral)
								{
									s.bAttitude = ATTACKSLAYONLY;
									TriggerNPCRecord(s.ubProfile, 28);
								}
								break;

							case ELDIN:
								if (s.bNeutral)
								{
									// If player is in behind the ropes of the museum display or
									// if alarm has gone off (status red)
									UINT8 const room = GetRoom(opponent.sGridNo);
									if ((!CheckFact(FACT_MUSEUM_OPEN, 0) &&
										22 <= room && room <= 41) ||
										CheckFact(FACT_MUSEUM_ALARM_WENT_OFF, 0) ||
										room == 39 ||
										room == 40 ||
										FindObj(&opponent, CHALICE) != NO_SLOT)
									{
										SetFactTrue(FACT_MUSEUM_ALARM_WENT_OFF);
										AddToShouldBecomeHostileOrSayQuoteList(&s);
									}
								}
								break;

							case JIM:
							case JACK:
							case OLAF:
							case RAY:
							case OLGA:
							case TYRONE:
								// change orders, reset action!
								if (s.bOrders != SEEKENEMY)
								{
									s.bOrders = SEEKENEMY;
									if (s.bOppCnt == 0)
									{
										// Didn't see anyone before!
										CancelAIAction(&s);
										SetNewSituation(&s);
									}
								}
								break;

							case ANGEL:
								if (opponent.ubProfile == MARIA)
								{
									if (CheckFact(FACT_MARIA_ESCORTED_AT_LEATHER_SHOP, MARIA) == TRUE)
									{
										// She was rescued! yay!
										TriggerNPCRecord(ANGEL, 12);
									}
								}
								else if (CheckFact(FACT_ANGEL_LEFT_DEED, ANGEL) == TRUE &&
									!CheckFact(FACT_ANGEL_MENTIONED_DEED, ANGEL))
								{
									CancelAIAction(&s);
									s.sAbsoluteFinalDestination = NOWHERE;
									EVENT_StopMerc(&s);
									TriggerNPCRecord(ANGEL, 20);
								}
								break;

							case JOE:
							case ELLIOT:
							{
								MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);
								if (!(p.ubMiscFlags2 & PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE) &&
									!AreInMeanwhile())
								{
									TriggerNPCRecord(s.ubProfile, 4);
									p.ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
								}
								break;
							}

							default:
								break;
						}
					}
				}
				else
				{
					switch (s.ubProfile)
					{
						case IGGY:
						{
							MERCPROFILESTRUCT& iggy = GetProfile(s.ubProfile);
							if (!(iggy.ubMiscFlags2 & PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE))
							{
								TriggerNPCRecord(s.ubProfile, 9);
								iggy.ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
								gbPublicOpplist[OUR_TEAM][s.ubID] = HEARD_THIS_TURN;
							}
							break;
						}
					}
				}
			}
			else if (s.bTeam == CIV_TEAM)
			{
				if (s.ubCivilianGroup != NON_CIV_GROUP &&
					gTacticalStatus.fCivGroupHostile[s.ubCivilianGroup] >= CIV_GROUP_WILL_BECOME_HOSTILE && s.bNeutral)
				{
					AddToShouldBecomeHostileOrSayQuoteList(&s);
				}
				else if (s.ubCivilianGroup == KINGPIN_CIV_GROUP)
				{
					// Generic kingpin goon

					// Check to see if we are looking at Maria or unauthorized personnel in the brothel
					if (opponent.ubProfile == MARIA)
					{
						MakeCivHostile(&s, 2);
						if (!(gTacticalStatus.uiFlags & INCOMBAT))
						{
							EnterCombatMode(s.bTeam);
						}
						SetFactTrue(FACT_MARIA_ESCAPE_NOTICED);
					}
					else
					{
						// JA2 Gold: only go hostile if see player IN guard room
						UINT8 const room = GetRoom(opponent.sGridNo);
						if (IN_BROTHEL_GUARD_ROOM(room))
						{
							// Unauthorized
							MakeCivHostile(&s, 2);
							if (!(gTacticalStatus.uiFlags & INCOMBAT))
							{
								EnterCombatMode(s.bTeam);
							}
						}
					}
				}
				else if (s.ubCivilianGroup == HICKS_CIV_GROUP &&
					!CheckFact(FACT_HICKS_MARRIED_PLAYER_MERC, 0))
				{
					// if before 6:05 or after 22:00, make hostile and enter combat
					UINT32 const uiTime = GetWorldMinutesInDay();
					if (uiTime < 365 || 1320 < uiTime)
					{
						// get off our farm!
						MakeCivHostile(&s, 2);
						if (!(gTacticalStatus.uiFlags & INCOMBAT))
						{
							EnterCombatMode(s.bTeam);
							LocateSoldier(&s, TRUE);
							INT16 sX;
							INT16 sY;
							GetSoldierScreenPos(&s, &sX, &sY);
							BeginCivQuote(&s, CIV_QUOTE_HICKS_SEE_US_AT_NIGHT, 0, sX, sY);
						}
					}
				}
			}
		}
		else if (s.bTeam == OUR_TEAM)
		{
			switch (opponent.ubProfile)
			{
				case MIKE:
					if (s.ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC &&
						!(s.usQuoteSaidExtFlags & SOLDIER_QUOTE_SAID_EXT_MIKE))
					{
						if (gfMikeShouldSayHi == FALSE) gfMikeShouldSayHi = TRUE;
						TacticalCharacterDialogue(&s, QUOTE_AIM_SEEN_MIKE);
						s.usQuoteSaidExtFlags |= SOLDIER_QUOTE_SAID_EXT_MIKE;
					}
					break;

				case JOEY:
					if (!gfPlayerTeamSawJoey)
					{
						TacticalCharacterDialogue(&s, QUOTE_SPOTTED_JOEY);
						gfPlayerTeamSawJoey = TRUE;
					}
					break;
			}
		}

		// As soon as a bloodcat sees someone, it becomes hostile. This is safe to
		// do here because we haven't made this new person someone we've seen yet
		// (so we are assured we won't count 'em twice for oppcnt purposes)
		if (s.ubBodyType == BLOODCAT)
		{
			bool do_roar = false;
			if (s.bNeutral)
			{
				MakeBloodcatsHostile();
				do_roar = true;
			}
			else if (s.bOppCnt == 0 && Random(2) == 0)
			{
				do_roar = true;
			}
			if (do_roar) PlayJA2Sample(BLOODCAT_ROAR, HIGHVOLUME, 1, MIDDLEPAN);
		}
		else if (opponent.ubBodyType == BLOODCAT && opponent.bNeutral)
		{
			MakeBloodcatsHostile();
		}

		// if both of us are not neutral, AND
		// if this man is actually a true opponent (we're not on the same side)
		if (!CONSIDERED_NEUTRAL(&opponent, &s) &&
			!CONSIDERED_NEUTRAL(&s, &opponent) &&
			s.bSide != opponent.bSide)
		{
			AddOneOpponent(&s);
			SLOGD("ManSeesMan: ID {}({}) to ID {} NEW TO ME", s.ubID, s.name, opponent.ubID);

			// if we also haven't seen him earlier this turn
			if (s.bOppList[opponent.ubID] != SEEN_THIS_TURN)
			{
				new_opponent = true;
				++s.bNewOppCnt; // increment looker's NEW opponent count

				IncrementWatchedLoc(&s, opponent.sGridNo, opponent.bLevel);

				if (s.bTeam == OUR_TEAM &&
					opponent.bTeam == ENEMY_TEAM &&
					!CheckFact(FACT_FIRST_BATTLE_FOUGHT, 0))
				{
					SetFactTrue(FACT_FIRST_BATTLE_BEING_FOUGHT);
				}
			}
			else
			{
				SetWatchedLocAsUsed(s.ubID, opponent.sGridNo, opponent.bLevel);
			}

			// we already know the soldier isn't SEEN_CURRENTLY,
			// now check if he is really "NEW" ie. not expected to be there

			// if the looker hasn't seen this opponent at all earlier this turn, OR
			// if the opponent is not where the looker last thought him to be
			if (s.bOppList[opponent.ubID] != SEEN_THIS_TURN ||
				gsLastKnownOppLoc[s.ubID][opponent.ubID] != opp_gridno)
			{
				SetNewSituation(&s);  // force the looker to re-evaluate
			}
			else
			{
				// if we in a non-combat movement decision, presumably this is not
				// something we were quite expecting, so make a new decision.  For
				// other (combat) movement decisions, we took his position into account
				// when we made it, so don't make us think again & slow things down.
				switch (s.bAction)
				{
					case AI_ACTION_RANDOM_PATROL:
					case AI_ACTION_SEEK_OPPONENT:
					case AI_ACTION_SEEK_FRIEND:
					case AI_ACTION_POINT_PATROL:
					case AI_ACTION_LEAVE_WATER_GAS:
					case AI_ACTION_SEEK_NOISE:
						SetNewSituation(&s);  // force the looker to re-evaluate
						break;
				}
			}
		}
	}
	else
	{
		SLOGD("ManSeesMan: ID {}({}) to ID {} ALREADYSEENCURRENTLY", s.ubID, s.name, opponent.ubID);
	}
	// Remember that the soldier is currently seen and his new location
	UpdatePersonal(&s, opponent.ubID, SEEN_CURRENTLY, opp_gridno, opp_level);

	if (caller2 == MANLOOKSFOROTHERTEAMS ||
		caller2 == OTHERTEAMSLOOKFORMAN  ||
		caller2 == CALLER_UNKNOWN) // unknown->hearing
	{
		if (gubBestToMakeSightingSize != BEST_SIGHTING_ARRAY_SIZE_INCOMBAT &&
			gTacticalStatus.bBoxingState == NOT_BOXING &&
			new_opponent)
		{
			if (gTacticalStatus.uiFlags & INCOMBAT)
			{
				// Presumably a door opening, we do require standard interrupt
				// conditions
				if (StandardInterruptConditionsMet(&s, &opponent, old_opp_list))
				{
					ReevaluateBestSightingPosition(&s, CalcInterruptDuelPts(&s, &opponent, TRUE));
				}
			}
			else if (s.bTeam != OUR_TEAM || opponent.bLife >= OKLIFE)
			{
				// Require the enemy not to be dying if we are the sighter; in other
				// words, always add for AI guys, and always add for people with life >=
				// OKLIFE
				ReevaluateBestSightingPosition(&s, CalcInterruptDuelPts(&s, &opponent, TRUE));
			}
		}
	}

	// if this man has never seen this opponent before in this sector
	INT8& seen = gbSeenOpponents[s.ubID][opponent.ubID];
	if (seen == FALSE)
	{
		// Remember that he is just seeing him now for the first time (-1)
		seen = -1;
	}
	else
	{
		// Man is seeing an opponent AGAIN whom he has seen at least once before
		seen = TRUE;
	}

	// if looker is on local team, and the enemy was invisible or "maybe"
	// visible just prior to this
	if (IsOnOurOrMilitiaTeam(s) && opponent.bVisible <= 0)
	{
		// If opponent was truly invisible, not just turned off temporarily (FALSE)
		// then locate to him and set his locator flag
		bool const do_locate = opponent.bVisible == -1;

		// Make opponent visible (to us). Must do this BEFORE the locate since it
		// checks for visibility
		opponent.bVisible = TRUE;

		// ATE: Cancel any fading going on
		// ATE: Added for fade in
		if (opponent.fBeginFade == 1 || opponent.fBeginFade == 2)
		{
			opponent.fBeginFade = FALSE;

			MAP_ELEMENT& me = gpWorldLevelData[opponent.sGridNo];
			opponent.ubFadeLevel = opponent.bLevel > 0 && me.pRoofHead ? me.pRoofHead->ubShadeLevel :
						me.pLandHead->ubShadeLevel;

			// Set levelnode shade level
			if (opponent.pLevelNode)
			{
				opponent.pLevelNode->ubShadeLevel = opponent.ubFadeLevel;
			}
		}
		SLOGD("ID {} ({}) MAKING {} VISIBLE", s.ubID, s.name, opponent.ubID);

		if (do_locate)
		{
			// Change his anim speed
			SetSoldierAniSpeed(&opponent);

			if ((gTacticalStatus.uiFlags & INCOMBAT || gTacticalStatus.fVirginSector) &&
				!opponent.bNeutral &&
				s.bSide != opponent.bSide)
			{
				SlideTo(&opponent, SETLOCATOR);
			}
		}
	}
	else if (!IsOnOurTeam(s))
	{
		// ATE: Check stance, change to threatending
		ReevaluateEnemyStance(&s, s.usAnimState);
	}
}


static void DecideTrueVisibility(SOLDIERTYPE* pSoldier)
{
	// if his visibility is still in the special "limbo" state (FALSE)
	if (pSoldier->bVisible == FALSE)
	{
		// then none of our team's merc turned him visible,
		// therefore he now becomes truly invisible
		pSoldier->bVisible = -1;

		// Don;t adjust anim speed here, it's done once fade is over!
	}
}


static void OtherTeamsLookForMan(SOLDIERTYPE* pOpponent)
{
	INT8 bOldOppList;

	// if the guy we're looking for is NOT on our team AND is currently visible
	if (!IsOnOurOrMilitiaTeam(*pOpponent) &&
		(pOpponent->bVisible >= 0 && pOpponent->bVisible < 2) && pOpponent->bLife)
	{
		// assume he's no longer visible, until one of our mercs sees him again
		pOpponent->bVisible = 0;
	}
		SLOGD("OTHERTEAMSLOOKFORMAN ID {}({}) team {} side {}",
			pOpponent->ubID, pOpponent->name, pOpponent->bTeam, pOpponent->bSide);

	// all soldiers not on oppPtr's team now look for him
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pSoldier = *i;

		// if this merc is active, in this sector, and well enough to look
		if (IsSoldierValidForSightings(*pSoldier) && pSoldier->bLife >= OKLIFE  && (pSoldier->ubBodyType != LARVAE_MONSTER))
		{
			// if this merc is on the same team as the target soldier
			if (pSoldier->bTeam == pOpponent->bTeam)
			{
				continue; // he doesn't look (he ALWAYS knows about him)
			}

			bOldOppList = pSoldier->bOppList[pOpponent->ubID];

			// this merc looks for the soldier in question
			// use both sides actual x,y co-ordinates (neither side's moving)
			if (ManLooksForMan(pSoldier,pOpponent,OTHERTEAMSLOOKFORMAN))
			{
				// if a new opponent is seen (which must be oppPtr himself)
				//if ((gTacticalStatus.uiFlags & INCOMBAT) && pSoldier->bNewOppCnt)
				// Calc interrupt points in non-combat because we might get an interrupt or be interrupted
				// on our first turn

				// if doing regular in-combat sighting (not on opening doors!)
				if ( gubBestToMakeSightingSize == BEST_SIGHTING_ARRAY_SIZE_INCOMBAT )
				{
					if ((gTacticalStatus.uiFlags & INCOMBAT) && pSoldier->bNewOppCnt)
					{
						// as long as viewer meets minimum interrupt conditions
						if (gubSightFlags & SIGHT_INTERRUPT && StandardInterruptConditionsMet(pSoldier, pOpponent, bOldOppList))
						{
							// calculate the interrupt duel points
							pSoldier->bInterruptDuelPts = CalcInterruptDuelPts(pSoldier, pOpponent, TRUE);
							SLOGD("Calculating int duel pts in OtherTeamsLookForMan, {} has {} points",
								pSoldier->ubID, pSoldier->bInterruptDuelPts);
						}
						else
						{
							pSoldier->bInterruptDuelPts = NO_INTERRUPT;
						}
					}
				}
			}
		}
	}


	// if he's not on our team
	if (pOpponent->bTeam != OUR_TEAM)
	{
		DecideTrueVisibility(pOpponent);
	}
}


static void AddOneOpponent(SOLDIERTYPE* pSoldier)
{
	INT8 bOldOppCnt = pSoldier->bOppCnt;

	pSoldier->bOppCnt++;

	if (!bOldOppCnt)
	{
		// if we hadn't known about opponents being here for sure prior to this
		if (pSoldier->ubBodyType == LARVAE_MONSTER)
		{
			// never become aware of you!
			return;
		}

		if (pSoldier->bAlertStatus < STATUS_RED)
		{
			CheckForChangingOrders(pSoldier);
		}

		pSoldier->bAlertStatus = STATUS_BLACK; // force black AI status right away

		if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
		{
			pSoldier->ubCaller = NOBODY;
			pSoldier->bCallPriority = 0;
		}
	}

	if (pSoldier->bTeam == OUR_TEAM)
	{
		// adding an opponent for player; reset # of turns that we haven't seen an enemy
		gTacticalStatus.bConsNumTurnsNotSeen = 0;
	}

}


static void RemoveOneOpponent(SOLDIERTYPE* pSoldier)
{
	pSoldier->bOppCnt--;

	if ( pSoldier->bOppCnt < 0 )
	{
		SLOGD("Oppcnt for {} ({}) tried to go below 0", pSoldier->ubID, pSoldier->name);
		pSoldier->bOppCnt = 0;
	}

	// if no opponents remain in sight, drop status to RED (but NOT newSit.!)
	if (!pSoldier->bOppCnt)
		pSoldier->bAlertStatus = STATUS_RED;
}


static void UpdatePublic(UINT8 ubTeam, SOLDIERTYPE* s, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel);
static void ResetLastKnownLocs(SOLDIERTYPE const&);


void RemoveManAsTarget(SOLDIERTYPE *pSoldier)
{

	SOLDIERTYPE *pOpponent;
	UINT8 ubTarget,ubLoop;


	ubTarget = pSoldier->ubID;

	// clean up the public opponent lists and locations
	for (ubLoop = 0; ubLoop < MAXTEAMS; ubLoop++)
	// never causes any additional looks
	UpdatePublic(ubLoop, pSoldier, NOT_HEARD_OR_SEEN, NOWHERE, 0);

	// clean up all opponent's opplists
	for (ubLoop = 0; ubLoop < guiNumMercSlots; ubLoop++)
	{
		pOpponent = MercSlots[ ubLoop ];

		// if the target is active, a true opponent and currently seen by this merc
		if (pOpponent)
		{
			// check to see if OPPONENT considers US neutral
			if ((pOpponent->bOppList[ubTarget] == SEEN_CURRENTLY) && !pOpponent->bNeutral &&
				!CONSIDERED_NEUTRAL(pOpponent, pSoldier) && (pSoldier->bSide != pOpponent->bSide))
			{
				RemoveOneOpponent(pOpponent);
			}
			UpdatePersonal(pOpponent, ubTarget, NOT_HEARD_OR_SEEN,NOWHERE,0);
			gbSeenOpponents[ubLoop][ubTarget] = FALSE;
		}
	}

	ResetLastKnownLocs(*pSoldier);

	TacticalTeamType* const tt = &gTacticalStatus.Team[pSoldier->bTeam];
	if (tt->last_merc_to_radio == pSoldier) tt->last_merc_to_radio = NULL;
}


static void UpdatePublic(const UINT8 ubTeam, SOLDIERTYPE* const s, const INT8 bNewOpplist, const INT16 sGridno, const INT8 bLevel)
{
	UINT8 ubTeamMustLookAgain = FALSE;

	INT8* const pbPublOL = &gbPublicOpplist[ubTeam][s->ubID];

	// if new opplist is more up-to-date, or we are just wiping it for some reason
	if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][bNewOpplist - OLDEST_HEARD_VALUE] > 0) ||
		(bNewOpplist == NOT_HEARD_OR_SEEN))
	{
		// if this team is becoming aware of a soldier it wasn't previously aware of
		if ((bNewOpplist != NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
			ubTeamMustLookAgain = TRUE;

		// change the public opplist *BEFORE* anyone looks again or we'll recurse!
		*pbPublOL = bNewOpplist;
	}


	// always update the gridno, no matter what
	gsPublicLastKnownOppLoc[ubTeam][s->ubID] = sGridno;
	gbPublicLastKnownOppLevel[ubTeam][s->ubID] = bLevel;

	// if team has been told about a guy the team was completely unaware of
	if (ubTeamMustLookAgain)
	{
		// then everyone on team who's not aware of guynum must look for him
		FOR_EACH_IN_TEAM(pSoldier, ubTeam)
		{
			// if this soldier is active, in this sector, and well enough to look
			if (IsSoldierValidForSightings(*pSoldier) && pSoldier->bLife >= OKLIFE && !(pSoldier->uiStatusFlags & SOLDIER_GASSED))
			{
				// if soldier isn't aware of guynum, give him another chance to see
				if (pSoldier->bOppList[s->ubID] == NOT_HEARD_OR_SEEN)
				{
					if (ManLooksForMan(pSoldier, s, UPDATEPUBLIC))
					{
						// then he actually saw guynum because of our new public knowledge
						// whether successful or not, whack newOppCnt.  Since this is a
						// delayed reaction to a radio call, there's no chance of interrupt!
						pSoldier->bNewOppCnt = 0;
					}
				}
			}
		}
	}
}


static void UpdatePersonal(SOLDIERTYPE* pSoldier, UINT8 ubID, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel)
{
	// if new opplist is more up-to-date, or we are just wiping it for some reason
	if ((gubKnowledgeValue[pSoldier->bOppList[ubID] - OLDEST_HEARD_VALUE][bNewOpplist - OLDEST_HEARD_VALUE] > 0) ||
		(bNewOpplist == NOT_HEARD_OR_SEEN))
	{
		pSoldier->bOppList[ubID] = bNewOpplist;
	}

	// always update the gridno, no matter what
	gsLastKnownOppLoc[pSoldier->ubID][ubID] = sGridno;
	gbLastKnownOppLevel[pSoldier->ubID][ubID] = bLevel;
}


static void ResetLastKnownLocs(SOLDIERTYPE const& s)
{
	FOR_EACH_MERC(i)
	{
		const SoldierID tgt_id = (*i)->ubID;
		gsLastKnownOppLoc[s.ubID][tgt_id] = NOWHERE;
		// IAN added this June 14/97
		gsPublicLastKnownOppLoc[s.bTeam][tgt_id] = NOWHERE;
	}
}


/*
// INITIALIZATION STUFF
-------------------------
// Upon loading a scenario, call these:
InitOpponentKnowledgeSystem();

// loop through all soldiers and for each soldier call
InitSoldierOpplist(pSoldier);

// call this once
AllTeamsLookForAll(NO_INTERRUPTS); // no interrupts permitted this early


// for each additional soldier created, call
InitSoldierOpplist(pSoldier);
HandleSight(pSoldier,SIGHT_LOOK);



MOVEMENT STUFF
-----------------
// whenever new tile is reached, call
HandleSight(pSoldier,SIGHT_LOOK);*/


void InitOpponentKnowledgeSystem(void)
{
	INT32 iTeam, cnt, cnt2;

	for (auto& i : gbSeenOpponents)
	{
		std::fill(std::begin(i), std::end(i), 0);
	}
	for (auto& i : gbPublicOpplist)
	{
		std::fill(std::begin(i), std::end(i), NOT_HEARD_OR_SEEN);
	}

	for (iTeam=0; iTeam < MAXTEAMS; iTeam++)
	{
		gubPublicNoiseVolume[iTeam] = 0;
		gsPublicNoiseGridno[iTeam] = NOWHERE;
		gbPublicNoiseLevel[iTeam] = 0;
		for (cnt = 0; cnt < MAX_NUM_SOLDIERS; cnt++)
		{
			gsPublicLastKnownOppLoc[ iTeam ][ cnt ] = NOWHERE;
		}
	}

	// initialize public last known locations for all teams
	for (cnt = 0; cnt < MAX_NUM_SOLDIERS; cnt++)
	{
		for (cnt2 = 0; cnt2 < NUM_WATCHED_LOCS; cnt2++ )
		{
			gsWatchedLoc[ cnt ][ cnt2 ] = NOWHERE;
			gubWatchedLocPoints[ cnt ][ cnt2 ] = 0;
			gfWatchedLocReset[ cnt ][ cnt2 ] = FALSE;
		}
	}

	for ( cnt = 0; cnt < SHOULD_BECOME_HOSTILE_SIZE; cnt++ )
	{
		gShouldBecomeHostileOrSayQuote[cnt] = NULL;
	}

	gubNumShouldBecomeHostileOrSayQuote = 0;
}



void InitSoldierOppList(SOLDIERTYPE& s)
{
	std::fill(std::begin(s.bOppList), std::end(s.bOppList), NOT_HEARD_OR_SEEN);
	s.bOppCnt = 0;
	ResetLastKnownLocs(s);
	std::fill_n(gbSeenOpponents[s.ubID], MAXMERCS, 0);
}


void BetweenTurnsVisibilityAdjustments()
{
	// Make all soldiers on other teams, that are no longer seen, not visible
	FOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE& s = *i;
		if (!IsSoldierValidForSightings(s)) continue;
		if (IsOnOurOrMilitiaTeam(s))        continue;
		// Check if anyone on our team currently sees him (exclude NOBODY)
		if (!TeamNoLongerSeesMan(OUR_TEAM, &s, 0, 0)) continue;
		// then our team has lost sight of him
		s.bVisible = -1; // Make him fully invisible
	}
}


static void SaySeenQuote(SOLDIERTYPE* pSoldier, BOOLEAN fSeenCreature, BOOLEAN fVirginSector)
{
	UINT8 ubNumEnemies = 0;
	UINT8 ubNumAllies = 0;

	if ( AreInMeanwhile( ) )
	{
		return;
	}

	// Check out for our under large fire quote
	if ( !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_IN_SHIT ) )
	{
		// Get total enemies.
		// Loop through all mercs in sector and count # of enemies
		FOR_EACH_MERC(i)
		{
			const SOLDIERTYPE* const t = *i;
			if (OK_ENEMY_MERC(t)) ++ubNumEnemies;
		}

		// OK, after this, check our guys
		FOR_EACH_MERC(i)
		{
			const SOLDIERTYPE* const t = *i;
			if (!OK_ENEMY_MERC(t) && t->bOppCnt >= ubNumEnemies / 2) ++ubNumAllies;
		}

		// now check!
		if ( ( pSoldier->bOppCnt - ubNumAllies ) > 2 )
		{
			// Say quote!
			TacticalCharacterDialogue( pSoldier, QUOTE_IN_TROUBLE_SLASH_IN_BATTLE );

			pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_IN_SHIT;

			return;
		}

	}


	if ( fSeenCreature == 1 )
	{

		// Is this our first time seeing them?
		if ( gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags & PROFILE_MISC_FLAG_HAVESEENCREATURE )
		{
			// Are there multiplaes and we have not said this quote during this battle?
			if ( !(	pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES ) )
			{
				// Check for multiples!
				ubNumEnemies = 0;

				// Get total enemies.
				// Loop through all mercs in sector and count # of enemies
				FOR_EACH_MERC(i)
				{
					const SOLDIERTYPE* const t = *i;
					if (OK_ENEMY_MERC(t) &&
						t->uiStatusFlags & SOLDIER_MONSTER &&
						pSoldier->bOppList[t->ubID] == SEEN_CURRENTLY)
					{
						++ubNumEnemies;
					}
				}

				if ( ubNumEnemies > 2 )
				{
					// Yes, set flag
					pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES;

					// Say quote
					TacticalCharacterDialogue( pSoldier, QUOTE_ATTACKED_BY_MULTIPLE_CREATURES );
				}
				else
				{
					TacticalCharacterDialogue( pSoldier, QUOTE_SEE_CREATURE );
				}
			}
			else
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_SEE_CREATURE );
			}
		}
		else
		{
			// Yes, set flag
			gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags |= PROFILE_MISC_FLAG_HAVESEENCREATURE;

			TacticalCharacterDialogue( pSoldier, QUOTE_FIRSTTIME_GAME_SEE_CREATURE );
		}
	}
	// 2 is for bloodcat...
	else if ( fSeenCreature == 2 )
	{
		TacticalCharacterDialogue( pSoldier, QUOTE_SPOTTED_BLOODCAT );
	}
	else
	{
		if ( fVirginSector )
		{
			// First time we've seen a guy this sector
			TacticalCharacterDialogue( pSoldier, QUOTE_SEE_ENEMY_VARIATION );
		}
		else
		{
			if(isEnglishVersion() || isChineseVersion())
			{
				if ( Random( 100 ) < 30 )
				{
					DoMercBattleSound( pSoldier, BATTLE_SOUND_ENEMY );
				}
				else
				{
					TacticalCharacterDialogue( pSoldier, QUOTE_SEE_ENEMY );
				}
			}
			else
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_SEE_ENEMY );
			}
		}
	}
}


static void OurTeamSeesSomeone(SOLDIERTYPE* pSoldier, INT8 bNumReRevealed, INT8 bNumNewEnemies)
{
	if ( gTacticalStatus.fVirginSector )
	{
		// If we are in NPC dialogue now... stop!
		DeleteTalkingMenu( );

		// Say quote!
		SaySeenQuote( pSoldier, gfPlayerTeamSawCreatures, TRUE);

		HaultSoldierFromSighting( pSoldier, TRUE );

		// Set virgin sector to false....
		gTacticalStatus.fVirginSector = FALSE;
	}
	else
	{
		if ( pSoldier->bTeam == OUR_TEAM )
		{
			// STOP IF WE WERE MOVING....
			/// Speek up!
			if ( bNumReRevealed > 0 && bNumNewEnemies == 0 )
			{
				DoMercBattleSound( pSoldier, BATTLE_SOUND_CURSE1 );
			}
			else
			{
				SaySeenQuote( pSoldier, gfPlayerTeamSawCreatures, FALSE);
			}

			HaultSoldierFromSighting( pSoldier, TRUE );

			if ( gTacticalStatus.fEnemySightingOnTheirTurn )
			{
				// Locate to our guy, then slide to enemy
				LocateSoldier(pSoldier, SETLOCATOR);

				// Now slide to other guy....
				SlideTo(gTacticalStatus.enemy_sighting_on_their_turn_enemy, SETLOCATOR);
			}

			// Unset User's turn UI
			UnSetUIBusy(pSoldier);
		}
	}

	// OK, check what music mode we are in, change to battle if we're in battle
	// If we are in combat....
	if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		// If we are NOT in any music mode...
		if ( gubMusicMode == MUSIC_NONE )
		{
			SetMusicMode( MUSIC_TACTICAL_BATTLE );
		}
	}
}


void RadioSightings(SOLDIERTYPE* const pSoldier, SOLDIERTYPE* const about, UINT8 ubTeamToRadioTo)
{
	INT32   iLoop;
	UINT8   start,end,revealedEnemies = 0,unknownEnemies = 0;
	//UINT8   oppIsCivilian;
	INT8    *pPersOL,*pbPublOL; //,dayQuote;
	BOOLEAN fContactSeen;
	BOOLEAN fSawCreatureForFirstTime = FALSE;

	SLOGD("RADIO SIGHTINGS: for {} about {}",
		pSoldier->ubID, SOLDIER2ID(about));

	gTacticalStatus.Team[pSoldier->bTeam].last_merc_to_radio = pSoldier;

	// who are we radioing about?
	if (about == EVERYBODY)
	{
		start = 0;
		end   = MAXMERCS;
	}
	else
	{
		start = about->ubID;
		end   = start + 1;
	}

	// hang a pointer to the start of our this guy's personal opplist
	pPersOL = &(pSoldier->bOppList[start]);

	// hang a pointer to the start of this guy's opponents in the public opplist
	pbPublOL = &(gbPublicOpplist[ubTeamToRadioTo][start]);

	SOLDIERTYPE* pOpponent = &GetMan(start);

	// loop through every one of this guy's opponents
	for (iLoop = start; iLoop < end; iLoop++,pOpponent++,pPersOL++,pbPublOL++)
	{
		fContactSeen = FALSE;
		SLOGD("RS: checking {}", pOpponent->ubID);


		// make sure this merc is active, here & still alive (unconscious OK)
		if (!IsSoldierValidForSightings(*pOpponent))
		{
			SLOGD("RS: inactive/notInSector/life {}", pOpponent->ubID);
			continue; // skip to the next merc
		}

		// if these two mercs are on the same SIDE, then they're NOT opponents
		// NEW: Apr. 21 '96: must allow ALL non-humans to get radioed about
		if ((pSoldier->bSide == pOpponent->bSide) && (pOpponent->uiStatusFlags & SOLDIER_PC))
		{
			SLOGD("RS: same side {}", pSoldier->bSide);
			continue; // skip to the next merc
		}

		// if we personally don't know a thing about this opponent
		if (*pPersOL == NOT_HEARD_OR_SEEN)
		{
			SLOGD("RS: not heard or seen");
			continue; // skip to the next opponent
		}

		// if personal knowledge is NOT more up to date and NOT the same as public
		if ((!gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pPersOL - OLDEST_HEARD_VALUE]) &&
			(*pbPublOL != *pPersOL))
		{
			SLOGD("RS: no new knowledge per {} pub {}", *pPersOL, *pbPublOL);
			continue; // skip to the next opponent
		}
		SLOGD("RS: made it!");

		// if it's our merc, and he currently sees this opponent
		if (IsOnOurTeam(*pSoldier) &&
			*pPersOL == SEEN_CURRENTLY &&
			pOpponent->bSide != pSoldier->bSide &&
			!pOpponent->bNeutral)
		{
			// don't care whether and how many new enemies are seen if everyone visible
			// and he's healthy enough to be a threat (so is worth talking about)

			// do the following if we're radioing to our own team; if radioing to militia
			// then alert them instead
			if ( ubTeamToRadioTo != MILITIA_TEAM )
			{
				if (!gbShowEnemies && (pOpponent->bLife >= OKLIFE))
				{
					// if this enemy has not been publicly seen or heard recently
					if (*pbPublOL == NOT_HEARD_OR_SEEN)
					{
						// chalk up another "unknown" enemy
						unknownEnemies++;

						fContactSeen = TRUE;

						// now the important part: does this enemy see him/her back?
						if (pOpponent->bOppList[pSoldier->ubID] != SEEN_CURRENTLY)
						{
							// EXPERIENCE GAIN (10): Discovered a new enemy without being seen
							StatChange(*pSoldier, EXPERAMT, 10, FROM_SUCCESS);
						}
					}
					else
					{

						// if he has publicly not been seen now, or anytime during this turn
						if ((*pbPublOL != SEEN_CURRENTLY) && (*pbPublOL != SEEN_THIS_TURN))
						{
							// chalk up another "revealed" enemy
							revealedEnemies++;
							fContactSeen = TRUE;
						}
					}

					if ( fContactSeen )
					{
						if ( pSoldier->bTeam == OUR_TEAM )
						{
							if ( gTacticalStatus.ubCurrentTeam != OUR_TEAM )
							{
								// Save some stuff!
								if (gTacticalStatus.fEnemySightingOnTheirTurn)
								{
									// this has already come up so turn OFF the
									// pause-all-anims flag for the previous
									// person and set it for this next person
									gTacticalStatus.enemy_sighting_on_their_turn_enemy->fPauseAllAnimation = FALSE;
								}
								else
								{
									gTacticalStatus.fEnemySightingOnTheirTurn = TRUE;
								}
								gTacticalStatus.enemy_sighting_on_their_turn_enemy = pOpponent;
								gTacticalStatus.uiTimeSinceDemoOn = GetJA2Clock( );

								pOpponent->fPauseAllAnimation = TRUE;

							}
						}

						if ( pOpponent->uiStatusFlags & SOLDIER_MONSTER )
						{
							gfPlayerTeamSawCreatures = TRUE;
						}

						// ATE: Added for bloodcat...
						if ( pOpponent->ubBodyType == BLOODCAT )
						{
							// 2 is for bloodcat
							gfPlayerTeamSawCreatures = 2;
						}

					}

					if ( pOpponent->uiStatusFlags & SOLDIER_MONSTER )
					{
						if ( !(gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags & PROFILE_MISC_FLAG_HAVESEENCREATURE) )
						{
							fSawCreatureForFirstTime = TRUE;
						}
					}

				}
			}
			else
			{
				// radioing to militia that we saw someone! alert them!
				if (IsTeamActive(MILITIA_TEAM) &&
					!gTacticalStatus.Team[MILITIA_TEAM].bAwareOfOpposition)
				{
					HandleInitialRedAlert(MILITIA_TEAM);
				}
			}
		} // end of our team's merc sees new opponent

		// IF WE'RE HERE, OUR PERSONAL INFORMATION IS AT LEAST AS UP-TO-DATE
		// AS THE PUBLIC KNOWLEDGE, SO WE WILL REPLACE THE PUBLIC KNOWLEDGE
		SLOGD("UPDATE PUBLIC: soldier {} SEEING soldier {}",
			pSoldier->ubID, pOpponent->ubID);
		UpdatePublic(ubTeamToRadioTo, pOpponent, *pPersOL,
				gsLastKnownOppLoc[pSoldier->ubID][pOpponent->ubID],
				gbLastKnownOppLevel[pSoldier->ubID][pOpponent->ubID]);
	}


	// if soldier heard a misc noise more important that his team's public one
	if (pSoldier->ubNoiseVolume > gubPublicNoiseVolume[ubTeamToRadioTo])
	{
		// replace the soldier's team's public noise with his
		gsPublicNoiseGridno[ubTeamToRadioTo] 	= pSoldier->sNoiseGridno;
		gbPublicNoiseLevel[ubTeamToRadioTo] 	= pSoldier->bNoiseLevel;
		gubPublicNoiseVolume[ubTeamToRadioTo] 	= pSoldier->ubNoiseVolume;
	}


	// if this soldier is on the local team
	if (IsOnOurTeam(*pSoldier))
	{
		// don't trigger sighting quotes or stop merc's movement if everyone visible
		//if (!(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
		{
			// if we've revealed any enemies, or seen any previously unknown enemies
			if (revealedEnemies || unknownEnemies)
			{
				// First check for a virgin map and set to false if we see our first guy....
				// Only if this guy is an ememy!

				OurTeamSeesSomeone( pSoldier, revealedEnemies, unknownEnemies );
			}
			else if (fSawCreatureForFirstTime)
			{
				gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags |= PROFILE_MISC_FLAG_HAVESEENCREATURE;
				TacticalCharacterDialogue( pSoldier, QUOTE_FIRSTTIME_GAME_SEE_CREATURE );
			}

		}
	}
}



void DebugSoldierPage1()
{
	INT32 const h = DEBUG_PAGE_LINE_HEIGHT;

	const SOLDIERTYPE* const s = gUIFullTarget;
	if (s != NULL)
	{
		MPageHeader(ST::format("DEBUG SOLDIER PAGE ONE, GRIDNO {}", s->sGridNo));

		INT32 y = DEBUG_PAGE_START_Y;

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "ID:",   s->ubID);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "TEAM:", s->bTeam);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "SIDE:", s->bSide);

		MHeader(DEBUG_PAGE_FIRST_COLUMN, y +=  h, "STATUS FLAGS:");
		MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{x}", s->uiStatusFlags));

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "HUMAN:",    gTacticalStatus.Team[s->bTeam].bHuman);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "APs:",      s->bActionPoints);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Breath:",   s->bBreath);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Life:",     s->bLife);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "LifeMax:",  s->bLifeMax);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Bleeding:", s->bBleeding);

		y = DEBUG_PAGE_START_Y;

		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Agility:",               s->bAgility,      EffectiveAgility(s));
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Dexterity:",             s->bDexterity,    EffectiveDexterity(s));
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Strength:",              s->bStrength);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Wisdom:",                s->bWisdom,       EffectiveWisdom(s));
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Exp Lvl:",               s->bExpLevel,     EffectiveExpLevel(s));
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Mrksmnship",             s->bMarksmanship, EffectiveMarksmanship(s));
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Mechanical:",            s->bMechanical);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Explosive:",             s->bExplosive);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Medical:",               s->bMedical);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Drug Effects:",          s->bDrugEffect[0]);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Drug Side Effects:",     s->bDrugSideEffect[0]);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Booze Effects:",         s->bDrugEffect[1]);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Hangover Side Effects:", s->bDrugSideEffect[1]);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "AI has Keys:",           s->bHasKeys);
	}
	else if (guiCurrentCursorGridNo != NOWHERE)
	{
		MPageHeader("DEBUG LAND PAGE ONE");
	}
}

void DebugSoldierPage2()
{
	static const char* const gzDirectionStr[] =
	{
		"NORTHEAST",
		"EAST",
		"SOUTHEAST",
		"SOUTH",
		"SOUTHWEST",
		"WEST",
		"NORTHWEST",
		"NORTH"
	};

	INT32 const h = DEBUG_PAGE_LINE_HEIGHT;

	const SOLDIERTYPE* const s = gUIFullTarget;
	if (s != NULL)
	{
		MPageHeader(ST::format("DEBUG SOLDIER PAGE TWO, GRIDNO {}", s->sGridNo));

		INT32 y = DEBUG_PAGE_START_Y;

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "ID:", s->ubID);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Body Type:", s->ubBodyType);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Opp Cnt:", s->bOppCnt);

		ST::string opp_header;
		INT8    const* opp_list = s->bOppList;
		if (IsOnOurOrMilitiaTeam(*s)) // look at 8 to 15 opplist entries
		{
			opp_header = "Opplist B:";
			opp_list += 20;
		}
		else	// team 1 - enemies so look at first 8 (0-7) opplist entries
		{
			opp_header = "OppList A:";
		}
		MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, opp_header);
		MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} {} {} {} {} {} {} {}",
			opp_list[0], opp_list[1], opp_list[2], opp_list[3],
			opp_list[4], opp_list[5], opp_list[6], opp_list[7])
		);

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Visible:",     s->bVisible);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Direction:",   gzDirectionStr[s->bDirection]);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "DesDirection", gzDirectionStr[s->bDesiredDirection]);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Dest:",        s->sFinalDestination);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Path Size:",   s->ubPathDataSize);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Path Index:",  s->ubPathIndex);

		MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "First 3 Steps:");
		MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} {} {}",
			s->ubPathingData[0],
			s->ubPathingData[1],
			s->ubPathingData[2])
		);

		MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Next 3 Steps:");
		MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} {} {}",
			s->ubPathingData[s->ubPathIndex],
			s->ubPathingData[s->ubPathIndex + 1],
			s->ubPathingData[s->ubPathIndex + 2])
		);

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "FlashInd:",    s->fFlashLocator);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "ShowInd:",     s->fShowLocator);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Main hand:",   GCM->getItem(s->inv[HANDPOS].usItem)->getShortName());
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Second hand:", GCM->getItem(s->inv[SECONDHANDPOS].usItem)->getShortName());

		const GridNo map_pos = guiCurrentCursorGridNo;
		if (map_pos != NOWHERE)
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "CurrGridNo:", map_pos);
		}
	}
	else
	{
		const GridNo map_pos = guiCurrentCursorGridNo;
		if (map_pos == NOWHERE) return;

		MPageHeader("DEBUG LAND PAGE TWO");

		INT32                    y  = DEBUG_PAGE_START_Y;
		MAP_ELEMENT const* const me = &gpWorldLevelData[map_pos];

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Land Raised:", me->sHeight);

		LEVELNODE const* const land_head = me->pLandHead;
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Land Node:", land_head);
		if (land_head != NULL)
		{
			UINT16 const idx = land_head->usIndex;
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Land Node:", idx);
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Full Land:", gTileDatabase[idx].ubFullTile);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Land St Node:", me->pLandStart);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "GRIDNO:",       map_pos);

		if (me->uiFlags & MAPELEMENT_MOVEMENT_RESERVED)
		{
			MPrint(  DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y += h, ST::format("Merc: {}",  me->ubReservedSoldierID));
			MPrint( DEBUG_PAGE_FIRST_COLUMN, y,      "RESERVED MOVEMENT FLAG ON:");
		}

		LEVELNODE const* const node = GetCurInteractiveTile();
		if (node != NULL)
		{
			MPrint(  DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y += h, ST::format("Tile: {}", node->usIndex));
			MPrint( DEBUG_PAGE_FIRST_COLUMN, y,      "ON INT TILE");
		}

		if (me->uiFlags & MAPELEMENT_REVEALED)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, "REVEALED");
		}

		if (me->uiFlags & MAPELEMENT_RAISE_LAND_START)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, "Land Raise Start");
		}

		if (me->uiFlags & MAPELEMENT_RAISE_LAND_END)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, "Raise Land End");
		}

		if (gubWorldRoomInfo[map_pos] != NO_ROOM)
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Room Number", gubWorldRoomInfo[map_pos]);
		}

		if (me->ubExtFlags[0] & MAPELEMENT_EXT_NOBURN_STRUCT)
		{
			MPrint(0, y += h, "Don't Use Burn Through For Soldier");
		}
	}
}


void DebugSoldierPage3()
{
	static const char* const gzAlertStr[] =
	{
		"GREEN",
		"YELLOW",
		"RED",
		"BLACK"
	};

	INT32 const h = DEBUG_PAGE_LINE_HEIGHT;

	const SOLDIERTYPE* const s = gUIFullTarget;
	if (s != NULL)
	{
		MPageHeader(ST::format("DEBUG SOLDIER PAGE THREE, GRIDNO {}", s->sGridNo));

		INT32 y = DEBUG_PAGE_START_Y;

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "ID:",     s->ubID);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Action:", gzActionStr[s->bAction]);

		if (s->uiStatusFlags & SOLDIER_ENEMY)
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Alert:", gzAlertStr[s->bAlertStatus]);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Action Data:", s->usActionData);

		if (s->uiStatusFlags & SOLDIER_ENEMY)
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "AIMorale", s->bAIMorale);
		}
		else
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Morale", s->bMorale);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Delayed Movement:", s->fDelayedMovement);

		if (gubWatchedLocPoints[s->ubID][0] > 0)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, ST::format("Watch {}/{} for {} pts",
				gsWatchedLoc[s->ubID][0],
				gbWatchedLocLevel[s->ubID][0],
				gubWatchedLocPoints[s->ubID][0])
			);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "ActionInProg:", s->bActionInProgress);

		if (gubWatchedLocPoints[s->ubID][1] > 0)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y += h, ST::format("Watch {}/{} for {} pts",
				gsWatchedLoc[s->ubID][1],
				gbWatchedLocLevel[s->ubID][1],
				gubWatchedLocPoints[s->ubID][1])
			);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Last Action:", gzActionStr[s->bLastAction]);

		if (gubWatchedLocPoints[s->ubID][2] > 0)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y += h, ST::format("Watch {}/{} for {} pts",
				gsWatchedLoc[s->ubID][2],
				gbWatchedLocLevel[s->ubID][2],
				gubWatchedLocPoints[s->ubID][2])
			);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Animation:",   gAnimControl[s->usAnimState].zAnimStr);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Getting Hit:", s->fGettingHit);

		if (s->ubCivilianGroup != 0)
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Civ group", s->ubCivilianGroup);
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Suppress pts:",       s->ubSuppressionPoints);
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Attacker ID:",        SOLDIER2ID(s->attacker));
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "EndAINotCalled:",     s->fTurnInProgress);

		y = DEBUG_PAGE_START_Y;

		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "PrevAnimation:",      gAnimControl[s->usOldAniState].zAnimStr);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "PrevAniCode:",        gusAnimInst[s->usOldAniState][s->sOldAniCode]);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "AniCode:",            gusAnimInst[s->usAnimState][s->usAniCode]);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "No APS To fin Move:", s->fNoAPToFinishMove);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Bullets out:",        s->bBulletsLeft);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "Anim non-int:",       s->fInNonintAnim);
		MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "RT Anim non-int:",    s->fRTInNonintAnim);

		// OPINION OF SELECTED MERC
		const SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel != NULL && s->ubProfile != NO_PROFILE)
		{
			MercProfile const p = MercProfile(s->ubProfile);
			if (p.isPlayerMerc() || p.isRPC())
			{
				MPrintStat(DEBUG_PAGE_SECOND_COLUMN, y += h, "NPC Opinion:", GetProfile(s->ubProfile).bMercOpinion[sel->ubProfile]);
			}
		}
	}
	else
	{
		const GridNo map_pos = guiCurrentCursorGridNo;
		if (map_pos == NOWHERE) return;

		MPageHeader("DEBUG LAND PAGE THREE");

		INT32 y = DEBUG_PAGE_START_Y;

		// OK, display door information here.....
		DOOR_STATUS const* const door = GetDoorStatus(map_pos);
		if (door == NULL)
		{
			MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "No Door Status");
		}
		else
		{
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Door Status Found:", map_pos);

			ST::string door_state =
				door->ubFlags & DOOR_OPEN ? "OPEN" : "CLOSED";
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Actual Status:", door_state);

			ST::string perceived_state =
				door->ubFlags & DOOR_PERCEIVED_NOTSET ? "NOT SET" :
				door->ubFlags & DOOR_PERCEIVED_OPEN   ? "OPEN"    :
									"CLOSED";
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Perceived Status:", perceived_state);
		}

		//Find struct data and se what it says......
		STRUCTURE const* const structure = FindStructure(map_pos, STRUCTURE_ANYDOOR);
		if (structure == NULL)
		{
			MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "No Door Struct Data");
		}
		else
		{
			ST::string structure_state =
				structure->fFlags & STRUCTURE_OPEN ? "OPEN" : "CLOSED";
			MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "State:", structure_state);
		}
	}
}


static void AppendAttachmentCode(UINT16 item, ST::string& str)
{
	switch (item)
	{
		case SILENCER:    str += " Sil"; break;
		case SNIPERSCOPE: str += " Scp"; break;
		case BIPOD:       str += " Bip"; break;
		case LASERSCOPE:  str += " Las"; break;
	}
}


static void WriteQuantityAndAttachments(OBJECTTYPE const* o, INT32 const y)
{
	//100%  Qty: 2  Attach:
	//100%  Qty: 2
	//100%  Attach:
	//100%
	if (GCM->getItem(o->usItem)->getItemClass() == IC_AMMO)
	{
		if (o->ubNumberOfObjects > 1)
		{
			ST::string str = ST::format("Clips:  {}  ({}", o->ubNumberOfObjects, o->bStatus[0]);
			for (UINT8 i = 1; i < o->ubNumberOfObjects; ++i)
			{
				ST::string temp = ST::format(", {}", o->bStatus[0]);
				str += temp;
			}
			str += ")";
			MPrint(DEBUG_PAGE_SECOND_COLUMN, y, str);
		}
		else
		{
			MPrint(DEBUG_PAGE_SECOND_COLUMN, y, ST::format("{} rounds", o->bStatus[0]));
		}
	}
	else
	{
		// Build attachment string
		ST::string attachments;
		if (o->usAttachItem[0] ||
				o->usAttachItem[1] ||
				o->usAttachItem[2] ||
				o->usAttachItem[3])
		{
			attachments = "  (";
			AppendAttachmentCode(o->usAttachItem[0], attachments);
			AppendAttachmentCode(o->usAttachItem[1], attachments);
			AppendAttachmentCode(o->usAttachItem[2], attachments);
			AppendAttachmentCode(o->usAttachItem[3], attachments);
			attachments += " )";
		}
		else
		{
			attachments.clear();
		}

		if (o->ubNumberOfObjects > 1)
		{
			//everything
			MPrint(DEBUG_PAGE_SECOND_COLUMN, y, ST::format("{}%  Qty:  {}{}", o->bStatus[0], o->ubNumberOfObjects, attachments));
		}
		else
		{
			//condition and attachments
			MPrint(DEBUG_PAGE_SECOND_COLUMN, y, ST::format("{}%{}", o->bStatus[0], attachments));
		}
	}
}


static void PrintItem(INT32 y, const ST::string& header, const OBJECTTYPE* o)
{
	MHeader(DEBUG_PAGE_FIRST_COLUMN, y, header);
	if (!o->usItem) return;
	MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{}", GCM->getItem(o->usItem)->getShortName()));
	WriteQuantityAndAttachments(o, y);
}


void DebugSoldierPage4()
{
	const SOLDIERTYPE* const s = gUIFullTarget;

	INT32 const h = DEBUG_PAGE_LINE_HEIGHT;

	if (s != NULL)
	{
		MPageHeader(ST::format("DEBUG SOLDIER PAGE FOUR, GRIDNO {}", s->sGridNo));

		INT32 y = DEBUG_PAGE_START_Y;

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Exp. Level:", s->bExpLevel);
		ST::string sclass;
		switch (s->ubSoldierClass)
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				sclass = "(Administrator)";
				break;
			case SOLDIER_CLASS_ELITE:
				sclass = "(Army Elite)";
				break;
			case SOLDIER_CLASS_ARMY:
				sclass = "(Army Troop)";
				break;
			case SOLDIER_CLASS_CREATURE:
				sclass = "(Creature)";
				break;
			case SOLDIER_CLASS_GREEN_MILITIA:
				sclass = "(Green Militia)";
				break;
			case SOLDIER_CLASS_REG_MILITIA:
				sclass = "(Reg Militia)";
				break;
			case SOLDIER_CLASS_ELITE_MILITIA:
				sclass = "(Elite Militia)";
				break;
			case SOLDIER_CLASS_MINER:
				sclass = "(Miner)";
				break;
			// don't care (don't write anything)
			default:
				sclass.clear();
				break;
		}
		if (!sclass.empty())
			MPrint(DEBUG_PAGE_SECOND_COLUMN, y, ST::format("{}", sclass));

		if (s->bTeam != OUR_TEAM)
		{
			ST::string orders;
			switch (s->bOrders)
			{
				case STATIONARY:  orders = "STATIONARY";    break;
				case ONGUARD:     orders = "ON GUARD";      break;
				case ONCALL:      orders = "ON CALL";       break;
				case SEEKENEMY:   orders = "SEEK ENEMY";    break;
				case CLOSEPATROL: orders = "CLOSE PATROL";  break;
				case FARPATROL:   orders = "FAR PATROL";    break;
				case POINTPATROL: orders = "POINT PATROL";  break;
				case RNDPTPATROL: orders = "RND PT PATROL"; break;
				default:          orders = "UNKNOWN";       break;
			}
			ST::string attitude;
			switch (s->bAttitude)
			{
				case DEFENSIVE:   attitude = "DEFENSIVE";    break;
				case BRAVESOLO:   attitude = "BRAVE SOLO";   break;
				case BRAVEAID:    attitude = "BRAVE AID";    break;
				case AGGRESSIVE:  attitude = "AGGRESSIVE";   break;
				case CUNNINGSOLO: attitude = "CUNNING SOLO"; break;
				case CUNNINGAID:  attitude = "CUNNING AID";  break;
				default:          attitude = "UNKNOWN";      break;
			}
			SOLDIERINITNODE const* const node = FindSoldierInitNodeBySoldier(*s);
			y += h;
			if (node)
			{
				MPrint(DEBUG_PAGE_FIRST_COLUMN, y, ST::format("{}, {}, REL EQUIP: {}, REL ATTR: {}",
					orders, attitude,
					node->pBasicPlacement->bRelativeEquipmentLevel,
					node->pBasicPlacement->bRelativeAttributeLevel)
				);
			}
			else
			{
				MPrint(DEBUG_PAGE_FIRST_COLUMN, y, ST::format("{}, {}", orders, attitude));
			}
		}

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "ID:", s->ubID);

		PrintItem(y += h, "HELMETPOS:",     &s->inv[HELMETPOS]);
		PrintItem(y += h, "VESTPOS:",       &s->inv[VESTPOS]);
		PrintItem(y += h, "LEGPOS:",        &s->inv[LEGPOS]);
		PrintItem(y += h, "HEAD1POS:",      &s->inv[HEAD1POS]);
		PrintItem(y += h, "HEAD2POS:",      &s->inv[HEAD2POS]);
		PrintItem(y += h, "HANDPOS:",       &s->inv[HANDPOS]);
		PrintItem(y += h, "SECONDHANDPOS:", &s->inv[SECONDHANDPOS]);
		PrintItem(y += h, "BIGPOCK1POS:",   &s->inv[BIGPOCK1POS]);
		PrintItem(y += h, "BIGPOCK2POS:",   &s->inv[BIGPOCK2POS]);
		PrintItem(y += h, "BIGPOCK3POS:",   &s->inv[BIGPOCK3POS]);
		PrintItem(y += h, "BIGPOCK4POS:",   &s->inv[BIGPOCK4POS]);
		PrintItem(y += h, "SMALLPOCK1POS:", &s->inv[SMALLPOCK1POS]);
		PrintItem(y += h, "SMALLPOCK2POS:", &s->inv[SMALLPOCK2POS]);
		PrintItem(y += h, "SMALLPOCK3POS:", &s->inv[SMALLPOCK3POS]);
		PrintItem(y += h, "SMALLPOCK4POS:", &s->inv[SMALLPOCK4POS]);
		PrintItem(y += h, "SMALLPOCK5POS:", &s->inv[SMALLPOCK5POS]);
		PrintItem(y += h, "SMALLPOCK6POS:", &s->inv[SMALLPOCK6POS]);
		PrintItem(y += h, "SMALLPOCK7POS:", &s->inv[SMALLPOCK7POS]);
		PrintItem(y += h, "SMALLPOCK8POS:", &s->inv[SMALLPOCK8POS]);
	}
	else
	{
		MPageHeader("DEBUG LAND PAGE FOUR");
	}
}


//
// Noise stuff
//

UINT8 MovementNoise(SOLDIERTYPE const * const pSoldier)
{
	constexpr UINT8 MAX_MOVEMENT_NOISE = 9;

	if ( pSoldier->bTeam == ENEMY_TEAM )
	{
		return( (UINT8) (MAX_MOVEMENT_NOISE - PreRandom( 2 )) );
	}

	INT32 iStealthSkill = 20 + 4 * EffectiveExpLevel( pSoldier ) + ((EffectiveDexterity( pSoldier ) * 4) / 10); // 24-100

	// big bonus for those "extra stealthy" mercs
	if ( pSoldier->ubBodyType == BLOODCAT )
	{
		iStealthSkill += 50;
	}
	else
	{
		iStealthSkill += 25 * NUM_SKILL_TRAITS(pSoldier, STEALTHY);
	}

	UINT8 const ubBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;
	UINT8 const ubEffLife = pSoldier->bLife + (ubBandaged / 2);

	// IF "SNEAKER'S" "EFFECTIVE LIFE" IS AT LESS THAN 50
	if (ubEffLife < 50)
	{
		// reduce effective stealth skill by up to 50% for low life
		iStealthSkill -= (iStealthSkill * (50 - ubEffLife)) / 100;
	}

	// if breath is below 50%
	if (pSoldier->bBreath < 50)
	{
		// reduce effective stealth skill by up to 50%
		iStealthSkill -= (iStealthSkill * (50 - pSoldier->bBreath)) / 100;
	}

	// if sneaker is moving through water
	// DeepWater() must be checked first because Water() matches all types of water
	if (DeepWater(pSoldier->sGridNo))
	{
		iStealthSkill -= 20;
	}
	else if (Water(pSoldier->sGridNo))
	{
		iStealthSkill -= 10;
	}

	// minus 3 percent per bonus AP from adrenaline
	iStealthSkill -= 3 * pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ];

	iStealthSkill = std::max(iStealthSkill, 0);

	UINT8 ubVolume;
	if (!pSoldier->bStealthMode)	// REGULAR movement
	{
		UINT8 ubMaxVolume = MAX_MOVEMENT_NOISE - (iStealthSkill / 16); // 9 - (0 to 9) => 0 to 9

		if (Water(pSoldier->sGridNo))
		{
			ubMaxVolume++; // in water, can be even louder
		}
		switch (pSoldier->usAnimState)
		{
			case CRAWLING:
				ubMaxVolume = ubMaxVolume <= 2 ? 0 : ubMaxVolume - 2;
				break;
			case SWATTING:
				ubMaxVolume = ubMaxVolume <= 1 ? 0 : ubMaxVolume - 1;
				break;
			case RUNNING:
				ubMaxVolume += 3;
				break;
		}

		if (ubMaxVolume < 2)
		{
			ubVolume = ubMaxVolume;
		}
		else
		{
			ubVolume = 1 + (UINT8) PreRandom(ubMaxVolume); // actual volume is 1 to max volume
		}
	}
	else // in STEALTH mode
	{
		INT32 iRoll = (INT32) PreRandom(100);	// roll them bones!

		if (iRoll >= iStealthSkill)   // v1.13 modification: give a second chance!
		{
			iRoll = (INT32) PreRandom(100);
		}

		if (iRoll < iStealthSkill)
		{
			ubVolume = 0;	// made it, stayed quiet moving through this tile
		}
		else	// OOPS!
		{
			ubVolume = 1 + ((iRoll - iStealthSkill + 1) / 16);	// volume is 1 - 7 ...
			switch (pSoldier->usAnimState)
			{
				case CRAWLING:
					ubVolume = ubVolume <= 2 ? 0 : ubVolume - 2;
					break;
				case SWATTING:
					ubVolume -= 1;
					break;
				case RUNNING:
					ubVolume += 3;
					break;
			}
		}
	}

	return( ubVolume );
}

UINT8 DoorOpeningNoise( SOLDIERTYPE *pSoldier )
{
	INT16 sGridNo;
	DOOR_STATUS *pDoorStatus;
	UINT8 ubDoorNoise;

	// door being opened gridno is always the pending-action-data2 value
	sGridNo = pSoldier->sPendingActionData2;
	pDoorStatus = GetDoorStatus( sGridNo );

	if ( pDoorStatus && pDoorStatus->ubFlags & DOOR_HAS_TIN_CAN )
	{
		// double noise possible!
		ubDoorNoise = DOOR_NOISE_VOLUME * 3;
	}
	else
	{
		ubDoorNoise = DOOR_NOISE_VOLUME;
	}
	if ( MovementNoise( pSoldier ) )
	{
		// failed any stealth checks
		return( ubDoorNoise );
	}
	else
	{
		// succeeded in being stealthy!
		return( 0 );
	}
}


void MakeNoise(SOLDIERTYPE* const noise_maker, INT16 const sGridNo, INT8 const bLevel, UINT8 const ubVolume, NoiseKind const ubNoiseType)
{
	if ( gTacticalStatus.ubAttackBusyCount )
	{
		// delay these events until the attack is over!
		EV_S_NOISE SNoise;
		SNoise.ubNoiseMaker = SOLDIER2ID(noise_maker);
		SNoise.sGridNo      = sGridNo;
		SNoise.bLevel       = bLevel;
		SNoise.ubVolume     = ubVolume;
		SNoise.ubNoiseType  = ubNoiseType;
		AddGameEvent(SNoise, DEMAND_EVENT_DELAY);
	}
	else
	{
		OurNoise(noise_maker, sGridNo, bLevel, ubVolume, ubNoiseType);
	}
}


static void ProcessNoise(SOLDIERTYPE* noise_maker, INT16 sGridNo, INT8 bLevel, UINT8 ubBaseVolume, NoiseKind);


void OurNoise(SOLDIERTYPE* const noise_maker, INT16 const sGridNo, INT8 const bLevel, UINT8 const ubVolume, NoiseKind const ubNoiseType)
{
#ifdef BYPASSNOISE
	return;
#endif

	// see if anyone actually hears this noise, sees noise_maker, etc.
	ProcessNoise(noise_maker, sGridNo, bLevel, ubVolume, ubNoiseType);

	if (noise_maker != NULL &&
		(gTacticalStatus.uiFlags & INCOMBAT) &&
		!gfDelayResolvingBestSightingDueToDoor)
	{
		// interrupts are possible, resolve them now (we're in control here)
		// (you can't interrupt NOBODY, even if you hear the noise)
		ResolveInterruptsVs(noise_maker, NOISEINTERRUPT);
	}
}


static void HearNoise(SOLDIERTYPE* pSoldier, SOLDIERTYPE* noise_maker, UINT16 sGridNo, INT8 bLevel, UINT8 ubVolume, NoiseKind, BOOLEAN* ubSeen);
static UINT8 CalcEffVolume(const SOLDIERTYPE* pSoldier, INT16 sGridNo, INT8 bLevel, NoiseKind, UINT8 ubBaseVolume, UINT8 ubTerrType1, UINT8 ubTerrType2);
static void TellPlayerAboutNoise(SOLDIERTYPE* pSoldier, const SOLDIERTYPE* noise_maker, INT16 sGridNo, INT8 bLevel, UINT8 ubVolume, NoiseKind, UINT8 ubNoiseDir);


static void ProcessNoise(SOLDIERTYPE* const noise_maker, INT16 const sGridNo, INT8 const bLevel, UINT8 const ubBaseVolume, NoiseKind const ubNoiseType)
{
	UINT8 ubLoudestEffVolume, ubEffVolume;
	//UINT8 ubPlayVolume;
	UINT8 ubNoiseDir        = (UINT8)-1; // XXX HACK000E probably ubLoudestNoiseDir should be used
	UINT8 ubLoudestNoiseDir = (UINT8)-1; // XXX HACK000E

	// if the base volume itself was negligible
	if (!ubBaseVolume)
		return;


	// EXPLOSIONs are special, because they COULD be caused by a merc who is
	// no longer alive (but he placed the bomb or flaky grenade in the past).
	// Later noiseMaker gets whacked to NOBODY anyway, so that's OK.  So a
	// dead noiseMaker is only used here to decide WHICH soldiers HearNoise().

	// if noise is made by a person, AND it's not noise from an explosion
	if (noise_maker != NULL && ubNoiseType != NOISE_EXPLOSION)
	{
		// inactive/not in sector/dead soldiers, shouldn't be making noise!
		if (!noise_maker->bActive ||
			!noise_maker->bInSector ||
			noise_maker->uiStatusFlags & SOLDIER_DEAD)
		{
			return;
		}

		// if he's out of life, and this isn't just his "dying scream" which is OK
		if (noise_maker->bLife == 0 && ubNoiseType != NOISE_SCREAM)
		{
			return;
		}
	}


	// DETERMINE THE TERRAIN TYPE OF THE GRIDNO WHERE NOISE IS COMING FROM
	const UINT8 ubSourceTerrType = gpWorldLevelData[sGridNo].ubTerrainID;


	// DETERMINE THE *PERCEIVED* SOURCE OF THE NOISE
	SOLDIERTYPE* source;
	switch (ubNoiseType)
	{
		// for noise generated by an OBJECT shot/thrown/dropped by the noiseMaker
		case NOISE_ROCK_IMPACT:
			gWhoThrewRock = noise_maker;
			/* FALLTHROUGH */
		case NOISE_BULLET_IMPACT:
		case NOISE_GRENADE_IMPACT:
		case NOISE_EXPLOSION:
			// the source of the noise is not at all obvious, so hide it from
			// the listener and maintain noiseMaker's cover by making source NOBODY
			source = NULL;
			break;

		default:
			// normal situation: the noiseMaker is obviously the source of the noise
			source = noise_maker;
			break;
	}

	// LOOP THROUGH EACH TEAM
	for (UINT8 bTeam = 0; bTeam < MAXTEAMS; ++bTeam)
	{
		// skip any inactive teams
		if (!IsTeamActive(bTeam)) continue;

		// if a the noise maker is a person, not just NOBODY
		if (noise_maker != NULL)
		{
			// if this team is the same TEAM as the noise maker's
			// (for now, assume we will report noises by unknown source on same SIDE)
			// OR, if the noise maker is currently in sight to this HUMAN team

			// CJC: changed to if the side is the same side as the noise maker's!
			// CJC: changed back!

			if (bTeam == noise_maker->bTeam) continue;

			if (gTacticalStatus.Team[bTeam].bHuman &&
					gbPublicOpplist[bTeam][noise_maker->ubID] == SEEN_CURRENTLY)
			{
				continue;
			}
		}

		// tell player about noise if enemies are present
		BOOLEAN bTellPlayer = gTacticalStatus.fEnemyInSector && ( !(gTacticalStatus.uiFlags & INCOMBAT) || (gTacticalStatus.ubCurrentTeam) );

		switch (ubNoiseType)
		{
			case NOISE_GUNFIRE:
			case NOISE_BULLET_IMPACT:
			case NOISE_ROCK_IMPACT:
			case NOISE_GRENADE_IMPACT:
				// It's noise caused by a projectile.  If the projectile was seen by
				// the local player while in flight (PublicBullet), then don't bother
				// giving him a message about the noise it made, he's obviously aware.
				if (1 /*PublicBullet*/)
				{
					bTellPlayer = FALSE;
				}

				break;

			case NOISE_EXPLOSION:
				// if center of explosion is in visual range of team, don't report
				// noise, because the player is already watching the thing go BOOM!
				if (TeamMemberNear(bTeam,sGridNo,STRAIGHT))
				{
					bTellPlayer = FALSE;
				}
				break;

			case NOISE_SILENT_ALARM:
				bTellPlayer = FALSE;
				break;
			default:
				break;
		}

		// if noise was made by a person
		if (noise_maker != NULL)
		{
			switch (gbPublicOpplist[bTeam][noise_maker->ubID])
			{
				case SEEN_CURRENTLY:
				case SEEN_THIS_TURN:
				case HEARD_THIS_TURN:
					// If noisemaker has been *PUBLICLY* SEEN OR HEARD during THIS TURN,
					// then don't bother reporting any noise made by him to the player
					bTellPlayer = FALSE;
					break;

				default:
					if (noise_maker->bVisible == TRUE && bTeam == OUR_TEAM)
					{
						SLOGD("Handling noise from person not currently seen in player's public opplist");
					}
					break;
			}

			if (noise_maker->bLife == 0)
			{
				// this guy is dead (just dying) so don't report to player
				bTellPlayer = FALSE;
			}
		}

		// refresh flags for this new team
		BOOLEAN bHeard = FALSE;
		BOOLEAN bSeen  = FALSE;
		ubLoudestEffVolume = 0;
		SOLDIERTYPE* heard_loudest_by = NULL;

		// All mercs on this team check if they are eligible to hear this noise
		FOR_EACH_IN_TEAM(pSoldier, bTeam)
		{
			// if this "listener" is in no condition to care
			if (!IsSoldierValidForSightings(*pSoldier) ||
				(pSoldier->bLife < OKLIFE) || pSoldier->ubBodyType == LARVAE_MONSTER)
			{
				continue; // skip him!
			}

			if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE && pSoldier->bTeam == OUR_TEAM  )
			{
				continue; // skip
			}

			if ( bTeam == OUR_TEAM && pSoldier->bAssignment == ASSIGNMENT_POW )
			{
				// POWs should not be processed for noise
				continue;
			}

			// if a the noise maker is a person, not just NOBODY
			if (noise_maker != NULL)
			{
				// if this listener can see this noise maker
				if (pSoldier->bOppList[noise_maker->ubID] == SEEN_CURRENTLY)
				{
					// civilians care about gunshots even if they come from someone they can see
					if ( !( pSoldier->bNeutral && ubNoiseType == NOISE_GUNFIRE ) )
					{
						continue; // then who cares whether he can also hear the guy?
					}
				}

				// screen out allied militia from hearing us
				switch (noise_maker->bTeam)
				{
					case OUR_TEAM:
						// if the listener is militia and still on our side, ignore noise from us
						if ( pSoldier->bTeam == MILITIA_TEAM && pSoldier->bSide == 0 )
						{
							continue;
						}
						break;
					case ENEMY_TEAM:
						switch( pSoldier->ubProfile )
						{
							case WARDEN:
							case GENERAL:
							case SERGEANT:
							case CONRAD:
								// ignore soldier team
								continue;
							default:
								break;
						}
						break;
					case MILITIA_TEAM:
						// if the noisemaker is militia and still on our side, ignore
						// noise if we're listening
						if (pSoldier->bTeam == OUR_TEAM && noise_maker->bSide == 0)
						{
							continue;
						}
						break;
				}

				if (gWorldSector.x == 5 && gWorldSector.y == MAP_ROW_N)
				{
					// in the bloodcat arena sector, skip noises between army & bloodcats
					if (pSoldier->bTeam == ENEMY_TEAM && noise_maker->bTeam == CREATURE_TEAM)
					{
						continue;
					}
					if (pSoldier->bTeam == CREATURE_TEAM && noise_maker->bTeam == ENEMY_TEAM)
					{
						continue;
					}
				}


			}
			else
			{
				// screen out allied militia from hearing us
				if (noise_maker == NULL && pSoldier->bTeam == MILITIA_TEAM && pSoldier->bSide == 0)
				{
					continue;
				}
			}

			if ((pSoldier->bTeam == CIV_TEAM) && (ubNoiseType == NOISE_GUNFIRE ||
				ubNoiseType == NOISE_EXPLOSION))
			{
				pSoldier->ubMiscSoldierFlags |= SOLDIER_MISC_HEARD_GUNSHOT;
			}

			// Can the listener hear noise of that volume given his circumstances?
			ubEffVolume = CalcEffVolume(pSoldier, sGridNo, bLevel, ubNoiseType,
							ubBaseVolume, pSoldier->bOverTerrainType, ubSourceTerrType);

			if (ubEffVolume > 0)
			{
				// ALL RIGHT!  Passed all the tests, this listener hears this noise!!!
				HearNoise(pSoldier, source, sGridNo, bLevel, ubEffVolume, ubNoiseType, &bSeen);

				bHeard = TRUE;

				ubNoiseDir = GetDirectionToGridNoFromGridNo(pSoldier->sGridNo, sGridNo);

				// check the 'noise heard & reported' bit for that soldier & direction
				if ( ubNoiseType != NOISE_MOVEMENT || bTeam != OUR_TEAM ||
					(pSoldier->bInterruptDuelPts != NO_INTERRUPT) ||
					!(pSoldier->ubMovementNoiseHeard & (1 << ubNoiseDir) ) )
				{
					if (ubEffVolume > ubLoudestEffVolume)
					{
						ubLoudestEffVolume = ubEffVolume;
						heard_loudest_by = pSoldier;
						ubLoudestNoiseDir = ubNoiseDir;
					}
				}

			}
			else
			{
				ubEffVolume = 0;
			}
		}


		// if the noise was heard at all
		if (bHeard)
		{
			// and we're doing our team
			if (bTeam == OUR_TEAM)
			{
				// if we are to tell the player about this type of noise
				if (bTellPlayer && heard_loudest_by != NULL)
				{
					// the merc that heard it the LOUDEST is the one to comment
					// should add level to this function call
					TellPlayerAboutNoise(heard_loudest_by, noise_maker, sGridNo, bLevel, ubLoudestEffVolume, ubNoiseType, ubLoudestNoiseDir);

					if ( ubNoiseType == NOISE_MOVEMENT)
					{
						heard_loudest_by->ubMovementNoiseHeard |= 1 << ubNoiseDir;
					}

				}
				//if ( !(pSoldier->ubMovementNoiseHeard & (1 << ubNoiseDir) ) )
			}
#ifdef REPORTTHEIRNOISE
			else // debugging: report noise heard by other team's soldiers
			{
				if (bTellPlayer)
				{
					TellPlayerAboutNoise(heard_loudest_by, noise_maker, sGridNo, bLevel, ubLoudestEffVolume, ubNoiseType, ubLoudestNoiseDir);
				}
			}
#endif
		}

		// if the listening team is human-controlled AND
		// the noise's source is another soldier
		// (computer-controlled teams don't radio or automatically report NOISE)
		if (source != NULL && gTacticalStatus.Team[bTeam].bHuman)
		{
			// if noise_maker was seen by at least one member of this team
			if (bSeen)
			{
				// this team is now allowed to report sightings and set Public flags
				OurTeamRadiosRandomlyAbout(source);
			}
			else // not seen
			{
				if (bHeard)
				{
					// mark noise maker as having been PUBLICLY heard THIS TURN
					UpdatePublic(bTeam, source, HEARD_THIS_TURN, sGridNo, bLevel);
				}
			}
		}
	}

	gWhoThrewRock = NULL;
}


static UINT8 CalcEffVolume(SOLDIERTYPE const* const pSoldier, INT16 const sGridNo, INT8 const bLevel, NoiseKind const ubNoiseType, UINT8 const ubBaseVolume, UINT8 const ubTerrType1, UINT8 const ubTerrType2)
{
	INT32 iEffVolume, iDistance;

	if (IsWearingHeadGear(*pSoldier, WALKMAN))
	{
		return( 0 );
	}

	if ( gTacticalStatus.uiFlags & INCOMBAT )
	{
		// ATE: Funny things happen to ABC stuff if bNewSituation set....
		if ( gTacticalStatus.ubCurrentTeam == pSoldier->bTeam )
		{
			return( 0 );
		}
	}

	// adjust default noise volume by listener's hearing capability
	iEffVolume = (INT32) ubBaseVolume + (INT32) DecideHearing( pSoldier );


	// effective volume reduced by listener's number of opponents in sight
	iEffVolume -= pSoldier->bOppCnt;


	// calculate the distance (in adjusted pixels) between the source of the
	// noise (gridno) and the location of the would-be listener (pSoldier->gridno)
	iDistance = (INT32) PythSpacesAway( pSoldier->sGridNo, sGridNo );

	// effective volume fades over distance beyond 1 tile away
	iEffVolume -= (iDistance - 1);

	/*
	if (pSoldier->bTeam == CIV_TEAM && pSoldier->ubBodyType != CROW )
	{
		if (pSoldier->ubCivilianGroup == 0 && pSoldier->ubProfile == NO_PROFILE)
		{
			// nameless civs reduce effective volume by 2 for gunshots etc
			// (double the reduction due to distance)
			// so that they don't cower from attacks that are really far away
			switch (ubNoiseType)
			{
				case NOISE_GUNFIRE:
				case NOISE_BULLET_IMPACT:
				case NOISE_GRENADE_IMPACT:
				case NOISE_EXPLOSION:
					iEffVolume -= iDistance;
					break;
				default:
					break;
			}
		}
		else if (pSoldier->bNeutral)
		{
			// NPCs and people in groups ignore attack noises unless they are no longer neutral
			switch (ubNoiseType)
			{
				case NOISE_GUNFIRE:
				case NOISE_BULLET_IMPACT:
				case NOISE_GRENADE_IMPACT:
				case NOISE_EXPLOSION:
					iEffVolume = 0;
					break;
				default:
					break;
			}
		}
	}*/

	if (pSoldier->usAnimState == RUNNING)
	{
		iEffVolume -= 5;
	}

#if 0 /* XXX Something is very wrong here... */
	if (pSoldier->bAssignment == SLEEPING )
	{
		// decrease effective volume since we're asleep!
		iEffVolume -= 5;
	}
#endif

	// check for floor/roof difference
	if (bLevel > pSoldier->bLevel)
	{
		// sound is amplified by roof
		iEffVolume += 5;
	}
	else if (bLevel < pSoldier->bLevel)
	{
		// sound is muffled
		iEffVolume -= 5;
	}

	// if we still have a chance of hearing this, and the terrain types are known
	if (iEffVolume > 0)
	{
			// if, between noise and listener, one is outside and one is inside

			// NOTE: This is a pretty dumb way of doing things, since it won't detect
			// the presence of walls between 2 spots both inside or both outside, but
			// given our current system it's the best that we can do

			if (((ubTerrType1 == FLAT_FLOOR) && (ubTerrType2 != FLAT_FLOOR)) ||
				((ubTerrType1 != FLAT_FLOOR) && (ubTerrType2 == FLAT_FLOOR)))
			{
				// sound is muffled, reduce the effective volume of the noise
				iEffVolume -= 5;
			}
	}
	if (iEffVolume > 0)
	{
		return( (UINT8) iEffVolume );
	}
	else
	{
		return( 0 );
	}
}


static void HearNoise(SOLDIERTYPE* const pSoldier, SOLDIERTYPE* const noise_maker, UINT16 const sGridNo, INT8 const bLevel, UINT8 const ubVolume, NoiseKind const ubNoiseType, BOOLEAN* const ubSeen)
{
	INT16   sNoiseX, sNoiseY;
	INT8    bHadToTurn = FALSE, bSourceSeen = FALSE;
	INT8    bOldOpplist;
	INT16   sDistVisible;
	INT8    bDirection;
	BOOLEAN fMuzzleFlash = FALSE;

	SLOGD("{} hears noise from {} ({}/{}) volume {}",
		pSoldier->ubID, SOLDIER2ID(noise_maker), sGridNo, bLevel, ubVolume);


	if ( pSoldier->ubBodyType == CROW )
	{
		CrowsFlyAway( pSoldier->bTeam );
		return;
	}

	// "Turn head" towards the source of the noise and try to see what's there

	// don't use DistanceVisible here, but use maximum visibility distance
	// in as straight line instead.  Represents guy "turning just his head"

	// CJC 97/10: CHANGE!  Since STRAIGHT can not reliably be used as a
	// max sighting distance (varies based on realtime/turnbased), call
	// the function with the new DIRECTION_IRRELEVANT define

	// is he close enough to see that gridno if he turns his head?

	// ignore muzzle flashes when turning head to see noise
	if (ubNoiseType == NOISE_GUNFIRE && noise_maker != NULL && noise_maker->fMuzzleFlash)
	{
		sNoiseX = CenterX(sGridNo);
		sNoiseY = CenterY(sGridNo);
		bDirection = atan8(pSoldier->sX,pSoldier->sY,sNoiseX,sNoiseY);
		if (pSoldier->bDirection != bDirection &&
			pSoldier->bDirection != OneCDirection(bDirection) &&
			pSoldier->bDirection != OneCCDirection(bDirection))
		{
			// temporarily turn off muzzle flash so DistanceVisible can be calculated without it
			noise_maker->fMuzzleFlash = FALSE;
			fMuzzleFlash = TRUE;
		}
	}

	sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, bLevel );

	if ( fMuzzleFlash )
	{
		// turn flash on again
		noise_maker->fMuzzleFlash = TRUE;
	}

	if (PythSpacesAway(pSoldier->sGridNo,sGridNo) <= sDistVisible )
	{
		// just use the XXadjustedXX center of the gridno
		sNoiseX = CenterX(sGridNo);
		sNoiseY = CenterY(sGridNo);

		if (pSoldier->bDirection != atan8(pSoldier->sX,pSoldier->sY,sNoiseX,sNoiseY))
		{
			bHadToTurn = TRUE;
		}
		else
		{
			bHadToTurn = FALSE;
		}

		// and we can trace a line of sight to his x,y coordinates?
		// (taking into account we are definitely aware of this guy now)

		// skip LOS check if we had to turn and we're a tank.  sorry Mr Tank, no looking out of the sides for you!
		if ( !( bHadToTurn && TANK( pSoldier ) ) )
		{
			if ( SoldierTo3DLocationLineOfSightTest( pSoldier, sGridNo, bLevel, 0, (UINT8) sDistVisible, TRUE ) )
			{
				// he can actually see the spot where the noise came from!
				bSourceSeen = TRUE;

				// if this sounds like a door opening/closing (could also be a crate)
				if (ubNoiseType == NOISE_CREAKING)
				{
					// then look around and update ALL doors that have secretly changed
					//LookForDoors(pSoldier,AWARE);
				}
			}
		}
	}

	// if noise is made by a person
	if (noise_maker != NULL)
	{
		bOldOpplist = pSoldier->bOppList[noise_maker->ubID];

		// WE ALREADY KNOW THAT HE'S ON ANOTHER TEAM, AND HE'S NOT BEING SEEN
		// ProcessNoise() ALREADY DID THAT WORK FOR US

		if (bSourceSeen)
		{
			ManSeesMan(*pSoldier, *noise_maker, CALLER_UNKNOWN);

			// if it's an AI soldier, he is not allowed to automatically radio any
			// noise heard, but manSeesMan has set his newOppCnt, so clear it here
			if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
			{
				pSoldier->bNewOppCnt = 0;
			}

			*ubSeen = TRUE;
			// RadioSightings() must only be called later on by ProcessNoise() itself
			// because we want the soldier who heard noise the LOUDEST to report it

			if ( pSoldier->bNeutral )
			{
				// could be a civilian watching us shoot at an enemy
				if (((ubNoiseType == NOISE_GUNFIRE) || (ubNoiseType == NOISE_BULLET_IMPACT)) &&
					(ubVolume >= 3))
				{
					// if status is only GREEN or YELLOW
					if (pSoldier->bAlertStatus < STATUS_RED)
					{
						// then this soldier goes to status RED, has proof of enemy presence
						pSoldier->bAlertStatus = STATUS_RED;
						CheckForChangingOrders(pSoldier);
					}
				}
			}

		}
		else // noise maker still can't be seen
		{
			SetNewSituation( pSoldier ); // re-evaluate situation

			// if noise type was unmistakably that of gunfire
			if (((ubNoiseType == NOISE_GUNFIRE) || (ubNoiseType == NOISE_BULLET_IMPACT)) && (ubVolume >= 3))
			{
				// if status is only GREEN or YELLOW
				if (pSoldier->bAlertStatus < STATUS_RED)
				{
					// then this soldier goes to status RED, has proof of enemy presence
					pSoldier->bAlertStatus = STATUS_RED;
					CheckForChangingOrders(pSoldier);
				}
			}

			// remember that the soldier has been heard and his new location
			UpdatePersonal(pSoldier, noise_maker->ubID ,HEARD_THIS_TURN, sGridNo, bLevel);

			// Public info is not set unless EVERYONE on the team fails to see the
			// ubnoisemaker, leaving the 'seen' flag FALSE.  See ProcessNoise().

			// CJC: set the noise gridno for the soldier, if appropriate - this is what is looked at by the AI!
			if (ubVolume >= pSoldier->ubNoiseVolume)
			{
				// yes it is, so remember this noise INSTEAD (old noise is forgotten)
				pSoldier->sNoiseGridno = sGridNo;
				pSoldier->bNoiseLevel = bLevel;

				// no matter how loud noise was, don't remember it for than 12 turns!
				if (ubVolume < MAX_MISC_NOISE_DURATION)
				{
					pSoldier->ubNoiseVolume = ubVolume;
				}
				else
				{
					pSoldier->ubNoiseVolume = MAX_MISC_NOISE_DURATION;
				}

				SetNewSituation( pSoldier );  // force a fresh AI decision to be made
			}

		}

		if ( pSoldier->fAIFlags & AI_ASLEEP )
		{
			switch( ubNoiseType )
			{
				case NOISE_BULLET_IMPACT:
				case NOISE_GUNFIRE:
				case NOISE_EXPLOSION:
				case NOISE_SCREAM:
				case NOISE_WINDOW_SMASHING:
				case NOISE_DOOR_SMASHING:
					// WAKE UP!
					pSoldier->fAIFlags &= (~AI_ASLEEP);
					break;
				default:
					break;
			}
		}

		// FIRST REQUIRE MUTUAL HOSTILES!
		if (!CONSIDERED_NEUTRAL(noise_maker, pSoldier) && !CONSIDERED_NEUTRAL(pSoldier, noise_maker) &&
			pSoldier->bSide != noise_maker->bSide)
		{
			// regardless of whether the noisemaker (who's not NOBODY) was seen or not,
			// as long as listener meets minimum interrupt conditions
			if ( gfDelayResolvingBestSightingDueToDoor)
			{
				if (bSourceSeen && (!(gTacticalStatus.uiFlags & INCOMBAT) ||
					(gubSightFlags & SIGHTINTERRUPT &&
					StandardInterruptConditionsMet(pSoldier, noise_maker, bOldOpplist))))
				{
					// we should be adding this to the array for the AllTeamLookForAll to handle
					// since this is a door opening noise, add a bonus equal to half the door volume
					UINT8 ubPoints;

					ubPoints = CalcInterruptDuelPts(pSoldier, noise_maker, TRUE);
					if ( ubPoints != NO_INTERRUPT )
					{
						// require the enemy not to be dying if we are the sighter; in other words,
						// always add for AI guys, and always add for people with life >= OKLIFE
						if (pSoldier->bTeam != OUR_TEAM || noise_maker->bLife >= OKLIFE)
						{
							ReevaluateBestSightingPosition( pSoldier, (UINT8) (ubPoints + (ubVolume / 2)) );
						}
					}
				}
			}
			else
			{
				if (gTacticalStatus.uiFlags & INCOMBAT)
				{
					if (StandardInterruptConditionsMet(pSoldier, noise_maker, bOldOpplist))
					{
						// he gets a chance to interrupt the noisemaker
						pSoldier->bInterruptDuelPts = CalcInterruptDuelPts(pSoldier, noise_maker, TRUE);
						SLOGD("Calculating int duel pts in noise code, {} has {} points",
							pSoldier->ubID, pSoldier->bInterruptDuelPts);
					}
					else
					{
						pSoldier->bInterruptDuelPts = NO_INTERRUPT;
					}
				}
				else if ( bSourceSeen )
				{
					// seen source, in realtime, so check for sighting stuff
					HandleBestSightingPositionInRealtime();
				}
			}

		}
	}
	else   // noise made by NOBODY
	{
		// if noise type was unmistakably that of an explosion (seen or not) or alarm
		if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
		{
			if ( ( ubNoiseType == NOISE_EXPLOSION || ubNoiseType == NOISE_SILENT_ALARM ) && (ubVolume >= 3) )
			{
				if ( ubNoiseType == NOISE_SILENT_ALARM )
				{
					WearGasMaskIfAvailable( pSoldier );
				}
				// if status is only GREEN or YELLOW
				if (pSoldier->bAlertStatus < STATUS_RED)
				{
					// then this soldier goes to status RED, has proof of enemy presence
					pSoldier->bAlertStatus = STATUS_RED;
					CheckForChangingOrders(pSoldier);
				}
			}
		}
		// if the source of the noise can't be seen,
		// OR if it's a rock and the listener had to turn so that by the time he
		// looked all his saw was a bunch of rocks lying still
		if (!bSourceSeen || ((ubNoiseType == NOISE_ROCK_IMPACT) && (bHadToTurn) ) ||
			ubNoiseType == NOISE_SILENT_ALARM )
		{
			// check if the effective volume of this new noise is greater than or at
			// least equal to the volume of the currently noticed noise stored
			if (ubVolume >= pSoldier->ubNoiseVolume)
			{
				// yes it is, so remember this noise INSTEAD (old noise is forgotten)
				pSoldier->sNoiseGridno = sGridNo;
				pSoldier->bNoiseLevel = bLevel;

				// no matter how loud noise was, don't remember it for than 12 turns!
				if (ubVolume < MAX_MISC_NOISE_DURATION)
				{
					pSoldier->ubNoiseVolume = ubVolume;
				}
				else
				{
					pSoldier->ubNoiseVolume = MAX_MISC_NOISE_DURATION;
				}

				SetNewSituation( pSoldier ); // force a fresh AI decision to be made
			}
		}
		else
		// if listener sees the source of the noise, AND it's either a grenade,
		//  or it's a rock that he watched land (didn't need to turn)
		{
			SetNewSituation( pSoldier );  // re-evaluate situation

			// if status is only GREEN or YELLOW
			if (pSoldier->bAlertStatus < STATUS_RED)
			{
				// then this soldier goes to status RED, has proof of enemy presence
				pSoldier->bAlertStatus = STATUS_RED;
				CheckForChangingOrders(pSoldier);
			}
		}

		if ( gubBestToMakeSightingSize == BEST_SIGHTING_ARRAY_SIZE_INCOMBAT )
		{
			// if the noise heard was the fall of a rock
			if ((gTacticalStatus.uiFlags & INCOMBAT) && ubNoiseType == NOISE_ROCK_IMPACT)
			{
				// give every ELIGIBLE listener an automatic interrupt, since it's
				// reasonable to assume the guy throwing wants to wait for their reaction!
				if (StandardInterruptConditionsMet(pSoldier, NULL, FALSE))
				{
					pSoldier->bInterruptDuelPts = AUTOMATIC_INTERRUPT; // force automatic interrupt
					SLOGD("Calculating int duel pts in noise code, {} has {} points",
						pSoldier->ubID, pSoldier->bInterruptDuelPts);
				}
				else
				{
					pSoldier->bInterruptDuelPts = NO_INTERRUPT;
				}
			}
		}
	}
}


static void TellPlayerAboutNoise(SOLDIERTYPE* const s, SOLDIERTYPE const* const noise_maker, INT16 const sGridNo, INT8 const level, UINT8 const volume, NoiseKind const noise_type, UINT8 const noise_dir)
{
	// CJC: tweaked the noise categories upwards a bit because our movement noises
	// can be louder now.
	UINT8 const volume_idx =
		volume <  4 ? 0 : // 1-3:  faint noise
		volume <  8 ? 1 : // 4-7:  definite noise
		volume < 12 ? 2 : // 8-11: loud noise
		3;                // 12+:  very loud noise

	if (noise_maker && s->bTeam == OUR_TEAM && s->bTeam == noise_maker->bTeam)
	{
		SLOGE("{} ({}) heard noise from {} ({}), noise at {}L{}, type {}",
			s->name, s->ubID, noise_maker->name, noise_maker->ubID, sGridNo, level, noise_type);
	}

	// display a message about a noise, e.g. Sidney hears a loud splash from/to?
	// the north.
	ST::string direction =
		level      == s->bLevel            ||
		noise_type == NOISE_EXPLOSION      ||
		noise_type == NOISE_SCREAM         ||
		noise_type == NOISE_ROCK_IMPACT    ||
		noise_type == NOISE_GRENADE_IMPACT ? pDirectionStr[noise_dir] :
		level      >  s->bLevel            ? gzLateLocalizedString[STR_LATE_06] : // From above
		gzLateLocalizedString[STR_LATE_07];                                       // From below
	ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, st_format_printf(pNewNoiseStr[noise_type], s->name, pNoiseVolStr[volume_idx], direction));

	// If the sound was faint, say something
	if (volume_idx == 0 &&
		!AreInMeanwhile() &&
		!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV) &&
		s->ubTurnsUntilCanSayHeardNoise == 0)
	{
		TacticalCharacterDialogue(s, QUOTE_HEARD_SOMETHING);
		s->ubTurnsUntilCanSayHeardNoise = gTacticalStatus.uiFlags & INCOMBAT ? 2 : 5;
	}
}


void VerifyAndDecayOpplist(SOLDIERTYPE *pSoldier)
{
	INT8 *pPersOL; // pointer into soldier's opponent list

	// reduce all seen/known opponent's turn counters by 1 (towards 0)
	// 1) verify accuracy of the opplist by testing sight vs known opponents
	// 2) increment opplist value if opponent is known but not currenly seen
	// 3) forget about known opponents who haven't been noticed in some time

	// if soldier is unconscious, make sure his opplist is wiped out & bail out
	if (pSoldier->bLife < OKLIFE)
	{
		std::fill(std::begin(pSoldier->bOppList), std::end(pSoldier->bOppList), NOT_HEARD_OR_SEEN);
		pSoldier->bOppCnt = 0;
		return;
	}

	// if any new opponents were seen earlier and not yet radioed
	if (pSoldier->bNewOppCnt)
	{
		if (pSoldier->uiStatusFlags & SOLDIER_PC)
		{
			RadioSightings(pSoldier,EVERYBODY,pSoldier->bTeam);
		}

		pSoldier->bNewOppCnt = 0;
	}

	// man looks for each of his opponents WHO ARE ALREADY KNOWN TO HIM
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;
		// if this merc is active, here, and alive
		if (IsSoldierValidForSightings(*pOpponent))
		{
			// if this merc is on the same team, he's no opponent, so skip him
			if (pSoldier->bTeam == pOpponent->bTeam)
			{
				continue;
			}

			pPersOL = pSoldier->bOppList + pOpponent->ubID;

			// if this opponent is "known" in any way (seen or heard recently)
			if (*pPersOL != NOT_HEARD_OR_SEEN)
			{
				// use both sides actual x,y co-ordinates (neither side's moving)
				ManLooksForMan(pSoldier,pOpponent,VERIFYANDDECAYOPPLIST);

				// decay opplist value if necessary
				DecayOppListValue(*pPersOL);
			}
		}
	}


	// if any new opponents were seen
	if (pSoldier->bNewOppCnt)
	{
		// turns out this is NOT an error!  If this guy was gassed last time he
		// looked, his sight limit was 2 tiles, and now he may no longer be gassed
		// and thus he sees opponents much further away for the first time!
		// - Always happens if you STUNGRENADE an opponent by surprise...
		if (pSoldier->uiStatusFlags & SOLDIER_PC)
			RadioSightings(pSoldier,EVERYBODY,pSoldier->bTeam);

		pSoldier->bNewOppCnt = 0;
	}
}

void DecayIndividualOpplist(SOLDIERTYPE *pSoldier)
{
	INT8 *pPersOL; // pointer into soldier's opponent list

	// reduce all currently seen opponent's turn counters by 1 (towards 0)

	// if soldier is unconscious, make sure his opplist is wiped out & bail out
	if (pSoldier->bLife < OKLIFE)
	{
		// must make sure that public opplist is kept to match...
		FOR_EACH_SOLDIER(tgt)
		{
			if (pSoldier->bOppList[tgt->ubID] == SEEN_CURRENTLY)
			{
				HandleManNoLongerSeen(pSoldier, tgt, &pSoldier->bOppList[tgt->ubID], &gbPublicOpplist[pSoldier->bTeam][tgt->ubID]);
			}
		}

		std::fill(std::begin(pSoldier->bOppList), std::end(pSoldier->bOppList), NOT_HEARD_OR_SEEN);
		pSoldier->bOppCnt = 0;
		return;
	}

	// man looks for each of his opponents WHO IS CURRENTLY SEEN
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const pOpponent = *i;
		// if this merc is active, here, and alive
		if (IsSoldierValidForSightings(*pOpponent))
		{
			// if this merc is on the same team, he's no opponent, so skip him
			if (pSoldier->bTeam == pOpponent->bTeam)
			{
				continue;
			}

			pPersOL = pSoldier->bOppList + pOpponent->ubID;

			// if this opponent is seen currently
			if (*pPersOL == SEEN_CURRENTLY)
			{
				// they are NOT visible now!
				(*pPersOL)++;
				if (!CONSIDERED_NEUTRAL(pOpponent, pSoldier) &&
					!CONSIDERED_NEUTRAL(pSoldier, pOpponent) &&
					(pSoldier->bSide != pOpponent->bSide))
				{
					RemoveOneOpponent(pSoldier);
				}

			}
		}
	}
}



void VerifyPublicOpplistDueToDeath(SOLDIERTYPE *pSoldier)
{
	// OK, someone died. Anyone that the deceased ALONE saw has to decay
	// immediately in the Public Opplist.

	// If deceased didn't see ANYONE, don't bother
	if (pSoldier->bOppCnt == 0)
	{
		return;
	}


	// Deceased looks for each of his opponents who is "seen currently"
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		// first, initialize flag since this will be a "new" opponent
		BOOLEAN bOpponentStillSeen = FALSE;

		// if this opponent is active, here, and alive
		if (IsSoldierValidForSightings(*pOpponent))
		{
			// if this opponent is on the same team, he's no opponent, so skip him
			if (pSoldier->bTeam == pOpponent->bTeam)
			{
				continue;
			}

			// point to what the deceased's personal opplist value is
			const INT8* const pPersOL = pSoldier->bOppList + pOpponent->ubID;

			// if this opponent was CURRENTLY SEEN by the deceased (before his
			// untimely demise)
			if (*pPersOL == SEEN_CURRENTLY)
			{
				// then we need to know if any teammates ALSO see this opponent, so loop through
				// trying to find ONE witness to the death...
				FOR_EACH_MERC(j)
				{
					const SOLDIERTYPE* const pTeamMate = *j;
					// if this teammate is active, here, and alive
					if (IsSoldierValidForSightings(*pTeamMate))
					{
						// if this opponent is NOT on the same team, then skip him
						if (pTeamMate->bTeam != pSoldier->bTeam)
						{
							continue;
						}

						// point to what the teammate's personal opplist value is
						const INT8* const pMatePersOL = pTeamMate->bOppList + pOpponent->ubID;

						// test to see if this value is "seen currently"
						if (*pMatePersOL == SEEN_CURRENTLY)
						{
							// this opponent HAS been verified!
							bOpponentStillSeen = TRUE;

							// we can stop looking for other witnesses now
							break;
						}
					}
				}
			}

			// if no witnesses for this opponent, then decay the Public Opplist
			if ( !bOpponentStillSeen )
			{
				DecayOppListValue(gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID]);
			}
		}
	}
}


static void DecayWatchedLocs(INT8 bTeam);


void DecayPublicOpplist(INT8 bTeam)
{
	// decay the team's public noise volume, forget public noise gridno if <= 0
	// used to be -1 per turn but that's not fast enough!
	if (gubPublicNoiseVolume[bTeam] > 0)
	{
		if ( gTacticalStatus.uiFlags & INCOMBAT )
		{
			gubPublicNoiseVolume[bTeam] = (UINT8) ( (UINT32) (gubPublicNoiseVolume[bTeam] * 7) / 10 );
		}
		else
		{
			gubPublicNoiseVolume[bTeam] = gubPublicNoiseVolume[bTeam] / 2;
		}

		if (gubPublicNoiseVolume[bTeam] <= 0)
		{
			gsPublicNoiseGridno[bTeam] = NOWHERE;
		}
	}

	// decay the team's Public Opplist
	bool bNoPubliclyKnownOpponents = true;
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pSoldier = *i;
		// for every living soldier on ANOTHER team
		if (pSoldier->bLife && pSoldier->bTeam != bTeam)
		{
			// hang a pointer to the byte holding team's public opplist for this merc
			INT8& pbPublOL = gbPublicOpplist[bTeam][pSoldier->ubID];

			if (pbPublOL == NOT_HEARD_OR_SEEN)
			{
				continue;
			}

			// well, that make this a "publicly known opponent", so nuke that flag
			bNoPubliclyKnownOpponents = false;

			// if it's been longer than the maximum we care to remember
			if (DecayOppListValue(pbPublOL))
			{
				// forget about him,
				// and also forget where he was last seen (it's been too long)
				// this is mainly so POINT_PATROL guys don't SEEK_OPPONENTs forever
				UpdatePublic(bTeam, pSoldier, NOT_HEARD_OR_SEEN, NOWHERE, 0);
			}
		}
	}

	// if all opponents are publicly unknown (NOT_HEARD_OR_SEEN)
	if (bNoPubliclyKnownOpponents)
	{
		// forget about the last radio alert (ie. throw away who made the call)
		// this is mainly so POINT_PATROL guys don't SEEK_FRIEND forever after
		gTacticalStatus.Team[bTeam].last_merc_to_radio = NULL;
	}

	// decay watched locs as well
	DecayWatchedLocs( bTeam );
}

// bit of a misnomer; this is now decay all opplists
void NonCombatDecayPublicOpplist( UINT32 uiTime )
{
	if ( uiTime - gTacticalStatus.uiTimeSinceLastOpplistDecay >= TIME_BETWEEN_RT_OPPLIST_DECAYS)
	{
		// decay!
		FOR_EACH_MERC(i) VerifyAndDecayOpplist(*i);

		for (UINT32 cnt = 0; cnt < MAXTEAMS; ++cnt)
		{
			if (IsTeamActive(cnt)) DecayPublicOpplist((INT8)cnt);
		}
		// update time
		gTacticalStatus.uiTimeSinceLastOpplistDecay = uiTime;
	}
}

void RecalculateOppCntsDueToNoLongerNeutral( SOLDIERTYPE * pSoldier )
{
	pSoldier->bOppCnt = 0;

	if (!pSoldier->bNeutral)
	{
		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE* const pOpponent = *i;
			// for every living soldier on ANOTHER team
			if (pOpponent->bLife &&
				!pOpponent->bNeutral &&
				pOpponent->bTeam != pSoldier->bTeam &&
				!CONSIDERED_NEUTRAL(pOpponent, pSoldier) &&
				!CONSIDERED_NEUTRAL(pSoldier, pOpponent) &&
				pSoldier->bSide != pOpponent->bSide)
			{
				if ( pSoldier->bOppList[pOpponent->ubID] == SEEN_CURRENTLY )
				{
					AddOneOpponent( pSoldier );
				}
				if ( pOpponent->bOppList[pSoldier->ubID] == SEEN_CURRENTLY )
				{
					// have to add to opponent's oppcount as well since we just became non-neutral
					AddOneOpponent( pOpponent );
				}
			}
		}
	}
}

void RecalculateOppCntsDueToBecomingNeutral( SOLDIERTYPE * pSoldier )
{
	if (pSoldier->bNeutral)
	{
		pSoldier->bOppCnt = 0;

		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE* const pOpponent = *i;
			// for every living soldier on ANOTHER team
			if (pOpponent->bLife &&
				!pOpponent->bNeutral &&
				pOpponent->bTeam != pSoldier->bTeam &&
				!CONSIDERED_NEUTRAL(pSoldier, pOpponent) &&
				pSoldier->bSide != pOpponent->bSide)
			{
				if ( pOpponent->bOppList[pSoldier->ubID] == SEEN_CURRENTLY )
				{
					// have to rem from opponent's oppcount as well since we just became neutral
					RemoveOneOpponent( pOpponent );
				}
			}
		}
	}
}

void NoticeUnseenAttacker( SOLDIERTYPE * pAttacker, SOLDIERTYPE * pDefender, INT8 bReason )
{
	INT8    bOldOppList;
	UINT8   ubTileSightLimit;
	BOOLEAN fSeesAttacker = FALSE;
	INT8    bDirection;
	BOOLEAN	fMuzzleFlash = FALSE;

	if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
	{
		return;
	}

	if ( pAttacker->usAttackingWeapon == DART_GUN )
	{
		// rarely noticed
		if ( SkillCheck( pDefender, NOTICE_DART_CHECK, 0 ) < 0)
		{
			return;
		}
	}

	// do we need to do checks for life/breath here?

	if ( pDefender->ubBodyType == LARVAE_MONSTER || (pDefender->uiStatusFlags & SOLDIER_VEHICLE &&
		pDefender->bTeam == OUR_TEAM) )
	{
		return;
	}

	bOldOppList = pDefender->bOppList[ pAttacker->ubID ];
	if ( PythSpacesAway( pAttacker->sGridNo, pDefender->sGridNo ) <= MaxDistanceVisible() )
	{
		// check LOS, considering we are now aware of the attacker
		// ignore muzzle flashes when must turning head
		if ( pAttacker->fMuzzleFlash )
		{
			bDirection = atan8( pDefender->sX,pDefender->sY, pAttacker->sX, pAttacker->sY );
			if (pDefender->bDirection != bDirection &&
				pDefender->bDirection != OneCDirection(bDirection) &&
				pDefender->bDirection != OneCCDirection(bDirection))
			{
				// temporarily turn off muzzle flash so DistanceVisible can be calculated without it
				pAttacker->fMuzzleFlash = FALSE;
				fMuzzleFlash = TRUE;
			}
		}

		ubTileSightLimit = (UINT8) DistanceVisible( pDefender, DIRECTION_IRRELEVANT, 0, pAttacker->sGridNo, pAttacker->bLevel );
		if (SoldierToSoldierLineOfSightTest( pDefender, pAttacker, ubTileSightLimit, TRUE ) != 0)
		{
			fSeesAttacker = TRUE;
		}
		if ( fMuzzleFlash )
		{
			pAttacker->fMuzzleFlash = TRUE;
		}
	}

	if (fSeesAttacker)
	{
		ManSeesMan(*pDefender, *pAttacker, CALLER_UNKNOWN);

		// newOppCnt not needed here (no radioing), must get reset right away
		// CJC: Huh? well, leave it in for now
		pDefender->bNewOppCnt = 0;


		if (pDefender->bTeam == OUR_TEAM)
		{
			// EXPERIENCE GAIN (5): Victim notices/sees a previously UNSEEN attacker
			StatChange(*pDefender, EXPERAMT, 5, FROM_SUCCESS);

			// mark attacker as being SEEN right now
			RadioSightings(pDefender, pAttacker, pDefender->bTeam);
		}
		// NOTE: ENEMIES DON'T REPORT A SIGHTING PUBLICLY UNTIL THEY RADIO IT IN!
		else
		{
			// go to threatening stance
			ReevaluateEnemyStance( pDefender, pDefender->usAnimState );
		}
	}
	else // victim NOTICED the attack, but CAN'T SEE the actual attacker
	{
		SetNewSituation( pDefender ); // re-evaluate situation

		// if victim's alert status is only GREEN or YELLOW
		if (pDefender->bAlertStatus < STATUS_RED)
		{
			// then this soldier goes to status RED, has proof of enemy presence
			pDefender->bAlertStatus = STATUS_RED;
			CheckForChangingOrders( pDefender );
		}

		UpdatePersonal( pDefender, pAttacker->ubID, HEARD_THIS_TURN, pAttacker->sGridNo, pAttacker->bLevel );

		// if the victim is a human-controlled soldier, instantly report publicly
		if (pDefender->uiStatusFlags & SOLDIER_PC)
		{
			// mark attacker as having been PUBLICLY heard THIS TURN & remember where
			UpdatePublic(pDefender->bTeam, pAttacker, HEARD_THIS_TURN, pAttacker->sGridNo, pAttacker->bLevel);
		}
	}

	if (StandardInterruptConditionsMet(pDefender, pAttacker, bOldOppList))
	{
		SLOGD("INTERRUPT: NoticeUnseenAttacker, standard\
			conditions are met; defender {}, attacker {}\n\
			Calculating int duel pts for defender in NUA",
			pDefender->ubID, pAttacker->ubID);

		// calculate the interrupt duel points
		pDefender->bInterruptDuelPts = CalcInterruptDuelPts(pDefender, pAttacker, FALSE);
	}
	else
	{
		pDefender->bInterruptDuelPts = NO_INTERRUPT;
	}

	// say quote

	if (pDefender->bInterruptDuelPts != NO_INTERRUPT)
	{
		// check for possible interrupt and handle control change if it happens
		// this code is basically ResolveInterruptsVs for 1 man only...

		// calculate active soldier's dueling pts for the upcoming interrupt duel
		SLOGD("Calculating int duel pts for attacker in NUA");
		pAttacker->bInterruptDuelPts = CalcInterruptDuelPts(pAttacker, pDefender, FALSE);
		if ( InterruptDuel( pDefender, pAttacker ) )
		{
			SLOGD("INTERRUPT: NoticeUnseenAttacker, defender\
				pts {}, attacker pts {}, defender gets interrupt",
				pDefender->bInterruptDuelPts, pAttacker->bInterruptDuelPts);
			AddToIntList(pAttacker, FALSE, TRUE);
			AddToIntList(pDefender, TRUE,  TRUE);
			DoneAddingToIntList();
		}
		// either way, clear out both sides' duelPts fields to prepare next duel
		SLOGD("Resetting int pts for {} and {} in NUA", pDefender->ubID, pAttacker->ubID);
		pDefender->bInterruptDuelPts = NO_INTERRUPT;
		pAttacker->bInterruptDuelPts = NO_INTERRUPT;
	}
}


void CheckForAlertWhenEnemyDies(SOLDIERTYPE* pDyingSoldier)
{
	INT8  bDir;
	INT16 sDistAway, sDistVisible;

	FOR_EACH_IN_TEAM(pSoldier, pDyingSoldier->bTeam)
	{
		if (IsSoldierValidForSightings(*pSoldier) && pSoldier != pDyingSoldier &&
		    pSoldier->bLife >= OKLIFE && pSoldier->bAlertStatus < STATUS_RED)
		{
			// this guy might have seen the man die

			// distance we "see" then depends on the direction he is located from us
			bDir = atan8(pSoldier->sX,pSoldier->sY,pDyingSoldier->sX,pDyingSoldier->sY);
			sDistVisible = DistanceVisible(pSoldier, pSoldier->bDesiredDirection, bDir,
							pDyingSoldier->sGridNo, pDyingSoldier->bLevel);
			sDistAway = PythSpacesAway( pSoldier->sGridNo, pDyingSoldier->sGridNo );

			// if we see close enough to see the soldier
			if (sDistAway <= sDistVisible)
			{
				// and we can trace a line of sight to his x,y coordinates
				// assume enemies are always aware of their buddies...
				if (SoldierTo3DLocationLineOfSightTest(pSoldier, pDyingSoldier->sGridNo,
									pDyingSoldier->bLevel, 0,
									(UINT8) sDistVisible, TRUE))
				{
					pSoldier->bAlertStatus = STATUS_RED;
					CheckForChangingOrders( pSoldier );
				}
			}
		}
	}
}


bool MercSeesCreature(SOLDIERTYPE const& s)
{
	if (s.bOppCnt <= 0) return false;
	CFOR_EACH_IN_TEAM(i, CREATURE_TEAM)
	{
		SOLDIERTYPE const& tgt = *i;
		if (!(tgt.uiStatusFlags & SOLDIER_MONSTER)) continue;
		if (!s.bOppList[tgt.ubID])                  continue;
		return true;
	}
	return false;
}


static INT8 FindUnusedWatchedLoc(UINT8 ubID)
{
	INT8 bLoop;

	for ( bLoop = 0; bLoop < NUM_WATCHED_LOCS; bLoop++ )
	{
		if ( gsWatchedLoc[ ubID ][ bLoop ] == NOWHERE )
		{
			return( bLoop );
		}
	}
	return( -1 );
}


static INT8 FindWatchedLocWithLessThanXPointsLeft(UINT8 ubID, UINT8 ubPointLimit)
{
	INT8 bLoop;

	for ( bLoop = 0; bLoop < NUM_WATCHED_LOCS; bLoop++ )
	{
		if ( gsWatchedLoc[ ubID ][ bLoop ] != NOWHERE && gubWatchedLocPoints[ ubID ][ bLoop ] <= ubPointLimit )
		{
			return( bLoop );
		}
	}
	return( -1 );
}


static INT8 FindWatchedLoc(UINT8 ubID, INT16 sGridNo, INT8 bLevel)
{
	INT8 bLoop;

	for ( bLoop = 0; bLoop < NUM_WATCHED_LOCS; bLoop++ )
	{
		if ( gsWatchedLoc[ ubID ][ bLoop ] != NOWHERE &&  gbWatchedLocLevel[ ubID ][ bLoop ] == bLevel )
		{
			if ( SpacesAway( gsWatchedLoc[ ubID ][ bLoop ], sGridNo ) <= WATCHED_LOC_RADIUS )
			{
				return( bLoop );
			}
		}
	}
	return( -1 );
}

INT8 GetWatchedLocPoints( UINT8 ubID, INT16 sGridNo, INT8 bLevel )
{
	INT8 bLoc;

	bLoc = FindWatchedLoc( ubID, sGridNo, bLevel );
	if (bLoc != -1)
	{
		if (gubWatchedLocPoints[ ubID ][ bLoc ] > 1)
		{
			SLOGD("Soldier {} getting {} points for interrupt in watched location", ubID, gubWatchedLocPoints[ ubID ][ bLoc ] - 1);
		}
		// one loc point is worth nothing, so return number minus 1

		// experiment with 1 loc point being worth 1 point
		return( gubWatchedLocPoints[ ubID ][ bLoc ] );
	}

	return( 0 );
}


INT8 GetHighestVisibleWatchedLoc(const SOLDIERTYPE* const s)
{
	const INT16* const loc_id = gsWatchedLoc[s->ubID];
	const UINT8* const pts_id = gubWatchedLocPoints[s->ubID];
	const INT8*  const lvl_id = gbWatchedLocLevel[s->ubID];
	INT8 bHighestLoc    = -1;
	INT8 bHighestPoints =  0;
	for (INT8 bLoop = 0; bLoop < NUM_WATCHED_LOCS; ++bLoop)
	{
		const INT16 loc = loc_id[bLoop];
		if (loc == NOWHERE) continue;

		const UINT8 pts = pts_id[bLoop];
		if (pts <= bHighestPoints) continue;

		const INT8  lvl          = lvl_id[bLoop];
		const INT16 sDistVisible = DistanceVisible(s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, loc, lvl);
		// look at standing height
		if (SoldierTo3DLocationLineOfSightTest(s, loc, lvl, 3, sDistVisible, TRUE))
		{
			bHighestLoc    = bLoop;
			bHighestPoints = pts;
		}
	}
	return bHighestLoc;
}


INT8 GetHighestWatchedLocPoints(const SOLDIERTYPE* const s)
{
	const INT16* const loc_id = gsWatchedLoc[s->ubID];
	const UINT8* const pts_id = gubWatchedLocPoints[s->ubID];
	INT8 bHighestPoints = 0;
	for (INT8 bLoop = 0; bLoop < NUM_WATCHED_LOCS; ++bLoop)
	{
		if (loc_id[bLoop] == NOWHERE) continue;

		const UINT8 pts = pts_id[bLoop];
		if (pts > bHighestPoints) bHighestPoints = pts;
	}
	return bHighestPoints;
}


static void CommunicateWatchedLoc(const SOLDIERTYPE* const watcher, const INT16 sGridNo, const INT8 bLevel, const UINT8 ubPoints)
{
	INT8 bLoopPoint, bPoint;

	const INT8 bTeam = watcher->bTeam;

	CFOR_EACH_IN_TEAM(s, bTeam)
	{
		if (s == watcher) continue;
		if (!IsSoldierValidForSightings(*s) || s->bLife < OKLIFE)
		{
			continue;
		}
		const SoldierID ubLoop = s->ubID;
		bLoopPoint = FindWatchedLoc( ubLoop, sGridNo, bLevel );
		if ( bLoopPoint == -1 )
		{
			// add this as a watched point
			bPoint = FindUnusedWatchedLoc( ubLoop );
			if (bPoint == -1)
			{
				// if we have a point with only 1 point left, replace it
				bPoint = FindWatchedLocWithLessThanXPointsLeft( ubLoop, ubPoints );
			}
			if (bPoint != -1)
			{
				gsWatchedLoc[ ubLoop ][ bPoint ] = sGridNo;
				gbWatchedLocLevel[ ubLoop ][ bPoint ] = bLevel;
				gubWatchedLocPoints[ ubLoop ][ bPoint ] = ubPoints;
				gfWatchedLocReset[ ubLoop ][ bPoint ] = FALSE;
				gfWatchedLocHasBeenIncremented[ ubLoop ][ bPoint ] = TRUE;
			}
			// else no points available!
		}
		else
		{
			// increment to max
			gubWatchedLocPoints[ ubLoop ][ bLoopPoint ] = std::max(gubWatchedLocPoints[ ubLoop ][ bLoopPoint ], ubPoints);

			gfWatchedLocReset[ ubLoop ][ bLoopPoint ] = FALSE;
			gfWatchedLocHasBeenIncremented[ ubLoop ][ bLoopPoint ] = TRUE;
		}
	}
}


static void IncrementWatchedLoc(const SOLDIERTYPE* const watcher, const INT16 sGridNo, const INT8 bLevel)
{
	const SoldierID ubID = watcher->ubID;
	INT8  bPoint;

	bPoint = FindWatchedLoc( ubID, sGridNo, bLevel );
	if (bPoint == -1)
	{
		// try adding point
		bPoint = FindUnusedWatchedLoc( ubID );
		if (bPoint == -1)
		{
			// if we have a point with only 1 point left, replace it
			bPoint = FindWatchedLocWithLessThanXPointsLeft( ubID, 1 );
		}

		if (bPoint != -1)
		{
			gsWatchedLoc[ ubID ][ bPoint ] = sGridNo;
			gbWatchedLocLevel[ ubID ][ bPoint ] = bLevel;
			gubWatchedLocPoints[ ubID ][ bPoint ] = 1;
			gfWatchedLocReset[ ubID ][ bPoint ] = FALSE;
			gfWatchedLocHasBeenIncremented[ ubID ][ bPoint ] = TRUE;

			CommunicateWatchedLoc(watcher, sGridNo, bLevel, 1);
		}
		// otherwise abort; no points available
	}
	else
	{
		if ( !gfWatchedLocHasBeenIncremented[ ubID ][ bPoint ] && gubWatchedLocPoints[ ubID ][ bPoint ] < MAX_WATCHED_LOC_POINTS )
		{
			gubWatchedLocPoints[ ubID ][ bPoint ]++;
			CommunicateWatchedLoc(watcher, sGridNo, bLevel, gubWatchedLocPoints[ubID][bPoint]);
		}
		gfWatchedLocReset[ ubID ][ bPoint ] = FALSE;
		gfWatchedLocHasBeenIncremented[ ubID ][ bPoint ] = TRUE;
	}
}


static void SetWatchedLocAsUsed(UINT8 ubID, INT16 sGridNo, INT8 bLevel)
{
	INT8 bPoint;

	bPoint = FindWatchedLoc( ubID, sGridNo, bLevel );
	if (bPoint != -1)
	{
		gfWatchedLocReset[ ubID ][ bPoint ] = FALSE;
	}
}


static BOOLEAN WatchedLocLocationIsEmpty(INT16 sGridNo, INT8 bLevel, INT8 bTeam)
{
	// look to see if there is anyone near the watched loc who is not on this team
	INT16 sTempGridNo, sX, sY;

	for ( sY = -WATCHED_LOC_RADIUS; sY <= WATCHED_LOC_RADIUS; sY++ )
	{
		for ( sX = -WATCHED_LOC_RADIUS; sX <= WATCHED_LOC_RADIUS; sX++ )
		{
			sTempGridNo = sGridNo + sX + sY * WORLD_ROWS;
			if ( sTempGridNo < 0 || sTempGridNo >= WORLD_MAX )
			{
				continue;
			}
			const SOLDIERTYPE* const tgt = WhoIsThere2(sTempGridNo, bLevel);
			if (tgt != NULL && tgt->bTeam != bTeam) return FALSE;
		}
	}
	return( TRUE );
}


static void DecayWatchedLocs(INT8 bTeam)
{
	UINT8 cnt, cnt2;

	// loop through all soldiers
	for ( cnt = gTacticalStatus.Team[ bTeam ].bFirstID; cnt <= gTacticalStatus.Team[ bTeam ].bLastID; cnt++ )
	{
		// for each watched location
		for ( cnt2 = 0; cnt2 < NUM_WATCHED_LOCS; cnt2++ )
		{
			if (gsWatchedLoc[cnt][cnt2] != NOWHERE &&
				WatchedLocLocationIsEmpty(gsWatchedLoc[cnt][cnt2], gbWatchedLocLevel[cnt][cnt2], bTeam))
			{
				// if the reset flag is still set, then we should decay this point
				if (gfWatchedLocReset[ cnt ][ cnt2 ])
				{
					// turn flag off again
					gfWatchedLocReset[ cnt ][ cnt2 ] = FALSE;

					// halve points
					gubWatchedLocPoints[ cnt ][ cnt2 ] /= 2;
					// if points have reached 0, then reset the location
					if (gubWatchedLocPoints[ cnt ][ cnt2 ] == 0)
					{
						gsWatchedLoc[ cnt ][ cnt2 ] = NOWHERE;
					}
				}
				else
				{
					// flag was false so set to true (will be reset if new people seen there next turn)
					gfWatchedLocReset[ cnt ][ cnt2 ] = TRUE;
				}
			}
		}
	}
}


static void MakeBloodcatsHostile(void)
{
	FOR_EACH_IN_TEAM(s, CREATURE_TEAM)
	{
		if (s->ubBodyType == BLOODCAT && IsSoldierValidForSightings(*s))
		{
			SetSoldierNonNeutral(s);
			RecalculateOppCntsDueToNoLongerNeutral(s);
			if (gTacticalStatus.uiFlags & INCOMBAT)
			{
				CheckForPotentialAddToBattleIncrement(s);
			}
		}
	}
}
