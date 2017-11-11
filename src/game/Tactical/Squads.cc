#include <stdexcept>

#include "Interface_Panels.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "Squads.h"
#include "Strategic_Pathing.h"
#include "StrategicMap.h"
#include "Faces.h"
#include "Strategic_Movement.h"
#include "Assignments.h"
#include "Overhead.h"
#include "Interface.h"
#include "Vehicles.h"
#include "Map_Screen_Helicopter.h"
#include "Soldier_Profile.h"
#include "Debug.h"
#include "JAScreens.h"
#include "Soldier_Macros.h"
#include "ScreenIDs.h"
#include "FileMan.h"


// squad array
SOLDIERTYPE *Squad[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];

// list of dead guys for squads...in id values -> -1 means no one home
INT16 sDeadMercs[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];

// the movement group ids
INT8 SquadMovementGroups[ NUMBER_OF_SQUADS ];

BOOLEAN fExitingVehicleToSquad = FALSE;


INT32 iCurrentTacticalSquad = FIRST_SQUAD;

void InitSquads( void )
{
	// init the squad lists to NULL ptrs.
	INT32 iCounter =0;

	// null each list of ptrs.
	for( iCounter = 0; iCounter <  NUMBER_OF_SQUADS; iCounter++ )
	{
		FOR_EACH_SLOT_IN_SQUAD(i, iCounter)
		{
			*i = 0;
		}

		// create mvt groups
		GROUP* const g = CreateNewPlayerGroupDepartingFromSector(1, 1);
		g->fPersistant = TRUE;
		SquadMovementGroups[iCounter] = g->ubGroupID;
	}

	memset( sDeadMercs, -1, sizeof( INT16 ) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD );
}

BOOLEAN IsThisSquadFull( INT8 bSquadValue )
{
	// run through entries in the squad list, make sure there is a free entry
	FOR_EACH_SLOT_IN_SQUAD(i, bSquadValue)
	{
		if (!*i) return FALSE;
	}
	return TRUE;
}

INT8 GetFirstEmptySquad( void )
{
	UINT8 ubCounter = 0;

	for( ubCounter = 0; ubCounter < NUMBER_OF_SQUADS; ubCounter++ )
	{
		if (SquadIsEmpty(ubCounter)) return ubCounter;
	}

	// not found - none are completely empty (shouldn't ever happen!)
	throw std::logic_error("Found no empty squad");
}


static BOOLEAN CopyPathOfSquadToCharacter(SOLDIERTYPE* pCharacter, INT8 bSquadValue);


BOOLEAN AddCharacterToSquad(SOLDIERTYPE* const s, INT8 const bSquadValue)
{
	// add character to squad...return success or failure
	// run through list of people in squad, find first free slo

	if (fExitingVehicleToSquad) return FALSE;

	// ATE: If any vehicle exists in this squad AND we're not set to
	// a driver or or passenger, when return false
	if (DoesVehicleExistInSquad(bSquadValue))
	{
		// We're not allowing anybody to go on a vehicle if they are not passengers!
		// NB: We obviously need to make sure that REAL passengers have their
		// flags set before adding them to a squad!
		if (!(s->uiStatusFlags & (SOLDIER_PASSENGER | SOLDIER_DRIVER | SOLDIER_VEHICLE)))
		{
			return FALSE;
		}
	}

	// if squad is on the move, can't add someone
	if (IsThisSquadOnTheMove(bSquadValue)) return FALSE;

	FOR_EACH_SLOT_IN_SQUAD(i, bSquadValue)
	{
		SOLDIERTYPE const* const t = *i;
		// check if on current squad and current slot?
		// 'successful of sorts, if there, then he's 'added'
		if (t == s) return TRUE;

		if (t) continue;
		// free slot, add here

		// check if squad empty, if not check sector x,y,z are the same as this guys
		INT16 sX;
		INT16 sY;
		INT8  bZ;
		if (SectorSquadIsIn(bSquadValue, &sX, &sY, &bZ) &&
			(s->sSectorX != sX || s->sSectorY != sY || s->bSectorZ != bZ))
		{
			return FALSE;
		}

		RemoveCharacterFromSquads(s);

		// copy path of squad to this char
		CopyPathOfSquadToCharacter(s, bSquadValue);

		// check if old mvt group
		if (s->ubGroupID != 0)
		{
			// in valid group, remove from that group
			RemovePlayerFromGroup(*s);

			// character not on a reserved group
			if (s->bAssignment >= ON_DUTY && s->bAssignment != VEHICLE)
			{
				// if valid group, delete it
				GROUP* const pGroup = GetGroup(s->ubGroupID);
				if (pGroup) RemoveGroup(*pGroup);
			}
		}

		GROUP& g = *GetGroup(SquadMovementGroups[bSquadValue]);
		if (s->bAssignment != VEHICLE || s->iVehicleId == -1)
		{
			AddPlayerToGroup(g, *s);
			SetGroupSectorValue(s->sSectorX, s->sSectorY, s->bSectorZ, g);
		}
		else if (InHelicopter(*s))
		{
			// if creating a new squad from guys exiting the chopper
			BOOLEAN const fNewSquad = SquadIsEmpty(bSquadValue);

			RemoveSoldierFromHelicopter(s);

			AddPlayerToGroup(g, *s);
			SetGroupSectorValue(s->sSectorX, s->sSectorY, s->bSectorZ, g);

			// if we've just started a new squad
			if (fNewSquad)
			{
				// set mvt group for
				GROUP const* const pGroup = GetGroup(GetHelicopter().ubMovementGroup);
				Assert(pGroup);
				if (pGroup)
				{
					// set where it is and where it's going, then make it arrive there.  Don't check for battle
					PlaceGroupInSector(g, pGroup->ubPrevX, pGroup->ubPrevY, pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, false); // XXX TODO001D
				}
			}
		}
		else
		{
			fExitingVehicleToSquad = TRUE;
			// remove from vehicle
			TakeSoldierOutOfVehicle(s);
			fExitingVehicleToSquad = FALSE;

			AddPlayerToGroup(g, *s);
			SetGroupSectorValue(s->sSectorX, s->sSectorY, s->bSectorZ, g);
		}

		*i = s;

		if (s->bAssignment != bSquadValue)
		{
			// check to see if we should wake them up
			if (s->fMercAsleep) SetMercAwake(s, FALSE, FALSE);
			SetTimeOfAssignmentChangeForMerc(s);
		}

		// set squad value
		ChangeSoldiersAssignment(s, bSquadValue);

		if (SquadIsEmpty(iCurrentTacticalSquad)) SetCurrentSquad(bSquadValue, TRUE);

		if (bSquadValue == iCurrentTacticalSquad) CheckForAndAddMercToTeamPanel(s);

		if (s == GetSelectedMan()) SetCurrentSquad(bSquadValue, TRUE);

		return TRUE;
	}

	return FALSE;
}


// find the first slot we can fit the guy in
void AddCharacterToAnySquad(SOLDIERTYPE* const pCharacter)
{
	// add character to any squad, if character is assigned to a squad, returns TRUE
	INT8 bCounter = 0;
	INT8 bFirstEmptySquad = -1;


	// remove them from current squad
	RemoveCharacterFromSquads( pCharacter ); // REDUNDANT AddCharacterToSquad()

	// first look for a compatible NON-EMPTY squad (don't start new squad if we don't have to)
	for( bCounter = 0; bCounter < NUMBER_OF_SQUADS; bCounter++ )
	{
		if (!SquadIsEmpty(bCounter))
		{
			if (AddCharacterToSquad(pCharacter, bCounter))
			{
				return;
			}
		}
		else
		{
			if ( bFirstEmptySquad == -1 )
			{
				bFirstEmptySquad = bCounter;
			}
		}
	}

	// no non-empty compatible squads were found

	// try the first empty one (and there better be one)
	if ( bFirstEmptySquad != -1 )
	{
		if (AddCharacterToSquad(pCharacter, bFirstEmptySquad))
		{
			return;
		}
	}

	throw std::logic_error("Failed to add character to any squad");
}


// find the first slot we can fit the guy in
INT8 AddCharacterToUniqueSquad(SOLDIERTYPE* const s)
{
	RemoveCharacterFromSquads(s); // REDUNDANT AddCharacterToSquad()
	INT8 const squad = GetFirstEmptySquad();
	AddCharacterToSquad(s, squad);
	return squad;
}


BOOLEAN SquadIsEmpty( INT8 bSquadValue )
{
	// run through this squad's slots and find if they ALL are empty
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		return FALSE;
	}
	return TRUE;
}


static BOOLEAN AddDeadCharacterToSquadDeadGuys(SOLDIERTYPE* pSoldier, INT32 iSquadValue);
static void RebuildSquad(INT8 bSquadValue);
static void UpdateCurrentlySelectedMerc(SOLDIERTYPE* pSoldier, INT8 bSquadValue);


// find and remove character from any squad
BOOLEAN RemoveCharacterFromSquads(SOLDIERTYPE* const s)
{
	for (INT32 squad = 0; squad < NUMBER_OF_SQUADS; ++squad)
	{
		FOR_EACH_SLOT_IN_SQUAD(i, squad)
		{
			if (*i != s) continue;
			*i = 0;

			// Release memory for his personal path, but don't clear his group's
			// path/waypoints (pass in groupID -1).  Just because one guy leaves a
			// group is no reason to cancel movement for the rest of the group.
			s->pMercPath = ClearStrategicPathList(s->pMercPath, -1);

			RemovePlayerFromGroup(*s);
			s->ubGroupID = 0;

			if (s->fBetweenSectors && s->uiStatusFlags & SOLDIER_VEHICLE)
			{
				GROUP& g = *CreateNewPlayerGroupDepartingFromSector(s->sSectorX, s->sSectorY);
				AddPlayerToGroup(g, *s);
			}

			RebuildSquad(squad);

			if (s->bLife == 0)
			{
				AddDeadCharacterToSquadDeadGuys(s, squad);
			}

			if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && guiCurrentScreen == GAME_SCREEN)
			{
				UpdateCurrentlySelectedMerc(s, squad);
			}

			return TRUE;
		}
	}

	// not found
	return FALSE;
}


INT8 NumberOfPeopleInSquad( INT8 bSquadValue )
{
	INT8 bSquadCount = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		++bSquadCount;
	}

	// return number found
	return( bSquadCount );
}

INT8 NumberOfNonEPCsInSquad( INT8 bSquadValue )
{
	INT8 bSquadCount = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		// valid slot?
		if (!AM_AN_EPC(*i)) ++bSquadCount;
	}

	// return number found
	return( bSquadCount );
}

BOOLEAN IsRobotControllerInSquad( INT8 bSquadValue )
{
	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		// valid slot?
		if (ControllingRobot(*i)) return TRUE;
	}

	// return number found
	return( FALSE );
}


BOOLEAN SectorSquadIsIn(const INT8 bSquadValue, INT16* const sMapX, INT16* const sMapY, INT8* const sMapZ)
{
	// returns if there is anyone on the squad and what sector ( strategic ) they are in
	Assert( bSquadValue < ON_DUTY );

	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		SOLDIERTYPE const* const s = *i;
		// if valid soldier, get current sector and return
		*sMapX = s->sSectorX;
		*sMapY = s->sSectorY;
		*sMapZ = s->bSectorZ;
		return TRUE;
	}

	// return there is no squad
	return ( FALSE );
}


static BOOLEAN CopyPathOfSquadToCharacter(SOLDIERTYPE* pCharacter, INT8 bSquadValue)
{
	// copy path from squad to character
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		SOLDIERTYPE const* const t = *i;
		if (t == pCharacter) continue;
		// valid character, copy paths
		ClearStrategicPathList(pCharacter->pMercPath, 0);
		pCharacter->pMercPath = CopyPaths(t->pMercPath);
		// return success
		return TRUE;
	}

	// return failure
	return ( FALSE );
}


void CopyPathOfCharacterToSquad(SOLDIERTYPE* const pCharacter, const INT8 bSquadValue)
{
	// copy path of this character to members of squad
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		SOLDIERTYPE* const t = *i;
		if (t == pCharacter) continue;
		// valid character, copy paths
		ClearStrategicPathList(t->pMercPath, -1);
		t->pMercPath = CopyPaths(pCharacter->pMercPath);
	}
}



INT32 CurrentSquad( void )
{
	// returns which squad is current squad

	return( iCurrentTacticalSquad );
}

BOOLEAN SetCurrentSquad( INT32 iCurrentSquad, BOOLEAN fForce )
{
	// set the current tactical squad

	// ARM: can't call SetCurrentSquad() in mapscreen, it calls SelectSoldier(), that will initialize interface panels!!!
	// ATE: Adjusted conditions a bit ( sometimes were not getting selected )
	if ( guiCurrentScreen == LAPTOP_SCREEN || guiCurrentScreen == MAP_SCREEN )
	{
		return( FALSE );
	}

	// ATE; Added to allow us to have NO current squad
	if ( iCurrentSquad == NO_CURRENT_SQUAD )
	{
		// set current squad and return success
		iCurrentTacticalSquad = iCurrentSquad;

		// cleat list
		RemoveAllPlayersFromSlot( );

		// set all auto faces inactive
		SetAllAutoFacesInactive( );

		return( FALSE );
	}


	// check if valid value passed
	if( ( iCurrentSquad >= NUMBER_OF_SQUADS )||( iCurrentSquad < 0 ) )
	{
		// no
		return ( FALSE );
	}

	// check if squad is current
	if( iCurrentSquad == iCurrentTacticalSquad && !fForce )
	{
		return ( TRUE );
	}

	// set current squad and return success
	iCurrentTacticalSquad = iCurrentSquad;

	// cleat list
	RemoveAllPlayersFromSlot( );

	// set all auto faces inactive
	SetAllAutoFacesInactive( );

	if (iCurrentSquad != NO_CURRENT_SQUAD)
	{
		FOR_EACH_IN_SQUAD(i, iCurrentSquad)
		{
			// squad set, now add soldiers in
			CheckForAndAddMercToTeamPanel(*i);
		}
	}

	// check if the currently selected guy is on this squad, if not, get the first one on the new squad
	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL || sel->bAssignment != iCurrentSquad)
	{
		// ATE: Changed this to FALSE for acknowledgement sounds.. sounds bad if just starting/entering sector..
		SelectSoldier(Squad[iCurrentSquad][0], SELSOLDIER_FORCE_RESELECT);
	}

	return ( TRUE );
}

void RebuildCurrentSquad( void )
{
	// rebuilds current squad to reset faces in tactical

	// check if valid value passed
	if( ( iCurrentTacticalSquad >= NUMBER_OF_SQUADS ) || ( iCurrentTacticalSquad < 0 ) )
	{
		// no
		return;
	}

	// set default squad..just inc ase we no longer have a valid squad
	SetDefaultSquadOnSectorEntry( TRUE );

	// cleat list
	RemoveAllPlayersFromSlot( );

	// set all auto faces inactive
	SetAllAutoFacesInactive( );

	gfPausedTacticalRenderInterfaceFlags = DIRTYLEVEL2;

	if( iCurrentTacticalSquad != NO_CURRENT_SQUAD )
	{
		FOR_EACH_IN_SQUAD(i, iCurrentTacticalSquad)
		{
			// squad set, now add soldiers in
			CheckForAndAddMercToTeamPanel(*i);
		}

		for (INT32 iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; ++iCounter)
		{
			const INT16 dead_id = sDeadMercs[iCurrentTacticalSquad][iCounter];
			if (dead_id == -1) continue;

			SOLDIERTYPE* const dead_soldier = FindSoldierByProfileIDOnPlayerTeam(dead_id);
			if (!dead_soldier) continue;

			// squad set, now add soldiers in
			CheckForAndAddMercToTeamPanel(dead_soldier);
		}
	}
}


void ExamineCurrentSquadLights( void )
{
	// OK, we should add lights for any guy currently bInSector who is not bad OKLIFE...
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector && s->bLife >= OKLIFE) PositionSoldierLight(s);
	}
}


BOOLEAN IsSquadOnCurrentTacticalMap( INT32 iCurrentSquad )
{
	// check to see if this squad is on the current map

	// check if valid value passed
	if( ( iCurrentSquad >= NUMBER_OF_SQUADS ) || ( iCurrentSquad < 0 ) )
	{
		// no
		return ( FALSE );
	}

	// go through memebrs of squad...if anyone on this map, return true
	FOR_EACH_IN_SQUAD(i, iCurrentSquad)
	{
		SOLDIERTYPE const* const s = *i;
		// ATE; Added more checks here for being in sector ( fBetweenSectors and SectorZ )
		if (s->sSectorX == gWorldSectorX  &&
				s->sSectorY == gWorldSectorY  &&
				s->bSectorZ == gbWorldSectorZ &&
				!s->fBetweenSectors)
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


void SetDefaultSquadOnSectorEntry( BOOLEAN fForce )
{
	INT32 iCounter = 0;
	// check if selected squad is in current sector, if so, do nothing, if not...first first case that they are

	if (IsSquadOnCurrentTacticalMap(iCurrentTacticalSquad))
	{
		// is in sector, leave
		return;
	}

	//otherwise...

	// find first squad availiable
	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		if (IsSquadOnCurrentTacticalMap(iCounter))
		{
			// squad in sector...set as current
			SetCurrentSquad( iCounter, fForce );

			return;
		}
	}

	// If here, set to no current squad
	SetCurrentSquad( NO_CURRENT_SQUAD, FALSE );
}

INT32 GetLastSquadActive( void )
{
	// find id of last squad in the list with active mercs in it
	INT32 iCounter = 0;
	INT32 iLastSquad = 0;

	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		FOR_EACH_IN_SQUAD(i, iCounter)
		{
			iLastSquad = iCounter;
			break;
		}
	}

	return ( iLastSquad );
}


void SaveSquadInfoToSavedGameFile(HWFILE const f)
{
	// Save the squad info to the Saved Game File
	BYTE data[NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD * 12];
	BYTE* d = data;
	for (INT32 squad = 0; squad < NUMBER_OF_SQUADS; ++squad)
	{
		FOR_EACH_SLOT_IN_SQUAD(slot, squad)
		{
			SOLDIERTYPE const* const s = *slot;
			INJ_I16(d, s ? s->ubID : -1)
			INJ_SKIP(d, 10)
		}
	}
	Assert(d == endof(data));
	FileWrite(f, data, sizeof(data));

	// Save all the squad movement IDs
	FileWrite(f, SquadMovementGroups, sizeof(SquadMovementGroups));
}


void LoadSquadInfoFromSavedGameFile(HWFILE const f)
{
	// Load in the squad info
	BYTE data[NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD * 12];
	BYTE const* d = data;
	FileRead(f, data, sizeof(data));
	for (INT32 squad = 0; squad != NUMBER_OF_SQUADS; ++squad)
	{
		FOR_EACH_SLOT_IN_SQUAD(slot, squad)
		{
			INT16 id;
			EXTR_I16(d, id)
			EXTR_SKIP(d, 10)
			*slot = id != -1 ? &GetMan(id) : 0;
		}
	}
	Assert(d == endof(data));

	// Load in the Squad movement IDs
	FileRead(f, SquadMovementGroups, sizeof(SquadMovementGroups));
}


BOOLEAN IsThisSquadOnTheMove( INT8 bSquadValue )
{
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		return (*i)->fBetweenSectors;
	}
	return FALSE;
}


// rebuild this squad after someone has been removed, to 'squeeze' together any empty spots
static void RebuildSquad(INT8 const squad_id)
{
	SOLDIERTYPE** const squad = Squad[squad_id];
	SOLDIERTYPE** const end   = endof(Squad[squad_id]);
	SOLDIERTYPE**       dst   = squad;
	for (SOLDIERTYPE** i = squad; i != end; ++i)
	{
		if (!*i) continue;
		*dst++ = *i;
	}
	for (; dst != end; ++dst) *dst = 0;
}


// update current merc selected in tactical
static void UpdateCurrentlySelectedMerc(SOLDIERTYPE* pSoldier, INT8 bSquadValue)
{
	// if this squad is the current one and and the psoldier is the currently selected soldier, get rid of 'em
	if( bSquadValue != iCurrentTacticalSquad )
	{
		return;
	}

	// Are we the selected guy?
	if (GetSelectedMan() == pSoldier)
	{
		SOLDIERTYPE* const next = FindNextActiveAndAliveMerc(pSoldier, FALSE, FALSE);
		if (next != pSoldier)
		{
			SelectSoldier(next, SELSOLDIER_NONE);
		}
		else
		{
			SetSelectedMan(NULL);

			// ATE: Make sure we are in TEAM panel at this point!
			SetCurrentInterfacePanel( TEAM_PANEL );
		}
	}
}


static BOOLEAN IsDeadGuyOnSquad(const ProfileID pid, const INT8 squad)
{
	for (INT32 i = 0; i < NUMBER_OF_SOLDIERS_PER_SQUAD; ++i)
	{
		if (sDeadMercs[squad][i] == pid) return TRUE;
	}
	return FALSE;
}


static BOOLEAN IsDeadGuyOnAnySquad(SOLDIERTYPE* pSoldier);


static BOOLEAN AddDeadCharacterToSquadDeadGuys(SOLDIERTYPE* pSoldier, INT32 iSquadValue)
{
	INT32 iCounter = 0;

	// is dead guy in any squad
	if (IsDeadGuyOnAnySquad(pSoldier)) return TRUE;

	if (IsDeadGuyOnSquad(pSoldier->ubProfile, iSquadValue)) return TRUE;

	// now insert the guy
	for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
	{
		const INT16 dead_id = sDeadMercs[iSquadValue][iCounter];
		if (dead_id == -1 || FindSoldierByProfileIDOnPlayerTeam(dead_id) == NULL)
		{
			sDeadMercs[iSquadValue][iCounter] = pSoldier->ubProfile;
		}
	}

	// no go
	return( FALSE );
}


static BOOLEAN IsDeadGuyOnAnySquad(SOLDIERTYPE* pSoldier)
{
	// squad?
	for (INT32 iCounterA = 0; iCounterA < NUMBER_OF_SQUADS; ++iCounterA)
	{
		if (IsDeadGuyOnSquad(pSoldier->ubProfile, iCounterA)) return TRUE;
	}

	return ( FALSE );
}


BOOLEAN SoldierIsDeadAndWasOnSquad( SOLDIERTYPE *pSoldier, INT8 bSquadValue )
{
	return
		bSquadValue != NO_CURRENT_SQUAD &&
		IsDeadGuyOnSquad(pSoldier->ubProfile, bSquadValue);
}


void ResetDeadSquadMemberList(INT32 const iSquadValue)
{
	memset( sDeadMercs[ iSquadValue ], -1, sizeof( INT16 ) * NUMBER_OF_SOLDIERS_PER_SQUAD );
}


// this passed  soldier on the current squad int he tactical map
BOOLEAN IsMercOnCurrentSquad(const SOLDIERTYPE* pSoldier)
{
	// current squad valid?
	if( iCurrentTacticalSquad >= NUMBER_OF_SQUADS )
	{
		// no
		return( FALSE );
	}

	FOR_EACH_SLOT_IN_SQUAD(i, iCurrentTacticalSquad)
	{
		if (*i == pSoldier) return TRUE;
	}

	return( FALSE );
}

INT8 NumberOfPlayerControllableMercsInSquad( INT8 bSquadValue )
{
	INT8 bSquadCount = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		SOLDIERTYPE const* const pSoldier = *i;
		//Kris:  This breaks the CLIENT of this function, tactical traversal.
		//       Do NOT check for EPCS or ROBOT here.
		//if ( !AM_AN_EPC( pSoldier ) && !AM_A_ROBOT( pSoldier ) &&
		if( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			bSquadCount++;
		}
	}

	// return number found
	return( bSquadCount );
}


BOOLEAN DoesVehicleExistInSquad( INT8 bSquadValue )
{
	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( FALSE );
	}

	// find number of characters in particular squad.
	FOR_EACH_IN_SQUAD(i, bSquadValue)
	{
		if ((*i)->uiStatusFlags & SOLDIER_VEHICLE) return TRUE;
	}

	return(FALSE );
}


void CheckSquadMovementGroups()
{
	FOR_EACH(INT8, i, SquadMovementGroups)
	{
		if (GetGroup(*i)) continue;

		// recreate group
		GROUP* const g = CreateNewPlayerGroupDepartingFromSector(1, 1);
		g->fPersistant = TRUE;
		*i = g->ubGroupID;
	}
}
