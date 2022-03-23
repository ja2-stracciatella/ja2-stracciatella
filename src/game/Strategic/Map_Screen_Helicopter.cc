#include "Map_Screen_Helicopter.h"

#include "Assignments.h"
#include "ContentManager.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Finances.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "Isometric_Utils.h"
#include "LaptopSave.h"
#include "MapScreen.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Meanwhile.h"
#include "Message.h"
#include "MessageBoxScreen.h"
#include "Overhead.h"
#include "Player_Command.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Random.h"
#include "RenderWorld.h"
#include "SamSiteModel.h"
#include "Scheduling.h"
#include "Soldier_Create.h"
#include "Soldier_Profile.h"
#include "SoundMan.h"
#include "Sound_Control.h"
#include "Squads.h"
#include "StrategicMap.h"
#include "StrategicMap_Secrets.h"
#include "Strategic_Event_Handler.h"
#include "Strategic_Movement.h"
#include "Strategic_Pathing.h"
#include "Text.h"
#include "TileDat.h"
#include "UILayout.h"
#include "Vehicles.h"
#include "WorldDef.h"
#include "WorldMan.h"

// the amounts of time to wait for hover stuff
#define TIME_DELAY_FOR_HOVER_WAIT						10		// minutes
#define TIME_DELAY_FOR_HOVER_WAIT_TOO_LONG	20		// mintues
#define MIN_DAYS_BETWEEN_SKYRIDER_MONOLOGUES 1

// maximum chance out of a hundred per unsafe sector that a SAM site in decent working condition will hit Skyrider
#define MAX_SAM_SITE_ACCURACY		33

// current temp path for dest char
extern PathSt* pTempHelicopterPath;

// whether helicopted variables have been set up
BOOLEAN fSkyRiderSetUp = FALSE;

// plotting for a helicopter
BOOLEAN fPlotForHelicopter = FALSE;

// helicopter vehicle id
INT32 iHelicopterVehicleId = -1;

// total owed to player
INT32 iTotalAccumulatedCostByPlayer = 0;

// helicopter destroyed
BOOLEAN fHelicopterDestroyed = FALSE;

struct RefuelSite
{
	INT16  sector;
	GridNo grid_no;
	INT16  heli_ostruct;
};

// list of sector locations where SkyRider can be refueled
static RefuelSite const g_refuel_site[] =
{
	{ CALCULATE_STRATEGIC_INDEX(13, 2),  9001, FIRSTOSTRUCT1  }, // Drassen airport
	{ CALCULATE_STRATEGIC_INDEX(6,  9), 13067, FOURTHOSTRUCT1 }  // Estoni
};

enum SkyriderMonologueEvent
{
	SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE = 0,
	SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES,
	SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL,
	SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL
};

// whether or not helicopter can refuel at this site
BOOLEAN fRefuelingSiteAvailable[ NUMBER_OF_REFUEL_SITES ] = { FALSE, FALSE };

// is the heli in the air?
BOOLEAN fHelicopterIsAirBorne = FALSE;

// is the pilot returning straight to base?
BOOLEAN fHeliReturnStraightToBase = FALSE;

// heli hovering
BOOLEAN fHoveringHelicopter = FALSE;

// time started hovering
UINT32 uiStartHoverTime = 0;

// what state are skyrider's monologues in in?
UINT32 guiHelicopterSkyriderTalkState = 0;

// the flags for skyrider events
BOOLEAN fShowEstoniRefuelHighLight = FALSE;
BOOLEAN fShowOtherSAMHighLight = FALSE;
BOOLEAN fShowDrassenSAMHighLight = FALSE;
BOOLEAN fShowCambriaHospitalHighLight = FALSE;
UINT32 guiTimeOfLastSkyriderMonologue = 0;
UINT8 gubHelicopterHitsTaken = 0;
BOOLEAN gfSkyriderSaidCongratsOnTakingSAM = FALSE;
UINT8 gubPlayerProgressSkyriderLastCommentedOn = 0;

static BOOLEAN DoesSkyriderNoticeEnemiesInSector(UINT8 ubNumEnemies);
static BOOLEAN HandleSAMSiteAttackOfHelicopterInSector(INT16 sSectorX, INT16 sSectorY);
static void HeliCharacterDialogue(UINT16 usQuoteNum);
static void PaySkyriderBill(void);
static void StartHoverTime(void);
static RefuelSite const* FindClosestRefuelSite(bool must_be_available);
static void LandHelicopter(void);
static void MakeHeliReturnToBase(void);
static void HandleSkyRiderMonologueEvent(SkyriderMonologueEvent, UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutCambriaHospital(UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutDrassenSAMSite(UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutEstoniRefuel(UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutOtherSAMSites(UINT32 uiSpecialCode);
static void AddHelicopterToMaps(bool add, RefuelSite const&);
static bool IsHelicopterOnGroundAtRefuelingSite(RefuelSite const&);

void InitializeHelicopter( void )
{
	// must be called whenever a new game starts up!
	iHelicopterVehicleId = -1;

	fSkyRiderSetUp = FALSE;

	fHelicopterIsAirBorne = FALSE;
	fHeliReturnStraightToBase = FALSE;

	fHoveringHelicopter = FALSE;
	uiStartHoverTime = 0;

	fPlotForHelicopter = FALSE;
	pTempHelicopterPath = NULL;

	iTotalAccumulatedCostByPlayer = 0;

	fHelicopterDestroyed = FALSE;

	guiHelicopterSkyriderTalkState = 0;
	guiTimeOfLastSkyriderMonologue = 0;

	fShowEstoniRefuelHighLight = FALSE;
	fShowOtherSAMHighLight = FALSE;
	fShowDrassenSAMHighLight = FALSE;
	fShowCambriaHospitalHighLight = FALSE;

	gfSkyriderEmptyHelpGiven = FALSE;

	gubHelicopterHitsTaken = 0;

	gfSkyriderSaidCongratsOnTakingSAM = FALSE;
	gubPlayerProgressSkyriderLastCommentedOn = 0;
}

BOOLEAN RemoveSoldierFromHelicopter( SOLDIERTYPE *pSoldier )
{
	// attempt to add soldier to helicopter
	if( iHelicopterVehicleId == -1 )
	{
		// no heli yet
		return( FALSE );
	}

	// check if heli is in motion or if on the ground
	if (fHelicopterIsAirBorne && !fHoveringHelicopter) return FALSE;

	// is the heli returning to base?..he ain't waiting if so
	if (fHeliReturnStraightToBase) return FALSE;

	VEHICLETYPE const& v = GetHelicopter();
	pSoldier->sSectorX = v.sSectorX;
	pSoldier->sSectorY = v.sSectorY;
	pSoldier->bSectorZ = 0;

	// reset between sectors
	pSoldier->fBetweenSectors = FALSE;

	// remove from the vehicle
	return( TakeSoldierOutOfVehicle( pSoldier ) );
}

BOOLEAN HandleHeliEnteringSector(const SGPSector& sMap)
{
	UINT8 ubNumEnemies;
	BOOLEAN endOfHelicoptersPath;

	VEHICLETYPE const& v = GetHelicopter();
	endOfHelicoptersPath = (!v.pMercPath || !v.pMercPath->pNext);

	// check for SAM attack upon the chopper.  If it's destroyed by the attack, do nothing else here
	if (HandleSAMSiteAttackOfHelicopterInSector(sMap.x, sMap.y))
	{
		// destroyed
		return( TRUE );
	}

	// count how many enemies are camped there or passing through
	ubNumEnemies = NumEnemiesInSector(sMap);

	// any baddies?
	if( ubNumEnemies > 0 )
	{
		// if the player didn't know about these prior to the chopper's arrival
		if (WhatPlayerKnowsAboutEnemiesInSector(sMap) == KNOWS_NOTHING)
		{
			// but Skyrider notices them
			if (DoesSkyriderNoticeEnemiesInSector(ubNumEnemies))
			{
				// if just passing through (different quotes are used below if it's his final destination)
				if( !endOfHelicoptersPath )
				{
					// stop time compression and inform player that there are enemies in the sector below
					StopTimeCompression();

					if( Random( 2 ) )
					{
						HeliCharacterDialogue(ENEMIES_SPOTTED_EN_ROUTE_IN_FRIENDLY_SECTOR_A);
					}
					else
					{
						HeliCharacterDialogue(ENEMIES_SPOTTED_EN_ROUTE_IN_FRIENDLY_SECTOR_B);
					}
				}
				// make their presence appear on the map while Skyrider remains in the sector
				SectorInfo[sMap.AsByte()].uiFlags |= SF_SKYRIDER_NOTICED_ENEMIES_HERE;
			}
		}
	}

	// player pays for travel if Skyrider is NOT returning to base (even if empty while scouting/going for pickup)
	if (!fHeliReturnStraightToBase)
	{
		// charge cost for flying another sector
		INT32 iCost;
		if( !StrategicMap[sMap.AsStrategicIndex()].fEnemyAirControlled)
			iCost = COST_AIRSPACE_SAFE;
		else
			iCost = COST_AIRSPACE_UNSAFE;
		iTotalAccumulatedCostByPlayer += iCost;
	}

	// check if heli has any real path left
	if( endOfHelicoptersPath )
	{
		// start hovering
		StartHoverTime( );

		// if sector is safe, or Skyrider MUST land anyway (returning to base)
		if ( ( ubNumEnemies == 0 ) || fHeliReturnStraightToBase )
		{
			// if he has passengers, or he's not going straight to base, tell player he's arrived
			// (i.e. don't say anything nor stop time compression if he's empty and just returning to base)
			if ( ( GetNumberInVehicle(v) > 0 ) || !fHeliReturnStraightToBase )
			{
				// arrived at destination
				HeliCharacterDialogue(ARRIVED_IN_NON_HOSTILE_SECTOR);
				StopTimeCompression();
			}

			// destination reached, payment due.  If player can't pay, mercs get kicked off and heli flies to base!
			PaySkyriderBill();
		}
		else
		{
			// Say quote: "Gonna have to abort.  Enemies below"
			HeliCharacterDialogue(ARRIVED_IN_HOSTILE_SECTOR);
			StopTimeCompression();
		}

		if (IsRefuelAvailableInSector(sMap.AsStrategicIndex()))
		{
			LandHelicopter();
		}
	}
	return( FALSE );
}

static RefuelSite const& NearestRefuelPoint(bool const fNotifyPlayerIfNoSafeLZ)
{
	// Try to find one, any one under the players control
	RefuelSite const* closest_site = FindClosestRefuelSite(TRUE);
	if (closest_site) return *closest_site;

	if (fNotifyPlayerIfNoSafeLZ)
	{ // No refueling sites available, might wanna warn player about this
		ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pHelicopterEtaStrings[5]);
	}

	// Find the closest location regardless
	closest_site = FindClosestRefuelSite(FALSE);

	// Always returns a valid refuel point, picking a hostile one if unavoidable
	Assert(closest_site);
	return *closest_site;
}

// find the location sector of closest refuel point for heli..and the criteria if the sector must be under the players control
static RefuelSite const* FindClosestRefuelSite(bool const must_be_available)
{
	INT32             shortest_distance = 9999;
	RefuelSite const* closest_site      = 0;

	VEHICLETYPE const& v        				= GetHelicopter();
	INT16 sectorID											= CALCULATE_STRATEGIC_INDEX(v.sSectorX , v.sSectorY);
	GROUP& g														= *GetGroup(v.ubMovementGroup);
	// find shortest distance to refuel site
	for (INT32 i = 0; i < NUMBER_OF_REFUEL_SITES; ++i)
	{
		// if this refuelling site is available
		if (!fRefuelingSiteAvailable[i] && must_be_available) continue;

		// find if sector is under control, find distance from heli to it
		RefuelSite  const& r        = g_refuel_site[i];
		INT16       const  dest     = r.sector;
		INT32       const  distance = FindStratPath( sectorID, dest, g, FALSE);
		if (distance >= shortest_distance) continue;

		// shorter, copy over
		shortest_distance = distance;
		closest_site      = &r;
	}
	return closest_site;
}

// helicopter shot down, kill all on board
static void SkyriderDestroyed(void)
{
	// remove any arrival events for the helicopter's group
	VEHICLETYPE& v = GetHelicopter();
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, v.ubMovementGroup);
	KillAllInVehicle(v);

	// kill skyrider
	gMercProfiles[ SKYRIDER ].bLife = 0;

	// destroy helicopter
	fHelicopterDestroyed = TRUE;

	// zero out balance due
	gMercProfiles[ SKYRIDER ].iBalance = 0;
	iTotalAccumulatedCostByPlayer = 0;

	// remove vehicle and reset
	RemoveVehicleFromList(v);
	iHelicopterVehicleId = -1;
}

BOOLEAN CanHelicopterFly( void )
{
	// check if heli is available for flight?

	// is the heli available
	if (iHelicopterVehicleId == -1) return FALSE;

	// is the pilot alive, well, and willing to help us?
	if (!IsHelicopterPilotAvailable()) return FALSE;

	if (fHeliReturnStraightToBase) return FALSE;

	// grounded by enemies in sector?
	if (!CanHelicopterTakeOff()) return FALSE;

	// everything A-OK!
	return( TRUE );
}

BOOLEAN IsHelicopterPilotAvailable( void )
{
	// what is state of skyrider?
	if (iHelicopterVehicleId == -1) return FALSE;

	// owe any money to skyrider?
	if( gMercProfiles[ SKYRIDER ].iBalance < 0 ) return( FALSE );

	// last case: Drassen too disloyal to wanna help player? if not, return true
	return ( !CheckFact( FACT_LOYALTY_LOW, SKYRIDER ) );
}

// land the helicopter here
static void LandHelicopter(void)
{
	// set the helictoper down, call arrive callback for this mvt group
	fHelicopterIsAirBorne = FALSE;

	// no longer hovering
	fHoveringHelicopter = FALSE;

	// reset fact that we might have returned straight here
	fHeliReturnStraightToBase = FALSE;
	HandleHelicopterOnGround(true);

	// if we'll be unable to take off again (because there are enemies in the sector, or we owe pilot money)
	if (!CanHelicopterFly())
	{
		// kick everyone out!
		MoveAllInHelicopterToFootMovementGroup( );
	}
	else
	{
		// play meanwhile scene if it hasn't been used yet
		HandleKillChopperMeanwhileScene();
	}
}

void TakeOffHelicopter( void )
{
	// heli in the air
	fHelicopterIsAirBorne = TRUE;

	// no longer hovering
	fHoveringHelicopter = FALSE;
	HandleHelicopterOnGround(true);
}

// start the heli hover time
static void StartHoverTime(void)
{
	// start hover in this sector
	fHoveringHelicopter = TRUE;

	// post event for x mins in future, save start time, if event time - delay = start time, then hover has gone on too long
	uiStartHoverTime = GetWorldTotalMin( );

	// post event..to call handle hover
	AddStrategicEvent( EVENT_HELICOPTER_HOVER_TOO_LONG, GetWorldTotalMin() + TIME_DELAY_FOR_HOVER_WAIT, 0 );
}

void HandleHeliHoverLong( void )
{
	// post message about hovering too long
	if( fHoveringHelicopter )
	{
		// proper event, post next one
		AddStrategicEvent( EVENT_HELICOPTER_HOVER_WAY_TOO_LONG, uiStartHoverTime + TIME_DELAY_FOR_HOVER_WAIT_TOO_LONG, 0 );

		// inform player
		HeliCharacterDialogue(HOVERING_A_WHILE);

		// stop time compression if it's on
		StopTimeCompression( );
	}
	else
	{
		// reset
		uiStartHoverTime = 0;
	}
}

void HandleHeliHoverTooLong( void )
{
	// reset hover time
	uiStartHoverTime = 0;
	if (!fHoveringHelicopter) return;

	// hovered too long, inform player heli is returning to base
	HeliCharacterDialogue(RETURN_TO_BASE);
	VEHICLETYPE const& v = GetHelicopter();
	// If the sector is safe
	if (NumEnemiesInSector(SGPSector(v.sSectorX, v.sSectorY)) == 0)
	{
		// kick everyone out!
		MoveAllInHelicopterToFootMovementGroup( );
	}
	MakeHeliReturnToBase();
}

// check if anyone in the chopper sees any baddies in sector
static BOOLEAN DoesSkyriderNoticeEnemiesInSector(UINT8 ubNumEnemies)
{
	UINT8 ubChance;

	// is the pilot and heli around?
	if (!CanHelicopterFly()) return FALSE;

	// if there aren't any, he obviously won't see them
	if( ubNumEnemies == 0 )
	{
		return( FALSE );
	}

	// figure out what the chance is of seeing them
	// make this relatively accurate most of the time, to encourage helicopter scouting by making it useful
	ubChance = 60 + ubNumEnemies;
	if( PreRandom( 100 ) < ubChance )
	{
		return( TRUE );
	}
	return( FALSE );
}

// if the heli is on the move, what is the distance it will move..the length of the merc path, less the first node
INT32 DistanceOfIntendedHelicopterPath( void )
{
	INT32 iLength = 0;
	if (!CanHelicopterFly())
	{
		// big number, no go
		return( 9999 );
	}

	PathSt const* pNode = GetHelicopter().pMercPath;

	// any path yet?
	if( pNode != NULL )
	{
		while( pNode -> pNext )
		{
			iLength++;
			pNode = pNode ->pNext;
		}
	}

	pNode = pTempHelicopterPath;

	// any path yet?
	if( pNode != NULL )
	{
		while( pNode -> pNext )
		{
			iLength++;
			pNode = pNode ->pNext;
		}
	}
	return( iLength );
}

void SetUpHelicopterForMovement( void )
{
	// check if helicopter vehicle has a mvt group, if not, assign one in this sector
	VEHICLETYPE& v = GetHelicopter();

	// if no group, create one for vehicle
	if (v.ubMovementGroup == 0)
	{
		// get the vehicle a mvt group
		GROUP& g = *CreateNewVehicleGroupDepartingFromSector(v.sSectorX, v.sSectorY);
		v.ubMovementGroup = g.ubGroupID;

		// add everyone in vehicle to this mvt group
		CFOR_EACH_PASSENGER(v, i)
		{
			AddPlayerToGroup(g, **i);
		}
	}
}

static void SkyriderDialogue(UINT16 const quote)
{
	CharacterDialogue(SKYRIDER, quote, GetExternalNPCFace(SKYRIDER), DIALOGUE_EXTERNAL_NPC_UI, FALSE);
}

static void SkyriderDialogueWithSpecialEvent(SkyriderMonologueEvent const event, UINT32 const special_code)
{
	class DialogueEventSkyriderMapScreenEvent : public DialogueEvent
	{
		public:
			DialogueEventSkyriderMapScreenEvent(SkyriderMonologueEvent const event, UINT32 const special_code) :
				event_(event),
				special_code_(special_code)
			{}

			bool Execute()
			{
				HandleSkyRiderMonologueEvent(event_, special_code_);
				return false;
			}

		private:
			SkyriderMonologueEvent const event_;
			UINT32                 const special_code_;
	};
	DialogueEvent::Add(new DialogueEventSkyriderMapScreenEvent(event, special_code));
}

static void HeliCharacterDialogue(UINT16 const usQuoteNum)
{
	// ARM: we could just return, but since various flags are often being set it's safer to honk so it gets fixed right!
	Assert(iHelicopterVehicleId != -1);
	SkyriderDialogue(usQuoteNum);
}

bool IsRefuelSiteInSector(INT16 const sector)
{
	FOR_EACH(RefuelSite const, i, g_refuel_site)
	{
		if (i->sector == sector) return true;
	}
	return false;
}

bool IsRefuelAvailableInSector(INT16 const sector)
{
	return NearestRefuelPoint(false).sector == sector;
}

void UpdateRefuelSiteAvailability( void )
{
	INT32 iCounter = 0;

	// Generally, only Drassen is initially available for refuelling
	// Estoni must first be captured (although player may already have it when he gets Skyrider!)

	for( iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++ )
	{
		// if enemy controlled sector (ground OR air, don't want to fly into enemy air territory)
		StrategicMapElement const& m = StrategicMap[g_refuel_site[iCounter].sector];
		if (m.fEnemyControlled    ||
				m.fEnemyAirControlled ||
				(iCounter == ESTONI_REFUELING_SITE && !CheckFact(FACT_ESTONI_REFUELLING_POSSIBLE, 0)))
		{
			// mark refueling site as unavailable
			fRefuelingSiteAvailable[ iCounter ] = FALSE;
		}
		else
		{
			// mark refueling site as available
			fRefuelingSiteAvailable[ iCounter ] = TRUE;
		}
	}
}

void SetUpHelicopterForPlayer( INT16 sX, INT16 sY )
{
	if (!fSkyRiderSetUp)
	{
		iHelicopterVehicleId = AddVehicleToList( sX, sY, 0, HELICOPTER );

		// set up for movement
		SetUpHelicopterForMovement( );
		UpdateRefuelSiteAvailability( );
		fSkyRiderSetUp = TRUE;
		gMercProfiles[ SKYRIDER ].fUseProfileInsertionInfo = FALSE;
	}
}

void MoveAllInHelicopterToFootMovementGroup(void)
{
	// take everyone out of heli and add to movement group
	INT8 bNewSquad;
	BOOLEAN fSuccess;
	UINT8   ubInsertionCode = (UINT8)-1; // XXX HACK000E
	BOOLEAN fInsertionCodeSet = FALSE;
	UINT16  usInsertionData = (UINT16)-1; // XXX HACK000E

	// put these guys on their own squad (we need to return their group ID, and can only return one, so they need a unique one
	bNewSquad = GetFirstEmptySquad();

	// go through list of everyone in helicopter
	VEHICLETYPE const& v = GetHelicopter();
	CFOR_EACH_PASSENGER(v, i)
	{
		SOLDIERTYPE* const pSoldier = *i;
		Assert(InHelicopter(*pSoldier));
		fSuccess = RemoveSoldierFromHelicopter( pSoldier );
		Assert( fSuccess );
		AddCharacterToSquad( pSoldier, bNewSquad );

		// ATE: OK - the ubStrategicInsertionCode is set 'cause groupArrivesInsector has been
		// called when buddy is added to a squad. However, the insertion code onlt sets set for
		// the first merc, so the rest are going to use whatever they had previously....
		if ( !fInsertionCodeSet )
		{
			ubInsertionCode = pSoldier->ubStrategicInsertionCode;
			usInsertionData = pSoldier->usStrategicInsertionData;
			fInsertionCodeSet = TRUE;
		}
		else
		{
			pSoldier->ubStrategicInsertionCode = ubInsertionCode;
			pSoldier->usStrategicInsertionData = usInsertionData;
		}
	}
}

void SkyRiderTalk( UINT16 usQuoteNum )
{
	// have skyrider talk to player
	HeliCharacterDialogue(usQuoteNum);
	fTeamPanelDirty = TRUE;
}

// Skyrider monlogue events for mapscreen
static void HandleSkyRiderMonologueEvent(SkyriderMonologueEvent const uiEventCode, UINT32 const uiSpecialCode)
{
	// will handle the skyrider monologue about where the SAM sites are and what not
	TurnOnAirSpaceMode();

	switch( uiEventCode )
	{
		case( SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE ):
			SetExternMapscreenSpeechPanelXY( DEFAULT_EXTERN_PANEL_X_POS, STD_SCREEN_Y + 117 );
			HandleSkyRiderMonologueAboutDrassenSAMSite( uiSpecialCode );
			break;
		case SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL:
			SetExternMapscreenSpeechPanelXY( DEFAULT_EXTERN_PANEL_X_POS, STD_SCREEN_Y + 172 );
			HandleSkyRiderMonologueAboutCambriaHospital( uiSpecialCode );
			break;
		case( SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES ):
			SetExternMapscreenSpeechPanelXY( STD_SCREEN_X + 335, DEFAULT_EXTERN_PANEL_Y_POS );
			HandleSkyRiderMonologueAboutOtherSAMSites( uiSpecialCode );
			break;
		case( SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL ):
			SetExternMapscreenSpeechPanelXY( DEFAULT_EXTERN_PANEL_X_POS, DEFAULT_EXTERN_PANEL_Y_POS );
			HandleSkyRiderMonologueAboutEstoniRefuel( uiSpecialCode );
			break;
	}

	// update time
	guiTimeOfLastSkyriderMonologue = GetWorldTotalMin();
}

static void HandleSkyRiderMonologueAboutEstoniRefuel(UINT32 const uiSpecialCode)
{
	// Once Estoni is free tell player about refueling
	switch (uiSpecialCode)
	{
		case 0:
		{
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL, 1);
			// if special event data 2 is true, then do dialogue, else this is just a trigger for an event
			SkyriderDialogue(SPIEL_ABOUT_ESTONI_AIRSPACE);
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL, 2);
			break;
		}

		case 1: // Highlight Estoni
			fShowEstoniRefuelHighLight = TRUE;
			break;

		case 2:
			fShowEstoniRefuelHighLight = FALSE;
			break;
	}
}

static void HandleSkyRiderMonologueAboutDrassenSAMSite(UINT32 const uiSpecialCode)
{
	switch (uiSpecialCode)
	{
		case 0:
		{
			// if special event data 2 is true, then do dialogue, else this is just a trigger for an event
			SkyriderDialogue(MENTION_DRASSEN_SAM_SITE);
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE, 1);

			auto samList = GCM->getSamSites();
			if (StrategicMap[SECTOR_INFO_TO_STRATEGIC_INDEX(samList[SAM_SITE_TWO]->sectorId)].fEnemyControlled)
			{
				SkyriderDialogue(SECOND_HALF_OF_MENTION_DRASSEN_SAM_SITE);
			}
			else if (CheckFact(FACT_SKYRIDER_USED_IN_MAPSCREEN, SKYRIDER))
			{ // Ian says don't use the SAM site quote unless player has tried flying already
				SkyriderDialogue(SAM_SITE_TAKEN);
				gfSkyriderSaidCongratsOnTakingSAM = TRUE;
			}
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE, 2);
			break;
		}

		case 1: // Highlight Drassen SAM site sector
			fShowDrassenSAMHighLight = TRUE;
			SetSAMSiteAsFound(SAM_SITE_TWO);
			break;

		case 2:
			fShowDrassenSAMHighLight = FALSE;
			break;
	}
}

static void HandleSkyRiderMonologueAboutCambriaHospital(UINT32 const uiSpecialCode)
{
	switch (uiSpecialCode)
	{
		case 0:
		{
			// if special event data 2 is true, then do dialogue, else this is just a trigger for an event
			SkyriderDialogue(MENTION_HOSPITAL_IN_CAMBRIA);
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL, 1);

			// Highlight Cambria hospital sector
			fShowCambriaHospitalHighLight = TRUE;
			break;
		}

		case 1:
			fShowCambriaHospitalHighLight = FALSE;
			break;
	}
}

static void HandleSkyRiderMonologueAboutOtherSAMSites(UINT32 const uiSpecialCode)
{
	/* Handle skyrider telling player about other SAM sites on fifth hiring or
	 * after one near drassen is taken out */
	switch (uiSpecialCode)
	{
		case 0:
		{
			// if special event data 2 is true, then do dialogue, else this is just a trigger for an event
			SkyriderDialogue(SPIEL_ABOUT_OTHER_SAM_SITES);
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES, 1);

			SkyriderDialogue(SECOND_HALF_OF_SPIEL_ABOUT_OTHER_SAM_SITES);
			SkyriderDialogueWithSpecialEvent(SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES, 2);
			break;
		}

		case 1: // Highlight other SAMs
			fShowOtherSAMHighLight = TRUE;
			// Reveal other 3 SAM sites
			SetSAMSiteAsFound(SAM_SITE_ONE);
			SetSAMSiteAsFound(SAM_SITE_THREE);
			SetSAMSiteAsFound(SAM_SITE_FOUR);
			break;

		case 2:
			fShowOtherSAMHighLight = FALSE;
			break;
	}
}

void CheckAndHandleSkyriderMonologues( void )
{
	// wait at least this many days between Skyrider monologues
	if ( ( GetWorldTotalMin() - guiTimeOfLastSkyriderMonologue ) >= ( MIN_DAYS_BETWEEN_SKYRIDER_MONOLOGUES * 24 * 60 ) )
	{
		if( guiHelicopterSkyriderTalkState == 0 )
		{
			HandleSkyRiderMonologueEvent( SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE, 0 );
			guiHelicopterSkyriderTalkState = 1;
		}
		else if( guiHelicopterSkyriderTalkState == 1 )
		{
			// if enemy still controls the Cambria hospital sector
			if( StrategicMap[ CALCULATE_STRATEGIC_INDEX( HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y ) ].fEnemyControlled )
			{
				HandleSkyRiderMonologueEvent( SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL, 0 );
			}
			// advance state even if player already has Cambria's hospital sector!!!
			guiHelicopterSkyriderTalkState = 2;
		}
		else if( guiHelicopterSkyriderTalkState == 2 )
		{
			// wait until player has taken over a SAM site before saying this and advancing state
			if ( gfSkyriderSaidCongratsOnTakingSAM )
			{
				HandleSkyRiderMonologueEvent( SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES, 0 );
				guiHelicopterSkyriderTalkState = 3;
			}
		}
		else if( guiHelicopterSkyriderTalkState == 3 )
		{
			// wait until Estoni refuelling site becomes available
			if ( fRefuelingSiteAvailable[ ESTONI_REFUELING_SITE ] )
			{
				HandleSkyRiderMonologueEvent( SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL, 0 );
				guiHelicopterSkyriderTalkState = 4;
			}
		}
	}
}


static void HandleBlitOfSectorLocatorIcon(UINT8 const sector, UINT8 const locator)
{
	HandleBlitOfSectorLocatorIcon(SECTORX(sector), SECTORY(sector), 0, locator);
}


void HandleAnimationOfSectors( void )
{
	BOOLEAN fSkipSpeakersLocator = FALSE;
	// these don't need to be saved, they merely turn off the highlights after they stop flashing
	static BOOLEAN fOldShowDrassenSAMHighLight = FALSE;
	static BOOLEAN fOldShowCambriaHospitalHighLight = FALSE;
	static BOOLEAN fOldShowEstoniRefuelHighLight = FALSE;
	static BOOLEAN fOldShowOtherSAMHighLight = FALSE;

	auto samList = GCM->getSamSites();

	// find out which mode we are in and animate for that mode

	// Drassen SAM site
	if( fShowDrassenSAMHighLight )
	{
		fOldShowDrassenSAMHighLight = TRUE;
		// Drassen's SAM site is #2
		HandleBlitOfSectorLocatorIcon(samList[SAM_SITE_TWO]->sectorId, LOCATOR_COLOR_RED);
		fSkipSpeakersLocator = TRUE;
	}
	else if( fOldShowDrassenSAMHighLight )
	{
		fOldShowDrassenSAMHighLight = FALSE;
		fMapPanelDirty = TRUE;
	}

	// Cambria hospital
	if( fShowCambriaHospitalHighLight )
	{
		fOldShowCambriaHospitalHighLight = TRUE;
		HandleBlitOfSectorLocatorIcon( HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y, 0, LOCATOR_COLOR_RED );
		fSkipSpeakersLocator = TRUE;
	}
	else if( fOldShowCambriaHospitalHighLight )
	{
		fOldShowCambriaHospitalHighLight = FALSE;
		fMapPanelDirty = TRUE;
	}

	// show other SAM sites
	if( fShowOtherSAMHighLight )
	{
		fOldShowOtherSAMHighLight = TRUE;
		HandleBlitOfSectorLocatorIcon(samList[SAM_SITE_ONE]->sectorId,   LOCATOR_COLOR_RED);
		HandleBlitOfSectorLocatorIcon(samList[SAM_SITE_THREE]->sectorId, LOCATOR_COLOR_RED);
		HandleBlitOfSectorLocatorIcon(samList[SAM_SITE_FOUR]->sectorId,  LOCATOR_COLOR_RED);
		fSkipSpeakersLocator = TRUE;
	}
	else if( fOldShowOtherSAMHighLight )
	{
		fOldShowOtherSAMHighLight = FALSE;
		fMapPanelDirty = TRUE;
	}

	// show Estoni site
	if( fShowEstoniRefuelHighLight )
	{
		fOldShowEstoniRefuelHighLight = TRUE;
		INT16 const sec = g_refuel_site[ESTONI_REFUELING_SITE].sector;
		HandleBlitOfSectorLocatorIcon(GET_X_FROM_STRATEGIC_INDEX(sec), GET_Y_FROM_STRATEGIC_INDEX(sec), 0, LOCATOR_COLOR_RED);
		fSkipSpeakersLocator = TRUE;
	}
	else if( fOldShowEstoniRefuelHighLight )
	{
		fOldShowEstoniRefuelHighLight = FALSE;
		fMapPanelDirty = TRUE;
	}

	// don't show sector locator over the speaker's sector if he is talking about another sector - it's confusing
	if ( !fSkipSpeakersLocator )
	{
		switch( gubBlitSectorLocatorCode )
		{
			case LOCATOR_COLOR_RED: // normal one used for mines (will now be overriden with yellow)
				HandleBlitOfSectorLocatorIcon(gsSectorLocator.x, gsSectorLocator.y, 0, LOCATOR_COLOR_RED);
				break;
			case LOCATOR_COLOR_YELLOW: // used for all other dialogues
				HandleBlitOfSectorLocatorIcon(gsSectorLocator.y, gsSectorLocator.z, 0, LOCATOR_COLOR_YELLOW);
				break;
		}
	}
}

void HandleHelicopterOnGround(BOOLEAN handleGraphicToo)
{
	// No worries if underground
	if (gWorldSector.z != 0) return;

	for (UINT8 site = 0; site != NUMBER_OF_REFUEL_SITES; ++site)
	{
		RefuelSite const& r = g_refuel_site[site];
		// Is this refueling site sector the loaded sector?
		if (gWorldSector.AsStrategicIndex() != r.sector) continue;

		// YES, so find out if the chopper is landed here
		if (IsHelicopterOnGroundAtRefuelingSite(r))
		{
			if(handleGraphicToo)
			{
				AddHelicopterToMaps(true, r);
			}
			// ATE: Add Skyrider too
			// ATE: only if hired
			if (iHelicopterVehicleId != -1)
			{
				MERCPROFILESTRUCT& p = GetProfile(SKYRIDER);
				p.sSectorX = gWorldSector.x;
				p.sSectorY = gWorldSector.y;
			}
		}
		else
		{
			if(handleGraphicToo)
			{
				AddHelicopterToMaps(false, r);
			}
			// ATE: Remove Skyrider
			if (iHelicopterVehicleId != -1)
			{
				MERCPROFILESTRUCT& p = GetProfile(SKYRIDER);
				p.sSectorX = 0;
				p.sSectorY = 0;

				// See if we can find him and remove him if so
				// ATE: Don't do this if buddy is on our team!
				SOLDIERTYPE* const s = FindSoldierByProfileID(SKYRIDER);
				if (s && s->bTeam != OUR_TEAM) TacticalRemoveSoldier(*s);
			}
		}
		if(handleGraphicToo)
		{
			InvalidateWorldRedundency();
		}
		break;
	}
}

static bool IsHelicopterOnGroundAtRefuelingSite(RefuelSite const& r)
{
	if (fHelicopterDestroyed)  return false;
	if (fHelicopterIsAirBorne) return false;

	// if we haven't even met SkyRider
	if (!fSkyRiderSetUp)
	{ // Then it's always at Drassen
		return &r == &g_refuel_site[DRASSEN_REFUELING_SITE];
	}

	// on the ground, but is it at this site or at another one?
	VEHICLETYPE const& v = GetHelicopter();
	return CALCULATE_STRATEGIC_INDEX(v.sSectorX, v.sSectorY) == r.sector;
}

static void HeliCrashSoundStopCallback(void* pData)
{
	SkyriderDestroyed( );
}

static BOOLEAN HandleSAMSiteAttackOfHelicopterInSector(INT16 sSectorX, INT16 sSectorY)
{
	INT8 bSamSiteID = -1;
	INT8 bSAMCondition;
	UINT8 ubChance;

	// if this sector is in friendly airspace, we're safe
	if (!StrategicMap[CALCULATE_STRATEGIC_INDEX(sSectorX, sSectorY)].fEnemyAirControlled)
	{
		// no problem, friendly airspace
		return( FALSE );
	}

	// which SAM controls this sector?
	bSamSiteID = GCM->getControllingSamSite(SECTOR(sSectorX, sSectorY));

	// if none of them (-1 means the sector is not covered by a SAM)
	if (bSamSiteID < 0)
	{
		return FALSE;
	}

	// get the condition of that SAM site (NOTE: SAM IDs are 0-3)
	Assert(bSamSiteID < NUMBER_OF_SAMS );
	UINT8 ubSAMSectorID = GCM->getSamSites()[bSamSiteID]->sectorId;
	bSAMCondition = StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX(ubSAMSectorID) ].bSAMCondition;

	// if the SAM site is too damaged to be a threat
	if( bSAMCondition < MIN_CONDITION_FOR_SAM_SITE_TO_WORK )
	{
		// no problem, SAM site not working
		return( FALSE );
	}

	// Hostile airspace controlled by a working SAM site, so SAM site fires a SAM at Skyrider!!!
	// calc chance that chopper will be shot down
	ubChance = bSAMCondition;

	// there's a fair chance of a miss even if the SAM site is in perfect working order
	if (ubChance > MAX_SAM_SITE_ACCURACY)
	{
		ubChance = MAX_SAM_SITE_ACCURACY;
	}

	if( PreRandom( 100 ) < ubChance)
	{
		// another hit!
		gubHelicopterHitsTaken++;

		// Took a hit!  Pause time so player can reconsider
		StopTimeCompression();

		// first hit?
		if ( gubHelicopterHitsTaken == 1 )
		{
			HeliCharacterDialogue(HELI_TOOK_MINOR_DAMAGE);
		}
		// second hit?
		else if ( gubHelicopterHitsTaken == 2 )
		{
			// going back to base (no choice, dialogue says so)
			HeliCharacterDialogue(HELI_TOOK_MAJOR_DAMAGE);
			MakeHeliReturnToBase();
		}
		// third hit!
		else
		{
			// Important: Skyrider must still be alive when he talks, so must do this before heli is destroyed!
			HeliCharacterDialogue(HELI_GOING_DOWN);

			// everyone die die die
			// play sound
			if (PlayJA2StreamingSampleFromFile(STSOUNDSDIR "/blah2.wav", HIGHVOLUME, 1, MIDDLEPAN, HeliCrashSoundStopCallback) == SOUND_ERROR)
			{
				// Destroy here if we cannot play streamed sound sample....
				SkyriderDestroyed( );
			}
			else
			{
				// otherwise it's handled in the callback
				// remove any arrival events for the helicopter's group
				DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, GetHelicopter().ubMovementGroup);
			}

			// special return code indicating heli was destroyed
			return( TRUE );
		}
	}
	// still flying
	return( FALSE );
}

// check if helicopter can take off?
BOOLEAN CanHelicopterTakeOff( void )
{
	// if it's already in the air
	if (fHelicopterIsAirBorne) return TRUE;

	VEHICLETYPE const& v = GetHelicopter();
	// grab location
	INT16 const sHelicopterSector = CALCULATE_STRATEGIC_INDEX(v.sSectorX, v.sSectorY);
	// if it's not in enemy control, we can take off
	if (!StrategicMap[sHelicopterSector].fEnemyControlled)
	{
		return( TRUE );
	}
	return( FALSE );
}

static void AddHeliPiece(INT16 const sGridNo, UINT16 const sOStruct)
{
	if (IndexExistsInStructLayer(sGridNo, sOStruct)) return;
	AddStructToTail(sGridNo, sOStruct);
}

static void AddHelicopterToMaps(bool const add, RefuelSite const& r)
{
	GridNo const grid_no = r.grid_no;
	INT16  const ostruct = r.heli_ostruct;

	// are we adding or taking away
	if (add)
	{
		AddHeliPiece(grid_no,       ostruct    );
		AddHeliPiece(grid_no,       ostruct + 1);
		AddHeliPiece(grid_no - 800, ostruct + 2);
		AddHeliPiece(grid_no,       ostruct + 3);
		AddHeliPiece(grid_no,       ostruct + 4);
		AddHeliPiece(grid_no - 800, ostruct + 5);

		// ATE: If any mercs here, bump them off!
		INT16	sCentreGridX;
		INT16 sCentreGridY;
		ConvertGridNoToXY(grid_no, &sCentreGridX, &sCentreGridY);

		for (INT16 y = sCentreGridY - 5; y < sCentreGridY + 5; ++y)
		{
			for (INT16 x = sCentreGridX - 5; x < sCentreGridX + 5; ++x)
			{
				BumpAnyExistingMerc(MAPROWCOLTOPOS(y, x));
			}
		}
	}
	else
	{
		// remove from the world
		RemoveStruct(grid_no,       ostruct    );
		RemoveStruct(grid_no,       ostruct + 1);
		RemoveStruct(grid_no - 800, ostruct + 2);
		RemoveStruct(grid_no,       ostruct + 3);
		RemoveStruct(grid_no,       ostruct + 4);
		RemoveStruct(grid_no - 800, ostruct + 5);
	}
	InvalidateWorldRedundency();
	SetRenderFlags(RENDER_FLAG_FULL);
}

bool IsSkyriderFlyingInSector(INT16 const x, INT16 const y)
{
	// up and about?
	if (iHelicopterVehicleId == -1) return false;
	if (!CanHelicopterFly())        return false;
	if (!fHelicopterIsAirBorne)     return false;
	VEHICLETYPE const& v = GetHelicopter();
	// the right sector?
	return x == v.sSectorX && y == v.sSectorY;
}

bool IsGroupTheHelicopterGroup(GROUP const& g)
{
	if (iHelicopterVehicleId == -1) return false;
	VEHICLETYPE const& v = GetHelicopter();
	return
		v.ubMovementGroup != 0 &&
		v.ubMovementGroup == g.ubGroupID;
}

INT16 GetNumSafeSectorsInPath()
{
	if (!CanHelicopterFly()) return 0;

	VEHICLETYPE const& v      = GetHelicopter();
	INT32       const  sector = CALCULATE_STRATEGIC_INDEX(v.sSectorX, v.sSectorY);
	GROUP*      const  g      = GetGroup(v.ubMovementGroup);
	UINT32             n      = 0;

	if (PathSt const* i = v.pMercPath)
	{
		/* First node: Skip it if that's the sector the chopper is currently in, AND
		 * we're NOT gonna be changing directions (not actually performed until
		 * waypoints are rebuilt AFTER plotting is done) */
		if ((INT32)i->uiSectorId == sector &&
				i->pNext &&
				!GroupBetweenSectorsAndSectorXYIsInDifferentDirection(g, GET_X_FROM_STRATEGIC_INDEX(i->pNext->uiSectorId), GET_Y_FROM_STRATEGIC_INDEX(i->pNext->uiSectorId)))
		{
			i = i->pNext;
		}

		for (; i; i = i->pNext)
		{
			if (StrategicMap[i->uiSectorId].fEnemyAirControlled) continue;
			++n;
		}
	}

	if (PathSt const* i = pTempHelicopterPath)
	{
		/* First node: Skip it if that's the sector the chopper is currently in, AND
		 * we're NOT gonna be changing directions (not actually performed until
		 * waypoints are rebuilt AFTER plotting is done) OR if the chopper has a
		 * mercpath, in which case this a continuation of it that would count the
		 * sector twice */
		if ((
					(INT32)i->uiSectorId == sector &&
					i->pNext &&
					!GroupBetweenSectorsAndSectorXYIsInDifferentDirection(g, GET_X_FROM_STRATEGIC_INDEX(i->pNext->uiSectorId), GET_Y_FROM_STRATEGIC_INDEX(i->pNext->uiSectorId))
				) ||
				GetLengthOfPath(v.pMercPath) > 0)
		{
			i = i->pNext;
		}

		for (; i; i = i->pNext)
		{
			if (StrategicMap[i->uiSectorId].fEnemyAirControlled) continue;
			++n;
		}
	}
	return n;
}

INT16 GetNumUnSafeSectorsInPath( void )
{
	// get the last sector value in the helictoper's path
	UINT32 uiLocation = 0;
	UINT32  uiCount = 0;

	// if the heli is on the move, what is the distance it will move..the length of the merc path, less the first node
	if (!CanHelicopterFly()) return 0;

	VEHICLETYPE const& v = GetHelicopter();
	// may need to skip the sector the chopper is currently in
	INT32 const iHeliSector = CALCULATE_STRATEGIC_INDEX(v.sSectorX, v.sSectorY);

	// get chopper's group ptr
	GROUP* const pGroup = GetGroup(v.ubMovementGroup);

	const PathSt* pNode = v.pMercPath;

	// any path yet?
	if( pNode != NULL )
	{
		// first node: skip it if that's the sector the chopper is currently in, AND
		// we're NOT gonna be changing directions (not actually performed until waypoints are rebuilt AFTER plotting is done)
		if ( ( ( INT32 ) pNode->uiSectorId == iHeliSector ) && ( pNode->pNext != NULL ) &&
				!GroupBetweenSectorsAndSectorXYIsInDifferentDirection( pGroup, ( UINT8 ) GET_X_FROM_STRATEGIC_INDEX( pNode->pNext->uiSectorId ), ( UINT8 ) GET_Y_FROM_STRATEGIC_INDEX( pNode->pNext->uiSectorId ) ) )
		{
			pNode = pNode->pNext;
		}

		while( pNode)
		{
			uiLocation = pNode -> uiSectorId;

			if ( StrategicMap[ uiLocation ].fEnemyAirControlled )
			{
				uiCount++;
			}

			pNode = pNode ->pNext;
		}
	}

	pNode = pTempHelicopterPath;
	// any path yet?
	if( pNode != NULL )
	{
		// first node: skip it if that's the sector the chopper is currently in, AND
		// we're NOT gonna be changing directions (not actually performed until waypoints are rebuilt AFTER plotting is done)
		// OR if the chopper has a mercpath, in which case this a continuation of it that would count the sector twice
		if ( ( ( ( INT32 ) pNode->uiSectorId == iHeliSector ) && ( pNode->pNext != NULL ) &&
				!GroupBetweenSectorsAndSectorXYIsInDifferentDirection( pGroup, ( UINT8 ) GET_X_FROM_STRATEGIC_INDEX( pNode->pNext->uiSectorId ), ( UINT8 ) GET_Y_FROM_STRATEGIC_INDEX( pNode->pNext->uiSectorId ) ) ) ||
				GetLengthOfPath(v.pMercPath) > 0)
		{
			pNode = pNode->pNext;
		}

		while( pNode)
		{
			uiLocation = pNode -> uiSectorId;

			if ( StrategicMap[ uiLocation ].fEnemyAirControlled )
			{
				uiCount++;
			}

			pNode = pNode ->pNext;
		}
	}
	return( (INT16)uiCount );
}

static void PaySkyriderBill(void)
{
	// if we owe anything for the trip
	if ( iTotalAccumulatedCostByPlayer > 0 )
	{
		// if player can afford to pay the Skyrider bill
		if( LaptopSaveInfo.iCurrentBalance >= iTotalAccumulatedCostByPlayer )
		{
			// no problem, pay the man
			// add the transaction
			AddTransactionToPlayersBook( PAYMENT_TO_NPC, SKYRIDER, GetWorldTotalMin( ), -iTotalAccumulatedCostByPlayer );
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, st_format_printf(pSkyriderText[ 0 ], iTotalAccumulatedCostByPlayer) );
		}
		else
		{
			// money owed
			if( LaptopSaveInfo.iCurrentBalance > 0 )
			{
				ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, st_format_printf(pSkyriderText[ 0 ], LaptopSaveInfo.iCurrentBalance) );
				gMercProfiles[ SKYRIDER ].iBalance = LaptopSaveInfo.iCurrentBalance - iTotalAccumulatedCostByPlayer;
				// add the transaction
				AddTransactionToPlayersBook( PAYMENT_TO_NPC, SKYRIDER, GetWorldTotalMin( ), -LaptopSaveInfo.iCurrentBalance );
			}
			else
			{
				gMercProfiles[ SKYRIDER ].iBalance = - iTotalAccumulatedCostByPlayer;
			}

			HeliCharacterDialogue(OWED_MONEY_TO_SKYRIDER);
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, st_format_printf(pSkyriderText[ 1 ], -gMercProfiles[ SKYRIDER ].iBalance) );

			// kick everyone out! (we know we're in a safe sector if we're paying)
			MoveAllInHelicopterToFootMovementGroup( );
			MakeHeliReturnToBase();
		}
		iTotalAccumulatedCostByPlayer = 0;
	}
}

void PayOffSkyriderDebtIfAny( )
{
	INT32 iAmountOwed;
	INT32 iPayAmount;

	iAmountOwed = - gMercProfiles[ SKYRIDER ].iBalance;

	// if we owe him anything, and have any money
	if ( ( iAmountOwed > 0 ) && ( LaptopSaveInfo.iCurrentBalance > 0 ) )
	{
		iPayAmount = MIN( iAmountOwed, LaptopSaveInfo.iCurrentBalance );

		// pay the man what we can
		gMercProfiles[ SKYRIDER ].iBalance += iPayAmount;
		// add the transaction
		AddTransactionToPlayersBook( PAYMENT_TO_NPC, SKYRIDER, GetWorldTotalMin( ), -iPayAmount );
		// tell player
		ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, st_format_printf(pSkyriderText[ 0 ], iPayAmount) );
		// now whaddawe owe?
		iAmountOwed = - gMercProfiles[ SKYRIDER ].iBalance;

		// if it wasn't enough
		if ( iAmountOwed > 0 )
		{
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, st_format_printf(pSkyriderText[ 1 ], iAmountOwed) );
			HeliCharacterDialogue(OWED_MONEY_TO_SKYRIDER);
		}
	}
}

static void MakeHeliReturnToBase(void)
{
	VEHICLETYPE& v = GetHelicopter();
	INT16 sectorID;

	sectorID = CALCULATE_STRATEGIC_INDEX(v.sSectorX, v.sSectorY);
	// if already at a refueling point
	if (IsRefuelAvailableInSector(sectorID))
	{
		LandHelicopter();
	}
	else
	{
		// choose destination (closest refueling sector)
		RefuelSite const& refuel_site = NearestRefuelPoint(true);

		ClearStrategicPathList(v.pMercPath, v.ubMovementGroup);
		GROUP& g = *GetGroup(v.ubMovementGroup);
		v.pMercPath = BuildAStrategicPath( sectorID, refuel_site.sector, g, FALSE);
		RebuildWayPointsForGroupPath(v.pMercPath, g);

		fHeliReturnStraightToBase = TRUE;
		fHoveringHelicopter       = FALSE;
	}
	// stop time compression if it's on so player can digest this
	StopTimeCompression();
}

bool SoldierAboardAirborneHeli(SOLDIERTYPE const& s)
{
	return InHelicopter(s) && fHelicopterIsAirBorne;
}
