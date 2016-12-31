#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

// header file to track the information a player 'knows' about a sector, which may in fact not be true
#include "Strategic_Movement.h"


// build main facilities strings for sector
void GetSectorFacilitiesFlags(INT16 sMapX, INT16 sMapY, wchar_t* sFacilitiesString, size_t Length);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(INT16 sMapX, INT16 sMapY, INT8 bMapZ);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled( INT16 sMapX, INT16 sMapY, INT8 bMapZ, BOOLEAN fContested );

#endif
