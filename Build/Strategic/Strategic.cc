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


void HandleStrategicDeath(SOLDIERTYPE* const s)
{
	if (s->bAssignment == VEHICLE && s->iVehicleId != -1)
	{
		TakeSoldierOutOfVehicle(s);
  }

	if (!(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN))
	{
		// ATE; At least make them dead!
		if (s->bAssignment != ASSIGNMENT_DEAD)
		{
			SetTimeOfAssignmentChangeForMerc(s);
		}
		ChangeSoldiersAssignment(s, ASSIGNMENT_DEAD);
	}
	else if (s->bLife == 0 && s->bAssignment != ASSIGNMENT_DEAD)
	{
		// died in mapscreen
		fReDrawFace = TRUE;

		if (s->bAssignment != ASSIGNMENT_DEAD)
		{
			SetTimeOfAssignmentChangeForMerc(s);
		}
		ChangeSoldiersAssignment(s, ASSIGNMENT_DEAD);

		s->bBreath    = 0;
		s->bBreathMax = 0;

		ReBuildCharactersList();

		RemoveCharacterFromSquads(s);
		HandleSoldierDeadComments(s);
		AddDeadSoldierToUnLoadedSector(s->sSectorX, s->sSectorY, s->bSectorZ, s, RandomGridNo(), ADD_DEAD_SOLDIER_TO_SWEETSPOT);

		fTeamPanelDirty          = TRUE;
		fMapPanelDirty           = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		StopTimeCompression();
	}
}


static void HandleSoldierDeadComments(SOLDIERTYPE* pSoldier)
{
	FOR_ALL_IN_TEAM(s, pSoldier->bTeam)
	{
		if (s->bLife < OKLIFE) continue;

		const INT8 bBuddyIndex = WhichBuddy(s->ubProfile, pSoldier->ubProfile);
		switch (bBuddyIndex)
		{
			case 0:
				// buddy #1 died!
				TacticalCharacterDialogue(s, QUOTE_BUDDY_ONE_KILLED);
				break;
			case 1:
				// buddy #2 died!
				TacticalCharacterDialogue(s, QUOTE_BUDDY_TWO_KILLED);
				break;
			case 2:
				// learn to like buddy died!
				TacticalCharacterDialogue(s, QUOTE_LEARNED_TO_LIKE_MERC_KILLED);
				break;
			default:
				break;
		}
	}
}
