#include "Interface_Panels.h"
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


typedef struct
{
	INT16	uiID;						// The soldiers ID
	INT16 sPadding[ 5 ];
//	INT8	bSquadValue;		// The squad id

} SAVE_SQUAD_INFO_STRUCT;

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
	INT32 iCounterB = 0;
	INT32 iCounter =0;
	GROUP		*pGroup	=	NULL;

	// null each list of ptrs.
	for( iCounter = 0; iCounter <  NUMBER_OF_SQUADS; iCounter++ )
	{
		for( iCounterB =0; iCounterB < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounterB++ )
		{

		  // squad, soldier
		  Squad[ iCounter ][ iCounterB ]= NULL;

		}

		// create mvt groups
		SquadMovementGroups[ iCounter ] = CreateNewPlayerGroupDepartingFromSector( 1, 1 );

		// Set persistent....
		pGroup = GetGroup( SquadMovementGroups[ iCounter ] );
		pGroup->fPersistant = TRUE;

	}

	memset( sDeadMercs, -1, sizeof( INT16 ) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD );
}

BOOLEAN IsThisSquadFull( INT8 bSquadValue )
{
	INT32 iCounter = 0;

	// run through entries in the squad list, make sure there is a free entry
	for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
	{
		// check this slot
		if( Squad[ bSquadValue ][ iCounter ] == NULL )
		{
			// a free slot found - not full
			return( FALSE );
		}
	}

	// no free slots - it's full
	return( TRUE );
}

INT8 GetFirstEmptySquad( void )
{
	UINT8 ubCounter = 0;

	for( ubCounter = 0; ubCounter < NUMBER_OF_SQUADS; ubCounter++ )
	{
		if (SquadIsEmpty(ubCounter)) return ubCounter;
	}

	// not found - none are completely empty (shouldn't ever happen!)
	Assert( FALSE );
	return( -1 );
}


static BOOLEAN CopyPathOfSquadToCharacter(SOLDIERTYPE* pCharacter, INT8 bSquadValue);


BOOLEAN AddCharacterToSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue )
{
	INT8 bCounter =0;
//	BOOLEAN fBetweenSectors = FALSE;
	GROUP	*pGroup;
	BOOLEAN fNewSquad;


	// add character to squad...return success or failure
	// run through list of people in squad, find first free slo

	if( fExitingVehicleToSquad )
	{
		return( FALSE );
	}


	// ATE: If any vehicle exists in this squad AND we're not set to
	// a driver or or passenger, when return false
	if ( DoesVehicleExistInSquad( bSquadValue ) )
	{
		// We're not allowing anybody to go on a vehicle if they are not passengers!
		// NB: We obviously need to make sure that REAL passengers have their
		// flags set before adding them to a squad!
		if ( !( pCharacter->uiStatusFlags & ( SOLDIER_PASSENGER | SOLDIER_DRIVER | SOLDIER_VEHICLE ) ) )
		{
			return( FALSE );
		}
	}



	// if squad is on the move, can't add someone
	if( IsThisSquadOnTheMove( bSquadValue ) == TRUE )
	{
		// nope, go away now
		return( FALSE );
	}



	for( bCounter =0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; bCounter++ )
	{
		const SOLDIERTYPE* const t = Squad[bSquadValue][bCounter];
		// check if on current squad and current slot?
		// 'successful of sorts, if there, then he's 'added'
		if (t == pCharacter) return TRUE;

		// free slot, add here
		if (t == NULL)
		{
			// check if squad empty, if not check sector x,y,z are the same as this guys
			INT16 sX;
			INT16 sY;
			INT8  bZ;
			if (SectorSquadIsIn(bSquadValue, &sX, &sY, &bZ) &&
					(pCharacter->sSectorX != sX || pCharacter->sSectorY != sY || pCharacter->bSectorZ != bZ))
			{
				return FALSE;
			}

			RemoveCharacterFromSquads(pCharacter);

			// copy path of squad to this char
			CopyPathOfSquadToCharacter( pCharacter, bSquadValue );

			// check if old mvt group
			if( pCharacter -> ubGroupID != 0 )
			{
				// in valid group, remove from that group
				RemovePlayerFromGroup(  pCharacter -> ubGroupID , pCharacter );

				// character not on a reserved group
				if( ( pCharacter->bAssignment >= ON_DUTY ) && ( pCharacter->bAssignment != VEHICLE ) )
				{
					// get the group from the character
					pGroup = GetGroup( pCharacter -> ubGroupID );

					// if valid group, delete it
					if( pGroup )
					{
						RemoveGroupFromList( pGroup );
					}
				}

			}




			if( ( pCharacter->bAssignment == VEHICLE ) && ( pCharacter->iVehicleId == iHelicopterVehicleId ) && ( pCharacter-> iVehicleId != -1 ) )
			{
				// if creating a new squad from guys exiting the chopper
				fNewSquad = SquadIsEmpty( bSquadValue );

				RemoveSoldierFromHelicopter( pCharacter );

				AddPlayerToGroup( SquadMovementGroups[ bSquadValue ], pCharacter  );
				SetGroupSectorValue( pCharacter->sSectorX, pCharacter->sSectorY, pCharacter->bSectorZ, SquadMovementGroups[ bSquadValue ] );
				pCharacter -> ubGroupID = SquadMovementGroups[ bSquadValue ];

				// if we've just started a new squad
				if ( fNewSquad )
				{
					// set mvt group for
					// grab group
					const GROUP* const pGroup = GetGroup(GetHelicopter()->ubMovementGroup);
					Assert( pGroup );

					if( pGroup )
					{
						// set where it is and where it's going, then make it arrive there.  Don't check for battle
						PlaceGroupInSector( SquadMovementGroups[ bSquadValue ], pGroup->ubPrevX, pGroup->ubPrevY, pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, FALSE );
					}
				}
			}
			else if( ( pCharacter->bAssignment == VEHICLE ) && ( pCharacter-> iVehicleId != -1 ) )
			{
				fExitingVehicleToSquad = TRUE;
				// remove from vehicle
				TakeSoldierOutOfVehicle( pCharacter );
				fExitingVehicleToSquad = FALSE;


				AddPlayerToGroup( SquadMovementGroups[ bSquadValue ], pCharacter  );
				SetGroupSectorValue( pCharacter->sSectorX, pCharacter->sSectorY, pCharacter->bSectorZ, SquadMovementGroups[ bSquadValue ] );
				pCharacter -> ubGroupID = SquadMovementGroups[ bSquadValue ];
			}
			else
			{
				AddPlayerToGroup( SquadMovementGroups[ bSquadValue ], pCharacter  );
				SetGroupSectorValue( pCharacter->sSectorX, pCharacter->sSectorY, pCharacter->bSectorZ, SquadMovementGroups[ bSquadValue ] );
				pCharacter -> ubGroupID = SquadMovementGroups[ bSquadValue ];
			}


			// assign here
			Squad[ bSquadValue ][ bCounter ] = pCharacter;

			if( ( pCharacter->bAssignment != bSquadValue ) )
			{
				// check to see if we should wake them up
				if ( pCharacter->fMercAsleep )
				{
					// try to wake him up
					SetMercAwake( pCharacter, FALSE, FALSE );
				}
				SetTimeOfAssignmentChangeForMerc( pCharacter );
			}

			// set squad value
			ChangeSoldiersAssignment( pCharacter, bSquadValue );
			if ( pCharacter->bOldAssignment < ON_DUTY )
			{
				pCharacter->bOldAssignment = bSquadValue;
			}

			// if current tactical sqaud...upadte panel
			if( NumberOfPeopleInSquad( ( INT8 )iCurrentTacticalSquad ) == 0 )
			{
				SetCurrentSquad( bSquadValue, TRUE );
			}

			if( bSquadValue == ( INT8 ) iCurrentTacticalSquad )
			{
				CheckForAndAddMercToTeamPanel( Squad[ iCurrentTacticalSquad ][ bCounter ] );
			}

			if (pCharacter == GetSelectedMan())
			{
				SetCurrentSquad( bSquadValue, TRUE );
			}


			return ( TRUE );
		}
	}

	return ( FALSE );
}


// find the first slot we can fit the guy in
BOOLEAN AddCharacterToAnySquad( SOLDIERTYPE *pCharacter )
{
	// add character to any squad, if character is assigned to a squad, returns TRUE
	INT8 bCounter = 0;
	INT8 bFirstEmptySquad = -1;


	// remove them from current squad
	RemoveCharacterFromSquads( pCharacter );

	// first look for a compatible NON-EMPTY squad (don't start new squad if we don't have to)
	for( bCounter = 0; bCounter < NUMBER_OF_SQUADS; bCounter++ )
	{
		if (!SquadIsEmpty(bCounter))
		{
			if( AddCharacterToSquad( pCharacter, bCounter ) == TRUE )
			{
				return ( TRUE );
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
		if( AddCharacterToSquad( pCharacter, bFirstEmptySquad ) == TRUE )
		{
			return ( TRUE );
		}
	}

	// should never happen!
	Assert( FALSE );
	return( FALSE );
}

// find the first slot we can fit the guy in
INT8 AddCharacterToUniqueSquad( SOLDIERTYPE *pCharacter )
{
	// add character to any squad, if character is assigned to a squad, returns TRUE
	INT8 bCounter = 0;

	// check if character on a squad

		// remove them
	RemoveCharacterFromSquads( pCharacter );

	for( bCounter = 0; bCounter < NUMBER_OF_SQUADS; bCounter++ )
	{
		if (SquadIsEmpty(bCounter))
		{
			if( AddCharacterToSquad( pCharacter, bCounter ) == TRUE )
			{
				return ( bCounter );
			}
		}
	}

	return( -1 );
}


BOOLEAN SquadIsEmpty( INT8 bSquadValue )
{
	// run through this squad's slots and find if they ALL are empty
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; iCounter++ )
	{
		if( Squad[ bSquadValue ][ iCounter ] != NULL )
		{
			return ( FALSE );
		}
	}

	return( TRUE );
}


static BOOLEAN AddDeadCharacterToSquadDeadGuys(SOLDIERTYPE* pSoldier, INT32 iSquadValue);
static void RebuildSquad(INT8 bSquadValue);
static void UpdateCurrentlySelectedMerc(SOLDIERTYPE* pSoldier, INT8 bSquadValue);


// find and remove characters from any squad
BOOLEAN RemoveCharacterFromSquads( SOLDIERTYPE *pCharacter )
{
	INT32 iCounterA = 0;
	INT32 iCounter = 0;
	UINT8 ubGroupId = 0;
	// find character and remove.. check characters in all squads


	// squad?
	for( iCounterA = 0; iCounterA < NUMBER_OF_SQUADS ; iCounterA++ )
	{
		// slot?
		for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; iCounter++ )
		{

			// check if on current squad and current slot?
			if( Squad[ iCounterA ][ iCounter ] == pCharacter )
			{


				// found and nulled
				Squad[ iCounterA ][ iCounter ] = NULL;

				// Release memory for his personal path, BUT DON'T CLEAR HIS GROUP'S PATH/WAYPOINTS (pass in groupID -1).
				// Just because one guy leaves a group is no reason to cancel movement for the rest of the group.
				pCharacter -> pMercPath = ClearStrategicPathList( pCharacter -> pMercPath, -1 );

				// remove character from mvt group
				RemovePlayerFromGroup( SquadMovementGroups[ iCounterA ], pCharacter  );

				// reset player mvt group id value
				pCharacter -> ubGroupID = 0;

				if( ( pCharacter->fBetweenSectors )&&( pCharacter-> uiStatusFlags & SOLDIER_VEHICLE ) )
				{
					ubGroupId = CreateNewPlayerGroupDepartingFromSector( ( INT8 ) ( pCharacter -> sSectorX ) , ( INT8 ) ( pCharacter -> sSectorY ) );

					// assign to a group
					AddPlayerToGroup( ubGroupId, pCharacter );
				}

				RebuildSquad( ( INT8 )iCounterA );

				if( pCharacter->bLife == 0 )
				{
					AddDeadCharacterToSquadDeadGuys( pCharacter, iCounterA );
				}

					//if we are not loading a saved game
				if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) && guiCurrentScreen == GAME_SCREEN )
        {
					UpdateCurrentlySelectedMerc( pCharacter, ( INT8 )iCounterA );
        }

				return ( TRUE );
			}
		}
	}

	// not found
	return ( FALSE );
}


INT8 SquadCharacterIsIn( SOLDIERTYPE *pCharacter )
{
	// returns which squad character is in, -1 if none found
	INT8 iCounterA =0, iCounter = 0;

	// squad?
	for( iCounterA = 0; iCounterA < NUMBER_OF_SQUADS ; iCounterA++ )
	{
		// slot?
		for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; iCounter++ )
		{

			// check if on current squad and current slot?
			if( Squad[ iCounterA ][ iCounter ] == pCharacter )
			{
				// return value
				return ( iCounterA );
			}
		}
	}

	// return failure
	return ( -1 );
}

INT8 NumberOfPeopleInSquad( INT8 bSquadValue )
{
	INT8 bCounter = 0;
	INT8 bSquadCount = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{

		// valid slot?
		if( Squad[ bSquadValue ][ bCounter ] != NULL )
		{
			// yep
			bSquadCount++;
		}
	}

	// return number found
	return( bSquadCount );
}

INT8 NumberOfNonEPCsInSquad( INT8 bSquadValue )
{
	INT8 bCounter = 0;
	INT8 bSquadCount = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		const SOLDIERTYPE* const s = Squad[bSquadValue][bCounter];
		// valid slot?
		if (s != NULL && !AM_AN_EPC(s)) ++bSquadCount;
	}

	// return number found
	return( bSquadCount );
}

BOOLEAN IsRobotControllerInSquad( INT8 bSquadValue )
{
	INT8 bCounter = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		const SOLDIERTYPE* const s = Squad[bSquadValue][bCounter];
		// valid slot?
		if (s != NULL && ControllingRobot(s)) return TRUE;
	}

	// return number found
	return( FALSE );
}


BOOLEAN SectorSquadIsIn(const INT8 bSquadValue, INT16* const sMapX, INT16* const sMapY, INT8* const sMapZ)
{
	// returns if there is anyone on the squad and what sector ( strategic ) they are in
  INT8 bCounter =0;

	Assert( bSquadValue < ON_DUTY );

	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		const SOLDIERTYPE* const s = Squad[bSquadValue][bCounter];
		// if valid soldier, get current sector and return
		if (s != NULL)
		{
			*sMapX = s->sSectorX;
			*sMapY = s->sSectorY;
			*sMapZ = s->bSectorZ;
			return TRUE;
		}

	}

	// return there is no squad
	return ( FALSE );
}


static BOOLEAN CopyPathOfSquadToCharacter(SOLDIERTYPE* pCharacter, INT8 bSquadValue)
{
	// copy path from squad to character
	INT8 bCounter = 0;



	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		const SOLDIERTYPE* const t = Squad[bSquadValue][bCounter];
		if (t != NULL && t != pCharacter)
		{
			// valid character, copy paths
			ClearStrategicPathList(pCharacter->pMercPath, 0);
			pCharacter->pMercPath = CopyPaths(t->pMercPath);

			 // return success
			 return ( TRUE );
		}
	}

	// return failure
	return ( FALSE );
}


void CopyPathOfCharacterToSquad(SOLDIERTYPE* const pCharacter, const INT8 bSquadValue)
{
	// copy path of this character to members of squad
	INT8 bCounter =0;

	// copy each person on squad, skip this character
  for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		SOLDIERTYPE* const t = Squad[bSquadValue][bCounter];
		if (t != NULL && t != pCharacter)
		{
			// valid character, copy paths
			ClearStrategicPathList(t->pMercPath, -1);
			t->pMercPath = CopyPaths(pCharacter->pMercPath);
		}
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
	INT32 iCounter = 0;


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

	if( iCurrentTacticalSquad != NO_CURRENT_SQUAD )
	{
		for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
		{
			SOLDIERTYPE* const s = Squad[iCurrentTacticalSquad][iCounter];
			// squad set, now add soldiers in
			if (s != NULL) CheckForAndAddMercToTeamPanel(s);
		}
	}

	// check if the currently selected guy is on this squad, if not, get the first one on the new squad
	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL || sel->bAssignment != iCurrentTacticalSquad)
	{
		// ATE: Changed this to FALSE for acknowledgement sounds.. sounds bad if just starting/entering sector..
		SelectSoldier(Squad[iCurrentTacticalSquad][0], SELSOLDIER_FORCE_RESELECT);
	}

	return ( TRUE );
}

void RebuildCurrentSquad( void )
{
	// rebuilds current squad to reset faces in tactical
	INT32 iCounter = 0;

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
		for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
		{
			SOLDIERTYPE* const s = Squad[iCurrentTacticalSquad][iCounter];
			// squad set, now add soldiers in
			if (s != NULL) CheckForAndAddMercToTeamPanel(s);
		}

		for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
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
	// rebuilds current squad to reset faces in tactical
	INT32 iCounter = 0;

	// OK, we should add lights for any guy currently bInSector who is not bad OKLIFE...
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		if (s->bInSector && s->bLife >= OKLIFE) PositionSoldierLight(s);
	}
}


BOOLEAN IsSquadOnCurrentTacticalMap( INT32 iCurrentSquad )
{
	INT32 iCounter = 0;
	// check to see if this squad is on the current map

	// check if valid value passed
	if( ( iCurrentSquad >= NUMBER_OF_SQUADS ) || ( iCurrentSquad < 0 ) )
	{
		// no
		return ( FALSE );
	}

	// go through memebrs of squad...if anyone on this map, return true
	for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
	{
		const SOLDIERTYPE* const s = Squad[iCurrentSquad][iCounter];
		if (s != NULL)
		{
			// ATE; Added more checks here for being in sector ( fBetweenSectors and SectorZ )
			if (s->sSectorX == gWorldSectorX  &&
					s->sSectorY == gWorldSectorY  &&
					s->bSectorZ == gbWorldSectorZ &&
					s->fBetweenSectors != TRUE)
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


void SetDefaultSquadOnSectorEntry( BOOLEAN fForce )
{
	INT32 iCounter = 0;
	// check if selected squad is in current sector, if so, do nothing, if not...first first case that they are

	if( IsSquadOnCurrentTacticalMap( iCurrentTacticalSquad ) == TRUE )
	{
		// is in sector, leave
		return;
	}

	//otherwise...

	// find first squad availiable
	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		if( IsSquadOnCurrentTacticalMap( iCounter ) == TRUE )
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
	INT32 iCounter =0, iCounterB = 0;
	INT32 iLastSquad = 0;

	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		for( iCounterB = 0; iCounterB < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounterB++ )
		{
			if( Squad[ iCounter ][ iCounterB ] != NULL )
			{
				iLastSquad = iCounter;
			}
		}
	}

	return ( iLastSquad );
}


BOOLEAN SaveSquadInfoToSavedGameFile( HWFILE hFile )
{
	SAVE_SQUAD_INFO_STRUCT sSquadSaveStruct[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];
	UINT32	uiSaveSize=0;
	//Reset the current squad info
	INT32 iCounterB = 0;
	INT32 iCounter =0;


	for( iCounter = 0; iCounter <  NUMBER_OF_SQUADS; iCounter++ )
	{
		for( iCounterB =0; iCounterB < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounterB++ )
		{
			const SOLDIERTYPE* const s = Squad[iCounter][iCounterB];
			sSquadSaveStruct[iCounter][iCounterB].uiID = (s != NULL ? s->ubID : -1);
		}
	}

	//Save the squad info to the Saved Game File
	uiSaveSize = sizeof( SAVE_SQUAD_INFO_STRUCT ) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD;

	if (!FileWrite(hFile, sSquadSaveStruct, uiSaveSize)) return FALSE;

	//Save all the squad movement id's
	if (!FileWrite(hFile, SquadMovementGroups, sizeof(INT8) * NUMBER_OF_SQUADS)) return FALSE;

	return( TRUE );
}



BOOLEAN LoadSquadInfoFromSavedGameFile( HWFILE hFile )
{
	SAVE_SQUAD_INFO_STRUCT sSquadSaveStruct[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];
	UINT32	uiSaveSize=0;

	//Reset the current squad info
	INT32 iCounterB = 0;
	INT32 iCounter =0;

	// null each list of ptrs.
	for( iCounter = 0; iCounter <  NUMBER_OF_SQUADS; iCounter++ )
	{
		for( iCounterB =0; iCounterB < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounterB++ )
		{
		  // squad, soldier
		  Squad[ iCounter ][ iCounterB ]= NULL;
		}
	}


	// Load in the squad info
	uiSaveSize = sizeof( SAVE_SQUAD_INFO_STRUCT ) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD;

	if (!FileRead(hFile, sSquadSaveStruct, uiSaveSize)) return FALSE;

	// Loop through the array loaded in
	for( iCounter = 0; iCounter <  NUMBER_OF_SQUADS; iCounter++ )
	{
		for( iCounterB =0; iCounterB < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounterB++ )
		{
			const INT16 id = sSquadSaveStruct[iCounter][iCounterB].uiID;
			Squad[iCounter][iCounterB] = (id != -1 ? GetMan(id) : NULL);
		}
	}


	//Load in the Squad movement id's
	if (!FileRead(hFile, SquadMovementGroups, sizeof(INT8) * NUMBER_OF_SQUADS)) return FALSE;

	return( TRUE );
}


BOOLEAN IsThisSquadOnTheMove( INT8 bSquadValue )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
	{
		const SOLDIERTYPE* const s = Squad[bSquadValue][iCounter];
		if (s) return s->fBetweenSectors;
	}

	return( FALSE );
}


// rebuild this squad after someone has been removed, to 'squeeze' together any empty spots
static void RebuildSquad(INT8 bSquadValue)
{
	INT32 iCounter = 0, iCounterB = 0;

	for( iCounterB = 0; iCounterB <NUMBER_OF_SOLDIERS_PER_SQUAD - 1; iCounterB++ )
	{
		for( iCounter = 0 ;iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD - 1; iCounter++ )
		{
			SOLDIERTYPE** const cur  = &Squad[bSquadValue][iCounter];
			SOLDIERTYPE** const next = &Squad[bSquadValue][iCounter + 1];
			if (*cur == NULL && *next != NULL)
			{
				*cur  = *next;
				*next = NULL;
			}
		}
	}
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
	if( IsDeadGuyOnAnySquad( pSoldier ) == TRUE )
	{
		return( TRUE );
	}

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


BOOLEAN ResetDeadSquadMemberList( INT32 iSquadValue )
{
	memset( sDeadMercs[ iSquadValue ], -1, sizeof( INT16 ) * NUMBER_OF_SOLDIERS_PER_SQUAD );

	return( TRUE );
}


// this passed  soldier on the current squad int he tactical map
BOOLEAN IsMercOnCurrentSquad(const SOLDIERTYPE* pSoldier)
{
	INT32 iCounter = 0;

	// valid soldier?
	if( pSoldier == NULL )
	{
		// no
		return( FALSE );
	}

	// active grunt?
	if( pSoldier->bActive == FALSE )
	{
		// no
		return( FALSE );
	}

	// current squad valid?
  if( iCurrentTacticalSquad >= NUMBER_OF_SQUADS )
	{
		// no
		return( FALSE );
	}


	for( iCounter = 0; iCounter < NUMBER_OF_SOLDIERS_PER_SQUAD; iCounter++ )
	{
		if( Squad[ iCurrentTacticalSquad ][ iCounter ] == pSoldier )
		{
			// found him
			return( TRUE );
		}
	}

	return( FALSE );
}

INT8 NumberOfPlayerControllableMercsInSquad( INT8 bSquadValue )
{
	INT8 bCounter = 0;
	INT8 bSquadCount = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( 0 );
	}

	// find number of characters in particular squad.
	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		const SOLDIERTYPE* const pSoldier = Squad[bSquadValue][bCounter];
		if (pSoldier != NULL)
		{
			//Kris:  This breaks the CLIENT of this function, tactical traversal.  Do NOT check for EPCS or ROBOT here.
			//if ( !AM_AN_EPC( pSoldier ) && !AM_A_ROBOT( pSoldier ) &&
			if( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
			{
				bSquadCount++;
			}
		}
	}

	// return number found
	return( bSquadCount );
}


BOOLEAN DoesVehicleExistInSquad( INT8 bSquadValue )
{
	INT8 bCounter = 0;

	if( bSquadValue == NO_CURRENT_SQUAD )
	{
		return( FALSE );
	}

	// find number of characters in particular squad.
	for( bCounter = 0; bCounter < NUMBER_OF_SOLDIERS_PER_SQUAD ; bCounter++ )
	{
		const SOLDIERTYPE* const s = Squad[bSquadValue][bCounter];
		if (s != NULL && s->uiStatusFlags & SOLDIER_VEHICLE) return TRUE;
	}

	return(FALSE );
}

void CheckSquadMovementGroups( void )
{
	INT32			iSquad;
	GROUP *		pGroup;

	for( iSquad = 0; iSquad < NUMBER_OF_SQUADS; iSquad++ )
	{
		pGroup = GetGroup( SquadMovementGroups[ iSquad ] );
		if ( pGroup == NULL )
		{
			// recreate group
			SquadMovementGroups[ iSquad ] = CreateNewPlayerGroupDepartingFromSector( 1, 1 );

			// Set persistent....
			pGroup = GetGroup( SquadMovementGroups[ iSquad ] );
			Assert( pGroup );
			pGroup->fPersistant = TRUE;
		}
	}
}
