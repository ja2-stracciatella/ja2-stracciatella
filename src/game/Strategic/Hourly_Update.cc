#include "Assignments.h"
#include "Hourly_Update.h"
#include "Map_Screen_Helicopter.h"
#include "Merc_Contract.h"
#include "Soldier_Profile.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Merc_Handler.h"
#include "Strategic_Mines.h"
#include "Campaign.h"
#include "Morale.h"
#include "Quests.h"
#include "Game_Clock.h"
#include "Overhead.h"
#include "Boxing.h"
#include "JAScreens.h"
#include "Items.h"
#include "Random.h"
#include "Finances.h"
#include "History.h"
#include "Dialogue_Control.h"
#include "ScreenIDs.h"

#include "ContentManager.h"
#include "GameInstance.h"


static void HourlyCheckIfSlayAloneSoHeCanLeave(void);
static void HourlyLarryUpdate(void);
static void HourlyQuestUpdate(void);
static void UpdateRegenCounters(void);


// This function gets called every hour, on the hour.
// It spawns the handling of all sorts of stuff:
// Morale changes, assignment progress, etc.
void HandleHourlyUpdate()
{
	//if the game hasnt even started yet ( we havent arrived in the sector ) dont process this
	if ( DidGameJustStart() )
		return;

	// hourly update of team assignments
	UpdateAssignments();

	// hourly update of hated/liked mercs
	UpdateBuddyAndHatedCounters();

	// update morale!
	HourlyMoraleUpdate();

	// degrade camouflage
	HourlyCamouflageUpdate();

	// check mines for monster infestation/extermination
	HourlyMinesUpdate();

	// check how well the player is doing right now
	HourlyProgressUpdate();

	// do any quest stuff necessary
	HourlyQuestUpdate();

	HourlyLarryUpdate();

	HourlyCheckIfSlayAloneSoHeCanLeave();

	PayOffSkyriderDebtIfAny();


	if ( GetWorldHour() % 6 == 0 ) // 4 times a day
	{
		UpdateRegenCounters();
	}
}


static void UpdateRegenCounters(void)
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bRegenBoostersUsedToday > 0) --s->bRegenBoostersUsedToday;
	}
}


void HandleQuarterHourUpdate()
{
	//if the game hasnt even started yet ( we havent arrived in the sector ) dont process this
	if ( DidGameJustStart() )
		return;

	DecayTacticalMoraleModifiers( );
}


static void HourlyQuestUpdate(void)
{
	UINT32 uiHour = GetWorldHour();

	// brothel
	if ( uiHour == 4 )
	{
		SetFactFalse( FACT_BROTHEL_OPEN );
	}
	else if ( uiHour == 20 )
	{
		SetFactTrue( FACT_BROTHEL_OPEN );
	}

	// bar/nightclub
	if ( uiHour == 15 )
	{
		UINT8 ubLoop;

		SetFactTrue( FACT_CLUB_OPEN );
		SetFactFalse( FACT_PAST_CLUB_CLOSING_AND_PLAYER_WARNED );

		// reset boxes fought
		for ( ubLoop = 0; ubLoop < NUM_BOXERS; ubLoop++ )
		{
			gfBoxerFought[ ubLoop ] = FALSE;
		}

		// if # of boxing matches the player has won is a multiple of
		// 3, and the boxers haven't rested, then make them rest
		if ( gfBoxersResting )
		{
			// done resting now!
			gfBoxersResting = FALSE;
			gubBoxersRests++;
		}
		else if ( gubBoxingMatchesWon / 3 > gubBoxersRests )
		{
			// time for the boxers to rest!
			gfBoxersResting = TRUE;
		}
	}
	else if ( uiHour == 2 )
	{
		SetFactFalse( FACT_CLUB_OPEN );
	}

	// museum
	if ( uiHour == 9 )
	{
		SetFactTrue( FACT_MUSEUM_OPEN );
	}
	else if ( uiHour == 18 )
	{
		SetFactFalse( FACT_MUSEUM_OPEN );
	}

}

#define BAR_TEMPTATION 4
#define NUM_LARRY_ITEMS 6
UINT16	LarryItems[ NUM_LARRY_ITEMS ][ 3 ] =
{
	// item, temptation, points to use
	{ ADRENALINE_BOOSTER,		5, 100 },
	{	ALCOHOL,							BAR_TEMPTATION, 25  },
	{ MEDICKIT,							4, 10  },
	{	WINE,									3, 50  },
	{ REGEN_BOOSTER,				3, 100 },
	{	BEER,									2, 100 },
};

#define LARRY_FALLS_OFF_WAGON 8


static void HourlyLarryUpdate(void)
{
	INT8							bSlot, bBoozeSlot;
	INT8							bLarryItemLoop;
	UINT16						usTemptation = 0;
	UINT16						usCashAmount;
	BOOLEAN						fBar = FALSE;

	SOLDIERTYPE* pSoldier = FindSoldierByProfileIDOnPlayerTeam(LARRY_NORMAL);
	if ( !pSoldier )
	{
		pSoldier = FindSoldierByProfileIDOnPlayerTeam(LARRY_DRUNK);
	}
	if ( pSoldier )
	{
		if ( pSoldier->bAssignment >= ON_DUTY )
		{
			return;
		}
		if ( pSoldier->fBetweenSectors )
		{
			return;
		}
		if (pSoldier->bInSector && (gTacticalStatus.fEnemyInSector || guiCurrentScreen == GAME_SCREEN))
		{
			return;
		}

		// look for items in Larry's inventory to maybe use
		for ( bLarryItemLoop = 0; bLarryItemLoop < NUM_LARRY_ITEMS; bLarryItemLoop++ )
		{
			bSlot = FindObj( pSoldier, LarryItems[ bLarryItemLoop ][ 0 ] );
			if ( bSlot != NO_SLOT )
			{
				usTemptation = LarryItems[ bLarryItemLoop ][ 1 ];
				break;
			}
		}

		// check to see if we're in a bar sector, if we are, we have access to alcohol
		// which may be better than anything we've got...
		static const SGPSector bars[] = {
			{13, MAP_ROW_D},
			{13, MAP_ROW_C},
			{5, MAP_ROW_C},
			{5, MAP_ROW_D},
			{6, MAP_ROW_C},
			{2, MAP_ROW_H}
		};
		if ( usTemptation < BAR_TEMPTATION && GetCurrentBalance() >= GCM->getItem(ALCOHOL)->getPrice() )
		{
			for (const auto& bar : bars)
			{
				if (pSoldier->sSector == bar)
				{
					// in a bar!
					fBar = TRUE;
					usTemptation = BAR_TEMPTATION;
					break;
				}
			}
		}

		if ( usTemptation > 0 )
		{
			if ( pSoldier->ubProfile == LARRY_NORMAL )
			{
				gMercProfiles[ LARRY_NORMAL ].bNPCData += (INT8) Random( usTemptation );
				if ( gMercProfiles[ LARRY_NORMAL ].bNPCData >= LARRY_FALLS_OFF_WAGON )
				{
					if ( fBar )
					{
						// take $ from player's account
						usCashAmount = GCM->getItem(ALCOHOL)->getPrice();
						AddTransactionToPlayersBook ( TRANSFER_FUNDS_TO_MERC, pSoldier->ubProfile, GetWorldTotalMin() , -( usCashAmount ) );
						// give Larry some booze and set slot etc values appropriately
						bBoozeSlot = FindEmptySlotWithin( pSoldier, HANDPOS, SMALLPOCK8POS );
						if ( bBoozeSlot != NO_SLOT )
						{
							// give Larry booze here
							CreateItem( ALCOHOL, 100, &(pSoldier->inv[bBoozeSlot]) );
						}
						bSlot = bBoozeSlot;
						bLarryItemLoop = 1;
					}
					// ahhhh!!!
					SwapLarrysProfiles( pSoldier );
					if ( bSlot != NO_SLOT )
					{
						UseKitPoints(pSoldier->inv[bSlot], LarryItems[bLarryItemLoop][2], *pSoldier);
					}
				}
			}
			else
			{
				// NB store all drunkenness info in LARRY_NORMAL profile (to use same values)
				// so long as he keeps consuming, keep number above level at which he cracked
				gMercProfiles[ LARRY_NORMAL ].bNPCData = __max( gMercProfiles[ LARRY_NORMAL ].bNPCData, LARRY_FALLS_OFF_WAGON );
				gMercProfiles[ LARRY_NORMAL ].bNPCData += (INT8) Random( usTemptation );
				// allow value to keep going up to 24 (about 2 days since we subtract Random( 2 ) when he has no access )
				gMercProfiles[ LARRY_NORMAL ].bNPCData = __min( gMercProfiles[ LARRY_NORMAL ].bNPCData, 24 );
				if ( fBar )
				{
					// take $ from player's account
					usCashAmount = GCM->getItem(ALCOHOL)->getPrice();
					AddTransactionToPlayersBook ( TRANSFER_FUNDS_TO_MERC, pSoldier->ubProfile, GetWorldTotalMin() , -( usCashAmount ) );
					// give Larry some booze and set slot etc values appropriately
					bBoozeSlot = FindEmptySlotWithin( pSoldier, HANDPOS, SMALLPOCK8POS );
					if ( bBoozeSlot != NO_SLOT )
					{
						// give Larry booze here
						CreateItem( ALCOHOL, 100, &(pSoldier->inv[bBoozeSlot]) );
					}
					bSlot = bBoozeSlot;
					bLarryItemLoop = 1;
				}
				if ( bSlot != NO_SLOT )
				{
					// ahhhh!!!
					UseKitPoints(pSoldier->inv[bSlot], LarryItems[bLarryItemLoop][2], *pSoldier);
				}
			}
		}
		else if ( pSoldier->ubProfile == LARRY_DRUNK )
		{
			gMercProfiles[ LARRY_NORMAL ].bNPCData -= (INT8) Random( 2 );
			if ( gMercProfiles[ LARRY_NORMAL ].bNPCData <= 0 )
			{
				// goes sober!
				SwapLarrysProfiles( pSoldier );
			}
		}

	}

}


static void HourlyCheckIfSlayAloneSoHeCanLeave(void)
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(SLAY);
	if( !pSoldier )
	{
		return;
	}
	if( pSoldier->fBetweenSectors )
	{
		return;
	}
	if (pSoldier->bLife == 0) return;
	if (PlayerMercsInSector(pSoldier->sSector) == 1)
	{
		if( Chance( 15 ) )
		{
			pSoldier->ubLeaveHistoryCode = HISTORY_SLAY_MYSTERIOUSLY_LEFT;
			MakeCharacterDialogueEventContractEndingNoAskEquip(*pSoldier);
		}
	}
}
