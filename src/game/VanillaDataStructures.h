#pragma once

/* This file contains copied of data structures of the original game. */

#include "Types.h"

/** Vanilla Data Structures */
namespace VDS
{

typedef uint16_t        CHAR16;


/* -------------------------------------------------------------------------
 * ja2/Build/GameSettings.h
 * ------------------------------------------------------------------------- */

typedef struct
{
	BOOLEAN fGunNut;
	BOOLEAN	fSciFi;
	UINT8		ubDifficultyLevel;
	BOOLEAN	fTurnTimeLimit;
	UINT8	ubGameSaveMode;

	UINT8		ubFiller[7];

} GAME_OPTIONS;


/* -------------------------------------------------------------------------
 * ja2/Build/SaveLoadGame.h
 * ------------------------------------------------------------------------- */

#define		SIZE_OF_SAVE_GAME_DESC				128

#define		GAME_VERSION_LENGTH						16

typedef struct
{
	UINT32	uiSavedGameVersion;
	INT8		zGameVersionNumber[ GAME_VERSION_LENGTH ];

	CHAR16	sSavedGameDesc[ SIZE_OF_SAVE_GAME_DESC ];

	UINT32	uiFlags;

#ifdef CRIPPLED_VERSION
	UINT8 ubCrippleFiller[20];
#endif


	//The following will be used to quickly access info to display in the save/load screen
	UINT32	uiDay;
	UINT8		ubHour;
	UINT8		ubMin;
	INT16		sSectorX;
	INT16		sSectorY;
	INT8		bSectorZ;
	UINT8		ubNumOfMercsOnPlayersTeam;
	INT32		iCurrentBalance;

	UINT32	uiCurrentScreen;

	BOOLEAN	fAlternateSector;

	BOOLEAN	fWorldLoaded;

	UINT8		ubLoadScreenID;		//The load screen that should be used when loading the saved game

	GAME_OPTIONS	sInitialGameOptions;	//need these in the header so we can get the info from it on the save load screen.

	UINT32	uiRandom;

	UINT8		ubFiller[110];

} SAVED_GAME_HEADER;

/* -------------------------------------------------------------------------
 *
 * ------------------------------------------------------------------------- */

}
