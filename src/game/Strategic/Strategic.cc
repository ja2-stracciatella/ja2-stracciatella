#include "Strategic.h"
#include "MapScreen.h"
#include "Types.h"
#include "Squads.h"
#include "Assignments.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Dialogue_Control.h"
#include "Personnel.h"
#include "Tactical_Save.h"
#include "Isometric_Utils.h"
#include "Vehicles.h"
#include "Game_Clock.h"


static void HandleSoldierDeadComments(SOLDIERTYPE const*);


void HandleStrategicDeath(SOLDIERTYPE& s)
{
	if (s.bAssignment == VEHICLE && s.iVehicleId != -1)
	{
		TakeSoldierOutOfVehicle(&s);
	}

	RemoveCharacterFromSquads(&s);
	if (s.bAssignment != ASSIGNMENT_DEAD)
	{
		SetTimeOfAssignmentChangeForMerc(&s);
	}
	ChangeSoldiersAssignment(&s, ASSIGNMENT_DEAD);

	if (fInMapMode)
	{
		ReBuildCharactersList();

		HandleSoldierDeadComments(&s);
		AddDeadSoldierToUnLoadedSector(s.sSector, &s, RandomGridNo(), ADD_DEAD_SOLDIER_TO_SWEETSPOT);

		fReDrawFace = TRUE;

		StopTimeCompression();
	}
}


static void HandleSoldierDeadComments(SOLDIERTYPE const* const dead)
{
	FOR_EACH_IN_TEAM(s, dead->bTeam)
	{
		if (s->bLife < OKLIFE) continue;

		UINT16     quote_num;
		INT8 const buddy_idx = WhichBuddy(s->ubProfile, dead->ubProfile);
		switch (buddy_idx)
		{
			case 0: quote_num = QUOTE_BUDDY_ONE_KILLED;            break;
			case 1: quote_num = QUOTE_BUDDY_TWO_KILLED;            break;
			case 2: quote_num = QUOTE_LEARNED_TO_LIKE_MERC_KILLED; break;

			default: continue;
		}
		TacticalCharacterDialogue(s, quote_num);
	}
}
