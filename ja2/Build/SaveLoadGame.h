#ifndef _SAVE_LOAD_GAME_H_
#define _SAVE_LOAD_GAME_H_

#include "Soldier Control.h"

#define		BYTESINMEGABYTE						1048576 //1024*1024
#define		REQUIRED_FREE_SPACE				(20 * BYTESINMEGABYTE)

#define		SIZE_OF_SAVE_GAME_DESC				128

#define		GAME_VERSION_LENGTH						16

#define		SAVE__ERROR_NUM						99
#define		SAVE__END_TURN_NUM				98

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



extern	UINT32		guiScreenToGotoAfterLoadingSavedGame;
extern UINT32 guiSaveGameVersion;

void			CreateSavedGameFileNameFromNumber( UINT8 ubSaveGameID, STR pzNewFileName );




BOOLEAN SaveGame( UINT8 ubSaveGameID, STR16 pGameDesc );
BOOLEAN LoadSavedGame( UINT8 ubSavedGameID );

BOOLEAN CopySavedSoldierInfoToNewSoldier( SOLDIERTYPE *pDestSourceInfo, SOLDIERTYPE *pSourceInfo );

BOOLEAN		SaveFilesToSavedGame( STR pSrcFileName, HWFILE hFile );
BOOLEAN		LoadFilesFromSavedGame( STR pSrcFileName, HWFILE hFile );

BOOLEAN DoesUserHaveEnoughHardDriveSpace();

void GetBestPossibleSectorXYZValues( INT16 *psSectorX, INT16 *psSectorY, INT8 *pbSectorZ );


UINT32	guiLastSaveGameNum;
INT8		GetNumberForAutoSave( BOOLEAN fLatestAutoSave );

#endif
