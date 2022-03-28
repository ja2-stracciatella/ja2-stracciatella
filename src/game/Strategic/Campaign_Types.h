#ifndef __CAMPAIGN_TYPES_H
#define __CAMPAIGN_TYPES_H

#include "Debug.h"
#include "Types.h"

//Macro to convert sector coordinates (1-16,1-16) to 0-255
static inline UINT8 SECTOR(UINT8 const x, UINT8 const y)
{
	Assert(1 <= x && x <= 16 && 1 <= y && y <= 16);
	return (y - 1) * 16 + x - 1;
}

#define SECTORX(SectorID) ((SectorID % 16) + 1)
#define SECTORY(SectorID) ((SectorID / 16) + 1)

//Sector enumerations
//
//NOTE: These use the 0-255 SectorInfo[] numbering system, and CAN'T be used as indexes into the StrategicMap[] array
//Use SGPSector::AsStrategicIndex() to convert
enum
{
	SEC_A1,	SEC_A2,	SEC_A3, SEC_A4,	SEC_A5, SEC_A6,	SEC_A7,	SEC_A8,	SEC_A9, SEC_A10, SEC_A11, SEC_A12, SEC_A13, SEC_A14, SEC_A15, SEC_A16,
	SEC_B1,	SEC_B2,	SEC_B3, SEC_B4,	SEC_B5, SEC_B6,	SEC_B7,	SEC_B8,	SEC_B9, SEC_B10, SEC_B11, SEC_B12, SEC_B13, SEC_B14, SEC_B15, SEC_B16,
	SEC_C1,	SEC_C2,	SEC_C3, SEC_C4,	SEC_C5, SEC_C6,	SEC_C7,	SEC_C8,	SEC_C9, SEC_C10, SEC_C11, SEC_C12, SEC_C13, SEC_C14, SEC_C15, SEC_C16,
	SEC_D1,	SEC_D2,	SEC_D3, SEC_D4,	SEC_D5, SEC_D6, SEC_D7,	SEC_D8,	SEC_D9, SEC_D10, SEC_D11, SEC_D12, SEC_D13, SEC_D14, SEC_D15, SEC_D16,
	SEC_E1,	SEC_E2,	SEC_E3, SEC_E4,	SEC_E5, SEC_E6,	SEC_E7,	SEC_E8,	SEC_E9, SEC_E10, SEC_E11, SEC_E12, SEC_E13, SEC_E14, SEC_E15, SEC_E16,
	SEC_F1,	SEC_F2,	SEC_F3, SEC_F4,	SEC_F5, SEC_F6,	SEC_F7,	SEC_F8,	SEC_F9, SEC_F10, SEC_F11, SEC_F12, SEC_F13, SEC_F14, SEC_F15, SEC_F16,
	SEC_G1,	SEC_G2,	SEC_G3, SEC_G4,	SEC_G5, SEC_G6,	SEC_G7,	SEC_G8,	SEC_G9, SEC_G10, SEC_G11, SEC_G12, SEC_G13, SEC_G14, SEC_G15, SEC_G16,
	SEC_H1,	SEC_H2,	SEC_H3, SEC_H4,	SEC_H5, SEC_H6,	SEC_H7,	SEC_H8,	SEC_H9, SEC_H10, SEC_H11, SEC_H12, SEC_H13, SEC_H14, SEC_H15, SEC_H16,
	SEC_I1,	SEC_I2,	SEC_I3, SEC_I4,	SEC_I5, SEC_I6,	SEC_I7,	SEC_I8,	SEC_I9, SEC_I10, SEC_I11, SEC_I12, SEC_I13, SEC_I14, SEC_I15, SEC_I16,
	SEC_J1,	SEC_J2,	SEC_J3, SEC_J4,	SEC_J5, SEC_J6,	SEC_J7,	SEC_J8,	SEC_J9, SEC_J10, SEC_J11, SEC_J12, SEC_J13, SEC_J14, SEC_J15, SEC_J16,
	SEC_K1,	SEC_K2,	SEC_K3, SEC_K4,	SEC_K5, SEC_K6,	SEC_K7,	SEC_K8,	SEC_K9, SEC_K10, SEC_K11, SEC_K12, SEC_K13, SEC_K14, SEC_K15, SEC_K16,
	SEC_L1,	SEC_L2,	SEC_L3, SEC_L4,	SEC_L5, SEC_L6,	SEC_L7,	SEC_L8,	SEC_L9, SEC_L10, SEC_L11, SEC_L12, SEC_L13, SEC_L14, SEC_L15, SEC_L16,
	SEC_M1,	SEC_M2,	SEC_M3, SEC_M4,	SEC_M5, SEC_M6,	SEC_M7,	SEC_M8,	SEC_M9, SEC_M10, SEC_M11, SEC_M12, SEC_M13, SEC_M14, SEC_M15, SEC_M16,
	SEC_N1,	SEC_N2,	SEC_N3, SEC_N4,	SEC_N5, SEC_N6,	SEC_N7,	SEC_N8,	SEC_N9, SEC_N10, SEC_N11, SEC_N12, SEC_N13, SEC_N14, SEC_N15, SEC_N16,
	SEC_O1,	SEC_O2,	SEC_O3, SEC_O4,	SEC_O5, SEC_O6,	SEC_O7,	SEC_O8,	SEC_O9, SEC_O10, SEC_O11, SEC_O12, SEC_O13, SEC_O14, SEC_O15, SEC_O16,
	SEC_P1,	SEC_P2,	SEC_P3, SEC_P4,	SEC_P5, SEC_P6,	SEC_P7,	SEC_P8,	SEC_P9, SEC_P10, SEC_P11, SEC_P12, SEC_P13, SEC_P14, SEC_P15, SEC_P16
};


//Various flag definitions
enum SectorFlags
{
	SF_USE_MAP_SETTINGS                   = 0x00000001,
	SF_ENEMY_AMBUSH_LOCATION              = 0x00000002,

	/* Special case flag used when players encounter enemies in a sector, then
	 * retreat.  The number of enemies will display on mapscreen until time is
	 * compressed.  When time is compressed, the flag is cleared, and a question
	 * mark is displayed to reflect that the player no longer knows. */
	SF_PLAYER_KNOWS_ENEMIES_ARE_HERE      = 0x00000004,

	SF_ALREADY_VISITED                    = 0x00000020,
	SF_USE_ALTERNATE_MAP                  = 0x00000040,
	SF_PENDING_ALTERNATE_MAP              = 0x00000080,
	SF_ALREADY_LOADED                     = 0x00000100,
	SF_HAS_ENTERED_TACTICAL               = 0x00000200,
	SF_SKYRIDER_NOTICED_ENEMIES_HERE      = 0x00000400,
	SF_HAVE_USED_GUIDE_QUOTE              = 0x00000800,


	SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS     = 0x00100000,	// Temp File starts with sm_
	SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS  = 0x00200000,	// Temp File starts with l_

	SF_REVEALED_STATUS_TEMP_FILE_EXISTS   = 0x01000000,	// Temp File starts with v_
	SF_DOOR_STATUS_TEMP_FILE_EXISTS       = 0x02000000,	// Temp File starts with ds_
	SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS   = 0x04000000,	// Temp File starts with e_
	SF_CIV_PRESERVED_TEMP_FILE_EXISTS     = 0x08000000,	// Temp File starts with c_
	SF_ITEM_TEMP_FILE_EXISTS              = 0x10000000,	// Temp File starts with i_
	SF_ROTTING_CORPSE_TEMP_FILE_EXISTS    = 0x20000000,	// Temp File starts with r_
	SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS = 0x40000000,	// Temp File starts with m_
	SF_DOOR_TABLE_TEMP_FILES_EXISTS       = 0x80000000  // Temp File starts with d_
};


// town militia experience categories
enum
{
	GREEN_MILITIA = 0,
	REGULAR_MILITIA,
	ELITE_MILITIA,
	MAX_MILITIA_LEVELS
};

// facilities flags
#define SFCF_HOSPITAL		0x00000001
#define SFCF_INDUSTRY		0x00000002
#define SFCF_PRISON			0x00000004
#define SFCF_MILITARY		0x00000008
#define SFCF_AIRPORT		0x00000010
#define SFCF_GUN_RANGE	0x00000020

// coordinates of shooting range sector
#define GUN_RANGE_X		13
#define GUN_RANGE_Y		MAP_ROW_H
#define GUN_RANGE_Z		0


//Vehicle types
enum VehicleMovementType
{
	FOOT = 0x01,		//anywhere
	CAR = 0x02,		//roads
	TRUCK = 0x04,		//roads, plains, sparse
	TRACKED = 0x08,		//roads, plains, sand, sparse
	AIR = 0x10		//can traverse all terrains at 100%
};

//Traversability ratings
enum
{
	TOWN,					//instant
	ROAD,					//everything travels at 100%
	PLAINS,				//foot 90%, truck 75%, tracked 100%
	SAND,					//foot 70%, tracked 60%
	SPARSE,				//foot 70%, truck 50%, tracked 60%
	DENSE,				//foot 50%
	SWAMP,				//foot 20%
	WATER,				//foot 15%
	HILLS,				//foot 50%, truck 50%, tracked 50%
	GROUNDBARRIER,//only air (super dense forest, ocean, etc.)
	NS_RIVER,			//river from north to south
	EW_RIVER,			//river from east to west
	EDGEOFWORLD,		//nobody can traverse.
	//NEW (not used for border values -- traversal calculations)
	TROPICS,
	FARMLAND,
	PLAINS_ROAD,
	SPARSE_ROAD,
	FARMLAND_ROAD,
	TROPICS_ROAD,
	DENSE_ROAD,
	COASTAL,
	HILLS_ROAD,
	COASTAL_ROAD,
	SAND_ROAD,
	SWAMP_ROAD,
	//only used for text purposes and not assigned to areas (SAM sites are hard coded throughout the code)
	SPARSE_SAM_SITE, //D15 near Drassen
	SAND_SAM_SITE,   //I8 near Tixa
	TROPICS_SAM_SITE, //D2 near Chitzena
	MEDUNA_SAM_SITE, //N4 in Meduna
	CAMBRIA_HOSPITAL_SITE,
	DRASSEN_AIRPORT_SITE,
	MEDUNA_AIRPORT_SITE,
	SAM_SITE,

	REBEL_HIDEOUT,
	TIXA_DUNGEON,
	CREATURE_LAIR,
	ORTA_BASEMENT,
	TUNNEL,
	SHELTER,
	ABANDONED_MINE,

	NUM_TRAVTERRAIN_TYPES
};

//Used by ubGarrisonID when a sector doesn't point to a garrison.  Used by strategic AI only.
#define NO_GARRISON							255

struct SECTORINFO
{
	//information pertaining to this sector
	UINT32	uiFlags;						//various special conditions
	UINT8		ubGarrisonID;						//IF the sector has an ID for this (non 255), then the queen values this sector and it
																	//indexes the garrison group.
	INT8		ubPendingReinforcements;	//when the enemy owns this sector, this value will keep track of HIGH priority reinforcements -- not regular.
	BOOLEAN fMilitiaTrainingPaid;
	UINT8		ubMilitiaTrainingPercentDone;
	UINT8		ubMilitiaTrainingHundredths;
	//enemy only info
	UINT8		ubNumTroops;				//the actual number of troops here.
	UINT8		ubNumElites;				//the actual number of elites here.
	UINT8		ubNumAdmins;				//the actual number of admins here.
	UINT8		ubNumCreatures;			//only set when immediately before ground attack made!
	UINT8   ubTroopsInBattle, ubElitesInBattle, ubAdminsInBattle, ubCreaturesInBattle;

	UINT32	ubDayOfLastCreatureAttack;
	UINT32	uiFacilitiesFlags;	// the flags for various facilities

	UINT8		ubTraversability[5];//determines the traversability ratings to adjacent sectors.
															//The last index represents the traversability if travelling
															//throught the sector without entering it.
	INT8 bBloodCats;
	INT8 bBloodCatPlacements;

	UINT8 ubTravelRating;	//Represents how travelled a sector is.  Typically, the higher the travel rating,
												//the more people go near it.  A travel rating of 0 means there are never people
												//around.  This value is used for determining how often items would "vanish" from
												//a sector (nice theory, except it isn't being used that way.  Stealing is only in towns.  ARM)
	UINT8 ubNumberOfCivsAtLevel[ MAX_MILITIA_LEVELS ]; // town militia per experience class, 0/1/2 is GREEN/REGULAR/ELITE
	UINT32	uiTimeCurrentSectorWasLastLoaded;		//Specifies the last time the player was in the sector
	UINT32 uiTimeLastPlayerLiberated; //in game seconds (used to prevent the queen from attacking for awhile)

	BOOLEAN fSurfaceWasEverPlayerControlled;

	UINT32	uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;
};

#define NO_ADJACENT_SECTOR		0x00
#define NORTH_ADJACENT_SECTOR	0x01
#define EAST_ADJACENT_SECTOR	0x02
#define SOUTH_ADJACENT_SECTOR	0x04
#define WEST_ADJACENT_SECTOR	0x08


struct UNDERGROUND_SECTORINFO
{
	UINT32 uiFlags;
	SGPSector ubSector;
	UINT8 ubNumElites, ubNumTroops, ubNumAdmins, ubNumCreatures;
	UINT32	uiTimeCurrentSectorWasLastLoaded;		//Specifies the last time the player was in the sector
	UNDERGROUND_SECTORINFO* next;
	UINT8	ubAdjacentSectors;	//mask containing which sectors are adjacent
	UINT8 ubCreatureHabitat;	//determines how creatures live in this sector (see creature spreading.c)
	UINT8 ubElitesInBattle, ubTroopsInBattle, ubAdminsInBattle, ubCreaturesInBattle;

	UINT32	uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;
};


//The sector information required for the strategic AI.  Contains the number of enemy troops,
//as well as intentions, etc.
extern SECTORINFO SectorInfo[256];
extern UNDERGROUND_SECTORINFO *gpUndergroundSectorInfoHead;

#endif
