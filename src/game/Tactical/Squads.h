#ifndef _SQUADS_H
#define _SQUADS_H

#include "JA2Types.h"


// header for squad management system
#define NUMBER_OF_SOLDIERS_PER_SQUAD		6
#define NUMBER_OF_DEAD_SOLDIERS_ON_SQUAD	6
#define SQUAD_INFO_FORMAT_VERSION		(1)                    // extending the Squad save format for dynamic squad sizes
#define SQUAD_INFO_NUM_RECORDS			(NUMBER_OF_SQUADS * 6)


// enums for squads
enum{
	FIRST_SQUAD=0,
	SECOND_SQUAD,
	THIRD_SQUAD,
	FOURTH_SQUAD,
	FIFTH_SQUAD,
	SIXTH_SQUAD,
	SEVENTH_SQUAD,
	EIGTH_SQUAD,
	NINTH_SQUAD,
	TENTH_SQUAD,
	ELEVENTH_SQUAD,
	TWELTH_SQUAD,
	THIRTEENTH_SQUAD,
	FOURTEENTH_SQUAD,
	FIFTHTEEN_SQUAD,
	SIXTEENTH_SQUAD,
	SEVENTEENTH_SQUAD,
	EIGTHTEENTH_SQUAD,
	NINTEENTH_SQUAD,
	TWENTYTH_SQUAD,
	NUMBER_OF_SQUADS,
};


// ATE: Added so we can have no current squad
// happens in we move off sector via tactical, but nobody is left!
#define NO_CURRENT_SQUAD			NUMBER_OF_SQUADS


// ptrs to soldier types of squads and their members

// squads
extern SOLDIERTYPE *Squad[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];

#define FOR_EACH_SLOT_IN_SQUAD(iter, squad) \
	for (SOLDIERTYPE** iter = Squad[(squad)], *const * const iter##__end = endof(Squad[(squad)]); iter != iter##__end; ++iter)

#define FOR_EACH_IN_SQUAD(iter, squad) \
	FOR_EACH_SLOT_IN_SQUAD(iter, squad) \
		if (!*iter) continue; else

extern INT32 iCurrentTacticalSquad;




// will initialize the squad lists for game initalization
void InitSquads( void );

// add character to squad
BOOLEAN AddCharacterToSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );


// find the first slot the guy will fit in, return true if he is in a squad or has been put in one
void AddCharacterToAnySquad(SOLDIERTYPE*);

// remove character from squads
BOOLEAN RemoveCharacterFromSquads( SOLDIERTYPE *pCharacter );


// return number of people in this squad
INT8 NumberOfPeopleInSquad( INT8 bSquadValue );

INT8 NumberOfNonEPCsInSquad( INT8 bSquadValue );

BOOLEAN IsRobotControllerInSquad( INT8 bSquadValue );

INT8 NumberOfPlayerControllableMercsInSquad( INT8 bSquadValue );

// what sector is the squad currently in?..return if anyone in squad
BOOLEAN SectorSquadIsIn(INT8 bSquadValue, INT16* sMapX, INT16* sMapY, INT8* sMapZ);

// rebuild current squad list
void RebuildCurrentSquad( void );

// copy path from character back to squad
void CopyPathOfCharacterToSquad(SOLDIERTYPE* pCharacter, INT8 bSquadValue);

// what is the id of the current squad?
INT32 CurrentSquad( void );

// add character to unique squad, returns the squad #
INT8 AddCharacterToUniqueSquad( SOLDIERTYPE *pCharacter );

// is this squad empty?
BOOLEAN SquadIsEmpty( INT8 bSquadValue );

// is this squad in the current tactical sector?
BOOLEAN IsSquadOnCurrentTacticalMap( INT32 iCurrentSquad );


// set this squad as the current tatcical squad
BOOLEAN SetCurrentSquad( INT32 iCurrentSquad, BOOLEAN fForce );

// set default squad in sector
void SetDefaultSquadOnSectorEntry( BOOLEAN fForce );

// get last squad that has active mercs
INT32 GetLastSquadActive( void );

void ExamineCurrentSquadLights( void );


//Save the squad information to the saved game file
void SaveSquadInfoToSavedGameFile(HWFILE);

//Load all the squad info from the saved game file
void LoadSquadInfoFromSavedGameFile(HWFILE);

// get squad id of first free squad
INT8 GetFirstEmptySquad( void );

// dead soldier was on squad
BOOLEAN SoldierIsDeadAndWasOnSquad( SOLDIERTYPE *pSoldier, INT8 bSquadValue );

// now reset the table for these mercs
void ResetDeadSquadMemberList(INT32 iSquadValue);

// this passed  soldier on the current squad int he tactical map
BOOLEAN IsMercOnCurrentSquad(const SOLDIERTYPE* pSoldier);

// is this squad filled up?
BOOLEAN IsThisSquadFull( INT8 bSquadValue );

// is this squad moving?
BOOLEAN IsThisSquadOnTheMove( INT8 bSquadValue );

// is there a vehicle in this squad?
BOOLEAN DoesVehicleExistInSquad( INT8 bSquadValue );

// re-create any trashed squad movement groups
void CheckSquadMovementGroups( void );

#endif
