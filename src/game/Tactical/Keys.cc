#include "Directories.h"
#include "Font_Control.h"
#include "Handle_Items.h"
#include "LoadSaveData.h"
#include "Soldier_Profile.h"
#include "Types.h"
#include "Soldier_Control.h"
#include "Keys.h"
#include "Debug.h"
#include "SkillCheck.h"
#include "OppList.h"
#include "Items.h"
#include "Weapons.h"
#include "AI.h"
#include "Message.h"
#include "Text.h"
#include "Explosion_Control.h"
#include "Isometric_Utils.h"
#include "StrategicMap.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"
#include "LOS.h"
#include "TileDat.h"
#include "Overhead.h"
#include "Structure.h"
#include "RenderWorld.h"
#include "WorldMan.h"
#include "Random.h"
#include "WorldDef.h"
#include "Campaign.h"
#include "Sound_Control.h"
#include "Interface.h"
#include "MapScreen.h"
#include "Game_Clock.h"
#include "Handle_Doors.h"
#include "Map_Screen_Interface.h"
#include "MemMan.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <string_theory/string>

#include <vector>

static std::vector<DOOR_STATUS> gpDoorStatus;


#define FOR_EACH_DOOR_STATUS(iter) \
	for (DOOR_STATUS& iter : gpDoorStatus)


KEY KeyTable[NUM_KEYS];

LOCK LockTable[NUM_LOCKS];

/*
LOCK LockTable[NUM_LOCKS] =
{
	// Keys that will open the lock,	Lock type,		Pick diff,	Smash diff
	{{NO_KEY, NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		0,		0},
	{{0,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-25,		-25},
	{{1,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-60,		-55},
	{{2,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-75,		-80},
	{{3,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-35,		-45},
	{{4,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-45,		-60},
	{{5,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-65,		-90},
	{{6,	NO_KEY, NO_KEY, NO_KEY},	LOCK_PADLOCK,		-60,		-70},
	{{7,	NO_KEY, NO_KEY, NO_KEY},	LOCK_ELECTRONIC,	-50,		-60},
	{{8,	NO_KEY, NO_KEY, NO_KEY},	LOCK_ELECTRONIC,	-75,		-80},
	{{9,	NO_KEY, NO_KEY, NO_KEY},	LOCK_CARD,		-50,		-40},
	{{10,	NO_KEY, NO_KEY, NO_KEY},	LOCK_CARD,		-85,		-80},
	{{11,	NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,		-50,		-50}
};
*/

DOORTRAP const DoorTrapTable[NUM_DOOR_TRAPS] =
{
	{0}, // nothing
	{DOOR_TRAP_STOPS_ACTION}, // explosion
	{DOOR_TRAP_STOPS_ACTION | DOOR_TRAP_RECURRING}, // electric
	{DOOR_TRAP_RECURRING}, // siren
	{DOOR_TRAP_RECURRING | DOOR_TRAP_SILENT}, // silent alarm
	{DOOR_TRAP_RECURRING}, // brothel siren
	{DOOR_TRAP_STOPS_ACTION | DOOR_TRAP_RECURRING}, // super electric
};



//Dynamic array of Doors.  For general game purposes, the doors that are locked and/or trapped
//are permanently saved within the map, and are loaded and allocated when the map is loaded.  Because
//the editor allows more doors to be added, or removed, the actual size of the DoorTable may change.
std::vector<DOOR> DoorTable;


void LoadLockTable(void)
try
{
	UINT32 uiBytesToRead;
	const char *pFileName = BINARYDATADIR "/Locks.bin";

	// Load the Lock Table

	AutoSGPFile hFile(GCM->openGameResForReading(pFileName));

	uiBytesToRead = sizeof( LOCK ) * NUM_LOCKS;
	FileRead(hFile, LockTable, uiBytesToRead);
}
catch (...)
{
	SLOGE("FAILED to LoadLockTable");
	throw;
}


static bool KeyExistsInInventory(SOLDIERTYPE const&, UINT8 key_id);


bool SoldierHasKey(SOLDIERTYPE const& s, UINT8 const key_id)
{
	return KeyExistsInKeyRing(s, key_id) ||
		KeyExistsInInventory(s, key_id);
}


bool KeyExistsInKeyRing(SOLDIERTYPE const& s, UINT8 const key_id)
{
	if (!s.pKeyRing) return FALSE; // No key ring

	KEY_ON_RING const* const end = s.pKeyRing + NUM_KEYS;
	for (KEY_ON_RING const* i = s.pKeyRing; i != end; ++i)
	{
		if (i->ubNumber == 0) continue;
		if (i->ubKeyID != key_id && key_id != ANYKEY) continue;
		return true;
	}
	return false;
}


static bool KeyExistsInInventory(SOLDIERTYPE const& s, UINT8 const key_id)
{
	CFOR_EACH_SOLDIER_INV_SLOT(i, s)
	{
		if (GCM->getItem(i->usItem)->getItemClass() != IC_KEY)    continue;
		if (i->ubKeyID != key_id && key_id != ANYKEY) continue;
		return true;
	}
	return false;
}


static BOOLEAN ValidKey(DOOR* pDoor, UINT8 ubKeyID)
{
	return (pDoor->ubLockID == ubKeyID);
}


static void DoLockDoor(DOOR* pDoor, UINT8 ubKeyID)
{
	// if the door is unlocked and this is the right key, lock the door
	if (!(pDoor->fLocked) && ValidKey( pDoor, ubKeyID ))
	{
		pDoor->fLocked = TRUE;
	}
}


static void DoUnlockDoor(DOOR* pDoor, UINT8 ubKeyID)
{
	// if the door is locked and this is the right key, unlock the door
	if ( (pDoor->fLocked) && ValidKey( pDoor, ubKeyID ))
	{
		// Play lockpicking
		PlayLocationJA2Sample(pDoor->sGridNo, UNLOCK_DOOR_1, MIDVOLUME, 1);

		pDoor->fLocked = FALSE;
	}
}


BOOLEAN AttemptToUnlockDoor(const SOLDIERTYPE* pSoldier, DOOR* pDoor)
{
	const UINT8 ubKeyID = pDoor->ubLockID;
	if (SoldierHasKey(*pSoldier, ubKeyID))
	{
		DoUnlockDoor(pDoor, ubKeyID);
		return TRUE;
	}

	// drat, couldn't find the key
	PlayJA2Sample(KEY_FAILURE, MIDVOLUME, 1, MIDDLEPAN);

	return FALSE;
}


BOOLEAN AttemptToLockDoor(const SOLDIERTYPE* pSoldier, DOOR* pDoor)
{
	const UINT8 ubKeyID = pDoor->ubLockID;
	if (SoldierHasKey(*pSoldier, ubKeyID))
	{
		DoLockDoor(pDoor, ubKeyID);
		return TRUE;
	}

	// drat, couldn't find the key
	return FALSE;
}


BOOLEAN AttemptToCrowbarLock( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32 iResult;
	INT8  bStress, bSlot;

	bSlot = FindUsableObj( pSoldier, CROWBAR );
	if ( bSlot == ITEM_NOT_FOUND )
	{
		// error!
		return( FALSE );
	}

	// generate a noise for thumping on the door
	MakeNoise(pSoldier, pSoldier->sGridNo, pSoldier->bLevel, CROWBAR_DOOR_VOLUME, NOISE_DOOR_SMASHING);

	if ( !pDoor->fLocked )
	{
		// auto success but no XP

		// succeeded! door can never be locked again, so remove from door list...
		RemoveDoorInfoFromTable( pDoor->sGridNo );
		// award experience points?

		// Play lock busted sound
		PlayLocationJA2Sample(pSoldier->sGridNo, BREAK_LOCK, MIDVOLUME, 1);

		return( TRUE );
	}

	if ( pDoor->ubLockID == LOCK_UNOPENABLE )
	{
		// auto failure!
		return( FALSE );
	}

	// possibly damage crowbar
	bStress = __min( EffectiveStrength( pSoldier ), LockTable[pDoor->ubLockID].ubSmashDifficulty + 30 );
	// reduce crowbar status by random % between 0 and 5%
	DamageObj( &(pSoldier->inv[ bSlot ]), (INT8) PreRandom( bStress / 20 ) );

	// did we succeed?

	if ( LockTable[pDoor->ubLockID].ubSmashDifficulty == OPENING_NOT_POSSIBLE )
	{
		// do this to get 'can't do this' messages
		iResult = SkillCheck( pSoldier, OPEN_WITH_CROWBAR, (INT8) ( -100 ) );
		iResult = -100;
	}
	else
	{
		iResult = SkillCheck( pSoldier, OPEN_WITH_CROWBAR, (INT8) ( - (INT8) (LockTable[pDoor->ubLockID].ubSmashDifficulty - pDoor->bLockDamage) ) );
	}

	if (iResult > 0)
	{
		// STR GAIN (20) - Pried open a lock
		StatChange(*pSoldier, STRAMT, 20, FROM_SUCCESS);

		// succeeded! door can never be locked again, so remove from door list...
		RemoveDoorInfoFromTable( pDoor->sGridNo );

		// Play lock busted sound
		PlayLocationJA2Sample(pSoldier->sGridNo, BREAK_LOCK, MIDVOLUME, 1);

		return( TRUE );
	}
	else
	{
		if (iResult > -10)
		{
			// STR GAIN - Damaged a lock by prying
			StatChange(*pSoldier, STRAMT, 5, FROM_SUCCESS);

			// we came close... so do some damage to the lock
			pDoor->bLockDamage += (INT8) (10 + iResult);
		}
		else if ( iResult > -40 && pSoldier->sGridNo != pSoldier->sSkillCheckGridNo )
		{
			// give token point for effort :-)
			StatChange(*pSoldier, STRAMT, 1, FROM_SUCCESS);
		}

		return( FALSE );
	}

}


BOOLEAN AttemptToSmashDoor( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32 iResult;
	LOCK  *pLock;

	// generate a noise for thumping on the door
	MakeNoise(pSoldier, pSoldier->sGridNo, pSoldier->bLevel, SMASHING_DOOR_VOLUME, NOISE_DOOR_SMASHING);

	if ( !pDoor->fLocked )
	{
		// auto success but no XP

		// succeeded! door can never be locked again, so remove from door list...
		RemoveDoorInfoFromTable( pDoor->sGridNo );
		// award experience points?

		// Play lock busted sound
		PlayLocationJA2Sample(pSoldier->sGridNo, BREAK_LOCK, MIDVOLUME, 1);

		return( TRUE );
	}

	if ( pDoor->ubLockID == LOCK_UNOPENABLE )
	{
		// auto failure!
		return( FALSE );
	}

	pLock = &(LockTable[pDoor->ubLockID]);

	// did we succeed?
	if ( pLock->ubSmashDifficulty == OPENING_NOT_POSSIBLE )
	{
		// do this to get 'can't do this' messages
		iResult = SkillCheck( pSoldier, SMASH_DOOR_CHECK, (INT8) ( -100 ) );
		iResult = -100;
	}
	else
	{
		iResult = SkillCheck( pSoldier, SMASH_DOOR_CHECK, (INT8) ( - (INT8) (LockTable[pDoor->ubLockID].ubSmashDifficulty - pDoor->bLockDamage) ) );
	}
	if (iResult > 0)
	{
		// STR GAIN (20) - Pried open a lock
		StatChange(*pSoldier, STRAMT, 20, FROM_SUCCESS);

		// succeeded! door can never be locked again, so remove from door list...
		RemoveDoorInfoFromTable( pDoor->sGridNo );
		// award experience points?

		// Play lock busted sound
		PlayLocationJA2Sample(pSoldier->sGridNo, BREAK_LOCK, MIDVOLUME, 1);

		return( TRUE );
	}
	else
	{
		if (iResult > -10)
		{
			// STR GAIN - Damaged a lock by prying
			StatChange(*pSoldier, STRAMT, 5, FROM_SUCCESS);

			// we came close... so do some damage to the lock
			pDoor->bLockDamage += (INT8) (10 + iResult);
		}
		else if ( iResult > -40 && pSoldier->sGridNo != pSoldier->sSkillCheckGridNo )
		{
			// give token point for effort :-)
			StatChange(*pSoldier, STRAMT, 1, FROM_SUCCESS);
		}
		return( FALSE );
	}
}

BOOLEAN AttemptToPickLock( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32 iResult;
	INT8  bReason;
	LOCK  *pLock;

	if ( pDoor->ubLockID == LOCK_UNOPENABLE )
	{
		// auto failure!
		return( FALSE );
	}

	pLock = &(LockTable[pDoor->ubLockID]);

	// look up the type of lock to see if it is electronic or not
	if (pLock->ubLockType == LOCK_CARD || pLock->ubLockType == LOCK_ELECTRONIC )
	{
		bReason = ELECTRONIC_LOCKPICKING_CHECK;
	}
	else
	{
		bReason = LOCKPICKING_CHECK;
	}

	// Play lockpicking
	// ATE: Moved to animation
	//PlayLocationJA2Sample(pSoldier->sGridNo, PICKING_LOCK, MIDVOLUME, 1);

	// See if we measure up to the task.
	// The difficulty is negated here to make it a skill adjustment
	if ( pLock->ubPickDifficulty == OPENING_NOT_POSSIBLE )
	{
		// do this to get 'can't do this' messages
		iResult = SkillCheck( pSoldier, bReason, (INT8) ( -100 ) );
		iResult = -100;
	}
	else
	{
		iResult = SkillCheck( pSoldier, bReason, (INT8) ( - (INT8) (pLock->ubPickDifficulty) ) );
	}
	if (iResult > 0)
	{
		// MECHANICAL GAIN:  Picked open a lock
		StatChange(*pSoldier, MECHANAMT, pLock->ubPickDifficulty / 5, FROM_SUCCESS);

		// DEXTERITY GAIN:  Picked open a lock
		StatChange(*pSoldier, DEXTAMT, pLock->ubPickDifficulty / 10, FROM_SUCCESS);

		// succeeded!
		pDoor->fLocked = FALSE;
		return( TRUE );
	}
	else
	{
		// NOTE: failures are not rewarded, since you can keep trying indefinitely...

		// check for traps
		return( FALSE );
	}
}

BOOLEAN AttemptToUntrapDoor( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32 iResult;

	// See if we measure up to the task.
	if ( pDoor->ubTrapID == EXPLOSION  )
	{
		iResult = SkillCheck( pSoldier, DISARM_TRAP_CHECK, (INT8) (pDoor->ubTrapLevel * 7) );
	}
	else
	{
		iResult = SkillCheck( pSoldier, DISARM_ELECTRONIC_TRAP_CHECK, (INT8) (pDoor->ubTrapLevel * 7) );
	}

	if (iResult > 0)
	{
		// succeeded!
		pDoor->ubTrapLevel = 0;
		pDoor->ubTrapID = NO_TRAP;
		return( TRUE );
	}
	else
	{
		// trap should REALLY go off now!
		return( FALSE );
	}
}

BOOLEAN ExamineDoorForTraps( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	// Check to see if there is a trap or not on this door
	INT8 bDetectLevel;

	if (pDoor->ubTrapID == NO_TRAP)
	{
		// No trap!
		pDoor->bPerceivedTrapped = DOOR_PERCEIVED_UNTRAPPED;
	}
	else
	{
		if (pDoor->bPerceivedTrapped == DOOR_PERCEIVED_TRAPPED)
		{
			return( TRUE );
		}
		else
		{
			bDetectLevel = CalcTrapDetectLevel( pSoldier, TRUE );
			if (bDetectLevel < pDoor->ubTrapLevel)
			{
				pDoor->bPerceivedTrapped = DOOR_PERCEIVED_UNTRAPPED;
			}
			else
			{
				pDoor->bPerceivedTrapped = DOOR_PERCEIVED_TRAPPED;
				return( TRUE );
			}
		}
	}
	return( FALSE );
}

BOOLEAN HasDoorTrapGoneOff( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	// Check to see if the soldier causes the trap to go off
	INT8 bDetectLevel;

	if (pDoor->ubTrapID != NO_TRAP)
	{
		// one quick check to see if the guy sees the trap ahead of time!
		bDetectLevel = CalcTrapDetectLevel( pSoldier, FALSE );
		if (bDetectLevel < pDoor->ubTrapLevel)
		{
			// trap goes off!
			return( TRUE );
		}
	}
	return( FALSE );
}


ST::string GetTrapName(DOOR const& d)
{
	UINT8 trap_kind = d.ubTrapID;
	switch (trap_kind)
	{
		case BROTHEL_SIREN:  trap_kind = SIREN;    break;
		case SUPER_ELECTRIC: trap_kind = ELECTRIC; break;
	}
	return pDoorTrapStrings[trap_kind];
}


void HandleDoorTrap(SOLDIERTYPE& s, DOOR const& d)
{
	if (!(DoorTrapTable[d.ubTrapID].fFlags & DOOR_TRAP_SILENT))
	{
		ST::string trap_name = GetTrapName(d);
		ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[LOCK_TRAP_HAS_GONE_OFF_STR], trap_name));
	}

	// set trap off
	switch (d.ubTrapID)
	{
		case EXPLOSION:
			// cause damage as a regular hand grenade
			IgniteExplosion(NULL, 25, s.sGridNo, HAND_GRENADE, 0);
			break;

		case SIREN:
			// play siren sound effect but otherwise treat as silent alarm, calling
			// available enemies to this location
			PlayLocationJA2Sample(d.sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			/* FALLTHROUGH */

		case SILENT_ALARM:
			// Get all available enemies running here
			CallAvailableEnemiesTo(d.sGridNo);
			break;

		case BROTHEL_SIREN:
			PlayLocationJA2Sample(d.sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			CallAvailableKingpinMenTo(d.sGridNo);
			// no one is authorized any more!
			gMercProfiles[MADAME].bNPCData = 0;
			break;

		case ELECTRIC:
			// insert electrical sound effect here
			PlayLocationJA2Sample(d.sGridNo, DOOR_ELECTRICITY, MIDVOLUME, 1);

			s.attacker = &s;
			s.bBeingAttackedCount++;
			gTacticalStatus.ubAttackBusyCount++;
			SLOGD("Trap gone off. Busy count: %d", gTacticalStatus.ubAttackBusyCount);

			SoldierTakeDamage(&s, 10 + PreRandom(10), 3 + PreRandom(3) * 1000,
						TAKE_DAMAGE_ELECTRICITY, NULL);
			break;

		case SUPER_ELECTRIC:
			// insert electrical sound effect here
			PlayLocationJA2Sample(d.sGridNo, DOOR_ELECTRICITY, MIDVOLUME, 1);

			s.attacker = &s;
			s.bBeingAttackedCount++;
			gTacticalStatus.ubAttackBusyCount++;
			SLOGD("Trap gone off. Busy count: %d", gTacticalStatus.ubAttackBusyCount);

			SoldierTakeDamage(&s, 20 + PreRandom(20), 6 + PreRandom(6) * 1000, TAKE_DAMAGE_ELECTRICITY, NULL);
			break;

		default:
			// no trap
			break;
	}
}


BOOLEAN AttemptToBlowUpLock( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32 iResult;
	INT8  bSlot = NO_SLOT;

	bSlot = FindObj( pSoldier, SHAPED_CHARGE );
	if (bSlot == NO_SLOT)
	{
		return( FALSE );
	}

	iResult = SkillCheck( pSoldier, PLANTING_BOMB_CHECK, 0 );
	if (iResult >= -20)
	{
		// Do explosive graphic....
		{
			ANITILE_PARAMS AniParams;
			INT16 sGridNo;
			INT16 sX, sY, sZ;

			// Get gridno
			sGridNo = pDoor->sGridNo;

			// Get sX, sy;
			sX = CenterX( sGridNo );
			sY = CenterY( sGridNo );

			// Get Z position, based on orientation....
			sZ = 20;

			AniParams = ANITILE_PARAMS{};
			AniParams.sGridNo = sGridNo;
			AniParams.ubLevelID = ANI_TOPMOST_LEVEL;
			AniParams.sDelay = (INT16)( 100 );
			AniParams.sStartFrame = 0;
			AniParams.uiFlags = ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
			AniParams.sX = sX;
			AniParams.sY = sY;
			AniParams.sZ = sZ;
			AniParams.zCachedFile = TILECACHEDIR "/miniboom.sti";
			CreateAnimationTile( &AniParams );

			PlayLocationJA2Sample(sGridNo, SMALL_EXPLODE_1, HIGHVOLUME, 1);

			// Remove the explosive.....
			bSlot = FindObj( pSoldier, SHAPED_CHARGE );
			if (bSlot != NO_SLOT)
			{
				RemoveObjs( &(pSoldier->inv[ bSlot ]), 1 );
				DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
			}
		}

		// Not sure if this makes sense, but the explosive is small.
		// Double the damage here as we are damaging a lock rather than a person
		pDoor->bLockDamage += Explosive[GCM->getItem(SHAPED_CHARGE)->getClassIndex()].ubDamage * 2;
		if (pDoor->bLockDamage > LockTable[ pDoor->ubLockID ].ubSmashDifficulty )
		{
			// succeeded! door can never be locked again, so remove from door list...
			RemoveDoorInfoFromTable( pDoor->sGridNo );
			// award experience points?
			return( TRUE );
		}
	}
	else
	{
		bSlot = FindObj( pSoldier, SHAPED_CHARGE );
		if (bSlot != NO_SLOT)
		{
			RemoveObjs( &(pSoldier->inv[ bSlot ]), 1 );
			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
		}

		// OOPS! ... BOOM!
		IgniteExplosionXY(NULL, pSoldier->sX, pSoldier->sY, gpWorldLevelData[pSoldier->sGridNo].sHeight, pSoldier->sGridNo, SHAPED_CHARGE, 0);
	}
	return( FALSE );
}

//File I/O for loading the door information from the map.  This automatically allocates
//the exact number of slots when loading.
void LoadDoorTableFromMap(HWFILE const f)
{
	TrashDoorTable();

	UINT8 numDoors = 0;
	FileRead(f, &numDoors, sizeof(numDoors));
	if (numDoors == 0) return;

	DoorTable.assign(numDoors, DOOR{});
	FileRead(f, DoorTable.data(), sizeof(DOOR) * numDoors);

	// OK, reset perceived values to nothing...
	FOR_EACH_DOOR(d)
	{
		d.bPerceivedLocked  = DOOR_PERCEIVED_UNKNOWN;
		d.bPerceivedTrapped = DOOR_PERCEIVED_UNKNOWN;
	}
}


//Saves the existing door information to the map.  Before it actually saves, it'll verify that the
//door still exists.  Otherwise, it'll ignore it.  It is possible in the editor to delete doors in
//many different ways, so I opted to put it in the saving routine.
void SaveDoorTableToMap( HWFILE fp )
{
	size_t i = 0;

	while (i < DoorTable.size())
	{
		if (!OpenableAtGridNo( DoorTable[ i ].sGridNo ))
			RemoveDoorInfoFromTable( DoorTable[ i ].sGridNo );
		else
			i++;
	}
	Assert(DoorTable.size() <= UINT8_MAX);
	UINT8 numDoors = static_cast<UINT8>(DoorTable.size());
	FileWriteArray(fp, numDoors, DoorTable.data());
}


//The editor adds locks to the world.  If the gridno already exists, then the currently existing door
//information is overwritten.
void AddDoorInfoToTable( DOOR *pDoor )
{
	FOR_EACH_DOOR(d)
	{
		if (d.sGridNo != pDoor->sGridNo) continue;
		d = *pDoor;
		return;
	}

	//no existing door found, so add a new one.
	DoorTable.push_back(*pDoor);
}

//When the editor removes a door from the world, this function looks for and removes accompanying door
//information.  If the entry is not the last entry, the last entry is move to it's current slot, to keep
//everything contiguous.
void RemoveDoorInfoFromTable( INT32 iMapIndex )
{
	for (size_t i = 0; i < DoorTable.size(); i++)
	{
		if (DoorTable[ i ].sGridNo == iMapIndex)
		{
			DoorTable.erase(DoorTable.begin() + i);
			return;
		}
	}
}

//This is the link to see if a door exists at a gridno.
DOOR* FindDoorInfoAtGridNo( INT32 iMapIndex )
{
	FOR_EACH_DOOR(d)
	{
		if (d.sGridNo == iMapIndex) return &d;
	}
	return NULL;
}

//Upon world deallocation, the door table needs to be deallocated.
void TrashDoorTable()
{
	DoorTable.clear();
}

void UpdateDoorPerceivedValue( DOOR *pDoor )
{
	if ( pDoor->fLocked )
	{
		pDoor->bPerceivedLocked = DOOR_PERCEIVED_LOCKED;
	}
	else if ( !pDoor->fLocked )
	{
		pDoor->bPerceivedLocked = DOOR_PERCEIVED_UNLOCKED;
	}

	if (pDoor->ubTrapID != NO_TRAP)
	{
		pDoor->bPerceivedTrapped = DOOR_PERCEIVED_TRAPPED;
	}
	else
	{
		pDoor->bPerceivedTrapped = DOOR_PERCEIVED_UNTRAPPED;
	}

}


void SaveDoorTableToDoorTableTempFile(INT16 const x, INT16 const y, INT8 const z)
{
	AutoSGPFile f(GCM->openTempFileForWriting(GetMapTempFileName(SF_DOOR_TABLE_TEMP_FILES_EXISTS, x, y, z), true));
	Assert(DoorTable.size() <= UINT8_MAX);
	UINT8 numDoors = static_cast<UINT8>(DoorTable.size());
	FileWriteArray(f, numDoors, DoorTable.data());
	// Set the sector flag indicating that there is a Door table temp file present
	SetSectorFlag(x, y, z, SF_DOOR_TABLE_TEMP_FILES_EXISTS);
}


void LoadDoorTableFromDoorTableTempFile()
{
	ST::string const zMapName = GetMapTempFileName( SF_DOOR_TABLE_TEMP_FILES_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//If the file doesnt exists, its no problem.
	if (!GCM->doesTempFileExist(zMapName)) return;

	//Get rid of the existing door table
	TrashDoorTable();

	AutoSGPFile hFile(GCM->openTempFileForReading(zMapName));

	//Read in the number of doors
	UINT8 numDoors = 0;
	FileRead(hFile, &numDoors, sizeof(UINT8));

	//if there is no doors to load
	if (numDoors != 0)
	{
		DoorTable.assign(numDoors, DOOR{});
		FileRead(hFile, DoorTable.data(), sizeof(DOOR) * numDoors);
	}
}


// is_open is True if the door is open, false if it is closed
bool ModifyDoorStatus(GridNo const gridno, BOOLEAN const is_open, BOOLEAN const perceived_open)
{
	// Find the base tile for the door structure and use that gridno
	STRUCTURE* const structure = FindStructure(gridno, STRUCTURE_ANYDOOR);
	if (!structure) return false;

	STRUCTURE* const base = FindBaseStructure(structure);
	if (!base) return false;
	GridNo const base_gridno = base->sGridNo;

	// Check to see if the user is adding an existing door
	FOR_EACH_DOOR_STATUS(d)
	{
		if (d.sGridNo != base_gridno) continue;

		// Set the status
		if (perceived_open != DONTSETDOORSTATUS)
		{
			if (perceived_open)
				d.ubFlags |= DOOR_PERCEIVED_OPEN;
			else
				d.ubFlags &= ~DOOR_PERCEIVED_OPEN;

			// Turn off perceived not set flag
			d.ubFlags &= ~DOOR_PERCEIVED_NOTSET;
		}

		if (is_open != DONTSETDOORSTATUS)
		{
			if (is_open)
				d.ubFlags |= DOOR_OPEN;
			else
				d.ubFlags &= ~DOOR_OPEN;
		}

		return true;
	}

	// Add a new door status structure
	gpDoorStatus.push_back(DOOR_STATUS{});

	DOOR_STATUS& d = gpDoorStatus.back();
	d.sGridNo = base_gridno;

	// Init the flags
	d.ubFlags = 0;

	if (is_open) d.ubFlags |= DOOR_OPEN;

	// if a new door, use same as actual
	if (perceived_open == DONTSETDOORSTATUS)
	{
		d.ubFlags |= DOOR_PERCEIVED_NOTSET;
	}
	else if (is_open)
	{
		d.ubFlags |= DOOR_PERCEIVED_OPEN;
	}

	// Flag the tile as containing a door status
	gpWorldLevelData[base_gridno].ubExtFlags[0] |= MAPELEMENT_EXT_DOOR_STATUS_PRESENT;
	return true;
}


void TrashDoorStatusArray( )
{
	gpDoorStatus.clear();
}


// Returns a doors status value, NULL if not found
DOOR_STATUS* GetDoorStatus(INT16 const sGridNo)
{
	if (gpDoorStatus.empty()) return 0;

	// Find the base tile for the door structure and use that gridno
	STRUCTURE* const structure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
	if (!structure) return 0;
	STRUCTURE const* const base = FindBaseStructure(structure);
	if (!base) return 0;

	FOR_EACH_DOOR_STATUS(d)
	{
		if (d.sGridNo == base->sGridNo) return &d;
	}

	return 0;
}


static bool IsCloseEnoughAndHasLOS(SOLDIERTYPE const& s, GridNo const gridno, INT16 const dist_visible)
{
	return
		// Is he close enough to see that gridno if he turns his head?
		PythSpacesAway(s.sGridNo, gridno) <= dist_visible &&
		// Can we trace a line of sight to his x,y coordinates? (taking into account
		// we are definitely aware of this guy now)
		SoldierTo3DLocationLineOfSightTest(&s, gridno, 0, 0, dist_visible, TRUE);
}


static INT8 const g_dirs[] = { NORTH, SOUTH, EAST, WEST, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST };


bool AllMercsLookForDoor(GridNo const gridno)
{
	if (!GetDoorStatus(gridno)) return false;

	CFOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE const& s = *i;
		if (s.bLife < OKLIFE)     continue;
		if (s.sGridNo == NOWHERE) continue;
		if (!s.bInSector)         continue;

		INT16 const dist_visible = DistanceVisible(&s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, gridno, 0);
		if (IsCloseEnoughAndHasLOS(s, gridno, dist_visible)) return true;

		// Now try other adjacent gridnos
		for (INT32 dir = 0; dir != 8; ++dir)
		{
			GridNo const new_gridno   = NewGridNo(gridno, DirectionInc(g_dirs[dir]));
			INT16  const dist_visible = DistanceVisible(&s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, new_gridno, 0);
			if (IsCloseEnoughAndHasLOS(s, new_gridno, dist_visible)) return true;
		}
	}

	return false;
}


static bool InternalIsPerceivedDifferentThanReality(DOOR_STATUS const&);
static void InternalUpdateDoorGraphicFromStatus(DOOR_STATUS const&, bool dirty);
static void InternalUpdateDoorsPerceivedValue(DOOR_STATUS&);


void MercLooksForDoors(SOLDIERTYPE const& s)
{
	FOR_EACH_DOOR_STATUS(d)
	{
		if (!InternalIsPerceivedDifferentThanReality(d)) continue;

		GridNo const gridno       = d.sGridNo;
		INT16  const dist_visible = DistanceVisible(&s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, gridno, 0);

		if (IsCloseEnoughAndHasLOS(s, gridno, dist_visible))
		{
sees_door:
			InternalUpdateDoorsPerceivedValue(d);
			InternalUpdateDoorGraphicFromStatus(d, true);
			break;
		}

		// Now try other adjacent gridnos
		for (INT32 dir = 0; dir != 8; ++dir)
		{
			GridNo const new_gridno = NewGridNo(gridno, DirectionInc(g_dirs[dir]));
			// XXX It seems strage that AllMercsLookForDoor() calculates a new dist_visible, but MercLooksForDoors() does not
			if (IsCloseEnoughAndHasLOS(s, new_gridno, dist_visible)) goto sees_door;
		}
	}
}


static void SynchronizeDoorStatusToStructureData(DOOR_STATUS const& d)
{
	// First look for a door structure here
	STRUCTURE* const s = FindStructure(d.sGridNo, STRUCTURE_ANYDOOR);
	if (!s) return;

	// ATE: One of the purposes of this function is to MAKE sure the door status
	// MATCHES the struct data value - if not - change (REGARDLESS of perceived
	// being used or not)
	bool const door_open   = d.ubFlags & DOOR_OPEN;
	bool const struct_open = s->fFlags & STRUCTURE_OPEN;
	if (door_open == struct_open) return;

	// Swap!
	STRUCTURE *base = FindBaseStructure(s);
	if (!base)
	{
		STLOGW("Door structure data at {} was not found", d.sGridNo);
		return;
	}
	INT16 sBaseGridNo = base->sGridNo;
	SwapStructureForPartner(base);
	RecompileLocalMovementCosts(sBaseGridNo);
}


void UpdateDoorGraphicsFromStatus()
{
	FOR_EACH_DOOR_STATUS(d)
	{
		// ATE: Make sure door status flag and struct info are synchronized
		SynchronizeDoorStatusToStructureData(d);
		InternalUpdateDoorGraphicFromStatus(d, false);
	}
}


static void InternalUpdateDoorGraphicFromStatus(DOOR_STATUS const& d, bool const dirty)
{
	// OK, look at perceived status and adjust graphic
	// First look for a door structure here...
	STRUCTURE* const s = FindStructure(d.sGridNo, STRUCTURE_ANYDOOR);
	if (!s) return;

	STRUCTURE* const base = FindBaseStructure(s);
	LEVELNODE* const node = FindLevelNodeBasedOnStructure(base);

	// Get status we want to change to
	bool const want_to_be_open = d.ubFlags & DOOR_PERCEIVED_OPEN;

	// First look for an opened door
	// get what it is now
	bool openend_graphic = false;
	INT32 cnt;
	for (cnt = 0; gClosedDoorList[cnt] != -1; ++cnt)
	{
		// IF WE ARE A SHADOW TYPE
		if (node->usIndex == gClosedDoorList[cnt])
		{
			openend_graphic = true;
			break;
		}
	}

	// We either have an opened graphic, in which case we want to switch to the
	// closed, or a closed in which case we want to switch to opened
	// adjust o' graphic

	// We now need to test these things against the true structure data we may
	// need to only adjust the graphic here
	if (want_to_be_open && s->fFlags & STRUCTURE_OPEN)
	{
		// Adjust graphic

		// Loop through and and find opened graphic for the closed one
		for (INT32 i = 0; gOpenDoorList[i] != -1; ++i)
		{
			// IF WE ARE A SHADOW TYPE
			if (node->usIndex == gOpenDoorList[i])
			{
				// OK, now use opened graphic.
				node->usIndex = gClosedDoorList[i];
				goto dirty_end;
			}
		}
		return;
	}

	// If we want to be closed but structure is closed
	if (!want_to_be_open && !(s->fFlags & STRUCTURE_OPEN))
	{
		// Adjust graphic

		// Loop through and and find closed graphic for the opend one
		for (INT32 i = 0; gClosedDoorList[i] != -1; ++i)
		{
			// IF WE ARE A SHADOW TYPE
			if (node->usIndex == gClosedDoorList[i])
			{
				node->usIndex = gOpenDoorList[i];
				goto dirty_end;
			}
		}
		return;
	}

	if (openend_graphic && !want_to_be_open)
	{
		// Close the beast!
		node->usIndex = gOpenDoorList[cnt];
	}
	else if (!openend_graphic && want_to_be_open)
	{
		bool different = false;
		// Find the closed door graphic and adjust
		for (INT32 i = 0; gOpenDoorList[i] != -1; ++i)
		{
			// IF WE ARE A SHADOW TYPE
			if (node->usIndex == gOpenDoorList[i])
			{
				// Open the beast!
				different = true;
				node->usIndex = gClosedDoorList[i];
				break;
			}
		}
		if (!different) return;
	}
	else
	{
		return;
	}

	{ GridNo const base_grid_no = base->sGridNo;
		SwapStructureForPartner(base);
		RecompileLocalMovementCosts(base_grid_no);
	}

dirty_end:
	if (dirty)
	{
		InvalidateWorldRedundency();
		SetRenderFlags(RENDER_FLAG_FULL);
	}
}


static bool InternalIsPerceivedDifferentThanReality(DOOR_STATUS const& d)
{
	return d.ubFlags & DOOR_PERCEIVED_NOTSET ||
		((d.ubFlags & DOOR_OPEN) != 0) != ((d.ubFlags & DOOR_PERCEIVED_OPEN) != 0);
}


static void InternalUpdateDoorsPerceivedValue(DOOR_STATUS& d)
{
	// Set perceived value the same as actual
	d.ubFlags &= ~(DOOR_PERCEIVED_NOTSET | DOOR_PERCEIVED_OPEN);
	d.ubFlags |= (d.ubFlags & DOOR_OPEN ? DOOR_PERCEIVED_OPEN : 0);
}


void SaveDoorStatusArrayToDoorStatusTempFile(INT16 const x, INT16 const y, INT8 const z)
{
	// Turn off any door busy flags
	FOR_EACH_DOOR_STATUS(d) d.ubFlags &= ~DOOR_BUSY;

	AutoSGPFile f(GCM->openTempFileForWriting(GetMapTempFileName(SF_DOOR_STATUS_TEMP_FILE_EXISTS, x, y, z), true));
	Assert(gpDoorStatus.size() <= UINT8_MAX);
	UINT8 numDoorStatus = static_cast<UINT8>(gpDoorStatus.size());
	FileWriteArray(f, numDoorStatus, gpDoorStatus.data());

	// Set the flag indicating that there is a door status array
	SetSectorFlag(x, y, z, SF_DOOR_STATUS_TEMP_FILE_EXISTS);
}


void LoadDoorStatusArrayFromDoorStatusTempFile()
{
	TrashDoorStatusArray();

	AutoSGPFile f(GCM->openTempFileForReading(GetMapTempFileName(SF_DOOR_STATUS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ)));

	// Load the number of elements in the door status array
	UINT8 numDoorStatus = 0;
	FileRead(f, &numDoorStatus, sizeof(UINT8));
	if (numDoorStatus == 0) return;

	gpDoorStatus.assign(numDoorStatus, DOOR_STATUS{});
	FileRead(f, gpDoorStatus.data(), sizeof(DOOR_STATUS) * numDoorStatus);

	// Set flags in map for containing a door status
	FOR_EACH_DOOR_STATUS(d)
	{
		gpWorldLevelData[d.sGridNo].ubExtFlags[0] |= MAPELEMENT_EXT_DOOR_STATUS_PRESENT;
	}

	// The graphics will be updated later in the loading process
	UpdateDoorGraphicsFromStatus();
}


void SaveKeyTableToSaveGameFile(HWFILE const f)
{
	FOR_EACH(KEY const, i, KeyTable)
	{
		KEY const& k = *i;
		BYTE       data[8];
		DataWriter d{data};
		INJ_SKIP(d, 4)
		INJ_U16( d, k.usSectorFound)
		INJ_U16( d, k.usDateFound)
		Assert(d.getConsumed() == lengthof(data));
		FileWrite(f, data, sizeof(data));
	}
}


void LoadKeyTableFromSaveedGameFile(HWFILE const f)
{
	FOR_EACH(KEY, i, KeyTable)
	{
		BYTE data[8];
		FileRead(f, data, sizeof(data));
		KEY&  k = *i;
		DataReader d{data};
		EXTR_SKIP(d, 4)
		EXTR_U16( d, k.usSectorFound)
		EXTR_U16( d, k.usDateFound)
		Assert(d.getConsumed() == lengthof(data));
	}
}


void ExamineDoorsOnEnteringSector()
{
	// If this is Omerta, don't do it
	if (GetTownIdForSector(SECTOR(gWorldSectorX, gWorldSectorY)) == OMERTA) return;

	// Check time
	if (GetWorldTotalMin() - gTacticalStatus.uiTimeSinceLastInTactical < 30) return;

	// If there is at least one human being in that sector, close doors.
	CFOR_EACH_NON_PLAYER_SOLDIER(s)
	{
		if (!s->bInSector) continue;

		FOR_EACH_DOOR_STATUS(d)
		{
			// If open, close
			if (!(d.ubFlags & DOOR_OPEN)) continue;
			HandleDoorChangeFromGridNo(0, d.sGridNo, TRUE);
		}
		break;
	}
}


void DropKeysInKeyRing(SOLDIERTYPE& s, GridNo const gridno, INT8 const level, Visibility const visible, bool const add_to_drop_list, INT32 const drop_list_slot, bool const use_unloaded)
{
	KEY_ON_RING* const key_ring = s.pKeyRing;
	if (!key_ring) return; // No key ring

	bool const here = !use_unloaded && s.sSectorX == gWorldSectorX && s.sSectorY == gWorldSectorY && s.bSectorZ == gbWorldSectorZ;
	for (KEY_ON_RING* i = key_ring; i != key_ring + NUM_KEYS; ++i)
	{
		KEY_ON_RING& k = *i;
		if (k.ubNumber == 0) continue;

		OBJECTTYPE o;
		CreateKeyObject(&o, k.ubNumber, k.ubKeyID);

		// Zero out entry
		k.ubNumber = 0;
		k.ubKeyID  = INVALID_KEY_NUMBER;

		if (add_to_drop_list)
		{
			AddItemToLeaveIndex(&o, drop_list_slot);
		}
		else if (here)
		{
			AddItemToPool(gridno, &o, visible, level, 0, 0);
		}
		else
		{
			AddItemsToUnLoadedSector(s.sSectorX, s.sSectorY, s.bSectorZ, gridno, 1, &o, level, WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE, 0, visible);
		}
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Keys, asserts)
{
	EXPECT_EQ(sizeof(LOCK), 46u);
	EXPECT_EQ(sizeof(DOOR), 14u);
	EXPECT_EQ(sizeof(DOOR_STATUS), 4u);
}

#endif
