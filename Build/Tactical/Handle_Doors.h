#ifndef _DOORS_H
#define _DOORS_H

enum HandleDoor
{
	HANDLE_DOOR_OPEN     = 1,
	HANDLE_DOOR_EXAMINE  = 2,
	HANDLE_DOOR_LOCKPICK = 3,
	HANDLE_DOOR_FORCE    = 4,
	HANDLE_DOOR_LOCK     = 5,
	HANDLE_DOOR_UNLOCK   = 6,
	HANDLE_DOOR_EXPLODE  = 7,
	HANDLE_DOOR_UNTRAP   = 8,
	HANDLE_DOOR_CROWBAR  = 9
};


BOOLEAN HandleOpenableStruct( SOLDIERTYPE *pSoldier, INT16 sGridNo, STRUCTURE *pStructure );

void InteractWithOpenableStruct( SOLDIERTYPE *pSoldier, STRUCTURE *pStructure, UINT8 ubDirection, BOOLEAN fDoor );

void InteractWithClosedDoor(SOLDIERTYPE*, HandleDoor);

void SetDoorString( INT16 sGridNo );

void HandleDoorChangeFromGridNo( SOLDIERTYPE *pSoldier, INT16 sGridNo, BOOLEAN fNoAnimations );

UINT16 GetAnimStateForInteraction(SOLDIERTYPE const&, BOOLEAN door, UINT16 anim_state);

#endif
