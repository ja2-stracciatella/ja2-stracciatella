#ifndef _ROTTING_CORPSES_H
#define _ROTTING_CORPSES_H

#include "Debug.h"
#include "Overhead_Types.h"
#include "Tile_Animation.h"

#include <string_theory/string>


#define NUM_CORPSE_SHADES				17

enum RottingCorpseDefines
{
	NO_CORPSE,
	SMERC_JFK,
	SMERC_BCK,
	SMERC_FWD,
	SMERC_DHD,
	SMERC_PRN,
	SMERC_WTR,
	SMERC_FALL,
	SMERC_FALLF,

	MMERC_JFK,
	MMERC_BCK,
	MMERC_FWD,
	MMERC_DHD,
	MMERC_PRN,
	MMERC_WTR,
	MMERC_FALL,
	MMERC_FALLF,

	FMERC_JFK,
	FMERC_BCK,
	FMERC_FWD,
	FMERC_DHD,
	FMERC_PRN,
	FMERC_WTR,
	FMERC_FALL,
	FMERC_FALLF,

	// CIVS
	M_DEAD1,
	K_DEAD1,
	H_DEAD1,
	FT_DEAD1,
	S_DEAD1,
	W_DEAD1,
	C_DEAD1,
	M_DEAD2,
	K_DEAD2,
	H_DEAD2,

	FT_DEAD2,
	S_DEAD2,
	W_DEAD2,
	C_DEAD2,
	BLOODCAT_DEAD,
	COW_DEAD,
	ADULTMONSTER_DEAD,
	INFANTMONSTER_DEAD,
	LARVAEMONSTER_DEAD,
	ROTTING_STAGE2,

	TANK1_DEAD,
	TANK2_DEAD,
	HUMMER_DEAD,
	ICECREAM_DEAD,
	QUEEN_MONSTER_DEAD,
	ROBOT_DEAD,
	BURNT_DEAD,
	EXPLODE_DEAD,

	NUM_CORPSES
};


#define ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO	0x01 //Find the closest spot to the given gridno
#define ROTTING_CORPSE_USE_NORTH_ENTRY_POINT		0x02 //Find the spot closest to the north entry grid
#define ROTTING_CORPSE_USE_SOUTH_ENTRY_POINT		0x04 //Find the spot closest to the south entry grid
#define ROTTING_CORPSE_USE_EAST_ENTRY_POINT		0x08 //Find the spot closest to the east entry grid
#define ROTTING_CORPSE_USE_WEST_ENTRY_POINT		0x10 //Find the spot closest to the west entry grid
#define ROTTING_CORPSE_USE_CAMO_PALETTE		0x20 //We use camo palette here....
#define ROTTING_CORPSE_VEHICLE				0x40 //Vehicle Corpse

struct ROTTING_CORPSE_DEFINITION
{
	UINT8   ubType;
	UINT8   ubBodyType;
	INT16   sGridNo;
	INT16   sHeightAdjustment;

	ST::string HeadPal; // Palette reps
	ST::string PantsPal;
	ST::string VestPal;
	ST::string SkinPal;

	INT8    bDirection;
	UINT32  uiTimeOfDeath;

	UINT16  usFlags;

	INT8    bLevel;

	INT8    bVisible;
	INT8    bNumServicingCrows;
	UINT8   ubProfile;
	BOOLEAN fHeadTaken;
	UINT8   ubAIWarningValue;
};


struct ROTTING_CORPSE
{
	ROTTING_CORPSE_DEFINITION def;
	BOOLEAN fActivated;

	ANITILE *pAniTile;

	UINT16  *pShades[ NUM_CORPSE_SHADES ];
};


ROTTING_CORPSE* AddRottingCorpse(ROTTING_CORPSE_DEFINITION* pCorpseDef);

void RemoveCorpses(void);


BOOLEAN TurnSoldierIntoCorpse(SOLDIERTYPE&);

INT16 FindNearestRottingCorpse( SOLDIERTYPE *pSoldier );

INT16 FindNearestAvailableGridNoForCorpse( ROTTING_CORPSE_DEFINITION *pCorpseDef, INT8 ubRadius );


void HandleRottingCorpses(void);

void VaporizeCorpse( INT16 sGridNo, UINT16 usStructureID );
void CorpseHit( INT16 sGridNo, UINT16 usStructureID );

void HandleCrowLeave( SOLDIERTYPE *pSoldier );

void HandleCrowFlyAway( SOLDIERTYPE *pSoldier );

#define MAX_ROTTING_CORPSES				100

extern ROTTING_CORPSE gRottingCorpse[ MAX_ROTTING_CORPSES ];
extern INT32 giNumRottingCorpse;
extern UINT8 gb4DirectionsFrom8[8];

static inline UINT32 Corpse2ID(const ROTTING_CORPSE* const c)
{
	Assert(gRottingCorpse <= c && c < endof(gRottingCorpse));
	Assert(c->fActivated);
	return (UINT32)(c - gRottingCorpse);
}

static inline ROTTING_CORPSE* ID2Corpse(const UINT32 id)
{
	Assert(id < lengthof(gRottingCorpse));
	ROTTING_CORPSE* const c = &gRottingCorpse[id];
	Assert(c->fActivated);
	return c;
}

#define CORPSE2ID(c) (Corpse2ID((c)))
#define ID2CORPSE(i) (ID2Corpse((i)))

#define BASE_FOR_EACH_ROTTING_CORPSE(type, iter)                      \
	for (type* iter = gRottingCorpse,                      \
		* const end__##iter = gRottingCorpse + giNumRottingCorpse; \
		iter != end__##iter;                                           \
		++iter)                                                        \
		if (!iter->fActivated) continue; else
#define  FOR_EACH_ROTTING_CORPSE(iter) BASE_FOR_EACH_ROTTING_CORPSE(      ROTTING_CORPSE, iter)
#define CFOR_EACH_ROTTING_CORPSE(iter) BASE_FOR_EACH_ROTTING_CORPSE(const ROTTING_CORPSE, iter)

ROTTING_CORPSE *GetCorpseAtGridNo( INT16 sGridNo , INT8 bLevel );
BOOLEAN IsValidDecapitationCorpse(const ROTTING_CORPSE* c);
void DecapitateCorpse(INT16 sGridNo, INT8 bLevel);

void GetBloodFromCorpse( SOLDIERTYPE *pSoldier );

UINT16 GetCorpseStructIndex(const ROTTING_CORPSE_DEFINITION* pCorpseDef, BOOLEAN fForImage);

void LookForAndMayCommentOnSeeingCorpse( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubLevel );

INT16 GetGridNoOfCorpseGivenProfileID( UINT8 ubProfileID );

void DecayRottingCorpseAIWarnings( void );
UINT8 GetNearestRottingCorpseAIWarning( INT16 sGridNo );

void ReduceAmmoDroppedByNonPlayerSoldiers(SOLDIERTYPE const&, OBJECTTYPE& ammo);

#endif
