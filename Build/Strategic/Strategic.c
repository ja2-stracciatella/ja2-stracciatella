#include "Strategic.h"
#include "Types.h"
#include "Squads.h"
#include "Assignments.h"
#include "Interface_Control.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Dialogue_Control.h"
#include "Personnel.h"
#include "Tactical_Save.h"
#include "Isometric_Utils.h"
#include "Vehicles.h"
#include "Game_Clock.h"


StrategicMapElement StrategicMap[MAP_WORLD_X*MAP_WORLD_Y];


static void HandleSoldierDeadComments(SOLDIERTYPE* pSoldier);


BOOLEAN HandleStrategicDeath( SOLDIERTYPE *pSoldier )
{
	// add the guy to the dead list
	//AddCharacterToDeadList( pSoldier );

  // If in a vehicle, remove them!
	if( ( pSoldier->bAssignment == VEHICLE ) && ( pSoldier->iVehicleId != -1 ) )
	{
		// remove from vehicle
		TakeSoldierOutOfVehicle( pSoldier );
  }

	// if not in mapscreen
	if ( !(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		// ATE; At least make them dead!
		if( ( pSoldier->bAssignment != ASSIGNMENT_DEAD ) )
		{
			SetTimeOfAssignmentChangeForMerc( pSoldier );
		}

		ChangeSoldiersAssignment( pSoldier, ASSIGNMENT_DEAD );
	}
	else if( ( pSoldier -> bLife == 0 )&&( pSoldier->bAssignment != ASSIGNMENT_DEAD ) )
	{
		// died in mapscreen

		fReDrawFace = TRUE;

		// dead
		if( ( pSoldier->bAssignment != ASSIGNMENT_DEAD ) )
		{
			SetTimeOfAssignmentChangeForMerc( pSoldier );
		}

		ChangeSoldiersAssignment( pSoldier, ASSIGNMENT_DEAD );

		//s et breath and breath max to 0
		pSoldier -> bBreath = pSoldier->bBreathMax = 0;

		// rebuild list
		ReBuildCharactersList( );

		// ste merc as dead
		// pSoldier->fUIdeadMerc = TRUE;

		// attempt o remove character from squad
		RemoveCharacterFromSquads( pSoldier );

		// handle any passign comments by grunts
		HandleSoldierDeadComments( pSoldier );

		// put the dead guys down
		AddDeadSoldierToUnLoadedSector( ( UINT8 ) ( pSoldier->sSectorX ), ( UINT8 )( pSoldier->sSectorY ), pSoldier->bSectorZ, pSoldier, RandomGridNo(), ADD_DEAD_SOLDIER_TO_SWEETSPOT );

		fTeamPanelDirty = TRUE;
		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		StopTimeCompression();
	}

	return( TRUE );
}


static void HandleSoldierDeadComments(SOLDIERTYPE* pSoldier)
{
	INT32 cnt = 0;
	SOLDIERTYPE *pTeamSoldier;
	INT8 bBuddyIndex;

	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;


	// see if this was the friend of a living merc
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		if ( pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive )
		{
			bBuddyIndex = WhichBuddy( pTeamSoldier->ubProfile, pSoldier->ubProfile );
			switch( bBuddyIndex )
			{
				case 0:
					// buddy #1 died!
					TacticalCharacterDialogue( pTeamSoldier, QUOTE_BUDDY_ONE_KILLED );
					break;
				case 1:
					// buddy #2 died!
					TacticalCharacterDialogue( pTeamSoldier, QUOTE_BUDDY_TWO_KILLED );
					break;
				case 2:
					// learn to like buddy died!
					TacticalCharacterDialogue( pTeamSoldier, QUOTE_LEARNED_TO_LIKE_MERC_KILLED );
					break;
				default:
					break;
				}
			}
		}
}
