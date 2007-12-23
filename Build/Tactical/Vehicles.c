#include "LoadSaveVehicleType.h"
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
#include "Interface_Panels.h"
#include "Strategic.h"
#include "WorldDef.h"
#include "Tile_Animation.h"
#include "Isometric_Utils.h"
#include "MapScreen.h"
#include "Interface.h"
#include "Random.h"
#include "Text.h"
#include "Explosion_Control.h"
#include "Soldier_Create.h"
#include "Animation_Control.h"
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

// the sqaud mvt groups
extern INT8 SquadMovementGroups[ ];


//ATE: These arrays below should all be in a large LUT which contains
// static info for each vehicle....


typedef struct VehicleTypeInfo
{
	INT32     enter_sound;
	INT32     move_sound;
	ProfileID profile;
	UINT8     movement_type;
	UINT16    armour_type;
	UINT8     seats;
} VehicleTypeInfo;

static const VehicleTypeInfo g_vehicle_type_info[] =
{
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_ELDERODO,   CAR, KEVLAR_VEST,  6 }, // Eldorado
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_HUMMER,     CAR, SPECTRA_VEST, 6 }, // Hummer
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_ICECREAM,   CAR, KEVLAR_VEST,  6 }, // Ice cream truck
	{ S_VECH1_INTO, S_VECH1_MOVE, NPC164,          CAR, KEVLAR_VEST,  6 }, // Jeep
	{ S_VECH1_INTO, S_VECH1_MOVE, NPC164,          CAR, SPECTRA_VEST, 6 }, // Tank
	{ S_VECH1_INTO, S_VECH1_MOVE, PROF_HELICOPTER, AIR, KEVLAR_VEST,  6 }  // Helicopter
};


/*
// location of crits based on facing
INT8 bInternalCritHitsByLocation[ NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE ][ NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ]={
	{ ENGINE_HIT_LOCATION, ENGINE_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION,CREW_COMPARTMENT_HIT_LOCATION, RF_TIRE_HIT_LOCATION, LF_TIRE_HIT_LOCATION }, // front
	{ ENGINE_HIT_LOCATION, LF_TIRE_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, LR_TIRE_HIT_LOCATION, GAS_TANK_HIT_LOCATION}, // left side
	{ ENGINE_HIT_LOCATION, RF_TIRE_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, RR_TIRE_HIT_LOCATION, GAS_TANK_HIT_LOCATION}, // right side
	{ CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, RR_TIRE_HIT_LOCATION, LR_TIRE_HIT_LOCATION, GAS_TANK_HIT_LOCATION }, // rear
	{ ENGINE_HIT_LOCATION, RF_TIRE_HIT_LOCATION, LF_TIRE_HIT_LOCATION, RR_TIRE_HIT_LOCATION,LR_TIRE_HIT_LOCATION, GAS_TANK_HIT_LOCATION,}, // bottom side
	{ ENGINE_HIT_LOCATION, ENGINE_HIT_LOCATION, ENGINE_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, GAS_TANK_HIT_LOCATION }, // top
};
*/

// original armor values for vehicles
/*
	ELDORADO_CAR = 0,
	HUMMER,
	ICE_CREAM_TRUCK,
	JEEP_CAR,
	TANK_CAR,
	HELICOPTER,
*/


/*
INT16 sVehicleExternalOrigArmorValues[ NUMBER_OF_TYPES_OF_VEHICLES ][ NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ]={
	{ 100,100,100,100,100,100 }, // helicopter
	{ 500,500,500,500,500,500 }, // hummer
};
*/

/*
// external armor values
INT16 sVehicleInternalOrigArmorValues[ NUMBER_OF_TYPES_OF_VEHICLES ][ NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ]={
	{ 250,250,250,250,250,250 }, // eldorado
	{ 250,250,250,250,250,250 }, // hummer
	{ 250,250,250,250,250,250 }, // ice cream
	{ 250,250,250,250,250,250 }, // feep
	{ 850,850,850,850,850,850 }, // tank
	{ 50,50,50,50,50,50 }, // helicopter
};
*/

// ap cost per crit
#define COST_PER_ENGINE_CRIT 15
#define COST_PER_TIRE_HIT 5
//#define VEHICLE_MAX_INTERNAL 250


// Loop through and create a few soldier squad ID's for vehicles ( max # 3 )
void InitVehicles( )
{
	INT32		cnt;
	GROUP		*pGroup	=	NULL;

	for( cnt = 0; cnt <  MAX_VEHICLES; cnt++ )
	{
		// create mvt groups
		gubVehicleMovementGroups[ cnt ] = CreateNewVehicleGroupDepartingFromSector( 1, 1, cnt );

		// Set persistent....
		pGroup = GetGroup( gubVehicleMovementGroups[ cnt ] );
		pGroup->fPersistant = TRUE;
	}
}


void SetVehicleValuesIntoSoldierType( SOLDIERTYPE *pVehicle )
{
	const VEHICLETYPE* const v = &pVehicleList[pVehicle->bVehicleID];
	wcscpy(pVehicle->name, zVehicleName[v->ubVehicleType]);

	pVehicle->ubProfile = g_vehicle_type_info[pVehicleList[pVehicle->bVehicleID].ubVehicleType].profile;

  // Init fuel!
  pVehicle->sBreathRed = 10000;
  pVehicle->bBreath    = 100;

  pVehicle->ubWhatKindOfMercAmI = MERC_TYPE__VEHICLE;
}


static void SetUpArmorForVehicle(UINT8 ubID);


INT32 AddVehicleToList( INT16 sMapX, INT16 sMapY, INT16 sGridNo, UINT8 ubType )
{
	// insert this vehicle into the list
	// how many vehicles are there?
	INT32 iVehicleIdValue = -1;
	INT32 iCounter = 0, iCount = 0;
	VEHICLETYPE *pTempList = NULL;
	BOOLEAN fFoundEmpty = FALSE;
	GROUP *pGroup;


	if( pVehicleList != NULL )
	{
		// not the first, add to list
		for( iCounter = 0; iCounter < ubNumberOfVehicles ; iCounter++ )
		{
			// might have an empty slot
			if( pVehicleList[ iCounter ].fValid == FALSE )
			{
				iCount = iCounter;
				iCounter = ubNumberOfVehicles;
				fFoundEmpty = TRUE;
			  iVehicleIdValue = iCount;
			}
		}
	}

	if( fFoundEmpty == FALSE )
	{
		iCount = ubNumberOfVehicles;
	}

	if( iCount == 0 )
	{
		pVehicleList = MemAlloc( sizeof( VEHICLETYPE ) );

		// Set!
		memset( pVehicleList, 0, sizeof( VEHICLETYPE ) );

		ubNumberOfVehicles = 1;
		iVehicleIdValue		 = 0;
	}


	if( ( iVehicleIdValue == -1 ) && ( iCount != 0 ) && ( fFoundEmpty == FALSE ) )
	{

		// no empty slot found, need to realloc
		pTempList = MemAlloc( sizeof( VEHICLETYPE ) * ubNumberOfVehicles );

		// copy to temp
		memcpy( pTempList, pVehicleList, sizeof( VEHICLETYPE ) * ubNumberOfVehicles );

		// now realloc
		pVehicleList = MemRealloc( pVehicleList, ( sizeof( VEHICLETYPE ) * ( ubNumberOfVehicles + 1 ) ) );

		// memset the stuff
		memset( pVehicleList, 0, ( sizeof( VEHICLETYPE ) * ( ubNumberOfVehicles + 1 ) ) );

		// now copy the stuff back
		memcpy( pVehicleList, pTempList, sizeof( VEHICLETYPE ) * ( ubNumberOfVehicles ) );

		// now get rid of crap
		MemFree( pTempList );

		// now get the index value
		iVehicleIdValue = ubNumberOfVehicles;

		ubNumberOfVehicles++;


	}

	// found a slot
	pVehicleList[ iCount ].ubMovementGroup = 0;
	pVehicleList[ iCount ].sSectorX = sMapX;
	pVehicleList[ iCount ].sSectorY = sMapY;
	pVehicleList[ iCount ].sSectorZ = 0;
	pVehicleList[ iCount ].sGridNo = sGridNo;
	memset( pVehicleList[ iCount ].pPassengers, 0, 10 * sizeof( SOLDIERTYPE * ) );
	pVehicleList[ iCount ].fValid = TRUE;
	pVehicleList[ iCount ].ubVehicleType = ubType;
	pVehicleList[ iCount ].pMercPath = NULL;
	pVehicleList[ iCount ].fDestroyed = FALSE;
	pVehicleList[ iCount ].ubMovementGroup  = gubVehicleMovementGroups[ iCount ];

	// ATE: Add movement mask to group...
	pGroup = GetGroup( pVehicleList[ iCount ].ubMovementGroup );

	if( !pGroup )
	{
		if( gfEditMode )
		{
			//This is okay, no groups exist, so simply return.
			return iVehicleIdValue;
		}
		Assert( 0 );
	}

	pGroup->ubTransportationMask = g_vehicle_type_info[ubType].movement_type;

	// ARM: setup group movement defaults
	pGroup->ubSectorX = ( UINT8 ) sMapX;
	pGroup->ubNextX   = ( UINT8 ) sMapX;
	pGroup->ubSectorY = ( UINT8 ) sMapY;
	pGroup->ubNextY   = ( UINT8 ) sMapY;
	pGroup->uiTraverseTime = 0;
	pGroup->uiArrivalTime  = 0;

	SetUpArmorForVehicle( ( UINT8 )iCount );

	return( iVehicleIdValue );
}


BOOLEAN RemoveVehicleFromList( INT32 iId )
{
	// remove this vehicle from the list
	VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return FALSE;

	// clear remaining path nodes
	if (v->pMercPath != NULL )
	{
		v->pMercPath = ClearStrategicPathList(v->pMercPath, 0);
	}

	// zero out mem
	memset(v, 0, sizeof(*v));

	return( TRUE );
}

void ClearOutVehicleList( void )
{
	// empty out the vehicle list
	if( pVehicleList )
	{
		FOR_ALL_VEHICLES(v)
		{
			//if the vehicle has a valid path
			if (v->pMercPath)
			{
				//toast the vehicle path
				v->pMercPath = ClearStrategicPathList(v->pMercPath, 0);
			}
		}

		MemFree( pVehicleList );
		pVehicleList = NULL;
		ubNumberOfVehicles = 0;
	}

/*
	// empty out the vehicle list
	if( pVehicleList )
	{
		MemFree( pVehicleList );
		pVehicleList = NULL;
		ubNumberOfVehicles = 0;
	}
*/
}

BOOLEAN IsThisVehicleAccessibleToSoldier( SOLDIERTYPE *pSoldier, INT32 iId )
{
	if( pSoldier == NULL )
	{
		return( FALSE );
	}

	const VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return FALSE;

	// if the soldier or the vehicle is between sectors
	if (pSoldier->fBetweenSectors || v->fBetweenSectors)
	{
		return( FALSE );
	}

	// any sector values off?
	if (pSoldier->sSectorX != v->sSectorX ||
			pSoldier->sSectorY != v->sSectorY ||
			pSoldier->bSectorZ != v->sSectorZ)
	{
		return( FALSE );
	}

	// if vehicle is not ok to use then return false
	if (!OKUseVehicle(g_vehicle_type_info[v->ubVehicleType].profile))
	{
		return( FALSE );
	}

	return( TRUE );
}


static BOOLEAN AddSoldierToVehicle(SOLDIERTYPE* pSoldier, INT32 iId)
{
	SOLDIERTYPE *pVehicleSoldier = NULL;

	// ok now check if any free slots in the vehicle

	// now check if vehicle is valid
	VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return FALSE;

	// get the vehicle soldiertype
	pVehicleSoldier = GetSoldierStructureForVehicle( iId );

	if( pVehicleSoldier )
	{
		if ( pVehicleSoldier->bTeam != gbPlayerNum )
		{
			// Change sides...
			pVehicleSoldier = ChangeSoldierTeam( pVehicleSoldier, gbPlayerNum );
			// add it to mapscreen list
			fReBuildCharacterList = TRUE;
		}
	}

	// If vehicle is empty, add to unique squad now that it has somebody in it!
	if ( GetNumberInVehicle( iId ) == 0 && pVehicleSoldier )
	{
		// 2 ) Add to unique squad...
		AddCharacterToUniqueSquad( pVehicleSoldier );

		// ATE: OK funcky stuff here!
		// We have now a guy on a squad group, remove him!
		RemovePlayerFromGroup( SquadMovementGroups[ pVehicleSoldier->bAssignment ], pVehicleSoldier  );

    // I really have vehicles.
    // ONLY add to vehicle group once!
		if (!DoesPlayerExistInPGroup(v->ubMovementGroup, pVehicleSoldier))
    {
		  //NOW.. add guy to vehicle group....
			AddPlayerToGroup(v->ubMovementGroup, pVehicleSoldier);
    }
    else
    {
			pVehicleSoldier->ubGroupID = v->ubMovementGroup;
    }
	}

	// check if the grunt is already here
	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if (v->pPassengers[iCounter] == pSoldier)
		{
			// guy found, no need to add
			return( TRUE );
		}
	}

	if ( pVehicleSoldier )
	{
		// can't call SelectSoldier in mapscreen, that will initialize interface panels!!!
	  if ( guiCurrentScreen == GAME_SCREEN )
		{
			SelectSoldier(pVehicleSoldier, SELSOLDIER_FORCE_RESELECT);
		}

		PlayLocationJA2Sample(pVehicleSoldier->sGridNo, g_vehicle_type_info[pVehicleList[pVehicleSoldier->bVehicleID].ubVehicleType].enter_sound, HIGHVOLUME, 1);
	}

	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		// check if slot free
		if (v->pPassengers[iCounter] == NULL)
		{
			// add person in
			v->pPassengers[iCounter] = pSoldier;

			if( pSoldier->bAssignment == VEHICLE )
			{
				TakeSoldierOutOfVehicle( pSoldier );
				// NOTE: This will leave the soldier on a squad.  Must be done PRIOR TO and in AS WELL AS the call
				// to RemoveCharacterFromSquads() that's coming up, to permit direct vehicle->vehicle reassignment!
			}

			// if in a squad, remove from squad, if not, check if in vehicle, if so remove, if not, then check if in mvt group..if so, move and destroy group
			if( pSoldier->bAssignment < ON_DUTY )
			{
				RemoveCharacterFromSquads( pSoldier );
			}
			else if( pSoldier->ubGroupID != 0 )
			{
				// destroy group and set to zero
				RemoveGroup( pSoldier->ubGroupID );
				pSoldier->ubGroupID = 0;
			}

			if( ( pSoldier->bAssignment != VEHICLE ) || ( 	pSoldier -> iVehicleId != iId ) )
			{
				SetTimeOfAssignmentChangeForMerc( pSoldier );
			}

			// set thier assignment
			ChangeSoldiersAssignment( pSoldier, VEHICLE );

			// set vehicle id
			pSoldier -> iVehicleId = iId;

			// if vehicle is part of mvt group, then add character to mvt group
			if (v->ubMovementGroup != 0)
			{
				// add character
				AddPlayerToGroup(v->ubMovementGroup, pSoldier);
			}

			// Are we the first?
			if ( GetNumberInVehicle(  iId ) == 1 )
			{
				// Set as driver...
				pSoldier->uiStatusFlags |= SOLDIER_DRIVER;
			}
			else
			{
				// Set as driver...
				pSoldier->uiStatusFlags |= SOLDIER_PASSENGER;
			}

			// Remove soldier's graphic
			RemoveSoldierFromGridNo( pSoldier );

			if ( pVehicleSoldier )
			{
				// Set gridno for vehicle.....
				EVENT_SetSoldierPositionXY(pSoldier, pVehicleSoldier->dXPos, pVehicleSoldier->dYPos, SSP_NONE);

				// Stop from any movement.....
				EVENT_StopMerc( pSoldier, pSoldier->sGridNo, pSoldier->bDirection );

				// can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface panels!!!
				if ( guiCurrentScreen == GAME_SCREEN )
				{
					SetCurrentSquad( pVehicleSoldier->bAssignment, TRUE );
				}
			}

			return( TRUE );
		}
	}

		// no slots, leave
	return( FALSE );
}


void SetSoldierExitVehicleInsertionData( SOLDIERTYPE *pSoldier, INT32 iId )
{
	if ( iId == iHelicopterVehicleId && !pSoldier->bInSector )
  {
	  if( pSoldier->sSectorX  != BOBBYR_SHIPPING_DEST_SECTOR_X || pSoldier->sSectorY != BOBBYR_SHIPPING_DEST_SECTOR_Y || pSoldier->bSectorZ != BOBBYR_SHIPPING_DEST_SECTOR_Z )
    {
      // Not anything different here - just use center gridno......
		  pSoldier->ubStrategicInsertionCode = INSERTION_CODE_CENTER;
    }
    else
    {
      // This is drassen, make insertion gridno specific...
		  pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		  pSoldier->usStrategicInsertionData = 10125;
    }
  }
}


static void TeleportVehicleToItsClosestSector(UINT8 ubGroupID);


// remove soldier from vehicle
static BOOLEAN RemoveSoldierFromVehicle(SOLDIERTYPE* pSoldier, INT32 iId)
{
	// remove soldier from vehicle
	BOOLEAN fSoldierLeft = FALSE;
	BOOLEAN	fSoldierFound = FALSE;
	SOLDIERTYPE *pVehicleSoldier;

	VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return FALSE;

	// now look for the grunt
	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if (v->pPassengers[iCounter] == pSoldier)
		{
			fSoldierFound = TRUE;

			v->pPassengers[iCounter]->ubGroupID = 0;
			v->pPassengers[iCounter]->sSectorY  = v->sSectorY;
			v->pPassengers[iCounter]->sSectorX  = v->sSectorX;
			v->pPassengers[iCounter]->bSectorZ  = v->sSectorZ;
			v->pPassengers[iCounter]            = NULL;


			pSoldier->uiStatusFlags &= ( ~( SOLDIER_DRIVER | SOLDIER_PASSENGER ) );

			// check if anyone left in vehicle
			fSoldierLeft = FALSE;
			for (iCounter = 0; iCounter < seats; ++iCounter)
			{
				if (v->pPassengers[iCounter] != NULL)
				{
					fSoldierLeft = TRUE;
				}
			}


			if (v->ubMovementGroup != 0)
			{
				RemovePlayerFromGroup(v->ubMovementGroup, pSoldier);
			}

			break;
		}
	}


	if ( !fSoldierFound )
	{
		return( FALSE );
	}


	// Are we the last?
	//if ( GetNumberInVehicle( iId ) == 0 )
	if( fSoldierLeft == FALSE )
	{
		// is the vehicle the helicopter?..it can continue moving when no soldiers aboard (Skyrider remains)
		if( iId != iHelicopterVehicleId )
		{
			pVehicleSoldier = GetSoldierStructureForVehicle( iId );
			Assert ( pVehicleSoldier );

			if ( pVehicleSoldier )
			{
				// and he has a route set
				if ( GetLengthOfMercPath( pVehicleSoldier ) > 0 )
				{
					// cancel the entire path (also handles reversing directions)
					CancelPathForVehicle(v, FALSE);
				}

				// if the vehicle was abandoned between sectors
				if (v->fBetweenSectors)
				{
					// teleport it to the closer of its current and next sectors (it beats having it arrive empty later)
					TeleportVehicleToItsClosestSector(pVehicleSoldier->ubGroupID);
				}

        // Remove vehicle from squad.....
        RemoveCharacterFromSquads( pVehicleSoldier );
        // ATE: Add him back to vehicle group!
				if (!DoesPlayerExistInPGroup(v->ubMovementGroup, pVehicleSoldier))
        {
					AddPlayerToGroup(v->ubMovementGroup, pVehicleSoldier);
        }
				ChangeSoldiersAssignment( pVehicleSoldier, ASSIGNMENT_EMPTY );


/* ARM Removed Feb. 17, 99 - causes pVehicleSoldier->ubGroupID to become 0, which will cause assert later on
				RemovePlayerFromGroup( pVehicleSoldier->ubGroupID, pVehicleSoldier );
*/

/*
				// Change sides...
				pVehicleSoldier = ChangeSoldierTeam( pVehicleSoldier, CIV_TEAM );
				// subtract it from mapscreen list
				fReBuildCharacterList = TRUE;

				RemoveCharacterFromSquads( pVehicleSoldier );
*/
			}
		}
	}


	// if he got out of the chopper
	if ( iId == iHelicopterVehicleId )
	{
		// and he's alive
		if ( pSoldier->bLife >= OKLIFE )
		{
			// mark the sector as visited (flying around in the chopper doesn't, so this does it as soon as we get off it)
			SetSectorFlag( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ, SF_ALREADY_VISITED );
		}

    SetSoldierExitVehicleInsertionData( pSoldier, iId );

    // Update in sector if this is the current sector.....
		if ( pSoldier->sSectorX == gWorldSectorX && pSoldier->sSectorY == gWorldSectorY && pSoldier->bSectorZ == gbWorldSectorZ )
		{
			UpdateMercInSector( pSoldier, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		}

	}

	// soldier successfully removed
	return( TRUE );
}


BOOLEAN MoveCharactersPathToVehicle( SOLDIERTYPE *pSoldier )
{
	INT32 iId;
	// valid soldier?
	if( pSoldier == NULL )
	{
		return ( FALSE );
	}

	// check if character is in fact in a vehicle
	if( ( pSoldier->bAssignment != VEHICLE ) && ( ! ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) ) )
	{
		// now clear soldier's path
		pSoldier->pMercPath = ClearStrategicPathList( pSoldier->pMercPath, 0 );
		return( FALSE );
	}

	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// grab the id the character is
		iId = pSoldier->bVehicleID;
	}
	else
	{
		// grab the id the character is
		iId = pSoldier->iVehicleId;
	}

	// check if vehicle is valid
	if( iId != -1 )
	{
		// check if vehicle has mvt group, if not, get one for it
		if (GetVehicle(iId) == NULL)
		{
			// now clear soldier's path
			pSoldier->pMercPath = ClearStrategicPathList( pSoldier->pMercPath, 0 );
			return ( FALSE );
		}
	}

	// valid vehicle

	// now clear soldier's path
	pVehicleList[ iId ].pMercPath = ClearStrategicPathList( pVehicleList[ iId ].pMercPath, pVehicleList[ iId ].ubMovementGroup );

	// now copy over
	pVehicleList[ iId ].pMercPath = CopyPaths( pSoldier->pMercPath, pVehicleList[ iId ].pMercPath );

	// move to beginning
	pVehicleList[ iId ].pMercPath = MoveToBeginningOfPathList( pVehicleList[ iId ].pMercPath );

	// now clear soldier's path
	pSoldier->pMercPath = ClearStrategicPathList( pSoldier->pMercPath, 0 );

	return( TRUE );
}


// set up soldier mvt for vehicle
static BOOLEAN CopyVehiclePathToSoldier(SOLDIERTYPE* pSoldier)
{
	INT32 iId;

	// valid soldier?
	if( pSoldier == NULL )
	{
		return ( FALSE );
	}

	// check if character is in fact in a vehicle
	if( ( pSoldier->bAssignment != VEHICLE ) && ( ! ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) ) )
	{
		return( FALSE );
	}

	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// grab the id the character is
		iId = pSoldier->bVehicleID;
	}
	else
	{
		// grab the id the character is
		iId = pSoldier->iVehicleId;
	}

	// check if vehicle is valid
	if( iId != -1 )
	{
		// check if vehicle has mvt group, if not, get one for it
		if (GetVehicle(iId) == NULL)
		{
			return ( FALSE );
		}
	}


	// reset mvt group for the grunt
	// ATE: NOT if we are the vehicle
	if ( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		pSoldier->ubGroupID = pVehicleList[ iId ].ubMovementGroup;
	}

	// valid vehicle

	// clear grunt path
	if( pSoldier->pMercPath )
	{
		// clear soldier's path
		pSoldier->pMercPath = ClearStrategicPathList( pSoldier->pMercPath, 0 );
	}

	// now copy over
	pSoldier->pMercPath = CopyPaths(pVehicleList[ iId ].pMercPath, pSoldier->pMercPath );

	return( TRUE );

}


BOOLEAN SetUpMvtGroupForVehicle( SOLDIERTYPE *pSoldier )
{
	// given this grunt, find out if asscoiated vehicle has a mvt group, if so, set this grunts mvt group tho the vehicle
	// for pathing purposes, will be reset to zero in copying of path
	INT32 iId = 0;

		// check if character is in fact in a vehicle
	if( ( pSoldier->bAssignment != VEHICLE ) && ( ! ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) ) )
	{
		return( FALSE );
	}

	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// grab the id the character is
		iId = pSoldier->bVehicleID;
	}
	else
	{
		// grab the id the character is
		iId = pSoldier->iVehicleId;
	}

	if( pSoldier->pMercPath )
	{
		// clear soldier's path
		pSoldier->pMercPath = ClearStrategicPathList( pSoldier->pMercPath, pSoldier->ubGroupID );
	}

	// if no group, create one for vehicle
	//if( pVehicleList[ iId ].ubMovementGroup == 0 )
	//{
		// get the vehicle a mvt group
		//pVehicleList[ iId ].ubMovementGroup = CreateNewVehicleGroupDepartingFromSector( ( UINT8 )( pVehicleList[ iId ].sSectorX ), ( UINT8 )( pVehicleList[ iId ].sSectorY ), iId );
		//pVehicleList[ iId ].ubMovementGroup = CreateNewVehicleGroupDepartingFromSector( ( UINT8 )( pVehicleList[ iId ].sSectorX ), ( UINT8 )( pVehicleList[ iId ].sSectorY ), iId );


		// add everyone in vehicle to this mvt group
		//const INT32 seats = GetVehicleSeats(&pVehicleList[iId]);
		//for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
		//{
		//	if( pVehicleList[ iId ].pPassengers[ iCounter ] != NULL )
		//	{
		//			// add character
		//		AddPlayerToGroup( pVehicleList[ iId ].ubMovementGroup, pVehicleList[ iId ].pPassengers[ iCounter ] );
		//	}
		//}
	//}

	CopyVehiclePathToSoldier( pSoldier );

	// set up mvt group
	pSoldier->ubGroupID = pVehicleList[ iId ].ubMovementGroup;


	return ( TRUE );
}


VEHICLETYPE* GetVehicle(const INT32 vehicle_id)
{
	if (vehicle_id < 0 || ubNumberOfVehicles <= vehicle_id) return FALSE;
	VEHICLETYPE* const v = &pVehicleList[vehicle_id];
	return v->fValid ? v : NULL;
}


void UpdatePositionOfMercsInVehicle(const VEHICLETYPE* const v)
{
	// go through list of mercs in vehicle and set all thier states as arrived
	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		SOLDIERTYPE* const s = v->pPassengers[iCounter];
		if (s == NULL) continue;

		s->sSectorY        = v->sSectorY;
		s->sSectorX        = v->sSectorX;
		s->fBetweenSectors = FALSE;
	}
}


INT32 GetVehicleIDFromMvtGroup(const GROUP* const group)
{
	// given the id of a mvt group, find a vehicle in this group
	CFOR_ALL_VEHICLES(v)
	{
		if (v->ubMovementGroup == group->ubGroupID)
		{
			return VEHICLE2ID(v);
		}
	}

	return -1;
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
	SoldierTakeDamage(pSoldier, 0, 100, 100, TAKE_DAMAGE_BLOODLOSS, NULL, NOWHERE, TRUE);
	return TRUE;
}


BOOLEAN KillAllInVehicle(const VEHICLETYPE* const v)
{
	// go through list of occupants and kill them
	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if (v->pPassengers[iCounter] != NULL)
		{
			if (!KillPersonInVehicle(v->pPassengers[iCounter]))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

INT32 GetNumberInVehicle( INT32 iId )
{
	// go through list of occupants in vehicles and count them
	INT32 iCount = 0;

	const VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return 0;

	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if (v->pPassengers[iCounter] != NULL)
		{
			iCount++;
		}
	}

	return( iCount );
}

INT32 GetNumberOfNonEPCsInVehicle( INT32 iId )
{
	// go through list of occupants in vehicles and count them
	INT32 iCount = 0;

	const VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return 0;

	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if (v->pPassengers[iCounter] != NULL && !AM_AN_EPC(v->pPassengers[iCounter]))
		{
			iCount++;
		}
	}

	return( iCount );
}

BOOLEAN IsRobotControllerInVehicle( INT32 iId )
{
	const VEHICLETYPE* const v = GetVehicle(iId);
	if (v == NULL) return FALSE;

	const INT32 seats = GetVehicleSeats(v);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		const SOLDIERTYPE* const pSoldier = v->pPassengers[iCounter];
		if ( pSoldier != NULL && ControllingRobot( pSoldier ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


BOOLEAN AnyAccessibleVehiclesInSoldiersSector( SOLDIERTYPE *pSoldier )
{
	CFOR_ALL_VEHICLES(v)
	{
		if (IsThisVehicleAccessibleToSoldier(pSoldier, VEHICLE2ID(v)))
		{
			return TRUE;
		}
	}
	return FALSE;
}


BOOLEAN IsEnoughSpaceInVehicle( INT32 iID )
{
	const VEHICLETYPE* const v = GetVehicle(iID);
	return
		v != NULL &&
		GetNumberInVehicle(iID) != GetVehicleSeats(v);
}


BOOLEAN PutSoldierInVehicle( SOLDIERTYPE *pSoldier, INT8 bVehicleId )
{

	SOLDIERTYPE *pVehicleSoldier = NULL;

	if( ( pSoldier->sSectorX != gWorldSectorX ) || ( pSoldier->sSectorY != gWorldSectorY ) || ( pSoldier->bSectorZ != 0 ) || ( bVehicleId  == iHelicopterVehicleId ) )
	{
		// add the soldier
		return( AddSoldierToVehicle( pSoldier, bVehicleId ) );
	}
	else
	{
		// grab the soldier struct for the vehicle
		pVehicleSoldier = GetSoldierStructureForVehicle( bVehicleId );

		// enter the vehicle
		return( EnterVehicle( pVehicleSoldier, pSoldier ) );
	}
}


BOOLEAN TakeSoldierOutOfVehicle( SOLDIERTYPE *pSoldier )
{
	// if not in vehicle, don't take out, not much point, now is there?
	if( pSoldier->bAssignment != VEHICLE )
	{
		return( FALSE );
	}

	if( ( pSoldier->sSectorX != gWorldSectorX ) || ( pSoldier->sSectorY != gWorldSectorY ) || ( pSoldier->bSectorZ != 0 ) || !pSoldier->bInSector )
	{
		// add the soldier
		return( RemoveSoldierFromVehicle( pSoldier, pSoldier->iVehicleId ) );
	}
	else
	{
		// helicopter isn't a soldiertype instance
		if( pSoldier->iVehicleId == iHelicopterVehicleId )
		{
			return( RemoveSoldierFromVehicle( pSoldier, pSoldier->iVehicleId ) );
		}
		else
		{
			// exit the vehicle
			return( ExitVehicle( pSoldier ) );
		}
	}
}


BOOLEAN EnterVehicle(const SOLDIERTYPE* pVehicle, SOLDIERTYPE* pSoldier)
{
	// TEST IF IT'S VALID...
	if ( pVehicle->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// Is there room...
		if ( IsEnoughSpaceInVehicle( pVehicle->bVehicleID ) )
		{
			// OK, add....
			AddSoldierToVehicle( pSoldier, pVehicle->bVehicleID );

			if ( !(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
			{
				// Change to team panel if we are not already...
				SetCurrentInterfacePanel( TEAM_PANEL );
			}

			return( TRUE );
		}
	}

	return( FALSE );
}


static SOLDIERTYPE* GetVehicleSoldierPointerFromPassenger(SOLDIERTYPE* pSrcSoldier)
{
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		if (s->uiStatusFlags & SOLDIER_VEHICLE && s->bVehicleID == pSrcSoldier->iVehicleId)
		{
			return s;
		}
	}
	return NULL;
}


BOOLEAN ExitVehicle( SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE		*pVehicle;
	INT16					sGridNo;

	// Get vehicle from soldier...
	pVehicle = GetVehicleSoldierPointerFromPassenger( pSoldier );

	if ( pVehicle == NULL )
	{
		return( FALSE );
	}

	// TEST IF IT'S VALID...
	if ( pVehicle->uiStatusFlags & SOLDIER_VEHICLE )
	{
		sGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, pSoldier->usUIMovementMode, 5, 3, pVehicle);

		if ( sGridNo == NOWHERE )
		{
			// ATE: BUT we need a place, widen the search
			sGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, pSoldier->usUIMovementMode, 20, 3, pVehicle);
		}

		// OK, remove....
		RemoveSoldierFromVehicle( pSoldier, pVehicle->bVehicleID );

		// Were we the driver, and if so, pick another....
		pSoldier->sInsertionGridNo = sGridNo;
		pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		pSoldier->usStrategicInsertionData = pSoldier->sInsertionGridNo;
		pSoldier->iVehicleId = -1;

		//AllTeamsLookForAll( FALSE );
		pSoldier->bOppList[ pVehicle->ubID ] = 1;

		// Add to sector....
		EVENT_SetSoldierPosition(pSoldier, sGridNo, SSP_NONE);

		// Update visiblity.....
		HandleSight(pSoldier,SIGHT_LOOK | SIGHT_RADIO );

		// Add to unique squad....
		AddCharacterToUniqueSquad( pSoldier );

		// can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface panels!!!
	  if ( guiCurrentScreen == GAME_SCREEN )
		{
			SetCurrentSquad( pSoldier->bAssignment, TRUE );
			SelectSoldier(pSoldier, SELSOLDIER_FORCE_RESELECT);
		}

		PlayLocationJA2Sample(pVehicle->sGridNo, g_vehicle_type_info[pVehicleList[pVehicle->bVehicleID].ubVehicleType].enter_sound, HIGHVOLUME, 1);
		return( TRUE );
	}

	return( FALSE );
}


void AddPassangersToTeamPanel( INT32 iId )
{
	const INT32 seats = GetVehicleSeats(&pVehicleList[iId]);
	for (INT32 cnt = 0; cnt < seats; ++cnt)
	{
		if( pVehicleList[ iId ].pPassengers[ cnt ] != NULL )
		{
			// add character
			AddPlayerToInterfaceTeamSlot(pVehicleList[iId].pPassengers[cnt]);
		}
	}
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

	if( pVehicleList[ ubID ].fDestroyed == FALSE )
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
	SOLDIERTYPE *pSoldier;
	BOOLEAN	fMadeCorpse = FALSE;

	pSoldier = GetSoldierStructureForVehicle( ubID );

	if ( sDmg > pSoldier->bLife )
	{
		pSoldier->bLife = 0;
	}
	else
	{
		// Decrease Health
		pSoldier->bLife -= sDmg;
	}

	if ( pSoldier->bLife < OKLIFE )
	{
		pSoldier->bLife = 0;
	}

	//Show damage
	pSoldier->sDamage += sDmg;

	if ( pSoldier->bInSector && pSoldier->bVisible != -1 )
	{
		// If we are already dead, don't show damage!
		if ( sDmg != 0 )
		{
			// Display damage
			INT16 sMercScreenX, sMercScreenY, sOffsetX, sOffsetY;

			// Set Damage display counter
			pSoldier->fDisplayDamage = TRUE;
			pSoldier->bDisplayDamageCount = 0;

			GetSoldierScreenPos( pSoldier, &sMercScreenX, &sMercScreenY );
			GetSoldierAnimOffsets( pSoldier, &sOffsetX, &sOffsetY );
			pSoldier->sDamageX = sOffsetX;
			pSoldier->sDamageY = sOffsetY;
		}
	}

	VEHICLETYPE* const v = &pVehicleList[ubID];
	if (pSoldier->bLife == 0 && !v->fDestroyed)
	{
		v->fDestroyed = TRUE;

		// Explode vehicle...
		IgniteExplosion(att, 0, sGridNo, GREAT_BIG_EXPLOSION, 0);

		if ( pSoldier != NULL )
		{
			// Tacticlly remove soldier....
			// EVENT_InitNewSoldierAnim( pSoldier, VEHICLE_DIE, 0, FALSE );
			//TacticalRemoveSoldier(pSoldier);

			CheckForAndHandleSoldierDeath( pSoldier, &fMadeCorpse );
		}

    KillAllInVehicle(v);
	}
}



BOOLEAN DoesVehicleNeedAnyRepairs( INT32 iVehicleId )
{
	SOLDIERTYPE		*pVehicleSoldier = NULL;

	const VEHICLETYPE* const v = GetVehicle(iVehicleId);
	if (v == NULL) return FALSE;

	// Skyrider isn't damagable/repairable
	if ( iVehicleId == iHelicopterVehicleId )
	{
		return( FALSE );
	}

	// get the vehicle soldiertype
	pVehicleSoldier = GetSoldierStructureForVehicle( iVehicleId );

	if ( pVehicleSoldier->bLife != pVehicleSoldier->bLifeMax )
	{
		return( TRUE );
	}

	// everything is in perfect condition
	return( FALSE);
}


INT8 RepairVehicle( INT32 iVehicleId, INT8 bRepairPtsLeft, BOOLEAN *pfNothingToRepair )
{
	SOLDIERTYPE		*pVehicleSoldier = NULL;
	INT8					bRepairPtsUsed = 0;
	INT8					bOldLife;

	const VEHICLETYPE* const v = GetVehicle(iVehicleId);
	if (v == NULL) return bRepairPtsUsed;

	if (!DoesVehicleNeedAnyRepairs(iVehicleId))
	{
		return( bRepairPtsUsed );
	}

	// get the vehicle soldiertype
	pVehicleSoldier = GetSoldierStructureForVehicle( iVehicleId );

	bOldLife = pVehicleSoldier->bLife;

	// Repair
	pVehicleSoldier->bLife += ( bRepairPtsLeft / VEHICLE_REPAIR_POINTS_DIVISOR );

	// Check
	if ( pVehicleSoldier->bLife > pVehicleSoldier->bLifeMax )
	{
		pVehicleSoldier->bLife = pVehicleSoldier->bLifeMax;
	}

	// Calculate pts used;
	bRepairPtsUsed = ( pVehicleSoldier->bLife - bOldLife ) * VEHICLE_REPAIR_POINTS_DIVISOR;

	// ARM: personally, I'd love to know where in Arulco the mechanic gets the PARTS to do this stuff, but hey, it's a game!
	(*pfNothingToRepair) = !DoesVehicleNeedAnyRepairs( iVehicleId );

	return( bRepairPtsUsed );
}


/*
INT16 GetOrigInternalArmorValueForVehicleInLocation( UINT8 ubID, UINT8 ubLocation )
{
	INT16 sArmorValue = 0;

	sArmorValue = sVehicleInternalOrigArmorValues[ pVehicleList[ ubID ].ubVehicleType ][ ubLocation ];

	// return the armor value
	return( sArmorValue );
}
*/

SOLDIERTYPE * GetSoldierStructureForVehicle( INT32 iId )
{
	FOR_ALL_SOLDIERS(s)
	{
		if (s->uiStatusFlags & SOLDIER_VEHICLE &&
				s->bVehicleID == iId)
		{
			return s;
		}
	}

	return NULL;
}


// set up armor values for this vehicle
static void SetUpArmorForVehicle(UINT8 ubID)
{
	INT32 iCounter = 0;

/*
	// set up the internal and external armor for vehicles
	for( iCounter = 0; iCounter < NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE; iCounter++ )
	{
		pVehicleList[ ubID ].sInternalHitLocations[ iCounter ] = sVehicleInternalOrigArmorValues[ pVehicleList[ ubID ].ubVehicleType ][ iCounter ];
	}


	for( iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE; iCounter++ )
	{
		pVehicleList[ ubID ].sExternalArmorLocationsStatus[ iCounter ] = 100;
	}
	*/
}

void AdjustVehicleAPs( SOLDIERTYPE *pSoldier, UINT8 *pubPoints )
{
	UINT8 pubDeducations = 0;
	INT32 iCounter = 0;

	(*pubPoints) += 35;

	// check for state of critcals


	// handle for each engine crit
	pubDeducations += pVehicleList[ pSoldier->bVehicleID ].sCriticalHits[ ENGINE_HIT_LOCATION ] * COST_PER_ENGINE_CRIT;

	// handle each tire
	for( iCounter = RF_TIRE_HIT_LOCATION; iCounter < LR_TIRE_HIT_LOCATION; iCounter++ )
	{
		if( pVehicleList[ pSoldier->bVehicleID ].sCriticalHits[ iCounter ] )
		{
			pubDeducations += COST_PER_TIRE_HIT;
		}
	}

	// make sure we don't go too far
	if( pubDeducations > (*pubPoints) )
	{
		pubDeducations = (*pubPoints);
	}

	// now deduct pts
	(*pubPoints) -= pubDeducations;
}



BOOLEAN SaveVehicleInformationToSaveGameFile( HWFILE hFile )
{
	PathSt* pTempPathPtr;
	UINT32		uiNodeCount=0;
	UINT8		cnt;

	//Save the number of elements
	if (!FileWrite(hFile, &ubNumberOfVehicles, sizeof(UINT8))) return FALSE;

	//loop through all the vehicles and save each one
	for( cnt=0; cnt< ubNumberOfVehicles; cnt++ )
	{
		//save if the vehicle spot is valid
		if (!FileWrite(hFile, &pVehicleList[cnt].fValid, sizeof(BOOLEAN))) return FALSE;

		if( pVehicleList[cnt].fValid )
		{
			if (!InjectVehicleTypeIntoFile(hFile, &pVehicleList[cnt])) return FALSE;

			//count the number of nodes in the vehicles path
			uiNodeCount=0;
			pTempPathPtr = pVehicleList[cnt].pMercPath;
			while( pTempPathPtr )
			{
				uiNodeCount++;
				pTempPathPtr = pTempPathPtr->pNext;
			}

			//Save the number of nodes
			if (!FileWrite(hFile, &uiNodeCount, sizeof(UINT32))) return FALSE;

			//save all the nodes
			pTempPathPtr = pVehicleList[cnt].pMercPath;
			while( pTempPathPtr )
			{
				//Save the node
				if (!FileWrite(hFile, pTempPathPtr, sizeof(PathSt))) return FALSE;

				pTempPathPtr = pTempPathPtr->pNext;
			}
		}
	}

	return( TRUE );
}


BOOLEAN LoadVehicleInformationFromSavedGameFile( HWFILE hFile, UINT32 uiSavedGameVersion )
{
	UINT32		uiTotalNodeCount=0;
	UINT8			cnt;
	UINT32		uiNodeCount=0;
	PathSt		*pPath=NULL;
	PathSt		*pTempPath;

	//Clear out th vehicle list
	ClearOutVehicleList( );

	//Load the number of elements
	if (!FileRead(hFile, &ubNumberOfVehicles, sizeof(UINT8))) return FALSE;

	if( ubNumberOfVehicles != 0 )
	{
		//allocate memory to hold the vehicle list
		pVehicleList = MemAlloc( sizeof( VEHICLETYPE ) * ubNumberOfVehicles );
		if( pVehicleList == NULL )
			return( FALSE );
		memset( pVehicleList, 0, sizeof( VEHICLETYPE ) * ubNumberOfVehicles );


		//loop through all the vehicles and load each one
		for( cnt=0; cnt< ubNumberOfVehicles; cnt++ )
		{
			//Load if the vehicle spot is valid
			if (!FileRead(hFile, &pVehicleList[cnt].fValid, sizeof(BOOLEAN))) return FALSE;

			if( pVehicleList[cnt].fValid )
			{
				if (!ExtractVehicleTypeFromFile(hFile, &pVehicleList[cnt], uiSavedGameVersion)) return FALSE;

				//Load the number of nodes
				if (!FileRead(hFile, &uiTotalNodeCount, sizeof(UINT32))) return FALSE;

				if( uiTotalNodeCount != 0 )
				{
					pPath = NULL;

					pVehicleList[cnt].pMercPath = NULL;

					//loop through each node
					for( uiNodeCount=0; uiNodeCount<uiTotalNodeCount; uiNodeCount++ )
					{
						//allocate memory to hold the vehicle path
						pTempPath = MemAlloc( sizeof( PathSt ) );
						if( pTempPath == NULL )
							return( FALSE );
						memset( pTempPath, 0, sizeof( PathSt ) );

						//Load all the nodes
						if (!FileRead(hFile, pTempPath, sizeof(PathSt))) return FALSE;

						//
						//Setup the pointer info
						//

						if( pVehicleList[cnt].pMercPath == NULL )
							pVehicleList[cnt].pMercPath = pTempPath;


						//if there is a previous node
						if( pPath != NULL )
						{
							pPath->pNext = pTempPath;

							pTempPath->pPrev = pPath;
						}
						else
							pTempPath->pPrev = NULL;


						pTempPath->pNext = NULL;

						pPath = pTempPath;
					}
				}
				else
				{
					pVehicleList[cnt].pMercPath = NULL;
				}
			}
		}
	}
	return( TRUE );
}


void SetVehicleSectorValues(VEHICLETYPE* const v, const UINT8 ubSectorX, const UINT8 ubSectorY)
{
	v->sSectorX = ubSectorX;
	v->sSectorY = ubSectorY;

	MERCPROFILESTRUCT* const p = GetProfile(g_vehicle_type_info[v->ubVehicleType].profile);
	p->sSectorX = ubSectorX;
	p->sSectorY = ubSectorY;
}

void UpdateAllVehiclePassengersGridNo( SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE *pPassenger;

	// If not a vehicle, ignore!
	if ( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		return;
	}

	const INT32 iId = pSoldier->bVehicleID;

	// Loop through passengers and update each guy's position
	const INT32 seats = GetVehicleSeats(&pVehicleList[iId]);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if( pVehicleList[ iId ].pPassengers[ iCounter ] != NULL )
		{
			pPassenger = pVehicleList[ iId ].pPassengers[ iCounter ];

			// Set gridno.....
			EVENT_SetSoldierPositionXY(pPassenger, pSoldier->dXPos, pSoldier->dYPos, SSP_NONE);
		}
	}
}


BOOLEAN LoadVehicleMovementInfoFromSavedGameFile( HWFILE hFile )
{
	INT32		cnt;
	GROUP		*pGroup	=	NULL;

	//Load in the Squad movement id's
	if (!FileRead(hFile, gubVehicleMovementGroups, sizeof(INT8) * 5)) return FALSE;

	for( cnt = 5; cnt <  MAX_VEHICLES; cnt++ )
	{
		// create mvt groups
		gubVehicleMovementGroups[ cnt ] = CreateNewVehicleGroupDepartingFromSector( 1, 1, cnt );

		// Set persistent....
		pGroup = GetGroup( gubVehicleMovementGroups[ cnt ] );
		pGroup->fPersistant = TRUE;
	}

	return( TRUE );
}


BOOLEAN NewSaveVehicleMovementInfoToSavedGameFile( HWFILE hFile )
{
	//Save all the vehicle movement id's
	if (!FileWrite(hFile, gubVehicleMovementGroups, sizeof(INT8) * MAX_VEHICLES)) return FALSE;

	return( TRUE );
}


BOOLEAN NewLoadVehicleMovementInfoFromSavedGameFile( HWFILE hFile )
{
	//Load in the Squad movement id's
	if (!FileRead(hFile, gubVehicleMovementGroups, sizeof(INT8) * MAX_VEHICLES)) return FALSE;

	return( TRUE );
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
	Assert( ( pGroup->uiTraverseTime > 0 ) && ( pGroup->uiTraverseTime != 0xffffffff ) );

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
	PlaceGroupInSector( ubGroupID, sPrevX, sPrevY, sNextX, sNextY, 0, FALSE );
}


void AddVehicleFuelToSave( )
{
	CFOR_ALL_VEHICLES(v)
	{
		// get the vehicle soldiertype
		SOLDIERTYPE* const pVehicleSoldier = GetSoldierStructureForVehicle(VEHICLE2ID(v));
		if( pVehicleSoldier )
		{
			// Init fuel!
			pVehicleSoldier->sBreathRed = 10000;
			pVehicleSoldier->bBreath    = 100;
		}
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

	if( !fIgnoreAsleep && ( pSoldier->fMercAsleep == TRUE ) )
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

	const VEHICLETYPE* const v = GetVehicle(iVehicleId);
	if (v == NULL) return FALSE;

	// if vehicle is not going anywhere, then nobody has to be driving it!
	// need the path length check in case we're doing a test while actually in a sector even though we're moving!
	if (!fTryingToTravel && !v->fBetweenSectors && GetLengthOfPath(v->pMercPath) == 0)
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
	SOLDIERTYPE *pVehicleSoldier;


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

	pVehicleSoldier = GetSoldierStructureForVehicle( iVehicleId );
	Assert( pVehicleSoldier );

	// check squad vehicle is on
	if ( pVehicleSoldier->bAssignment != bSquadNumber )
	{
		return( FALSE );
	}


	// yes, he's in a vehicle assigned to this squad
	return( TRUE );
}


SOLDIERTYPE*  PickRandomPassengerFromVehicle( SOLDIERTYPE *pSoldier )
{
	UINT8	ubMercsInSector[ 20 ] = { 0 };
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;

	// If not a vehicle, ignore!
	if ( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		return( NULL );
	}

	const INT32 iId = pSoldier->bVehicleID;

	// Loop through passengers and update each guy's position
	const INT32 seats = GetVehicleSeats(&pVehicleList[iId]);
	for (INT32 iCounter = 0; iCounter < seats; ++iCounter)
	{
		if( pVehicleList[ iId ].pPassengers[ iCounter ] != NULL )
		{
			ubMercsInSector[ ubNumMercs ] = (UINT8)iCounter;
			ubNumMercs++;
		}
	}

	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		// If we are air raid, AND red exists somewhere...
		return pVehicleList[iId].pPassengers[ubChosenMerc]; // XXX TODO000D
  }

  return( NULL );
}


static BOOLEAN DoesVehicleHaveAnyPassengers(INT32 iVehicleID)
{
	if( !GetNumberInVehicle( iVehicleID ) )
	{
		return FALSE;
	}
	return TRUE;
}

BOOLEAN DoesVehicleGroupHaveAnyPassengers( GROUP *pGroup )
{
	const INT32 iVehicleID = GetVehicleIDFromMvtGroup(pGroup);
	if( iVehicleID == -1 )
	{
		#ifdef JA2BETAVERSION
			AssertMsg( iVehicleID != -1, "DoesVehicleGroupHaveAnyPassengers() for vehicle group.  Invalid iVehicleID." );
		#endif
		return FALSE;
	}

	return DoesVehicleHaveAnyPassengers( iVehicleID );
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
