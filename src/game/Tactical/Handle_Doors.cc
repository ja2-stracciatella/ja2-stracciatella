#include "Cursors.h"
#include "Font_Control.h"
#include "TileDat.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "RenderWorld.h"
#include "Soldier_Find.h"
#include "Structure.h"
#include "Animation_Control.h"
#include "Points.h"
#include "Overhead.h"
#include "Structure_Wrap.h"
#include "Tile_Animation.h"
#include "Interactive_Tiles.h"
#include "Handle_Doors.h"
#include "Sound_Control.h"
#include "Interface.h"
#include "Keys.h"
#include "Message.h"
#include "Text.h"
#include "Random.h"
#include "SkillCheck.h"
#include "Dialogue_Control.h"
#include "Render_Fun.h"
#include "Quests.h"
#include "StrategicMap.h"
#include "Map_Screen_Interface_Map.h"
#include "Soldier_Profile.h"
#include "Isometric_Utils.h"
#include "AI.h"
#include "Soldier_Macros.h"
#include "Debug.h"
#include "GameRes.h"

#include <string_theory/string>


static BOOLEAN HandleDoorsOpenClose(SOLDIERTYPE* pSoldier, INT16 sGridNo, STRUCTURE* pStructure, BOOLEAN fNoAnimations);


void HandleDoorChangeFromGridNo(SOLDIERTYPE* const s, INT16 const grid_no, BOOLEAN const no_animation)
{
	STRUCTURE* const structure = FindStructure(grid_no, STRUCTURE_ANYDOOR);
	if (!structure)
	{
		SLOGE("Told to handle door that does not exist at %d.", grid_no);
		return;
	}

	BOOLEAN const door_animated = HandleDoorsOpenClose(s, grid_no, structure, no_animation);
	if (SwapStructureForPartner(structure))
	{
		RecompileLocalMovementCosts(grid_no);
	}

	// set door busy
	DOOR_STATUS* const door_status = GetDoorStatus(grid_no);
	if (!door_status)
	{
		SLOGE("Told to set door busy but can't get door status at %d!", grid_no);
		return;
	}

	// ATE: Only do if animated.....
	if (door_animated) door_status->ubFlags |= DOOR_BUSY;
}


UINT16 GetAnimStateForInteraction(SOLDIERTYPE const& s, BOOLEAN const door, UINT16 const anim_state)
{
	bool const standing = gAnimControl[s.usAnimState].ubEndHeight == ANIM_STAND;
	switch (anim_state)
	{
		case OPEN_DOOR:
			if (s.ubBodyType == CRIPPLECIV)
			{
				return CRIPPLE_OPEN_DOOR;
			}
			else if (door)
			{
				return standing ? anim_state : OPEN_DOOR_CROUCHED;
			}
			else
			{
				return standing ? BEGIN_OPENSTRUCT : BEGIN_OPENSTRUCT_CROUCHED;
			}

		case CLOSE_DOOR:
			if (s.ubBodyType == CRIPPLECIV)
			{
				return CRIPPLE_CLOSE_DOOR;
			}
			else if (door)
			{
				return standing ? anim_state : CLOSE_DOOR_CROUCHED;
			}
			else
			{
				return standing ? OPEN_STRUCT : OPEN_STRUCT_CROUCHED;
			}

		case END_OPEN_DOOR:
			if (s.ubBodyType == CRIPPLECIV)
			{
				return CRIPPLE_END_OPEN_DOOR;
			}
			else if (door)
			{
				return standing ? anim_state : END_OPEN_DOOR_CROUCHED;
			}
			else
			{
				return standing ? END_OPENSTRUCT : END_OPENSTRUCT_CROUCHED;
			}

		case END_OPEN_LOCKED_DOOR:
			if (s.ubBodyType == CRIPPLECIV)
			{
				return CRIPPLE_END_OPEN_LOCKED_DOOR;
			}
			else if (door)
			{
				return standing ? END_OPEN_LOCKED_DOOR : END_OPEN_LOCKED_DOOR_CROUCHED;
			}
			else
			{
				return standing ? END_OPENSTRUCT_LOCKED : END_OPENSTRUCT_LOCKED_CROUCHED;
			}

		case PICK_LOCK:
			return standing ? PICK_LOCK : LOCKPICK_CROUCHED;

		default:
			// should never happen!
			Assert(FALSE);
			return anim_state;
	}
}


void InteractWithClosedDoor(SOLDIERTYPE* const pSoldier, HandleDoor const ubHandleCode)
{
	pSoldier->ubDoorHandleCode = ubHandleCode;

	UINT16 state;
	switch( ubHandleCode )
	{
		case HANDLE_DOOR_OPEN:
		case HANDLE_DOOR_UNLOCK:
		case HANDLE_DOOR_EXAMINE:
		case HANDLE_DOOR_EXPLODE:
		case HANDLE_DOOR_LOCK:
		case HANDLE_DOOR_UNTRAP:
		case HANDLE_DOOR_CROWBAR:
			state = GetAnimStateForInteraction(*pSoldier, TRUE, OPEN_DOOR);
			break;

		case HANDLE_DOOR_FORCE:
			state = KICK_DOOR;
			break;

		case HANDLE_DOOR_LOCKPICK:
			state = GetAnimStateForInteraction(*pSoldier, TRUE, PICK_LOCK);
			break;

		default: return;
	}
	ChangeSoldierState(pSoldier, state, 0 , FALSE);
}


static bool DoTrapCheckOnStartingMenu(SOLDIERTYPE& s, DOOR& d)
{
	if (!d.fLocked)
		return false;
	if (d.ubTrapID == NO_TRAP)
		return false;
	if (d.bPerceivedTrapped != DOOR_PERCEIVED_UNKNOWN)
		return false;

	// check for noticing the trap
	INT8 const detect_level = CalcTrapDetectLevel(&s, FALSE);
	if (detect_level < d.ubTrapLevel) return false;

	// say quote, update status
	TacticalCharacterDialogue(&s, QUOTE_BOOBYTRAP_ITEM);
	UpdateDoorPerceivedValue(&d);
	return true;
}


void InteractWithOpenableStruct(SOLDIERTYPE& s, STRUCTURE& structure, UINT8 const direction)
{
	STRUCTURE& base    = *FindBaseStructure(&structure);
	bool const is_door = structure.fFlags & STRUCTURE_ANYDOOR;

	if (is_door)
	{
		// get door status, if busy then just return!
		DOOR_STATUS const* const ds = GetDoorStatus(base.sGridNo);
		if (ds && ds->ubFlags & DOOR_BUSY)
		{
			// Send this guy into stationary stance
			EVENT_StopMerc(&s);
			return;
		}
	}

	if (!s.bCollapsed)
	{
		EVENT_SetSoldierDesiredDirectionForward(&s, direction);
	}

	// Is the door opened?
	if (structure.fFlags & STRUCTURE_OPEN)
	{
		if (IsOnOurTeam(s) && !(structure.fFlags & STRUCTURE_SWITCH))
		{
			// Bring up menu to decide what to do
			if (!s.bCollapsed)
			{
				SoldierGotoStationaryStance(&s);
			}
			DOOR* const d = FindDoorInfoAtGridNo(base.sGridNo);
			if (!d || !DoTrapCheckOnStartingMenu(s, *d))
				InitDoorOpenMenu(&s, TRUE);
		}
		else
		{ // Easily close door
			ChangeSoldierState(&s, GetAnimStateForInteraction(s, is_door, CLOSE_DOOR), 0, FALSE);
		}
	}
	else
	{
		if (IsOnOurTeam(s))
		{
			DOOR* const d = FindDoorInfoAtGridNo(base.sGridNo);
			if (d && d->fLocked) // Bring up the menu, only if it has a lock
			{
				// Bring up menu to decide what to do
				if (!s.bCollapsed)
				{
					SoldierGotoStationaryStance(&s);
				}
				if (!DoTrapCheckOnStartingMenu(s, *d))
				{
					InitDoorOpenMenu(&s, FALSE);
				}
				else
				{
					UnSetUIBusy(&s);
				}
				return;
			}
		}

		s.ubDoorHandleCode = HANDLE_DOOR_OPEN;
		ChangeSoldierState(&s, GetAnimStateForInteraction(s, is_door, OPEN_DOOR), 0, FALSE);
	}
}


static void ProcessImplicationsOfPCMessingWithDoor(SOLDIERTYPE* pSoldier)
{
	// if player is hacking at a door in the brothel and a kingpin guy can see him
	UINT8 const room = GetRoom(pSoldier->sGridNo);
	if (IN_BROTHEL(room) ||
		(gWorldSectorX == 5 &&
		gWorldSectorY == MAP_ROW_D &&
		gbWorldSectorZ == 0 &&
		(pSoldier->sGridNo == 11010 ||
		pSoldier->sGridNo == 11177 ||
		pSoldier->sGridNo == 11176)))
	{
		// see if a kingpin goon can see us
		FOR_EACH_IN_TEAM(pGoon, CIV_TEAM)
		{
			if (pGoon->ubCivilianGroup == KINGPIN_CIV_GROUP &&
				pGoon->bInSector &&
				pGoon->bLife >= OKLIFE &&
				pGoon->bOppList[pSoldier->ubID] == SEEN_CURRENTLY)
			{
				MakeCivHostile( pGoon, 2 );
				if ( ! (gTacticalStatus.uiFlags & INCOMBAT) )
				{
					EnterCombatMode( pGoon->bTeam );
				}
			}
		}
	}

	if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y )
	{
		SOLDIERTYPE* const pGoon = FindSoldierByProfileID(WARDEN);
		if ( pGoon && pGoon->bAlertStatus < STATUS_RED && PythSpacesAway( pSoldier->sGridNo, pGoon->sGridNo ) <= 5 )
		{
			// alert her if she hasn't been alerted
			pGoon->bAlertStatus = STATUS_RED;
			CheckForChangingOrders( pGoon );
			CancelAIAction(pGoon);
		}
	}
}



BOOLEAN HandleOpenableStruct( SOLDIERTYPE *pSoldier, INT16 sGridNo, STRUCTURE *pStructure )
{
	BOOLEAN fHandleDoor = FALSE;
	INT16   sAPCost = 0, sBPCost = 0;
	DOOR    *pDoor;
	BOOLEAN fDoAction = TRUE;
	BOOLEAN fDoor     = FALSE;

	// Are we a door?
	if (pStructure->fFlags & STRUCTURE_ANYDOOR)
	{
		fDoor = TRUE;
	}

	// Calculate basic points...

	// We'll add any aps for things like lockpicking, booting, etc

	// If we are already open....no need for lockpick checks, etc
	if ( pStructure->fFlags & STRUCTURE_OPEN )
	{
		// Set costs for these
		sAPCost = AP_OPEN_DOOR;
		sBPCost = BP_OPEN_DOOR;

		fHandleDoor = TRUE;
	}
	else
	{
		if (IsOnOurTeam(*pSoldier))
		{
			// Find locked door here....
			pDoor = FindDoorInfoAtGridNo( sGridNo );

			// Alrighty, first check for traps ( unless we are examining.... )
			if (pSoldier->ubDoorHandleCode != HANDLE_DOOR_EXAMINE &&
				pSoldier->ubDoorHandleCode != HANDLE_DOOR_UNTRAP &&
				pSoldier->ubDoorHandleCode != HANDLE_DOOR_UNLOCK)
			{
				if ( pDoor != NULL )
				{
					// Do we have a trap? NB if door is unlocked disable all traps
					if (pDoor->fLocked && pDoor->ubTrapID != NO_TRAP)
					{
						// Set costs for these
						// Set AP costs to that of opening a door
						sAPCost = AP_OPEN_DOOR;
						sBPCost = BP_OPEN_DOOR;

						ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);

						// Did we inadvertently set it off?
						if ( HasDoorTrapGoneOff( pSoldier, pDoor ) )
						{
							// Kaboom
							// Code to handle trap here...
							HandleDoorTrap(*pSoldier, *pDoor);
							if (DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_STOPS_ACTION)
							{
								// trap stops person from opening door!
								fDoAction = FALSE;
							}
							if (!( DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING ) )
							{
								// trap only happens once
								pDoor->ubTrapLevel = 0;
								pDoor->ubTrapID = NO_TRAP;
							}
							UpdateDoorPerceivedValue( pDoor );
						}
						else
						{
							// If we didn't set it off then we must have noticed it or know about it already

							// do we know it's trapped?
							if ( pDoor->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN )
							{
								ST::string trap_name = GetTrapName(*pDoor);
								ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[DOOR_LOCK_DESCRIPTION_STR], trap_name));

								// Stop action this time....
								fDoAction = FALSE;

								// report!
								TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );
							}
							else
							{
								// Set it off!
								HandleDoorTrap(*pSoldier, *pDoor);
								if (DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_STOPS_ACTION)
								{
									// trap stops person from opening door!
									fDoAction = FALSE;
								}
								if (!( DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING ) )
								{
									// trap only happens once
									pDoor->ubTrapLevel = 0;
									pDoor->ubTrapID = NO_TRAP;
								}
							}
							UpdateDoorPerceivedValue( pDoor );

						}
					}
				}
			}

			if ( fDoAction )
			{
				// OK, switch based on how we are going to open door....
				switch (pSoldier->ubDoorHandleCode)
				{
					case HANDLE_DOOR_OPEN:

						// If we have no lock on door...
						if ( pDoor == NULL )
						{
							// Set costs for these
							sAPCost = AP_OPEN_DOOR;
							sBPCost = BP_OPEN_DOOR;

							// Open if it's not locked....
							ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
							fHandleDoor = TRUE;
						}
						else
						{
							if ( pDoor->fLocked )
							{
								// it's locked....
								ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0, FALSE);

								// Do we have a quote for locked stuff?
								// Now just show on message bar
								if ( !AM_AN_EPC( pSoldier ) )
								{
									DoMercBattleSound( pSoldier, BATTLE_SOUND_LOCKED );
								}
								else
								{
									ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_LOCKED_STR ] );
								}
							}
							else
							{
								ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
								fHandleDoor = TRUE;
							}
							UpdateDoorPerceivedValue( pDoor );
						}
						break;

					case HANDLE_DOOR_FORCE:

						// Set costs for these
						sAPCost = AP_BOOT_DOOR;
						sBPCost = BP_BOOT_DOOR;

						// OK, using force, if we have no lock, just open the door!
						if ( pDoor == NULL )
						{
							ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
							fHandleDoor = TRUE;

							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							// Attempt to force door
							if ( AttemptToSmashDoor( pSoldier, pDoor ) )
							{
								// DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								fHandleDoor = TRUE;
							}
							else
							{
								UpdateDoorPerceivedValue( pDoor );
							}
							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;


					case HANDLE_DOOR_CROWBAR:

						// Set costs for these
						sAPCost = AP_USE_CROWBAR;
						sBPCost = BP_USE_CROWBAR;

						// OK, using force, if we have no lock, just open the door!
						if ( pDoor == NULL )
						{
							ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
							fHandleDoor = TRUE;

							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							// Attempt to force door
							if ( AttemptToCrowbarLock( pSoldier, pDoor ) )
							{
								//DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								fHandleDoor = TRUE;
							}
							else
							{
								UpdateDoorPerceivedValue( pDoor );
							}

							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;

					case HANDLE_DOOR_EXPLODE:

						// Set costs for these
						sAPCost = AP_EXPLODE_DOOR;
						sBPCost = BP_EXPLODE_DOOR;

						if ( pDoor == NULL )
						{
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							// Attempt to force door
							if ( AttemptToBlowUpLock( pSoldier, pDoor ) )
							{
								//DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								fHandleDoor = TRUE;
							}
							else
							{
								UpdateDoorPerceivedValue( pDoor );
							}
							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;

					case HANDLE_DOOR_LOCKPICK:

						// Set costs for these
						sAPCost = AP_PICKLOCK;
						sBPCost = BP_PICKLOCK;

						// Attempt to pick lock
						if ( pDoor == NULL )
						{
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							if ( AttemptToPickLock( pSoldier, pDoor ) )
							{
								DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
								fHandleDoor = TRUE;
							}
							ProcessImplicationsOfPCMessingWithDoor( pSoldier );
						}
						break;


					case HANDLE_DOOR_EXAMINE:

						// Set costs for these
						sAPCost = AP_EXAMINE_DOOR;
						sBPCost = BP_EXAMINE_DOOR;

						// Attempt to examine door
						// Whatever the result, end the open animation
						ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);

						if ( pDoor == NULL )
						{
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
						}
						else
						{
							if ( ExamineDoorForTraps( pSoldier, pDoor ) )
							{
								// We have a trap. Use door pointer to determine what type, etc
								TacticalCharacterDialogue( pSoldier, QUOTE_BOOBYTRAP_ITEM );
								ST::string trap_name = GetTrapName(*pDoor);
								ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[DOOR_LOCK_DESCRIPTION_STR], trap_name));

								UpdateDoorPerceivedValue( pDoor );
							}
							else
							{
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_UNTRAPPED_STR ] );
							}
						}
						break;


					case HANDLE_DOOR_UNLOCK:

						// Set costs for these
						sAPCost = AP_UNLOCK_DOOR;
						sBPCost = BP_UNLOCK_DOOR;

						// OK, if we have no lock, show that!
						if ( pDoor == NULL )
						{
							// Open if it's not locked....
							//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
							ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
							break;
						}
						else
						{

							// it's locked....
							// Attempt to unlock....
							if ( AttemptToUnlockDoor( pSoldier, pDoor ) )
							{
								//DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );

								ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
								UpdateDoorPerceivedValue( pDoor );

								fHandleDoor = TRUE;
							}
							else
							{
								ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0, FALSE);
								// Do we have a quote for locked stuff?
								// Now just show on message bar
								//ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_NOT_PROPER_KEY_STR ], pSoldier->name );

								// OK PLay damn battle sound
								if ( Random( 2 ) )
								{
									DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
								}
							}
						}
						break;


					case HANDLE_DOOR_UNTRAP:

						// Set costs for these
						sAPCost = AP_UNTRAP_DOOR;
						sBPCost = BP_UNTRAP_DOOR;

						// OK, if we have no lock, show that!
						if ( pDoor == NULL )
						{
							// Open if it's not locked....
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
							ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
							break;
						}
						else
						{
							// Do we have a trap?
							if (pDoor->ubTrapID != NO_TRAP)
							{
								if ( AttemptToUntrapDoor( pSoldier, pDoor ) )
								{
									DoMercBattleSound(pSoldier, BATTLE_SOUND_COOL1);
									ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
									UpdateDoorPerceivedValue( pDoor );
									//fHandleDoor = TRUE;
								}
								else
								{
									ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0, FALSE);
									// Now just show on message bar
									HandleDoorTrap(*pSoldier, *pDoor);

									if (!( DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING ) )
									{
										// trap only happens once
										pDoor->ubTrapLevel = 0;
										pDoor->ubTrapID = NO_TRAP;
									}

									// Update perceived lock value
									UpdateDoorPerceivedValue( pDoor );
								}
							}
							else
							{
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_IS_NOT_TRAPPED_STR ] );
							}
						}
						break;


					case HANDLE_DOOR_LOCK:

						// Set costs for these
						sAPCost = AP_LOCK_DOOR;
						sBPCost = BP_LOCK_DOOR;

						// OK, if we have no lock, show that!
						if ( pDoor == NULL )
						{
							// Open if it's not locked....
							ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ] );
							ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
							break;
						}
						else
						{

							// it's locked....
							// Attempt to unlock....
							if ( AttemptToLockDoor( pSoldier, pDoor ) )
							{
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_HAS_BEEN_LOCKED_STR ] );
								ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
								UpdateDoorPerceivedValue( pDoor );
							}
							else
							{
								ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0, FALSE);
								// Do we have a quote for locked stuff?
								// Now just show on message bar
								ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ DOOR_NOT_PROPER_KEY_STR ], pSoldier->name) );

								// Update perceived lock value
								UpdateDoorPerceivedValue( pDoor );
							}
						}
						break;
				}
			}
		}
		else
		{
			// Set costs for these
			sAPCost = AP_OPEN_DOOR;
			sBPCost = BP_OPEN_DOOR;

			// Open if it's not locked....
			ChangeSoldierState(pSoldier, GetAnimStateForInteraction(*pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
			fHandleDoor = TRUE;
		}
	}

	if ( fHandleDoor )
	{
		if ( fDoor )
		{
			HandleDoorChangeFromGridNo( pSoldier, sGridNo, FALSE );
		}
		else
		{
			HandleStructChangeFromGridNo( pSoldier, sGridNo );
		}
	}

	// Deduct points!
	//if ( fDoor )
	{
		DeductPoints( pSoldier, sAPCost, sBPCost );
	}

	return( fHandleDoor );
}


static BOOLEAN HandleDoorsOpenClose(SOLDIERTYPE* pSoldier, INT16 sGridNo, STRUCTURE* pStructure, BOOLEAN fNoAnimations)
try
{
	INT32 cnt;
	BOOLEAN fOpenedGraphic = FALSE;
	BOOLEAN fDoAnimation = TRUE;
	STRUCTURE *pBaseStructure;

	pBaseStructure = FindBaseStructure( pStructure );
	if (!pBaseStructure)
	{
		return( FALSE );
	}

	LEVELNODE* const pNode = FindLevelNodeBasedOnStructure(pBaseStructure);

	// ATE: if we are about to swap, but have an animation playing here..... stop the animation....
	if ( ( pNode->uiFlags & LEVELNODE_ANIMATION ) )
	{
		if ( pNode->pAniTile != NULL )
		{
			if ( pNode->pAniTile->uiFlags & ANITILE_DOOR )
			{
				// ATE: No two doors can exist ( there can be only one )
				// Update value.. ie: prematurely end door animation
				// Update current frame...

				if ( pNode->pAniTile->uiFlags & ANITILE_FORWARD )
				{
					pNode->sCurrentFrame = pNode->pAniTile->sStartFrame + pNode->pAniTile->usNumFrames;
				}

				if ( pNode->pAniTile->uiFlags & ANITILE_BACKWARD )
				{
					pNode->sCurrentFrame = pNode->pAniTile->sStartFrame - pNode->pAniTile->usNumFrames;
				}

				pNode->sCurrentFrame = pNode->pAniTile->usNumFrames - 1;

				// Delete...
				DeleteAniTile( pNode->pAniTile );

				pNode->uiFlags &= ~( LEVELNODE_LASTDYNAMIC | LEVELNODE_UPDATESAVEBUFFERONCE );

				if ( GridNoOnScreen( pBaseStructure->sGridNo ) )
				{
					SetRenderFlags(RENDER_FLAG_FULL);
				}
			}
		}
	}


	// Check the graphic which is down!
	// Check for Open Door!
	cnt = 0;
	while( gOpenDoorList[ cnt ] != -1 )
	{
		// IF WE ARE A SHADOW TYPE
		if ( pNode->usIndex == gOpenDoorList[ cnt ] )
		{
			fOpenedGraphic = TRUE;
			break;
		}
		cnt++;
	}

	if ( !(pStructure->fFlags & STRUCTURE_OPEN) )
	{
		//ATE, the last parameter is the perceived value, I dont know what it is so could you please add the value?
		//ModifyDoorStatus( INT16 sGridNo, BOOLEAN fOpen, BOOLEAN fPercievedOpen )
		ModifyDoorStatus(sGridNo, TRUE, DONTSETDOORSTATUS);

		if ( gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_I )
		{
			DoPOWPathChecks();
		}

		if (pSoldier )
		{
			// OK, Are we a player merc or AI?
			if ( pSoldier->bTeam != OUR_TEAM )
			{
				// If an AI guy... do LOS check first....
				// If guy is visible... OR fading...
				if (pSoldier->bVisible == -1 && !AllMercsLookForDoor(sGridNo) && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
				{
					fDoAnimation = FALSE;
				}
			}
		}
		else
		{
			// door opening by action item... just do a LOS check
			if (!AllMercsLookForDoor(sGridNo))
			{
				fDoAnimation = FALSE;
			}
		}

		if ( fNoAnimations )
		{
			fDoAnimation = FALSE;
		}

		if ( fDoAnimation  )
		{
			// Update perceived value
			ModifyDoorStatus( sGridNo, DONTSETDOORSTATUS, TRUE );

			ANITILE_PARAMS AniParams;
			AniParams = ANITILE_PARAMS{};
			AniParams.uiFlags         = ANITILE_DOOR | ANITILE_EXISTINGTILE | (fOpenedGraphic ? ANITILE_FORWARD : ANITILE_BACKWARD);
			AniParams.ubLevelID       = ANI_STRUCT_LEVEL;
			AniParams.sStartFrame     = pNode->sCurrentFrame;
			AniParams.sDelay          = INTTILE_DOOR_OPENSPEED;
			AniParams.usTileIndex     = pNode->usIndex;
			AniParams.sGridNo         = sGridNo;
			AniParams.pGivenLevelNode = pNode;
			CreateAnimationTile(&AniParams);
		}

		if ( fDoAnimation && pSoldier && pSoldier->ubDoorOpeningNoise)
		{
			// ATE; Default to normal door...
			SoundID uiSoundID = SoundRange<DROPEN_1, DROPEN_3>();

			// OK, check if this door is sliding and is multi-tiled...
			if ( pStructure->fFlags & STRUCTURE_SLIDINGDOOR )
			{
				// Get database value...
				if ( pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1 )
				{
					// change sound ID
					uiSoundID = GARAGE_DOOR_OPEN;
				}
				else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_CLOTH )
				{
					// change sound ID
					uiSoundID = CURTAINS_OPEN;
				}
			}
			else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_METAL ||
					pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_THICKER_METAL ||
					pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_HEAVY_METAL)
			{
				// change sound ID
				uiSoundID = METAL_DOOR_OPEN;
			}

			// OK, We must know what sound to play, for now use same sound for all doors...
			PlayLocationJA2Sample(sGridNo, uiSoundID, MIDVOLUME, 1);
		}

	}
	else
	{
		//ATE, the last parameter is the perceived value, I dont know what it is so could you please add the value?
		//ModifyDoorStatus( INT16 sGridNo, BOOLEAN fOpen, BOOLEAN fInitiallyPercieveOpen )
		ModifyDoorStatus(sGridNo, FALSE, DONTSETDOORSTATUS);

		if (pSoldier )
		{
			// OK, Are we a player merc or AI?
			if ( pSoldier->bTeam != OUR_TEAM )
			{
				// If an AI guy... do LOS check first....
				// If guy is visible... OR fading...
				if (pSoldier->bVisible == -1 && !AllMercsLookForDoor(sGridNo) && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
				{
					fDoAnimation = FALSE;
				}
			}
		}
		else
		{
			// door opening by action item... just do a LOS check
			if (!AllMercsLookForDoor(sGridNo))
			{
				fDoAnimation = FALSE;
			}
		}

		if ( fNoAnimations )
		{
			fDoAnimation = FALSE;
		}

		if ( fDoAnimation )
		{
			// Update perceived value
			ModifyDoorStatus( sGridNo, DONTSETDOORSTATUS, FALSE );

			// ATE; Default to normal door...
			SoundID uiSoundID = SoundRange<DRCLOSE_1, DRCLOSE_2>();

			// OK, check if this door is sliding and is multi-tiled...
			if ( pStructure->fFlags & STRUCTURE_SLIDINGDOOR )
			{
				// Get database value...
				if ( pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1 )
				{
					// change sound ID
					uiSoundID = GARAGE_DOOR_CLOSE;
				}
				else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_CLOTH )
				{
					// change sound ID
					uiSoundID = CURTAINS_CLOSE;
				}
			}
			else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_METAL ||
				pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_THICKER_METAL ||
				pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_HEAVY_METAL)
			{
				// change sound ID
				uiSoundID = METAL_DOOR_CLOSE;
			}

			ANITILE_PARAMS AniParams;
			AniParams = ANITILE_PARAMS{};
			AniParams.uiFlags         = ANITILE_DOOR | ANITILE_EXISTINGTILE | (fOpenedGraphic ? ANITILE_BACKWARD : ANITILE_FORWARD);
			AniParams.ubLevelID       = ANI_STRUCT_LEVEL;
			AniParams.sStartFrame     = pNode->sCurrentFrame;
			AniParams.sDelay          = INTTILE_DOOR_OPENSPEED;
			AniParams.usTileIndex     = pNode->usIndex;
			AniParams.sGridNo         = sGridNo;
			AniParams.pGivenLevelNode = pNode;
			AniParams.ubKeyFrame1     = pNode->sCurrentFrame + (fOpenedGraphic ? -2 : 2);
			AniParams.uiKeyFrame1Code = ANI_KEYFRAME_DO_SOUND;
			AniParams.v.sound         = uiSoundID;
			CreateAnimationTile( &AniParams );
		}

	}

	if ( fDoAnimation )
	{
		gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
		// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
		InvalidateWorldRedundency( );

		if ( GridNoOnScreen( sGridNo ) )
		{
			SetRenderFlags(RENDER_FLAG_FULL);
		}
	}

	return( fDoAnimation );
}
catch (...) { return FALSE; }


void SetDoorString(INT16 const sGridNo)
{
	if (GetIntTileLocationText().empty())
	{
		SetIntTileLocationText(TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);
		DOOR const* const d = FindDoorInfoAtGridNo(sGridNo);
		if (d != NULL)
		{
			ST::string state;
			if (d->bPerceivedTrapped == DOOR_PERCEIVED_TRAPPED)
			{
				state = TacticalStr[DOOR_TRAPPED_MOUSE_DESCRIPTION];
			}
			else switch (d->bPerceivedLocked)
			{
				case DOOR_PERCEIVED_UNKNOWN:
					break;
				case DOOR_PERCEIVED_LOCKED:
					state = TacticalStr[DOOR_LOCKED_MOUSE_DESCRIPTION];
					break;
				case DOOR_PERCEIVED_UNLOCKED:
					state = TacticalStr[DOOR_UNLOCKED_MOUSE_DESCRIPTION];
					break;
				case DOOR_PERCEIVED_BROKEN:
					state = TacticalStr[DOOR_BROKEN_MOUSE_DESCRIPTION];
					break;
			}
			if (!state.empty()) SetIntTileLocation2Text(state);
		}
	}

	// ATE: If here, we try to say, opened or closed...
	if (GetIntTileLocation2Text().empty())
	{
	if(isGermanVersion())
	{
		SetIntTileLocation2Text(TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);
	}

		// Try to get doors status here...
		bool                     open;
		DOOR_STATUS const* const ds = GetDoorStatus(sGridNo);
		if (ds == NULL || ds->ubFlags & DOOR_PERCEIVED_NOTSET)
		{
			// OK, get status based on graphic.....
			STRUCTURE const* const structure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
			if (!structure)
				return;

			open = (structure->fFlags & STRUCTURE_OPEN) != 0;
		}
		else
		{
			// Use percived value
			open = (ds->ubFlags & DOOR_PERCEIVED_OPEN) != 0;
		}
		ST::string state = open ?
			pMessageStrings[MSG_OPENED] : pMessageStrings[MSG_CLOSED];
	if(isGermanVersion())
	{
		SetIntTileLocationText(state);
	}
	else
	{
		SetIntTileLocation2Text(state);
	}
	}
}
