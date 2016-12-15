#ifndef WORLD_DAT_H
#define WORLD_DAT_H

#include "sgp/Types.h"
#include "TileDat.h"
#include "World_Tileset_Enums.h"

typedef void (*TILESET_CALLBACK)( void );


struct TILESET
{
	wchar_t							zName[ 32 ];
	CHAR8								TileSurfaceFilenames[ NUMBEROFTILETYPES ][32];
	UINT8								ubAmbientID;
	TILESET_CALLBACK		MovementCostFnc;
};



extern TILESET	 gTilesets[ NUM_TILESETS ];


void InitEngineTilesets(void);


// THESE FUNCTIONS WILL SET TERRAIN VALUES - CALL ONE FOR EACH TILESET
void SetTilesetOneTerrainValues(void);

#endif
