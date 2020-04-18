#ifndef		_KEYS_H_
#define		_KEYS_H_

#include "Handle_Items.h"
#include "Item_Types.h"
#include "Types.h"

#include <string_theory/string>

#include <vector>


struct KEY
{
	UINT16 usSectorFound; // where and
	UINT16 usDateFound; // when the key was found
};


#define KEY_USED			0x01

#define LOCK_UNOPENABLE		255
#define NO_KEY				255

#define LOCK_REGULAR			1
#define LOCK_PADLOCK			2
#define LOCK_CARD			3
#define LOCK_ELECTRONIC		4
#define LOCK_SPECIAL			5

#define MAXLOCKDESCLENGTH		40
struct LOCK
{
	CHAR8  ubEditorName[ MAXLOCKDESCLENGTH ]; // name to display in editor
	UINT16 usKeyItem; // key for this door uses which graphic (item #)?
	UINT8  ubLockType; // regular, padlock, electronic, etc
	UINT8  ubPickDifficulty; // difficulty to pick such a lock
	UINT8  ubSmashDifficulty; // difficulty to smash such a lock
	UINT8  ubFiller; // XXX HACK000B
};

// Defines below for the perceived value of the door
#define DOOR_PERCEIVED_UNKNOWN		0
#define DOOR_PERCEIVED_LOCKED		1
#define DOOR_PERCEIVED_UNLOCKED	2
#define DOOR_PERCEIVED_BROKEN		3

#define DOOR_PERCEIVED_TRAPPED		1
#define DOOR_PERCEIVED_UNTRAPPED	2

struct DOOR
{
	INT16   sGridNo;
	BOOLEAN fLocked; // is the door locked
	UINT8   ubTrapLevel; // difficulty of finding the trap, 0-10
	UINT8   ubTrapID; // the trap type (0 is no trap)
	UINT8   ubLockID; // the lock (0 is no lock)
	INT8    bPerceivedLocked;	// The perceived lock value can be different than the fLocked.
					// Values for this include the fact that we don't know the status of
					// the door, etc
	INT8    bPerceivedTrapped; // See above, but with respect to traps rather than locked status
	INT8    bLockDamage; // Damage to the lock
	INT8    bPadding[4]; // extra bytes // XXX HACK000B
};


enum DoorTrapTypes
{
	NO_TRAP = 0,
	EXPLOSION,
	ELECTRIC,
	SIREN,
	SILENT_ALARM,
	BROTHEL_SIREN,
	SUPER_ELECTRIC,
	NUM_DOOR_TRAPS
};

#define DOOR_TRAP_STOPS_ACTION		0x01
#define DOOR_TRAP_RECURRING		0x02
#define DOOR_TRAP_SILENT		0x04


struct DOORTRAP
{
	UINT8 fFlags; // stops action?  recurring trap?
};


//The status of the door, either open or closed
#define DOOR_OPEN			0x01
#define DOOR_PERCEIVED_OPEN		0x02
#define DOOR_PERCEIVED_NOTSET		0x04
#define DOOR_BUSY			0x08
#define DOOR_HAS_TIN_CAN		0x10


#define DONTSETDOORSTATUS		2

struct DOOR_STATUS
{
	INT16 sGridNo;
	UINT8 ubFlags;

};


// This is the number of different types of doors we can have
// in one map at a time...

#define NUM_KEYS			64
#define NUM_LOCKS			64
#define INVALID_KEY_NUMBER		255

#define ANYKEY				252
#define AUTOUNLOCK			253
#define OPENING_NOT_POSSIBLE		254

extern KEY KeyTable[NUM_KEYS];
extern LOCK LockTable[NUM_LOCKS];
extern DOORTRAP const DoorTrapTable[NUM_DOOR_TRAPS];

extern BOOLEAN AddKeysToKeyRing( SOLDIERTYPE *pSoldier, UINT8 ubKeyID, UINT8 ubNumber );
extern BOOLEAN RemoveKeyFromKeyRing( SOLDIERTYPE *pSoldier, UINT8 ubPos, OBJECTTYPE * pObj );
extern BOOLEAN RemoveAllOfKeyFromKeyRing( SOLDIERTYPE *pSoldier, UINT8 ubPos, OBJECTTYPE * pObj );
bool KeyExistsInKeyRing(SOLDIERTYPE const&, UINT8 key_id);
bool SoldierHasKey(SOLDIERTYPE const&, UINT8 key_id);

//**********************************
//* Door utils add by Kris Morness *
//**********************************

//Dynamic array of Doors.  For general game purposes, the doors that are locked and/or trapped
//are permanently saved within the map, and are loaded and allocated when the map is loaded.  Because
//the editor allows more doors to be added, or removed, the actual size of the DoorTable may change.
extern std::vector<DOOR> DoorTable;

//File I/O for loading the door information from the map.  This automatically allocates
//the exact number of slots when loading.

#define FOR_EACH_DOOR(iter) \
	for (DOOR& iter : DoorTable)

void LoadDoorTableFromMap(HWFILE);

//Saves the existing door information to the map.  Before it actually saves, it'll verify that the
//door still exists.  Otherwise, it'll ignore it.  It is possible in the editor to delete doors in
//many different ways, so I opted to put it in the saving routine.
extern void SaveDoorTableToMap( HWFILE fp );

//The editor adds locks to the world.  If the gridno already exists, then the currently existing door
//information is overwritten.
extern void AddDoorInfoToTable( DOOR *pDoor );
//When the editor removes a door from the world, this function looks for and removes accompanying door
//information.  If the entry is not the last entry, the last entry is move to it's current slot, to keep
//everything contiguous.
extern void RemoveDoorInfoFromTable( INT32 iMapIndex );
//This is the link to see if a door exists at a gridno.
DOOR * FindDoorInfoAtGridNo( INT32 iMapIndex );
//Upon world deallocation, the door table needs to be deallocated.
void TrashDoorTable(void);

ST::string GetTrapName(DOOR const&);

BOOLEAN AttemptToUnlockDoor(const SOLDIERTYPE* pSoldier, DOOR* pDoor);
BOOLEAN AttemptToLockDoor(const SOLDIERTYPE* pSoldier, DOOR* pDoor);
BOOLEAN AttemptToSmashDoor( SOLDIERTYPE * pSoldier, DOOR * pDoor );
BOOLEAN AttemptToPickLock( SOLDIERTYPE * pSoldier, DOOR * pDoor );
BOOLEAN AttemptToBlowUpLock( SOLDIERTYPE * pSoldier, DOOR * pDoor );
BOOLEAN AttemptToUntrapDoor( SOLDIERTYPE * pSoldier, DOOR * pDoor );
BOOLEAN ExamineDoorForTraps( SOLDIERTYPE * pSoldier, DOOR * pDoor );
BOOLEAN HasDoorTrapGoneOff( SOLDIERTYPE * pSoldier, DOOR * pDoor );
void HandleDoorTrap(SOLDIERTYPE&, DOOR const&);


// Updates the perceived value to the user of the state of the door
void UpdateDoorPerceivedValue( DOOR *pDoor );


//Saves the Door Table array to the temp file
void SaveDoorTableToDoorTableTempFile(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);

//Load the door table from the temp file
void LoadDoorTableFromDoorTableTempFile(void);


// Add a door to the door status array. As the user comes across the door, they
// are added. If the door already exists, nothing happens.
// is_open is True if the door is to be initially open, false if it is closed
// perceived_open is true if the door is to be initially open, else false
bool ModifyDoorStatus(GridNo, BOOLEAN is_open, BOOLEAN perceived_open);

//Deletes the door status array
void TrashDoorStatusArray(void);

// Saves the Door Status array to the MapTempfile
void SaveDoorStatusArrayToDoorStatusTempFile(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);

//Load the door status from the door status temp file
void LoadDoorStatusArrayFromDoorStatusTempFile();


//Save the key table to the saved game file
void SaveKeyTableToSaveGameFile(HWFILE);

//Load the key table from the saved game file
void LoadKeyTableFromSaveedGameFile(HWFILE);

// Returns a doors status value, NULL if not found
DOOR_STATUS *GetDoorStatus( INT16 sGridNo );

bool AllMercsLookForDoor(GridNo);

void MercLooksForDoors(SOLDIERTYPE const&);

void UpdateDoorGraphicsFromStatus();

BOOLEAN AttemptToCrowbarLock( SOLDIERTYPE * pSoldier, DOOR * pDoor );

void LoadLockTable(void);

void ExamineDoorsOnEnteringSector();

void AttachStringToDoor( INT16 sGridNo );

void DropKeysInKeyRing(SOLDIERTYPE&, GridNo, INT8 level, Visibility, bool add_to_drop_list, INT32 drop_list_slot, bool use_unloaded);

#endif
