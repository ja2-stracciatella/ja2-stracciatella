#ifndef __MAP_SCREEN_HELICOPTER_H
#define __MAP_SCREEN_HELICOPTER_H

#include "Types.h"
#include "Soldier Control.h"

// costs of flying through sectors
#define COST_AIRSPACE_SAFE    100
#define COST_AIRSPACE_UNSAFE  1000		// VERY dangerous

#define MIN_PROGRESS_FOR_SKYRIDER_QUOTE_DOING_WELL 25	// scale of 0-100
#define MIN_REGRESS_FOR_SKYRIDER_QUOTE_DOING_BADLY 10	// scale of 0-100


// skyrider quotes
#define OWED_MONEY_TO_SKYRIDER 11
#define MENTION_DRASSEN_SAM_SITE 20
#define SECOND_HALF_OF_MENTION_DRASSEN_SAM_SITE 21
#define SAM_SITE_TAKEN 22
#define SKYRIDER_SAYS_HI 23
#define SPIEL_ABOUT_OTHER_SAM_SITES 24
#define SECOND_HALF_OF_SPIEL_ABOUT_OTHER_SAM_SITES 25

#define SPIEL_ABOUT_ESTONI_AIRSPACE 26
#define CONFIRM_DESTINATION 27
//#define DESTINATION_TOO_FAR 28		// unused
#define ALTERNATE_FUEL_SITE 26
#define ARRIVED_IN_HOSTILE_SECTOR 29
#define BELIEVED_ENEMY_SECTOR 30		// may become unused
#define ARRIVED_IN_NON_HOSTILE_SECTOR 31
#define HOVERING_A_WHILE 32
#define RETURN_TO_BASE  33
#define ENEMIES_SPOTTED_EN_ROUTE_IN_FRIENDLY_SECTOR_A 34
#define ENEMIES_SPOTTED_EN_ROUTE_IN_FRIENDLY_SECTOR_B 35
#define MENTION_HOSPITAL_IN_CAMBRIA 45
#define THINGS_ARE_GOING_BADLY 46
#define THINGS_ARE_GOING_WELL 47
#define CHOPPER_NOT_ACCESSIBLE 48
#define DOESNT_WANT_TO_FLY 49
#define HELI_TOOK_MINOR_DAMAGE 52
#define HELI_TOOK_MAJOR_DAMAGE 53
#define HELI_GOING_DOWN 54


// enums for skyrider monologue
enum{
	SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE = 0,
	SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES,
	SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL,
	SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL,
};

enum
{
	DRASSEN_REFUELING_SITE = 0,
	ESTONI_REFUELING_SITE,
	NUMBER_OF_REFUEL_SITES,
};

// the sam site enums
enum{
	SAM_SITE_ONE = 0,			// near Chitzena
	SAM_SITE_TWO,					// near Drassen
	SAM_SITE_THREE,				// near Cambria
	SAM_SITE_FOUR,				// near Meduna
	NUMBER_OF_SAM_SITES,
};

// list of refueling site's sector locations
extern	UINT8 ubRefuelList[ NUMBER_OF_REFUEL_SITES ][ 2 ];


// is the helicopter available to player?
extern BOOLEAN fHelicopterAvailable;

// helicopter vehicle id value
extern INT32 iHelicopterVehicleId;

// heli is hovering
extern BOOLEAN fHoveringHelicopter;

// helicopter icon
extern UINT32 guiHelicopterIcon;

// helicopter destroyed
extern BOOLEAN fHelicopterDestroyed;

// is the pilot returning straight to base?
extern BOOLEAN fHeliReturnStraightToBase;

// is the heli in the air?
extern BOOLEAN fHelicopterIsAirBorne;


// total distance travelled
//extern INT32 iTotalHeliDistanceSinceRefuel;

// total owed to player
//extern INT32 iTotalAccumlatedCostByPlayer;

// whether or not skyrider is alive and well? and on our side yet?
extern BOOLEAN fSkyRiderAvailable;

// time started hovering
extern UINT32 uiStartHoverTime;

// what state is skyrider's dialogue in in?
extern UINT32 guiHelicopterSkyriderTalkState;

// plot for helicopter
extern BOOLEAN fPlotForHelicopter;

// the flags for skyrider events
extern BOOLEAN fShowEstoniRefuelHighLight;
extern BOOLEAN fShowOtherSAMHighLight;
extern BOOLEAN fShowDrassenSAMHighLight;
extern BOOLEAN fShowCambriaHospitalHighLight;

extern INT32 iTotalAccumulatedCostByPlayer;
extern UINT32 guiTimeOfLastSkyriderMonologue;
extern BOOLEAN fSkyRiderSetUp;
extern BOOLEAN fRefuelingSiteAvailable[ NUMBER_OF_REFUEL_SITES ];

extern UINT8 gubHelicopterHitsTaken;
extern BOOLEAN gfSkyriderSaidCongratsOnTakingSAM;
extern UINT8 gubPlayerProgressSkyriderLastCommentedOn;

// add and remove grunts from the helicopter
BOOLEAN AddSoldierToHelicopter( SOLDIERTYPE *pSoldier );
BOOLEAN RemoveSoldierFromHelicopter( SOLDIERTYPE *pSoldier );

#ifdef JA2TESTVERSION
extern BOOLEAN	fSAMSitesDisabledFromAttackingPlayer;
#endif


/* ARM: Max. fuel range system removed
// add another sector to how far helictoper has travelled
void AddSectorToHelicopterDistanceTravelled( void );

// total distance travelled since last refuel
INT32 HowFarHelicopterhasTravelledSinceRefueling( void );

// get the total the heli can go
INT32 GetTotalDistanceHelicopterCanTravel( void );

// how far can helicopter can travel before refuel
INT32 HowFurtherCanHelicopterTravel( void );

// check if this sector is out of the way
BOOLEAN IsSectorOutOfTheWay( INT16 sX, INT16 sY );

*/

// how far to nearest refuel point from this sector?
INT32 DistanceToNearestRefuelPoint( INT16 sX, INT16 sY );

// location of closest
INT32 LocationOfNearestRefuelPoint( BOOLEAN fNotifyPlayerIfNoSafeLZ );

// refuel helicopter
void ReFuelHelicopter( void );

// how much will it cost for helicopter to travel through this sector?
INT32 GetCostOfPassageForHelicopter( INT16 sX, INT16 sY );

// helicopter shot down, kill all on board
void SkyriderDestroyed( void );

// have pilot say different stuff
void HelicopterDialogue( UINT8 ubDialogueCondition );

// is the helicopter available for flight?
BOOLEAN CanHelicopterFly( void );

// is the pilot alive and on our side?
BOOLEAN IsHelicopterPilotAvailable( void );

// land the helicopter here
void LandHelicopter( void );

// have helicopter take off
void TakeOffHelicopter( void );

// test whether or not a sector contains a fuel site
BOOLEAN IsRefuelSiteInSector( INT16 sMapX, INT16 sMapY );

// update which refueling sites are controlled by player & therefore available
void UpdateRefuelSiteAvailability( void );

// setup helicopter for player
void SetUpHelicopterForPlayer( INT16 sX, INT16 sY );

// the intended path of the helicopter
INT32 DistanceOfIntendedHelicopterPath( void );

// handle a little wait for hover
void HandleHeliHoverLong( void );

// handle a LONG wait in hover mode
void HandleHeliHoverTooLong( void );

// start the heli hover time
void StartHoverTime( void );

// drop off everyone in helicopter
void DropOffEveryOneInHelicopter( void );

// handle heli entering this sector
BOOLEAN HandleHeliEnteringSector( INT16 sX, INT16 sY );

// check for arrival at refuel
BOOLEAN CheckForArrivalAtRefuelPoint( void );

// set up helic, if it doesn't have a mvt group
void SetUpHelicopterForMovement( void );

// number of passengers in helicopter
INT32 GetNumberOfPassengersInHelicopter( void );

// skyrider talking to player
void SkyRiderTalk( UINT16 usQuoteNum );

// sky rider monlogue events for mapscreen
void HandleSkyRiderMonologueEvent( UINT32 uiEventCode, UINT32 uiSpecialCode );

// handle animation of sectors for mapscreen
void HandleAnimationOfSectors( void );

// check and handle skyrider monologue
void CheckAndHandleSkyriderMonologues( void );

// last sector value in helicopter's path
INT16 LastSectorInHelicoptersPath( void );

// total cost of helicopter trip
//INT32 GetTotalCostOfHelicopterTrip( void );

void HandleHelicopterOnGroundGraphic( void );

void HandleHelicopterOnGroundSkyriderProfile( void );

BOOLEAN IsHelicopterOnGroundAtRefuelingSite( UINT8 ubRefuelingSite );

BOOLEAN HandleSAMSiteAttackOfHelicopterInSector( INT16 sSectorX, INT16 sSectorY );

// will a sam site under the players control shoot down an airraid?
//BOOLEAN WillAirRaidBeStopped( INT16 sSectorX, INT16 sSectorY );

// is the helicopter capable of taking off for the player?
BOOLEAN CanHelicopterTakeOff( void );

void InitializeHelicopter( void );

BOOLEAN IsSkyriderIsFlyingInSector( INT16 sSectorX, INT16 sSectorY );

BOOLEAN IsGroupTheHelicopterGroup( GROUP *pGroup );

INT16 GetNumSafeSectorsInPath( void );

INT16 GetNumUnSafeSectorsInPath( void );

BOOLEAN SoldierAboardAirborneHeli( SOLDIERTYPE *pSoldier );

UINT8 MoveAllInHelicopterToFootMovementGroup( void );


#endif
