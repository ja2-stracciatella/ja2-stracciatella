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

#include "ContentManager.h"
#include "GameInstance.h"
#include "ShippingDestinationModel.h"
#include "VehicleModel.h"

#include <stdexcept>
#include <vector>


INT8 gubVehicleMovementGroups[ MAX_VEHICLES ];

// the list of vehicle slots
std::vector<VEHICLETYPE> pVehicleList;


// Loop through and create a few soldier squad ID's for vehicles ( max # 3 )
void InitVehicles(void)
{
	INT32 cnt;
	for( cnt = 0; cnt <  MAX_VEHICLES; cnt++ )
	{
		// create mvt groups
		GROUP* const g = CreateNewVehicleGroupDepartingFromSector(SGPSector(1, 1));
		g->fPersistant = TRUE;
		gubVehicleMovementGroups[cnt] = g->ubGroupID;
	}
}


void SetVehicleValuesIntoSoldierType(SOLDIERTYPE* const vs)
{
	const VEHICLETYPE* const v = &pVehicleList[vs->bVehicleID];
	vs->name = zVehicleName[v->ubVehicleType];
	vs->ubProfile           = GCM->getVehicle(v->ubVehicleType)->profile;
	vs->sBreathRed          = 10000; // Init fuel
	vs->bBreath             = 100;
	vs->ubWhatKindOfMercAmI = MERC_TYPE__VEHICLE;
}


INT32 AddVehicleToList(const SGPSector& sMap, const INT16 sGridNo, const UINT8 ubType)
{
	INT32 vid;
	for (vid = 0;; ++vid)
	{
		Assert(pVehicleList.size() <= INT32_MAX);
		if (vid == static_cast<INT32>(pVehicleList.size()))
		{
			pVehicleList.push_back(VEHICLETYPE{});
			break;
		}
		if (!pVehicleList[vid].fValid) break;
	}
	VEHICLETYPE* const v = &pVehicleList[vid];

	// found a slot
	*v = VEHICLETYPE{};
	v->ubMovementGroup = 0;
	v->sSector         = sMap;
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
	g->ubTransportationMask = GCM->getVehicle(ubType)->movement_type;
	g->ubSector             = sMap;
	g->ubNext               = sMap;
	g->uiTraverseTime       = 0;
	g->uiArrivalTime        = 0;

	return vid;
}


void RemoveVehicleFromList(VEHICLETYPE& v)
{
	v.pMercPath = ClearStrategicPathList(v.pMercPath, 0);
	v = VEHICLETYPE{};
}


void ClearOutVehicleList(void)
{
	if (pVehicleList.size() == 0) return;

	FOR_EACH_VEHICLE(v)
	{
		v.pMercPath = ClearStrategicPathList(v.pMercPath, 0);
	}

	pVehicleList.clear();
}


bool IsThisVehicleAccessibleToSoldier(SOLDIERTYPE const& s, VEHICLETYPE const& v)
{
	return !s.fBetweenSectors &&
		!v.fBetweenSectors &&
		s.sSector == v.sSector &&
		OKUseVehicle(GCM->getVehicle(v.ubVehicleType)->profile);
}


static bool AddSoldierToVehicle(SOLDIERTYPE& s, VEHICLETYPE& v)
{
	// ok now check if any free slots in the vehicle

	SOLDIERTYPE* vs = 0;
	if (!IsHelicopter(v))
	{
		vs = &GetSoldierStructureForVehicle(v);
		if (vs->bTeam != OUR_TEAM)
		{
			// Change sides
			vs = ChangeSoldierTeam(vs, OUR_TEAM);
			// add it to mapscreen list
			fReBuildCharacterList = TRUE;
		}

		// If vehicle is empty, add to unique squad now that it has somebody in it!
		if (GetNumberInVehicle(v) == 0)
		{
			// 2 ) Add to unique squad...
			AddCharacterToUniqueSquad(vs);

			// ATE: OK funcky stuff here!
			// We have now a guy on a squad group, remove him!
			RemovePlayerFromGroup(*vs);

			// I really have vehicles.
			// ONLY add to vehicle group once!
			GROUP& g = *GetGroup(v.ubMovementGroup);
			if (!DoesPlayerExistInPGroup(g, *vs))
			{
				//NOW.. add guy to vehicle group....
				AddPlayerToGroup(g, *vs);
			}
			else
			{
				vs->ubGroupID = v.ubMovementGroup;
			}
		}
	}

	// check if the grunt is already here
	CFOR_EACH_PASSENGER(v, i)
	{
		if (*i == &s) return true; // guy found, no need to add
	}

	if (vs)
	{
		// can't call SelectSoldier in mapscreen, that will initialize interface panels!!!
		if (guiCurrentScreen == GAME_SCREEN)
		{
			SelectSoldier(vs, SELSOLDIER_FORCE_RESELECT);
		}

		PlayLocationJA2Sample(vs->sGridNo, GCM->getVehicle(v.ubVehicleType)->enter_sound, HIGHVOLUME, 1);
	}

	INT32 const seats = GetVehicleSeats(v);
	for (INT32 i = 0; i < seats; ++i)
	{
		if (v.pPassengers[i]) continue;
		v.pPassengers[i] = &s;

		if (s.bAssignment == VEHICLE)
		{
			TakeSoldierOutOfVehicle(&s);
			// NOTE: This will leave the soldier on a squad.  Must be done PRIOR TO
			// and in AS WELL AS the call to RemoveCharacterFromSquads() that's coming
			// up, to permit direct vehicle->vehicle reassignment!
		}

		// if in a squad, remove from squad, if not, then check if in mvt group, if
		// so, move and destroy group
		if (s.bAssignment < ON_DUTY)
		{
			RemoveCharacterFromSquads(&s);
		}
		else if (s.ubGroupID != 0)
		{
			// destroy group and set to zero
			RemoveGroup(*GetGroup(s.ubGroupID));
			s.ubGroupID = 0;
		}

		if (s.bAssignment != VEHICLE || s.iVehicleId != VEHICLE2ID(v))
		{
			SetTimeOfAssignmentChangeForMerc(&s);
		}

		ChangeSoldiersAssignment(&s, VEHICLE);

		s.iVehicleId = VEHICLE2ID(v);

		// if vehicle is part of mvt group, then add character to mvt group
		if (v.ubMovementGroup != 0)
		{
			AddPlayerToGroup(*GetGroup(v.ubMovementGroup), s);
		}

		// Are we the first?
		s.uiStatusFlags |= GetNumberInVehicle(v) == 1 ?
			SOLDIER_DRIVER : SOLDIER_PASSENGER;

		RemoveSoldierFromGridNo(s);

		if (vs)
		{
			// Set gridno for vehicle.....
			EVENT_SetSoldierPositionXY(&s, vs->dXPos, vs->dYPos, SSP_NONE);

			// Stop from any movement.....
			EVENT_StopMerc(&s);

			// can't call SetCurrentSquad OR SelectSoldier in mapscreen,
			// that will initialize interface panels!!!
			if (guiCurrentScreen == GAME_SCREEN)
			{
				SetCurrentSquad(vs->bAssignment, TRUE);
			}
		}

		return true;
	}

	// no slots, leave
	return false;
}


void SetSoldierExitHelicopterInsertionData(SOLDIERTYPE* const s)
{
	if (s->bInSector) return;

	auto shippingDest = GCM->getPrimaryShippingDestination();
	if (s->sSector == shippingDest->deliverySector)
	{
		// This is Drassen, make insertion gridno specific
		s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		s->usStrategicInsertionData = 10125;
	}
	else
	{
		// Not anything different here - just use center gridno
		s->ubStrategicInsertionCode = INSERTION_CODE_CENTER;
	}
}


static void TeleportVehicleToItsClosestSector(UINT8 ubGroupID);


// remove soldier from vehicle
static bool RemoveSoldierFromVehicle(SOLDIERTYPE& s)
{
	VEHICLETYPE& v = GetVehicle(s.iVehicleId);

	// now look for the grunt
	INT32 const seats = GetVehicleSeats(v);
	for (INT32 i = 0;; ++i)
	{
		if (i == seats) return false;
		if (v.pPassengers[i] != &s) continue;
		v.pPassengers[i] = 0;
		break;
	}

	RemovePlayerFromGroup(s);

	s.ubGroupID      = 0;
	s.sSector        = v.sSector;
	s.uiStatusFlags &= ~(SOLDIER_DRIVER | SOLDIER_PASSENGER);

	if (IsHelicopter(v))
	{
		// The vehicle the helicopter? It can continue moving when no soldiers
		// aboard (Skyrider remains)
		if (s.bLife >= OKLIFE)
		{
			// Mark the sector as visited (flying around in the chopper doesn't, so
			// this does it as soon as we get off it)
			SetSectorFlag(s.sSector, SF_ALREADY_VISITED);
		}

		SetSoldierExitHelicopterInsertionData(&s);

		// Update in sector if this is the current sector
		if (s.sSector == gWorldSector)
		{
			UpdateMercInSector(s, gWorldSector);
		}
	}
	else
	{
		// check if anyone left in vehicle
		CFOR_EACH_PASSENGER(v, i) return true;

		SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);

		// and he has a route set
		if (GetLengthOfMercPath(&vs) > 0)
		{
			// cancel the entire path (also handles reversing directions)
			CancelPathForVehicle(v, FALSE);
		}

		if (v.fBetweenSectors)
		{
			// The vehicle was abandoned between sectors. Teleport it to the closer of
			// its current and next sectors (it beats having it arrive empty later)
			TeleportVehicleToItsClosestSector(vs.ubGroupID);
		}

		// Remove vehicle from squad
		RemoveCharacterFromSquads(&vs);
		// ATE: Add him back to vehicle group!
		GROUP& g = *GetGroup(v.ubMovementGroup);
		if (!DoesPlayerExistInPGroup(g, vs)) AddPlayerToGroup(g, vs);
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
	Assert(pVehicleList.size() <= INT32_MAX);
	if (0 <= vehicle_id && vehicle_id < static_cast<INT32>(pVehicleList.size()))
	{
		VEHICLETYPE& v = pVehicleList[vehicle_id];
		if (v.fValid) return v;
	}
	throw std::logic_error("Invalid vehicle ID");
}


VEHICLETYPE& GetVehicleFromMvtGroup(GROUP const& g)
{
	// given the id of a mvt group, find a vehicle in this group
	FOR_EACH_VEHICLE(v)
	{
		if (v.ubMovementGroup == g.ubGroupID) return v;
	}
	throw std::logic_error("Group does not contain a vehicle");
}


// Kill this person in the vehicle
static bool KillPersonInVehicle(SOLDIERTYPE& s)
{
	if (s.bLife == 0) return false; // Guy is dead, leave
	// Otherwise hurt him
	SoldierTakeDamage(&s, 100, 100, TAKE_DAMAGE_BLOODLOSS, 0);
	return true;
}


BOOLEAN KillAllInVehicle(VEHICLETYPE const& v)
{
	// go through list of occupants and kill them
	CFOR_EACH_PASSENGER(v, i)
	{
		if (!KillPersonInVehicle(**i)) return FALSE;
	}
	return TRUE;
}


INT32 GetNumberInVehicle(VEHICLETYPE const& v)
{
	// go through list of occupants in vehicles and count them
	INT32 count = 0;
	CFOR_EACH_PASSENGER(v, i) ++count;
	return count;
}


INT32 GetNumberOfNonEPCsInVehicle( INT32 iId )
{
	// go through list of occupants in vehicles and count them
	VEHICLETYPE const& v = GetVehicle(iId);

	INT32 count = 0;
	CFOR_EACH_PASSENGER(v, i)
	{
		const SOLDIERTYPE* const s = *i;
		if (!AM_AN_EPC(s)) ++count;
	}
	return count;
}


BOOLEAN IsRobotControllerInVehicle( INT32 iId )
{
	VEHICLETYPE const& v = GetVehicle(iId);
	CFOR_EACH_PASSENGER(v, i)
	{
		if (ControllingRobot(*i)) return TRUE;
	}
	return FALSE;
}


bool AnyAccessibleVehiclesInSoldiersSector(SOLDIERTYPE const& s)
{
	CFOR_EACH_VEHICLE(v)
	{
		if (IsThisVehicleAccessibleToSoldier(s, v)) return true;
	}
	return false;
}


bool IsEnoughSpaceInVehicle(VEHICLETYPE const& v)
{
	return GetNumberInVehicle(v) != GetVehicleSeats(v);
}


BOOLEAN TakeSoldierOutOfVehicle(SOLDIERTYPE* const s)
{
	// if not in vehicle, don't take out, not much point, now is there?
	if (s->bAssignment != VEHICLE) return FALSE;

	if (s->sSector == gWorldSector &&
		s->bInSector &&
		!InHelicopter(*s)) // helicopter isn't a soldiertype instance
	{
		return ExitVehicle(s);
	}
	else
	{
		return RemoveSoldierFromVehicle(*s);
	}
}


bool PutSoldierInVehicle(SOLDIERTYPE& s, VEHICLETYPE& v)
{
	if (!AddSoldierToVehicle(s, v)) return false;

	if (s.sSector == gWorldSector &&
		!IsHelicopter(v) &&
		guiCurrentScreen == GAME_SCREEN)
	{
		SetCurrentInterfacePanel(TEAM_PANEL);
	}

	return true;
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
	HandleSight(*s, SIGHT_LOOK | SIGHT_RADIO);

	AddCharacterToUniqueSquad(s);

	// can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface panels!!!
	if (guiCurrentScreen == GAME_SCREEN)
	{
		SetCurrentSquad(s->bAssignment, TRUE);
		SelectSoldier(s, SELSOLDIER_FORCE_RESELECT);
	}

	UINT8 ubVehicleType = pVehicleList[vs.bVehicleID].ubVehicleType;
	PlayLocationJA2Sample(vs.sGridNo, GCM->getVehicle(ubVehicleType)->enter_sound, HIGHVOLUME, 1);
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

		KillAllInVehicle(v);
	}
}


bool DoesVehicleNeedAnyRepairs(VEHICLETYPE const& v)
{
	// Skyrider isn't damagable/repairable
	if (IsHelicopter(v)) return false;

	// get the vehicle soldiertype
	SOLDIERTYPE const& vs = GetSoldierStructureForVehicle(v);
	return vs.bLife != vs.bLifeMax;
}


INT8 RepairVehicle(VEHICLETYPE const& v, INT8 const bRepairPtsLeft, BOOLEAN* const pfNothingToRepair)
{
	INT8 bRepairPtsUsed = 0;
	INT8 bOldLife;

	if (!DoesVehicleNeedAnyRepairs(v)) return bRepairPtsUsed;

	// get the vehicle soldiertype
	SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);

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
	FOR_EACH_SOLDIER(s)
	{
		if (!(s->uiStatusFlags & SOLDIER_VEHICLE)) continue;
		if (s->bVehicleID != VEHICLE2ID(v))        continue;
		return *s;
	}
	throw std::logic_error("Vehicle has no corresponding soldier");
}


void SaveVehicleInformationToSaveGameFile(HWFILE const f)
{
	//Save the number of elements
	Assert(pVehicleList.size() <= UINT8_MAX);
	UINT8 numVehicles = static_cast<UINT8>(pVehicleList.size());
	f->write(&numVehicles, sizeof(UINT8));

	//loop through all the vehicles and save each one
	for (const VEHICLETYPE& v : pVehicleList)
	{
		//save if the vehicle spot is valid
		f->write(&v.fValid, sizeof(BOOLEAN));
		if (!v.fValid) continue;

		InjectVehicleTypeIntoFile(f, &v);
		SaveMercPath(f, v.pMercPath);
	}
}


void LoadVehicleInformationFromSavedGameFile(HWFILE const hFile, UINT32 const uiSavedGameVersion)
{
	ClearOutVehicleList();

	//Load the number of elements
	UINT8 numVehicles = 0;
	hFile->read(&numVehicles, sizeof(UINT8));
	if (numVehicles == 0) return;

	//allocate memory to hold the vehicle list
	pVehicleList.assign(numVehicles, VEHICLETYPE{});

	//loop through all the vehicles and load each one
	for (VEHICLETYPE& v : pVehicleList)
	{
		//Load if the vehicle spot is valid
		hFile->read(&v.fValid, sizeof(BOOLEAN));
		if (!v.fValid) continue;

		ExtractVehicleTypeFromFile(hFile, &v, uiSavedGameVersion);
		LoadMercPath(hFile, &v.pMercPath);
	}
}


void SetVehicleSectorValues(VEHICLETYPE& v, const SGPSector& sMap)
{
	v.sSector = sMap;

	ProfileID vehicleProfile = GCM->getVehicle(v.ubVehicleType)->profile;
	MERCPROFILESTRUCT& p = GetProfile(vehicleProfile);
	p.sSector = sMap;

	// Go through list of mercs in vehicle and set all their states as arrived
	CFOR_EACH_PASSENGER(v, i)
	{
		SOLDIERTYPE& s = **i;
		s.sSector        = sMap;
		s.fBetweenSectors = FALSE;
	}
}


void UpdateAllVehiclePassengersGridNo(SOLDIERTYPE* const vs)
{
	// If not a vehicle, ignore!
	if (!(vs->uiStatusFlags & SOLDIER_VEHICLE)) return;
	VEHICLETYPE const& v = pVehicleList[vs->bVehicleID];

	// Loop through passengers and update each guy's position
	CFOR_EACH_PASSENGER(v, i)
	{
		EVENT_SetSoldierPositionXY(*i, vs->dXPos, vs->dYPos, SSP_NONE);
	}
}


void LoadVehicleMovementInfoFromSavedGameFile(HWFILE const hFile)
{
	INT32 cnt;

	//Load in the Squad movement id's
	hFile->read(gubVehicleMovementGroups, sizeof(INT8) * 5);

	for( cnt = 5; cnt <  MAX_VEHICLES; cnt++ )
	{
		// create mvt groups
		GROUP* const g = CreateNewVehicleGroupDepartingFromSector(SGPSector(1, 1));
		g->fPersistant = TRUE;
		gubVehicleMovementGroups[cnt] = g->ubGroupID;
	}
}


void NewSaveVehicleMovementInfoToSavedGameFile(HWFILE const hFile)
{
	//Save all the vehicle movement id's
	hFile->write(gubVehicleMovementGroups, sizeof(INT8) * MAX_VEHICLES);
}


void NewLoadVehicleMovementInfoFromSavedGameFile(HWFILE const hFile)
{
	//Load in the Squad movement id's
	hFile->read(gubVehicleMovementGroups, sizeof(INT8) * MAX_VEHICLES);
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
	GROUP  *pGroup = NULL;
	UINT32 uiTimeToNextSector;
	UINT32 uiTimeToLastSector;
	SGPSector sPrev, sNext;

	pGroup = GetGroup( ubGroupID );
	Assert( pGroup );

	Assert(pGroup->uiTraverseTime > 0 && pGroup->uiTraverseTime != TRAVERSE_TIME_IMPOSSIBLE);

	Assert( pGroup->uiArrivalTime >= GetWorldTotalMin() );
	uiTimeToNextSector = pGroup->uiArrivalTime - GetWorldTotalMin();

	Assert( pGroup->uiTraverseTime >= uiTimeToNextSector );
	uiTimeToLastSector = pGroup->uiTraverseTime - uiTimeToNextSector;

	if ( uiTimeToNextSector >= uiTimeToLastSector )
	{
		// go to the last sector
		sPrev = pGroup->ubNext;
		sNext = pGroup->ubSector;
	}
	else
	{
		// go to the next sector
		sPrev = pGroup->ubSector;
		sNext = pGroup->ubNext;
	}

	// make it arrive immediately, not eventually (it's driverless)
	pGroup->setArrivalTime(GetWorldTotalMin());

	// change where it is and where it's going, then make it arrive there.  Don't check for battle
	PlaceGroupInSector(*pGroup, sPrev, sNext, false);
}


void AddVehicleFuelToSave( )
{
	CFOR_EACH_VEHICLE(v)
	{
		if (IsHelicopter(v)) continue;
		SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);
		// Init fuel!
		vs.sBreathRed = 10000;
		vs.bBreath    = 100;
	}
}


static bool CanSoldierDriveVehicle(SOLDIERTYPE const& s, INT32 const vehicle_id, bool const ignore_asleep)
{
	return s.bAssignment == VEHICLE            && // In a vehicle?
		vehicle_id == s.iVehicleId         && // In this vehicle?
		vehicle_id != iHelicopterVehicleId && // Only Skyrider can pilot the helicopter
		(ignore_asleep || !s.fMercAsleep)  &&
		!IsMechanical(s)                   && // Vehicles, robot, and EPCs can't drive
		!AM_AN_EPC(&s)                     &&
		s.bLife >= OKLIFE                  && // Too wounded to drive?
		s.bBreathMax > BREATHMAX_ABSOLUTE_MINIMUM; // Too tired to drive?
}


static bool OnlyThisSoldierCanDriveVehicle(SOLDIERTYPE const& s, INT32 const vehicle_id)
{
	CFOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE const& other = *i;
		// Skip checking this soldier, we want to know about everyone else
		if (&other == &s) continue;
		// Don't count mercs who are asleep here
		if (!CanSoldierDriveVehicle(other, vehicle_id, false)) continue;
		// This guy can drive it, too
		return false;
	}
	return true;
}


bool SoldierMustDriveVehicle(SOLDIERTYPE const& s, bool const trying_to_travel)
{
	INT32       const  vehicle_id = s.iVehicleId;
	VEHICLETYPE const& v          = GetVehicle(vehicle_id);

	// If vehicle is not going anywhere, then nobody has to be driving it. Need
	// the path length check in case we're doing a test while actually in a sector
	// even though we're moving.
	if (!trying_to_travel && !v.fBetweenSectors && GetLengthOfPath(v.pMercPath) == 0) return false;

	// Can he drive it (don't care if he is currently asleep) and is he the only
	// one aboard who can do so? If there are multiple possible drivers, than the
	// assumption is that this guy isn't driving, so he can sleep.
	if (!CanSoldierDriveVehicle(s, vehicle_id, true))   return false;
	if (!OnlyThisSoldierCanDriveVehicle(s, vehicle_id)) return false;
	return true;
}


BOOLEAN IsSoldierInThisVehicleSquad(const SOLDIERTYPE* const pSoldier, const INT8 bSquadNumber)
{
	Assert( pSoldier );
	Assert( ( bSquadNumber >= 0 ) && ( bSquadNumber < NUMBER_OF_SQUADS ) );

	// not in a vehicle?
	if( pSoldier->bAssignment != VEHICLE )
	{
		return( FALSE );
	}

	if (InHelicopter(*pSoldier)) return FALSE; // they don't get a squad #

	SOLDIERTYPE const& vs = GetSoldierStructureForVehicle(GetVehicle(pSoldier->iVehicleId));

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

	VEHICLETYPE const& v = pVehicleList[pSoldier->bVehicleID];

	INT32       n_mercs = 0;
	SOLDIERTYPE* mercs_in_vehicle[20];
	CFOR_EACH_PASSENGER(v, i) mercs_in_vehicle[n_mercs++] = *i;

	return n_mercs == 0 ? NULL : mercs_in_vehicle[Random(n_mercs)];
}


bool DoesVehicleGroupHaveAnyPassengers(GROUP const& g)
{
	return GetNumberInVehicle(GetVehicleFromMvtGroup(g)) != 0;
}


void HandleVehicleMovementSound(const SOLDIERTYPE* const s, const BOOLEAN fOn)
{
	VEHICLETYPE* const v = &pVehicleList[s->bVehicleID];
	if (fOn)
	{
		if (v->uiMovementSoundID == NO_SAMPLE)
		{
			v->uiMovementSoundID = PlayLocationJA2Sample(s->sGridNo, GCM->getVehicle(v->ubVehicleType)->move_sound, HIGHVOLUME, 1);
		}
	}
	else
	{
		if (v->uiMovementSoundID != NO_SAMPLE)
		{
			SoundStop(v->uiMovementSoundID);
			v->uiMovementSoundID = NO_SAMPLE;
		}
	}
}


UINT8 GetVehicleArmourType(const UINT8 vehicle_id)
{
	auto armourItemIndex = GCM->getVehicle(pVehicleList[vehicle_id].ubVehicleType)->armour_type;
	return GCM->getItem(armourItemIndex)->getClassIndex();
}


UINT8 GetVehicleSeats(VEHICLETYPE const& v)
{
	return GCM->getVehicle(v.ubVehicleType)->seats;
}
