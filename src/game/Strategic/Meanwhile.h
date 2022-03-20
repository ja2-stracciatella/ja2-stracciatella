#ifndef MEANWHILE_H
#define MEANWHILE_H

#include "Types.h"


enum
{
	END_OF_PLAYERS_FIRST_BATTLE,
	DRASSEN_LIBERATED,
	CAMBRIA_LIBERATED,
	ALMA_LIBERATED,
	GRUMM_LIBERATED,
	CHITZENA_LIBERATED,
	NW_SAM,
	NE_SAM,
	CENTRAL_SAM,
	FLOWERS,
	LOST_TOWN,
	INTERROGATION,
	CREATURES,
	KILL_CHOPPER,
	AWOL_SCIENTIST,
	OUTSKIRTS_MEDUNA,
	BALIME_LIBERATED,
	NUM_MEANWHILES
};


struct MEANWHILE_DEFINITION
{
	SGPSector				sSector;
	UINT16			usTriggerEvent;

	UINT8				ubMeanwhileID;
	UINT8				ubNPCNumber;
};


void ScheduleMeanwhileEvent(const SGPSector& sector, UINT16 trigger_event, UINT8 meanwhile_id, UINT8 npc_profile, UINT32 time);

void BeginMeanwhile(UINT8 ubMeanwhileID);

void CheckForMeanwhileOKStart(void);
void EndMeanwhile(void);



bool AreInMeanwhile();
UINT8 GetMeanwhileID(void);
BOOLEAN AreReloadingFromMeanwhile(void);

void LocateToMeanwhileCharacter(void);

// post meanwhile event for town liberation
void HandleMeanWhileEventPostingForTownLiberation( UINT8 bTownId );

// post meanwhile event for SAM liberation
void HandleMeanWhileEventPostingForSAMLiberation( INT8 bSAMId );

// trash world has been called, should we do the first meanwhile?
void HandleFirstMeanWhileSetUpWithTrashWorld( void );

// battle ended, check if we should set up a meanwhile?
void HandleFirstBattleEndingWhileInTown( INT16 sSectorX, INT16 sSectorY, INT16 bSectorZ, BOOLEAN fFromAutoResolve );

// lost an entire town to the enemy!
void HandleMeanWhileEventPostingForTownLoss();

// handle short cutting past a meanwhilescene while it is being handled
void HandleShortCuttingOfMeanwhileSceneByPlayer( UINT8 ubMeanwhileID, INT32 iLastProfileId, INT32 iLastProfileAction );

// handle release of creatures meanwhile
void HandleCreatureRelease( void );

// handle sending flowers to the queen
void HandleFlowersMeanwhileScene( INT8 bTimeCode );

// player reaches the outskirts of Meduna
void HandleOutskirtsOfMedunaMeanwhileScene( void );

// let player know about Madlab after certain status % reached
void HandleScientistAWOLMeanwhileScene( void );

// handle chopper used meanwhile
void HandleKillChopperMeanwhileScene( void );

extern	MEANWHILE_DEFINITION	gCurrentMeanwhileDef;
extern	MEANWHILE_DEFINITION	gMeanwhileDef[NUM_MEANWHILES];
extern	BOOLEAN								gfMeanwhileTryingToStart;
extern	BOOLEAN								gfInMeanwhile;
extern	UINT32								uiMeanWhileFlags;

#endif
