#include "Font.h"
//#include "AI.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "Event_Pump.h"
#include "Random.h"
#include "Overhead_Types.h"
#include "OppList.h"
#include "AI.h"
#include "Font_Control.h"
#include "Soldier_Find.h"
#include "Sys_Globals.h"
#include "Animation_Control.h"
#include "LOS.h"
#include "FOV.h"
#include "Dialogue_Control.h"
#include "Lighting.h"
#include "Environment.h"
#include "Points.h"
#include "Interface_Dialogue.h"
#include "Message.h"
#include "Soldier_Profile.h"
#include "TeamTurns.h"
#include "Interactive_Tiles.h"
#include "Render_Fun.h"
#include "Text.h"
#include "Timer_Control.h"
#include "Soldier_Macros.h"
#include "Soldier_Functions.h"
#include "Handle_UI.h"
#include "Queen_Command.h"
#include "Keys.h"
#include "Campaign.h"
#include "Soldier_Init_List.h"
#include "Music_Control.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Meanwhile.h"
#include "WorldMan.h"
#include "SkillCheck.h"
#include "Smell.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "Civ_Quotes.h"
#include "Sound_Control.h"
#include "Drugs_And_Alcohol.h"
#include "Debug.h"
#include "Items.h"


#define WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA


static const BOOLEAN gbShowEnemies = FALSE; // XXX seems to be a debug switch


//#define TESTOPPLIST


// for ManLooksForMan()
#define MANLOOKSFOROTHERTEAMS   0
#define OTHERTEAMSLOOKFORMAN    1
#define VERIFYANDDECAYOPPLIST   2
#define HANDLESTEPPEDLOOKAT     3
#define LOOKANDLISTEN           4
#define UPDATEPUBLIC						5
#define CALLER_UNKNOWN					6

// this variable is a flag used in HandleSight to determine whether (while in non-combat RT)
// someone has just been seen, EITHER THE MOVER OR SOMEONE THE MOVER SEES
static BOOLEAN gfPlayerTeamSawCreatures = FALSE;
BOOLEAN   gfPlayerTeamSawJoey			 = FALSE;
BOOLEAN   gfMikeShouldSayHi				 = FALSE;

static SOLDIERTYPE* gBestToMakeSighting[BEST_SIGHTING_ARRAY_SIZE];
UINT8			gubBestToMakeSightingSize = 0;
//BOOLEAN		gfHumanSawSomeoneInRealtime;

static BOOLEAN gfDelayResolvingBestSightingDueToDoor = FALSE;

#define SHOULD_BECOME_HOSTILE_SIZE 32

static UINT8 gubShouldBecomeHostileOrSayQuote[SHOULD_BECOME_HOSTILE_SIZE];
static UINT8 gubNumShouldBecomeHostileOrSayQuote;

// NB this ID is set for someone opening a door
UINT8			gubInterruptProvoker = NOBODY;

INT8 gbPublicOpplist[MAXTEAMS][TOTAL_SOLDIERS];
INT8 gbSeenOpponents[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
INT16 gsLastKnownOppLoc[TOTAL_SOLDIERS][TOTAL_SOLDIERS];		// merc vs. merc
INT8 gbLastKnownOppLevel[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
INT16 gsPublicLastKnownOppLoc[MAXTEAMS][TOTAL_SOLDIERS];	// team vs. merc
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

#define MAX_WATCHED_LOC_POINTS 4
#define WATCHED_LOC_RADIUS 1

INT16			gsWatchedLoc[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
INT8			gbWatchedLocLevel[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
UINT8			gubWatchedLocPoints[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
BOOLEAN		gfWatchedLocReset[ TOTAL_SOLDIERS ][ NUM_WATCHED_LOCS ];
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

static const INT8 gbSmellStrength[3] =
{
	NORMAL_HUMAN_SMELL_STRENGTH, // normal
	NORMAL_HUMAN_SMELL_STRENGTH + 2, // slob
	NORMAL_HUMAN_SMELL_STRENGTH - 1  // snob
};


UINT16 gsWhoThrewRock = NOBODY;

#define NIGHTSIGHTGOGGLES_BONUS 2
#define UVGOGGLES_BONUS 4

// % values of sighting distance at various light levels

INT8 gbLightSighting[1][16] =
{
{ // human
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
}
};
/*
{
{ // human
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
}
};
*/

UINT8			gubSightFlags = 0;

#define DECAY_OPPLIST_VALUE( value )\
{\
	if ( (value) >= SEEN_THIS_TURN)\
	{\
		(value)++;\
		if ( (value) > OLDEST_SEEN_VALUE )\
		{\
			(value) = NOT_HEARD_OR_SEEN;\
		}\
	}\
	else\
	{\
		if ( (value) <= HEARD_THIS_TURN)\
		{\
			(value)--;\
			if ( (value) < OLDEST_HEARD_VALUE)\
			{\
				(value) = NOT_HEARD_OR_SEEN;\
			}\
		}\
	}\
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
	UINT8			ubLoop, ubLoop2;
	BOOLEAN		fFound = FALSE;
	BOOLEAN		fPointsGotLower = FALSE;

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
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "RBSP: reducing points for %d to %d", pSoldier->ubID, bInterruptDuelPts ) );
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
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "RBSP: reduced points for last individual %d to %d", pSoldier->ubID, bInterruptDuelPts ) );
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
						DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "RBSP: resetting points for %d to zilch", pSoldier->ubID ) );
					}

					// set new points
					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "RBSP: setting points for %d to %d", pSoldier->ubID, bInterruptDuelPts ) );
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
		// else points didn't get lower, so do nothing (because we want to leave each merc with as low int points as possible)
	}

	for ( ubLoop = 0; ubLoop < BEST_SIGHTING_ARRAY_SIZE; ubLoop++ )
	{
		if (gBestToMakeSighting[ubLoop] != NULL)
		{
			DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("RBSP entry %d: %d (%d pts)", ubLoop, gBestToMakeSighting[ubLoop]->ubID, gBestToMakeSighting[ubLoop]->bInterruptDuelPts));
		}
	}
}


static void HandleBestSightingPositionInRealtime(void)
{
	// This function is called for handling interrupts when opening a door in non-combat or
	// just sighting in non-combat, deciding who gets the first turn

	UINT8		ubLoop;

	if ( gfDelayResolvingBestSightingDueToDoor )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "HBSPIR: skipping due to door flag" );
		return;
	}

	if (gBestToMakeSighting[0] != NULL)
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "HBSPIR called and there is someone in the list" );

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
					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Entering combat mode: turn for 2nd best, int for best" );

					EnterCombatMode(gBestToMakeSighting[1]->bTeam);
					// 2nd guy loses control
					AddToIntList(gBestToMakeSighting[1]->ubID, FALSE, TRUE);
					// 1st guy gains control
					AddToIntList(gBestToMakeSighting[0]->ubID, TRUE, TRUE);
					DoneAddingToIntList();
				}
			}
		}

		for ( ubLoop = 0; ubLoop < BEST_SIGHTING_ARRAY_SIZE; ubLoop++ )
		{
			if (gBestToMakeSighting[ubLoop] != NULL)
			{
				gBestToMakeSighting[ubLoop]->bInterruptDuelPts = NO_INTERRUPT;
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("RBSP: done, resetting points for %d to zilch", gBestToMakeSighting[ubLoop]->ubID));
			}
		}

		for ( ubLoop = 0; ubLoop < guiNumMercSlots; ubLoop++ )
		{
			if ( MercSlots[ ubLoop ] )
			{
				AssertMsg( MercSlots[ ubLoop ]->bInterruptDuelPts == NO_INTERRUPT, String( "%ls (%d) still has interrupt pts!", MercSlots[ ubLoop ]->name, MercSlots[ ubLoop ]->ubID ) );
			}
		}
	}
}


static void HandleBestSightingPositionInTurnbased(void)
{
	// This function is called for handling interrupts when opening a door in turnbased

	UINT8		ubLoop, ubLoop2;
	BOOLEAN	fOk = FALSE;

	if (gBestToMakeSighting[0] != NULL)
	{
		if (gBestToMakeSighting[0]->bTeam != gTacticalStatus.ubCurrentTeam)
		{
			// interrupt!
			for ( ubLoop = 0; ubLoop < gubBestToMakeSightingSize; ubLoop++ )
			{
				if (gBestToMakeSighting[ubLoop] == NULL)
				{
					if (gubInterruptProvoker == NOBODY)
					{
						// do nothing (for now) - abort!
						return;
					}
					else
					{
						// use this guy as the "interrupted" fellow
						gBestToMakeSighting[ubLoop] = GetMan(gubInterruptProvoker);
						fOk = TRUE;
						break;
					}
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
				AddToIntList(gBestToMakeSighting[ubLoop]->ubID, FALSE, TRUE);
				for ( ubLoop2 = 0; ubLoop2 < ubLoop; ubLoop2++ )
				{
					AddToIntList(gBestToMakeSighting[ubLoop2]->ubID, TRUE, TRUE);
				}
				DoneAddingToIntList();
			}

		}
		for ( ubLoop = 0; ubLoop < BEST_SIGHTING_ARRAY_SIZE; ubLoop++ )
		{
			if (gBestToMakeSighting[ubLoop] != NULL)
			{
				gBestToMakeSighting[ubLoop]->bInterruptDuelPts = NO_INTERRUPT;
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("RBSP (TB): done, resetting points for %d to zilch", gBestToMakeSighting[ubLoop]->ubID));
			}
		}

		for ( ubLoop = 0; ubLoop < guiNumMercSlots; ubLoop++ )
		{
			if ( MercSlots[ ubLoop ] )
			{
				AssertMsg( MercSlots[ ubLoop ]->bInterruptDuelPts == NO_INTERRUPT, String( "%ls (%d) still has interrupt pts!", MercSlots[ ubLoop ]->name, MercSlots[ ubLoop ]->ubID ) );
			}
		}
	}
}


static void InitSightArrays(void)
{
	UINT32		uiLoop;

	for ( uiLoop = 0; uiLoop < BEST_SIGHTING_ARRAY_SIZE; uiLoop++ )
	{
		gBestToMakeSighting[uiLoop] = NULL;
	}
	//gfHumanSawSomeoneInRealtime = FALSE;
}

void AddToShouldBecomeHostileOrSayQuoteList( UINT8 ubID )
{
	UINT8		ubLoop;

	Assert( gubNumShouldBecomeHostileOrSayQuote < SHOULD_BECOME_HOSTILE_SIZE );

	if ( MercPtrs[ ubID ]->bLife < OKLIFE )
	{
		return;
	}

	// make sure not already in list
	for ( ubLoop = 0; ubLoop < gubNumShouldBecomeHostileOrSayQuote; ubLoop++ )
	{
		if ( gubShouldBecomeHostileOrSayQuote[ ubLoop ] == ubID )
		{
			return;
		}
	}

	gubShouldBecomeHostileOrSayQuote[ gubNumShouldBecomeHostileOrSayQuote ] = ubID;
	gubNumShouldBecomeHostileOrSayQuote++;
}


static UINT8 SelectSpeakerFromHostileOrSayQuoteList(void)
{
	UINT8						ubProfileList[ SHOULD_BECOME_HOSTILE_SIZE ]; // NB list of merc IDs, not profiles!
	UINT8						ubLoop, ubNumProfiles = 0;
	SOLDIERTYPE *		pSoldier;

	for ( ubLoop = 0; ubLoop < gubNumShouldBecomeHostileOrSayQuote; ubLoop++ )
	{
		pSoldier = MercPtrs[ gubShouldBecomeHostileOrSayQuote[ ubLoop ] ];
		if ( pSoldier->ubProfile != NO_PROFILE )
		{

			// make sure person can say quote!!!!
			gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_NEEDS_TO_SAY_HOSTILE_QUOTE;

			if ( NPCHasUnusedHostileRecord( pSoldier->ubProfile, APPROACH_DECLARATION_OF_HOSTILITY ) )
			{
				ubProfileList[ ubNumProfiles ] = gubShouldBecomeHostileOrSayQuote[ ubLoop ];
				ubNumProfiles++;
			}
			else
			{
				// turn flag off again
				gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 &= ~PROFILE_MISC_FLAG2_NEEDS_TO_SAY_HOSTILE_QUOTE;
			}

		}
	}

	if ( ubNumProfiles == 0 )
	{
		return( NOBODY );
	}
	else
	{
		return( ubProfileList[ Random( ubNumProfiles ) ] );
	}
}


void CheckHostileOrSayQuoteList( void )
{
	if ( gubNumShouldBecomeHostileOrSayQuote == 0 || !DialogueQueueIsEmpty() || gfInTalkPanel || gfWaitingForTriggerTimer )
	{
		return;
	}
	else
	{
		UINT8	ubSpeaker, ubLoop;
		SOLDIERTYPE * pSoldier;

		ubSpeaker = SelectSpeakerFromHostileOrSayQuoteList();
		if ( ubSpeaker == NOBODY )
		{
			// make sure everyone on this list is hostile
			for ( ubLoop = 0; ubLoop < gubNumShouldBecomeHostileOrSayQuote; ubLoop++ )
			{
				pSoldier = MercPtrs[ gubShouldBecomeHostileOrSayQuote[ ubLoop ] ];
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
			memset( &gubShouldBecomeHostileOrSayQuote, NOBODY, SHOULD_BECOME_HOSTILE_SIZE );
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
			if ( MercPtrs[ ubSpeaker ]->bVisible != TRUE )
			{
				gbPublicOpplist[ gbPlayerNum ][ ubSpeaker ] = HEARD_THIS_TURN;
				HandleSight( MercPtrs[ ubSpeaker ], SIGHT_LOOK | SIGHT_RADIO );
			}
			// trigger hater
			TriggerNPCWithIHateYouQuote( MercPtrs[ ubSpeaker ]->ubProfile );
		}
	}
}


static void ManLooksForOtherTeams(SOLDIERTYPE* pSoldier);
static void OurTeamRadiosRandomlyAbout(UINT8 ubAbout);
static void OtherTeamsLookForMan(SOLDIERTYPE* pOpponent);


void HandleSight(SOLDIERTYPE *pSoldier, UINT8 ubSightFlags)
{
 UINT32 uiLoop;
 SOLDIERTYPE *pThem;
 INT8			bTempNewSituation;

 if (!pSoldier->bActive || !pSoldier->bInSector || pSoldier->uiStatusFlags & SOLDIER_DEAD )
 {
	// I DON'T THINK SO!
	return;
 }

 gubSightFlags = ubSightFlags;

	if ( gubBestToMakeSightingSize != BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL )
	{
		// if this is not being called as a result of all teams look for all, reset array size
		if ( (gTacticalStatus.uiFlags & INCOMBAT) )
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

	for ( uiLoop = 0; uiLoop < NUM_WATCHED_LOCS; uiLoop++ )
	{
		gfWatchedLocHasBeenIncremented[ pSoldier->ubID ][ uiLoop ] = FALSE;
	}

	gfPlayerTeamSawCreatures = FALSE;

	// store new situation value
	bTempNewSituation = pSoldier->bNewSituation;
	pSoldier->bNewSituation = FALSE;

	// if we've been told to make this soldier look (& others look back at him)
	if (ubSightFlags & SIGHT_LOOK)
	{

		// if this soldier's under our control and well enough to look
		if (pSoldier->bLife >= OKLIFE )
		{
		 /*
#ifdef RECORDOPPLIST
     fprintf(OpplistFile,"ManLooksForOtherTeams (HandleSight/Look) for %d\n",pSoldier->guynum);
#endif
		*/
			// he looks for all other soldiers not on his own team
			ManLooksForOtherTeams(pSoldier);
		}


	 /*
#ifdef RECORDOPPLIST
   fprintf(OpplistFile,"OtherTeamsLookForMan (HandleSight/Look) for %d\n",ptr->guynum);
#endif
	 */

		// all soldiers under our control but not on ptr's team look for him
		OtherTeamsLookForMan(pSoldier);
	} // end of SIGHT_LOOK

	// if we've been told that interrupts are possible as a result of sighting
	if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) && (ubSightFlags & SIGHT_INTERRUPT))
	{
		ResolveInterruptsVs( pSoldier, SIGHTINTERRUPT );
  }

	if ( gubBestToMakeSightingSize == BEST_SIGHTING_ARRAY_SIZE_NONCOMBAT )
	{
		HandleBestSightingPositionInRealtime();
	}

	if ( pSoldier->bNewSituation && !(pSoldier->uiStatusFlags & SOLDIER_PC) )
	{
		HaultSoldierFromSighting( pSoldier, TRUE );
	}
	pSoldier->bNewSituation = __max( pSoldier->bNewSituation, bTempNewSituation );

	// if we've been told to radio the results
	if (ubSightFlags & SIGHT_RADIO)
	{
		if (pSoldier->uiStatusFlags & SOLDIER_PC )
		{
			// update our team's public knowledge
			RadioSightings(pSoldier,EVERYBODY, pSoldier->bTeam );

#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
			RadioSightings(pSoldier,EVERYBODY, MILITIA_TEAM);
#endif

			// if it's our local player's merc
			if (PTR_OURTEAM)
				// revealing roofs and looking for items handled here, too
				RevealRoofsAndItems(pSoldier,TRUE, TRUE, pSoldier->bLevel, FALSE );
		}
		// unless in easy mode allow alerted enemies to radio
		else if ( gGameOptions.ubDifficultyLevel >= DIF_LEVEL_MEDIUM )
		{
			// don't allow admins to radio
			if ( pSoldier->bTeam == ENEMY_TEAM && gTacticalStatus.Team[ ENEMY_TEAM ].bAwareOfOpposition && pSoldier->ubSoldierClass != 	SOLDIER_CLASS_ADMINISTRATOR )
			{
				RadioSightings(pSoldier,EVERYBODY, pSoldier->bTeam );
			}
		}

		pSoldier->bNewOppCnt = 0;
		pSoldier->bNeedToLook = FALSE;


// Temporary for opplist synching - disable random order radioing
#ifndef RECORDOPPLIST
		// if this soldier's NOT on our team (MAY be under our control, though!)
		if (!PTR_OURTEAM)
			OurTeamRadiosRandomlyAbout(pSoldier->ubID);	// radio about him only
#endif


		// all non-humans under our control would now radio, if they were allowed
		// to radio automatically (but they're not).  So just nuke new opp cnt
		// NEW: under LOCALOPPLIST, humans on other teams now also radio in here
		for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
		{
			pThem = MercSlots[ uiLoop ];

			if (pThem != NULL && pThem->bLife >= OKLIFE)
			{
				// if this merc is on the same team as the target soldier
				if (pThem->bTeam == pSoldier->bTeam)
					continue;        // he doesn't look (he ALWAYS knows about him)

				// other human team's merc report sightings to their teams now
				if (pThem->uiStatusFlags & SOLDIER_PC)
				{
// Temporary for opplist synching - disable random order radioing
#ifdef RECORDOPPLIST
					// do our own team, too, since we've bypassed random radioing
					if (TRUE)
#else
					// exclude our own team, we've already done them, randomly
					if (pThem->bTeam != gbPlayerNum)
#endif
						RadioSightings(pThem,pSoldier->ubID, pThem->bTeam);
				}
				// unless in easy mode allow alerted enemies to radio
				else if ( gGameOptions.ubDifficultyLevel >= DIF_LEVEL_MEDIUM )
				{
					// don't allow admins to radio
					if ( pThem->bTeam == ENEMY_TEAM && gTacticalStatus.Team[ ENEMY_TEAM ].bAwareOfOpposition && pThem->ubSoldierClass != 	SOLDIER_CLASS_ADMINISTRATOR )
					{
						RadioSightings(pThem,EVERYBODY, pThem->bTeam );
					}
				}


				pThem->bNewOppCnt = 0;
				pThem->bNeedToLook = FALSE;
			}
		}
	}

	// CJC August 13 2002: at the end of handling sight, reset sight flags to allow interrupts in case an audio cue should
	// cause someone to see an enemy
	gubSightFlags |= SIGHT_INTERRUPT;
}


static void OurTeamRadiosRandomlyAbout(UINT8 ubAbout)
{
 INT32				iLoop;
 INT8					radioCnt = 0,radioMan[20];
 SOLDIERTYPE	*pSoldier;

 // All mercs on our local team check if they should radio about him
 iLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;

 // make a list of all of our team's mercs
 for (pSoldier = MercPtrs[iLoop]; iLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; iLoop++,pSoldier++)
  {
   // if this merc is active, in this sector, and well enough to look
   if (pSoldier->bActive && pSoldier->bInSector && (pSoldier->bLife >= OKLIFE))
     // put him on our list, and increment the counter
     radioMan[radioCnt++] = (INT8)iLoop;
  }


 // now RANDOMLY handle each of the mercs on our list, until none remain
 // (this is all being done ONLY so that the mercs in the earliest merc
 //  slots do not arbitrarily get the bulk of the sighting speech quote
 //  action, while the later ones almost never pipe up, and is NOT
 //  strictly necessary, but a nice improvement over original JA)
 while (radioCnt)
  {
   // pick a merc from one of the remaining slots at random
   iLoop = Random(radioCnt);

   // handle radioing for that merc
   RadioSightings(MercPtrs[radioMan[iLoop]],ubAbout,MercPtrs[radioMan[iLoop]]->bTeam);
   Menptr[radioMan[iLoop]].bNewOppCnt = 0;

   // unless it WAS the last used slot that we happened to pick
   if (iLoop != (radioCnt - 1))
     // move the contents of the last slot into the one just handled
     radioMan[iLoop] = radioMan[radioCnt - 1];

   radioCnt--;
  }
}


static INT16 TeamNoLongerSeesMan(UINT8 ubTeam, SOLDIERTYPE* pOpponent, UINT8 ubExcludeID, INT8 bIteration)
{
 UINT16 bLoop;
 SOLDIERTYPE *pMate;


 bLoop = gTacticalStatus.Team[ubTeam].bFirstID;

 // look for all mercs on the same team, check opplists for this soldier
 for (pMate = MercPtrs[bLoop]; bLoop <= gTacticalStatus.Team[ubTeam].bLastID; bLoop++,pMate++)
  {
   // if this "teammate" is me, myself, or I (whom we want to exclude)
   if (bLoop == ubExcludeID)
     continue;          // skip to next teammate, I KNOW I don't see him...

   // if this merc is not on the same team
   if (pMate->bTeam != ubTeam)
     continue;  // skip him, he's no teammate at all!

   // if this merc is not active, at base, on assignment, dead, unconscious
   if (!pMate->bActive || !pMate->bInSector || (pMate->bLife < OKLIFE))
     continue;  // next merc

   // if this teammate currently sees this opponent
   if (pMate->bOppList[pOpponent->ubID] == SEEN_CURRENTLY)
     return(FALSE);     // that's all I need to know, get out of here
  }

#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
	if ( bIteration == 0 )
	{
		if ( ubTeam == gbPlayerNum && gTacticalStatus.Team[ MILITIA_TEAM ].bTeamActive )
		{
			// check militia team as well
			return( TeamNoLongerSeesMan( MILITIA_TEAM, pOpponent, ubExcludeID, 1 ) );
		}
		else if ( ubTeam == MILITIA_TEAM && gTacticalStatus.Team[ gbPlayerNum ].bTeamActive )
		{
			// check player team as well
			return( TeamNoLongerSeesMan( gbPlayerNum, pOpponent, ubExcludeID, 1 ) );
		}
	}
#endif

 // none of my friends is currently seeing the guy, so return success
 return(TRUE);
}


static INT16 DistanceSmellable(SOLDIERTYPE* pSubject)
{
	INT16 sDistVisible = STRAIGHT; // as a base

	//if (gTacticalStatus.uiFlags & TURNBASED)
	//{
		sDistVisible *= 2;
	//}
	//else
	//{

	//	sDistVisible += 3;
	//}

	if (pSubject)
	{
		if (pSubject->uiStatusFlags & SOLDIER_MONSTER)
		{
			// trying to smell a friend; change nothing
		}
		else
		{
			// smelling a human or animal; if they are coated with monster smell, distance shrinks
			sDistVisible = sDistVisible * (pSubject->bNormalSmell - pSubject->bMonsterSmell) / NORMAL_HUMAN_SMELL_STRENGTH;
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
	INT16 sDistVisible;
	INT8	bLightLevel;
	SOLDIERTYPE * pSubject;

	pSubject = SimpleFindSoldier( sSubjectGridNo, bLevel );

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

	if ( !TANK( pSoldier ) && ( bFacingDir == DIRECTION_IRRELEVANT || (pSoldier->uiStatusFlags & SOLDIER_ROBOT) || (pSubject && pSubject->fMuzzleFlash) ) )
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

			if ( sDistVisible == ANGLE && (pSoldier->bTeam == OUR_TEAM || pSoldier->bAlertStatus >= STATUS_RED ) )
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
	bLightLevel = LightTrueLevel(sSubjectGridNo, bLevel);


	if ( pSubject && !( pSubject->fMuzzleFlash && (bLightLevel > NORMAL_LIGHTLEVEL_DAY) ) )
	{
		// ATE: Made function to adjust light distence...
		sDistVisible = AdjustMaxSightRangeForEnvEffects(bLightLevel, sDistVisible);
	}

	// if we wanted to simulate desert-blindness, we'd bump up the light level
	// under certain conditions (daytime in the desert, for instance)
	if (bLightLevel < NORMAL_LIGHTLEVEL_DAY)
	{
		// greater than normal daylight level; check for sun goggles
		if (pSoldier->inv[HEAD1POS].usItem == SUNGOGGLES || pSoldier->inv[HEAD2POS].usItem == SUNGOGGLES)
		{
			// increase sighting distance by up to 2 tiles
			sDistVisible++;
			if (bLightLevel < NORMAL_LIGHTLEVEL_DAY - 1)
			{
				sDistVisible++;;
			}
		}
	}
	else if (bLightLevel > NORMAL_LIGHTLEVEL_DAY + 5)
	{
		if ( (pSoldier->inv[HEAD1POS].usItem == NIGHTGOGGLES || pSoldier->inv[HEAD2POS].usItem == NIGHTGOGGLES || pSoldier->inv[HEAD1POS].usItem == UVGOGGLES || pSoldier->inv[HEAD2POS].usItem == UVGOGGLES) || (pSoldier->ubBodyType == BLOODCAT || AM_A_ROBOT( pSoldier ) ) )
		{
			if ( pSoldier->inv[HEAD1POS].usItem == NIGHTGOGGLES || pSoldier->inv[HEAD2POS].usItem == NIGHTGOGGLES || AM_A_ROBOT( pSoldier ) )
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
			else if ( gbWorldSectorZ == 0 )
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
		sDistVisible = __max( sDistVisible + 5, MaxDistanceVisible() );
	}

	if ( gpWorldLevelData[ pSoldier->sGridNo ].ubExtFlags[ bLevel ] & (MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS) )
	{
		if ( pSoldier->inv[HEAD1POS].usItem != GASMASK && pSoldier->inv[HEAD2POS].usItem != GASMASK )
		{
			// in gas without a gas mask; reduce max distance visible to 2 tiles at most
			sDistVisible = __min( sDistVisible, 2 );
		}
	}

	return(sDistVisible);
}


static void HandleManNoLongerSeen(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, INT8* pPersOL, INT8* pbPublOL);
static void DecideTrueVisibility(SOLDIERTYPE* pSoldier);


void EndMuzzleFlash( SOLDIERTYPE * pSoldier )
{
	UINT32					uiLoop;
	SOLDIERTYPE *		pOtherSoldier;

	pSoldier->fMuzzleFlash = FALSE;

#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
	if ( pSoldier->bTeam != gbPlayerNum && pSoldier->bTeam != MILITIA_TEAM )
#else
	if ( pSoldier->bTeam != gbPlayerNum )
#endif
	{
		pSoldier->bVisible = 0; // indeterminate state
	}

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOtherSoldier = MercSlots[ uiLoop ];

		if ( pOtherSoldier != NULL )
		{
			if ( pOtherSoldier->bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY )
			{
        if ( pOtherSoldier->sGridNo != NOWHERE )
        {
				  if ( PythSpacesAway( pOtherSoldier->sGridNo, pSoldier->sGridNo ) > DistanceVisible( pOtherSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, pSoldier->sGridNo, pSoldier->bLevel ) )
				  {
					  // if this guy can no longer see us, change to seen this turn
					  HandleManNoLongerSeen( pOtherSoldier, pSoldier, &(pOtherSoldier->bOppList[ pSoldier->ubID ]), &(gbPublicOpplist[ pOtherSoldier->bTeam ][ pSoldier->ubID ] ) );
				  }
				  // else this person is still seen, if the looker is on our side or the militia the person should stay visible
			  #ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
				  else if ( pOtherSoldier->bTeam == gbPlayerNum || pOtherSoldier->bTeam == MILITIA_TEAM )
			  #else
				  else if ( pOtherSoldier->bTeam == gbPlayerNum )
			  #endif
				  {
					  pSoldier->bVisible = TRUE; // yes, still seen
				  }
        }
			}
		}
	}
	DecideTrueVisibility(pSoldier);
}

void TurnOffEveryonesMuzzleFlashes( void )
{
	UINT32					uiLoop;
	SOLDIERTYPE *		pSoldier;

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pSoldier = MercSlots[ uiLoop ];

		if ( pSoldier != NULL && pSoldier->fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}
	}
}

void TurnOffTeamsMuzzleFlashes( UINT8 ubTeam )
{
	UINT8						ubLoop;
	SOLDIERTYPE *		pSoldier;

	for (ubLoop = gTacticalStatus.Team[ ubTeam ].bFirstID; ubLoop <= gTacticalStatus.Team[ ubTeam ].bLastID; ubLoop++)
	{
		pSoldier = MercPtrs[ ubLoop ];

		if ( pSoldier->fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}
	}
}


static INT8 DecideHearing(const SOLDIERTYPE* pSoldier)
{
	// calculate the hearing value for the merc...

	INT8		bSlot;
	INT8		bHearing;

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
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "HBSPIR: setting door flag on" );
	// must init sight arrays here
	InitSightArrays();
}


void AllTeamsLookForAll(UINT8 ubAllowInterrupts)
{
 UINT32 uiLoop;
 SOLDIERTYPE *pSoldier;

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
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "HBSPIR: turning door flag off" );
			gfDelayResolvingBestSightingDueToDoor = FALSE;
		}
		else
		{
			InitSightArrays();
		}
	}

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pSoldier = MercSlots[ uiLoop ];

		if ( pSoldier != NULL && pSoldier->bLife >= OKLIFE )
		{
			HandleSight(pSoldier,SIGHT_LOOK);  // no radio or interrupts yet
		}
  }

	// the player team now radios about all sightings
	for ( uiLoop = gTacticalStatus.Team[ gbPlayerNum ].bFirstID; uiLoop <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; uiLoop++ )
	{
		HandleSight( MercPtrs[ uiLoop ], SIGHT_RADIO );      // looking was done above
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
 gubInterruptProvoker = NOBODY;
}


static INT16 ManLooksForMan(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubCaller);


static void ManLooksForOtherTeams(SOLDIERTYPE* pSoldier)
{
 UINT32 uiLoop;
 SOLDIERTYPE *pOpponent;


#ifdef TESTOPPLIST
 DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
	   String("MANLOOKSFOROTHERTEAMS ID %d(%ls) team %d side %d",pSoldier->ubID,pSoldier->name,pSoldier->bTeam,pSoldier->bSide));
#endif


  // one soldier (pSoldier) looks for every soldier on another team (pOpponent)


 for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
 {
   pOpponent = MercSlots[ uiLoop ];

	 // if this soldier is around and alive
	 if (pOpponent && pOpponent->bLife)
	 {
     // and if he's on another team...
     if (pSoldier->bTeam != pOpponent->bTeam)
     {

			// use both sides actual x,y co-ordinates (neither side's moving)
			// if he sees this opponent...
			ManLooksForMan(pSoldier,pOpponent,MANLOOKSFOROTHERTEAMS);

			// OK, We now want to , if in non-combat, set visiblity to 0 if not visible still....
			// This allows us to walk away from buddy and have them disappear instantly
			if ( gTacticalStatus.uiFlags & TURNBASED && !( gTacticalStatus.uiFlags & INCOMBAT ) )
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
	if (!CONSIDERED_NEUTRAL( pOpponent, pSoldier ) && !CONSIDERED_NEUTRAL( pSoldier, pOpponent ) && (pSoldier->bSide != pOpponent->bSide))
	{
		RemoveOneOpponent(pSoldier);
	}

	// change personal opplist to indicate "seen this turn"
	// don't use UpdatePersonal() here, because we're changing to a *lower*
	// opplist value (which UpdatePersonal ignores) and we're not updating
	// the lastKnown gridno at all, we're keeping it at its previous value
	 /*
#ifdef RECORDOPPLIST
   fprintf(OpplistFile,"ManLooksForMan: changing personalOpplist to %d for guynum %d, opp %d\n",SEEN_THIS_TURN,ptr->guynum,oppPtr->guynum);
#endif
	 */

	*pPersOL = SEEN_THIS_TURN;

	if ( (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP) && (pOpponent->bTeam == gbPlayerNum ) )
	{
		UINT8 ubRoom;

		if ( InARoom( pOpponent->sGridNo, &ubRoom ) && IN_BROTHEL( ubRoom ) && ( IN_BROTHEL_GUARD_ROOM( ubRoom ) ) )
		{
			// unauthorized!
			// make guard run to block guard room
			CancelAIAction( pSoldier, TRUE );
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
		if (TeamNoLongerSeesMan(pSoldier->bTeam,pOpponent,pSoldier->ubID, 0 ))
		{
#ifdef TESTOPPLIST
			DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String( "TeamNoLongerSeesMan: ID %d(%ls) to ID %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif


#ifdef RECORDOPPLIST
			fprintf(OpplistFile,"TeamNoLongerSeesMan returns TRUE for team %d, opp %d\n",ptr->team,oppPtr->guynum);
			fprintf(OpplistFile,"ManLooksForMan: changing publicOpplist to %d for team %d, opp %d\n",SEEN_THIS_TURN,ptr->team,oppPtr->guynum);
#endif

			// don't use UpdatePublic() here, because we're changing to a *lower*
			// opplist value (which UpdatePublic ignores) and we're not updating
			// the lastKnown gridno at all, we're keeping it at its previous value
			*pbPublOL = SEEN_THIS_TURN;

			// ATE: Set visiblity to 0
#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
			if ( (pSoldier->bTeam == gbPlayerNum || pSoldier->bTeam == MILITIA_TEAM) && !(pOpponent->bTeam == gbPlayerNum || pOpponent->bTeam == MILITIA_TEAM ) )
#else
			if ( pSoldier->bTeam == gbPlayerNum && pOpponent->bTeam != gbPlayerNum )
#endif
			{
				pOpponent->bVisible = 0;
			}
		}
	}
#ifdef TESTOPPLIST
	else
		DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String("ManLooksForMan: ID %d(%ls) to ID %d Personally seen, public %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID,*pbPublOL) );
#endif

	// if we had only seen the guy for an instant and now lost sight of him
	if (gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] == -1)
		// we can't leave it -1, because InterruptDuel() uses the special -1
		// value to know if we're only JUST seen the guy and screw up otherwise
		// it's enough to know we have seen him before
		gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] = TRUE;

}


static void ManSeesMan(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, INT16 sOppGridno, INT8 bOppLevel, UINT8 ubCaller2);


static INT16 ManLooksForMan(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubCaller)
{
 INT8 bDir,bAware = FALSE,bSuccess = FALSE;
 INT16 sDistVisible,sDistAway;
 INT8  *pPersOL,*pbPublOL;


 /*
 if (ptr->guynum >= NOBODY)
  {
#ifdef BETAVERSION
   NumMessage("ManLooksForMan: ERROR - ptr->guynum = ",ptr->guynum);
#endif
   return(success);
  }

 if (oppPtr->guynum >= NOBODY)
  {
#ifdef BETAVERSION
   NumMessage("ManLooksForMan: ERROR - oppPtr->guynum = ",oppPtr->guynum);
#endif
   return(success);
  }

*/

 // if we're somehow looking while inactive, at base, dead or dying
 if (!pSoldier->bActive || !pSoldier->bInSector || (pSoldier->bLife < OKLIFE))
 {
#ifdef TESTOPPLIST
	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			String("ERROR: ManLooksForMan - WE are inactive/dead etc ID %d(%ls)to ID %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif

   return(FALSE);
  }



 // if we're somehow looking for a guy who is inactive, at base, or already dead
 if (!pOpponent->bActive || !pOpponent->bInSector || pOpponent->bLife <= 0 || pOpponent->sGridNo == NOWHERE )
 {
#ifdef TESTOPPLIST
	 DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			String("ERROR: ManLooksForMan - TARGET is inactive etc ID %d(%ls)to ID %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif

   return(FALSE);
 }


 // if he's looking for a guy who is on the same team
 if (pSoldier->bTeam == pOpponent->bTeam)
  {
#ifdef TESTOPPLIST
	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			String("ERROR: ManLooksForMan - SAME TEAM ID %d(%ls)to ID %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif

   return(FALSE);
  }

	if (pSoldier->bLife < OKLIFE || pSoldier->fMercAsleep == TRUE )
	{
		return( FALSE );
	}

 // NEED TO CHANGE THIS
 /*
 // don't allow unconscious persons to look, but COLLAPSED, etc. is OK
 if (ptr->anitype[ptr->anim] == UNCONSCIOUS)
   return(success);
*/

	if ( pSoldier->ubBodyType == LARVAE_MONSTER || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE && pSoldier->bTeam == OUR_TEAM) )
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
  }
*/

 pPersOL = &(pSoldier->bOppList[pOpponent->ubID]);
 pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID]);


 // if soldier is known about (SEEN or HEARD within last few turns)
 if (*pPersOL || *pbPublOL)
  {
   bAware = TRUE;

   // then we look for him full viewing distance in EVERY direction
 	 sDistVisible = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, 0, pOpponent->sGridNo, pOpponent->bLevel );
  }
 else   // soldier is not currently known about
  {
   // distance we "see" then depends on the direction he is located from us
   bDir = atan8(pSoldier->sX,pSoldier->sY,pOpponent->sX,pOpponent->sY);
   // BIG NOTE: must use desdir instead of direction, since in a projected
   // situation, the direction may still be changing if it's one of the first
   // few animation steps when this guy's turn to do his stepped look comes up
   sDistVisible = DistanceVisible(pSoldier,pSoldier->bDesiredDirection,bDir, pOpponent->sGridNo, pOpponent->bLevel );
  }

 // calculate how many spaces away soldier is (using Pythagoras' theorem)
 sDistAway = PythSpacesAway(pSoldier->sGridNo,pOpponent->sGridNo);

#ifdef TESTOPPLIST
	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String( "MANLOOKSFORMAN: ID %d(%ls) to ID %d: sDistAway %d sDistVisible %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID,sDistAway,sDistVisible) );
#endif


 // if we see close enough to see the soldier
 if (sDistAway <= sDistVisible)
  {
   // and we can trace a line of sight to his x,y coordinates
   // must use the REAL opplist value here since we may or may not know of him
   if (SoldierToSoldierLineOfSightTest(pSoldier,pOpponent,(UINT8)sDistVisible,bAware))
    {
			ManSeesMan(pSoldier, pOpponent, pOpponent->sGridNo, pOpponent->bLevel, ubCaller);
			bSuccess = TRUE;
    }
#ifdef TESTOPPLIST
	 else
			DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String("FAILED LINEOFSIGHT: ID %d (%ls)to ID %d Personally %d, public %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID,*pPersOL,*pbPublOL) );
#endif

/*
   // if we're looking for a local merc, and changed doors were in the way
   if (PTR_OURTEAM && (NextFreeDoorIndex > 0))
     // make or fail, if we passed through any "changed" doors along the way,
     // reveal their true status (change the structure to its real value)
     // (do this even if we don't have LOS, to close doors that *BREAK* LOS)
     RevealDoorsAlongLOS();
*/

 }



/*
#ifdef RECORDOPPLIST
 fprintf(OpplistFile,"MLFM: %s by %2d(g%4d,x%3d,y%3d,%s) at %2d(g%4d,x%3d,y%3d,%s), aware %d, dA=%d,dV=%d, desDir=%d, %s\n",
		(success) ? "SCS" : "FLR",
		ptr->guynum,fromGridno,fromX,fromY,(ptrProjected)?"PROJ":"REG.",
		oppPtr->guynum,toGridno,toX,toY,(oppPtrProjected)?"PROJ":"REG.",
		aware,distAway,distVisible,ptr->desdir,
		LastCaller2Text[caller]);
#endif
*/

 // if soldier seen personally LAST time could not be seen THIS time
 if (!bSuccess && (*pPersOL == SEEN_CURRENTLY))
 {
		HandleManNoLongerSeen( pSoldier, pOpponent, pPersOL, pbPublOL );
 }
 else
 {
	if (!bSuccess)
	{
#ifdef TESTOPPLIST
		DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String("NO LONGER VISIBLE ID %d (%ls)to ID %d Personally %d, public %d success: %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID,*pPersOL,*pbPublOL,bSuccess) );
#endif


		// we didn't see the opponent, but since we didn't last time, we should be
		//if (*pbPublOL)
			//pOpponent->bVisible = TRUE;
	}
#ifdef TESTOPPLIST
	else
		DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String("COOL. STILL VISIBLE ID %d (%ls)to ID %d Personally %d, public %d success: %d",pSoldier->ubID,pSoldier->name,pOpponent->ubID,*pPersOL,*pbPublOL,bSuccess) );
#endif



}



 return(bSuccess);
}


static void IncrementWatchedLoc(UINT8 ubID, INT16 sGridNo, INT8 bLevel);
static void SetWatchedLocAsUsed(UINT8 ubID, INT16 sGridNo, INT8 bLevel);
static void MakeBloodcatsHostile(void);
static void AddOneOpponent(SOLDIERTYPE* pSoldier);
static void UpdatePersonal(SOLDIERTYPE* pSoldier, UINT8 ubID, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel);


static void ManSeesMan(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, INT16 sOppGridno, INT8 bOppLevel, UINT8 ubCaller2)
{
 INT8 bDoLocate = FALSE;
 BOOLEAN fNewOpponent = FALSE;
 BOOLEAN fNotAddedToList = TRUE;
 INT8 bOldOppList = pSoldier->bOppList[pOpponent->ubID];

 if (pSoldier->ubID >= NOBODY)
  {
	 /*
#ifdef BETAVERSION
   NumMessage("ManSeesMan: ERROR - ptr->guynum = ",ptr->guynum);
#endif
	 */
   return;
  }

 if (pOpponent->ubID >= NOBODY)
  {
	 /*
#ifdef BETAVERSION
   NumMessage("ManSeesMan: ERROR - oppPtr->guynum = ",oppPtr->guynum);
#endif
	 */
   return;
  }

 // if we're somehow looking while inactive, at base, dying or already dead
 if (!pSoldier->bActive || !pSoldier->bInSector || (pSoldier->bLife < OKLIFE))
  {
	 /*
#ifdef BETAVERSION
   sprintf(tempstr,"ManSeesMan: ERROR - %s is SEEING ManSeesMan while inactive/at base/dead/dying",ExtMen[ptr->guynum].name);
   PopMessage(tempstr);
#endif
	*/
   return;
  }

 // if we're somehow seeing a guy who is inactive, at base, or already dead
 if (!pOpponent->bActive || !pOpponent->bInSector || pOpponent->bLife <= 0)
  {
	 /*
#ifdef BETAVERSION
   sprintf(tempstr,"ManSeesMan: ERROR - %s sees %s, ManSeesMan, who is inactive/at base/dead",ExtMen[ptr->guynum].name,ExtMen[oppPtr->guynum].name);
   PopMessage(tempstr);
#endif
	 */
   return;
  }


 // if we're somehow seeing a guy who is on the same team
 if (pSoldier->bTeam == pOpponent->bTeam)
  {
	 /*
#ifdef BETAVERSION
   sprintf(tempstr,"ManSeesMan: ERROR - on SAME TEAM.  ptr->guynum = %d, oppPtr->guynum = %d",
					ptr->guynum,oppPtr->guynum);
   PopMessage(tempstr);
#endif
	 */
   return;
  }

 // if we're seeing a guy we didn't see on our last chance to look for him
 if (pSoldier->bOppList[pOpponent->ubID] != SEEN_CURRENTLY)
  {
		if ( pOpponent->bTeam == gbPlayerNum )
		{
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				if ( pSoldier->bTeam == CIV_TEAM )
				{
					// if this person doing the sighting is a member of a civ group that hates us but
					// this fact hasn't been revealed, change the side of these people now. This will
					// make them non-neutral so AddOneOpponent will be called, and the guy will say his
					// "I hate you" quote
					if ( pSoldier->bNeutral )
					{
						if ( pSoldier->ubCivilianGroup != NON_CIV_GROUP && gTacticalStatus.fCivGroupHostile[ pSoldier->ubCivilianGroup ] >= CIV_GROUP_WILL_BECOME_HOSTILE )
						{
							AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
							fNotAddedToList = FALSE;
						}
					}
					else if ( NPCHasUnusedRecordWithGivenApproach( pSoldier->ubProfile, APPROACH_DECLARATION_OF_HOSTILITY ) )
					{
						// only add if have something to say
						AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
						fNotAddedToList = FALSE;
					}

					if ( fNotAddedToList )
					{
						switch( pSoldier->ubProfile )
						{
							case CARMEN:
								if (pOpponent->ubProfile == SLAY ) // 64
								{
									// Carmen goes to war (against Slay)
									if ( pSoldier->bNeutral )
									{
										//SetSoldierNonNeutral( pSoldier );
										pSoldier->bAttitude = ATTACKSLAYONLY;
										TriggerNPCRecord( pSoldier->ubProfile, 28 );
									}
									/*
									if ( ! gTacticalStatus.uiFlags & INCOMBAT )
									{
										EnterCombatMode( pSoldier->bTeam );
									}
									*/
								}
								break;
							case ELDIN:
								if ( pSoldier->bNeutral )
								{
									UINT8 ubRoom = 0;
									// if player is in behind the ropes of the museum display
									// or if alarm has gone off (status red)
									InARoom( pOpponent->sGridNo, &ubRoom );

									if ( ( CheckFact( FACT_MUSEUM_OPEN, 0 ) == FALSE && ubRoom >= 22 && ubRoom <= 41 ) || CheckFact( FACT_MUSEUM_ALARM_WENT_OFF, 0 ) || ( ubRoom == 39 || ubRoom == 40 ) || ( FindObj( pOpponent, CHALICE ) != NO_SLOT ) )
									{
										SetFactTrue( FACT_MUSEUM_ALARM_WENT_OFF );
										AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
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
								if ( pSoldier->bOrders != SEEKENEMY )
								{
									pSoldier->bOrders = SEEKENEMY;
									if ( pSoldier->bOppCnt == 0 )
									{
										// didn't see anyone before!
										CancelAIAction( pSoldier, TRUE );
										SetNewSituation( pSoldier );
									}
								}
								break;
							case ANGEL:
								if ( pOpponent->ubProfile == MARIA )
								{
									if ( CheckFact( FACT_MARIA_ESCORTED_AT_LEATHER_SHOP, MARIA ) == TRUE )
									{
										// she was rescued! yay!
										TriggerNPCRecord( ANGEL, 12 );
									}
								}
								else if ( ( CheckFact( FACT_ANGEL_LEFT_DEED, ANGEL ) == TRUE ) && ( CheckFact( FACT_ANGEL_MENTIONED_DEED, ANGEL ) == FALSE ) )
								{
									CancelAIAction( pSoldier, TRUE );
									pSoldier->sAbsoluteFinalDestination = NOWHERE;
									EVENT_StopMerc( pSoldier, pSoldier->sGridNo, pSoldier->bDirection );
									TriggerNPCRecord( ANGEL, 20 );
									// trigger Angel to walk off afterwards
									//TriggerNPCRecord( ANGEL, 24 );
								}
								break;
							//case QUEEN:
							case JOE:
							case ELLIOT:
								if ( ! ( gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 & PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE ) )
								{
									if ( !AreInMeanwhile() )
									{
										TriggerNPCRecord( pSoldier->ubProfile, 4 );
										gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
									}
								}
								break;
							default:
								break;
						}
					}
				}
				else
				{
					switch( pSoldier->ubProfile )
					{
						/*
						case MIKE:
							if ( gfPlayerTeamSawMike && !( gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 & PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE ) )
							{
								InitiateConversation( pSoldier, pOpponent, NPC_INITIAL_QUOTE, 0 );
								gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
							}
							break;
							*/
						case IGGY:
							if ( ! ( gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 & PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE ) )
							{
								TriggerNPCRecord( pSoldier->ubProfile, 9 );
								gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
								gbPublicOpplist[ gbPlayerNum ][ pSoldier->ubID ] = HEARD_THIS_TURN;
							}
							break;
					}
				}
			}
			else
			{
				if ( pSoldier->bTeam == CIV_TEAM )
				{
					if ( pSoldier->ubCivilianGroup != NON_CIV_GROUP && gTacticalStatus.fCivGroupHostile[ pSoldier->ubCivilianGroup ] >= CIV_GROUP_WILL_BECOME_HOSTILE && pSoldier->bNeutral )
					{
						AddToShouldBecomeHostileOrSayQuoteList( pSoldier->ubID );
					}
					else if ( pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP )
					{
						// generic kingpin goon...

						// check to see if we are looking at Maria or unauthorized personnel in the brothel
						if (pOpponent->ubProfile == MARIA)
						{
							MakeCivHostile( pSoldier, 2 );
							if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
							{
								EnterCombatMode( pSoldier->bTeam );
							}
							SetFactTrue( FACT_MARIA_ESCAPE_NOTICED );
						}
						else
						{
							UINT8 ubRoom;

							// JA2 Gold: only go hostile if see player IN guard room
							//if ( InARoom( pOpponent->sGridNo, &ubRoom ) && IN_BROTHEL( ubRoom ) && ( gMercProfiles[ MADAME ].bNPCData == 0 || IN_BROTHEL_GUARD_ROOM( ubRoom ) ) )
							if ( InARoom( pOpponent->sGridNo, &ubRoom ) && IN_BROTHEL_GUARD_ROOM( ubRoom ) )
							{
								// unauthorized!
								MakeCivHostile( pSoldier, 2 );
								if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
								{
									EnterCombatMode( pSoldier->bTeam );
								}
							}
						}
					}
					else if ( pSoldier->ubCivilianGroup == HICKS_CIV_GROUP && CheckFact( FACT_HICKS_MARRIED_PLAYER_MERC, 0 ) == FALSE )
					{
						UINT32	uiTime;
           	INT16	sX, sY;

						// if before 6:05 or after 22:00, make hostile and enter combat
						uiTime = GetWorldMinutesInDay();
						if ( uiTime < 365 || uiTime > 1320 )
						{
							// get off our farm!
							MakeCivHostile( pSoldier, 2 );
							if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
							{
								EnterCombatMode( pSoldier->bTeam );

								LocateSoldier(pSoldier, TRUE);
	              GetSoldierScreenPos( pSoldier, &sX, &sY );
	              // begin quote
	              BeginCivQuote( pSoldier, CIV_QUOTE_HICKS_SEE_US_AT_NIGHT, 0, sX, sY );
              }
						}
					}
				}
			}
		}
    else if ( pSoldier->bTeam == gbPlayerNum )
    {
		  if ( (pOpponent->ubProfile == MIKE) && ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ) && !(pSoldier->usQuoteSaidExtFlags & SOLDIER_QUOTE_SAID_EXT_MIKE) )
		  {
			  if (gfMikeShouldSayHi == FALSE)
			  {
				gfMikeShouldSayHi = TRUE;
			  }
			  TacticalCharacterDialogue( pSoldier, QUOTE_AIM_SEEN_MIKE );
			  pSoldier->usQuoteSaidExtFlags |= SOLDIER_QUOTE_SAID_EXT_MIKE;
		  }
      else if ( pOpponent->ubProfile == JOEY && gfPlayerTeamSawJoey == FALSE )
      {
        TacticalCharacterDialogue( pSoldier, QUOTE_SPOTTED_JOEY );
        gfPlayerTeamSawJoey = TRUE;
      }
    }

	// as soon as a bloodcat sees someone, it becomes hostile
	// this is safe to do here because we haven't made this new person someone we've seen yet
	// (so we are assured we won't count 'em twice for oppcnt purposes)
	if ( pSoldier->ubBodyType == BLOODCAT )
	{
    if ( pSoldier->bNeutral )
    {
			MakeBloodcatsHostile();
			/*
		  SetSoldierNonNeutral( pSoldier );
		  RecalculateOppCntsDueToNoLongerNeutral( pSoldier );
		  if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
		  {
			  CheckForPotentialAddToBattleIncrement( pSoldier );
		  }
			*/

    	PlayJA2Sample(BLOODCAT_ROAR, HIGHVOLUME, 1, MIDDLEPAN);
    }
    else
    {
      if ( pSoldier->bOppCnt == 0 )
      {
        if ( Random( 2 ) == 0 )
        {
    	    PlayJA2Sample(BLOODCAT_ROAR, HIGHVOLUME, 1, MIDDLEPAN);
        }
      }
    }
	}
	else if ( pOpponent->ubBodyType == BLOODCAT && pOpponent->bNeutral)
	{
		MakeBloodcatsHostile();
		/*
		SetSoldierNonNeutral( pOpponent );
		RecalculateOppCntsDueToNoLongerNeutral( pOpponent );
		if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			CheckForPotentialAddToBattleIncrement( pOpponent );
		}
		*/
	}

   // if both of us are not neutral, AND
   // if this man is actually a true opponent (we're not on the same side)
   if (!CONSIDERED_NEUTRAL( pOpponent, pSoldier ) && !CONSIDERED_NEUTRAL( pSoldier, pOpponent ) && (pSoldier->bSide != pOpponent->bSide))
	 {
     AddOneOpponent(pSoldier);

#ifdef TESTOPPLIST
		 DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String( "ManSeesMan: ID %d(%ls) to ID %d NEW TO ME",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif

     // if we also haven't seen him earlier this turn
     if (pSoldier->bOppList[pOpponent->ubID] != SEEN_THIS_TURN)
      {
			 fNewOpponent = TRUE;
       pSoldier->bNewOppCnt++;        // increment looker's NEW opponent count
			 //ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Soldier %d sees soldier %d!", pSoldier->ubID, pOpponent->ubID );

			 //ExtMen[ptr->guynum].lastCaller = caller;
       //ExtMen[ptr->guynum].lastCaller2 = caller2;

				IncrementWatchedLoc( pSoldier->ubID, pOpponent->sGridNo, pOpponent->bLevel );

				if ( pSoldier->bTeam == OUR_TEAM && pOpponent->bTeam == ENEMY_TEAM )
				{
					if ( CheckFact( FACT_FIRST_BATTLE_FOUGHT, 0 ) == FALSE )
					{
						SetFactTrue( FACT_FIRST_BATTLE_BEING_FOUGHT );
					}
				}


      }
			else
			{
				SetWatchedLocAsUsed( pSoldier->ubID, pOpponent->sGridNo, pOpponent->bLevel );
			}

     // we already know the soldier isn't SEEN_CURRENTLY,
     // now check if he is really "NEW" ie. not expected to be there

     // if the looker hasn't seen this opponent at all earlier this turn, OR
     // if the opponent is not where the looker last thought him to be
     if ((pSoldier->bOppList[pOpponent->ubID] != SEEN_THIS_TURN) ||
         (gsLastKnownOppLoc[pSoldier->ubID][pOpponent->ubID] != sOppGridno))
		  {
				SetNewSituation( pSoldier );  // force the looker to re-evaluate
			}
     else
      {
       // if we in a non-combat movement decision, presumably this is not
       // something we were quite expecting, so make a new decision.  For
       // other (combat) movement decisions, we took his position into account
       // when we made it, so don't make us think again & slow things down.
       switch (pSoldier->bAction)
				{
				 case AI_ACTION_RANDOM_PATROL:
				 case AI_ACTION_SEEK_OPPONENT:
				 case AI_ACTION_SEEK_FRIEND:
				 case AI_ACTION_POINT_PATROL:
				 case AI_ACTION_LEAVE_WATER_GAS:
				 case AI_ACTION_SEEK_NOISE:
					 SetNewSituation( pSoldier );  // force the looker to re-evaluate
					 break;
				}
      }
    }

  }
#ifdef TESTOPPLIST
else
  DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String( "ManSeesMan: ID %d(%ls) to ID %d ALREADYSEENCURRENTLY",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif
	//bOldOppValue = pSoldier->bOppList[ pOpponent->ubID ];
 // remember that the soldier is currently seen and his new location
 UpdatePersonal(pSoldier,pOpponent->ubID,SEEN_CURRENTLY,sOppGridno,bOppLevel);

 if ( ubCaller2 == MANLOOKSFOROTHERTEAMS || ubCaller2 == OTHERTEAMSLOOKFORMAN || ubCaller2 == CALLER_UNKNOWN ) // unknown->hearing
 {

	if ( gubBestToMakeSightingSize != BEST_SIGHTING_ARRAY_SIZE_INCOMBAT && gTacticalStatus.bBoxingState == NOT_BOXING )
	{
		if ( fNewOpponent )
		{
			if ( gTacticalStatus.uiFlags & INCOMBAT )
			{
				// presumably a door opening... we do require standard interrupt conditions
				if (StandardInterruptConditionsMet(pSoldier,pOpponent->ubID,bOldOppList))
				{
					ReevaluateBestSightingPosition( pSoldier, CalcInterruptDuelPts( pSoldier, pOpponent->ubID, TRUE ) );
				}
			}
			// require the enemy not to be dying if we are the sighter; in other words,
			// always add for AI guys, and always add for people with life >= OKLIFE
			else if ( !(pSoldier->bTeam == gbPlayerNum && pOpponent->bLife < OKLIFE ) )
			{
				ReevaluateBestSightingPosition( pSoldier, CalcInterruptDuelPts( pSoldier, pOpponent->ubID, TRUE ) );
			}
		}
	}
 }

 // if this man has never seen this opponent before in this sector
 if (gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] == FALSE)
   // remember that he is just seeing him now for the first time (-1)
   gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] = -1;
 else
   // man is seeing an opponent AGAIN whom he has seen at least once before
   gbSeenOpponents[pSoldier->ubID][pOpponent->ubID] = TRUE;



 // if looker is on local team, and the enemy was invisible or "maybe"
 // visible just prior to this
#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
 if ( ( PTR_OURTEAM || (pSoldier->bTeam == MILITIA_TEAM) ) && (pOpponent->bVisible <= 0))
#else
 if (PTR_OURTEAM && (pOpponent->bVisible <= 0))
#endif
  {
   // if opponent was truly invisible, not just turned off temporarily (FALSE)
   if (pOpponent->bVisible == -1)
	 {
     // then locate to him and set his locator flag
     bDoLocate = TRUE;

	 }

   // make opponent visible (to us)
   // must do this BEFORE the locate since it checks for visibility
   pOpponent->bVisible = TRUE;

	 //ATE: Cancel any fading going on!
	 // ATE: Added for fade in.....
	 if ( pOpponent->fBeginFade == 1 || pOpponent->fBeginFade == 2 )
	 {
		 pOpponent->fBeginFade = FALSE;

		if ( pOpponent->bLevel > 0 && gpWorldLevelData[ pOpponent->sGridNo ].pRoofHead != NULL )
		{
			pOpponent->ubFadeLevel = gpWorldLevelData[ pOpponent->sGridNo ].pRoofHead->ubShadeLevel;
		}
		else
		{
			pOpponent->ubFadeLevel = gpWorldLevelData[ pOpponent->sGridNo ].pLandHead->ubShadeLevel;
		}

		 // Set levelnode shade level....
		 if ( pOpponent->pLevelNode )
		 {
			 pOpponent->pLevelNode->ubShadeLevel = pOpponent->ubFadeLevel;
		 }
	 }


#ifdef TESTOPPLIST
	 DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3, String("!!! ID %d (%ls) MAKING %d VISIBLE",pSoldier->ubID,pSoldier->name,pOpponent->ubID) );
#endif

   // update variable for STATUS screen
   //pOpponent->bLastKnownLife = pOpponent->life;

   if (bDoLocate)
    {

		 // Change his anim speed!
		 SetSoldierAniSpeed( pOpponent );

     // if show enemies is ON, then we must have already revealed these roofs
     // and we're also following his movements, so don't bother sliding
     if (!gbShowEnemies)
      {
       //DoSoldierRoofs(pOpponent);

       // slide to the newly seen opponent, and if appropriate, start his locator
       //SlideToMe = oppPtr->guynum;
      }

     //LastOpponentLocatedTo = oppPtr->guynum;

		 if ( gTacticalStatus.uiFlags & TURNBASED && ( ( gTacticalStatus.uiFlags & INCOMBAT ) | gTacticalStatus.fVirginSector ) )
		 {
			 if (!pOpponent->bNeutral && (pSoldier->bSide != pOpponent->bSide))
			 {
				SlideTo(0, pOpponent, pSoldier->ubID, SETLOCATOR);
			 }
		 }
    }


  }
	else if (!PTR_OURTEAM)
	{
	 // ATE: Check stance, change to threatending
	 ReevaluateEnemyStance( pSoldier, pSoldier->usAnimState );
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
	UINT32 uiLoop;
	INT8 bOldOppList;
	SOLDIERTYPE *pSoldier;


	//NumMessage("OtherTeamsLookForMan, guy#",oppPtr->guynum);

	// if the guy we're looking for is NOT on our team AND is currently visible
#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
	if ((pOpponent->bTeam != gbPlayerNum && pOpponent->bTeam != MILITIA_TEAM) && (pOpponent->bVisible >= 0 && pOpponent->bVisible < 2) && pOpponent->bLife)
#else
	if ((pOpponent->bTeam != gbPlayerNum) && (pOpponent->bVisible >= 0 && pOpponent->bVisible < 2) && pOpponent->bLife)
#endif
	{
		// assume he's no longer visible, until one of our mercs sees him again
		pOpponent->bVisible = 0;
	}

#ifdef TESTOPPLIST
	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			String("OTHERTEAMSLOOKFORMAN ID %d(%ls) team %d side %d",pOpponent->ubID,pOpponent->name,pOpponent->bTeam,pOpponent->bSide ));
#endif


	// all soldiers not on oppPtr's team now look for him
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pSoldier = MercSlots[ uiLoop ];

		// if this merc is active, in this sector, and well enough to look
		if (pSoldier != NULL && pSoldier->bLife >= OKLIFE  && (pSoldier->ubBodyType != LARVAE_MONSTER))
		{
			// if this merc is on the same team as the target soldier
			if (pSoldier->bTeam == pOpponent->bTeam)
			{
				continue;        // he doesn't look (he ALWAYS knows about him)
			}

			bOldOppList = pSoldier->bOppList[pOpponent->ubID];

			// this merc looks for the soldier in question
			// use both sides actual x,y co-ordinates (neither side's moving)
			if (ManLooksForMan(pSoldier,pOpponent,OTHERTEAMSLOOKFORMAN))
			{
				// if a new opponent is seen (which must be oppPtr himself)
				//if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) && pSoldier->bNewOppCnt)
				// Calc interrupt points in non-combat because we might get an interrupt or be interrupted
				// on our first turn

				// if doing regular in-combat sighting (not on opening doors!)
				if ( gubBestToMakeSightingSize == BEST_SIGHTING_ARRAY_SIZE_INCOMBAT )
				{
					if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) && pSoldier->bNewOppCnt )
					{
						// as long as viewer meets minimum interrupt conditions
						if ( gubSightFlags & SIGHT_INTERRUPT && StandardInterruptConditionsMet(pSoldier,pOpponent->ubID,bOldOppList))
						{
							// calculate the interrupt duel points
							pSoldier->bInterruptDuelPts = CalcInterruptDuelPts(pSoldier, pOpponent->ubID, TRUE);
							DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Calculating int duel pts in OtherTeamsLookForMan, %d has %d points", pSoldier->ubID, pSoldier->bInterruptDuelPts ) );
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
	if (pOpponent->bTeam != gbPlayerNum)
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

		pSoldier->bAlertStatus = STATUS_BLACK;   // force black AI status right away

		if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
		{
			pSoldier->ubCaller = NOBODY;
			pSoldier->bCallPriority = 0;
		}
	}

	if (pSoldier->bTeam == gbPlayerNum)
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
	 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Oppcnt for %d (%ls) tried to go below 0", pSoldier->ubID, pSoldier->name ) );
	 #ifdef JA2BETAVERSION
		ScreenMsg( MSG_FONT_YELLOW, MSG_UI_FEEDBACK,  L"Opponent counter dropped below 0 for person %d (%ls).  Please inform Sir-tech of this, and what has just been happening in the game.", pSoldier->ubID, pSoldier->name );
	 #endif
	 pSoldier->bOppCnt = 0;
 }

 // if no opponents remain in sight, drop status to RED (but NOT newSit.!)
 if (!pSoldier->bOppCnt)
   pSoldier->bAlertStatus = STATUS_RED;
}


static void UpdatePublic(UINT8 ubTeam, UINT8 ubID, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel);
static void ResetLastKnownLocs(const SOLDIERTYPE* pSoldier);


void RemoveManAsTarget(SOLDIERTYPE *pSoldier)
{

 SOLDIERTYPE *pOpponent;
 UINT8 ubTarget,ubLoop;


 ubTarget = pSoldier->ubID;

 // clean up the public opponent lists and locations
 for (ubLoop = 0; ubLoop < MAXTEAMS; ubLoop++)
   // never causes any additional looks
   UpdatePublic(ubLoop,ubTarget,NOT_HEARD_OR_SEEN,NOWHERE,0);

/*


IAN COMMENTED THIS OUT MAY 1997 - DO WE NEED THIS?

 // make sure this guy is no longer a possible target for anyone
 for (cnt = 0, pOpponent = Menptr; cnt < MAXMERCS; cnt++,pOpponent++)
  {
   if (pOpponent->bOppNum == ubTarget)
       pOpponent->bOppNum = NOBODY;
  }

	*/


	// clean up all opponent's opplists
	for (ubLoop = 0; ubLoop < guiNumMercSlots; ubLoop++)
	{
		pOpponent = MercSlots[ ubLoop ];

   // if the target is active, a true opponent and currently seen by this merc
		if (pOpponent)
		{
			// check to see if OPPONENT considers US neutral
			if ( (pOpponent->bOppList[ubTarget] == SEEN_CURRENTLY) && !pOpponent->bNeutral && !CONSIDERED_NEUTRAL( pOpponent, pSoldier ) && (pSoldier->bSide != pOpponent->bSide) )
			{
				RemoveOneOpponent(pOpponent);
			}
			UpdatePersonal(pOpponent,ubTarget,NOT_HEARD_OR_SEEN,NOWHERE,0);
			gbSeenOpponents[ubLoop][ubTarget] = FALSE;
		}
	}

/*

 for (ubLoop = 0,pOpponent = Menptr; ubLoop < MAXMERCS; ubLoop++,pOpponent++)
  {
   // if the target is a true opponent and currently seen by this merc
   if (!pSoldier->bNeutral && !pSoldier->bNeutral &&
       (pOpponent->bOppList[ubTarget] == SEEN_CURRENTLY)

			 )
			 ///*** UNTIL ANDREW GETS THE SIDE PARAMETERS WORKING
       // && (pSoldier->side != pOpponent->side))
    {
     RemoveOneOpponent(pOpponent);
    }

   UpdatePersonal(pOpponent,ubTarget,NOT_HEARD_OR_SEEN,NOWHERE,0);

   gbSeenOpponents[ubLoop][ubTarget] = FALSE;
  }
*/

 ResetLastKnownLocs(pSoldier);

 if (gTacticalStatus.Team[pSoldier->bTeam].ubLastMercToRadio == ubTarget)
   gTacticalStatus.Team[pSoldier->bTeam].ubLastMercToRadio = NOBODY;


}


static void UpdatePublic(UINT8 ubTeam, UINT8 ubID, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel)
{
 INT32 cnt;
 INT8 *pbPublOL;
 UINT8 ubTeamMustLookAgain = FALSE;
 SOLDIERTYPE *pSoldier;


 pbPublOL = &(gbPublicOpplist[ubTeam][ubID]);

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
 gsPublicLastKnownOppLoc[ubTeam][ubID] = sGridno;
 gbPublicLastKnownOppLevel[ubTeam][ubID] = bLevel;

 // if team has been told about a guy the team was completely unaware of
 if (ubTeamMustLookAgain)
  {
   // then everyone on team who's not aware of guynum must look for him
   cnt = gTacticalStatus.Team[ubTeam].bFirstID;

   for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[ubTeam].bLastID; cnt++,pSoldier++)
    {
     // if this soldier is active, in this sector, and well enough to look
     if (pSoldier->bActive && pSoldier->bInSector && (pSoldier->bLife >= OKLIFE) && !( pSoldier->uiStatusFlags & SOLDIER_GASSED ) )
     {
       // if soldier isn't aware of guynum, give him another chance to see
       if (pSoldier->bOppList[ubID] == NOT_HEARD_OR_SEEN)
			 {
					if (ManLooksForMan(pSoldier,MercPtrs[ubID],UPDATEPUBLIC))
						// then he actually saw guynum because of our new public knowledge

					// whether successful or not, whack newOppCnt.  Since this is a
					// delayed reaction to a radio call, there's no chance of interrupt!
					pSoldier->bNewOppCnt = 0;
			 }
		 }
	 }
 }
}


static void UpdatePersonal(SOLDIERTYPE* pSoldier, UINT8 ubID, INT8 bNewOpplist, INT16 sGridno, INT8 bLevel)
{
	/*
#ifdef RECORDOPPLIST
 fprintf(OpplistFile,"UpdatePersonal - for %d about %d to %d (was %d) at g%d\n",
		ptr->guynum,guynum,newOpplist,ptr->opplist[guynum],gridno);
#endif

	*/



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


static INT8 OurMaxPublicOpplist(void)
{
 UINT32 uiLoop;
 INT8 bHighestOpplist = 0;
 UINT8 ubOppValue,ubHighestValue = 0;
 SOLDIERTYPE * pSoldier;

 for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
	 pSoldier = MercSlots[ uiLoop ];

	 // if this merc is inactive, at base, on assignment, or dead
	 if (!pSoldier || !pSoldier->bLife)
		 continue;		// next merc

   // if this man is NEUTRAL / on our side, he's not an opponent
	 if (pSoldier->bNeutral || (gTacticalStatus.Team[gbPlayerNum].bSide == Menptr[pSoldier->ubID].bSide))
     continue;		// next merc

   // opponent, check our public opplist value for him
   ubOppValue = gubKnowledgeValue[0 - OLDEST_HEARD_VALUE][gbPublicOpplist[gbPlayerNum][pSoldier->ubID] - OLDEST_HEARD_VALUE];

   if (ubOppValue > ubHighestValue)
    {
     ubHighestValue = ubOppValue;
     bHighestOpplist = gbPublicOpplist[gbPlayerNum][pSoldier->ubID];
    }
  }

 return(bHighestOpplist);
}


static void ResetLastKnownLocs(const SOLDIERTYPE* pSoldier)
{
	UINT32 uiLoop;

	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		if (MercSlots[uiLoop])
		{
			gsLastKnownOppLoc[pSoldier->ubID][MercSlots[uiLoop]->ubID] = NOWHERE;

			// IAN added this June 14/97
			gsPublicLastKnownOppLoc[pSoldier->bTeam][MercSlots[uiLoop]->ubID] = NOWHERE;
		}
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
AllTeamsLookForAll(NO_INTERRUPTS);	// no interrupts permitted this early


// for each additional soldier created, call
InitSoldierOpplist(pSoldier);
HandleSight(pSoldier,SIGHT_LOOK);



MOVEMENT STUFF
-----------------
// whenever new tile is reached, call
HandleSight(pSoldier,SIGHT_LOOK);

*/

void InitOpponentKnowledgeSystem(void)
{
	INT32	iTeam, cnt, cnt2;

	memset(gbSeenOpponents,0,sizeof(gbSeenOpponents));
	memset(gbPublicOpplist,NOT_HEARD_OR_SEEN,sizeof(gbPublicOpplist));

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
		gubShouldBecomeHostileOrSayQuote[ cnt ] = NOBODY;
	}

	gubNumShouldBecomeHostileOrSayQuote = 0;
}



void InitSoldierOppList(SOLDIERTYPE *pSoldier)
{
	memset(pSoldier->bOppList,NOT_HEARD_OR_SEEN,sizeof(pSoldier->bOppList));
	pSoldier->bOppCnt = 0;
	ResetLastKnownLocs(pSoldier);
	memset(gbSeenOpponents[pSoldier->ubID],0,MAXMERCS);
}


void BetweenTurnsVisibilityAdjustments(void)
{
	INT32 cnt;
	SOLDIERTYPE *pSoldier;


  // make all soldiers on other teams that are no longer seen not visible
  for (cnt = 0, pSoldier = Menptr; cnt < MAXMERCS; cnt++,pSoldier++)
	{
		if (pSoldier->bActive && pSoldier->bInSector && pSoldier->bLife)
		{
#ifdef WE_SEE_WHAT_MILITIA_SEES_AND_VICE_VERSA
			if (!PTR_OURTEAM && pSoldier->bTeam != MILITIA_TEAM)
#else
			if (!PTR_OURTEAM)
#endif
			{
				// check if anyone on our team currently sees him (exclude NOBODY)
				if (TeamNoLongerSeesMan(gbPlayerNum,pSoldier,NOBODY,0))
				{
					// then our team has lost sight of him
					pSoldier->bVisible = -1;		// make him fully invisible

					// Allow fade to adjust anim speed
				}
			}
		}
	}
}


static void SaySeenQuote(SOLDIERTYPE* pSoldier, BOOLEAN fSeenCreature, BOOLEAN fVirginSector)
{
	SOLDIERTYPE *pTeamSoldier;
	UINT8				ubNumEnemies = 0;
	UINT8				ubNumAllies = 0;
	UINT32			cnt;

	if ( AreInMeanwhile( ) )
	{
		return;
	}

	// Check out for our under large fire quote
	if ( !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_IN_SHIT ) )
	{
		// Get total enemies.
		// Loop through all mercs in sector and count # of enemies
		for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
		{
			pTeamSoldier = MercSlots[ cnt ];

			if ( pTeamSoldier != NULL )
			{
				if ( OK_ENEMY_MERC( pTeamSoldier ) )
				{
					ubNumEnemies++;
				}
			}
		}

		// OK, after this, check our guys
		for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
		{
			pTeamSoldier = MercSlots[ cnt ];

			if ( pTeamSoldier != NULL )
			{
				if ( !OK_ENEMY_MERC( pTeamSoldier ) )
				{
					 if ( pTeamSoldier->bOppCnt >= ( ubNumEnemies / 2 ) )
					 {
							ubNumAllies++;
					 }
				}
			}
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
				for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
				{
					pTeamSoldier = MercSlots[ cnt ];

					if ( pTeamSoldier != NULL )
					{
						if ( OK_ENEMY_MERC( pTeamSoldier ) )
						{
							if ( pTeamSoldier->uiStatusFlags & SOLDIER_MONSTER && pSoldier->bOppList[ pTeamSoldier->ubID ] == SEEN_CURRENTLY )
							{
								ubNumEnemies++;
							}
						}
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
#ifdef ENGLISH
			if ( Random( 100 ) < 30 )
			{
				DoMercBattleSound( pSoldier, BATTLE_SOUND_ENEMY );
			}
			else
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_SEE_ENEMY );
			}
#else
			TacticalCharacterDialogue( pSoldier, QUOTE_SEE_ENEMY );
#endif
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
		// if this merc is selected and he's actually moving
		//if ((pSoldier->ubID == gusSelectedSoldier) && !pSoldier->bStopped)
		// ATE: Change this to if the guy is ours....
		// How will this feel?
		if ( pSoldier->bTeam == gbPlayerNum )
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
				SlideTo(NOWHERE, GetMan(gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID), NOBODY, SETLOCATOR);

			}

			// Unset User's turn UI
			UnSetUIBusy( pSoldier->ubID );
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

void RadioSightings(SOLDIERTYPE *pSoldier, UINT8 ubAbout, UINT8 ubTeamToRadioTo )
{
 SOLDIERTYPE *pOpponent;
 INT32 	iLoop;
 UINT8 	start,end,revealedEnemies = 0,unknownEnemies = 0;
 //UINT8 	oppIsCivilian;
 INT8 	*pPersOL,*pbPublOL; //,dayQuote;
 BOOLEAN	fContactSeen;
 BOOLEAN fSawCreatureForFirstTime = FALSE;


#ifdef TESTOPPLIST
DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			 String("RADIO SIGHTINGS: for %d about %d",pSoldier->ubID,ubAbout) );
#endif

 gTacticalStatus.Team[pSoldier->bTeam].ubLastMercToRadio = pSoldier->ubID;






 // who are we radioing about?
 if (ubAbout == EVERYBODY)
  {
   start	= 0;
   end		= MAXMERCS;
  }
 else
  {
   start	= ubAbout;
   end 		= ubAbout + 1;
  }


 // hang a pointer to the start of our this guy's personal opplist
 pPersOL = &(pSoldier->bOppList[start]);

 // hang a pointer to the start of this guy's opponents in the public opplist
 pbPublOL = &(gbPublicOpplist[ubTeamToRadioTo][start]);

 pOpponent = MercPtrs[start];

 // loop through every one of this guy's opponents
 for (iLoop = start; iLoop < end; iLoop++,pOpponent++,pPersOL++,pbPublOL++)
  {
	  fContactSeen = FALSE;

#ifdef TESTOPPLIST
	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			 String("RS: checking %d",pOpponent->ubID) );
#endif


   // make sure this merc is active, here & still alive (unconscious OK)
   if (!pOpponent->bActive || !pOpponent->bInSector || !pOpponent->bLife)
    {
#ifdef TESTOPPLIST
		DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			String("RS: inactive/notInSector/life %d",pOpponent->ubID) );
#endif


     continue;                          // skip to the next merc
    }

   // if these two mercs are on the same SIDE, then they're NOT opponents
   // NEW: Apr. 21 '96: must allow ALL non-humans to get radioed about
   if ((pSoldier->bSide == pOpponent->bSide) && (pOpponent->uiStatusFlags & SOLDIER_PC))
    {
#ifdef TESTOPPLIST
		 	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			 String("RS: same side %d",pSoldier->bSide) );
#endif

     continue;                          // skip to the next merc
    }

   // if we personally don't know a thing about this opponent
   if (*pPersOL == NOT_HEARD_OR_SEEN)
    {
#ifdef RECORDOPPLIST
     //fprintf(OpplistFile,"not heard or seen\n");
#endif

#ifdef TESTOPPLIST
			DebugMsg(TOPIC_JA2OPPLIST, DBG_LEVEL_3, "RS: not heard or seen");
#endif

     continue;                          // skip to the next opponent
    }

	 // if personal knowledge is NOT more up to date and NOT the same as public
   if ((!gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pPersOL - OLDEST_HEARD_VALUE]) &&
       (*pbPublOL != *pPersOL))
    {
#ifdef RECORDOPPLIST
     //fprintf(OpplistFile,"no new knowledge (per %d, pub %d)\n",*pPersOL,*pbPublOL);
#endif

#ifdef TESTOPPLIST
		 	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			  String("RS: no new knowledge per %d pub %d",*pPersOL,*pbPublOL) );
#endif



		 continue;                          // skip to the next opponent
    }

#ifdef RECORDOPPLIST
   //fprintf(OpplistFile,"made it!\n");
#endif

#ifdef TESTOPPLIST
		DebugMsg(TOPIC_JA2OPPLIST, DBG_LEVEL_3, "RS: made it!");
#endif



   // if it's our merc, and he currently sees this opponent
   if (PTR_OURTEAM && (*pPersOL == SEEN_CURRENTLY) && !(( pOpponent->bSide == pSoldier->bSide) || pOpponent->bNeutral))
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
	     			StatChange(pSoldier,EXPERAMT,10,FALSE);
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
					if ( pSoldier->bTeam == gbPlayerNum )
					{
						if ( gTacticalStatus.ubCurrentTeam != gbPlayerNum )
						{
							// Save some stuff!
							if (gTacticalStatus.fEnemySightingOnTheirTurn)
							{
								// this has already come up so turn OFF the pause-all-anims flag for the previous
								// person and set it for this next person
								MercPtrs[gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID]->fPauseAllAnimation = FALSE;
							}
							else
							{
								gTacticalStatus.fEnemySightingOnTheirTurn = TRUE;
							}
							gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID = pOpponent->ubID;
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
			if ( gTacticalStatus.Team[ MILITIA_TEAM ].bTeamActive && !gTacticalStatus.Team[ MILITIA_TEAM ].bAwareOfOpposition )
			{
				HandleInitialRedAlert(MILITIA_TEAM);
			}
		}
   } 	// end of our team's merc sees new opponent

   // IF WE'RE HERE, OUR PERSONAL INFORMATION IS AT LEAST AS UP-TO-DATE
   // AS THE PUBLIC KNOWLEDGE, SO WE WILL REPLACE THE PUBLIC KNOWLEDGE
#ifdef RECORDOPPLIST
   fprintf(OpplistFile,"UpdatePublic (RadioSightings) for team %d about %d\n",ptr->team,oppPtr->guynum);
#endif


#ifdef TESTOPPLIST
	DebugMsg( TOPIC_JA2OPPLIST, DBG_LEVEL_3,
			 String("...............UPDATE PUBLIC: soldier %d SEEING soldier %d",pSoldier->ubID,pOpponent->ubID) );
#endif



   UpdatePublic(ubTeamToRadioTo,pOpponent->ubID,*pPersOL,gsLastKnownOppLoc[pSoldier->ubID][pOpponent->ubID],gbLastKnownOppLevel[pSoldier->ubID][pOpponent->ubID]);
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
  if (PTR_OURTEAM)
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



#define COLOR1 FONT_MCOLOR_BLACK<<8 | FONT_MCOLOR_LTGREEN
#define COLOR2 FONT_MCOLOR_BLACK<<8 | FONT_MCOLOR_LTGRAY2

#define LINE_HEIGHT 15


extern UINT32 guiNumBackSaves;

void DebugSoldierPage1( )
{
	UINT16						usMapPos;
	UINT8							ubLine=0;

	const SOLDIERTYPE* const pSoldier = FindSoldierFromMouse();
	if (pSoldier != NULL)
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG SOLDIER PAGE ONE, GRIDNO %d", pSoldier->sGridNo );
		SetFont( LARGEFONT1 );

		ubLine = 2;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"ID:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubID );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"TEAM:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bTeam );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SIDE:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bSide );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"STATUS FLAGS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%x", pSoldier->uiStatusFlags );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"HUMAN:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", gTacticalStatus.Team[pSoldier->bTeam].bHuman);
		ubLine++;
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"APs:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bActionPoints );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Breath:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bBreath );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Life:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bLife );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"LifeMax:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bLifeMax );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Bleeding:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bBleeding );

		ubLine = 2;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Agility:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d ( %d )", pSoldier->bAgility, EffectiveAgility( pSoldier ) );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Dexterity:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d( %d )", pSoldier->bDexterity, EffectiveDexterity( pSoldier ) );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Strength:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d", pSoldier->bStrength );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Wisdom:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d ( %d )", pSoldier->bWisdom, EffectiveWisdom( pSoldier ) );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Exp Lvl:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d ( %d )", pSoldier->bExpLevel, EffectiveExpLevel( pSoldier ) );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Mrksmnship:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d ( %d )", pSoldier->bMarksmanship, EffectiveMarksmanship( pSoldier ) );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Mechanical:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d", pSoldier->bMechanical);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Explosive:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d", pSoldier->bExplosive);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Medical:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 350, LINE_HEIGHT * ubLine, L"%d", pSoldier->bMedical);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Drug Effects:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 400, LINE_HEIGHT * ubLine, L"%d", pSoldier->bDrugEffect[0] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Drug Side Effects:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 400, LINE_HEIGHT * ubLine, L"%d", pSoldier->bDrugSideEffect[0] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Booze Effects:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 400, LINE_HEIGHT * ubLine, L"%d", pSoldier->bDrugEffect[1] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"Hangover Side Effects:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 400, LINE_HEIGHT * ubLine, L"%d", pSoldier->bDrugSideEffect[1] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 200, LINE_HEIGHT * ubLine, L"AI has Keys:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 400, LINE_HEIGHT * ubLine, L"%d", pSoldier->bHasKeys );
		ubLine++;
	}
	else if ( GetMouseMapPos( &usMapPos ) )
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG LAND PAGE ONE" );
		SetFont( LARGEFONT1 );

		ubLine++;
		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Num dirty rects:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 200, LINE_HEIGHT * ubLine, L"%d", guiNumBackSaves );
		ubLine++;


	}

}


void DebugSoldierPage2( )
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

	UINT16						usMapPos;
  TILE_ELEMENT							 TileElem;
	LEVELNODE					*pNode;
	UINT8							ubLine;

	const SOLDIERTYPE* const pSoldier = FindSoldierFromMouse();
	if (pSoldier != NULL)
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG SOLDIER PAGE TWO, GRIDNO %d", pSoldier->sGridNo );
		SetFont( LARGEFONT1 );

		ubLine = 2;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"ID:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubID );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Body Type:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubBodyType );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Opp Cnt:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bOppCnt);
		ubLine++;

		if (pSoldier->bTeam == OUR_TEAM || pSoldier->bTeam == MILITIA_TEAM)	// look at 8 to 15 opplist entries
		{
			SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
			gprintf( 0, LINE_HEIGHT * ubLine, L"Opplist B:");
			SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
			gprintf( 150, LINE_HEIGHT * ubLine, L"%d %d %d %d %d %d %d %d", pSoldier->bOppList[20],pSoldier->bOppList[21],pSoldier->bOppList[22],
							pSoldier->bOppList[23],pSoldier->bOppList[24],pSoldier->bOppList[25],pSoldier->bOppList[26],pSoldier->bOppList[27]);
			ubLine++;
		}
		else	// team 1 - enemies so look at first 8 (0-7) opplist entries
		{
			SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
			gprintf( 0, LINE_HEIGHT * ubLine, L"OppList A:");
			SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
			gprintf( 150, LINE_HEIGHT * ubLine, L"%d %d %d %d %d %d %d %d", pSoldier->bOppList[0],pSoldier->bOppList[1],pSoldier->bOppList[2],
							pSoldier->bOppList[3],pSoldier->bOppList[4],pSoldier->bOppList[5],pSoldier->bOppList[6],
							pSoldier->bOppList[7]);
			ubLine++;
		}

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Visible:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bVisible);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Direction:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf(150, LINE_HEIGHT * ubLine, L"%hs", gzDirectionStr[pSoldier->bDirection]);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"DesDirection:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf(150, LINE_HEIGHT * ubLine, L"%hs", gzDirectionStr[pSoldier->bDesiredDirection]);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"GridNo:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->sGridNo );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Dest:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->sFinalDestination );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Path Size:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->usPathDataSize);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Path Index:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->usPathIndex );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"First 3 Steps:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d %d %d", pSoldier->usPathingData[0],
		pSoldier->usPathingData[1],
		pSoldier->usPathingData[2] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Next 3 Steps:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d %d %d", pSoldier->usPathingData[pSoldier->usPathIndex],
		pSoldier->usPathingData[pSoldier->usPathIndex + 1],
		pSoldier->usPathingData[pSoldier->usPathIndex + 2] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"FlashInd:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fFlashLocator );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"ShowInd:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fShowLocator );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Main hand:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[HANDPOS].usItem] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Second hand:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SECONDHANDPOS].usItem] );
		ubLine++;

		if ( GetMouseMapPos( &usMapPos ) )
		{
			SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
			gprintf( 0, LINE_HEIGHT * ubLine, L"CurrGridNo:");
			SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
			gprintf( 150, LINE_HEIGHT * ubLine, L"%d", usMapPos );
			ubLine++;
		}

	}
	else if ( GetMouseMapPos( &usMapPos ) )
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG LAND PAGE TWO" );
		SetFont( LARGEFONT1 );

		ubLine = 1;

		SetFontColors(COLOR1);
		mprintf( 0, LINE_HEIGHT * ubLine, L"Land Raised:");
		SetFontColors(COLOR2);
		mprintf( 150, LINE_HEIGHT * ubLine, L"%d", gpWorldLevelData[ usMapPos ].sHeight );
		ubLine++;

		SetFontColors(COLOR1);
		mprintf( 0, LINE_HEIGHT * ubLine, L"Land Node:");
		SetFontColors(COLOR2);
		mprintf( 150, LINE_HEIGHT * ubLine, L"%x", gpWorldLevelData[ usMapPos ].pLandHead );
		ubLine++;

		if ( gpWorldLevelData[ usMapPos ].pLandHead != NULL )
		{
			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Land Node:");
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"%d", gpWorldLevelData[ usMapPos ].pLandHead->usIndex );
			ubLine++;

			TileElem = gTileDatabase[ gpWorldLevelData[ usMapPos ].pLandHead->usIndex  ];

			// Check for full tile
			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Full Land:");
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"%d", TileElem.ubFullTile );
			ubLine++;
		}

		SetFontColors(COLOR1);
		mprintf( 0, LINE_HEIGHT * ubLine, L"Land St Node:");
		SetFontColors(COLOR2);
		mprintf( 150, LINE_HEIGHT * ubLine, L"%x", gpWorldLevelData[ usMapPos ].pLandStart );
		ubLine++;

		SetFontColors(COLOR1);
		mprintf( 0, LINE_HEIGHT * ubLine, L"GRIDNO:");
		SetFontColors(COLOR2);
		mprintf( 150, LINE_HEIGHT * ubLine, L"%d", usMapPos );
		ubLine++;

		if ( gpWorldLevelData[ usMapPos ].uiFlags & MAPELEMENT_MOVEMENT_RESERVED )
		{
			SetFontColors(COLOR2);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Merc: %d",  gpWorldLevelData[ usMapPos ].ubReservedSoldierID );
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"RESERVED MOVEMENT FLAG ON:" );
			ubLine++;
		}


		pNode =  GetCurInteractiveTile( );

		if ( pNode != NULL )
		{
			SetFontColors(COLOR2);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Tile: %d",  pNode->usIndex );
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"ON INT TILE" );
			ubLine++;
		}


		if ( gpWorldLevelData[ usMapPos ].uiFlags & MAPELEMENT_REVEALED )
		{
			SetFontColors(COLOR2);
			//mprintf( 0, LINE_HEIGHT * 9, L"Merc: %d",  gpWorldLevelData[ usMapPos ].ubReservedSoldierID );
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"REVEALED" );
			ubLine++;
		}

		if ( gpWorldLevelData[ usMapPos ].uiFlags & MAPELEMENT_RAISE_LAND_START )
		{
			SetFontColors(COLOR2);
			//mprintf( 0, LINE_HEIGHT * 9, L"Merc: %d",  gpWorldLevelData[ usMapPos ].ubReservedSoldierID );
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"Land Raise Start" );
			ubLine++;
		}

		if ( gpWorldLevelData[ usMapPos ].uiFlags & MAPELEMENT_RAISE_LAND_END )
		{
			SetFontColors(COLOR2);
			//mprintf( 0, LINE_HEIGHT * 9, L"Merc: %d",  gpWorldLevelData[ usMapPos ].ubReservedSoldierID );
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"Raise Land End" );
			ubLine++;
		}

		if (gubWorldRoomInfo[ usMapPos ] != NO_ROOM )
		{
			SetFontColors(COLOR2);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Room Number" );
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L"%d", gubWorldRoomInfo[ usMapPos ] );
			ubLine++;
		}

		if ( gpWorldLevelData[ usMapPos ].ubExtFlags[0] & MAPELEMENT_EXT_NOBURN_STRUCT )
		{
			SetFontColors(COLOR2);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Don't Use Burn Through For Soldier" );
			ubLine++;
		}

	}

}


void DebugSoldierPage3( )
{
	static const char* const gzAlertStr[] =
	{
		"GREEN",
		"YELLOW",
		"RED",
		"BLACK"
	};

	UINT16						usMapPos;
	UINT8							ubLine;

	const SOLDIERTYPE* const pSoldier = FindSoldierFromMouse();
	if (pSoldier != NULL)
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG SOLDIER PAGE THREE, GRIDNO %d", pSoldier->sGridNo );
		SetFont( LARGEFONT1 );

		ubLine = 2;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"ID:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubID );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Action:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf(150, LINE_HEIGHT * ubLine, L"%hs", gzActionStr[pSoldier->bAction]);
		if (pSoldier->uiStatusFlags & SOLDIER_ENEMY )
		{
			gprintf(350, LINE_HEIGHT * ubLine, L"Alert %hs", gzAlertStr[pSoldier->bAlertStatus]);
		}
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Action Data:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->usActionData );

		if (pSoldier->uiStatusFlags & SOLDIER_ENEMY )
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"AIMorale %d", pSoldier->bAIMorale );
		}
		else
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"Morale %d", pSoldier->bMorale );
		}
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Delayed Movement:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fDelayedMovement );
		if ( gubWatchedLocPoints[ pSoldier->ubID ][ 0 ] > 0 )
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"Watch %d/%d for %d pts",
				gsWatchedLoc[ pSoldier->ubID ][ 0 ],
				gbWatchedLocLevel[ pSoldier->ubID ][ 0 ],
				gubWatchedLocPoints[ pSoldier->ubID ][ 0 ]
				);
		}

		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"ActionInProg:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bActionInProgress);
		ubLine++;
		if ( gubWatchedLocPoints[ pSoldier->ubID ][ 1 ] > 0 )
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"Watch %d/%d for %d pts",
				gsWatchedLoc[ pSoldier->ubID ][ 1 ],
				gbWatchedLocLevel[ pSoldier->ubID ][ 1 ],
				gubWatchedLocPoints[ pSoldier->ubID ][ 1 ]
				);
		}

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Last Action:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf(150, LINE_HEIGHT * ubLine, L"%hs", gzActionStr[pSoldier->bLastAction]);
		ubLine++;

		if ( gubWatchedLocPoints[ pSoldier->ubID ][ 2 ] > 0 )
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"Watch %d/%d for %d pts",
				gsWatchedLoc[ pSoldier->ubID ][ 2 ],
				gbWatchedLocLevel[ pSoldier->ubID ][ 2 ],
				gubWatchedLocPoints[ pSoldier->ubID ][ 2 ]
				);
		}

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Animation:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf(150, LINE_HEIGHT * ubLine, L"%hs", gAnimControl[pSoldier->usAnimState].zAnimStr);
		ubLine++;

/*
		if ( gubWatchedLocPoints[ pSoldier->ubID ][ 3 ] > 0 )
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"Watch %d/%d for %d pts",
				gsWatchedLoc[ pSoldier->ubID ][ 3 ],
				gbWatchedLocLevel[ pSoldier->ubID ][ 3 ],
				gubWatchedLocPoints[ pSoldier->ubID ][ 3 ]
				);
		}
*/

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Getting Hit:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fGettingHit );

		if (pSoldier->ubCivilianGroup != 0)
		{
			gprintf( 350, LINE_HEIGHT * ubLine, L"Civ group %d", pSoldier->ubCivilianGroup );
		}
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Suppress pts:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubSuppressionPoints );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Attacker ID:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubAttackerID );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"EndAINotCalled:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fTurnInProgress );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"PrevAnimation:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf(150, LINE_HEIGHT * ubLine, L"%hs", gAnimControl[pSoldier->usOldAniState].zAnimStr);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"PrevAniCode:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", gusAnimInst[ pSoldier->usOldAniState ][ pSoldier->sOldAniCode ] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"GridNo:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->sGridNo);
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"AniCode:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", gusAnimInst[ pSoldier->usAnimState ][ pSoldier->usAniCode ] );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"No APS To fin Move:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fNoAPToFinishMove );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Reload Delay:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->sReloadDelay );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Reloading:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fReloading );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Bullets out:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bBulletsLeft );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Anim non-int:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fInNonintAnim );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"RT Anim non-int:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->fRTInNonintAnim );
		ubLine++;

		// OPIONION OF SELECTED MERC
		if (gusSelectedSoldier != NOBODY)
		{
			const SOLDIERTYPE* const s = GetSelectedMan();
			if (s->ubProfile < FIRST_NPC && pSoldier->ubProfile != NO_PROFILE)
			{
				SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
				gprintf( 0, LINE_HEIGHT * ubLine, L"NPC Opinion:");
				SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
				gprintf(150, LINE_HEIGHT * ubLine, L"%d", gMercProfiles[pSoldier->ubProfile].bMercOpinion[s->ubProfile]);
				ubLine++;
			}
		}
	}
	else if ( GetMouseMapPos( &usMapPos ) )
	{
		DOOR_STATUS	*pDoorStatus;
		STRUCTURE *pStructure;

		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG LAND PAGE THREE" );
		SetFont( LARGEFONT1 );

		// OK, display door information here.....
		pDoorStatus = GetDoorStatus( usMapPos );

		ubLine = 1;

		if ( pDoorStatus == NULL )
		{
			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"No Door Status");
			ubLine++;
			ubLine++;
			ubLine++;
		}
		else
		{
			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Door Status Found:");
			SetFontColors(COLOR2);
			mprintf( 150, LINE_HEIGHT * ubLine, L" %d", usMapPos );
			ubLine++;

			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Actual Status:");
			SetFontColors(COLOR2);

			if ( pDoorStatus->ubFlags & DOOR_OPEN )
			{
				mprintf( 200, LINE_HEIGHT * ubLine, L"OPEN" );
			}
			else
			{
				mprintf( 200, LINE_HEIGHT * ubLine, L"CLOSED" );
			}
			ubLine++;


			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"Perceived Status:");
			SetFontColors(COLOR2);

			if ( pDoorStatus->ubFlags & DOOR_PERCEIVED_NOTSET )
			{
				mprintf( 200, LINE_HEIGHT * ubLine, L"NOT SET" );
			}
			else
			{
				if ( pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN )
				{
					mprintf( 200, LINE_HEIGHT * ubLine, L"OPEN" );
				}
				else
				{
					mprintf( 200, LINE_HEIGHT * ubLine, L"CLOSED" );
				}
			}
			ubLine++;
		}

		//Find struct data and se what it says......
		pStructure = FindStructure( usMapPos, STRUCTURE_ANYDOOR );

		if ( pStructure == NULL )
		{
			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"No Door Struct Data");
			ubLine++;
		}
		else
		{

			SetFontColors(COLOR1);
			mprintf( 0, LINE_HEIGHT * ubLine, L"State:");
			SetFontColors(COLOR2);
			if ( !(pStructure->fFlags & STRUCTURE_OPEN) )
			{
				mprintf( 200, LINE_HEIGHT * ubLine, L"CLOSED" );
			}
			else
			{
				mprintf( 200, LINE_HEIGHT * ubLine, L"OPEN" );
			}
			ubLine++;
		}
	}

}


static void AppendAttachmentCode(UINT16 usItem, wchar_t *str) /* XXX Needs length */
{
	switch( usItem )
	{
		case SILENCER:
			wcscat( str, L" Sil" );
			break;
		case SNIPERSCOPE:
			wcscat( str, L" Scp" );
			break;
		case BIPOD:
			wcscat( str, L" Bip" );
			break;
		case LASERSCOPE:
			wcscat( str, L" Las" );
			break;
	}
}


static void WriteQuantityAndAttachments(const OBJECTTYPE* pObject, INT32 yp)
{
	wchar_t szAttach[30];
	BOOLEAN fAttachments;
	//100%  Qty: 2  Attach:
	//100%  Qty: 2
	//100%  Attach:
	//100%
	if( !pObject->usItem )
		return;
	//Build attachment string
	fAttachments = FALSE;
	if( pObject->usAttachItem[0] || pObject->usAttachItem[1] ||
		  pObject->usAttachItem[2] || pObject->usAttachItem[3] )
	{
		fAttachments = TRUE;
		swprintf( szAttach, lengthof(szAttach), L"(" );
		AppendAttachmentCode( pObject->usAttachItem[0], szAttach );
		AppendAttachmentCode( pObject->usAttachItem[1], szAttach );
		AppendAttachmentCode( pObject->usAttachItem[2], szAttach );
		AppendAttachmentCode( pObject->usAttachItem[3], szAttach );
		wcscat( szAttach, L" )" );
	}

	if( Item[pObject->usItem].usItemClass == IC_AMMO )
	{ //ammo
		if( pObject->ubNumberOfObjects > 1 )
		{
			wchar_t str[50];
			wchar_t temp[5];
			UINT8 i;
			swprintf( str, lengthof(str), L"Clips:  %d  (%d", pObject->ubNumberOfObjects, pObject->bStatus[0] );
			for( i = 1; i < pObject->ubNumberOfObjects; i++ )
			{
				swprintf( temp, lengthof(temp), L", %d", pObject->bStatus[0] );
				wcscat( str, temp );
			}
			wcscat( str, L")" );
			gprintf( 320, yp, str );
		}
		else
			gprintf( 320, yp, L"%d rounds", pObject->bStatus[0] );
		return;
	}
	if( pObject->ubNumberOfObjects > 1 && fAttachments )
	{ //everything
		gprintf( 320, yp, L"%d%%  Qty:  %d  %ls",
			pObject->bStatus[0], pObject->ubNumberOfObjects, szAttach );
	}
	else if( pObject->ubNumberOfObjects > 1 )
	{ //condition and quantity
		gprintf( 320, yp, L"%d%%  Qty:  %d  ",
			pObject->bStatus[0], pObject->ubNumberOfObjects );
	}
	else if( fAttachments )
	{ //condition and attachments
		gprintf( 320, yp, L"%d%%  %ls", pObject->bStatus[0], szAttach );
	}
	else
	{ //condition
		gprintf( 320, yp, L"%d%%", pObject->bStatus[0] );
	}
}

void DebugSoldierPage4( )
{
	UINT8							ubLine;

	const SOLDIERTYPE* const pSoldier = FindSoldierFromMouse();
	if (pSoldier != NULL)
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG SOLDIER PAGE FOUR, GRIDNO %d", pSoldier->sGridNo );
		SetFont( LARGEFONT1 );
		ubLine = 2;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"Exp. Level:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->bExpLevel );
		switch( pSoldier->ubSoldierClass )
		{
			case SOLDIER_CLASS_ADMINISTRATOR:		gprintf( 320, LINE_HEIGHT * ubLine, L"(Administrator)" );	break;
			case SOLDIER_CLASS_ELITE:						gprintf( 320, LINE_HEIGHT * ubLine, L"(Army Elite)" );		break;
			case SOLDIER_CLASS_ARMY:						gprintf( 320, LINE_HEIGHT * ubLine, L"(Army Troop)" );		break;
			case SOLDIER_CLASS_CREATURE:				gprintf( 320, LINE_HEIGHT * ubLine, L"(Creature)" );			break;
			case SOLDIER_CLASS_GREEN_MILITIA:		gprintf( 320, LINE_HEIGHT * ubLine, L"(Green Militia)" );	break;
			case SOLDIER_CLASS_REG_MILITIA:			gprintf( 320, LINE_HEIGHT * ubLine, L"(Reg Militia)" );		break;
			case SOLDIER_CLASS_ELITE_MILITIA:		gprintf( 320, LINE_HEIGHT * ubLine, L"(Elite Militia)" );	break;
			case SOLDIER_CLASS_MINER:						gprintf( 320, LINE_HEIGHT * ubLine, L"(Miner)" );					break;
			default:  break; //don't care (don't write anything)
		}
		ubLine++;

		if( pSoldier->bTeam != OUR_TEAM )
		{
			SOLDIERINITNODE		*pNode;
			const wchar_t* Orders;
			switch( pSoldier->bOrders )
			{
				case STATIONARY:  Orders = L"STATIONARY";    break;
				case ONGUARD:     Orders = L"ON GUARD";      break;
				case ONCALL:      Orders = L"ON CALL";       break;
				case SEEKENEMY:   Orders = L"SEEK ENEMY";    break;
				case CLOSEPATROL: Orders = L"CLOSE PATROL";  break;
				case FARPATROL:   Orders = L"FAR PATROL";    break;
				case POINTPATROL: Orders = L"POINT PATROL";  break;
				case RNDPTPATROL: Orders = L"RND PT PATROL"; break;
				default:          Orders = L"UNKNOWN";       break;
			}
			const wchar_t* Attitude;
			switch( pSoldier->bAttitude )
			{
				case DEFENSIVE:   Attitude = L"DEFENSIVE";    break;
				case BRAVESOLO:   Attitude = L"BRAVE SOLO";   break;
				case BRAVEAID:    Attitude = L"BRAVE AID";    break;
				case AGGRESSIVE:  Attitude = L"AGGRESSIVE";   break;
				case CUNNINGSOLO: Attitude = L"CUNNING SOLO"; break;
				case CUNNINGAID:  Attitude = L"CUNNING AID";  break;
				default:          Attitude = L"UNKNOWN";      break;
			}
			pNode = gSoldierInitHead;
			while( pNode )
			{
				if( pNode->pSoldier == pSoldier )
					break;
				pNode = pNode->next;
			}
			SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
			if( pNode )
			{
				gprintf( 0, LINE_HEIGHT * ubLine, L"%ls, %ls, REL EQUIP: %d, REL ATTR: %d",
					Orders, Attitude, pNode->pBasicPlacement->bRelativeEquipmentLevel,
					pNode->pBasicPlacement->bRelativeAttributeLevel );
			}
			else
			{
				gprintf(0, LINE_HEIGHT * ubLine, L"%ls, %ls", Orders, Attitude);
			}
			ubLine++;
		}

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"ID:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		gprintf( 150, LINE_HEIGHT * ubLine, L"%d", pSoldier->ubID );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"HELMETPOS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[HELMETPOS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[HELMETPOS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[HELMETPOS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"VESTPOS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[VESTPOS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[VESTPOS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[VESTPOS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"LEGPOS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[LEGPOS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[LEGPOS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[LEGPOS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"HEAD1POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[HEAD1POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[HEAD1POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[HEAD1POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"HEAD2POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[HEAD2POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[HEAD2POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[HEAD2POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"HANDPOS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[HANDPOS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[HANDPOS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[HANDPOS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SECONDHANDPOS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SECONDHANDPOS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SECONDHANDPOS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SECONDHANDPOS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"BIGPOCK1POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[BIGPOCK1POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[BIGPOCK1POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[BIGPOCK1POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"BIGPOCK2POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[BIGPOCK2POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[BIGPOCK2POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[BIGPOCK2POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"BIGPOCK3POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[BIGPOCK3POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[BIGPOCK3POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[BIGPOCK3POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"BIGPOCK4POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[BIGPOCK4POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[BIGPOCK4POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[BIGPOCK4POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK1POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK1POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK1POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK1POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK2POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK2POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK2POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK2POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK3POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK3POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK3POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK3POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK4POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK4POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK4POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK4POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK5POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK5POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK5POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK5POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK6POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK6POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK6POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK6POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK7POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK7POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK7POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK7POS], LINE_HEIGHT*ubLine );
		ubLine++;

		SetFontShade(LARGEFONT1, FONT_SHADE_GREEN);
		gprintf( 0, LINE_HEIGHT * ubLine, L"SMALLPOCK8POS:");
		SetFontShade(LARGEFONT1, FONT_SHADE_NEUTRAL);
		if( pSoldier->inv[SMALLPOCK8POS].usItem )
			gprintf( 150, LINE_HEIGHT * ubLine, L"%ls", ShortItemNames[pSoldier->inv[SMALLPOCK8POS].usItem] );
		WriteQuantityAndAttachments( &pSoldier->inv[SMALLPOCK8POS], LINE_HEIGHT*ubLine );
		ubLine++;
	}
	else
	{
		SetFont( LARGEFONT1 );
		gprintf( 0,0,L"DEBUG LAND PAGE FOUR" );
		SetFont( LARGEFONT1 );
	}
}

//
// Noise stuff
//

#define MAX_MOVEMENT_NOISE 9

UINT8 MovementNoise(SOLDIERTYPE* pSoldier) // XXX TODO000B
{
 INT32	iStealthSkill, iRoll;
 UINT8	ubMaxVolume, ubVolume, ubBandaged, ubEffLife;
 INT8		bInWater = FALSE;

	if ( pSoldier->bTeam == ENEMY_TEAM )
	{
		return( (UINT8) (MAX_MOVEMENT_NOISE - PreRandom( 2 )) );
	}

	iStealthSkill = 20 + 4 * EffectiveExpLevel( pSoldier ) + ((EffectiveDexterity( pSoldier ) * 4) / 10); // 24-100

	// big bonus for those "extra stealthy" mercs
	if ( pSoldier->ubBodyType == BLOODCAT )
	{
		iStealthSkill += 50;
	}
	else
	{
		iStealthSkill += 25 * NUM_SKILL_TRAITS(pSoldier, STEALTHY);
	}


 //NumMessage("Base Stealth = ",stealthSkill);


	ubBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;
	ubEffLife = pSoldier->bLife + (ubBandaged / 2);

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
	if (Water( pSoldier->sGridNo ) )
	{
		iStealthSkill -= 10; // 10% penalty
	}
	else if (DeepWater( pSoldier->sGridNo ) )
	{
		iStealthSkill -= 20; // 20% penalty
	}

	if ( pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ] )
	{
		// minus 3 percent per bonus AP from adrenaline
		iStealthSkill -= 3 * pSoldier->bDrugEffect[ DRUG_TYPE_ADRENALINE ];
	}

/*
	// if sneaker is too eager and impatient to "do it right"
	if ((pSoldier->bTrait == OVER_ENTHUS) || (pSoldier->bAttitude == AGGRESSIVE))
	{
		ubStealthSkill -= 10;	// 10% penalty
	}
*/
 //NumMessage("Modified Stealth = ",stealthSkill);

	iStealthSkill = __max( iStealthSkill, 0 );

	if (!pSoldier->bStealthMode)	// REGULAR movement
	{
		ubMaxVolume = MAX_MOVEMENT_NOISE - (iStealthSkill / 16);	// 9 - (0 to 6) => 3 to 9

		if (bInWater)
		{
			ubMaxVolume++;		// in water, can be even louder
		}
		switch (pSoldier->usAnimState)
		{
			case CRAWLING:
				ubMaxVolume -= 2;
				break;
			case SWATTING:
				ubMaxVolume -= 1;
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
			ubVolume = 1 + (UINT8) PreRandom(ubMaxVolume);	// actual volume is 1 to max volume
		}
	}
	else			// in STEALTH mode
	{
		iRoll = (INT32) PreRandom(100);	// roll them bones!

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
					ubVolume -= 2;
					break;
				case SWATTING:
					ubVolume -= 1;
					break;
				case RUNNING:
					ubVolume += 3;
					break;
			}
			if (ubVolume < 1)
			{
				ubVolume = 0;
			}
		}
	}

	return( ubVolume );
}

UINT8 DoorOpeningNoise( SOLDIERTYPE *pSoldier )
{
	INT16						sGridNo;
	DOOR_STATUS	*		pDoorStatus;
	UINT8						ubDoorNoise;

	// door being opened gridno is always the pending-action-data2 value
	sGridNo					= pSoldier->sPendingActionData2;
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

void MakeNoise(UINT8 ubNoiseMaker, INT16 sGridNo, INT8 bLevel, UINT8 ubTerrType, UINT8 ubVolume, UINT8 ubNoiseType )
{
	EV_S_NOISE	SNoise;

	SNoise.ubNoiseMaker = ubNoiseMaker;
	SNoise.sGridNo = sGridNo;
	SNoise.bLevel = bLevel;
	SNoise.ubTerrType = ubTerrType;
	SNoise.ubVolume = ubVolume;
	SNoise.ubNoiseType = ubNoiseType;

	if ( gTacticalStatus.ubAttackBusyCount )
	{
		// delay these events until the attack is over!
		AddGameEvent( S_NOISE, DEMAND_EVENT_DELAY, &SNoise );
	}
	else
	{
		// AddGameEvent( S_NOISE, 0, &SNoise );

		// now call directly
		OurNoise( SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubTerrType, SNoise.ubVolume, SNoise.ubNoiseType );

	}
}


static void ProcessNoise(UINT8 ubNoiseMaker, INT16 sGridNo, INT8 bLevel, UINT8 ubTerrType, UINT8 ubBaseVolume, UINT8 ubNoiseType);


void OurNoise( UINT8 ubNoiseMaker, INT16 sGridNo, INT8 bLevel, UINT8 ubTerrType, UINT8 ubVolume, UINT8 ubNoiseType )
{
	SOLDIERTYPE *pSoldier;


#ifdef BYPASSNOISE
	return;
#endif

	// see if anyone actually hears this noise, sees ubNoiseMaker, etc.
	ProcessNoise(ubNoiseMaker, sGridNo, bLevel, ubTerrType,	ubVolume,	ubNoiseType);

	if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) && (ubNoiseMaker < NOBODY) && !gfDelayResolvingBestSightingDueToDoor )
	{
		pSoldier = MercPtrs[ubNoiseMaker];

		// interrupts are possible, resolve them now (we're in control here)
		// (you can't interrupt NOBODY, even if you hear the noise)

		ResolveInterruptsVs(pSoldier,NOISEINTERRUPT);
	}

}


static void HearNoise(SOLDIERTYPE* pSoldier, UINT8 ubNoiseMaker, UINT16 sGridNo, INT8 bLevel, UINT8 ubVolume, UINT8 ubNoiseType, UINT8* ubSeen);
static UINT8 CalcEffVolume(const SOLDIERTYPE* pSoldier, INT16 sGridNo, INT8 bLevel, UINT8 ubNoiseType, UINT8 ubBaseVolume, UINT8 bCheckTerrain, UINT8 ubTerrType1, UINT8 ubTerrType2);
static void TellPlayerAboutNoise(SOLDIERTYPE* pSoldier, UINT8 ubNoiseMaker, INT16 sGridNo, INT8 bLevel, UINT8 ubVolume, UINT8 ubNoiseType, UINT8 ubNoiseDir);


static void ProcessNoise(UINT8 ubNoiseMaker, INT16 sGridNo, INT8 bLevel, UINT8 ubTerrType, UINT8 ubBaseVolume, UINT8 ubNoiseType)
{
	SOLDIERTYPE *pSoldier;
	UINT8 bLoop, bTeam;
	UINT8 ubLoudestEffVolume, ubEffVolume;
//	UINT8 ubPlayVolume;
	INT8 bCheckTerrain = FALSE;
	UINT8 ubSourceTerrType, ubSource;
	INT8 bTellPlayer = FALSE, bHeard, bSeen;
	UINT8 ubHeardLoudestBy, ubNoiseDir, ubLoudestNoiseDir;


#ifdef RECORDOPPLIST
	fprintf(OpplistFile,"PN: nType=%s, nMaker=%d, g=%d, tType=%d, bVol=%d\n",
		NoiseTypeStr[noiseType],ubNoiseMaker,sGridNo,ubTerrType,baseVolume);
#endif

	// if the base volume itself was negligible
	if (!ubBaseVolume)
		return;


	// EXPLOSIONs are special, because they COULD be caused by a merc who is
	// no longer alive (but he placed the bomb or flaky grenade in the past).
	// Later noiseMaker gets whacked to NOBODY anyway, so that's OK.  So a
	// dead noiseMaker is only used here to decide WHICH soldiers HearNoise().

	// if noise is made by a person, AND it's not noise from an explosion
	if ((ubNoiseMaker < NOBODY) && (ubNoiseType != NOISE_EXPLOSION))
	{
		// inactive/not in sector/dead soldiers, shouldn't be making noise!
		if (!Menptr[ubNoiseMaker].bActive || !Menptr[ubNoiseMaker].bInSector ||
        Menptr[ubNoiseMaker].uiStatusFlags & SOLDIER_DEAD)
		{
#ifdef BETAVERSION
			NumMessage("ProcessNoise: ERROR - Noisemaker is inactive/not in sector/dead, Guy #",ubNoiseMaker);
#endif
			return;
		}

		// if he's out of life, and this isn't just his "dying scream" which is OK
		if (!Menptr[ubNoiseMaker].bLife && (ubNoiseType != NOISE_SCREAM))
		{
#ifdef BETAVERSION
			NumMessage("ProcessNoise: ERROR - Noisemaker is lifeless, Guy #",ubNoiseMaker);
#endif
			return;
		}
	}


	// DETERMINE THE TERRAIN TYPE OF THE GRIDNO WHERE NOISE IS COMING FROM

	ubSourceTerrType = gpWorldLevelData[sGridNo].ubTerrainID;

	// if we have now somehow obtained a valid terrain type
	if ((ubSourceTerrType >= FLAT_GROUND) || (ubSourceTerrType <= DEEP_WATER))
	{
		//NumMessage("Source Terrain Type = ",ubSourceTerrType);
		bCheckTerrain = TRUE;
	}
	// else give up trying to get terrain type, just assume sound isn't muffled


	// DETERMINE THE *PERCEIVED* SOURCE OF THE NOISE
	switch (ubNoiseType)
	{
		// for noise generated by an OBJECT shot/thrown/dropped by the noiseMaker
		case NOISE_ROCK_IMPACT:
			gsWhoThrewRock = ubNoiseMaker;
			//fall through here!!!
		case NOISE_BULLET_IMPACT:
		case NOISE_GRENADE_IMPACT:
		case NOISE_EXPLOSION:
			// the source of the noise is not at all obvious, so hide it from
			// the listener and maintain noiseMaker's cover by making source NOBODY
			ubSource = NOBODY;
			break;

		default:
			// normal situation: the noiseMaker is obviously the source of the noise
			ubSource = ubNoiseMaker;
			break;
	}

	// LOOP THROUGH EACH TEAM
	for (bTeam = 0; bTeam < MAXTEAMS; bTeam++)
	{
		// skip any inactive teams
		if (!gTacticalStatus.Team[bTeam].bTeamActive)
		{
			continue;
		}

		// if a the noise maker is a person, not just NOBODY
		if (ubNoiseMaker < NOBODY)
		{
			// if this team is the same TEAM as the noise maker's
			// (for now, assume we will report noises by unknown source on same SIDE)
			// OR, if the noise maker is currently in sight to this HUMAN team

			// CJC: changed to if the side is the same side as the noise maker's!
			// CJC: changed back!

			if (bTeam == Menptr[ubNoiseMaker].bTeam)
			{
				continue;
			}

			if (gTacticalStatus.Team[bTeam].bHuman)
			{
				if (gbPublicOpplist[bTeam][ubNoiseMaker] == SEEN_CURRENTLY)
				{
					continue;
				}
			}
		}

		// tell player about noise if enemies are present
		bTellPlayer = gTacticalStatus.fEnemyInSector && ( !(gTacticalStatus.uiFlags & INCOMBAT) || (gTacticalStatus.ubCurrentTeam) );

#ifndef TESTNOISE
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
		}

		// if noise was made by a person
		if (ubNoiseMaker < NOBODY)
		{
			// if noisemaker has been *PUBLICLY* SEEN OR HEARD during THIS TURN
			if ((gbPublicOpplist[bTeam][ubNoiseMaker] == SEEN_CURRENTLY) || // seen now
				(gbPublicOpplist[bTeam][ubNoiseMaker] == SEEN_THIS_TURN) || // seen this turn
				(gbPublicOpplist[bTeam][ubNoiseMaker] == HEARD_THIS_TURN))  // heard this turn
			{
				// then don't bother reporting any noise made by him to the player
				bTellPlayer = FALSE;
			}
			/*
			else if ( (Menptr[ubNoiseMaker].bVisible == TRUE) && (bTeam == gbPlayerNum) )
			{
				ScreenMsg( MSG_FONT_YELLOW, MSG_TESTVERSION, L"Handling noise from person not currently seen in player's public opplist" );
			}
			*/

			if ( MercPtrs[ ubNoiseMaker ]->bLife == 0 )
			{
				// this guy is dead (just dying) so don't report to player
				bTellPlayer = FALSE;
			}

		}
#endif

		// refresh flags for this new team
		bHeard = FALSE;
		bSeen = FALSE;
		ubLoudestEffVolume = 0;
		ubHeardLoudestBy = NOBODY;

		// All mercs on this team check if they are eligible to hear this noise
		for (bLoop = gTacticalStatus.Team[bTeam].bFirstID,pSoldier = Menptr + bLoop; bLoop <= gTacticalStatus.Team[bTeam].bLastID; bLoop++,pSoldier++)
		{
			// if this "listener" is inactive, or in no condition to care
			if (!pSoldier->bActive || !pSoldier->bInSector || pSoldier->uiStatusFlags & SOLDIER_DEAD || (pSoldier->bLife < OKLIFE) || pSoldier->ubBodyType == LARVAE_MONSTER)
			{
				continue;          // skip him!
			}

			if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE && pSoldier->bTeam == OUR_TEAM  )
			{
				continue; // skip
			}

			if ( bTeam == gbPlayerNum && pSoldier->bAssignment == ASSIGNMENT_POW )
			{
				// POWs should not be processed for noise
				continue;
			}

			// if a the noise maker is a person, not just NOBODY
			if (ubNoiseMaker < NOBODY)
			{
				// if this listener can see this noise maker
				if (pSoldier->bOppList[ubNoiseMaker] == SEEN_CURRENTLY)
				{
					// civilians care about gunshots even if they come from someone they can see
					if ( !( pSoldier->bNeutral && ubNoiseType == NOISE_GUNFIRE ) )
					{
						continue;        // then who cares whether he can also hear the guy?
					}
				}

				// screen out allied militia from hearing us
				switch( MercPtrs[ ubNoiseMaker ]->bTeam )
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
						// if the noisemaker is militia and still on our side, ignore noise if we're listening
						if ( pSoldier->bTeam == OUR_TEAM && MercPtrs[ ubNoiseMaker ]->bSide == 0 )
						{
							continue;
						}
						break;
				}

				if ( gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_N )
				{
					// in the bloodcat arena sector, skip noises between army & bloodcats
					if ( pSoldier->bTeam == ENEMY_TEAM && MercPtrs[ ubNoiseMaker ]->bTeam == CREATURE_TEAM )
					{
						continue;
					}
					if ( pSoldier->bTeam == CREATURE_TEAM && MercPtrs[ ubNoiseMaker ]->bTeam == ENEMY_TEAM )
					{
						continue;
					}
				}


			}
			else
			{
				// screen out allied militia from hearing us
				if ( (ubNoiseMaker == NOBODY) && pSoldier->bTeam == MILITIA_TEAM && pSoldier->bSide == 0 )
				{
					continue;
				}
			}

			if ( (pSoldier->bTeam == CIV_TEAM) && (ubNoiseType == NOISE_GUNFIRE || ubNoiseType == NOISE_EXPLOSION) )
			{
				pSoldier->ubMiscSoldierFlags |= SOLDIER_MISC_HEARD_GUNSHOT;
			}

			// Can the listener hear noise of that volume given his circumstances?
			ubEffVolume = CalcEffVolume(pSoldier,sGridNo,bLevel,ubNoiseType,ubBaseVolume,bCheckTerrain,pSoldier->bOverTerrainType,ubSourceTerrType);

#ifdef RECORDOPPLIST
			fprintf(OpplistFile,"PN: guy %d - effVol=%d,chkTer=%d,pSoldier->tType=%d,srcTType=%d\n",
		     bLoop,effVolume,bCheckTerrain,pSoldier->terrtype,ubSourceTerrType);
#endif


			if (ubEffVolume > 0)
			{
				// ALL RIGHT!  Passed all the tests, this listener hears this noise!!!
				HearNoise(pSoldier,ubSource,sGridNo,bLevel,ubEffVolume,ubNoiseType,&bSeen);

				bHeard = TRUE;

				ubNoiseDir = atan8(CenterX(pSoldier->sGridNo),CenterY(pSoldier->sGridNo),CenterX(sGridNo),CenterY(sGridNo));

				// check the 'noise heard & reported' bit for that soldier & direction
				if ( ubNoiseType != NOISE_MOVEMENT || bTeam != OUR_TEAM || (pSoldier->bInterruptDuelPts != NO_INTERRUPT) || !(pSoldier->ubMovementNoiseHeard & (1 << ubNoiseDir) ) )
				{
					if (ubEffVolume > ubLoudestEffVolume)
					{
						ubLoudestEffVolume = ubEffVolume;
						ubHeardLoudestBy = pSoldier->ubID;
						ubLoudestNoiseDir = ubNoiseDir;
					}
				}

			}
			else
			{
       //NameMessage(pSoldier," can't hear this noise",2500);
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
				if (bTellPlayer && ubHeardLoudestBy != NOBODY )
				{
					// the merc that heard it the LOUDEST is the one to comment
					// should add level to this function call
					TellPlayerAboutNoise(MercPtrs[ubHeardLoudestBy],ubNoiseMaker,sGridNo,bLevel,ubLoudestEffVolume,ubNoiseType, ubLoudestNoiseDir);

					if ( ubNoiseType == NOISE_MOVEMENT)
					{
						MercPtrs[ ubHeardLoudestBy ]->ubMovementNoiseHeard |= (1 << ubNoiseDir);
					}

				}
				//if ( !(pSoldier->ubMovementNoiseHeard & (1 << ubNoiseDir) ) )
			}
#ifdef REPORTTHEIRNOISE
			else   // debugging: report noise heard by other team's soldiers
			{
				if (bTellPlayer)
				{
					TellPlayerAboutNoise(MercPtrs[ubHeardLoudestBy],ubNoiseMaker,sGridNo,bLevel,ubLoudestEffVolume,ubNoiseType, ubLoudestNoiseDir);
				}
			}
#endif
		}

		// if the listening team is human-controlled AND
		// the noise's source is another soldier
		// (computer-controlled teams don't radio or automatically report NOISE)
		if (gTacticalStatus.Team[bTeam].bHuman && (ubSource < NOBODY))
		{
			// if ubNoiseMaker was seen by at least one member of this team
			if (bSeen)
			{
				// this team is now allowed to report sightings and set Public flags
				OurTeamRadiosRandomlyAbout(ubSource);
			}
			else // not seen
			{
				if (bHeard)
				{
#ifdef RECORDOPPLIST
					fprintf(OpplistFile,"UpdatePublic (ProcessNoise/heard) for team %d about %d\n",team,ubSource);
#endif

					// mark noise maker as having been PUBLICLY heard THIS TURN
					UpdatePublic(bTeam,ubSource,HEARD_THIS_TURN,sGridNo,bLevel);
				}
			}
		}
	}

	gsWhoThrewRock = NOBODY;
}


static UINT8 CalcEffVolume(const SOLDIERTYPE* pSoldier, INT16 sGridNo, INT8 bLevel, UINT8 ubNoiseType, UINT8 ubBaseVolume, UINT8 bCheckTerrain, UINT8 ubTerrType1, UINT8 ubTerrType2)
{
	INT32 iEffVolume, iDistance;

	if ( pSoldier->inv[HEAD1POS].usItem == WALKMAN || pSoldier->inv[HEAD2POS].usItem == WALKMAN )
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

	//sprintf(tempstr,"CalcEffVolume BY %s for gridno %d, baseVolume = %d",pSoldier->name,gridno,baseVolume);
	//PopMessage(tempstr);

	// adjust default noise volume by listener's hearing capability
	iEffVolume = (INT32) ubBaseVolume + (INT32) DecideHearing( pSoldier );


	// effective volume reduced by listener's number of opponents in sight
	iEffVolume -= pSoldier->bOppCnt;


 // calculate the distance (in adjusted pixels) between the source of the
 // noise (gridno) and the location of the would-be listener (pSoldier->gridno)
 iDistance = (INT32) PythSpacesAway( pSoldier->sGridNo, sGridNo );
 /*
 distance = AdjPixelsAway(pSoldier->x,pSoldier->y,CenterX(sGridNo),CenterY(sGridNo));

	distance /= 15;      // divide by 15 to convert from adj. pixels to tiles
	*/
	//NumMessage("Distance = ",distance);

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
	}
	*/

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
		if (bCheckTerrain)
		{
			// if, between noise and listener, one is outside and one is inside

			// NOTE: This is a pretty dumb way of doing things, since it won't detect
			// the presence of walls between 2 spots both inside or both outside, but
			// given our current system it's the best that we can do

			if (((ubTerrType1 == FLAT_FLOOR) && (ubTerrType2 != FLAT_FLOOR)) ||
				((ubTerrType1 != FLAT_FLOOR) && (ubTerrType2 == FLAT_FLOOR)))
			{
				//PopMessage("Sound is muffled by wall(s)");

				// sound is muffled, reduce the effective volume of the noise
				iEffVolume -= 5;
			}
		}

	}

	//NumMessage("effVolume = ",ubEffVolume);
	if (iEffVolume > 0)
	{
		return( (UINT8) iEffVolume );
	}
	else
	{
		return( 0 );
	}
}


static void HearNoise(SOLDIERTYPE* pSoldier, UINT8 ubNoiseMaker, UINT16 sGridNo, INT8 bLevel, UINT8 ubVolume, UINT8 ubNoiseType, UINT8* ubSeen)
{
	INT16		sNoiseX, sNoiseY;
	INT8		bHadToTurn = FALSE, bSourceSeen = FALSE;
	INT8		bOldOpplist;
	INT16		sDistVisible;
	INT8		bDirection;
	BOOLEAN fMuzzleFlash = FALSE;

//	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "%d hears noise from %d (%d/%d) volume %d", pSoldier->ubID, ubNoiseMaker, sGridNo, bLevel, ubVolume ) );


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
	if ( ubNoiseType == NOISE_GUNFIRE && ubNoiseMaker != NOBODY && MercPtrs[ ubNoiseMaker ]->fMuzzleFlash )
	{
		sNoiseX = CenterX(sGridNo);
		sNoiseY = CenterY(sGridNo);
		bDirection = atan8(pSoldier->sX,pSoldier->sY,sNoiseX,sNoiseY);
		if (pSoldier->bDirection != bDirection &&
				pSoldier->bDirection != OneCDirection(bDirection) &&
				pSoldier->bDirection != OneCCDirection(bDirection))
		{
			// temporarily turn off muzzle flash so DistanceVisible can be calculated without it
			MercPtrs[ ubNoiseMaker ]->fMuzzleFlash = FALSE;
			fMuzzleFlash = TRUE;
		}
	}

	sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, bLevel );

	if ( fMuzzleFlash )
	{
		// turn flash on again
		MercPtrs[ ubNoiseMaker ]->fMuzzleFlash = TRUE;
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

#ifdef RECORDOPPLIST
		fprintf(OpplistFile,"HN: %s by %2d(g%4d,x%3d,y%3d) at %2d(g%4d,x%3d,y%3d), hTT=%d\n",
			(bSourceSeen) ? "SCS" : "FLR",
			pSoldier->guynum,pSoldier->sGridNo,pSoldier->sX,pSoldier->sY,
			ubNoiseMaker,sGridNo,sNoiseX,sNoiseY,
			bHadToTurn);
#endif
	}

	// if noise is made by a person
	if (ubNoiseMaker < NOBODY)
	{
		bOldOpplist = pSoldier->bOppList[ubNoiseMaker];

		// WE ALREADY KNOW THAT HE'S ON ANOTHER TEAM, AND HE'S NOT BEING SEEN
		// ProcessNoise() ALREADY DID THAT WORK FOR US

		if (bSourceSeen)
		{
			ManSeesMan(pSoldier, MercPtrs[ubNoiseMaker], Menptr[ubNoiseMaker].sGridNo, Menptr[ubNoiseMaker].bLevel, CALLER_UNKNOWN);

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
			}

		}
		else         // noise maker still can't be seen
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
			UpdatePersonal(pSoldier,ubNoiseMaker,HEARD_THIS_TURN,sGridNo, bLevel);

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
		const SOLDIERTYPE* const noise_maker = GetMan(ubNoiseMaker);
		if (!CONSIDERED_NEUTRAL(noise_maker, pSoldier) && !CONSIDERED_NEUTRAL(pSoldier, noise_maker) && pSoldier->bSide != noise_maker->bSide)
		{
			// regardless of whether the noisemaker (who's not NOBODY) was seen or not,
			// as long as listener meets minimum interrupt conditions
			if ( gfDelayResolvingBestSightingDueToDoor)
			{
				if ( bSourceSeen && (!( (gTacticalStatus.uiFlags & TURNBASED) && ( gTacticalStatus.uiFlags & INCOMBAT ) ) || (gubSightFlags & SIGHTINTERRUPT && StandardInterruptConditionsMet(pSoldier,ubNoiseMaker,bOldOpplist)) ) )
				{
					// we should be adding this to the array for the AllTeamLookForAll to handle
					// since this is a door opening noise, add a bonus equal to half the door volume
					UINT8	ubPoints;

					ubPoints = CalcInterruptDuelPts( pSoldier, ubNoiseMaker, TRUE );
					if ( ubPoints != NO_INTERRUPT )
					{
						// require the enemy not to be dying if we are the sighter; in other words,
						// always add for AI guys, and always add for people with life >= OKLIFE
						if (pSoldier->bTeam != gbPlayerNum || noise_maker->bLife >= OKLIFE)
						{
							ReevaluateBestSightingPosition( pSoldier, (UINT8) (ubPoints + (ubVolume / 2)) );
						}
					}
				}
			}
			else
			{
				if ( (gTacticalStatus.uiFlags & TURNBASED) && ( gTacticalStatus.uiFlags & INCOMBAT ) )
				{
					if (StandardInterruptConditionsMet(pSoldier,ubNoiseMaker,bOldOpplist))
					{
						// he gets a chance to interrupt the noisemaker
						pSoldier->bInterruptDuelPts = CalcInterruptDuelPts(pSoldier,ubNoiseMaker, TRUE );
						DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Calculating int duel pts in noise code, %d has %d points", pSoldier->ubID, pSoldier->bInterruptDuelPts ) );
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
		if (!bSourceSeen || ((ubNoiseType == NOISE_ROCK_IMPACT) && (bHadToTurn) ) || ubNoiseType == NOISE_SILENT_ALARM )
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

				SetNewSituation( pSoldier );  // force a fresh AI decision to be made
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
			if ((gTacticalStatus.uiFlags & TURNBASED) && ( gTacticalStatus.uiFlags & INCOMBAT ) && ubNoiseType == NOISE_ROCK_IMPACT )
			{
				// give every ELIGIBLE listener an automatic interrupt, since it's
				// reasonable to assume the guy throwing wants to wait for their reaction!
				if (StandardInterruptConditionsMet(pSoldier,NOBODY,FALSE))
				{
					pSoldier->bInterruptDuelPts = AUTOMATIC_INTERRUPT;	     	// force automatic interrupt
					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Calculating int duel pts in noise code, %d has %d points", pSoldier->ubID, pSoldier->bInterruptDuelPts ) );
				}
				else
				{
					pSoldier->bInterruptDuelPts = NO_INTERRUPT;
				}
			}
		}
	}
}


static void TellPlayerAboutNoise(SOLDIERTYPE* pSoldier, UINT8 ubNoiseMaker, INT16 sGridNo, INT8 bLevel, UINT8 ubVolume, UINT8 ubNoiseType, UINT8 ubNoiseDir)
{
	UINT8 ubVolumeIndex;

	// CJC: tweaked the noise categories upwards a bit because our movement noises can be louder now.
	if (ubVolume < 4)
	{
		ubVolumeIndex = 0;		// 1-3: faint noise
	}
	else if (ubVolume < 8)	// 4-7: definite noise
	{
		ubVolumeIndex = 1;
	}
	else if (ubVolume < 12)	// 8-11: loud noise
	{
		ubVolumeIndex = 2;
	}
	else										// 12+: very loud noise
	{
		ubVolumeIndex = 3;
	}

	// display a message about a noise...
	// e.g. Sidney hears a loud splash from/to? the north.

	if ( ubNoiseMaker != NOBODY && pSoldier->bTeam == gbPlayerNum && pSoldier->bTeam == Menptr[ubNoiseMaker].bTeam )
	{
		#ifdef JA2BETAVERSION
			ScreenMsg( MSG_FONT_RED, MSG_ERROR, L"ERROR! TAKE SCREEN CAPTURE AND TELL CAMFIELD NOW!" );
			ScreenMsg( MSG_FONT_RED, MSG_ERROR, L"%ls (%d) heard noise from %ls (%d), noise at %dL%d, type %d", pSoldier->name, pSoldier->ubID, Menptr[ubNoiseMaker].name, ubNoiseMaker, sGridNo, bLevel, ubNoiseType );
		#endif
	}

	if ( bLevel == pSoldier->bLevel || ubNoiseType == NOISE_EXPLOSION || ubNoiseType == NOISE_SCREAM || ubNoiseType == NOISE_ROCK_IMPACT || ubNoiseType == NOISE_GRENADE_IMPACT )
	{
		ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, pNewNoiseStr[ubNoiseType], pSoldier->name, pNoiseVolStr[ubVolumeIndex], pDirectionStr[ubNoiseDir] );
	}
	else if ( bLevel > pSoldier->bLevel )
	{
		// from above!
		ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, pNewNoiseStr[ubNoiseType], pSoldier->name, pNoiseVolStr[ubVolumeIndex], gzLateLocalizedString[6] );
	}
	else
	{
		// from below!
		ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, pNewNoiseStr[ubNoiseType], pSoldier->name, pNoiseVolStr[ubVolumeIndex], gzLateLocalizedString[7] );
	}

	// if the quote was faint, say something
	if (ubVolumeIndex == 0)
	{
		if ( !AreInMeanwhile( ) && !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV) && pSoldier->ubTurnsUntilCanSayHeardNoise == 0)
		{
			TacticalCharacterDialogue( pSoldier, QUOTE_HEARD_SOMETHING );
			if ( gTacticalStatus.uiFlags & INCOMBAT )
			{
				pSoldier->ubTurnsUntilCanSayHeardNoise = 2;
			}
			else
			{
				pSoldier->ubTurnsUntilCanSayHeardNoise = 5;
			}
		}
	}

	// flag soldier as having reported noise in a particular direction

}

void VerifyAndDecayOpplist(SOLDIERTYPE *pSoldier)
{
	UINT32 uiLoop;
	INT8 *pPersOL;           // pointer into soldier's opponent list
	SOLDIERTYPE *pOpponent;

	// reduce all seen/known opponent's turn counters by 1 (towards 0)
	// 1) verify accuracy of the opplist by testing sight vs known opponents
	// 2) increment opplist value if opponent is known but not currenly seen
	// 3) forget about known opponents who haven't been noticed in some time

	// if soldier is unconscious, make sure his opplist is wiped out & bail out
	if (pSoldier->bLife < OKLIFE)
	{
		memset(pSoldier->bOppList,NOT_HEARD_OR_SEEN,sizeof(pSoldier->bOppList));
		pSoldier->bOppCnt = 0;
		return;
	}

	// if any new opponents were seen earlier and not yet radioed
	if (pSoldier->bNewOppCnt)
	{
#ifdef BETAVERSION
		sprintf(tempstr,"VerifyAndDecayOpplist: WARNING - %d(%ls) still has %d NEW OPPONENTS - lastCaller %ls/%ls",
			pSoldier->guynum,ExtMen[pSoldier->guynum].name,pSoldier->newOppCnt,
			LastCallerText[ExtMen[pSoldier->guynum].lastCaller],
			LastCaller2Text[ExtMen[pSoldier->guynum].lastCaller2]);

#ifdef TESTVERSION	// make this ERROR/BETA again when it's fixed!
		PopMessage(tempstr);
#endif

#endif

		if (pSoldier->uiStatusFlags & SOLDIER_PC)
		{
			RadioSightings(pSoldier,EVERYBODY,pSoldier->bTeam);
		}

		pSoldier->bNewOppCnt = 0;
	}

	// man looks for each of his opponents WHO ARE ALREADY KNOWN TO HIM
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is active, here, and alive
		if (pOpponent != NULL && pOpponent->bLife)
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
			 DECAY_OPPLIST_VALUE( *pPersOL );
			 /*
       // if opponent was SEEN recently but is NOT visible right now
       if (*pPersOL >= SEEN_THIS_TURN)
        {
         (*pPersOL)++;          // increment #turns it's been since last seen

         // if it's now been longer than the maximum we care to remember
         if (*pPersOL > SEEN_2_TURNS_AGO)
           *pPersOL = 0;        // forget that we knew this guy
        }
       else
        {
         // if opponent was merely HEARD recently, not actually seen
         if (*pPersOL <= HEARD_THIS_TURN)
          {
           (*pPersOL)--;        // increment #turns it's been since last heard

	   // if it's now been longer than the maximum we care to remember
	   if (*pPersOL < HEARD_2_TURNS_AGO)
	     *pPersOL = 0;      // forget that we knew this guy
			  }
				}
			*/
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
	UINT32 uiLoop;
	INT8 *pPersOL;           // pointer into soldier's opponent list
	SOLDIERTYPE *pOpponent;

	// reduce all currently seen opponent's turn counters by 1 (towards 0)

	// if soldier is unconscious, make sure his opplist is wiped out & bail out
	if (pSoldier->bLife < OKLIFE)
	{
		// must make sure that public opplist is kept to match...
		for ( uiLoop = 0; uiLoop < TOTAL_SOLDIERS; uiLoop++ )
		{
			if ( pSoldier->bOppList[ uiLoop ] == SEEN_CURRENTLY )
			{
				HandleManNoLongerSeen( pSoldier, MercPtrs[ uiLoop ], &(pSoldier->bOppList[ uiLoop ]), &(gbPublicOpplist[ pSoldier->bTeam ][ uiLoop ]) );
			}
		}
	//void HandleManNoLongerSeen( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pOpponent, INT8 * pPersOL, INT8 * pbPublOL )

		memset(pSoldier->bOppList,NOT_HEARD_OR_SEEN,sizeof(pSoldier->bOppList));
		pSoldier->bOppCnt = 0;
		return;
	}

	// man looks for each of his opponents WHO IS CURRENTLY SEEN
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pOpponent = MercSlots[ uiLoop ];

		// if this merc is active, here, and alive
		if (pOpponent != NULL && pOpponent->bLife)
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
				if (!CONSIDERED_NEUTRAL( pOpponent, pSoldier ) && !CONSIDERED_NEUTRAL( pSoldier, pOpponent ) && (pSoldier->bSide != pOpponent->bSide))
				{
					RemoveOneOpponent(pSoldier);
				}

      }
    }
  }
}



void VerifyPublicOpplistDueToDeath(SOLDIERTYPE *pSoldier)
{
	UINT32 uiLoop,uiTeamMateLoop;
	INT8 *pPersOL,*pMatePersOL;    // pointers into soldier's opponent list
	SOLDIERTYPE *pOpponent,*pTeamMate;
	BOOLEAN bOpponentStillSeen;


	// OK, someone died. Anyone that the deceased ALONE saw has to decay
	// immediately in the Public Opplist.


  // If deceased didn't see ANYONE, don't bother
	if (pSoldier->bOppCnt == 0)
	{
		return;
	}


	// Deceased looks for each of his opponents who is "seen currently"
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		// first, initialize flag since this will be a "new" opponent
		bOpponentStillSeen = FALSE;

		// grab a pointer to the "opponent"
		pOpponent = MercSlots[ uiLoop ];

		// if this opponent is active, here, and alive
		if (pOpponent != NULL && pOpponent->bLife)
		{
			// if this opponent is on the same team, he's no opponent, so skip him
			if (pSoldier->bTeam == pOpponent->bTeam)
			{
				continue;
			}

			// point to what the deceased's personal opplist value is
			pPersOL = pSoldier->bOppList + pOpponent->ubID;

			// if this opponent was CURRENTLY SEEN by the deceased (before his
			// untimely demise)
			if (*pPersOL == SEEN_CURRENTLY)
			{
				// then we need to know if any teammates ALSO see this opponent, so loop through
				// trying to find ONE witness to the death...
				for (uiTeamMateLoop = 0; uiTeamMateLoop < guiNumMercSlots; uiTeamMateLoop++)
				{
					// grab a pointer to the potential teammate
					pTeamMate = MercSlots[ uiTeamMateLoop ];

					// if this teammate is active, here, and alive
					if (pTeamMate != NULL && pTeamMate->bLife)
					{
						// if this opponent is NOT on the same team, then skip him
						if (pTeamMate->bTeam != pSoldier->bTeam)
						{
							continue;
						}

						// point to what the teammate's personal opplist value is
						pMatePersOL = pTeamMate->bOppList + pOpponent->ubID;

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
				DECAY_OPPLIST_VALUE( gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID] );
			}
		}
	}
}


static void DecayWatchedLocs(INT8 bTeam);


void DecayPublicOpplist(INT8 bTeam)
{
	UINT32 uiLoop;
	INT8 bNoPubliclyKnownOpponents = TRUE;
	SOLDIERTYPE *pSoldier;
	INT8 *pbPublOL;


	//NumMessage("Decay for team #",team);

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
	for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
	{
		pSoldier = MercSlots[uiLoop];

		// for every active, living soldier on ANOTHER team
		if (pSoldier && pSoldier->bLife && (pSoldier->bTeam != bTeam))
		{
			// hang a pointer to the byte holding team's public opplist for this merc
			pbPublOL = &gbPublicOpplist[bTeam][pSoldier->ubID];

			if (*pbPublOL == NOT_HEARD_OR_SEEN)
			{
				continue;
			}

			// well, that make this a "publicly known opponent", so nuke that flag
			bNoPubliclyKnownOpponents = FALSE;

			// if this person has been SEEN recently, but is not currently visible
			if (*pbPublOL >= SEEN_THIS_TURN)
			{
				(*pbPublOL)++;      // increment how long it's been
			}
			else
			{
				// if this person has been only HEARD recently
				if (*pbPublOL <= HEARD_THIS_TURN)
				{
					(*pbPublOL)--;    // increment how long it's been
				}
			}

			// if it's been longer than the maximum we care to remember
			if ((*pbPublOL > OLDEST_SEEN_VALUE) || (*pbPublOL < OLDEST_HEARD_VALUE))
			{
#ifdef RECORDOPPLIST
				fprintf(OpplistFile,"UpdatePublic (DecayPublicOpplist) for team %d about %d\n",team,pSoldier->guynum);
#endif

				// forget about him,
				// and also forget where he was last seen (it's been too long)
				// this is mainly so POINT_PATROL guys don't SEEK_OPPONENTs forever
				UpdatePublic(bTeam,pSoldier->ubID,NOT_HEARD_OR_SEEN,NOWHERE,0);
			}
		}
	}

	// if all opponents are publicly unknown (NOT_HEARD_OR_SEEN)
	if (bNoPubliclyKnownOpponents)
	{
		// forget about the last radio alert (ie. throw away who made the call)
		// this is mainly so POINT_PATROL guys don't SEEK_FRIEND forever after
		gTacticalStatus.Team[bTeam].ubLastMercToRadio = NOBODY;
	}

	// decay watched locs as well
	DecayWatchedLocs( bTeam );
}

// bit of a misnomer; this is now decay all opplists
void NonCombatDecayPublicOpplist( UINT32 uiTime )
{
	UINT32	cnt;

	if ( uiTime - gTacticalStatus.uiTimeSinceLastOpplistDecay >= TIME_BETWEEN_RT_OPPLIST_DECAYS)
	{
		// decay!
		for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
		{
			if ( MercSlots[ cnt ] )
			{
				VerifyAndDecayOpplist( MercSlots[ cnt ] );
			}
		}


		for( cnt = 0; cnt < MAXTEAMS; cnt++ )
		{
			if ( gTacticalStatus.Team[ cnt ].bMenInSector > 0 )
			{
				// decay team's public opplist
				DecayPublicOpplist( (INT8)cnt );
			}
		}
		// update time
		gTacticalStatus.uiTimeSinceLastOpplistDecay = uiTime;
	}
}

void RecalculateOppCntsDueToNoLongerNeutral( SOLDIERTYPE * pSoldier )
{
	UINT32					uiLoop;
	SOLDIERTYPE *		pOpponent;

	pSoldier->bOppCnt = 0;

	if (!pSoldier->bNeutral)
	{
		for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
		{
			pOpponent = MercSlots[uiLoop];

			// for every active, living soldier on ANOTHER team
			if (pOpponent && pOpponent->bLife && !pOpponent->bNeutral && (pOpponent->bTeam != pSoldier->bTeam) && (!CONSIDERED_NEUTRAL( pOpponent, pSoldier ) && !CONSIDERED_NEUTRAL( pSoldier, pOpponent ) && (pSoldier->bSide != pOpponent->bSide)) )
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
	UINT32					uiLoop;
	SOLDIERTYPE *		pOpponent;

	if (pSoldier->bNeutral)
	{
		pSoldier->bOppCnt = 0;

		for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++)
		{
			pOpponent = MercSlots[uiLoop];

			// for every active, living soldier on ANOTHER team
			if (pOpponent && pOpponent->bLife && !pOpponent->bNeutral && (pOpponent->bTeam != pSoldier->bTeam) && !CONSIDERED_NEUTRAL( pSoldier, pOpponent ) && (pSoldier->bSide != pOpponent->bSide) )
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
	INT8		bOldOppList;
	UINT8		ubTileSightLimit;
	BOOLEAN fSeesAttacker = FALSE;
	INT8		bDirection;
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

	if ( pDefender->ubBodyType == LARVAE_MONSTER || (pDefender->uiStatusFlags & SOLDIER_VEHICLE && pDefender->bTeam == OUR_TEAM) )
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
		ManSeesMan(pDefender, pAttacker, pAttacker->sGridNo, pAttacker->bLevel, CALLER_UNKNOWN);

		// newOppCnt not needed here (no radioing), must get reset right away
		// CJC: Huh? well, leave it in for now
		pDefender->bNewOppCnt = 0;


		if (pDefender->bTeam == gbPlayerNum)
		{
			// EXPERIENCE GAIN (5): Victim notices/sees a previously UNSEEN attacker
			StatChange( pDefender, EXPERAMT, 5, FALSE );

			// mark attacker as being SEEN right now
			RadioSightings( pDefender, pAttacker->ubID, pDefender->bTeam );

		}
		// NOTE: ENEMIES DON'T REPORT A SIGHTING PUBLICLY UNTIL THEY RADIO IT IN!
		else
		{
			// go to threatening stance
			ReevaluateEnemyStance( pDefender, pDefender->usAnimState );
		}
	}
	else  // victim NOTICED the attack, but CAN'T SEE the actual attacker
	{
		SetNewSituation( pDefender );          // re-evaluate situation

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
			UpdatePublic( pDefender->bTeam, pAttacker->ubID, HEARD_THIS_TURN, pAttacker->sGridNo, pAttacker->bLevel );
		}
  }

	if ( StandardInterruptConditionsMet( pDefender, pAttacker->ubID, bOldOppList ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("INTERRUPT: NoticeUnseenAttacker, standard conditions are met; defender %d, attacker %d", pDefender->ubID, pAttacker->ubID ) );

		// calculate the interrupt duel points
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Calculating int duel pts for defender in NUA" );
		pDefender->bInterruptDuelPts = CalcInterruptDuelPts( pDefender, pAttacker->ubID, FALSE);
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
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Calculating int duel pts for attacker in NUA" );
		pAttacker->bInterruptDuelPts = CalcInterruptDuelPts( pAttacker, pDefender->ubID, FALSE );
		if ( InterruptDuel( pDefender, pAttacker ) )
		{
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("INTERRUPT: NoticeUnseenAttacker, defender pts %d, attacker pts %d, defender gets interrupt", pDefender->bInterruptDuelPts, pAttacker->bInterruptDuelPts ) );
			AddToIntList( pAttacker->ubID, FALSE, TRUE);
			AddToIntList( pDefender->ubID, TRUE, TRUE);
			DoneAddingToIntList();
		}
		// either way, clear out both sides' duelPts fields to prepare next duel
		pDefender->bInterruptDuelPts = NO_INTERRUPT;
		#ifdef DEBUG_INTERRUPTS
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Resetting int pts for %d in NUA", pDefender->ubID ) );
		#endif
		pAttacker->bInterruptDuelPts = NO_INTERRUPT;
		#ifdef DEBUG_INTERRUPTS
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Resetting int pts for %d in NUA", pAttacker->ubID ) );
		#endif

	}
}


void CheckForAlertWhenEnemyDies(SOLDIERTYPE* pDyingSoldier)
{
	UINT8						ubID;
	SOLDIERTYPE *		pSoldier;
	INT8						bDir;
	INT16						sDistAway, sDistVisible;

	for ( ubID = gTacticalStatus.Team[ pDyingSoldier->bTeam ].bFirstID; ubID <= gTacticalStatus.Team[ pDyingSoldier->bTeam ].bLastID; ubID++ )
	{

		pSoldier = MercPtrs[ ubID ];

		if ( pSoldier->bActive && pSoldier->bInSector && (pSoldier != pDyingSoldier) && (pSoldier->bLife >= OKLIFE) && (pSoldier->bAlertStatus < STATUS_RED ) )
		{
			// this guy might have seen the man die

			// distance we "see" then depends on the direction he is located from us
			bDir = atan8(pSoldier->sX,pSoldier->sY,pDyingSoldier->sX,pDyingSoldier->sY);
			sDistVisible = DistanceVisible( pSoldier, pSoldier->bDesiredDirection, bDir, pDyingSoldier->sGridNo, pDyingSoldier->bLevel );
			sDistAway = PythSpacesAway( pSoldier->sGridNo, pDyingSoldier->sGridNo );

			// if we see close enough to see the soldier
			if (sDistAway <= sDistVisible)
			{
				// and we can trace a line of sight to his x,y coordinates
				// assume enemies are always aware of their buddies...
				if ( SoldierTo3DLocationLineOfSightTest( pSoldier, pDyingSoldier->sGridNo, pDyingSoldier->bLevel, 0, (UINT8) sDistVisible, TRUE ) )
				{
					pSoldier->bAlertStatus = STATUS_RED;
					CheckForChangingOrders( pSoldier );
				}
			}
		}

	}

}


static BOOLEAN ArmyKnowsOfPlayersPresence(void)
{
	UINT8						ubID;
	SOLDIERTYPE *		pSoldier;

	// if anyone is still left...
	if (gTacticalStatus.Team[ ENEMY_TEAM ].bTeamActive && gTacticalStatus.Team[ ENEMY_TEAM ].bMenInSector > 0 )
	{
		for ( ubID = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; ubID <= gTacticalStatus.Team[ ENEMY_TEAM ].bLastID; ubID++ )
		{
			pSoldier = MercPtrs[ ubID ];

			if ( pSoldier->bActive && pSoldier->bInSector && (pSoldier->bLife >= OKLIFE) && (pSoldier->bAlertStatus >= STATUS_RED ) )
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );
}

BOOLEAN MercSeesCreature( SOLDIERTYPE * pSoldier )
{
	UINT8						ubID;

	if (pSoldier->bOppCnt > 0)
	{
		for ( ubID = gTacticalStatus.Team[ CREATURE_TEAM ].bFirstID; ubID <= gTacticalStatus.Team[ CREATURE_TEAM ].bLastID; ubID++ )
		{
			if ( (pSoldier->bOppList[ ubID ] == SEEN_CURRENTLY) && (MercPtrs[ ubID ]->uiStatusFlags & SOLDIER_MONSTER) )
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );
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
	INT8	bLoop;

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
	INT8	bLoc;

	bLoc = FindWatchedLoc( ubID, sGridNo, bLevel );
	if (bLoc != -1)
	{
		#ifdef JA2BETAVERSION
			/*
			if (gubWatchedLocPoints[ ubID ][ bLoc ] > 1)
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Soldier %d getting %d points for interrupt in watched location", ubID, gubWatchedLocPoints[ ubID ][ bLoc ] - 1 );
			}
			*/
		#endif
		// one loc point is worth nothing, so return number minus 1

		// experiment with 1 loc point being worth 1 point
		return( gubWatchedLocPoints[ ubID ][ bLoc ] );
	}

	return( 0 );
}


INT8 GetHighestVisibleWatchedLoc( UINT8 ubID )
{
	INT8	bLoop;
	INT8	bHighestLoc = -1;
	INT8	bHighestPoints = 0;
	INT16	sDistVisible;

	for ( bLoop = 0; bLoop < NUM_WATCHED_LOCS; bLoop++ )
	{
		if ( gsWatchedLoc[ ubID ][ bLoop ] != NOWHERE && gubWatchedLocPoints[ ubID ][ bLoop ] > bHighestPoints )
		{
			sDistVisible =  DistanceVisible( MercPtrs[ ubID ], DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, gsWatchedLoc[ ubID ][ bLoop ], gbWatchedLocLevel[ ubID ][ bLoop ] );
			// look at standing height
			if ( SoldierTo3DLocationLineOfSightTest( MercPtrs[ ubID ], gsWatchedLoc[ ubID ][ bLoop ], gbWatchedLocLevel[ ubID ][ bLoop ], 3, (UINT8) sDistVisible, TRUE ) )
			{
				bHighestLoc = bLoop;
				bHighestPoints = gubWatchedLocPoints[ ubID ][ bLoop ];
			}
		}
	}
	return( bHighestLoc );
}

INT8 GetHighestWatchedLocPoints( UINT8 ubID )
{
	INT8	bLoop;
	INT8	bHighestPoints = 0;

	for ( bLoop = 0; bLoop < NUM_WATCHED_LOCS; bLoop++ )
	{
		if ( gsWatchedLoc[ ubID ][ bLoop ] != NOWHERE && gubWatchedLocPoints[ ubID ][ bLoop ] > bHighestPoints )
		{
			bHighestPoints = gubWatchedLocPoints[ ubID ][ bLoop ];
		}
	}
	return( bHighestPoints );
}


static void CommunicateWatchedLoc(UINT8 ubID, INT16 sGridNo, INT8 bLevel, UINT8 ubPoints)
{
	UINT8		ubLoop;
	INT8		bTeam, bLoopPoint, bPoint;

	bTeam = MercPtrs[ ubID ]->bTeam;

	for ( ubLoop = gTacticalStatus.Team[ bTeam ].bFirstID; ubLoop < gTacticalStatus.Team[ bTeam ].bLastID; ubLoop++ )
	{
		if (ubLoop == ubID) continue;
		const SOLDIERTYPE* const s = GetMan(ubLoop);
		if (s->bActive == FALSE || s->bInSector == FALSE || s->bLife < OKLIFE)
		{
			continue;
		}
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
			gubWatchedLocPoints[ ubLoop ][ bLoopPoint ] = __max( gubWatchedLocPoints[ ubLoop ][ bLoopPoint ], ubPoints );

			gfWatchedLocReset[ ubLoop ][ bLoopPoint ] = FALSE;
			gfWatchedLocHasBeenIncremented[ ubLoop ][ bLoopPoint ] = TRUE;
		}
	}
}


static void IncrementWatchedLoc(UINT8 ubID, INT16 sGridNo, INT8 bLevel)
{
	INT8	bPoint;

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

			CommunicateWatchedLoc( ubID, sGridNo, bLevel, 1 );
		}
		// otherwise abort; no points available
	}
	else
	{
		if ( !gfWatchedLocHasBeenIncremented[ ubID ][ bPoint ] && gubWatchedLocPoints[ ubID ][ bPoint ] < MAX_WATCHED_LOC_POINTS )
		{
			gubWatchedLocPoints[ ubID ][ bPoint ]++;
			CommunicateWatchedLoc( ubID, sGridNo, bLevel, gubWatchedLocPoints[ ubID ][ bPoint ] );
		}
		gfWatchedLocReset[ ubID ][ bPoint ] = FALSE;
		gfWatchedLocHasBeenIncremented[ ubID ][ bPoint ] = TRUE;
	}
}


static void SetWatchedLocAsUsed(UINT8 ubID, INT16 sGridNo, INT8 bLevel)
{
	INT8	bPoint;

	bPoint = FindWatchedLoc( ubID, sGridNo, bLevel );
	if (bPoint != -1)
	{
		gfWatchedLocReset[ ubID ][ bPoint ] = FALSE;
	}
}


static BOOLEAN WatchedLocLocationIsEmpty(INT16 sGridNo, INT8 bLevel, INT8 bTeam)
{
	// look to see if there is anyone near the watched loc who is not on this team
	UINT8	ubID;
	INT16	sTempGridNo, sX, sY;

	for ( sY = -WATCHED_LOC_RADIUS; sY <= WATCHED_LOC_RADIUS; sY++ )
	{
		for ( sX = -WATCHED_LOC_RADIUS; sX <= WATCHED_LOC_RADIUS; sX++ )
		{
			sTempGridNo = sGridNo + sX + sY * WORLD_ROWS;
			if ( sTempGridNo < 0 || sTempGridNo >= WORLD_MAX )
			{
				continue;
			}
			ubID = WhoIsThere2( sTempGridNo, bLevel );
			if ( ubID != NOBODY && MercPtrs[ ubID ]->bTeam != bTeam )
			{
				return( FALSE );
			}
		}
	}
	return( TRUE );
}


static void DecayWatchedLocs(INT8 bTeam)
{
	UINT8	cnt, cnt2;

	// loop through all soldiers
	for ( cnt = gTacticalStatus.Team[ bTeam ].bFirstID; cnt <= gTacticalStatus.Team[ bTeam ].bLastID; cnt++ )
	{
		// for each watched location
		for ( cnt2 = 0; cnt2 < NUM_WATCHED_LOCS; cnt2++ )
		{
			if ( gsWatchedLoc[ cnt ][ cnt2 ] != NOWHERE && WatchedLocLocationIsEmpty( gsWatchedLoc[ cnt ][ cnt2 ], gbWatchedLocLevel[ cnt ][ cnt2 ], bTeam ) )
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
	INT32						iLoop;
	SOLDIERTYPE *		pSoldier;

	iLoop = gTacticalStatus.Team[ CREATURE_TEAM ].bFirstID;

	for (pSoldier = MercPtrs[iLoop]; iLoop <= gTacticalStatus.Team[ CREATURE_TEAM ].bLastID; iLoop++,pSoldier++ )
  {
		if ( pSoldier->ubBodyType == BLOODCAT && pSoldier->bActive && pSoldier->bInSector && pSoldier->bLife > 0 )
		{
		  SetSoldierNonNeutral( pSoldier );
		  RecalculateOppCntsDueToNoLongerNeutral( pSoldier );
		  if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
		  {
			  CheckForPotentialAddToBattleIncrement( pSoldier );
		  }
		}
	}

}
