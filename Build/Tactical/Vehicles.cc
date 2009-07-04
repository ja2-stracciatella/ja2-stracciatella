#include <stdexcept>

#include "LoadSaveVehicleType.h"
#include "SaveLoadGame.h"
#include "Soldier_Find.h"
#include "Vehicles.h"
#include "Strategic_Pathing.h"
#include "Assignments.h"
#include "Strategic_Movement.h"
#include "Squads.h"
#include "Map_Screen_Helicopter.h"
#include "Game_Clock.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "Soldier_Add.h"
#include "Strategic.h"
#include "WorldDef.h"
#include "Tile_Animation.h"
#include "Isometric_Utils.h"
#include "Interface.h"
#include "Random.h"
#include "Text.h"
#include "Explosion_Control.h"
#include "Soldier_Create.h"
#include "StrategicMap.h"
#include "Interface_Control.h"
#include "Campaign_Types.h"
#include "Sys_Globals.h"
#include "Map_Screen_Interface.h"
#include "JAScreens.h"
#include "Quests.h"
#include "Tactical_Save.h"
#include "Soldier_Macros.h"
#include "OppList.h"
#include "Soldier_Ani.h"
#include "MemMan.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "FileMan.h"


INT8 gubVehicleMovementGroups[ MAX_VEHICLES ];

// the list of vehicles
VEHICLETYPE *pVehicleList = NULL;

// number of vehicle slots on the list
UINT8 ubNumberOfVehicles = 0;


//ATE: These arrays below should all be in a large LUT which contains
// static info for each vehicle....


struct VehicleTypeInfo
{
	SoundID   enter_sound;
	SoundID   move_sound;
	ProfileID profile;
	UINT8     movement_type;
	UINT16    armour_type;
	UINT8     seats;
};

static const VehicleTypeInfo g_vehicle_type_info[] =
{
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_ELDERODO,   CAR, KEVLAR_VEST,  6 }, // Eldorado
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_HUMMER,     CAR, SPECTRA_VEST, 6 }, // Hummer
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_ICECREAM,   CAR, KEVLAR_VEST,  6 }, // Ice cream truck
	{ S_VECH1_INTO, S_VECH1_MOVE, NPC164,          CAR, KEVLAR_VEST,  6 }, // Jeep
	{ S_VECH1_INTO, S_VECH1_MOVE, NPC164,          CAR, SPECTRA_VEST, 6 }, // Tank
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_HELICOPTER, AIR, KEVLAR_VEST,  6 }  // Helicopter
};


// Loop through and create a few soldier squad ID's for vehicles ( max # 3 )
void InitVehicles(void)
{
	INT32		cnt;
	for( cnt = 0; cnt <  MAX_VEHICLES; cnt++ )
	{
		// create mvt groups
		GROUP* const g = CreateNewVehicleGroupDepartingFromSector(1, 1);
		g->fPersistant = TRUE;
		gubVehicleMovementGroups[cnt] = g->ubGroupID;
	}
}


void SetVehicleValuesIntoSoldierType(SOLDIERTYPE* const vs)
{
	const VEHICLETYPE* const v = &pVehicleList[vs->bVehicleID];
	wcscpy(vs->name, zVehicleName[v->ubVehicleType]);
	vs->ubProfile           = g_vehicle_type_info[v->ubVehicleType].profile;
  vs->sBreathRed          = 10000; // Init fuel
  vs->bBreath             = 100;
  vs->ubWhatKindOfMercAmI = MERC_TYPE__VEHICLE;
}


INT32 AddVehicleToList(const INT16 sMapX, const INT16 sMapY, const INT16 sGridNo, const UINT8 ubType)
{
	INT32 vid;
	for (vid = 0;; ++vid)
	{
		if (vid == ubNumberOfVehicles)
		{
			pVehicleList = REALLOC(pVehicleList, VEHICLETYPE, ++ubNumberOfVehicles);
			break;
		}
		if (!pVehicleList[vid].fValid) break;
	}
	VEHICLETYPE* const v = &pVehicleList[vid];

	// found a slot
	memset(v, 0, sizeof(*v));
	v->ubMovementGroup = 0;
	v->sSectorX        = sMapX;
	v->sSectorY        = sMapY;
	v->sSectorZ        = 0;
	v->sGridNo         = sGridNo;
	v->fValid          = TRUE;
	v->ubVehicleType   = ubType;
	v->pMercPath       = NULL;
	v->fDestroyed      = FALSE;
	v->ubMovementGroup = gubVehicleMovementGroups[vid];

	// ATE: Add movement mask to group...
	GROUP* const g = GetGroup(v->ubMovementGroup);
	// This is okay, no groups exist, so simply return.
	if (!g && gfEditMode) return vid;
	Assert(g);

	// ARM: setup group movement defaults
	g->ubTransportationMask = g_vehicle_type_info[ubType].movement_type;
	g->ubSectorX            = sMapX;
	g->ubNextX              = sMapX;
	g->ubSectorY            = sMapY;
	g->ubNextY              = sMapY;
	g->uiTraverseTime       = 0;
	g->uiArrivalTime        = 0;

	return vid;
}


void RemoveVehicleFromList(VEHICLETYPE* const v)
{
	v->pMercPath = ClearStrategicPathList(v->pMercPath, 0);
	memset(v, 0, sizeof(*v));
}


void ClearOutVehicleList(void)
{
	if (pVehicleList == NULL) return;

	FOR_ALL_VEHICLES(v)
	{
		v->pMercPath = ClearStrategicPathList(v->pMercPath, 0);
	}

	MemFree(pVehicleList);
	pVehicleList       = NULL;
	ubNumberOfVehicles = 0;
}


BOOLEAN IsThisVehicleAccessibleToSoldier(const SOLDIERTYPE* const s, const VEHICLETYPE* const v)
{
	return
		s != NULL                  &&
		!s->fBetweenSectors        &&
		!v->fBetweenSectors        &&
		s->sSectorX == v->sSectorX &&
		s->sSectorY == v->sSectorY &&
		s->bSectorZ == v->sSectorZ &&
		OKUseVehicle(g_vehicle_type_info[v->ubVehicleType].profile);
}


static BOOLEAN AddSoldierToVehicle(SOLDIERTYPE* const s, VEHICLETYPE* const v)
{
	// ok now check if any free slots in the vehicle

	SOLDIERTYPE* vs = 0;
	if (VEHICLE2ID(v) != iHelicopterVehicleId)
	{
		vs = &GetSoldierStructureForVehicle(*v);
		if (vs->bTeam != gbPlayerNum)
		{
			// Change sides
			vs = ChangeSoldierTeam(vs, gbPlayerNum);
			// add it to mapscreen list
			fReBuildCharacterList = TRUE;
		}

		// If vehicle is empty, add to unique squad now that it has somebody in it!
		if (GetNumberInVehicle(*v) == 0)
		{
			// 2 ) Add to unique squad...
			AddCharacterToUniqueSquad(vs);

			// ATE: OK funcky stuff here!
			// We have now a guy on a squad group, remove him!
			RemovePlayerFromGroup(vs);

			// I really have vehicles.
			// ONLY add to vehicle group once!
			GROUP* const g = GetGroup(v->ubMovementGroup);
			if (!DoesPlayerExistInPGroup(g, vs))
			{
				//NOW.. add guy to vehicle group....
				AddPlayerToGroup(g, vs);
			}
			else
			{
				vs->ubGroupID = v->ubMovementGroup;
			}
		}
	}

	// check if the grunt is already here
	CFOR_ALL_PASSENGERS(v, i)
	{
		if (*i == s) return TRUE; // guy found, no need to add
	}

	if (vs)
	{
		// can't call SelectSoldier in mapscreen, that will initialize interface panels!!!
	  if (guiCurrentScreen == GAME_SCREEN)
		{
			SelectSoldier(vs, SELSOLDIER_FORCE_RESELECT);
		}

		PlayLocationJA2Sample(vs->sGridNo, g_vehicle_type_info[v->ubVehicleType].enter_sound, HIGHVOLUME, 1);
	}

	INT32 const seats = GetVehicleSeats(v);
	for (INT32 i = 0; i < seats; ++i)
	{
		if (v->pPassengers[i]) continue;
		v->pPassengers[i] = s;

		if (s->bAssignment == VEHICLE)
		{
			TakeSoldierOutOfVehicle(s);
			/* NOTE: This will leave the soldier on a squad.  Must be done PRIOR TO
			 * and in AS WELL AS the call to RemoveCharacterFromSquads() that's coming
			 * up, to permit direct vehicle->vehicle reassignment! */
		}

		/* if in a squad, remove from squad, if not, then check if in mvt group, if
		 * so, move and destroy group */
		if (s->bAssignment < ON_DUTY)
		{
			RemoveCharacterFromSquads(s);
		}
		else if (s->ubGroupID != 0)
		{
			// destroy group and set to zero
			RemoveGroup(s->ubGroupID);
			s->ubGroupID = 0;
		}

		if (s->bAssignment != VEHICLE || s->iVehicleId != VEHICLE2ID(v))
		{
			SetTimeOfAssignmentChangeForMerc(s);
		}

		ChangeSoldiersAssignment(s, VEHICLE);

		s->iVehicleId = VEHICLE2ID(v);

		// if vehicle is part of mvt group, then add character to mvt group
		if (v->ubMovementGroup != 0)
		{
			AddPlayerToGroup(GetGroup(v->ubMovementGroup), s);
		}

		// Are we the first?
		s->uiStatusFlags |= GetNumberInVehicle(*v) == 1 ?
			SOLDIER_DRIVER : SOLDIER_PASSENGER;

		RemoveSoldierFromGridNo(s);

		if (vs)
		{
			// Set gridno for vehicle.....
			EVENT_SetSoldierPositionXY(s, vs->dXPos, vs->dYPos, SSP_NONE);

			// Stop from any movement.....
			EVENT_StopMerc(s);

			// can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface panels!!!
			if (guiCurrentScreen == GAME_SCREEN)
			{
				SetCurrentSquad(vs->bAssignment, TRUE);
			}
		}

		return TRUE;
	}

	// no slots, leave
	return FALSE;
}


void SetSoldierExitHelicopterInsertionData(SOLDIERTYPE* const s)
{
	if (s->bInSector) return;

	if (s->sSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X &&
			s->sSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y &&
			s->bSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z)
	{ // This is Drassen, make insertion gridno specific
		s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		s->usStrategicInsertionData = 10125;
	}
	else
	{ // Not anything different here - just use center gridno
		s->ubStrategicInsertionCode = INSERTION_CODE_CENTER;
	}
}


static void TeleportVehicleToItsClosestSector(UINT8 ubGroupID);


// remove soldier from vehicle
static bool RemoveSoldierFromVehicle(SOLDIERTYPE& s)
{
	INT32 const  iId = s.iVehicleId;
	VEHICLETYPE& v   = GetVehicle(iId);

	// now look for the grunt
	INT32 const seats = GetVehicleSeats(&v);
	for (INT32 i = 0;; ++i)
	{
		if (i == seats) return false;
		if (v.pPassengers[i] != &s) continue;
		v.pPassengers[i] = 0;
		break;
	}

	RemovePlayerFromGroup(&s);

	s.ubGroupID      = 0;
	s.sSectorY       = v.sSectorY;
	s.sSectorX       = v.sSectorX;
	s.bSectorZ       = v.sSectorZ;
	s.uiStatusFlags &= ~(SOLDIER_DRIVER | SOLDIER_PASSENGER);

	if (iId == iHelicopterVehicleId)
	{ /* The vehicle the helicopter? It can continue moving when no soldiers
		 * aboard (Skyrider remains) */
		if (s.bLife >= OKLIFE)
		{ /* Mark the sector as visited (flying around in the chopper doesn't, so
			 * this does it as soon as we get off it) */
			SetSectorFlag(s.sSectorX, s.sSectorY, s.bSectorZ, SF_ALREADY_VISITED);
		}

		SetSoldierExitHelicopterInsertionData(&s);

    // Update in sector if this is the current sector
		if (s.sSectorX == gWorldSectorX &&
				s.sSectorY == gWorldSectorY &&
				s.bSectorZ == gbWorldSectorZ)
		{
			UpdateMercInSector(s, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
		}
	}
	else
	{
		// check if anyone left in vehicle
		CFOR_ALL_PASSENGERS(&v, i) return true;

		SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);

		// and he has a route set
		if (GetLengthOfMercPath(&vs) > 0)
		{
			// cancel the entire path (also handles reversing directions)
			CancelPathForVehicle(&v, FALSE);
		}

		if (v.fBetweenSectors)
		{ /* The vehicle was abandoned between sectors. Teleport it to the closer of
			 * its current and next sectors (it beats having it arrive empty later) */
			TeleportVehicleToItsClosestSector(vs.ubGroupID);
		}

		// Remove vehicle from squad
		RemoveCharacterFromSquads(&vs);
		// ATE: Add him back to vehicle group!
		GROUP* const g = GetGroup(v.ubMovementGroup);
		if (!DoesPlayerExistInPGroup(g, &vs)) AddPlayerToGroup(g, &vs);
		ChangeSoldiersAssignment(&vs, ASSIGNMENT_EMPTY);
	}
	return true;
}


BOOLEAN MoveCharactersPathToVehicle(SOLDIERTYPE* const s)
{
	if (!s) return FALSE;

	// check if character is in fact in a vehicle
	INT32 vid;
	if (s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		vid = s->bVehicleID;
	}
	else if (s->bAssignment == VEHICLE)
	{
		vid = s->iVehicleId;
	}
	else
	{
		s->pMercPath = ClearStrategicPathList(s->pMercPath, 0);
		return FALSE;
	}

	VEHICLETYPE& v = GetVehicle(vid);

	ClearStrategicPathList(v.pMercPath, v.ubMovementGroup);
	v.pMercPath = CopyPaths(s->pMercPath);

	s->pMercPath = ClearStrategicPathList(s->pMercPath, 0);
	return TRUE;
}


void SetUpMvtGroupForVehicle(SOLDIERTYPE* const s)
{
	INT32 vid;
	// Check if character is in fact in a vehicle
	if      (s->uiStatusFlags &  SOLDIER_VEHICLE) vid = s->bVehicleID;
	else if (s->bAssignment   == VEHICLE)         vid = s->iVehicleId;
	else                                          return;
	VEHICLETYPE& v = GetVehicle(vid);
	ClearStrategicPathList(s->pMercPath, s->ubGroupID);
	s->pMercPath = CopyPaths(v.pMercPath);
	s->ubGroupID = v.ubMovementGroup;
}


VEHICLETYPE& GetVehicle(INT32 const vehicle_id)
{
	if (0 <= vehicle_id && vehicle_id < ubNumberOfVehicles)
	{
		VEHICLETYPE& v = pVehicleList[vehicle_id];
		if (v.fValid) return v;
	}
	throw std::logic_error("Invalid vehicle ID");
}


VEHICLETYPE& GetVehicleFromMvtGroup(GROUP const* const g)
{
	// given the id of a mvt group, find a vehicle in this group
	FOR_ALL_VEHICLES(i)
	{
		VEHICLETYPE& v = *i;
		if (v.ubMovementGroup == g->ubGroupID) return v;
	}
	throw std::logic_error("Group does not contain a vehicle");
}


// kill this person in the vehicle
static BOOLEAN KillPersonInVehicle(SOLDIERTYPE* pSoldier)
{
	// now check hpts of merc
	if (pSoldier->bLife == 0)
	{
		// guy is dead, leave
		return FALSE;
	}

	// otherwise hurt them
	SoldierTakeDamage(pSoldier, 100, 100, TAKE_DAMAGE_BLOODLOSS, NULL);
	return TRUE;
}


BOOLEAN KillAllInVehicle(const VEHICLETYPE* const v)
{
	// go through list of occupants and kill them
	CFOR_ALL_PASSENGERS(v, i)
	{
		if (!KillPersonInVehicle(*i)) return FALSE;
	}
	return TRUE;
}


INT32 GetNumberInVehicle(VEHICLETYPE const& v)
{
	// go through list of occupants in vehicles and count them
	INT32 count = 0;
	CFOR_ALL_PASSENGERS(&v, i) ++count;
	return count;
}


INT32 GetNumberOfNonEPCsInVehicle( INT32 iId )
{
	// go through list of occupants in vehicles and count them
	VEHICLETYPE const& v = GetVehicle(iId);

	INT32 count = 0;
	CFOR_ALL_PASSENGERS(&v, i)
	{
		const SOLDIERTYPE* const s = *i;
		if (!AM_AN_EPC(s)) ++count;
	}
	return count;
}


BOOLEAN IsRobotControllerInVehicle( INT32 iId )
{
	VEHICLETYPE const& v = GetVehicle(iId);
	CFOR_ALL_PASSENGERS(&v, i)
	{
		if (ControllingRobot(*i)) return TRUE;
	}
	return FALSE;
}


BOOLEAN AnyAccessibleVehiclesInSoldiersSector(const SOLDIERTYPE* const s)
{
	CFOR_ALL_VEHICLES(v)
	{
		if (IsThisVehicleAccessibleToSoldier(s, v)) return TRUE;
	}
	return FALSE;
}


bool IsEnoughSpaceInVehicle(VEHICLETYPE const& v)
{
	return GetNumberInVehicle(v) != GetVehicleSeats(&v);
}


BOOLEAN TakeSoldierOutOfVehicle(SOLDIERTYPE* const s)
{
	// if not in vehicle, don't take out, not much point, now is there?
	if (s->bAssignment != VEHICLE) return FALSE;

	if (s->sSectorX == gWorldSectorX &&
			s->sSectorY == gWorldSectorY &&
			s->bSectorZ == 0             &&
			s->bInSector                 &&
			s->iVehicleId != iHelicopterVehicleId) // helicopter isn't a soldiertype instance
	{
		return ExitVehicle(s);
	}
	else
	{
		return RemoveSoldierFromVehicle(*s);
	}
}


BOOLEAN PutSoldierInVehicle(SOLDIERTYPE* const s, VEHICLETYPE* const v)
{
	if (!AddSoldierToVehicle(s, v)) return FALSE;

	if (s->sSectorX   == gWorldSectorX        &&
			s->sSectorY   == gWorldSectorY        &&
			s->bSectorZ   == 0                    &&
			VEHICLE2ID(v) != iHelicopterVehicleId &&
			!(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN))
	{
		SetCurrentInterfacePanel(TEAM_PANEL);
	}

	return TRUE;
}


BOOLEAN ExitVehicle(SOLDIERTYPE* const s)
{
	SOLDIERTYPE& vs = GetSoldierStructureForVehicle(GetVehicle(s->iVehicleId));

	INT16 sGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(s, s->usUIMovementMode, 5, 3, &vs);
	if (sGridNo == NOWHERE)
	{
		// ATE: BUT we need a place, widen the search
		sGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(s, s->usUIMovementMode, 20, 3, &vs);
	}

	RemoveSoldierFromVehicle(*s);

	s->sInsertionGridNo         = sGridNo;
	s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
	s->usStrategicInsertionData = s->sInsertionGridNo;
	s->iVehicleId               = -1;

	//AllTeamsLookForAll( FALSE );
	s->bOppList[vs.ubID] = 1;

	// Add to sector....
	EVENT_SetSoldierPosition(s, sGridNo, SSP_NONE);

	// Update visiblity.....
	HandleSight(s, SIGHT_LOOK | SIGHT_RADIO);

	AddCharacterToUniqueSquad(s);

	// can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface panels!!!
	if (guiCurrentScreen == GAME_SCREEN)
	{
		SetCurrentSquad(s->bAssignment, TRUE);
		SelectSoldier(s, SELSOLDIER_FORCE_RESELECT);
	}

	PlayLocationJA2Sample(vs.sGridNo, g_vehicle_type_info[pVehicleList[vs.bVehicleID].ubVehicleType].enter_sound, HIGHVOLUME, 1);
	return TRUE;
}


static void HandleCriticalHitForVehicleInLocation(UINT8 ubID, INT16 sDmg, INT16 sGridNo, SOLDIERTYPE* att);


void VehicleTakeDamage(const UINT8 ubID, const UINT8 ubReason, const INT16 sDamage, const INT16 sGridNo, SOLDIERTYPE* const att)
{
  if ( ubReason != TAKE_DAMAGE_GAS )
  {
  	PlayLocationJA2Sample(sGridNo, S_METAL_IMPACT3, MIDVOLUME, 1);
  }

	// check if there was in fact damage done to the vehicle
	if( ( ubReason == TAKE_DAMAGE_HANDTOHAND ) || ( ubReason == TAKE_DAMAGE_GAS ) )
	{
		// nope
		return;
	}

	if (!pVehicleList[ubID].fDestroyed)
	{
		switch( ubReason )
		{
			case( TAKE_DAMAGE_GUNFIRE ):
			case( TAKE_DAMAGE_EXPLOSION):
			case( TAKE_DAMAGE_STRUCTURE_EXPLOSION):

			HandleCriticalHitForVehicleInLocation(ubID, sDamage, sGridNo, att);
			break;
		}
	}
}


// handle crit hit to vehicle in this location
static void HandleCriticalHitForVehicleInLocation(const UINT8 ubID, const INT16 sDmg, const INT16 sGridNo, SOLDIERTYPE* const att)
{
	// check state the armor was s'posed to be in vs. the current state..the difference / orig state is % chance
	// that a critical hit will occur
	BOOLEAN	fMadeCorpse = FALSE;

	VEHICLETYPE& v  = pVehicleList[ubID];
	SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);

	if (sDmg > vs.bLife)
	{
		vs.bLife = 0;
	}
	else
	{
		// Decrease Health
		vs.bLife -= sDmg;
	}

	if (vs.bLife < OKLIFE) vs.bLife = 0;

	//Show damage
	vs.sDamage += sDmg;

	if (vs.bInSector && vs.bVisible != -1)
	{
		// If we are already dead, don't show damage!
		if ( sDmg != 0 )
		{
			// Display damage

			// Set Damage display counter
			vs.fDisplayDamage = TRUE;
			vs.bDisplayDamageCount = 0;

			vs.sDamageX = vs.sBoundingBoxOffsetX;
			vs.sDamageY = vs.sBoundingBoxOffsetY;
		}
	}

	if (vs.bLife == 0 && !v.fDestroyed)
	{
		v.fDestroyed = TRUE;

		// Explode vehicle...
		IgniteExplosion(att, 0, sGridNo, GREAT_BIG_EXPLOSION, 0);

		CheckForAndHandleSoldierDeath(&vs, &fMadeCorpse);

    KillAllInVehicle(&v);
	}
}


BOOLEAN DoesVehicleNeedAnyRepairs(const VEHICLETYPE* const v)
{
	// Skyrider isn't damagable/repairable
	if (VEHICLE2ID(v) == iHelicopterVehicleId) return FALSE;

	// get the vehicle soldiertype
	SOLDIERTYPE const& vs = GetSoldierStructureForVehicle(*v);
	return vs.bLife != vs.bLifeMax;
}


INT8 RepairVehicle(VEHICLETYPE const* const v, INT8 const bRepairPtsLeft, BOOLEAN* const pfNothingToRepair)
{
	INT8					bRepairPtsUsed = 0;
	INT8					bOldLife;

	if (!DoesVehicleNeedAnyRepairs(v)) return bRepairPtsUsed;

	// get the vehicle soldiertype
	SOLDIERTYPE& vs = GetSoldierStructureForVehicle(*v);

	bOldLife = vs.bLife;

	// Repair
	vs.bLife += bRepairPtsLeft / VEHICLE_REPAIR_POINTS_DIVISOR;

	// Check
	if (vs.bLife > vs.bLifeMax) vs.bLife = vs.bLifeMax;

	// Calculate pts used;
	bRepairPtsUsed = (vs.bLife - bOldLife) * VEHICLE_REPAIR_POINTS_DIVISOR;

	// ARM: personally, I'd love to know where in Arulco the mechanic gets the PARTS to do this stuff, but hey, it's a game!
	*pfNothingToRepair = !DoesVehicleNeedAnyRepairs(v);

	return( bRepairPtsUsed );
}


SOLDIERTYPE& GetSoldierStructureForVehicle(VEHICLETYPE const& v)
{
	FOR_ALL_SOLDIERS(s)
	{
		if (!(s->uiStatusFlags & SOLDIER_VEHICLE)) continue;
		if (s->bVehicleID != VEHICLE2ID(&v))       continue;
		return *s;
	}
	throw std::logic_error("Vehicle has no corresponding soldier");
}


void SaveVehicleInformationToSaveGameFile(HWFILE const f)
{
	//Save the number of elements
	FileWrite(f, &ubNumberOfVehicles, sizeof(UINT8));

	//loop through all the vehicles and save each one
	for (UINT8 i = 0; i < ubNumberOfVehicles; ++i)
	{
		const VEHICLETYPE* const v = &pVehicleList[i];
		//save if the vehicle spot is valid
		FileWrite(f, &v->fValid, sizeof(BOOLEAN));
		if (!v->fValid) continue;

		InjectVehicleTypeIntoFile(f, v);
		SaveMercPath(f, v->pMercPath);
	}
}


void LoadVehicleInformationFromSavedGameFile(HWFILE const hFile, UINT32 const uiSavedGameVersion)
{
	ClearOutVehicleList();

	//Load the number of elements
	FileRead(hFile, &ubNumberOfVehicles, sizeof(UINT8));
	if (ubNumberOfVehicles == 0) return;

	//allocate memory to hold the vehicle list
	VEHICLETYPE* const vl = MALLOCNZ(VEHICLETYPE, ubNumberOfVehicles);
	pVehicleList = vl;

	//loop through all the vehicles and load each one
	for (UINT8 cnt = 0; cnt < ubNumberOfVehicles; ++cnt)
	{
		VEHICLETYPE* const v = &vl[cnt];
		//Load if the vehicle spot is valid
		FileRead(hFile, &v->fValid, sizeof(BOOLEAN));
		if (!v->fValid) continue;

		ExtractVehicleTypeFromFile(hFile, v, uiSavedGameVersion);
		LoadMercPath(hFile, &v->pMercPath);
	}
}


void SetVehicleSectorValues(VEHICLETYPE& v, UINT8 const x, UINT8 const y)
{
	v.sSectorX = x;
	v.sSectorY = y;

	MERCPROFILESTRUCT& p = GetProfile(g_vehicle_type_info[v.ubVehicleType].profile);
	p.sSectorX = x;
	p.sSectorY = y;

	// Go through list of mercs in vehicle and set all their states as arrived
	CFOR_ALL_PASSENGERS(&v, i)
	{
		SOLDIERTYPE& s = **i;
		s.sSectorX        = x;
		s.sSectorY        = y;
		s.fBetweenSectors = FALSE;
	}
}


void UpdateAllVehiclePassengersGridNo(SOLDIERTYPE* const vs)
{
	// If not a vehicle, ignore!
	if (!(vs->uiStatusFlags & SOLDIER_VEHICLE)) return;
	const VEHICLETYPE* const v = &pVehicleList[vs->bVehicleID];

	// Loop through passengers and update each guy's position
	CFOR_ALL_PASSENGERS(v, i)
	{
		EVENT_SetSoldierPositionXY(*i, vs->dXPos, vs->dYPos, SSP_NONE);
	}
}


void LoadVehicleMovementInfoFromSavedGameFile(HWFILE const hFile)
{
	INT32		cnt;

	//Load in the Squad movement id's
	FileRead(hFile, gubVehicleMovementGroups, sizeof(INT8) * 5);

	for( cnt = 5; cnt <  MAX_VEHICLES; cnt++ )
	{
		// create mvt groups
		GROUP* const g = CreateNewVehicleGroupDepartingFromSector(1, 1);
		g->fPersistant = TRUE;
		gubVehicleMovementGroups[cnt] = g->ubGroupID;
	}
}


void NewSaveVehicleMovementInfoToSavedGameFile(HWFILE const hFile)
{
	//Save all the vehicle movement id's
	FileWrite(hFile, gubVehicleMovementGroups, sizeof(INT8) * MAX_VEHICLES);
}


void NewLoadVehicleMovementInfoFromSavedGameFile(HWFILE const hFile)
{
	//Load in the Squad movement id's
	FileRead(hFile, gubVehicleMovementGroups, sizeof(INT8) * MAX_VEHICLES);
}


BOOLEAN OKUseVehicle( UINT8 ubProfile )
{
	if ( ubProfile == PROF_HUMMER )
	{
		return( CheckFact( FACT_OK_USE_HUMMER, NO_PROFILE ) );
	}
	else if ( ubProfile == PROF_ICECREAM )
	{
		return( CheckFact( FACT_OK_USE_ICECREAM, NO_PROFILE ) );
	}
	else if ( ubProfile == PROF_HELICOPTER )
	{
		// don't allow mercs to get inside vehicle if it's grounded (enemy controlled, Skyrider owed money, etc.)
		return( CanHelicopterFly() );
	}
	else
	{
		return( TRUE );
	}
}


static void TeleportVehicleToItsClosestSector(const UINT8 ubGroupID)
{
	GROUP *pGroup = NULL;
	UINT32 uiTimeToNextSector;
	UINT32 uiTimeToLastSector;
	INT16 sPrevX, sPrevY, sNextX, sNextY;


	pGroup = GetGroup( ubGroupID );
	Assert( pGroup );

	Assert( pGroup->uiTraverseTime != -1 );
	Assert(pGroup->uiTraverseTime > 0 && pGroup->uiTraverseTime != TRAVERSE_TIME_IMPOSSIBLE);

	Assert( pGroup->uiArrivalTime >= GetWorldTotalMin() );
	uiTimeToNextSector = pGroup->uiArrivalTime - GetWorldTotalMin();

	Assert( pGroup->uiTraverseTime >= uiTimeToNextSector );
	uiTimeToLastSector = pGroup->uiTraverseTime - uiTimeToNextSector;

	if ( uiTimeToNextSector >= uiTimeToLastSector )
	{
		// go to the last sector
		sPrevX = pGroup->ubNextX;
		sPrevY = pGroup->ubNextY;

		sNextX = pGroup->ubSectorX;
		sNextY = pGroup->ubSectorY;
	}
	else
	{
		// go to the next sector
		sPrevX = pGroup->ubSectorX;
		sPrevY = pGroup->ubSectorY;

		sNextX = pGroup->ubNextX;
		sNextY = pGroup->ubNextY;
	}

	// make it arrive immediately, not eventually (it's driverless)
	SetGroupArrivalTime( pGroup, GetWorldTotalMin() );

	// change where it is and where it's going, then make it arrive there.  Don't check for battle
	PlaceGroupInSector(pGroup, sPrevX, sPrevY, sNextX, sNextY, 0, FALSE);
}


void AddVehicleFuelToSave( )
{
	CFOR_ALL_VEHICLES(v)
	{
		if (VEHICLE2ID(v) == iHelicopterVehicleId) continue;
		SOLDIERTYPE& vs = GetSoldierStructureForVehicle(*v);
		// Init fuel!
		vs.sBreathRed = 10000;
		vs.bBreath    = 100;
	}
}


static BOOLEAN CanSoldierDriveVehicle(const SOLDIERTYPE* const pSoldier, const INT32 iVehicleId, const BOOLEAN fIgnoreAsleep)
{
	Assert( pSoldier );

	if ( pSoldier->bAssignment != VEHICLE )
	{
		// not in a vehicle!
		return( FALSE );
	}

	if ( pSoldier->iVehicleId != iVehicleId )
	{
		// not in THIS vehicle!
		return( FALSE );
	}

	if ( iVehicleId == iHelicopterVehicleId )
	{
		// only Skyrider can pilot the helicopter
		return( FALSE );
	}

	if (!fIgnoreAsleep && pSoldier->fMercAsleep)
	{
		// asleep!
		return( FALSE );
	}

	if( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) || AM_A_ROBOT( pSoldier ) || AM_AN_EPC( pSoldier ) )
	{
		// vehicles, robot, and EPCs can't drive!
		return (FALSE);
	}

	// too wounded to drive
	if( pSoldier->bLife < OKLIFE )
	{
		return (FALSE);
	}

	// too tired to drive
	if( pSoldier->bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM )
	{
		return (FALSE);
	}


	// yup, he could drive this vehicle
	return( TRUE );
}


static BOOLEAN OnlyThisSoldierCanDriveVehicle(const SOLDIERTYPE* pThisSoldier, INT32 iVehicleId);


BOOLEAN SoldierMustDriveVehicle(const SOLDIERTYPE* const pSoldier, const INT32 iVehicleId, const BOOLEAN fTryingToTravel)
{
	Assert( pSoldier );

	VEHICLETYPE const& v = GetVehicle(iVehicleId);

	// if vehicle is not going anywhere, then nobody has to be driving it!
	// need the path length check in case we're doing a test while actually in a sector even though we're moving!
	if (!fTryingToTravel && !v.fBetweenSectors && GetLengthOfPath(v.pMercPath) == 0)
	{
		return( FALSE );
	}

	// if he CAN drive it (don't care if he is currently asleep)
	if ( CanSoldierDriveVehicle( pSoldier, iVehicleId, TRUE ) )
	{
		// and he's the ONLY one aboard who can do so
		if ( OnlyThisSoldierCanDriveVehicle( pSoldier, iVehicleId ) )
		{
			return( TRUE );
		}
		// (if there are multiple possible drivers, than the assumption is that this guy ISN'T driving, so he CAN sleep)
	}

	return( FALSE );
}


static BOOLEAN OnlyThisSoldierCanDriveVehicle(const SOLDIERTYPE* const pThisSoldier, const INT32 iVehicleId)
{
	CFOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// skip checking THIS soldier, we wanna know about everyone else
		if ( pSoldier == pThisSoldier )
		{
			continue;
		}

		// don't count mercs who are asleep here
		if (CanSoldierDriveVehicle(pSoldier, iVehicleId, FALSE))
		{
			// this guy can drive it, too
			return FALSE;
		}
	}

	// you're da man!
	return( TRUE );
}


BOOLEAN IsSoldierInThisVehicleSquad(const SOLDIERTYPE* const pSoldier, const INT8 bSquadNumber)
{
	INT32 iVehicleId;


	Assert( pSoldier );
	Assert( ( bSquadNumber >= 0 ) && ( bSquadNumber < NUMBER_OF_SQUADS ) );

	// not in a vehicle?
	if( pSoldier->bAssignment != VEHICLE )
	{
		return( FALSE );
	}

	// get vehicle ID
	iVehicleId = pSoldier->iVehicleId;

	// if in helicopter
	if ( iVehicleId == iHelicopterVehicleId )
	{
		// they don't get a squad #
		return( FALSE );
	}

	SOLDIERTYPE const& vs = GetSoldierStructureForVehicle(GetVehicle(iVehicleId));

	// check squad vehicle is on
	if (vs.bAssignment != bSquadNumber)
	{
		return( FALSE );
	}


	// yes, he's in a vehicle assigned to this squad
	return( TRUE );
}


SOLDIERTYPE* PickRandomPassengerFromVehicle(SOLDIERTYPE* const pSoldier)
{
	// If not a vehicle, ignore!
	if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) return NULL;

	const VEHICLETYPE* const v = &pVehicleList[pSoldier->bVehicleID];

	size_t       n_mercs = 0;
	SOLDIERTYPE* mercs_in_vehicle[20];
	CFOR_ALL_PASSENGERS(v, i) mercs_in_vehicle[n_mercs++] = *i;

	return n_mercs == 0 ? NULL : mercs_in_vehicle[Random(n_mercs)];
}


BOOLEAN DoesVehicleGroupHaveAnyPassengers(GROUP const* const g)
{
	return GetNumberInVehicle(GetVehicleFromMvtGroup(g)) != 0;
}


void HandleVehicleMovementSound(const SOLDIERTYPE* const s, const BOOLEAN fOn)
{
	VEHICLETYPE* const v = &pVehicleList[s->bVehicleID];
	if (fOn)
	{
		if (v->iMovementSoundID == NO_SAMPLE)
		{
			v->iMovementSoundID = PlayLocationJA2Sample(s->sGridNo, g_vehicle_type_info[v->ubVehicleType].move_sound, HIGHVOLUME, 1);
		}
	}
	else
	{
		if (v->iMovementSoundID != NO_SAMPLE)
		{
			SoundStop(v->iMovementSoundID);
			v->iMovementSoundID = NO_SAMPLE;
		}
	}
}


UINT8 GetVehicleArmourType(const UINT8 vehicle_id)
{
	return Item[g_vehicle_type_info[pVehicleList[vehicle_id].ubVehicleType].armour_type].ubClassIndex;
}


UINT8 GetVehicleSeats(const VEHICLETYPE* const v)
{
	return g_vehicle_type_info[v->ubVehicleType].seats;
}
