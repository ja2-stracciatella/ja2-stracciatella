#ifndef _DOORS_H
#define _DOORS_H

#include "JA2Types.h"

enum HandleDoor
{
	HANDLE_DOOR_DENIED   = 0,
	HANDLE_DOOR_OPEN            = 1, // standard mode for an open/close interaction, takes 3 APs in vanilla
	HANDLE_DOOR_OPEN_DIAG_SHORT = 2, // interacting from a diagonally located tile, takes more APs
	HANDLE_DOOR_OPEN_DIAG_LONG  = 3, // interacting from a diagonally located tile, takes even more APs
	HANDLE_DOOR_EXAMINE  = 4,
	HANDLE_DOOR_LOCKPICK = 5,
	HANDLE_DOOR_FORCE    = 6,
	HANDLE_DOOR_LOCK     = 7,
	HANDLE_DOOR_UNLOCK   = 8,
	HANDLE_DOOR_EXPLODE  = 9,
	HANDLE_DOOR_UNTRAP   = 10,
	HANDLE_DOOR_CROWBAR  = 11
};


BOOLEAN HandleOpenableStruct( SOLDIERTYPE *pSoldier, INT16 sGridNo, STRUCTURE *pStructure );

void InteractWithOpenableStruct(SOLDIERTYPE&, STRUCTURE&, UINT8 direction);

void InteractWithClosedDoor(SOLDIERTYPE*, HandleDoor);

void SetDoorString( INT16 sGridNo );

void HandleDoorChangeFromGridNo( SOLDIERTYPE *pSoldier, INT16 sGridNo, BOOLEAN fNoAnimations );

UINT16 GetAnimStateForInteraction(SOLDIERTYPE const&, BOOLEAN door, UINT16 anim_state);

#endif
