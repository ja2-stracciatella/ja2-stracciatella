#ifdef PRECOMPILEDHEADERS
	#include "TileEngine All.h"
#else
	#include "tiledef.h"
	#include "worlddef.h"
	#include <stdio.h>
	#include <string.h>
	#include "wcheck.h"
	#include "stdlib.h"
	#include "time.h"
	#include "video.h"
	#include "debug.h"
	#include "worldman.h"
	#include "gameloop.h"
	#include "edit_sys.h"
	#include "pathai.h"
	#include "tile surface.h"
#endif

//#include "editscreen.h"


// GLobals
TILE_ELEMENT		gTileDatabase[ NUMBEROFTILES ];
UINT16					gTileDatabaseSize;
UINT16					gusNumAnimatedTiles = 0;
UINT16					gusAnimatedTiles[ MAX_ANIMATED_TILES ];



UINT16						gTileTypeStartIndex[ NUMBEROFTILETYPES ];

UINT8 gubEncryptionArray2[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ] =
{
    {
    81,168,102,49,61,70,172,
    127,7,148,115,179,10,117,
    253,35,30,218,76,63,116,
    210,241,65,169,157,4,9,
    29,205,160,111,41,213,193,
    190,86,19,207,133,25,190,
    187,131,66,196,253,227,163
    },

    {
    236,81,128,26,96,137,92,
    120,243,71,33,141,55,2,
    185,185,187,33,230,72,146,
    143,226,21,110,155,226,150,
    111,124,165,242,4,76,201,
    34,223,227,29,202,119,86,
    172,219,8,121,183,148,142
    },

    {
    73,81,249,189,120,220,96,
    79,28,37,30,199,170,56,
    1,10,70,215,151,190,146,
    156,233,11,251,146,195,134,
    188,140,23,233,137,204,150,
    116,36,97,40,119,204,194,
    55,9,193,18,143,76,209
    },

    {
    143,200,80,50,154,231,247,
    140,225,72,226,136,2,99,
    209,182,82,146,128,49,3,
    98,101,196,95,136,85,3,
    81,37,131,9,251,79,209,
    101,128,157,238,229,205,178,
    49,161,141,101,175,48,2
    },

    {
    171,145,70,2,204,252,166,
    194,2,182,182,189,17,205,
    141,80,12,104,215,96,174,
    11,106,181,150,113,213,71,
    209,89,219,35,229,63,18,
    64,237,140,153,224,215,31,
    48,198,108,180,139,233,39
    },

    {
    31,60,240,131,203,6,32,
    206,137,249,244,36,8,39,
    38,142,11,77,76,18,200,
    76,210,138,106,198,130,22,
    37,178,76,18,233,77,149,
    77,28,47,186,126,72,234,
    254,158,97,10,162,58,93
    },

    {
    181,91,80,208,248,218,224,
    90,219,176,181,120,63,35,
    132,167,206,85,6,76,223,
    90,15,100,142,121,79,111,
    255,88,196,134,19,72,133,
    40,95,98,232,153,217,182,
    186,109,241,66,128,26,140
    },

    {
    225,112,22,157,253,52,123,
    209,31,7,165,18,48,83,
    190,53,154,87,188,199,251,
    248,185,48,164,93,44,22,
    165,77,239,88,148,21,155,
    115,203,231,20,170,83,100,
    222,204,57,83,39,241,126
    },

    {
    51,228,1,96,176,54,89,
    49,157,66,125,113,124,187,
    15,110,70,101,246,16,108,
    156,20,32,200,96,70,170,
    120,198,42,199,79,93,100,
    115,246,113,213,187,66,169,
    129,241,230,251,145,23,29
    },

    {
    101,67,67,78,108,22,58,
    97,212,30,230,190,142,195,
    243,169,75,210,61,248,180,
    47,75,218,191,5,154,172,
    19,204,224,123,244,251,1,
    94,192,187,249,33,79,120,
    115,109,88,173,40,7,61
    },

    {
    178,239,46,52,245,141,192,
    77,114,7,54,78,179,195,
    12,213,79,16,184,192,27,
    100,195,74,5,1,15,226,
    223,50,245,39,244,230,85,
    8,196,217,18,128,187,251,
    52,225,65,96,62,49,239
    },

    {
    94,190,122,81,36,204,171,
    249,104,120,214,89,133,203,
    103,19,163,117,53,59,48,
    182,240,91,101,160,234,121,
    1,78,151,26,3,28,228,
    130,138,137,188,21,167,198,
    197,34,63,165,195,11,206
    },

    {
    220,164,4,55,94,159,83,
    86,186,109,101,150,60,4,
    146,58,195,221,221,148,11,
    50,155,232,95,39,71,37,
    106,104,102,59,34,41,206,
    61,129,242,45,11,12,85,
    27,184,210,214,165,106,238
    },

    {
    44,151,227,47,39,93,122,
    237,91,225,164,198,235,53,
    226,36,124,86,57,94,185,
    251,231,101,120,178,168,162,
    218,38,34,33,126,235,196,
    151,30,175,212,103,170,17,
    208,48,75,16,231,10,66
    },

    {
    90,201,253,44,89,120,222,
    145,82,80,6,63,114,22,
    37,106,33,40,177,4,112,
    152,9,70,250,103,229,235,
    22,147,65,5,164,55,48,
    233,103,112,216,69,34,63,
    31,159,108,38,14,111,132
    },

    {
    170,177,43,7,140,86,71,
    207,205,235,155,239,88,129,
    53,6,111,139,249,208,172,
    252,34,88,175,213,251,129,
    86,54,242,107,142,144,79,
    96,161,29,171,196,16,17,
    143,205,207,201,47,88,10
    },

    {
    112,203,199,48,59,153,46,
    130,221,189,166,213,63,67,
    50,190,21,148,242,238,113,
    94,28,228,234,100,103,187,
    95,53,124,69,6,55,6,
    39,204,11,231,64,176,183,
    86,248,21,152,62,205,200
    },

    {
    133,189,180,176,46,27,167,
    213,33,253,7,209,112,92,
    8,60,122,206,229,50,184,
    207,25,142,34,149,64,234,
    16,252,247,255,22,70,90,
    126,212,16,93,248,50,89,
    194,248,244,230,224,203,62
    },

    {
    234,231,52,206,57,230,130,
    158,240,79,58,175,226,31,
    213,160,154,113,199,241,84,
    178,66,108,201,41,240,188,
    24,9,50,145,172,208,226,
    100,127,42,93,55,123,245,
    197,102,110,206,48,234,81
    },

    {
    176,193,13,70,253,101,15,
    39,156,34,168,239,92,185,
    235,239,109,151,115,6,12,
    184,92,210,40,152,12,36,
    57,183,30,136,247,232,152,
    77,59,225,190,208,165,59,
    133,118,100,111,45,20,177
    },

    {
    242,224,56,94,247,232,26,
    5,5,92,145,57,226,205,
    229,79,2,235,214,16,252,
    135,176,247,51,4,190,78,
    45,155,6,184,220,121,136,
    151,66,29,253,210,166,255,
    75,82,234,246,152,230,26
    },

    {
    71,92,199,196,224,54,108,
    71,191,224,240,145,231,54,
    198,135,241,115,219,139,162,
    178,253,95,131,237,175,212,
    251,85,23,132,211,92,89,
    14,166,88,5,254,58,254,
    204,5,41,160,72,105,230
    },

    {
    122,240,140,138,243,30,90,
    95,32,36,108,168,91,94,
    223,161,93,96,118,32,153,
    235,41,222,65,8,181,221,
    196,207,70,218,41,74,198,
    79,6,240,9,240,142,41,
    69,48,113,102,15,168,239
    },

    {
    33,100,248,1,115,220,254,
    216,113,156,238,175,27,36,
    61,20,255,131,209,181,91,
    40,126,238,227,186,46,228,
    253,64,201,169,138,5,71,
    30,20,119,212,255,127,121,
    170,240,13,84,96,212,210
    },

    {
    213,91,87,107,184,54,252,
    30,240,165,139,29,230,205,
    42,49,78,136,10,113,160,
    132,246,82,211,200,164,178,
    112,128,15,43,150,157,197,
    88,7,48,126,75,210,2,
    196,228,191,100,25,80,254
    },

    {
    161,100,85,71,23,7,176,
    250,104,108,62,88,195,110,
    27,102,177,102,127,163,105,
    65,133,21,24,117,155,208,
    39,37,206,154,225,85,176,
    161,128,129,8,6,18,114,
    12,53,53,165,90,177,26
    },

    {
    36,101,106,199,26,144,50,
    33,13,177,101,149,6,56,
    112,251,141,157,46,152,191,
    112,214,180,104,160,119,202,
    112,75,167,190,116,185,126,
    27,229,214,110,194,128,191,
    19,24,123,193,148,15,69
    },

    {
    94,179,36,34,168,18,167,
    136,168,214,94,144,245,66,
    241,83,82,74,25,138,106,
    213,7,78,133,181,254,75,
    73,230,160,60,93,65,204,
    19,24,42,169,150,246,86,
    182,211,11,68,108,69,44
    },

    {
    127,97,243,9,60,97,125,
    234,114,130,69,83,11,157,
    186,44,46,37,4,205,85,
    226,218,134,60,127,11,227,
    55,80,85,109,26,120,25,
    19,60,173,155,255,182,196,
    64,12,112,91,145,131,23
    },

    {
    166,108,127,147,227,40,125,
    27,76,18,129,108,39,24,
    100,92,205,245,147,59,47,
    190,251,54,204,28,72,76,
    151,24,71,228,181,206,140,
    172,161,216,38,201,154,203,
    141,252,226,169,135,208,183
    },

    {
    143,63,98,145,133,62,237,
    139,191,134,154,119,47,128,
    196,154,179,252,48,125,206,
    245,106,207,37,87,203,218,
    175,115,73,116,243,111,167,
    231,22,81,34,216,26,234,
    36,136,62,97,46,137,172
    },

    {
    125,118,61,116,135,101,54,
    15,215,129,210,84,238,226,
    43,56,40,197,174,30,226,
    95,117,34,65,115,192,53,
    67,222,155,83,103,157,18,
    188,158,107,3,133,177,144,
    242,76,231,253,233,120,19
    },

    {
    65,199,211,202,123,9,216,
    255,223,125,141,168,222,211,
    71,184,143,146,32,40,117,
    82,140,45,50,203,212,115,
    70,166,219,132,100,127,245,
    11,146,38,66,68,135,130,
    71,59,123,29,51,25,249
    },

    {
    190,161,56,194,42,46,58,
    104,89,177,209,173,64,141,
    117,68,136,202,201,9,67,
    112,209,78,255,233,204,77,
    72,199,213,85,252,204,10,
    221,117,125,115,252,250,46,
    101,111,125,194,139,186,255
    },

    {
    31,86,139,193,121,21,227,
    136,133,87,49,18,190,230,
    224,29,177,1,149,9,151,
    24,129,123,31,191,15,12,
    186,16,109,103,75,43,16,
    233,124,37,188,28,178,253,
    78,27,215,234,165,87,234
    },

    {
    170,4,161,157,18,78,11,
    223,148,86,130,184,244,199,
    171,43,229,209,235,160,189,
    23,30,145,9,19,238,221,
    41,113,74,156,184,137,226,
    186,241,135,67,86,193,87,
    95,11,215,251,31,39,80
    },

    {
    85,223,21,67,248,93,201,
    234,55,255,161,143,221,252,
    110,10,206,90,36,240,21,
    39,237,78,109,10,6,248,
    7,125,44,88,104,198,17,
    195,47,15,140,61,184,167,
    251,182,72,224,147,159,196
    },

    {
    123,19,239,168,30,154,104,
    60,207,7,69,172,234,176,
    23,31,137,211,242,79,208,
    145,194,222,40,205,220,218,
    202,197,167,163,162,203,119,
    188,250,215,100,172,230,115,
    79,119,241,177,204,213,247
    },

    {
    164,47,183,166,25,101,55,
    173,255,232,132,251,73,117,
    235,22,152,184,146,245,171,
    192,242,110,184,81,188,135,
    183,224,50,201,210,124,141,
    254,235,54,153,73,133,105,
    251,27,228,131,103,255,205
    },

    {
    141,53,177,160,37,136,109,
    76,46,150,5,207,226,32,
    160,180,136,191,175,88,111,
    36,245,86,149,192,56,237,
    119,37,117,196,117,115,54,
    203,93,49,208,170,113,233,
    46,143,140,211,5,170,166
    },

    {
    2,245,56,116,61,150,114,
    76,28,201,61,29,126,115,
    226,153,233,246,200,95,246,
    112,88,77,97,48,22,186,
    203,102,27,213,211,119,235,
    204,147,244,229,21,29,175,
    52,103,21,222,146,203,247
    },

    {
    203,187,75,44,10,228,157,
    113,156,9,120,202,29,117,
    184,122,64,90,44,31,194,
    191,30,155,50,190,115,208,
    130,69,132,201,246,81,100,
    108,67,226,228,85,112,150,
    243,222,107,49,158,2,199
    },

    {
    243,107,29,155,25,62,54,
    254,77,132,226,20,121,227,
    78,33,189,27,153,172,240,
    78,17,13,117,133,92,119,
    205,123,66,233,177,141,20,
    2,21,244,85,47,218,7,
    194,47,175,180,65,92,188
    },

    {
    3,223,186,179,171,112,64,
    226,172,223,83,208,235,215,
    235,20,187,212,96,218,104,
    41,66,233,141,50,89,115,
    102,35,34,147,219,175,203,
    216,121,7,60,166,18,77,
    246,202,53,41,124,33,72
    },

    {
    115,8,155,105,168,30,132,
    213,57,253,89,18,62,83,
    13,138,197,40,21,218,47,
    156,13,133,209,66,82,67,
    99,176,107,172,80,52,146,
    108,148,19,61,193,41,114,
    250,121,164,57,125,29,35
    },

    {
    252,37,152,15,100,236,254,
    226,253,7,172,123,6,10,
    139,17,7,254,146,114,216,
    45,95,154,241,252,4,47,
    134,193,201,19,97,207,39,
    144,254,165,157,60,218,144,
    173,48,97,157,173,165,139
    },

    {
    43,160,148,141,246,225,109,
    162,9,196,127,84,62,238,
    41,82,224,161,103,65,28,
    169,227,113,157,43,7,17,
    78,6,67,179,102,7,16,
    186,197,164,203,143,211,114,
    255,34,222,17,63,184,47
    },

    {
    202,17,101,190,148,37,65,
    227,178,78,200,202,174,105,
    90,13,220,198,78,74,44,
    37,172,23,111,225,219,23,
    223,119,96,149,12,206,14,
    131,243,148,179,37,120,26,
    168,77,232,223,65,125,224
    },

    {
    95,160,190,37,50,215,151,
    196,31,209,227,48,62,1,
    249,123,173,73,232,137,16,
    25,39,88,73,51,149,31,
    178,19,34,120,74,112,230,
    97,243,213,31,34,121,189,
    200,255,68,57,75,208,129
    },

    {
    135,244,60,186,169,184,217,
    128,88,234,225,59,98,108,
    180,189,228,11,222,179,81,
    129,225,122,192,210,117,133,
    142,124,112,228,166,55,29,
    3,133,178,40,47,107,31,
    237,22,170,212,75,186,142
    },

    {
    254,191,1,222,153,4,115,
    100,149,222,68,112,139,102,
    61,231,195,98,156,181,242,
    24,137,44,136,34,43,76,
    219,155,8,100,28,81,181,
    252,75,50,126,154,46,177,
    11,53,126,99,180,248,57
    },

    {
    109,132,42,96,40,141,189,
    8,138,168,147,13,172,54,
    196,69,7,241,14,128,187,
    124,187,63,140,172,27,97,
    195,63,235,191,196,254,187,
    234,123,1,174,6,88,6,
    177,247,45,156,120,226,125
    },

    {
    30,95,46,155,25,137,245,
    52,68,72,74,137,152,60,
    3,26,152,42,137,223,113,
    244,114,229,1,192,53,115,
    90,174,232,119,234,38,77,
    114,195,46,6,151,52,203,
    85,96,69,205,58,68,39
    },

    {
    30,59,201,53,236,197,180,
    160,196,103,128,88,254,209,
    225,82,124,227,135,183,59,
    172,203,39,154,168,155,144,
    235,35,68,198,41,8,87,
    105,92,255,110,51,167,55,
    248,69,103,157,208,146,105
    },

    {
    249,205,245,168,172,15,16,
    148,112,53,141,74,5,64,
    218,194,253,75,243,113,168,
    228,80,59,85,44,153,81,
    216,126,55,215,165,220,72,
    217,83,199,41,128,227,208,
    24,3,173,137,18,75,53
    },

    {
    21,130,191,209,19,213,58,
    123,18,147,139,147,81,14,
    156,255,245,64,150,115,131,
    244,12,61,185,211,155,114,
    151,96,11,203,200,138,71,
    17,26,234,71,106,73,60,
    76,106,210,50,115,45,2
    },

    {
    55,17,107,248,62,204,196,
    224,45,253,50,10,2,51,
    233,34,73,107,137,68,253,
    235,235,24,99,52,125,184,
    3,3,88,146,80,226,29,
    5,25,66,196,226,87,125,
    83,169,124,218,190,135,197
    },
};

// These values coorespond to TerrainTypeDefines order
UINT8		gTileTypeMovementCost[ NUM_TERRAIN_TYPES ] =
{
	TRAVELCOST_FLAT,						// NO_TERRAIN
	TRAVELCOST_FLAT,						// FLAT GROUND
	TRAVELCOST_FLATFLOOR,				// FLAT FLOOR
	TRAVELCOST_PAVEDROAD,				// PAVED ROAD
	TRAVELCOST_DIRTROAD,				// DIRT ROAD
	TRAVELCOST_FLAT,						// LOW_GRASS
	TRAVELCOST_FLAT,						// HIGH GRASS
	TRAVELCOST_TRAINTRACKS,			// TRAIN TRACKS
	TRAVELCOST_SHORE,						// LOW WATER
	TRAVELCOST_SHORE,						// MED WATER
	TRAVELCOST_SHORE,						// DEEP WATER
};


void CreateTileDatabase( )
{
	UINT32					cnt1, cnt2;
	UINT8						ubLoop;
	UINT32					NumRegions;
	PTILE_IMAGERY   TileSurf;
	TILE_ELEMENT		TileElement;

	// Loop through all surfaces and tiles and build database
	for( cnt1 = 0; cnt1 < NUMBEROFTILETYPES; cnt1++ )
	{
		// Get number of regions
		TileSurf = gTileSurfaceArray[ cnt1 ];

		if ( TileSurf != NULL )
		{

			// Build start index list
	    gTileTypeStartIndex[ cnt1 ] = (UINT16)gTileDatabaseSize;

			NumRegions = TileSurf->vo->usNumberOfObjects;

			// Check for overflow
			if ( NumRegions > gNumTilesPerType[ cnt1 ] )
			{
				#ifdef JA2EDITOR
					// Display warning
					gfWarning = (UINT8)cnt1;
				#endif

				// Cutof
				NumRegions = gNumTilesPerType[ cnt1 ];
			}

			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Type: %s Size: %d Index: %d", gTileSurfaceName[cnt1], gNumTilesPerType[ cnt1 ], gTileDatabaseSize ) );

			for( cnt2 = 0; cnt2 < NumRegions; cnt2++ )
			{

				memset( &TileElement, 0, sizeof( TileElement ) );
				TileElement.usRegionIndex = (UINT16)cnt2;
				TileElement.hTileSurface	= TileSurf->vo;
				TileElement.sBuddyNum			= -1;

				// Check for multi-z stuff
				if ( TileSurf->vo->ppZStripInfo != NULL )
				{
					// Only do this if we are within the # of video objects
					if ( cnt2 < TileSurf->vo->usNumberOfObjects )
					{
						if ( TileSurf->vo->ppZStripInfo[ cnt2 ] != NULL )
						{
							TileElement.uiFlags |= MULTI_Z_TILE;
						}
					}
					else
					{
						//Ate test to see if problems is gone
						int i = 0;
					}
				}

				// Structure database stuff!
				if (TileSurf->pStructureFileRef != NULL && TileSurf->pStructureFileRef->pubStructureData != NULL)
				{
					if (TileSurf->pStructureFileRef->pDBStructureRef[cnt2].pDBStructure != NULL)
					{
						TileElement.pDBStructureRef	= &(TileSurf->pStructureFileRef->pDBStructureRef[cnt2]);

						if ( TileElement.pDBStructureRef->pDBStructure->fFlags & STRUCTURE_HIDDEN )
						{
							// ATE: These are ignored!
							//TileElement.uiFlags |= HIDDEN_TILE;
						}
					}
				}

				TileElement.fType							= (UINT16)TileSurf->fType;
				TileElement.ubTerrainID				= TileSurf->ubTerrainID;
				TileElement.usWallOrientation = NO_ORIENTATION;

				if (TileSurf->pAuxData != NULL)
				{
					if (TileSurf->pAuxData[cnt2].fFlags & AUX_FULL_TILE)
					{
						TileElement.ubFullTile = 1;
					}
					if (TileSurf->pAuxData[cnt2].fFlags & AUX_ANIMATED_TILE)
					{
						// Allocate Animated tile data
						AllocateAnimTileData( &TileElement, TileSurf->pAuxData[cnt2].ubNumberOfFrames );

						// Set values into tile element
						TileElement.pAnimData->bCurrentFrame = TileSurf->pAuxData[cnt2].ubCurrentFrame;
						for (ubLoop = 0; ubLoop < TileElement.pAnimData->ubNumFrames; ubLoop++)
						{
							TileElement.pAnimData->pusFrames[ ubLoop ] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
						}

/*
						for (ubLoop = TileElement.pAnimData->bCurrentFrame; ubLoop < TileElement.pAnimData->ubNumFrames; ubLoop++)
						{
							TileElement.pAnimData->pusFrames[ ubLoop ] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
						}
						for (ubLoop = 0; ubLoop < TileElement.pAnimData->bCurrentFrame; ubLoop++)
						{
							TileElement.pAnimData->pusFrames[ ubLoop ] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
						}

*/

						// set into animation controller array
						gusAnimatedTiles[ gusNumAnimatedTiles ] = gTileDatabaseSize;
						gusNumAnimatedTiles++;

						Assert( gusNumAnimatedTiles <= MAX_ANIMATED_TILES );

						TileElement.uiFlags				 |= ANIMATED_TILE;
					}
					TileElement.usWallOrientation = TileSurf->pAuxData[cnt2].ubWallOrientation;
					if (TileSurf->pAuxData[cnt2].ubNumberOfTiles > 0)
					{
						TileElement.ubNumberOfTiles = TileSurf->pAuxData[cnt2].ubNumberOfTiles;
						TileElement.pTileLocData = TileSurf->pTileLocData + TileSurf->pAuxData[cnt2].usTileLocIndex;
					}
				}

				SetSpecificDatabaseValues( (UINT16)cnt1, gTileDatabaseSize, &TileElement, TileSurf->bRaisedObjectType );

				gTileDatabase[ gTileDatabaseSize ] = TileElement;
				gTileDatabaseSize++;
			}

			// Check if data matches what should be there
			if ( NumRegions < gNumTilesPerType[ cnt1 ] )
			{
				#ifdef JA2EDITOR
					// Display warning here
					gfWarning = (UINT8)cnt1;
				#endif

				// Do underflows here
				for ( cnt2 = NumRegions; cnt2 < gNumTilesPerType[ cnt1 ]; cnt2++ )
				{
					memset( &TileElement, 0, sizeof( TileElement ) );
					TileElement.usRegionIndex = 0;
					TileElement.hTileSurface	= TileSurf->vo;
					TileElement.fType					= (UINT16)TileSurf->fType;
					TileElement.ubFullTile    = FALSE;
					TileElement.sOffsetHeight = 0;
					TileElement.ubFullTile		= 0;
          TileElement.uiFlags       |= UNDERFLOW_FILLER;

					gTileDatabase[ gTileDatabaseSize ] = TileElement;
					gTileDatabaseSize++;
				}

			}

		}
	}

	//Calculate mem usgae
	gSurfaceMemUsage = guiMemTotal - gSurfaceMemUsage;
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Sizes: %d vs %d", gTileDatabaseSize, NUMBEROFTILES ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Types: %d", NUMBEROFTILETYPES ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Item Mem:		%d", gTileDatabaseSize * sizeof( TILE_ELEMENT ) ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Item Total Mem:		%d", gSurfaceMemUsage ) );

}

void DeallocateTileDatabase( )
{
	INT32 cnt;

	for( cnt = 0; cnt < NUMBEROFTILES; cnt++ )
	{
		// Check if an existing set of animated tiles are in place, remove if found
		if ( gTileDatabase[ cnt ].pAnimData != NULL )
		{
			FreeAnimTileData( &gTileDatabase[ cnt ] );
		}
	}

	gTileDatabaseSize = 0;
	gSurfaceMemUsage = 0;
	gusNumAnimatedTiles = 0;

}


BOOLEAN GetLandHeadType( INT32 iMapIndex, UINT32 *puiType )
{
 UINT16					usIndex;

 Assert( puiType != NULL );

 CHECKF( gpWorldLevelData[ iMapIndex ].pLandHead != NULL );

 usIndex = gpWorldLevelData[ iMapIndex ].pLandHead->usIndex;

 GetTileType( usIndex, puiType );

 return( TRUE );

}

BOOLEAN SetLandIndex( INT32 iMapIndex, UINT16 usIndex, UINT32 uiNewType, BOOLEAN fDelete )
{
	UINT16	usTempIndex;
	UINT8		ubLastHighLevel=0;

	if ( fDelete )
	{
		RemoveLand( iMapIndex, usIndex );
		return( TRUE );
	}

	if ( AnyHeigherLand( iMapIndex, uiNewType, &ubLastHighLevel ) )
	{
			// Check if type exists and get it's index if so
			if ( TypeExistsInLandLayer( iMapIndex, uiNewType, &usTempIndex ) )
			{
				// Replace with new index
				return( ReplaceLandIndex( iMapIndex, usTempIndex, usIndex ) );
			}
			else
			{
				return( InsertLandIndexAtLevel( iMapIndex, usIndex, (UINT8)(ubLastHighLevel+1) ) );
			}

	}
	else
	{

		// Check if type exists and get it's index if so
		if ( TypeExistsInLandLayer( iMapIndex, uiNewType, &usTempIndex ) )
		{
			// Replace with new index
			return( ReplaceLandIndex( iMapIndex, usTempIndex, usIndex ) );
		}
		else
		{
			// Otherwise, add to head
			return( AddLandToHead( iMapIndex, usIndex ) );
		}
	}

}


BOOLEAN SetLandIndexWithRadius( INT32 iMapIndex, UINT16 usIndex, UINT32 uiNewType, UINT8 ubRadius, BOOLEAN fReplace )
{
	UINT16				usTempIndex;
	INT16					sTop, sBottom;
	INT16					sLeft, sRight;
	INT16					cnt1, cnt2;
	INT32				  iNewIndex;
	BOOLEAN				fDoPaste = FALSE;
	INT32					leftmost;
	//BOOLEAN				fNewCommand;
	UINT16				Dummy;

	// Determine start end end indicies and num rows
	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//fNewCommand = fFirstDrawMode; //NEW_UNDO_COMMAND;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{

		leftmost = ( ( iMapIndex + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			iNewIndex = iMapIndex + ( WORLD_COLS * cnt1 ) + cnt2;

			if ( iNewIndex >=0 && iNewIndex < WORLD_MAX &&
				   iNewIndex >= leftmost && iNewIndex < ( leftmost + WORLD_COLS ) )
			{

				if ( fReplace )
				{
					fDoPaste = TRUE;
				}
				else
				{
					if ( !TypeExistsInLandLayer( iNewIndex, uiNewType, &usTempIndex ) )
					{
						fDoPaste = TRUE;
					}
				}

				if ( fDoPaste && ((uiNewType >= FIRSTFLOOR && uiNewType <= LASTFLOOR) ||
												 ((uiNewType < FIRSTFLOOR || uiNewType > LASTFLOOR) &&
												 !TypeRangeExistsInLandLayer( iNewIndex, FIRSTFLOOR, LASTFLOOR, &Dummy ))) )
				{
					SetLandIndex( iNewIndex, usIndex, uiNewType, FALSE );
				}
			}
		}
	}

	return( TRUE );
}


BOOLEAN GetTypeLandLevel( UINT32 iMapIndex, UINT32 uiNewType, UINT8 *pubLevel )
{
	UINT8					level = 0;
	LEVELNODE			*pLand;
	UINT32				fTileType=0;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	while( pLand != NULL )
	{

		if ( pLand->usIndex != NO_TILE )
		{
			GetTileType( pLand->usIndex, &fTileType );

			if ( fTileType == uiNewType )
			{
				*pubLevel = level;
				return( TRUE );
			}

		}

		level++;
		pLand = pLand->pNext;
	}

	return( FALSE );

}


UINT8		GetLandLevelDepth( UINT32 iMapIndex )
{
	UINT8					level = 0;
	LEVELNODE		*pLand;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	while( pLand != NULL )
	{
		level++;
		pLand = pLand->pNext;
	}

	return( level );

}

BOOLEAN GetSubIndexFromTileIndex( UINT16 usTileIndex, UINT16 *pusSubIndex )
{
	UINT32 uiType=0;
	if( GetTileType( usTileIndex, &uiType ) )
	{
		*pusSubIndex = usTileIndex - gTileTypeStartIndex[ uiType ] + 1;
		return TRUE;
	}
	return FALSE;
}

BOOLEAN GetTypeSubIndexFromTileIndex( UINT32 uiCheckType, UINT16 usIndex, UINT16 *pusSubIndex )
{

	// Tile database is zero-based, Type indecies are 1-based!

	CHECKF ( uiCheckType < NUMBEROFTILETYPES );

	*pusSubIndex = usIndex - gTileTypeStartIndex[ uiCheckType ] + 1;

	return( TRUE );
}

BOOLEAN GetTypeSubIndexFromTileIndexChar( UINT32 uiCheckType, UINT16 usIndex, UINT8 *pubSubIndex )
{

	// Tile database is zero-based, Type indecies are 1-based!

	CHECKF ( uiCheckType < NUMBEROFTILETYPES );

	*pubSubIndex = (UINT8)(usIndex - gTileTypeStartIndex[ uiCheckType ] + 1);

	return( TRUE );
}

BOOLEAN	GetTileIndexFromTypeSubIndex( UINT32 uiCheckType, UINT16 usSubIndex, UINT16 *pusTileIndex )
{
	// Tile database is zero-based, Type indecies are 1-based!

	CHECKF ( uiCheckType < NUMBEROFTILETYPES );

	*pusTileIndex = usSubIndex + gTileTypeStartIndex[ uiCheckType ] - 1;

	return( TRUE );

}



BOOLEAN MoveLandIndexToTop( UINT32 iMapIndex, UINT16 usIndex )
{

	// Remove, then add again
	RemoveLand( iMapIndex, usIndex );

	AddLandToHead( iMapIndex, usIndex );

	return( TRUE );
}


// Database access functions
BOOLEAN	GetTileType( UINT16 usIndex, UINT32 *puiType )
{
  TILE_ELEMENT		TileElem;

	CHECKF( usIndex != NO_TILE );

	// Get tile element
	TileElem = gTileDatabase[ usIndex ];

	*puiType = TileElem.fType;

	return( TRUE );
}

BOOLEAN	GetTileFlags( UINT16 usIndex, UINT32 *puiFlags )
{
  TILE_ELEMENT		TileElem;

	CHECKF( usIndex != NO_TILE );
	CHECKF( usIndex < NUMBEROFTILES );

	// Get tile element
	TileElem = gTileDatabase[ usIndex ];

	*puiFlags = TileElem.uiFlags;

	return( TRUE );
}

BOOLEAN	GetTileTypeLogicalHeight( UINT32 fType, UINT8 *pubLogHeight )
{
	*pubLogHeight = gTileTypeLogicalHeight[ fType ];

	return( TRUE );
}



BOOLEAN LandTypeHeigher( UINT32 uiDestType, UINT32 uiSrcType  )
{
  UINT8					ubDestLogHeight;
  UINT8					ubSrcLogHeight;

	// Get logical height of type at head and type we wish to paste
	GetTileTypeLogicalHeight( uiDestType, &ubDestLogHeight );
	GetTileTypeLogicalHeight( uiSrcType, &ubSrcLogHeight );

	return( ubDestLogHeight > ubSrcLogHeight );

}


BOOLEAN AnyHeigherLand( UINT32 iMapIndex, UINT32 uiSrcType, UINT8 *pubLastLevel )
{
	LEVELNODE		*pLand		 = NULL;
  UINT8					ubSrcLogHeight=0;
	UINT32				fTileType=0;
	UINT8					level = 0;
	UINT8					ubSrcTypeLevel=0;
	BOOLEAN				fFound = FALSE;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	GetTileTypeLogicalHeight( uiSrcType, &ubSrcLogHeight );

	// Check that src type is not head
	if ( GetTypeLandLevel( iMapIndex, uiSrcType, &ubSrcTypeLevel ) )
	{
		 if ( ubSrcTypeLevel == LANDHEAD )
		 {
				return( FALSE );
		 }
	}

	// Look through all objects and Search for type

	while( pLand != NULL )
	{

		// Get type and height
		GetTileType( pLand->usIndex, &fTileType );

		if ( gTileTypeLogicalHeight[ fTileType ] > ubSrcLogHeight )
		{
			*pubLastLevel = level;
			fFound = TRUE;
		}

		// Advance to next
		pLand = pLand->pNext;

		level++;

	}

	// Could not find it, return FALSE
	return( fFound );

}

BOOLEAN AnyLowerLand( UINT32 iMapIndex, UINT32 uiSrcType, UINT8 *pubLastLevel )
{
	LEVELNODE		*pLand		 = NULL;
  UINT8					ubSrcLogHeight;
	UINT32				fTileType=0;
	UINT8					level = 0;
	UINT8					ubSrcTypeLevel;
  TILE_ELEMENT		TileElem;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	GetTileTypeLogicalHeight( uiSrcType, &ubSrcLogHeight );

	GetTypeLandLevel( iMapIndex, uiSrcType, &ubSrcTypeLevel );

	// Look through all objects and Search for type
	while( pLand != NULL )
	{

		// Get type and height
		GetTileType( pLand->usIndex, &fTileType );

		if ( gTileTypeLogicalHeight[ fTileType ] < ubSrcLogHeight )
		{
			*pubLastLevel = level;
			return( TRUE );
		}

		// Get tile element
		TileElem = gTileDatabase[ pLand->usIndex ];

		// Get full tile flag
		if ( TileElem.ubFullTile && fTileType != uiSrcType )
		{
			return( FALSE );
		}


		// Advance to next
		pLand = pLand->pNext;

		level++;

	}

	// Could not find it, return FALSE
	return( FALSE );

}


BOOLEAN GetWallOrientation( UINT16 usIndex, UINT16 *pusWallOrientation )
{
  TILE_ELEMENT		TileElem;

	CHECKF( usIndex != NO_TILE );

	// Get tile element
	TileElem = gTileDatabase[ usIndex ];

	*pusWallOrientation = TileElem.usWallOrientation;

	return( TRUE );
}

BOOLEAN ContainsWallOrientation( INT32 iMapIndex, UINT32 uiType, UINT16 usWallOrientation, UINT8 *pubLevel )
{
	LEVELNODE	*pStruct = NULL;
	UINT8					level = 0;
	UINT16				usCheckWallOrient=0;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all objects and Search for type

	while( pStruct != NULL )
	{

		GetWallOrientation( pStruct->usIndex, &usCheckWallOrient );

		if ( usCheckWallOrient == usWallOrientation )
		{
				*pubLevel = level;
				return( TRUE );
		}

		// Advance to next
		pStruct = pStruct->pNext;

		level++;

	}

	// Could not find it, return FALSE
	return( FALSE );

}

//Kris:  This function returns two special types if there are two walls
//sharing the same tile.  This case only happens with the exterior and
//interior bottom corners.  Otherwise, it returns the orientation of the
//first wall encountered -- not that there should be duplicate walls...
UINT8 CalculateWallOrientationsAtGridNo( INT32 iMapIndex )
{
	UINT16 usCheckWallOrientation=0;
	LEVELNODE *pStruct = NULL;
	UINT8 ubFinalWallOrientation = NO_ORIENTATION;
	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;
	//Traverse all of the pStructs
	while( pStruct != NULL )
	{
		GetWallOrientation( pStruct->usIndex, &usCheckWallOrientation );
		if( ubFinalWallOrientation == NO_ORIENTATION )
		{	//Get the first valid orientation.
			ubFinalWallOrientation = (UINT8)usCheckWallOrientation;
		}
		else switch( ubFinalWallOrientation )
		{	//If the first valid orientation has the key counterpart orientation,
			//return the special corner orientations.
			case INSIDE_TOP_LEFT:
				if( usCheckWallOrientation == INSIDE_TOP_RIGHT )
					return INSIDE_BOTTOM_CORNER;
				break;
			case INSIDE_TOP_RIGHT:
				if( usCheckWallOrientation == INSIDE_TOP_LEFT )
					return INSIDE_BOTTOM_CORNER;
				break;
			case OUTSIDE_TOP_LEFT:
				if( usCheckWallOrientation == OUTSIDE_TOP_RIGHT )
					return OUTSIDE_BOTTOM_CORNER;
				break;
			case OUTSIDE_TOP_RIGHT:
				if( usCheckWallOrientation == OUTSIDE_TOP_LEFT )
					return OUTSIDE_BOTTOM_CORNER;
				break;
		}
		// Advance to next
		pStruct = pStruct->pNext;
	}
	// Only one wall, so return it's orienation.
	return ubFinalWallOrientation;
}


BOOLEAN AllocateAnimTileData( TILE_ELEMENT *pTileElem, UINT8 ubNumFrames )
{
	pTileElem->pAnimData = MemAlloc( sizeof( TILE_ANIMATION_DATA ) );

	CHECKF( pTileElem->pAnimData != NULL );

	pTileElem->pAnimData->pusFrames = MemAlloc( sizeof( UINT16 ) * ubNumFrames );

	CHECKF( pTileElem->pAnimData->pusFrames != NULL );

	// Set # if frames!
	pTileElem->pAnimData->ubNumFrames = ubNumFrames;

	return( TRUE );
}

void FreeAnimTileData( TILE_ELEMENT *pTileElem )
{
	if ( pTileElem->pAnimData != NULL )
	{
		// Free frames list
		MemFree( pTileElem->pAnimData->pusFrames );

		// Free frames
		MemFree( pTileElem->pAnimData );
	}
}

