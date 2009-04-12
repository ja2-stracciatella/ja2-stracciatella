#include "Font_Control.h"
#include "MapScreen.h"
#include "Map_Screen_Helicopter.h"
#include "LaptopSave.h"
#include "MessageBoxScreen.h"
#include "Vehicles.h"
#include "Finances.h"
#include "Quests.h"
#include "Game_Clock.h"
#include "Queen_Command.h"
#include "Strategic_Pathing.h"
#include "Random.h"
#include "Game_Event_Hook.h"
#include "Dialogue_Control.h"
#include "Message.h"
#include "Strategic_Movement.h"
#include "Soldier_Profile.h"
#include "Assignments.h"
#include "PreBattle_Interface.h"
#include "StrategicMap.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "TileDat.h"
#include "Map_Screen_Interface.h"
#include "Text.h"
#include "Squads.h"
#include "Player_Command.h"
#include "Sound_Control.h"
#include "Meanwhile.h"
#include "Map_Screen_Interface_Border.h"
#include "Strategic_Event_Handler.h"
#include "Overhead.h"
#include "Soldier_Create.h"
#include "RenderWorld.h"
#include "SoundMan.h"
#include "Isometric_Utils.h"
#include "Scheduling.h"
#include "Debug.h"


// the amounts of time to wait for hover stuff
#define TIME_DELAY_FOR_HOVER_WAIT						10		// minutes
#define TIME_DELAY_FOR_HOVER_WAIT_TOO_LONG	20		// mintues


#define MIN_DAYS_BETWEEN_SKYRIDER_MONOLOGUES 1

// maximum chance out of a hundred per unsafe sector that a SAM site in decent working condition will hit Skyrider
#define MAX_SAM_SITE_ACCURACY		33


// current temp path for dest char
extern PathSt* pTempHelicopterPath;

extern UINT8 ubSAMControlledSectors[ MAP_WORLD_X ][ MAP_WORLD_Y ];


// whether helicopted variables have been set up
BOOLEAN fSkyRiderSetUp = FALSE;

// plotting for a helicopter
BOOLEAN fPlotForHelicopter = FALSE;

// helicopter vehicle id
INT32 iHelicopterVehicleId = -1;

// total owed to player
INT32 iTotalAccumulatedCostByPlayer = 0;

#ifdef JA2TESTVERSION
BOOLEAN	fSAMSitesDisabledFromAttackingPlayer = FALSE;
#endif

// helicopter destroyed
BOOLEAN fHelicopterDestroyed = FALSE;

// list of sector locations where SkyRider can be refueled
UINT8 ubRefuelList[ NUMBER_OF_REFUEL_SITES ][ 2 ] =
{
	{ 13, 2 },		// Drassen airport
	{  6, 9 },		// Estoni
};


INT16 sRefuelStartGridNo[ NUMBER_OF_REFUEL_SITES ] ={
	 9001, // drassen
	13067, // estoni
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

	const VEHICLETYPE* const v = GetHelicopter();
	pSoldier->sSectorX = v->sSectorX;
	pSoldier->sSectorY = v->sSectorY;
	pSoldier->bSectorZ = 0;

	// reset between sectors
	pSoldier->fBetweenSectors = FALSE;


	// remove from the vehicle
	return( TakeSoldierOutOfVehicle( pSoldier ) );
}


static BOOLEAN CheckForArrivalAtRefuelPoint(void);
static BOOLEAN DoesSkyriderNoticeEnemiesInSector(UINT8 ubNumEnemies);
static BOOLEAN EndOfHelicoptersPath(void);
static INT32 GetCostOfPassageForHelicopter(INT16 sX, INT16 sY);
static BOOLEAN HandleSAMSiteAttackOfHelicopterInSector(INT16 sSectorX, INT16 sSectorY);
static void HeliCharacterDialogue(UINT16 usQuoteNum);
static void PaySkyriderBill(void);
static void ReFuelHelicopter(void);
static void StartHoverTime(void);


BOOLEAN HandleHeliEnteringSector( INT16 sX, INT16 sY )
{
	UINT8 ubNumEnemies;


	// check for SAM attack upon the chopper.  If it's destroyed by the attack, do nothing else here
	if (HandleSAMSiteAttackOfHelicopterInSector(sX, sY))
	{
		// destroyed
		return( TRUE );
	}


	// count how many enemies are camped there or passing through
	ubNumEnemies = NumEnemiesInSector( sX, sY );

	// any baddies?
	if( ubNumEnemies > 0 )
	{
		// if the player didn't know about these prior to the chopper's arrival
		if( WhatPlayerKnowsAboutEnemiesInSector( sX, sY ) == KNOWS_NOTHING )
		{
			// but Skyrider notices them
			if (DoesSkyriderNoticeEnemiesInSector(ubNumEnemies))
			{
				// if just passing through (different quotes are used below if it's his final destination)
				if( !EndOfHelicoptersPath( ) )
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
				SectorInfo[ SECTOR( sX, sY ) ].uiFlags |= SF_SKYRIDER_NOTICED_ENEMIES_HERE;
			}
		}
	}


	// player pays for travel if Skyrider is NOT returning to base (even if empty while scouting/going for pickup)
	if (!fHeliReturnStraightToBase)
	{
		// charge cost for flying another sector
		iTotalAccumulatedCostByPlayer += GetCostOfPassageForHelicopter( sX, sY );
	}

	// check if heli has any real path left
	if( EndOfHelicoptersPath( ) )
	{
		// start hovering
		StartHoverTime( );

		// if sector is safe, or Skyrider MUST land anyway (returning to base)
		if ( ( ubNumEnemies == 0 ) || fHeliReturnStraightToBase )
		{
			// if he has passengers, or he's not going straight to base, tell player he's arrived
			// (i.e. don't say anything nor stop time compression if he's empty and just returning to base)
			if ( ( GetNumberOfPassengersInHelicopter() > 0 ) || !fHeliReturnStraightToBase )
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

		if( CheckForArrivalAtRefuelPoint( ) )
		{
			ReFuelHelicopter( );
		}
	}

	return( FALSE );
}


static INT32 FindLocationOfClosestRefuelSite(BOOLEAN fMustBeAvailable);


static INT32 LocationOfNearestRefuelPoint(BOOLEAN fNotifyPlayerIfNoSafeLZ)
{
	INT32 iClosestLocation = -1;

	// try to find one, any one under the players control
	iClosestLocation = FindLocationOfClosestRefuelSite( TRUE );

	// no go?...then find
	if( iClosestLocation == -1 )
	{
		if( fNotifyPlayerIfNoSafeLZ )
		{
			// no refueling sites available, might wanna warn player about this
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pHelicopterEtaStrings[ 5 ] );
		}

		// find the closest location regardless
		iClosestLocation = FindLocationOfClosestRefuelSite( FALSE );
	}

	// always returns a valid refuel point, picking a hostile one if unavoidable
	Assert( iClosestLocation != -1 );

	return( iClosestLocation );
}


// find the location sector of closest refuel point for heli..and the criteria if the sector must be under the players control
static INT32 FindLocationOfClosestRefuelSite(BOOLEAN fMustBeAvailable)
{
	INT32 iShortestDistance = 9999;
	INT32 iCounter = 0;
	INT32 iClosestLocation = -1;

	// find shortest distance to refuel site
	for( iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++ )
	{
		// if this refuelling site is available
		if (fRefuelingSiteAvailable[iCounter] || !fMustBeAvailable)
		{
			// find if sector is under control, find distance from heli to it
			const VEHICLETYPE* const v = GetHelicopter();
			const INT32 iDistance = FindStratPath(CALCULATE_STRATEGIC_INDEX(v->sSectorX , v->sSectorY), CALCULATE_STRATEGIC_INDEX(ubRefuelList[iCounter][0], ubRefuelList[iCounter][1]), GetGroup(v->ubMovementGroup), FALSE);

			if( iDistance < iShortestDistance )
			{
				// shorter, copy over
				iShortestDistance = iDistance;
				iClosestLocation = iCounter;
			}
		}
	}

	// return the location
	return( iClosestLocation );
}


// How far to nearest refuel point from this sector?
static INT32 DistanceToNearestRefuelPoint(VEHICLETYPE const& heli)
{
	// Don't notify player during these checks!
	INT32 const closest_location = LocationOfNearestRefuelPoint(FALSE);
	INT16 const start            = CALCULATE_STRATEGIC_INDEX(heli.sSectorX, heli.sSectorY);
	INT16 const dest             = CALCULATE_STRATEGIC_INDEX(ubRefuelList[closest_location][0], ubRefuelList[closest_location][1]);
	INT32 const distance         = FindStratPath(start, dest, GetGroup(heli.ubMovementGroup), FALSE);
	return distance;
}


static void LandHelicopter(void);


static void ReFuelHelicopter(void)
{
	// land, pay the man, and refuel
	LandHelicopter( );
}


// how much will it cost for helicopter to travel through this sector?
static INT32 GetCostOfPassageForHelicopter(INT16 sX, INT16 sY)
{
	// check if sector is air controlled or not, if so, then normal cost, otherwise increase the cost
	INT32 iCost = 0;

	// if they don't control it
	if (!StrategicMap[CALCULATE_STRATEGIC_INDEX(sX, sY)].fEnemyAirControlled)
	{
		iCost = COST_AIRSPACE_SAFE;
	}
	else
	{
		iCost = COST_AIRSPACE_UNSAFE;
	}

	return( iCost );
}


// helicopter shot down, kill all on board
static void SkyriderDestroyed(void)
{
	// remove any arrival events for the helicopter's group
	VEHICLETYPE* const v = GetHelicopter();
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, v->ubMovementGroup);

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
	if( gMercProfiles[ SKYRIDER ].iBalance < 0 )
	{
		return( FALSE );
	}

	// Drassen too disloyal to wanna help player?
	if ( CheckFact( FACT_LOYALTY_LOW, SKYRIDER ) )
	{
		return ( FALSE );
	}

	return( TRUE );
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

	HandleHelicopterOnGroundGraphic( );
	HandleHelicopterOnGroundSkyriderProfile( );

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

	HandleHelicopterOnGroundGraphic( );
	HandleHelicopterOnGroundSkyriderProfile( );
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


static void MakeHeliReturnToBase(void);


void HandleHeliHoverTooLong( void )
{
	// reset hover time
	uiStartHoverTime = 0;

	if (!fHoveringHelicopter) return;

	// hovered too long, inform player heli is returning to base
	HeliCharacterDialogue(RETURN_TO_BASE);

	const VEHICLETYPE* const v = GetHelicopter();
	// If the sector is safe
	if (NumEnemiesInSector(v->sSectorX, v->sSectorY) == 0)
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

	const PathSt* pNode = GetHelicopter()->pMercPath;

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


static BOOLEAN CheckForArrivalAtRefuelPoint(void)
{
	const VEHICLETYPE* const v = GetHelicopter();
	// check if this is our final destination
	if (GetLengthOfPath(v->pMercPath) > 0) return FALSE;

	// check if we're at a refuel site
	if (DistanceToNearestRefuelPoint(*v) > 0)
	{
		// not at a refuel point
		return( FALSE );
	}

	// we are at a refuel site
	return( TRUE );
}



void SetUpHelicopterForMovement( void )
{
	// check if helicopter vehicle has a mvt group, if not, assign one in this sector
	VEHICLETYPE* const v = GetHelicopter();

	// if no group, create one for vehicle
	if (v->ubMovementGroup == 0)
	{
		// get the vehicle a mvt group
		GROUP* const g = CreateNewVehicleGroupDepartingFromSector(v->sSectorX, v->sSectorY);
		v->ubMovementGroup = g->ubGroupID;

		// add everyone in vehicle to this mvt group
		CFOR_ALL_PASSENGERS(v, i)
		{
			AddPlayerToGroup(g, *i);
		}
	}
}


static void SkyriderDialogue(UINT16 const quote)
{
	CharacterDialogue(SKYRIDER, quote, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE], DIALOGUE_EXTERNAL_NPC_UI, FALSE);
}


enum SkyriderMonologueEvent
{
	SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE = 0,
	SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES,
	SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL,
	SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL
};


static void HandleSkyRiderMonologueEvent(SkyriderMonologueEvent, UINT32 uiSpecialCode);


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


INT32 GetNumberOfPassengersInHelicopter( void )
{
	return GetNumberInVehicle(GetHelicopter());
}


BOOLEAN IsRefuelSiteInSector( INT16 sMapX, INT16 sMapY )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++ )
	{
		if ( ( ubRefuelList[ iCounter ][ 0 ] == sMapX ) && ( ubRefuelList[ iCounter ][ 1 ] == sMapY ) )
		{
			return(TRUE);
		}
	}

	return(FALSE);
}


void UpdateRefuelSiteAvailability( void )
{
	INT32 iCounter = 0;

	// Generally, only Drassen is initially available for refuelling
	// Estoni must first be captured (although player may already have it when he gets Skyrider!)

	for( iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++ )
	{
		// if enemy controlled sector (ground OR air, don't want to fly into enemy air territory)
		if (StrategicMap[CALCULATE_STRATEGIC_INDEX(ubRefuelList[iCounter][0], ubRefuelList[iCounter][1])].fEnemyControlled    ||
				StrategicMap[CALCULATE_STRATEGIC_INDEX(ubRefuelList[iCounter][0], ubRefuelList[iCounter][1])].fEnemyAirControlled ||
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
	const VEHICLETYPE* const v = GetHelicopter();
	CFOR_ALL_PASSENGERS(v, i)
	{
		SOLDIERTYPE* const pSoldier = *i;

		// better really be in there!
		Assert ( pSoldier->bAssignment == VEHICLE );
		Assert ( pSoldier->iVehicleId == iHelicopterVehicleId );

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


static void HandleSkyRiderMonologueAboutCambriaHospital(UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutDrassenSAMSite(UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutEstoniRefuel(UINT32 uiSpecialCode);
static void HandleSkyRiderMonologueAboutOtherSAMSites(UINT32 uiSpecialCode);


// Skyrider monlogue events for mapscreen
static void HandleSkyRiderMonologueEvent(SkyriderMonologueEvent const uiEventCode, UINT32 const uiSpecialCode)
{
	// will handle the skyrider monologue about where the SAM sites are and what not

	TurnOnAirSpaceMode();

	switch( uiEventCode )
	{
		case( SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE ):
			SetExternMapscreenSpeechPanelXY( DEFAULT_EXTERN_PANEL_X_POS, 117 );
			HandleSkyRiderMonologueAboutDrassenSAMSite( uiSpecialCode );
			break;
		case SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL:
			SetExternMapscreenSpeechPanelXY( DEFAULT_EXTERN_PANEL_X_POS, 172 );
			HandleSkyRiderMonologueAboutCambriaHospital( uiSpecialCode );
			break;
		case( SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES ):
			SetExternMapscreenSpeechPanelXY( 335, DEFAULT_EXTERN_PANEL_Y_POS );
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

			if (!SAMSitesUnderPlayerControl(SAM_2_X, SAM_2_Y))
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


void HandleAnimationOfSectors( void )
{
	BOOLEAN fSkipSpeakersLocator = FALSE;
	// these don't need to be saved, they merely turn off the highlights after they stop flashing
	static BOOLEAN fOldShowDrassenSAMHighLight = FALSE;
	static BOOLEAN fOldShowCambriaHospitalHighLight = FALSE;
	static BOOLEAN fOldShowEstoniRefuelHighLight = FALSE;
	static BOOLEAN fOldShowOtherSAMHighLight = FALSE;

	// find out which mode we are in and animate for that mode

	// Drassen SAM site
	if( fShowDrassenSAMHighLight )
	{
		fOldShowDrassenSAMHighLight = TRUE;
		// Drassen's SAM site is #3
		HandleBlitOfSectorLocatorIcon( SAM_2_X, SAM_2_Y, 0, LOCATOR_COLOR_RED );
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
		HandleBlitOfSectorLocatorIcon( SAM_1_X, SAM_1_Y, 0, LOCATOR_COLOR_RED );
		HandleBlitOfSectorLocatorIcon( SAM_3_X, SAM_3_Y, 0, LOCATOR_COLOR_RED );
		HandleBlitOfSectorLocatorIcon( SAM_4_X, SAM_4_Y, 0, LOCATOR_COLOR_RED );
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
		HandleBlitOfSectorLocatorIcon( ubRefuelList[ ESTONI_REFUELING_SITE ][ 0 ], ubRefuelList[ ESTONI_REFUELING_SITE ][ 1 ], 0, LOCATOR_COLOR_RED );
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
			case LOCATOR_COLOR_RED:		 // normal one used for mines (will now be overriden with yellow)
				HandleBlitOfSectorLocatorIcon( gsSectorLocatorX, gsSectorLocatorY, 0, LOCATOR_COLOR_RED );
				break;
			case LOCATOR_COLOR_YELLOW: // used for all other dialogues
				HandleBlitOfSectorLocatorIcon( gsSectorLocatorX, gsSectorLocatorY, 0, LOCATOR_COLOR_YELLOW );
				break;
		}
	}
}


static void AddHelicopterToMaps(BOOLEAN fAdd, UINT8 ubSite);
static BOOLEAN IsHelicopterOnGroundAtRefuelingSite(UINT8 ubRefuelingSite);


void HandleHelicopterOnGroundGraphic( void )
{
	UINT8					ubSite = 0;

	// no worries if underground
	if (gbWorldSectorZ != 0 )
	{
		return;
	}

	for( ubSite = 0; ubSite < NUMBER_OF_REFUEL_SITES; ubSite++ )
	{
		// is this refueling site sector the loaded sector ?
		if ( ( ubRefuelList[ ubSite ][ 0 ] == gWorldSectorX ) && ( ubRefuelList[ ubSite ][ 1 ] == gWorldSectorY ) )
		{
			// YES, so find out if the chopper is landed here
			if ( IsHelicopterOnGroundAtRefuelingSite( ubSite ) )
			{
				// Add....
				AddHelicopterToMaps( TRUE, ubSite );
				// ATE: Add skyridder too
				// ATE: only if hired......
				if (iHelicopterVehicleId != -1)
				{
					gMercProfiles[ SKYRIDER ].sSectorX = gWorldSectorX;
					gMercProfiles[ SKYRIDER ].sSectorY = gWorldSectorY;
				}
			}
			else
			{
				AddHelicopterToMaps( FALSE, ubSite );
				// ATE: Remove skyridder....
				if (iHelicopterVehicleId != -1)
				{
					gMercProfiles[ SKYRIDER ].sSectorX = 0;
					gMercProfiles[ SKYRIDER ].sSectorY = 0;

					// see if we can find him and remove him if so....
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(SKYRIDER);

          // ATE: Don't do this if buddy is on our team!
					if ( pSoldier != NULL && pSoldier->bTeam != gbPlayerNum )
					{
						TacticalRemoveSoldier(pSoldier);
					}
				}
			}

			// Invalidate rendering
			InvalidateWorldRedundency( );

			// can't be 2 places at once!
			break;
		}
	}
}


void HandleHelicopterOnGroundSkyriderProfile( void )
{
	UINT8					ubSite = 0;

	// no worries if underground
	if (gbWorldSectorZ != 0 )
	{
		return;
	}

	for( ubSite = 0; ubSite < NUMBER_OF_REFUEL_SITES; ubSite++ )
	{
		// is this refueling site sector the loaded sector ?
		if ( ( ubRefuelList[ ubSite ][ 0 ] == gWorldSectorX ) && ( ubRefuelList[ ubSite ][ 1 ] == gWorldSectorY ) )
		{
			// YES, so find out if the chopper is landed here
			if ( IsHelicopterOnGroundAtRefuelingSite( ubSite ) )
			{
				// ATE: Add skyridder too
				// ATE: only if hired......
				if (iHelicopterVehicleId != -1)
				{
					gMercProfiles[ SKYRIDER ].sSectorX = gWorldSectorX;
					gMercProfiles[ SKYRIDER ].sSectorY = gWorldSectorY;
				}
			}
			else
			{
				// ATE: Remove skyridder....
				if (iHelicopterVehicleId != -1)
				{
					gMercProfiles[ SKYRIDER ].sSectorX = 0;
					gMercProfiles[ SKYRIDER ].sSectorY = 0;

					// see if we can find him and remove him if so....
					SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(SKYRIDER);

          // ATE: Don't do this if buddy is on our team!
					if ( pSoldier != NULL && pSoldier->bTeam != gbPlayerNum )
					{
						TacticalRemoveSoldier(pSoldier);
					}
				}
			}

			// can't be 2 places at once!
			break;
		}
	}
}


static BOOLEAN IsHelicopterOnGroundAtRefuelingSite(UINT8 ubRefuelingSite)
{
	if ( fHelicopterDestroyed )
	{
		return(FALSE);
	}

	if ( fHelicopterIsAirBorne )
	{
		return(FALSE);
	}

	// if we haven't even met SkyRider
	if ( !fSkyRiderSetUp )
	{
		// then it's always at Drassen
		if ( ubRefuelingSite == DRASSEN_REFUELING_SITE )
		{
			return(TRUE);
		}
		else
		{
			return(FALSE);
		}
	}

	const VEHICLETYPE* const v = GetHelicopter();
	// on the ground, but is it at this site or at another one?
	if (ubRefuelList[ubRefuelingSite][0] == v->sSectorX &&
			ubRefuelList[ubRefuelingSite][1] == v->sSectorY)
	{
		return(TRUE);
	}

	// not here
	return(FALSE);
}


/*
BOOLEAN WillAirRaidBeStopped( INT16 sSectorX, INT16 sSectorY )
{
	UINT8 ubSamNumber = 0;
	INT8 bSAMCondition;
	UINT8 ubChance;


	// if enemy controls this SAM site, then it can't stop an air raid
	if (StrategicMap[CALCULATE_STRATEGIC_INDEX(sSectorX, sSectorY)].fEnemyAirControlled)
	{
		return( FALSE );
	}

	// which SAM controls this sector?
	ubSamNumber = ubSAMControlledSectors[ sSectorX ][ sSectorY ];

	// if none of them
	if (ubSamNumber == 0)
	{
		return( FALSE);
	}

	// get the condition of that SAM site (NOTE: SAM #s are 1-4, but indexes are 0-3!!!)
	Assert( ubSamNumber <= NUMBER_OF_SAMS );
	bSAMCondition = StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( pSamList[ ubSamNumber - 1 ] ) ].bSAMCondition;

	// if it's too busted to work, then it can't stop an air raid
	if( bSAMCondition < MIN_CONDITION_FOR_SAM_SITE_TO_WORK )
	{
		// no problem, SAM site not working
		return( FALSE );
	}


	// Friendly airspace controlled by a working SAM site, so SAM site fires a SAM at air raid bomber

	// calc chance that chopper will be shot down
	ubChance = bSAMCondition;

	// there's a fair chance of a miss even if the SAM site is in perfect working order
	if (ubChance > MAX_SAM_SITE_ACCURACY)
	{
		ubChance = MAX_SAM_SITE_ACCURACY;
	}

	if( PreRandom( 100 ) < ubChance)
	{
		return( TRUE );
	}

	return( FALSE );
}
*/


static void HeliCrashSoundStopCallback(void* pData)
{
	SkyriderDestroyed( );
}


static BOOLEAN HandleSAMSiteAttackOfHelicopterInSector(INT16 sSectorX, INT16 sSectorY)
{
	UINT8 ubSamNumber = 0;
	INT8 bSAMCondition;
	UINT8 ubChance;

	// if this sector is in friendly airspace, we're safe
	if (!StrategicMap[CALCULATE_STRATEGIC_INDEX(sSectorX, sSectorY)].fEnemyAirControlled)
	{
		// no problem, friendly airspace
		return( FALSE );
	}

	// which SAM controls this sector?
	ubSamNumber = ubSAMControlledSectors[ sSectorX ][ sSectorY ];

	// if none of them
	if (ubSamNumber == 0)
	{
		return( FALSE);
	}

	// get the condition of that SAM site (NOTE: SAM #s are 1-4, but indexes are 0-3!!!)
	Assert( ubSamNumber <= NUMBER_OF_SAMS );
	bSAMCondition = StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( pSamList[ ubSamNumber - 1 ] ) ].bSAMCondition;

	// if the SAM site is too damaged to be a threat
	if( bSAMCondition < MIN_CONDITION_FOR_SAM_SITE_TO_WORK )
	{
		// no problem, SAM site not working
		return( FALSE );
	}

#ifdef JA2TESTVERSION
	if (fSAMSitesDisabledFromAttackingPlayer) return FALSE;
#endif
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
			if (PlayJA2StreamingSampleFromFile("stsounds/blah2.wav", HIGHVOLUME, 1, MIDDLEPAN, HeliCrashSoundStopCallback) == SOUND_ERROR)
      {
        // Destroy here if we cannot play streamed sound sample....
  			SkyriderDestroyed( );
      }
      else
      {
        // otherwise it's handled in the callback
	      // remove any arrival events for the helicopter's group
	      DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, GetHelicopter()->ubMovementGroup);
      }

			// special return code indicating heli was destroyed
			return( TRUE );
		}
	}

	// still flying
	return( FALSE );
}


// are we at the end of the path for the heli?
static BOOLEAN EndOfHelicoptersPath(void)
{
	const VEHICLETYPE* const v = GetHelicopter();
	return v->pMercPath == NULL || v->pMercPath->pNext == NULL;
}


// check if helicopter can take off?
BOOLEAN CanHelicopterTakeOff( void )
{
	// if it's already in the air
	if (fHelicopterIsAirBorne) return TRUE;

	const VEHICLETYPE* const v = GetHelicopter();
	// grab location
	const INT16 sHelicopterSector = v->sSectorX + v->sSectorY * MAP_WORLD_X;
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


static void AddHelicopterToMaps(BOOLEAN const fAdd, UINT8 const ubSite)
{
 	INT16 const sGridNo = sRefuelStartGridNo[ubSite];

	// find out what slot it is by which site
	INT16 const sOStruct = ubSite == 0 ?
		FIRSTOSTRUCT1 : // Drassen
		FOURTHOSTRUCT1; // Estoni

	// are we adding or taking away
	if (fAdd)
	{
		AddHeliPiece(sGridNo,       sOStruct    );
		AddHeliPiece(sGridNo,       sOStruct + 1);
		AddHeliPiece(sGridNo - 800, sOStruct + 2);
		AddHeliPiece(sGridNo,       sOStruct + 3);
		AddHeliPiece(sGridNo,       sOStruct + 4);
		AddHeliPiece(sGridNo - 800, sOStruct + 5);

    // ATE: If any mercs here, bump them off!
		INT16	sCentreGridX;
		INT16 sCentreGridY;
  	ConvertGridNoToXY(sGridNo, &sCentreGridX, &sCentreGridY);

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
		RemoveStruct(sGridNo,       sOStruct    );
		RemoveStruct(sGridNo,       sOStruct + 1);
		RemoveStruct(sGridNo - 800, sOStruct + 2);
		RemoveStruct(sGridNo,       sOStruct + 3);
		RemoveStruct(sGridNo,       sOStruct + 4);
		RemoveStruct(sGridNo - 800, sOStruct + 5);
	}

	InvalidateWorldRedundency();
	SetRenderFlags(RENDER_FLAG_FULL);
}


BOOLEAN IsSkyriderIsFlyingInSector( INT16 sSectorX, INT16 sSectorY )
{
	// up and about?
	if (iHelicopterVehicleId != -1 && CanHelicopterFly() && fHelicopterIsAirBorne)
	{
		const GROUP* const pGroup = GetGroup(GetHelicopter()->ubMovementGroup);

		// the right sector?
		if ( ( sSectorX == pGroup->ubSectorX ) && ( sSectorY == pGroup->ubSectorY ) )
		{
			return(TRUE);
		}
	}

	return( FALSE );
}


BOOLEAN IsGroupTheHelicopterGroup(const GROUP* const pGroup)
{
	if (iHelicopterVehicleId == -1) return FALSE;
	VEHICLETYPE const* const v = GetHelicopter();
	return
		v->ubMovementGroup != 0 &&
		v->ubMovementGroup == pGroup->ubGroupID;
}


INT16 GetNumSafeSectorsInPath( void )
{
	// get the last sector value in the helictoper's path
	UINT32 uiLocation = 0;
  UINT32  uiCount = 0;

	// if the heli is on the move, what is the distance it will move..the length of the merc path, less the first node
	if (!CanHelicopterFly()) return 0;

	const VEHICLETYPE* const v = GetHelicopter();
	// may need to skip the sector the chopper is currently in
	const INT32 iHeliSector = CALCULATE_STRATEGIC_INDEX(v->sSectorX, v->sSectorY);

	// get chopper's group ptr
	GROUP* const pGroup = GetGroup(v->ubMovementGroup);

	const PathSt* pNode = v->pMercPath;

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

      if ( !StrategicMap[ uiLocation ].fEnemyAirControlled )
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
				GetLengthOfPath(v->pMercPath) > 0)
		{
			pNode = pNode->pNext;
		}

		while( pNode)
		{
			uiLocation = pNode -> uiSectorId;

      if ( !StrategicMap[ uiLocation ].fEnemyAirControlled )
      {
        uiCount++;
      }

			pNode = pNode ->pNext;
		}
	}

	return( (INT16)uiCount );
}


INT16 GetNumUnSafeSectorsInPath( void )
{
	// get the last sector value in the helictoper's path
	UINT32 uiLocation = 0;
  UINT32  uiCount = 0;

	// if the heli is on the move, what is the distance it will move..the length of the merc path, less the first node
	if (!CanHelicopterFly()) return 0;

	const VEHICLETYPE* const v = GetHelicopter();
	// may need to skip the sector the chopper is currently in
	const INT32 iHeliSector = CALCULATE_STRATEGIC_INDEX(v->sSectorX, v->sSectorY);

	// get chopper's group ptr
	GROUP* const pGroup = GetGroup(v->ubMovementGroup);

	const PathSt* pNode = v->pMercPath;

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
				GetLengthOfPath(v->pMercPath) > 0)
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
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[ 0 ], iTotalAccumulatedCostByPlayer );
		}
		else
		{
			// money owed
			if( LaptopSaveInfo.iCurrentBalance > 0 )
			{
				ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[ 0 ], LaptopSaveInfo.iCurrentBalance );
				gMercProfiles[ SKYRIDER ].iBalance = LaptopSaveInfo.iCurrentBalance - iTotalAccumulatedCostByPlayer;
				// add the transaction
				AddTransactionToPlayersBook( PAYMENT_TO_NPC, SKYRIDER, GetWorldTotalMin( ), -LaptopSaveInfo.iCurrentBalance );
			}
			else
			{
				gMercProfiles[ SKYRIDER ].iBalance = - iTotalAccumulatedCostByPlayer;
			}

			HeliCharacterDialogue(OWED_MONEY_TO_SKYRIDER);
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[ 1 ], -gMercProfiles[ SKYRIDER ].iBalance );

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
		ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[ 0 ], iPayAmount );

		// now whaddawe owe?
		iAmountOwed = - gMercProfiles[ SKYRIDER ].iBalance;

		// if it wasn't enough
		if ( iAmountOwed > 0 )
		{
			ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[ 1 ], iAmountOwed );
			HeliCharacterDialogue(OWED_MONEY_TO_SKYRIDER);
		}
	}
}


static void MakeHeliReturnToBase(void)
{
	// if already at a refueling point
	if (CheckForArrivalAtRefuelPoint())
	{
		ReFuelHelicopter();
	}
	else
	{
		// choose destination (closest refueling sector)
		const INT32 iLocation = LocationOfNearestRefuelPoint(TRUE);

		VEHICLETYPE* const v = GetHelicopter();
		ClearStrategicPathList(v->pMercPath, v->ubMovementGroup);
		GROUP*       const g = GetGroup(v->ubMovementGroup);
		v->pMercPath = BuildAStrategicPath(CALCULATE_STRATEGIC_INDEX(v->sSectorX , v->sSectorY), CALCULATE_STRATEGIC_INDEX(ubRefuelList[iLocation][0], ubRefuelList[iLocation][1]), g, FALSE);
		RebuildWayPointsForGroupPath(v->pMercPath, g);

		fHeliReturnStraightToBase = TRUE;
		fHoveringHelicopter       = FALSE;
	}

	// stop time compression if it's on so player can digest this
	StopTimeCompression();
}


BOOLEAN SoldierAboardAirborneHeli(const SOLDIERTYPE* s)
{
	Assert(s);

	// if not in a vehicle, or not aboard the helicopter
	if (s->bAssignment != VEHICLE || s->iVehicleId != iHelicopterVehicleId)
	{
		return( FALSE );
	}

	// he's in the heli - is it airborne?
	if ( !fHelicopterIsAirBorne )
	{
		// nope, it's currently on the ground
		return( FALSE );
	}

	// yes, airborne
	return( TRUE );
}
