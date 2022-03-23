#include "Font_Control.h"
#include "LoadSaveData.h"
#include "MapScreen.h"
#include "Strategic_Town_Loyalty.h"
#include "StrategicMap.h"
#include "Overhead.h"
#include "Queen_Command.h"
#include "Animation_Data.h"
#include "Quests.h"
#include "Message.h"
#include "LOS.h"
#include "World_Items.h"
#include "Tactical_Save.h"
#include "Soldier_Profile.h"
#include "Handle_Items.h"
#include "Random.h"
#include "Strategic_Movement.h"
#include "Strategic_Pathing.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "Morale.h"
#include "Text.h"
#include "MessageBoxScreen.h"
#include "Creature_Spreading.h"
#include "Town_Militia.h"
#include "History.h"
#include "Meanwhile.h"
#include "GameSettings.h"
#include "Strategic_Status.h"
#include "MemMan.h"
#include "Debug.h"
#include "FileMan.h"
#include "Logger.h"

#include <string_theory/string>

#include <algorithm>
#include <iterator>
#include <vector>

// the max loyalty rating for any given town
#define MAX_LOYALTY_VALUE 100

// loyalty Omerta drops to and maxes out at if the player betrays the rebels
#define HOSTILE_OMERTA_LOYALTY_RATING 10

#define LOYALTY_EVENT_DISTANCE_THRESHOLD 3 // in sectors


// effect of unintentional killing /100 vs intentional murder
#define REDUCTION_FOR_UNINTENTIONAL_KILLING 50
// the effect of a murder by rebel / 100 vs player murdering a civ
#define REDUCTION_FOR_MURDER_BY_REBEL 70
// reduction % for being falsely blamed for a murder we didn't do, out of 100
#define REDUCTION_FOR_MURDER_NOT_OUR_FAULT 50
// reduction % if enemies kills civs in our sector / 100
#define REDUCTION_FOR_MURDER_OF_INNOCENT_BY_ENEMY_IN_OUR_SECTOR 25
// how much worse loyalty hit is for trauma of someone killed by a monster
#define MULTIPLIER_FOR_MURDER_BY_MONSTER 2



// gain for hiring an NPC from a particular town (represents max. at 100% town attachment)
#define MULTIPLIER_LOCAL_RPC_HIRED 25 // 5%

// multiplier for causing pts of damage directly done to a building
#define MULTIPLIER_FOR_DAMAGING_A_BUILDING 10 // 50 pts = 1%
// multiplier for not preventing pts of damage to a building
#define MULTIPLIER_FOR_NOT_PREVENTING_BUILDING_DAMAGE 3 // 167 pts = 1%

// divisor for dmg to a building by allied rebel
#define DIVISOR_FOR_REBEL_BUILDING_DMG 2


// town loyalty table
TOWN_LOYALTY gTownLoyalty[ NUM_TOWNS ];


// Town names and locations
std::vector<TownSectorInfo> g_town_sectors;


#define BASIC_COST_FOR_CIV_MURDER	(10 * GAIN_PTS_PER_LOYALTY_PT)

UINT32 uiPercentLoyaltyDecreaseForCivMurder[]={
	// These get multiplied by GAIN_PTS_PER_LOYALTY_PT so they're in % of loyalty decrease (for an average town)
	(5 * GAIN_PTS_PER_LOYALTY_PT), // fat civ
	(7 * GAIN_PTS_PER_LOYALTY_PT), // man civ
	(8 * GAIN_PTS_PER_LOYALTY_PT), // min civ
	(10 * GAIN_PTS_PER_LOYALTY_PT), // dress (woman)
	(20 * GAIN_PTS_PER_LOYALTY_PT), // hat kid
	(20 * GAIN_PTS_PER_LOYALTY_PT), // kid
	(20 * GAIN_PTS_PER_LOYALTY_PT), // cripple
	(1 * GAIN_PTS_PER_LOYALTY_PT), // cow
};


// on a scale of 1-100, this is a measure of how much each town hates the Queen's opression & is willing to stand against it
// it primarily controls the RATE of loyalty change in each town: the loyalty effect of the same events depends on it
UINT8 gubTownRebelSentiment[ NUM_TOWNS ] =
{
	0, // not a town - blank sector index
	90, // OMERTA, - They ARE the rebels!!!
	30, // DRASSEN, - Rebel friendly, makes it pretty easy to get first mine's income going at the start
	12, // ALMA - Military town, high loyalty to Queen, need quests to get 100%
	15, // GRUMM, - Close to Meduna, strong influence
	20, // TIXA, - Not a real town
	15, // CAMBRIA, - Artificially much lower 'cause it's big and central and too easy to get loyalty up there
	20, // SAN_MONA, - Neutral ground, loyalty doesn't vary
	20, // ESTONI, - Not a real town
	20, // ORTA, - Not a real town
	12, // BALIME, - Rich town, high loyalty to Queen
	10, // MEDUNA, - Enemy HQ, for God's sake!
	35, // CHITZENA, - Artificially high 'cause there's not enough fights near it to get the loyalty up otherwise
};

BOOLEAN gfTownUsesLoyalty[ NUM_TOWNS ] =
{
	FALSE, // not a town - blank sector index
	TRUE, // OMERTA
	TRUE, // DRASSEN
	TRUE, // ALMA
	TRUE, // GRUMM
	FALSE, // TIXA
	TRUE, // CAMBRIA
	FALSE, // SAN_MONA
	FALSE, // ESTONI
	FALSE, // ORTA
	TRUE, // BALIME
	TRUE, // MEDUNA
	TRUE, // CHITZENA
};

// location of first enocunter with enemy
INT16 sWorldSectorLocationOfFirstBattle = 0;


void InitTownLoyalty( void )
{
	UINT8 ubTown = 0;

	// set up town loyalty table
	for( ubTown = FIRST_TOWN; ubTown < NUM_TOWNS; ubTown++ )
	{
		gTownLoyalty[ ubTown ].ubRating = 0;
		gTownLoyalty[ ubTown ].sChange = 0;
		gTownLoyalty[ ubTown ].fStarted = FALSE;
		gTownLoyalty[ ubTown ].fLiberatedAlready = FALSE;
	}
}


void StartTownLoyaltyIfFirstTime( INT8 bTownId )
{
	Assert( ( bTownId >= FIRST_TOWN ) && ( bTownId < NUM_TOWNS ) );

	// if loyalty tracking hasn't yet been started for this town, and the town does use loyalty
	if (!gTownLoyalty[ bTownId ].fStarted && gfTownUsesLoyalty[ bTownId ])
	{
		// set starting town loyalty now, equally to that town's current rebel sentiment - not all towns begin equally loyal
		gTownLoyalty[ bTownId ].ubRating = gubTownRebelSentiment[ bTownId ];

		// if player hasn't made contact with Miguel yet, or the rebels hate the player
		if (!CheckFact(FACT_MIGUEL_READ_LETTER, 0) || CheckFact( FACT_REBELS_HATE_PLAYER, 0 ))
		{
			// if town is Omerta
			if (bTownId == OMERTA)
			{
				// start loyalty there at 0, since rebels distrust the player until Miguel receives the letter
				gTownLoyalty[ bTownId ].ubRating  = 0;
			}
			else
			{
				// starting loyalty is halved - locals not sure what to make of the player's presence
				gTownLoyalty[ bTownId ].ubRating /= 2;
			}
		}

		gTownLoyalty[ bTownId ].sChange = 0;

		// remember we've started
		gTownLoyalty[ bTownId ].fStarted = TRUE;
	}
}


// set a specified town's loyalty rating (ignores previous loyalty value - probably NOT what you want)
void SetTownLoyalty( INT8 bTownId, UINT8 ubNewLoyaltyRating )
{
	Assert( ( bTownId >= FIRST_TOWN ) && ( bTownId < NUM_TOWNS ) );

	// if the town does use loyalty
	if (gfTownUsesLoyalty[ bTownId ])
	{
		gTownLoyalty[ bTownId ].ubRating = ubNewLoyaltyRating;
		gTownLoyalty[ bTownId ].sChange = 0;

		// this is just like starting the loyalty if it happens first
		gTownLoyalty[ bTownId ].fStarted = TRUE;
	}
}


static void UpdateTownLoyaltyRating(INT8 bTownId);


// increments the town's loyalty rating by that many HUNDREDTHS of loyalty pts
void IncrementTownLoyalty( INT8 bTownId, UINT32 uiLoyaltyIncrease )
{
	UINT32 uiRemainingIncrement;
	INT16 sThisIncrement;


	Assert( ( bTownId >= FIRST_TOWN ) && ( bTownId < NUM_TOWNS ) );

	// doesn't affect towns where player hasn't established a "presence" yet
	if (!gTownLoyalty[ bTownId ].fStarted)
	{
		return;
	}

	// modify loyalty change by town's individual attitude toward rebelling (20 is typical)
	uiLoyaltyIncrease *= (5 * gubTownRebelSentiment[ bTownId ]);
	uiLoyaltyIncrease /= 100;


	// this whole thing is a hack to avoid rolling over the -32 to 32k range on the sChange value
	// only do a maximum of 10000 pts at a time...
	uiRemainingIncrement = uiLoyaltyIncrease;
	while ( uiRemainingIncrement )
	{
		sThisIncrement = ( INT16 ) MIN( uiRemainingIncrement, 10000 );

		// up the gain value
		gTownLoyalty[ bTownId ].sChange += (INT16) sThisIncrement;
		// update town value now
		UpdateTownLoyaltyRating( bTownId );

		uiRemainingIncrement -= sThisIncrement;
	}
}

// decrements the town's loyalty rating by that many HUNDREDTHS of loyalty pts
//NOTE: This function expects a POSITIVE number for a decrease!!!
void DecrementTownLoyalty( INT8 bTownId, UINT32 uiLoyaltyDecrease )
{
	UINT32 uiRemainingDecrement;
	INT16 sThisDecrement;


	Assert( ( bTownId >= FIRST_TOWN ) && ( bTownId < NUM_TOWNS ) );

	// doesn't affect towns where player hasn't established a "presence" yet
	if (!gTownLoyalty[ bTownId ].fStarted)
	{
		return;
	}

	// modify loyalty change by town's individual attitude toward rebelling (20 is typical)
	uiLoyaltyDecrease *= 100;
	uiLoyaltyDecrease /= (5 * gubTownRebelSentiment[ bTownId ]);

	// this whole thing is a hack to avoid rolling over the -32 to 32k range on the sChange value
	// only do a maximum of 10000 pts at a time...
	uiRemainingDecrement = uiLoyaltyDecrease;
	while ( uiRemainingDecrement )
	{
		sThisDecrement = ( INT16 ) MIN( uiRemainingDecrement, 10000 );

		// down the gain value
		gTownLoyalty[ bTownId ].sChange -= sThisDecrement;
		// update town value now
		UpdateTownLoyaltyRating( bTownId );

		uiRemainingDecrement -= sThisDecrement;
	}
}


// update town loyalty rating based on gain values
static void UpdateTownLoyaltyRating(INT8 bTownId)
{
	// check gain value and update loyaty
	UINT8 ubOldLoyaltyRating = 0;
	INT16 sRatingChange = 0;
	UINT8 ubMaxLoyalty = 0;


	Assert( ( bTownId >= FIRST_TOWN ) && ( bTownId < NUM_TOWNS ) );

	// remember previous loyalty value
	ubOldLoyaltyRating = gTownLoyalty[ bTownId ].ubRating;

	sRatingChange = gTownLoyalty[ bTownId ].sChange / GAIN_PTS_PER_LOYALTY_PT;

	// if loyalty is ready to increase
	if( sRatingChange > 0 )
	{
		// if the town is Omerta, and the rebels are/will become hostile
		if ((bTownId == OMERTA) && (gTacticalStatus.fCivGroupHostile[ REBEL_CIV_GROUP ] != CIV_GROUP_NEUTRAL))
		{
			// maximum loyalty is much less than normal
			ubMaxLoyalty = HOSTILE_OMERTA_LOYALTY_RATING;
		}
		else
		{
			ubMaxLoyalty = MAX_LOYALTY_VALUE;
		}

		// check if we'd be going over the max
		if( (gTownLoyalty[ bTownId ].ubRating + sRatingChange ) >= ubMaxLoyalty )
		{
			// set to max and null out gain pts
			gTownLoyalty[ bTownId ].ubRating = ubMaxLoyalty;
			gTownLoyalty[ bTownId ].sChange = 0;
		}
		else
		{
			// increment loyalty rating, reduce sChange
			gTownLoyalty[ bTownId ].ubRating += sRatingChange;
			gTownLoyalty[ bTownId ].sChange %= GAIN_PTS_PER_LOYALTY_PT;
		}
	}
	else
	// if loyalty is ready to decrease
	if( sRatingChange < 0 )
	{
		// check if we'd be going below zero
		if( ( gTownLoyalty[ bTownId ].ubRating + sRatingChange ) < 0 )
		{
			// set to zero and null out gain pts
			gTownLoyalty[ bTownId ].ubRating = 0;
			gTownLoyalty[ bTownId ].sChange = 0;
		}
		else
		{
			// decrement loyalty rating, reduce sChange
			gTownLoyalty[ bTownId ].ubRating += sRatingChange;
			gTownLoyalty[ bTownId ].sChange %= GAIN_PTS_PER_LOYALTY_PT;
		}
	}


	// check old aginst new, if diff, dirty map panel
	if( ubOldLoyaltyRating != gTownLoyalty[ bTownId ].ubRating )
	{
		fMapPanelDirty = TRUE;
	}
}


static void AffectAllTownsLoyaltyByDistanceFrom(INT32 iLoyaltyChange, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);


void HandleMurderOfCivilian(const SOLDIERTYPE* const pSoldier)
{
	// handle the impact on loyalty of the murder of a civilian
	INT32 iLoyaltyChange = 0;
	INT8 bSeenState = 0;
	UINT32 uiChanceFalseAccusal = 0;
	INT8 bKillerTeam = 0;
	BOOLEAN fIncrement = FALSE;

	// attacker CAN be NOBODY...  Don't treat is as murder if NOBODY killed us...
	SOLDIERTYPE* const killer = pSoldier->attacker;
	if (killer == NULL) return;

	// ignore murder of non-civilians!
	if ( ( pSoldier->bTeam != CIV_TEAM ) || ( pSoldier->ubBodyType == CROW ) )
	{
		return;
	}

	// if this is a profiled civilian NPC
	if ( pSoldier->ubProfile != NO_PROFILE )
	{
		// ignore murder of NPCs if they're not loyal to the rebel cause - they're really just enemies in civilian clothing
		if ( gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH )
		{
			return;
		}
	}

	// if civilian belongs to a civilian group
	if ( pSoldier->ubCivilianGroup != NON_CIV_GROUP )
	{
		// and it's one that is hostile to the player's cause
		switch ( pSoldier->ubCivilianGroup )
		{
			case KINGPIN_CIV_GROUP:
			case ALMA_MILITARY_CIV_GROUP:
			case HICKS_CIV_GROUP:
			case WARDEN_CIV_GROUP:
				return;
		}
	}

	// set killer team
	bKillerTeam = killer->bTeam;

	// if the player did the killing
	if( bKillerTeam == OUR_TEAM )
	{
		// apply morale penalty for killing a civilian!
		HandleMoraleEvent(killer, MORALE_KILLED_CIVILIAN, killer->sSectorX, killer->sSectorY, killer->bSectorZ);
	}

	UINT8 const bTownId = GetTownIdForSector(SECTOR(pSoldier->sSectorX, pSoldier->sSectorY));

	// if civilian is NOT in a town
	if( bTownId == BLANK_SECTOR )
	{
		return;
	}

	// check if this town does use loyalty (to skip a lot of unnecessary computation)
	if (!gfTownUsesLoyalty[ bTownId ])
	{
		return;
	}


	if( ( pSoldier->ubBodyType >= FATCIV) && ( pSoldier->ubBodyType <= COW ) )
	{
		// adjust value for killer and type
		iLoyaltyChange = uiPercentLoyaltyDecreaseForCivMurder[ pSoldier->ubBodyType - FATCIV ];
	}
	else
	{
		iLoyaltyChange = BASIC_COST_FOR_CIV_MURDER;
	}

	if (!pSoldier->fIntendedTarget)
	{
		// accidental killing, reduce value
		iLoyaltyChange *= REDUCTION_FOR_UNINTENTIONAL_KILLING;
		iLoyaltyChange /= 100;
	}

	// check if LOS between any civ, killer and killed
	// if so, then do not adjust

	FOR_EACH_IN_TEAM(pCivSoldier, CIV_TEAM)
	{
		if ( pCivSoldier == pSoldier )
		{
			continue;
		}

		// killer seen by civ?
		if (SoldierToSoldierLineOfSightTest(pCivSoldier, killer, STRAIGHT_RANGE, TRUE) != 0)
		{
			bSeenState |= 1;
		}

		// victim seen by civ?
		if( SoldierToSoldierLineOfSightTest( pCivSoldier, pSoldier, STRAIGHT_RANGE, TRUE ) != 0 )
		{
			bSeenState |= 2;
		}
	}

	// if player didn't do it
	if( bKillerTeam != OUR_TEAM )
	{
		// If the murder is not fully witnessed, there's a chance of player being blamed for it even if it's not his fault
		switch (bSeenState)
		{
			case 0:
				// nobody saw anything.  When body is found, chance player is blamed depends on the town's loyalty at this time
				uiChanceFalseAccusal = MAX_LOYALTY_VALUE - gTownLoyalty[ bTownId ].ubRating;
				break;
			case 1:
				// civilians saw the killer, but not the victim. 10 % chance of blaming player whether or not he did it
				uiChanceFalseAccusal = 10;
				break;
			case 2:
				// civilians only saw the victim.  50/50 chance...
				uiChanceFalseAccusal = 50;
				break;
			case 3:
				// civilians have seen it all, and in this case, they're always honest
				uiChanceFalseAccusal = 0;
				break;
			default:
				Assert(FALSE);
				return;
		}

		// check whether player is falsely accused
		if( Random(100) < uiChanceFalseAccusal )
		{
			// blame player whether or not he did it - set killer team as our team
			bKillerTeam = OUR_TEAM;

			// not really player's fault, reduce penalty, because some will believe it to be a lie
			iLoyaltyChange *= REDUCTION_FOR_MURDER_NOT_OUR_FAULT;
			iLoyaltyChange /= 100;

			// debug message
			SLOGD("You're being blamed for a death you didn't cause!");
		}
	}


	// check who town thinks killed the civ
	switch (bKillerTeam)
	{
		case OUR_TEAM:
			// town thinks player committed the murder, bad bad bad
			fIncrement = FALSE;

			// debug message
			SLOGD("Civilian killed by friendly forces.");
			break;

		case ENEMY_TEAM:
			// check whose sector this is
			if (StrategicMap[pSoldier->sSectorX + MAP_WORLD_X * pSoldier->sSectorY].fEnemyControlled)
			{
				// enemy soldiers... in enemy controlled sector.  Gain loyalty
				fIncrement = TRUE;

				// debug message
				SLOGD("Enemy soldiers murdered a civilian. Town loyalty increases");
			}
			else
			{
				// reduce, we're expected to provide some protection, but not miracles
				iLoyaltyChange *= REDUCTION_FOR_MURDER_OF_INNOCENT_BY_ENEMY_IN_OUR_SECTOR;
				iLoyaltyChange /= 100;

				// lose loyalty
				fIncrement = FALSE;

				// debug message
				SLOGD("Town holds you responsible for murder by enemy.");
			}
			break;

		case MILITIA_TEAM:
			// the rebels did it... check if are they on our side
			if (!CheckFact(FACT_REBELS_HATE_PLAYER, 0))
			{
				// on our side, penalty
				iLoyaltyChange *= REDUCTION_FOR_MURDER_BY_REBEL;
				iLoyaltyChange /= 100;

				// lose loyalty
				fIncrement = FALSE;

				// debug message
				SLOGD("Town holds you responsible for murder by rebels.");
			}
			break;

	case CREATURE_TEAM:
			// killed by a monster - make sure it was one
			if (ADULTFEMALEMONSTER <= killer->ubBodyType && killer->ubBodyType <= QUEENMONSTER)
			{
				// increase for the extreme horror of being killed by a monster
				iLoyaltyChange *= MULTIPLIER_FOR_MURDER_BY_MONSTER;

				// check whose sector this is
				if (StrategicMap[pSoldier->sSectorX + MAP_WORLD_X * pSoldier->sSectorY].fEnemyControlled)
				{
					// enemy controlled sector - gain loyalty
					fIncrement = TRUE;
				}
				else
				{
					// our sector - lose loyalty
					fIncrement = FALSE;
				}
			}
			break;

		default:
			Assert(FALSE);
			return;
	}


	// if it's a decrement, negate the value
	if ( !fIncrement )
	{
		iLoyaltyChange *= -1;
	}

	// this is a hack: to avoid having to adjust the values, divide by 1.75 to compensate for the distance 0
	iLoyaltyChange *= 100;
	iLoyaltyChange /= (100 + ( 25 * LOYALTY_EVENT_DISTANCE_THRESHOLD ) );

	AffectAllTownsLoyaltyByDistanceFrom( iLoyaltyChange, pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );
}



// check town and raise loyalty value for hiring a merc from a town...not a lot of a gain, but some
void HandleTownLoyaltyForNPCRecruitment( SOLDIERTYPE *pSoldier )
{
	UINT32 uiLoyaltyValue = 0;

	UINT8 const bTownId = GetTownIdForSector(SECTOR(pSoldier->sSectorX, pSoldier->sSectorY));

	// is the merc currently in their home town?
	if( bTownId == gMercProfiles[ pSoldier->ubProfile ].bTown )
	{
		// yep, value of loyalty bonus depends on his importance to this to town
		uiLoyaltyValue = MULTIPLIER_LOCAL_RPC_HIRED * gMercProfiles[ pSoldier->ubProfile ].bTownAttachment;

		// increment town loyalty gain
		IncrementTownLoyalty( bTownId, uiLoyaltyValue );

		// DESIGN QUESTION: How easy is it to abuse this bonus by repeatedly hiring the guy over & over
		// (at worst daily? even more often if terminated & rehired?)  (ARM)
	}
}

void RemoveRandomItemsInSector(INT16 const sSectorX, INT16 const sSectorY, INT16 const sSectorZ, UINT8 const ubChance)
{
	/* Stealing should fail anyway 'cause there shouldn't be a temp file for
	 * unvisited sectors, but let's check anyway */
	Assert(GetSectorFlagStatus(sSectorX, sSectorY, sSectorZ, SF_ALREADY_VISITED));

	ST::string wSectorName = GetSectorIDString(sSectorX, sSectorY, sSectorZ, TRUE);

	// go through list of items in sector and randomly remove them

	// if unloaded sector
	if (gWorldSectorX  != sSectorX ||
			gWorldSectorY  != sSectorY ||
			gbWorldSectorZ != sSectorZ)
	{
		/* if the player has never been there, there's no temp file, and 0 items
		 * will get returned, preventing any stealing */
		std::vector<WORLDITEM> pItemList = LoadWorldItemsFromTempItemFile(sSectorX, sSectorY, sSectorZ);
		if (pItemList.size() == 0) return;

		bool somethingWasStolen = false;

		// set up item list ptrs
		for (WORLDITEM& wi : pItemList)
		{
			//if the item exists, and is visible and reachable, see if it should be stolen
			if (!wi.fExists)                          continue;
			if (wi.bVisible != VISIBLE)               continue;
			if (!(wi.usFlags & WORLD_ITEM_REACHABLE)) continue;
			if (Random(100) >= ubChance)               continue;

			// remove
			somethingWasStolen = true;
			wi.fExists = FALSE;

			SLOGD("%s stolen in %s!", ItemNames[wi.o.usItem].c_str(), wSectorName.c_str());
		}

		// only save if something was stolen
		if (somethingWasStolen)
		{
			SaveWorldItemsToTempItemFile(sSectorX, sSectorY, sSectorZ, pItemList);
		}
	}
	else	// handle a loaded sector
	{
		FOR_EACH_WORLD_ITEM(wi)
		{
			// note, can't do reachable test here because we'd have to do a path call
			if (wi.bVisible != VISIBLE) continue;
			if (Random(100) >= ubChance) continue;

			SLOGD("%s stolen in %s!", ItemNames[wi.o.usItem].c_str(), wSectorName.c_str());
			RemoveItemFromPool(wi);
		}
	}
}


void BuildListOfTownSectors()
{
	for (INT32 x = 1; x != MAP_WORLD_X - 1; ++x)
	{
		for (INT32 y = 1; y != MAP_WORLD_Y - 1; ++y)
		{
			INT8 const town = StrategicMap[CALCULATE_STRATEGIC_INDEX(x, y)].bNameId;
			if (town < FIRST_TOWN || NUM_TOWNS <= town) continue;
			g_town_sectors.push_back(
				TownSectorInfo{ (UINT8)town, SECTOR(x, y) }
			);
		}
	}
}


void SaveStrategicTownLoyaltyToSaveGameFile(HWFILE const f)
{
	//Save the Town Loyalty
	FOR_EACH(TOWN_LOYALTY const, i, gTownLoyalty)
	{
		BYTE  data[26];
		DataWriter d{data};
		INJ_U8(  d, i->ubRating)
		INJ_SKIP(d, 1)
		INJ_I16( d, i->sChange)
		INJ_BOOL(d, i->fStarted)
		INJ_SKIP(d, 1)
		INJ_BOOL(d, i->fLiberatedAlready)
		INJ_SKIP(d, 19)
		Assert(d.getConsumed() == lengthof(data));

		f->write(data, sizeof(data));
	}
}


void LoadStrategicTownLoyaltyFromSavedGameFile(HWFILE const f)
{
	//Restore the Town Loyalty
	FOR_EACH(TOWN_LOYALTY, i, gTownLoyalty)
	{
		BYTE data[26];
		f->read(data, sizeof(data));

		DataReader d{data};
		EXTR_U8(  d, i->ubRating)
		EXTR_SKIP(d, 1)
		EXTR_I16( d, i->sChange)
		EXTR_BOOL(d, i->fStarted)
		EXTR_SKIP(d, 1)
		EXTR_BOOL(d, i->fLiberatedAlready)
		EXTR_SKIP(d, 19)
		Assert(d.getConsumed() == lengthof(data));
	}
}


void ReduceLoyaltyForRebelsBetrayed(void)
{
	INT8 bTownId;

	// reduce loyalty to player all across Arulco
	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		if (bTownId == OMERTA)
		{
			// if not already really low
			if (gTownLoyalty[ bTownId ].ubRating > HOSTILE_OMERTA_LOYALTY_RATING)
			{
				// loyalty in Omerta tanks big time, and will stay low permanently since this becomes its maximum
				SetTownLoyalty(bTownId, HOSTILE_OMERTA_LOYALTY_RATING);
				// note that rebel sentiment isn't affected - they remain loyal to themselves, after all!
			}
		}
		else
		{

			// loyalty in other places is also strongly affected by this falling out with rebels, but this is not permanent
			SetTownLoyalty(bTownId, (UINT8) (gTownLoyalty[ bTownId ].ubRating / 3));
		}
	}
}

INT32 GetNumberOfWholeTownsUnderControl( void )
{
	INT32 iNumber = 0;
	INT8 bTownId = 0;

	// run through the list of towns..if the entire town is under player control, then increment the number of towns under player control

	// make sure that each town is one for which loyalty matters
	for ( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		if ( IsTownUnderCompleteControlByPlayer( bTownId ) && gfTownUsesLoyalty[ bTownId ] )
		{
			iNumber++;
		}
	}

	return( iNumber );
}


INT32 GetNumberOfWholeTownsUnderControlButExcludeCity( INT8 bCityToExclude )
{
	INT32 iNumber = 0;
	INT8 bTownId = 0;

	// run through the list of towns..if the entire town is under player control, then increment the number of towns under player control
	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		if ( IsTownUnderCompleteControlByPlayer( bTownId ) && ( bCityToExclude != bTownId ) && gfTownUsesLoyalty[ bTownId ] )
		{
			iNumber++;
		}
	}

	return( iNumber );
}

// is the ENTIRE town under player control?
INT32 IsTownUnderCompleteControlByPlayer( INT8 bTownId )
{
	return GetTownSectorSize(bTownId) == GetTownSectorsUnderControl(bTownId);
}


// is the ENTIRE town under enemy control?
static INT32 IsTownUnderCompleteControlByEnemy(INT8 bTownId)
{
	return GetTownSectorsUnderControl(bTownId) == 0;
}

void AdjustLoyaltyForCivsEatenByMonsters( INT16 sSectorX, INT16 sSectorY, UINT8 ubHowMany)
{
	UINT32 uiLoyaltyChange = 0;
	ST::string str;
	ST::string pSectorString;

	UINT8 const bTownId = GetTownIdForSector(SECTOR(sSectorX, sSectorY));

	// if NOT in a town
	if( bTownId == BLANK_SECTOR )
	{
		return;
	}

	//Report this to player
	pSectorString = GetSectorIDString(sSectorX, sSectorY, 0, TRUE);
	str = st_format_printf(gpStrategicString[ STR_DIALOG_CREATURES_KILL_CIVILIANS ], ubHowMany, pSectorString);
	DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );

	// use same formula as if it were a civilian "murder" in tactical!!!
	uiLoyaltyChange = ubHowMany * BASIC_COST_FOR_CIV_MURDER * MULTIPLIER_FOR_MURDER_BY_MONSTER;
	DecrementTownLoyalty( bTownId, uiLoyaltyChange );
}


// this applies the SAME change to every town equally, regardless of distance from the event
void IncrementTownLoyaltyEverywhere( UINT32 uiLoyaltyIncrease )
{
	INT8 bTownId;

	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		IncrementTownLoyalty( bTownId, uiLoyaltyIncrease );
	}
}

void DecrementTownLoyaltyEverywhere( UINT32 uiLoyaltyDecrease )
{
	INT8 bTownId;

	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		DecrementTownLoyalty( bTownId, uiLoyaltyDecrease );
	}
}
// this applies the change to every town differently, depending on the distance from the event
void HandleGlobalLoyaltyEvent( UINT8 ubEventType, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ)
{
	INT32 iLoyaltyChange;
	INT8 bTownId = 0;

	if( bSectorZ == 0 )
	{
		// grab town id, if this event occured within one
		bTownId = GetTownIdForSector(SECTOR(sSectorX, sSectorY));
	}

	// should other towns ignore events occuring in this town?
	if( bTownId == SAN_MONA )
	{
		return;
	}

	// determine what the base loyalty change of this event type is worth
	// these are ->hundredths<- of loyalty points, so choose appropriate values accordingly!
	// the closer a town is to the event, the more pronounced the effect upon that town is
	switch (ubEventType)
	{
		case GLOBAL_LOYALTY_BATTLE_WON:
			CheckConditionsForTriggeringCreatureQuest( sSectorX, sSectorY, bSectorZ );
			iLoyaltyChange = 500;
			break;
		case GLOBAL_LOYALTY_QUEEN_BATTLE_WON:
			CheckConditionsForTriggeringCreatureQuest( sSectorX, sSectorY, bSectorZ );
			iLoyaltyChange = 1000;
			break;
		case GLOBAL_LOYALTY_BATTLE_LOST:
			iLoyaltyChange = -750;
			break;
		case GLOBAL_LOYALTY_ENEMY_KILLED:
			iLoyaltyChange = 50;
			break;
		case GLOBAL_LOYALTY_NATIVE_KILLED:
			// note that there is special code (much more severe) for murdering civilians in the currently loaded sector.
			// this event is intended more for processing militia casualties, and the like
			iLoyaltyChange = -50;
			break;
		case GLOBAL_LOYALTY_ABANDON_MILITIA:
			// it doesn't matter how many of them are being abandoned
			iLoyaltyChange = -750;
			break;
		case GLOBAL_LOYALTY_GAIN_TOWN_SECTOR:
			iLoyaltyChange = 500;
			break;
		case GLOBAL_LOYALTY_LOSE_TOWN_SECTOR:
			iLoyaltyChange = -1000;
			break;
		case GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN:
			iLoyaltyChange = 1000;
			break;
		case GLOBAL_LOYALTY_GAIN_MINE:
			iLoyaltyChange = 1000;
			break;
		case GLOBAL_LOYALTY_LOSE_MINE:
			iLoyaltyChange = -1500;
			break;
		case GLOBAL_LOYALTY_GAIN_SAM:
			iLoyaltyChange = 250;
			break;
		case GLOBAL_LOYALTY_LOSE_SAM:
			iLoyaltyChange = -250;
			break;

		default:
			Assert(FALSE);
			return;
	}

	AffectAllTownsLoyaltyByDistanceFrom( iLoyaltyChange, sSectorX, sSectorY, bSectorZ);
}


static void AffectAllTownsLoyaltyByDistanceFrom(INT32 iLoyaltyChange, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ)
{
	INT16 sEventSector;
	INT8 bTownId;
	INT32 iThisDistance;
	INT32 iShortestDistance[NUM_TOWNS];
	INT32 iPercentAdjustment;
	INT32 iDistanceAdjustedLoyalty;


	// preset shortest distances to high values prior to searching for a minimum
	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		iShortestDistance[ bTownId ] = 999999;
	}

	sEventSector = sSectorX + ( MAP_WORLD_X * sSectorY );

	// need a temporary group create to use for laying down distance paths
	GROUP& g = *CreateNewPlayerGroupDepartingFromSector(sSectorX, sSectorY);

	// calc distance to the event sector from EACH SECTOR of each town, keeping only the shortest one for every town
	FOR_EACH_TOWN_SECTOR(i)
	{
		bTownId = i->town;

		// skip path test if distance is already known to be zero to speed this up a bit
		if (iShortestDistance[ bTownId ] > 0 )
		{
			// calculate across how many sectors the fastest travel path from event to this town sector
			iThisDistance = FindStratPath(sEventSector, SECTOR_INFO_TO_STRATEGIC_INDEX(i->sector), g, FALSE);

			if (iThisDistance < iShortestDistance[ bTownId ])
			{
				iShortestDistance[ bTownId ] = iThisDistance;
			}
		}
	}

	// must always remove that temporary group!
	RemoveGroup(g);

	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		// doesn't affect towns where player hasn't established a "presence" yet
		if (!gTownLoyalty[ bTownId ].fStarted)
		{
			continue;
		}

		// if event was underground, double effective distance
		if (bSectorZ != 0)
		{
			iShortestDistance[ bTownId ] *= 2;
		}

		// This number here controls how many sectors away is the "norm" for the unadjusted loyalty change value
		if (iShortestDistance[ bTownId ] < LOYALTY_EVENT_DISTANCE_THRESHOLD)
		{
			// add 25% per sector below the threshold
			iPercentAdjustment = 25 * (LOYALTY_EVENT_DISTANCE_THRESHOLD - iShortestDistance[ bTownId ]);
		}
		else
		if (iShortestDistance[ bTownId ] > LOYALTY_EVENT_DISTANCE_THRESHOLD)
		{
			// subtract 10% per sector above the threshold
			iPercentAdjustment = -10 * (iShortestDistance[ bTownId ] - LOYALTY_EVENT_DISTANCE_THRESHOLD);

			// don't allow it to go below -100, that would change the sign of the loyalty effect!
			if (iPercentAdjustment < -100)
			{
				iPercentAdjustment = -100;
			}
		}
		else
		{
			// no distance adjustment necessary
			iPercentAdjustment = 0;
		}


		// calculate loyalty affects as adjusted for distance to this town
		iDistanceAdjustedLoyalty = (iLoyaltyChange * (100 + iPercentAdjustment)) / 100;

		if (iDistanceAdjustedLoyalty == 0)
		{
			// no measurable effect, skip this town
			continue;
		}

		if ( iDistanceAdjustedLoyalty > 0)
		{
			IncrementTownLoyalty( bTownId, iDistanceAdjustedLoyalty );
		}
		else
		{
			// the decrement amount MUST be positive
			iDistanceAdjustedLoyalty *= -1;
			DecrementTownLoyalty( bTownId, iDistanceAdjustedLoyalty );
		}
	}

}



// to be called whenever player gains control of a sector in any way
void CheckIfEntireTownHasBeenLiberated( INT8 bTownId, INT16 sSectorX, INT16 sSectorY )
{
	// the whole town is under our control, check if we never libed this town before
	if ( !gTownLoyalty[ bTownId ].fLiberatedAlready && IsTownUnderCompleteControlByPlayer ( bTownId ) )
	{
		if ( MilitiaTrainingAllowedInSector( sSectorX, sSectorY, 0 ) )
		{
			// give a loyalty bonus
			HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN, sSectorX, sSectorY, 0 );

			// set fact is has been lib'ed and set history event
			AddHistoryToPlayersLog( HISTORY_LIBERATED_TOWN, bTownId, GetWorldTotalMin(), sSectorX, sSectorY );

			HandleMeanWhileEventPostingForTownLiberation( bTownId );
		}

		// even taking over non-trainable "towns" like Orta/Tixa for the first time should count as "player activity"
		if ( gGameOptions.ubDifficultyLevel >= DIF_LEVEL_HARD )
		{
			UpdateLastDayOfPlayerActivity( ( UINT16 ) ( GetWorldDay() + 4 ) );
		}
		else
		{
			UpdateLastDayOfPlayerActivity( ( UINT16 ) ( GetWorldDay() + 2 ) );
		}

		// set flag even for towns where you can't train militia, useful for knowing Orta/Tixa were previously controlled
		gTownLoyalty[ bTownId ].fLiberatedAlready = TRUE;
	}
}

void CheckIfEntireTownHasBeenLost( INT8 bTownId, INT16 sSectorX, INT16 sSectorY )
{
	// NOTE:  only towns which allow you to train militia are important enough to get
	// reported here (and they're the only ones you can protect)
	if ( MilitiaTrainingAllowedInSector( sSectorX, sSectorY, 0 ) && IsTownUnderCompleteControlByEnemy(bTownId) )
	{
		// the whole town is under enemy control, check if we libed this town before
		if ( gTownLoyalty[ bTownId ].fLiberatedAlready )
		{
			HandleMeanWhileEventPostingForTownLoss();
		}
	}
}




void HandleLoyaltyChangeForNPCAction( UINT8 ubNPCProfileId )
{
	switch ( ubNPCProfileId )
	{
		case MIGUEL:
			// Omerta loyalty increases when Miguel receives letter from Enrico
			IncrementTownLoyalty( OMERTA, LOYALTY_BONUS_MIGUEL_READS_LETTER );
			break;

		case DOREEN:
			// having freed the child labourers... she is releasing them herself!
			IncrementTownLoyalty( DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED );
			break;

		case MARTHA:
			// if Joey is still alive
			if ( gMercProfiles[ JOEY ].bMercStatus != MERC_IS_DEAD )
			{
				IncrementTownLoyalty( CAMBRIA, LOYALTY_BONUS_MARTHA_WHEN_JOEY_RESCUED );
			}
			break;

		case KEITH:
			// Hillbilly problem solved
			IncrementTownLoyalty( CAMBRIA, LOYALTY_BONUS_KEITH_WHEN_HILLBILLY_SOLVED );
			break;

		case YANNI:
			// Chalice of Chance returned to Chitzena
			IncrementTownLoyalty( CHITZENA, LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_LOCAL );
			// NOTE: This affects Chitzena,too, a second time, so first value is discounted for it
			IncrementTownLoyaltyEverywhere( LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_GLOBAL );
			break;

		case AUNTIE:
			// Bloodcats killed
			IncrementTownLoyalty( ALMA, LOYALTY_BONUS_AUNTIE_WHEN_BLOODCATS_KILLED );
			break;

		case MATT:
			// Brother Dynamo freed
			IncrementTownLoyalty( ALMA, LOYALTY_BONUS_MATT_WHEN_DYNAMO_FREED );
			break;

	}
}



// set the location of the first encounter with enemy
void SetTheFirstBattleSector( INT16 sSectorValue )
{
	if( sWorldSectorLocationOfFirstBattle == 0 )
	{
		sWorldSectorLocationOfFirstBattle = sSectorValue;
	}
}


// Did first battle take place here?
bool DidFirstBattleTakePlaceInThisTown(INT8 const town)
{
	return GetTownIdForSector(STRATEGIC_INDEX_TO_SECTOR_INFO(sWorldSectorLocationOfFirstBattle)) == town;
}


static UINT32 PlayerStrength(void)
{
	UINT32 uiStrength, uiTotal = 0;

	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector ||
			(
				s->fBetweenSectors &&
				s->ubPrevSectorID % 16 + 1 == gWorldSectorX &&
				s->ubPrevSectorID / 16 + 1 == gWorldSectorY &&
				s->bSectorZ == gbWorldSectorZ
			))
		{
			// count this person's strength (condition), calculated as life reduced up to half according to maxbreath
			uiStrength = s->bLife * (s->bBreathMax + 100) / 200;
			uiTotal += uiStrength;
		}
	}
	return( uiTotal );
}


static UINT32 EnemyStrength(void)
{
	UINT32 strength = 0;
	CFOR_EACH_NON_PLAYER_SOLDIER(s)
	{
		if (!s->bInSector || s->bNeutral) continue;
		/* count this person's strength (condition), calculated as life reduced up
		 * to half according to maxbreath */
		strength += s->bLife * (s->bBreathMax + 100) / 200;
	}
	return strength;
}


//Function assumes that mercs have retreated already.  Handles two cases, one for general merc retreat
//which slightly demoralizes the mercs, the other handles abandonment of militia forces which poses
//as a serious loyalty penalty.
void HandleLoyaltyImplicationsOfMercRetreat( INT8 bRetreatCode, INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	if( CountAllMilitiaInSector( sSectorX, sSectorY ) )
	{ //Big morale penalty!
		HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_ABANDON_MILITIA, sSectorX, sSectorY, (INT8)sSectorZ );
	}

	//Standard retreat penalty
	if ( bRetreatCode == RETREAT_TACTICAL_TRAVERSAL )
	{
		// if not worse than 2:1 odds, then penalize morale
		if ( gTacticalStatus.fEnemyInSector && ( PlayerStrength() * 2 >= EnemyStrength() ) )
		{
			HandleMoraleEvent( NULL, MORALE_RAN_AWAY, sSectorX, sSectorY, (INT8)sSectorZ );
		}
	}
	else
	{
		HandleMoraleEvent( NULL, MORALE_RAN_AWAY, sSectorX, sSectorY, (INT8)sSectorZ );
	}
}


void MaximizeLoyaltyForDeidrannaKilled(void)
{
	INT8 bTownId;

	// max out loyalty to player all across Arulco
	for( bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++ )
	{
		// it's possible one of the towns still has creature problems, but it's too much of a pain to worry about it now
		SetTownLoyalty(bTownId, 100);
	}
}
