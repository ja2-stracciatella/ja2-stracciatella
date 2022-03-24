#ifndef _VEHICLES_H
#define _VEHICLES_H

#include "JA2Types.h"
#include "Strategic_Movement.h"

#include <vector>


#define MAX_VEHICLES 10
#define MAX_PASSENGERS_IN_VEHICLE 10

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


// struct for vehicles
struct VEHICLETYPE
{
	PathSt  *pMercPath; // vehicle's stategic path list
	UINT8   ubMovementGroup; // the movement group this vehicle belongs to
	UINT8   ubVehicleType; // type of vehicle
	SGPSector sSector; // position on the Stategic Map
	BOOLEAN fBetweenSectors; // between sectors?
	INT16   sGridNo; // location in tactical
	SOLDIERTYPE *pPassengers[MAX_PASSENGERS_IN_VEHICLE];
	BOOLEAN fDestroyed;
	UINT32  uiMovementSoundID;
	BOOLEAN fValid;
};

#define CFOR_EACH_PASSENGER(v, iter)                                                   \
	for (SOLDIERTYPE* const* iter = (v).pPassengers,                        \
		* const* const end__##iter = (v).pPassengers + GetVehicleSeats((v)); \
		iter != end__##iter;                                                            \
		++iter)                                                                         \
		if (!*iter) continue; else


// the list of vehicles
extern std::vector<VEHICLETYPE> pVehicleList;

#define VEHICLE2ID(v) static_cast<INT32>(&(v) - pVehicleList.data())

#define BASE_FOR_EACH_VEHICLE(type, iter) \
	for (type& iter : pVehicleList) \
		if (!iter.fValid) continue; else
#define FOR_EACH_VEHICLE( iter) BASE_FOR_EACH_VEHICLE(      VEHICLETYPE, iter)
#define CFOR_EACH_VEHICLE(iter) BASE_FOR_EACH_VEHICLE(const VEHICLETYPE, iter)


void SetVehicleValuesIntoSoldierType( SOLDIERTYPE *pVehicle );

// add vehicle to list and return id value
INT32 AddVehicleToList( INT16 sMapX, INT16 sMapY, INT16 sGridNo, UINT8 ubType );

// remove this vehicle from the list
void RemoveVehicleFromList(VEHICLETYPE&);

// clear out the vehicle list
void ClearOutVehicleList( void );

bool AnyAccessibleVehiclesInSoldiersSector(SOLDIERTYPE const&);

// is this vehicle in the same sector (not between sectors), and accesible
bool IsThisVehicleAccessibleToSoldier(SOLDIERTYPE const&, VEHICLETYPE const&);

// strategic mvt stuff
// move character path to the vehicle
BOOLEAN MoveCharactersPathToVehicle( SOLDIERTYPE *pSoldier );

// Return the vehicle, iff the vehicle ID is valid, NULL otherwise
VEHICLETYPE& GetVehicle(INT32 vehicle_id);

// Given this grunt, find out if asscoiated vehicle has a mvt group, if so,
// set this grunts mvt group to the vehicle.  For pathing purposes, will be
// reset to zero in copying of path
void SetUpMvtGroupForVehicle(SOLDIERTYPE*);

// find vehicle id of group with this vehicle
VEHICLETYPE& GetVehicleFromMvtGroup(GROUP const&);

// kill everyone in vehicle
BOOLEAN KillAllInVehicle(VEHICLETYPE const&);

// grab number of occupants in vehicles
INT32 GetNumberInVehicle(VEHICLETYPE const&);

// grab # in vehicle skipping EPCs (who aren't allowed to drive :-)
INT32 GetNumberOfNonEPCsInVehicle( INT32 iId );

BOOLEAN ExitVehicle( SOLDIERTYPE *pSoldier );


void VehicleTakeDamage(UINT8 ubID, UINT8 ubReason, INT16 sDamage, INT16 sGridNo, SOLDIERTYPE* att);

// the soldiertype containing this tactical incarnation of this vehicle
SOLDIERTYPE& GetSoldierStructureForVehicle(VEHICLETYPE const&);

// does it need fixing?
bool DoesVehicleNeedAnyRepairs(VEHICLETYPE const&);

// repair the vehicle
INT8 RepairVehicle(VEHICLETYPE const&, INT8 bTotalPts, BOOLEAN* pfNothingToRepair);


//Save all the vehicle information to the saved game file
void SaveVehicleInformationToSaveGameFile(HWFILE);

//Load all the vehicle information From the saved game file
void LoadVehicleInformationFromSavedGameFile(HWFILE, UINT32 uiSavedGameVersion);

// take soldier out of vehicle
BOOLEAN TakeSoldierOutOfVehicle( SOLDIERTYPE *pSoldier );

bool PutSoldierInVehicle(SOLDIERTYPE&, VEHICLETYPE&);

void SetVehicleSectorValues(VEHICLETYPE&, const SGPSector& sMap);

void UpdateAllVehiclePassengersGridNo( SOLDIERTYPE *pSoldier );

void LoadVehicleMovementInfoFromSavedGameFile(HWFILE);
void NewSaveVehicleMovementInfoToSavedGameFile(HWFILE);
void NewLoadVehicleMovementInfoFromSavedGameFile(HWFILE);

BOOLEAN OKUseVehicle( UINT8 ubProfile );

BOOLEAN IsRobotControllerInVehicle( INT32 iId );

void AddVehicleFuelToSave(void);

bool SoldierMustDriveVehicle(SOLDIERTYPE const&, bool trying_to_travel);

bool IsEnoughSpaceInVehicle(VEHICLETYPE const&);

BOOLEAN IsSoldierInThisVehicleSquad(const SOLDIERTYPE* pSoldier, INT8 bSquadNumber);

SOLDIERTYPE*  PickRandomPassengerFromVehicle( SOLDIERTYPE *pSoldier );

bool DoesVehicleGroupHaveAnyPassengers(GROUP const&);

void SetSoldierExitHelicopterInsertionData(SOLDIERTYPE*);

void HandleVehicleMovementSound(const SOLDIERTYPE*, BOOLEAN fOn);

UINT8 GetVehicleArmourType(UINT8 vehicle_id);

UINT8 GetVehicleSeats(VEHICLETYPE const&);

void InitVehicles(void);

#endif
