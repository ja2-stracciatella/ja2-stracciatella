#ifndef _SQUADS_H
#define _SQUADS_H

#include "Soldier Control.h"
#include "FileMan.h"

// header for squad management system
#define NUMBER_OF_SOLDIERS_PER_SQUAD 6

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
#define NO_CURRENT_SQUAD  NUMBER_OF_SQUADS


// ptrs to soldier types of squads and their members

// squads
extern SOLDIERTYPE *Squad[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];

extern INT32 iCurrentTacticalSquad;




// will initialize the squad lists for game initalization
void InitSquads( void );

// add character to squad
BOOLEAN AddCharacterToSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );


// find the first slot the guy will fit in, return true if he is in a squad or has been put in one
BOOLEAN AddCharacterToAnySquad( SOLDIERTYPE *pCharacter );

// remove character from squads
BOOLEAN RemoveCharacterFromSquads( SOLDIERTYPE *pCharacter );

// remove character from a squad
BOOLEAN RemoveCharacterFromASquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );

// check if character is in this squad
BOOLEAN IsCharacterInSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );


// return which squad character is in, -1 if none found
INT8 SquadCharacterIsIn( SOLDIERTYPE *pCharacter );

// what slot is character in in this squad?..-1 if not found in squad
INT8 SlotCharacterIsInSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );

// return number of people in this squad
INT8 NumberOfPeopleInSquad( INT8 bSquadValue );

INT8 NumberOfNonEPCsInSquad( INT8 bSquadValue );

BOOLEAN IsRobotControllerInSquad( INT8 bSquadValue );

INT8 NumberOfPlayerControllableMercsInSquad( INT8 bSquadValue );

// what sector is the squad currently in?..return if anyone in squad
BOOLEAN SectorSquadIsIn(INT8 bSquadValue, INT16 *sMapX, INT16 *sMapY, INT16 *sMapZ );

// rebuild current squad list
void RebuildCurrentSquad( void );

// copy path of squad to character
BOOLEAN CopyPathOfSquadToCharacter(  SOLDIERTYPE *pCharacter, INT8 bSquadValue );

// copy path from character back to squad
BOOLEAN CopyPathOfCharacterToSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );

// what is the id of the current squad?
INT32 CurrentSquad( void );

// add character to unique squad (returns the squad #, -1 if failed)
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

// set squads between sector position
void SetSquadPositionBetweenSectors( UINT8 ubNextX, UINT8 ubNextY, UINT8 ubPrevX, UINT8 ubPrevY, UINT32 uiTraverseTime, UINT32 uiArriveTime, UINT8 ubSquadValue  );

// get squads between sector positions and times
void GetSquadPosition( UINT8 *ubNextX, UINT8 *ubNextY, UINT8 *ubPrevX, UINT8 *ubPrevY, UINT32 *uiTraverseTime, UINT32 *uiArriveTime, UINT8 ubSquadValue );

void ExamineCurrentSquadLights( void );

// get location of this squad
void GetLocationOfSquad( INT16 *sX, INT16 *sY, INT8 *bZ, INT8 bSquadValue );


//Save the squad information to the saved game file
BOOLEAN SaveSquadInfoToSavedGameFile( HWFILE hFile );

//Load all the squad info from the saved game file
BOOLEAN LoadSquadInfoFromSavedGameFile( HWFILE hFile );

// get squad id of first free squad
INT8 GetFirstEmptySquad( void );

// is this squad in the same sector as soldier?
BOOLEAN IsSquadInSector( SOLDIERTYPE *pSoldier, UINT8 ubSquad );

// is any merc on squad asleep?
BOOLEAN IsAnyMercOnSquadAsleep( UINT8 ubSquadValue );

// is therea  dead guy here
BOOLEAN IsDeadGuyInThisSquadSlot( INT8 bSlotId, INT8 bSquadValue , INT8 *bNumberOfDeadGuysSoFar );

// dead soldier was on squad
BOOLEAN SoldierIsDeadAndWasOnSquad( SOLDIERTYPE *pSoldier, INT8 bSquadValue );

// now reset the table for these mercs
BOOLEAN ResetDeadSquadMemberList( INT32 iSquadValue );

// this passed  soldier on the current squad int he tactical map
BOOLEAN IsMercOnCurrentSquad( SOLDIERTYPE *pSoldier );

// is this squad filled up?
BOOLEAN IsThisSquadFull( INT8 bSquadValue );

// is this squad moving?
BOOLEAN IsThisSquadOnTheMove( INT8 bSquadValue );

// is there a vehicle in this squad?
BOOLEAN DoesVehicleExistInSquad( INT8 bSquadValue );

// re-create any trashed squad movement groups
void CheckSquadMovementGroups( void );

#endif
