#ifndef _VEHICLES_H
#define _VEHICLES_H

#include "Types.h"

#include "Soldier_Control.h"
#include "Strategic_Movement.h"

#define	MAX_VEHICLES	10

// type of vehicles
enum{
	ELDORADO_CAR = 0,
	HUMMER,
	ICE_CREAM_TRUCK,
	JEEP_CAR,
	TANK_CAR,
	HELICOPTER,
	NUMBER_OF_TYPES_OF_VEHICLES,
};


// external armor hit locations
enum{
	FRONT_EXTERNAL_HIT_LOCATION,
	LEFT_EXTERNAL_HIT_LOCATION,
	RIGHT_EXTERNAL_HIT_LOCATION,
	REAR_EXTERNAL_HIT_LOCATION,
	BOTTOM_EXTERNAL_HIT_LOCATION,
	TOP_EXTERNAL_HIT_LOCATION,
	NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE,
};

// internal critical hit locations
enum{
	ENGINE_HIT_LOCATION,
	CREW_COMPARTMENT_HIT_LOCATION,
	RF_TIRE_HIT_LOCATION,
	LF_TIRE_HIT_LOCATION,
	RR_TIRE_HIT_LOCATION,
	LR_TIRE_HIT_LOCATION,
	GAS_TANK_HIT_LOCATION,
	NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE,
};

//extern INT8 bInternalCritHitsByLocation[NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE][ NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ];

extern INT16 sVehicleOrigArmorValues[ NUMBER_OF_TYPES_OF_VEHICLES][ NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE];

// struct for vehicles
typedef struct
{
	PathSt* pMercPath; // vehicle's stategic path list
 UINT8     ubMovementGroup; // the movement group this vehicle belongs to
 UINT8     ubVehicleType; // type of vehicle
 INT16     sSectorX;   // X position on the Stategic Map
 INT16     sSectorY;   // Y position on the Stategic Map
 INT16     sSectorZ;
 BOOLEAN fBetweenSectors;  // between sectors?
 INT16		 sGridNo;   // location in tactical
 SOLDIERTYPE *pPassengers[ 10 ];
 UINT8		 ubDriver;
 INT16		 sInternalHitLocations[ NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE ];
 INT16		 sArmourType;
 INT16		 sExternalArmorLocationsStatus[ NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE ];
 INT16		 sCriticalHits[ NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ];
 INT32		 iOnSound;
 INT32		 iOffSound;
 INT32		 iMoveSound;
 INT32		 iOutOfSound;
 BOOLEAN   fFunctional;
 BOOLEAN	 fDestroyed;
 INT32		 iMovementSoundID;
 UINT8		 ubProfileID;

 BOOLEAN fValid;
} VEHICLETYPE;


// the list of vehicles
extern VEHICLETYPE *pVehicleList;

// number of vehicles on the list
extern UINT8 ubNumberOfVehicles;

#define VEHICLE2ID(v) ((v) - pVehicleList)

#define BASE_FOR_ALL_VEHICLES(type, iter)                           \
	for (type*       iter        = pVehicleList,                      \
	         * const end__##iter = pVehicleList + ubNumberOfVehicles; \
	     iter != end__##iter;                                         \
	     ++iter)                                                      \
		if (!iter->fValid) continue; else
#define FOR_ALL_VEHICLES( iter) BASE_FOR_ALL_VEHICLES(      VEHICLETYPE, iter)
#define CFOR_ALL_VEHICLES(iter) BASE_FOR_ALL_VEHICLES(const VEHICLETYPE, iter)


extern INT32 iMvtTypes[];

void SetVehicleValuesIntoSoldierType( SOLDIERTYPE *pVehicle );

// add vehicle to list and return id value
INT32 AddVehicleToList( INT16 sMapX, INT16 sMapY, INT16 sGridNo, UINT8 ubType );

// remove this vehicle from the list
BOOLEAN RemoveVehicleFromList( INT32 iId );

// clear out the vehicle list
void ClearOutVehicleList( void );

BOOLEAN AnyAccessibleVehiclesInSoldiersSector( SOLDIERTYPE *pSoldier );

// is this vehicle in the same sector (not between sectors), and accesible
BOOLEAN IsThisVehicleAccessibleToSoldier( SOLDIERTYPE *pSoldier, INT32 iId );

// strategic mvt stuff
// move character path to the vehicle
BOOLEAN MoveCharactersPathToVehicle( SOLDIERTYPE *pSoldier );

// is this vehicle a valid one?
BOOLEAN VehicleIdIsValid( INT32 iId );

// set up vehicle mvt for this grunt involved
BOOLEAN SetUpMvtGroupForVehicle( SOLDIERTYPE *pSoldier );

// update mercs position when vehicle arrives
void UpdatePositionOfMercsInVehicle(const VEHICLETYPE*);

// find vehicle id of group with this vehicle
INT32 GetVehicleIDFromMvtGroup(const GROUP*);

// kill everyone in vehicle
BOOLEAN KillAllInVehicle(const VEHICLETYPE*);

// grab number of occupants in vehicles
INT32 GetNumberInVehicle( INT32 iId );

// grab # in vehicle skipping EPCs (who aren't allowed to drive :-)
INT32 GetNumberOfNonEPCsInVehicle( INT32 iId );

BOOLEAN EnterVehicle(const SOLDIERTYPE* pVehicle, SOLDIERTYPE* pSoldier);

void SetVehicleName( SOLDIERTYPE *pVehicle );

BOOLEAN ExitVehicle( SOLDIERTYPE *pSoldier );

void AddPassangersToTeamPanel( INT32 iId );


void VehicleTakeDamage(UINT8 ubID, UINT8 ubReason, INT16 sDamage, INT16 sGridNo, SOLDIERTYPE* att);

// the soldiertype containing this tactical incarnation of this vehicle
SOLDIERTYPE * GetSoldierStructureForVehicle( INT32 iId );

void AdjustVehicleAPs( SOLDIERTYPE *pSoldier, UINT8 *pubPoints );

// get orig armor values for vehicle in this location
//INT16 GetOrigInternalArmorValueForVehicleInLocation( UINT8 ubID, UINT8 ubLocation );

// does it need fixing?
BOOLEAN DoesVehicleNeedAnyRepairs( INT32 iVehicleId );

// repair the vehicle
INT8 RepairVehicle( INT32 iVehicleId, INT8 bTotalPts, BOOLEAN *pfNothingToRepair );


//Save all the vehicle information to the saved game file
BOOLEAN SaveVehicleInformationToSaveGameFile( HWFILE hFile );

//Load all the vehicle information From the saved game file
BOOLEAN LoadVehicleInformationFromSavedGameFile( HWFILE hFile, UINT32 uiSavedGameVersion );

// take soldier out of vehicle
BOOLEAN TakeSoldierOutOfVehicle( SOLDIERTYPE *pSoldier );

// put soldier in vehicle
BOOLEAN PutSoldierInVehicle( SOLDIERTYPE *pSoldier, INT8 bVehicleId );

void SetVehicleSectorValues(VEHICLETYPE*, UINT8 ubSectorX, UINT8 ubSectorY);

void UpdateAllVehiclePassengersGridNo( SOLDIERTYPE *pSoldier );

BOOLEAN LoadVehicleMovementInfoFromSavedGameFile( HWFILE hFile );
BOOLEAN NewSaveVehicleMovementInfoToSavedGameFile( HWFILE hFile );
BOOLEAN NewLoadVehicleMovementInfoFromSavedGameFile( HWFILE hFile );

BOOLEAN OKUseVehicle( UINT8 ubProfile );

BOOLEAN IsRobotControllerInVehicle( INT32 iId );

void AddVehicleFuelToSave(void);

BOOLEAN SoldierMustDriveVehicle(const SOLDIERTYPE* s, INT32 iVehicleId, BOOLEAN fTryingToTravel);

BOOLEAN IsEnoughSpaceInVehicle( INT32 iID );

BOOLEAN IsSoldierInThisVehicleSquad(const SOLDIERTYPE* pSoldier, INT8 bSquadNumber);

SOLDIERTYPE*  PickRandomPassengerFromVehicle( SOLDIERTYPE *pSoldier );

BOOLEAN DoesVehicleGroupHaveAnyPassengers( GROUP *pGroup );

void SetSoldierExitVehicleInsertionData( SOLDIERTYPE *pSoldier, INT32 iId );


#endif
