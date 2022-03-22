#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

// header file to track the information a player 'knows' about a sector, which may in fact not be true
#include "Strategic_Movement.h"

#include <string_theory/string>


// build main facilities strings for sector
ST::string GetSectorFacilitiesFlags(const SGPSector& sector);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(const SGPSector& sector);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled( INT16 sMapX, INT16 sMapY, INT8 bMapZ, BOOLEAN fContested );

#endif
