#ifndef __SOLDER_PROFILE_H
#define __SOLDER_PROFILE_H

#include "Soldier Control.h"
#include "Soldier Profile Type.h"
#include "Merc Hiring.h"

extern INT8 gbSkillTraitBonus[NUM_SKILLTRAITS];
extern UINT8 gubTerrorists[];
extern INT16 gsTerroristSector[][5][2];
extern BOOLEAN	gfPotentialTeamChangeDuringDeath;

MERCPROFILESTRUCT gMercProfiles[ NUM_PROFILES ];

#define AIM_AND_MERC_MERCS		51		// A.I.M. is 0-39, M.E.R.C.s are 40-50

//enums for the mercs
enum
{
	BIFF = 40,
	HAYWIRE,
	GASKET,
	RAZOR,
	FLO,
	GUMPY,
	LARRY_NORMAL,
	LARRY_DRUNK,
	COUGAR,
	NUMB,
	BUBBA,

	MIGUEL = 57,
	CARLOS,
	IRA,
	DIMITRI,

	DEVIN,	// 61
	ROBOT,
	HAMOUS,
	SLAY,
	RPC65,
	DYNAMO,
	SHANK,
	IGGY,
	VINCE,
	CONRAD,

	RPC71,	// 71
	MADDOG,
	DARREL,
	PERKO,
	QUEEN,
	AUNTIE,
	ENRICO,
	CARMEN,
	JOE,
	STEVE,

	RAT,		// 81
	ANNIE,
	CHRIS,
	BOB,
	BRENDA,
	KINGPIN,
	DARREN,
	MARIA,
	ANGEL,
	JOEY,

	TONY,		// 91
	FRANK,
	SPIKE,
	DAMON,
	KYLE,
	MICKY,
	SKYRIDER,
	PABLO,
	SAL,
	FATHER,

	FATIMA,	// 101
	WARDEN,
	GORDON,
	GABBY,
	ERNEST,
	FRED,
	MADAME,
	YANNI,
	MARTHA,
	TIFFANY,

	T_REX,	// 111
	DRUGGIST,	// ELGIN
	JAKE,
	PACOS,
	GERARD,
	SKIPPER,
	HANS,
	JOHN,
	MARY,
	GENERAL,

	SERGEANT,	// 121
	ARMAND,
	LORA,
	FRANZ,
	HOWARD,
	SAM,
	ELDIN,
	ARNIE,
	TINA,
	FREDO,

	WALTER,	// 131
	JENNY,
	BILLY,
	BREWSTER,
	ELLIOT,
	DEREK,
	OLIVER,
	WALDO,
	DOREEN,
	JIM,

	JACK,		// 141
	OLAF,
	RAY,
	OLGA,
	TYRONE,
	MADLAB,
	KEITH,
	MATT,
	MIKE,
	DARYL,

	HERVE,	// 151
	PETER,
	ALBERTO,
	CARLO,
	MANNY,
	OSWALD,
	CALVIN,
	CARL,
	SPECK,

	PROF_HUMMER,

	PROF_ELDERODO,	// 161
	PROF_ICECREAM,
	PROF_HELICOPTER,
	NPC164,
	NPC165,
	NPC166,
	NPC167,
	NPC168,
	NPC169
} NPCIDs;

BOOLEAN LoadMercProfiles(void);

// This function will overload the default for the face index ( which is the ubCharNum )
void SetProfileFaceData( UINT8 ubCharNum, UINT8 ubFaceIndex, UINT16 usEyesX, UINT16 usEyesY, UINT16 usMouthX, UINT16 usMouthY );
SOLDIERTYPE * FindSoldierByProfileID( UINT8 ubProfileID, BOOLEAN fPlayerMercsOnly );

BOOLEAN RecruitRPC( UINT8 ubCharNum );

BOOLEAN RecruitEPC( UINT8 ubCharNum );
BOOLEAN UnRecruitEPC( UINT8 ubCharNum );

INT8 WhichBuddy( UINT8 ubCharNum, UINT8 ubBuddy );
INT8 WhichHated( UINT8 ubCharNum, UINT8 ubHated );

SOLDIERTYPE *ChangeSoldierTeam( SOLDIERTYPE *pSoldier, UINT8 ubTeam );

BOOLEAN IsProfileATerrorist( UINT8 ubProfile );
BOOLEAN IsProfileAHeadMiner( UINT8 ubProfile );

void UpdateSoldierPointerDataIntoProfile( BOOLEAN fPlayerMercs );


// Returns true if a buddy of the merc is on team
BOOLEAN DoesMercHaveABuddyOnTheTeam( UINT8 ubMercID );

void StartSomeMercsOnAssignment( void );

void DecideOnAssassin( void );

extern BOOLEAN gubNumTerrorists;

void MakeRemainingTerroristsTougher( void );
void MakeRemainingAssassinsTougher( void );
SOLDIERTYPE * SwapLarrysProfiles( SOLDIERTYPE * pSoldier );

BOOLEAN DoesNPCOwnBuilding( SOLDIERTYPE *pSoldier, INT16 sGridNo );

#endif
