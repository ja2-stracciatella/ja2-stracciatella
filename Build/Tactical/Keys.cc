#include "Font_Control.h"
#include "Handle_Items.h"
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


static DOOR_STATUS* gpDoorStatus     = NULL;
static UINT8        gubNumDoorStatus = 0;


KEY KeyTable[NUM_KEYS] =
{
	// Item #			Flags		Sector, Date Found
	//
	{KEY_1,				0,			0,			0},
	{KEY_2,				0,			0,			0},
	{KEY_3,				0,			0,			0},
	{KEY_4,				0,			0,			0},
	{KEY_5,				0,			0,			0},
	{KEY_6,				0,			0,			0},
	{KEY_7,				0,			0,			0},
	{KEY_8,				0,			0,			0},
	{KEY_9,				0,			0,			0},
	{KEY_10,			0,			0,			0},

	{KEY_11,			0,			0,			0},
	{KEY_12,			0,			0,			0},
	{KEY_13,			0,			0,			0},
	{KEY_14,			0,			0,			0},
	{KEY_15,			0,			0,			0},
	{KEY_16,			0,			0,			0},
	{KEY_17,			0,			0,			0},
	{KEY_18,			0,			0,			0},
	{KEY_19,			0,			0,			0},
	{KEY_20,			0,			0,			0},

	{KEY_21,			0,			0,			0},
	{KEY_22,			0,			0,			0},
	{KEY_23,			0,			0,			0},
	{KEY_24,			0,			0,			0},
	{KEY_25,			0,			0,			0},
	{KEY_26,			0,			0,			0},
	{KEY_27,			0,			0,			0},
	{KEY_28,			0,			0,			0},
	{KEY_29,			0,			0,			0},
	{KEY_30,			0,			0,			0},

	{KEY_31,			0,			0,			0},
	{KEY_32,			0,			0,			0}

};

//Current number of doors in world.
UINT8 gubNumDoors = 0;

//Current max number of doors.  This is only used by the editor.  When adding doors to the
//world, we may run out of space in the DoorTable, so we will allocate a new array with extra slots,
//then copy everything over again.  gubMaxDoors holds the arrays actual number of slots, even though
//the current number (gubNumDoors) will be <= to it.
static UINT8 gubMaxDoors = 0;

LOCK LockTable[NUM_LOCKS];

/*
LOCK LockTable[NUM_LOCKS] =
{
	// Keys that will open the lock				Lock type			Pick diff			Smash diff
	{ { NO_KEY, NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,					0,						0},
	{ { 0,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-25,					-25},
	{ { 1,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-60,					-55},
	{ { 2,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-75,					-80},
	{ { 3,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-35,					-45},
	{ { 4,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-45,					-60},
	{ { 5,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-65,					-90},
	{ { 6,			NO_KEY, NO_KEY, NO_KEY},	LOCK_PADLOCK,				-60,					-70},
	{ { 7,			NO_KEY, NO_KEY, NO_KEY},	LOCK_ELECTRONIC,		-50,					-60},
	{ { 8,			NO_KEY, NO_KEY, NO_KEY},	LOCK_ELECTRONIC,		-75,					-80},
	{ { 9,			NO_KEY, NO_KEY, NO_KEY},	LOCK_CARD,					-50,					-40},
	{ { 10,			NO_KEY, NO_KEY, NO_KEY},	LOCK_CARD,					-85,					-80},
	{ { 11,			NO_KEY, NO_KEY, NO_KEY},	LOCK_REGULAR,				-50,					-50}
};
*/

DOORTRAP const DoorTrapTable[NUM_DOOR_TRAPS] =
{
	{ 0 },																						// nothing
	{ DOOR_TRAP_STOPS_ACTION },												// explosion
	{ DOOR_TRAP_STOPS_ACTION | DOOR_TRAP_RECURRING},	// electric
	{ DOOR_TRAP_RECURRING },													// siren
	{ DOOR_TRAP_RECURRING | DOOR_TRAP_SILENT},				// silent alarm
	{ DOOR_TRAP_RECURRING },													// brothel siren
	{ DOOR_TRAP_STOPS_ACTION | DOOR_TRAP_RECURRING},	// super electric
};



//Dynamic array of Doors.  For general game purposes, the doors that are locked and/or trapped
//are permanently saved within the map, and are loaded and allocated when the map is loaded.  Because
//the editor allows more doors to be added, or removed, the actual size of the DoorTable may change.
DOOR * DoorTable = NULL;


void LoadLockTable(void)
try
{
	UINT32	uiBytesToRead;
	const char *pFileName = "BINARYDATA/Locks.bin";

	// Load the Lock Table

	AutoSGPFile hFile(FileOpen(pFileName, FILE_ACCESS_READ));

	uiBytesToRead = sizeof( LOCK ) * NUM_LOCKS;
	FileRead(hFile, LockTable, uiBytesToRead);
}
catch (...)
{
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("FAILED to LoadLockTable"));
	throw;
}


static bool KeyExistsInInventory(SOLDIERTYPE const&, UINT8 key_id);


bool SoldierHasKey(SOLDIERTYPE const& s, UINT8 const key_id)
{
	return
		KeyExistsInKeyRing(s, key_id) ||
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
	CFOR_ALL_SOLDIER_INV_SLOTS(i, s)
	{
		if (Item[i->usItem].usItemClass != IC_KEY)    continue;
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
	INT32		iResult;
	INT8		bStress, bSlot;

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
    StatChange(pSoldier, STRAMT, 20, FROM_SUCCESS);

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
			StatChange(pSoldier, STRAMT, 5, FROM_SUCCESS);

			// we came close... so do some damage to the lock
			pDoor->bLockDamage += (INT8) (10 + iResult);
		}
		else if ( iResult > -40 && pSoldier->sGridNo != pSoldier->sSkillCheckGridNo )
		{
			// give token point for effort :-)
			StatChange(pSoldier, STRAMT, 1, FROM_SUCCESS);
		}

		return( FALSE );
	}

}


BOOLEAN AttemptToSmashDoor( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32		iResult;

	LOCK * pLock;

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
    StatChange(pSoldier, STRAMT, 20, FROM_SUCCESS);

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
			StatChange(pSoldier, STRAMT, 5, FROM_SUCCESS);

			// we came close... so do some damage to the lock
			pDoor->bLockDamage += (INT8) (10 + iResult);
		}
		else if ( iResult > -40 && pSoldier->sGridNo != pSoldier->sSkillCheckGridNo )
		{
			// give token point for effort :-)
			StatChange(pSoldier, STRAMT, 1, FROM_SUCCESS);
		}
		return( FALSE );
	}
}

BOOLEAN AttemptToPickLock( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32	iResult;
	INT8 bReason;
	LOCK * pLock;

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
    StatChange(pSoldier, MECHANAMT, pLock->ubPickDifficulty / 5, FROM_SUCCESS);

	  // DEXTERITY GAIN:  Picked open a lock
    StatChange(pSoldier, DEXTAMT, pLock->ubPickDifficulty / 10, FROM_SUCCESS);

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
	INT32		iResult;

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


void HandleDoorTrap(SOLDIERTYPE* const s, const DOOR* const d)
{
	if (!(DoorTrapTable[d->ubTrapID].fFlags & DOOR_TRAP_SILENT))
	{
		wchar_t const* trap_name;
		switch (d->ubTrapID)
		{
			case BROTHEL_SIREN:  trap_name = pDoorTrapStrings[SIREN];       break;
			case SUPER_ELECTRIC: trap_name = pDoorTrapStrings[ELECTRIC];    break;
			default:             trap_name = pDoorTrapStrings[d->ubTrapID]; break;
		}
		ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[LOCK_TRAP_HAS_GONE_OFF_STR], trap_name);
	}

	// set trap off
	switch (d->ubTrapID)
	{
		case EXPLOSION:
			// cause damage as a regular hand grenade
			IgniteExplosion(NULL, 25, s->sGridNo, HAND_GRENADE, 0);
			break;

 		case SIREN:
			/* play siren sound effect but otherwise treat as silent alarm, calling
			 * available enemies to this location */
			PlayLocationJA2Sample(d->sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			/* FALLTHROUGH */

		case SILENT_ALARM:
			// Get all available enemies running here
			CallAvailableEnemiesTo(d->sGridNo);
			break;

		case BROTHEL_SIREN:
			PlayLocationJA2Sample(d->sGridNo, KLAXON_ALARM, MIDVOLUME, 5);
			CallAvailableKingpinMenTo(d->sGridNo);
			// no one is authorized any more!
			gMercProfiles[MADAME].bNPCData = 0;
			break;

		case ELECTRIC:
			// insert electrical sound effect here
			PlayLocationJA2Sample(d->sGridNo, DOOR_ELECTRICITY, MIDVOLUME, 1);

	    s->attacker = s;
	    s->bBeingAttackedCount++;
		  gTacticalStatus.ubAttackBusyCount++;
		  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Trap gone off %d", gTacticalStatus.ubAttackBusyCount));

			SoldierTakeDamage(s, 10 + PreRandom(10), 3 + PreRandom(3) * 1000, TAKE_DAMAGE_ELECTRICITY, NULL);
			break;

		case SUPER_ELECTRIC:
			// insert electrical sound effect here
			PlayLocationJA2Sample(d->sGridNo, DOOR_ELECTRICITY, MIDVOLUME, 1);

	    s->attacker = s;
	    s->bBeingAttackedCount++;
		  gTacticalStatus.ubAttackBusyCount++;
		  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Trap gone off %d", gTacticalStatus.ubAttackBusyCount));

			SoldierTakeDamage(s, 20 + PreRandom(20), 6 + PreRandom(6) * 1000, TAKE_DAMAGE_ELECTRICITY, NULL);
			break;

		default:
			// no trap
			break;
	}
}


BOOLEAN AttemptToBlowUpLock( SOLDIERTYPE * pSoldier, DOOR * pDoor )
{
	INT32	iResult;
	INT8	bSlot = NO_SLOT;

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
			ANITILE_PARAMS	AniParams;
			INT16						sGridNo;
			INT16						sX, sY, sZ;

			// Get gridno
			sGridNo = pDoor->sGridNo;

			// Get sX, sy;
			sX = CenterX( sGridNo );
			sY = CenterY( sGridNo );

			// Get Z position, based on orientation....
			sZ = 20;

			AniParams.sGridNo							= sGridNo;
			AniParams.ubLevelID						= ANI_TOPMOST_LEVEL;
			AniParams.sDelay							= (INT16)( 100 );
			AniParams.sStartFrame					= 0;
			AniParams.uiFlags             = ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
			AniParams.sX									= sX;
			AniParams.sY									= sY;
			AniParams.sZ									= sZ;
			AniParams.zCachedFile = "TILECACHE/miniboom.sti";
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
		pDoor->bLockDamage += Explosive[Item[SHAPED_CHARGE].ubClassIndex].ubDamage * 2;
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
	INT32 cnt;

	TrashDoorTable();

	FileRead(f, &gubNumDoors, sizeof(gubNumDoors));
	if (gubNumDoors == 0) return;

	gubMaxDoors = gubNumDoors;
	DoorTable = MALLOCN(DOOR, gubMaxDoors);
	FileRead(f, DoorTable, sizeof(*DoorTable) * gubMaxDoors);

	// OK, reset perceived values to nothing...
	for ( cnt = 0; cnt < gubNumDoors; cnt++ )
	{
		DoorTable[ cnt ].bPerceivedLocked = DOOR_PERCEIVED_UNKNOWN;
		DoorTable[ cnt ].bPerceivedTrapped = DOOR_PERCEIVED_UNKNOWN;
	}
}


#ifdef JA2EDITOR

//Saves the existing door information to the map.  Before it actually saves, it'll verify that the
//door still exists.  Otherwise, it'll ignore it.  It is possible in the editor to delete doors in
//many different ways, so I opted to put it in the saving routine.
void SaveDoorTableToMap( HWFILE fp )
{
	INT32 i = 0;

	while( i < gubNumDoors )
	{
		if( !OpenableAtGridNo( DoorTable[ i ].sGridNo ) )
			RemoveDoorInfoFromTable( DoorTable[ i ].sGridNo );
		else
			i++;
	}
	FileWriteArray(fp, gubNumDoors, DoorTable);
}

#endif


//The editor adds locks to the world.  If the gridno already exists, then the currently existing door
//information is overwritten.
void AddDoorInfoToTable( DOOR *pDoor )
{
	INT32 i;
	for( i = 0; i < gubNumDoors; i++ )
	{
		if( DoorTable[ i ].sGridNo == pDoor->sGridNo )
		{
			DoorTable[i] = *pDoor;
			return;
		}
	}
	//no existing door found, so add a new one.
	if( gubNumDoors < gubMaxDoors )
	{
		DoorTable[gubNumDoors] = *pDoor;
		gubNumDoors++;
	}
	else
	{ //we need to allocate more memory, so add ten more slots.
		gubMaxDoors += 10;
		//Allocate new table with max+10 doors.
		DOOR* const NewDoorTable = MALLOCN(DOOR, gubMaxDoors);
		//Copy contents of existing door table to new door table.
		memcpy( NewDoorTable, DoorTable, sizeof( DOOR ) * gubNumDoors );
		//Deallocate the existing door table (possible to not have one).
		if( DoorTable )
			MemFree( DoorTable );
		//Assign the new door table as the existing door table
		DoorTable = NewDoorTable;
		//Add the new door info to the table.
		DoorTable[gubNumDoors] = *pDoor;
		gubNumDoors++;
	}
}

//When the editor removes a door from the world, this function looks for and removes accompanying door
//information.  If the entry is not the last entry, the last entry is move to it's current slot, to keep
//everything contiguous.
void RemoveDoorInfoFromTable( INT32 iMapIndex )
{
	INT32 i;
  INT32 iNumDoorsToCopy;
	for( i = 0; i < gubNumDoors; i++ )
	{
		if( DoorTable[ i ].sGridNo == iMapIndex )
		{
      iNumDoorsToCopy = gubNumDoors - i - 1;
      if( iNumDoorsToCopy )
      {
			  memmove( &DoorTable[ i ], &DoorTable[ i+1 ], sizeof( DOOR ) * iNumDoorsToCopy );
      }
			gubNumDoors--;
			return;
		}
	}
}

//This is the link to see if a door exists at a gridno.
DOOR* FindDoorInfoAtGridNo( INT32 iMapIndex )
{
	INT32 i;
	for( i = 0; i < gubNumDoors; i++ )
	{
		if( DoorTable[ i ].sGridNo == iMapIndex )
			return &DoorTable[ i ];
	}
	return NULL;
}

//Upon world deallocation, the door table needs to be deallocated.  Remember, this function
//resets the values, so make sure you do this before you change gubNumDoors or gubMaxDoors.
void TrashDoorTable()
{
	if( DoorTable )
		MemFree( DoorTable );
	DoorTable = NULL;
	gubNumDoors = 0;
	gubMaxDoors = 0;
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


void SaveDoorTableToDoorTableTempFile(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	UINT32	uiSizeToSave=0;
	CHAR8		zMapName[ 128 ];

//	return( TRUE );

	uiSizeToSave = gubNumDoors * sizeof( DOOR );

	GetMapTempFileName( SF_DOOR_TABLE_TEMP_FILES_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ );

	AutoSGPFile hFile(FileOpen(zMapName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS));

	//Save the number of doors
	FileWrite(hFile, &gubNumDoors, sizeof(UINT8));

	//if there are doors to save
	if (uiSizeToSave != 0) FileWrite(hFile, DoorTable, uiSizeToSave);

	//Set the sector flag indicating that there is a Door table temp file present
	SetSectorFlag( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, SF_DOOR_TABLE_TEMP_FILES_EXISTS );
}


void LoadDoorTableFromDoorTableTempFile()
{
	CHAR8		zMapName[ 128 ];

//	return( TRUE );

	GetMapTempFileName( SF_DOOR_TABLE_TEMP_FILES_EXISTS, zMapName, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//If the file doesnt exists, its no problem.
	if (!FileExists(zMapName)) return;

	//Get rid of the existing door table
	TrashDoorTable();

	AutoSGPFile hFile(FileOpen(zMapName, FILE_ACCESS_READ));

	//Read in the number of doors
	FileRead(hFile, &gubMaxDoors, sizeof(UINT8));

	gubNumDoors = gubMaxDoors;

	//if there is no doors to load
	if( gubNumDoors != 0 )
	{
		DoorTable = MALLOCN(DOOR, gubMaxDoors);
		FileRead(hFile, DoorTable, sizeof(DOOR) * gubMaxDoors);
	}
}


// fOpen is True if the door is open, false if it is closed
BOOLEAN ModifyDoorStatus( INT16 sGridNo, BOOLEAN fOpen, BOOLEAN fPerceivedOpen )
try
{
	UINT8	ubCnt;
	STRUCTURE * pStructure;
	STRUCTURE * pBaseStructure;

	//Set the gridno for the door

	// Find the base tile for the door structure and use that gridno
	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
	}
	else
	{
		pBaseStructure = NULL;
	}

	if ( pBaseStructure == NULL )
	{
#if 0
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Door structure data at %d was not found", sGridNo );
#endif
		return( FALSE );
	}

	//if there is an array
	if( gpDoorStatus )
	{
		//Check to see if the user is adding an existing door
		for( ubCnt=0; ubCnt<gubNumDoorStatus; ubCnt++)
		{
			//if the door is already in the array
			if( gpDoorStatus[ ubCnt ].sGridNo == 	pBaseStructure->sGridNo )
			{
				//set the status
				// ATE: Don't set if set to DONTSET
				if ( fPerceivedOpen != DONTSETDOORSTATUS )
				{
					if( fPerceivedOpen )
						gpDoorStatus[ ubCnt ].ubFlags |= DOOR_PERCEIVED_OPEN;
					else
						gpDoorStatus[ ubCnt ].ubFlags &= ~DOOR_PERCEIVED_OPEN;

					// Turn off perceived not set flag....
						gpDoorStatus[ ubCnt ].ubFlags &= ~DOOR_PERCEIVED_NOTSET;

				}

				if ( fOpen != DONTSETDOORSTATUS )
				{
					if( fOpen )
						gpDoorStatus[ ubCnt ].ubFlags |= DOOR_OPEN;
					else
						gpDoorStatus[ ubCnt ].ubFlags &= ~DOOR_OPEN;
				}

				//Dont add it
				return( TRUE );
			}
		}
	}

	// add a new door status structure

	//if there is an array
	if( gpDoorStatus )
	{
		//Increment the number of doors
		gubNumDoorStatus++;

		//reallocate memory to hold the new door
		gpDoorStatus = REALLOC(gpDoorStatus, DOOR_STATUS, gubNumDoorStatus);
	}
	else
	{
		//Set the initial number of doors
		gubNumDoorStatus = 1;

		gpDoorStatus = MALLOC(DOOR_STATUS);
	}

	gpDoorStatus[ gubNumDoorStatus-1 ].sGridNo = pBaseStructure->sGridNo;

	//Init the flags
	gpDoorStatus[ gubNumDoorStatus-1 ].ubFlags = 0;

	//If the door is to be initially open
	if( fOpen )
		gpDoorStatus[ gubNumDoorStatus-1 ].ubFlags |= DOOR_OPEN;

	// IF A NEW DOOR, USE SAME AS ACTUAL
	if ( fPerceivedOpen != DONTSETDOORSTATUS )
	{
		if( fOpen )
			gpDoorStatus[ gubNumDoorStatus-1 ].ubFlags |= DOOR_PERCEIVED_OPEN;
	}
	else
	{
		gpDoorStatus[ gubNumDoorStatus-1 ].ubFlags |= DOOR_PERCEIVED_NOTSET;
	}

	// flag the tile as containing a door status
	gpWorldLevelData[ pBaseStructure->sGridNo ].ubExtFlags[0] |= MAPELEMENT_EXT_DOOR_STATUS_PRESENT;

	return( TRUE );
}
catch (...) { return FALSE; }


void TrashDoorStatusArray( )
{
	if( gpDoorStatus )
	{
		MemFree( gpDoorStatus );
		gpDoorStatus = NULL;
	}

	gubNumDoorStatus = 0;
}


// Returns a doors status value, NULL if not found
DOOR_STATUS* GetDoorStatus(INT16 const sGridNo)
{
	if (!gpDoorStatus) return 0;

	// Find the base tile for the door structure and use that gridno
	STRUCTURE* const structure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
	if (!structure) return 0;
	STRUCTURE const* const base = FindBaseStructure(structure);
	if (!base) return 0;

	DOOR_STATUS* const end = gpDoorStatus + gubNumDoorStatus;
	for (DOOR_STATUS* i = gpDoorStatus; i != end; ++i)
	{
		if (i->sGridNo == base->sGridNo) return i;
	}

	return 0;
}


BOOLEAN AllMercsLookForDoor(INT16 sGridNo)
{
	INT8										 bDirs[ 8 ] = { NORTH, SOUTH, EAST, WEST, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST };
	INT16										 sDistVisible;
	INT16											usNewGridNo;

	if (GetDoorStatus(sGridNo) == NULL) return FALSE;

	CFOR_ALL_IN_TEAM(pSoldier, gbPlayerNum)
	{
		// ATE: Ok, lets check for some basic things here!
		if (pSoldier->bLife >= OKLIFE && pSoldier->sGridNo != NOWHERE && pSoldier->bInSector)
		{
			// is he close enough to see that gridno if he turns his head?
			sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, 0 );

			if (PythSpacesAway( pSoldier->sGridNo, sGridNo ) <= sDistVisible )
			{
				// and we can trace a line of sight to his x,y coordinates?
				// (taking into account we are definitely aware of this guy now)
				if ( SoldierTo3DLocationLineOfSightTest( pSoldier, sGridNo, 0, 0, (UINT8) sDistVisible, TRUE ) )
				{
					return( TRUE );
				}
			}

			// Now try other adjacent gridnos...
			for (INT32 cnt2 = 0; cnt2 < 8; ++cnt2)
			{
					usNewGridNo = NewGridNo( sGridNo, DirectionInc( bDirs[ cnt2 ] ) );
					sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, usNewGridNo, 0 );

					if (PythSpacesAway( pSoldier->sGridNo, usNewGridNo ) <= sDistVisible )
					{
						// and we can trace a line of sight to his x,y coordinates?
						// (taking into account we are definitely aware of this guy now)
						if ( SoldierTo3DLocationLineOfSightTest( pSoldier, usNewGridNo, 0, 0, (UINT8) sDistVisible, TRUE ) )
						{
							return( TRUE );
						}
					}
			}
		}
	}

	return( FALSE );
}


static BOOLEAN InternalIsPerceivedDifferentThanReality(const DOOR_STATUS* d);
static void InternalUpdateDoorGraphicFromStatus(DOOR_STATUS const&, bool dirty);
static void InternalUpdateDoorsPerceivedValue(DOOR_STATUS* d);


BOOLEAN MercLooksForDoors(const SOLDIERTYPE* s, BOOLEAN fUpdateValue)
{
	static const INT8 bDirs[] = { NORTH, SOUTH, EAST, WEST, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST };

	// Loop through all corpses....
	for (INT32 cnt = 0; cnt < gubNumDoorStatus; ++cnt)
	{
		DOOR_STATUS* d = &gpDoorStatus[cnt];

		if (!InternalIsPerceivedDifferentThanReality(d))
		{
			continue;
		}

		const INT16 sGridNo = d->sGridNo;

		// is he close enough to see that gridno if he turns his head?
		const INT16 sDistVisible = DistanceVisible(s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, 0);

		if (PythSpacesAway(s->sGridNo, sGridNo) <= sDistVisible)
		{
			// and we can trace a line of sight to his x,y coordinates?
			// (taking into account we are definitely aware of this guy now)
			if (SoldierTo3DLocationLineOfSightTest(s, sGridNo, 0, 0, sDistVisible, TRUE))
			{
				// OK, here... update perceived value....
				if (fUpdateValue)
				{
					InternalUpdateDoorsPerceivedValue(d);

					// Update graphic....
					InternalUpdateDoorGraphicFromStatus(*d, true);
				}
				return TRUE;
			}
		}

		// Now try other adjacent gridnos...
		for (INT32 cnt2 = 0; cnt2 < 8; ++cnt2)
		{
			const INT16 usNewGridNo = NewGridNo(sGridNo, DirectionInc(bDirs[cnt2]));

			if (PythSpacesAway(s->sGridNo, usNewGridNo) <= sDistVisible)
			{
				// and we can trace a line of sight to his x,y coordinates?
				// (taking into account we are definitely aware of this guy now)
				if (SoldierTo3DLocationLineOfSightTest(s, usNewGridNo, 0, 0, sDistVisible, TRUE))
				{
					// Update status...
					if (fUpdateValue)
					{
						InternalUpdateDoorsPerceivedValue(d);

						// Update graphic....
						InternalUpdateDoorGraphicFromStatus(*d, true);
					}
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


static void SyncronizeDoorStatusToStructureData(DOOR_STATUS const& d)
{
	// First look for a door structure here
	STRUCTURE* const s = FindStructure(d.sGridNo, STRUCTURE_ANYDOOR);
	if (!s) return;

	/* ATE: One of the purposes of this function is to MAKE sure the door status
	 * MATCHES the struct data value - if not - change (REGARDLESS of perceived
	 * being used or not) */
	bool const door_open   = d.ubFlags & DOOR_OPEN;
	bool const struct_open = s->fFlags & STRUCTURE_OPEN;
	if (door_open == struct_open) return;

	// Swap!
	STRUCTURE* const base = FindBaseStructure(s);
	SwapStructureForPartner(base);
	RecompileLocalMovementCosts(base->sGridNo);
}


void UpdateDoorGraphicsFromStatus(void)
{
	for (INT32 cnt = 0; cnt < gubNumDoorStatus; ++cnt)
	{
		DOOR_STATUS* pDoorStatus = &gpDoorStatus[cnt];

		// ATE: Make sure door status flag and struct info are syncronized....
		SyncronizeDoorStatusToStructureData(*pDoorStatus);

		InternalUpdateDoorGraphicFromStatus(*pDoorStatus, false);
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

	/* We either have an opened graphic, in which case we want to switch to the
	 * closed, or a closed in which case we want to switch to opened
	 * adjust o' graphic */

	/* We now need to test these things against the true structure data we may
	 * need to only adjust the graphic here */
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


static BOOLEAN InternalIsPerceivedDifferentThanReality(const DOOR_STATUS* d)
{
	return
		d->ubFlags & DOOR_PERCEIVED_NOTSET ||
		((d->ubFlags & DOOR_OPEN) != 0) != ((d->ubFlags & DOOR_PERCEIVED_OPEN) != 0);
}


static void InternalUpdateDoorsPerceivedValue(DOOR_STATUS* d)
{
	// OK, look at door, set perceived value the same as actual....
	d->ubFlags &= ~(DOOR_PERCEIVED_NOTSET | DOOR_PERCEIVED_OPEN);
	d->ubFlags |= (d->ubFlags & DOOR_OPEN ? DOOR_PERCEIVED_OPEN : 0);
}


void SaveDoorStatusArrayToDoorStatusTempFile(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	CHAR8		zMapName[ 128 ];
	UINT8		ubCnt;

	// Turn off any DOOR BUSY flags....
	for( ubCnt=0; ubCnt < gubNumDoorStatus; ubCnt++)
	{
		gpDoorStatus[ ubCnt ].ubFlags &= ( ~DOOR_BUSY );
	}

	GetMapTempFileName( SF_DOOR_STATUS_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ );

	AutoSGPFile f(FileOpen(zMapName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS));
	FileWriteArray(f, gubNumDoorStatus, gpDoorStatus);

	//Set the flag indicating that there is a door status array
	SetSectorFlag( sSectorX, sSectorY, bSectorZ, SF_DOOR_STATUS_TEMP_FILE_EXISTS );
}


void LoadDoorStatusArrayFromDoorStatusTempFile()
{
	CHAR8		zMapName[ 128 ];
	UINT8		ubLoop;

	GetMapTempFileName( SF_DOOR_STATUS_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//Get rid of the existing door array
	TrashDoorStatusArray( );

	AutoSGPFile hFile(FileOpen(zMapName, FILE_ACCESS_READ));

	// Load the number of elements in the door status array
	FileRead(hFile, &gubNumDoorStatus, sizeof(UINT8));

	if (gubNumDoorStatus == 0) return;

	//Allocate space for the door status array
	gpDoorStatus = MALLOCNZ(DOOR_STATUS, gubNumDoorStatus);

	// Load the number of elements in the door status array
	FileRead(hFile, gpDoorStatus, sizeof(DOOR_STATUS) * gubNumDoorStatus);

	// the graphics will be updated later in the loading process.

	// set flags in map for containing a door status
	for (ubLoop = 0; ubLoop < gubNumDoorStatus; ubLoop++)
	{
		gpWorldLevelData[ gpDoorStatus[ ubLoop ].sGridNo ].ubExtFlags[0] |= MAPELEMENT_EXT_DOOR_STATUS_PRESENT;
	}

	UpdateDoorGraphicsFromStatus();
}


void SaveKeyTableToSaveGameFile(HWFILE const hFile)
{
	// Save the KeyTable
	FileWrite(hFile, KeyTable, sizeof(KEY) * NUM_KEYS);
}


void LoadKeyTableFromSaveedGameFile(HWFILE const hFile)
{
	// Load the KeyTable
	FileRead(hFile, KeyTable, sizeof(KEY) * NUM_KEYS);
}


void ExamineDoorsOnEnteringSector( )
{
	DOOR_STATUS							 *pDoorStatus;
	BOOLEAN									 fOK = FALSE;
	INT8										 bTownId;

	// OK, only do this if conditions are met....
	// If this is any omerta tow, don't do it...
	bTownId = GetTownIdForSector( gWorldSectorX, gWorldSectorY );

	if ( bTownId == OMERTA )
	{
		return;
	}

	// Check time...
	if ( ( GetWorldTotalMin( ) - gTacticalStatus.uiTimeSinceLastInTactical ) < 30 )
	{
		return;
	}

	// there is at least one human being in that sector.
	// check for civ
	CFOR_ALL_NON_PLAYER_SOLDIERS(s)
	{
		if (s->bInSector)
		{
			fOK = TRUE;
			break;
		}
	}

	// Let's do it!
	if ( fOK )
	{
		for (INT32 cnt = 0; cnt < gubNumDoorStatus; ++cnt)
		{
			pDoorStatus = &( gpDoorStatus[ cnt ] );

			// Get status of door....
			if ( pDoorStatus->ubFlags & DOOR_OPEN )
			{
				// If open, close!
				HandleDoorChangeFromGridNo( NULL, pDoorStatus->sGridNo, TRUE );
			}
		}
	}
}


void DropKeysInKeyRing(SOLDIERTYPE* const pSoldier, INT16 const sGridNo, INT8 const bLevel, Visibility const bVisible, BOOLEAN const fAddToDropList, INT32 const iDropListSlot, BOOLEAN const fUseUnLoaded)
{
	UINT8		    ubLoop;
  UINT8       ubItem;
  OBJECTTYPE  Object;

	if (!(pSoldier->pKeyRing))
	{
		// no key ring!
		return;
	}
	for (ubLoop = 0; ubLoop < NUM_KEYS; ubLoop++)
	{
  	ubItem = pSoldier->pKeyRing[ ubLoop ].ubKeyID;

    if ( pSoldier->pKeyRing[ubLoop].ubNumber > 0 )
    {
  	  CreateKeyObject( &Object, pSoldier->pKeyRing[ubLoop].ubNumber, ubItem );

      // Zero out entry
		  pSoldier->pKeyRing[ ubLoop ].ubNumber = 0;
		  pSoldier->pKeyRing[ ubLoop ].ubKeyID = INVALID_KEY_NUMBER;

      if ( fAddToDropList )
      {
        AddItemToLeaveIndex( &Object, iDropListSlot );
      }
      else
      {
	      if( pSoldier->sSectorX != gWorldSectorX || pSoldier->sSectorY != gWorldSectorY || pSoldier->bSectorZ != gbWorldSectorZ || fUseUnLoaded )
	      {
          // Set flag for item...
					AddItemsToUnLoadedSector(pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ , sGridNo, 1, &Object , bLevel, WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE, 0, bVisible);
        }
			  else
			  {
          // Add to pool
          AddItemToPool( sGridNo, &Object, bVisible, bLevel, 0, 0 );
			  }
      }
    }
	}
}
