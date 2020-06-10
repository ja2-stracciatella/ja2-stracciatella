#ifndef _SAVE_LOAD_GAME_H_
#define _SAVE_LOAD_GAME_H_

#include "GameSettings.h"
#include "ScreenIDs.h"

#include <string_theory/string>


#define BYTESINMEGABYTE				1048576 //1024*1024
#define REQUIRED_FREE_SPACE				(20 * BYTESINMEGABYTE)

#define SIZE_OF_SAVE_GAME_DESC				128

#define NUM_SAVE_GAME_BACKUPS				2

#define GAME_VERSION_LENGTH				16

#define SAVE__ERROR_NUM				99
#define SAVE__END_TURN_NUM				98

#define SAVED_GAME_HEADER_ON_DISK_SIZE			(432) // Size of SAVED_GAME_HEADER on disk in Vanilla and Stracciatella Windows
#define SAVED_GAME_HEADER_ON_DISK_SIZE_STRAC_LIN	(688) // Size of SAVED_GAME_HEADER on disk in Stracciatella Linux

struct SAVED_GAME_HEADER
{
	UINT32	uiSavedGameVersion;
	char zGameVersionNumber[GAME_VERSION_LENGTH];

	ST::string sSavedGameDesc;

	/* (vanilla) UINT32	uiFlags; */

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

	/* (vanilla) UINT8		ubFiller[110]; */
};

/** Parse binary data and fill SAVED_GAME_HEADER structure.
 * @param data Data to be parsed.
 * @param h Header structure to be filled.
 * @param stracLinuxFormat Flag, telling to use "Stracciatella Linux" format. */
extern void ParseSavedGameHeader(const BYTE *data, SAVED_GAME_HEADER& h, bool stracLinuxFormat);

/** @brief Check if SAVED_GAME_HEADER structure contains valid data.
 * This function does the basic check. */
extern bool isValidSavedGameHeader(SAVED_GAME_HEADER& h);


/** @brief Extract saved game header from a file.
 * Return \a stracLinuxFormat = true, when the file is in "Stracciatella Linux" format. */
void ExtractSavedGameHeaderFromFile(HWFILE, SAVED_GAME_HEADER&, bool *stracLinuxFormat);


extern ScreenID guiScreenToGotoAfterLoadingSavedGame;

void CreateSavedGameFileNameFromNumber(UINT8 ubSaveGameID, char* pzNewFileName);


BOOLEAN SaveGame(UINT8 ubSaveGameID, const ST::string& gameDesc);
void    LoadSavedGame(UINT8 save_slot_id);

void BackupSavedGame(UINT8 const ubSaveGameID);

void SaveFilesToSavedGame(char const* pSrcFileName, HWFILE);
void LoadFilesFromSavedGame(char const* pSrcFileName, HWFILE);

void GetBestPossibleSectorXYZValues(INT16* psSectorX, INT16* psSectorY, INT8* pbSectorZ);

void SaveMercPath(HWFILE, PathSt const* head);
void LoadMercPath(HWFILE, PathSt** head);

extern UINT32 guiLastSaveGameNum;
INT8		GetNumberForAutoSave( BOOLEAN fLatestAutoSave );

extern UINT32 guiJA2EncryptionSet;

extern BOOLEAN gfUseConsecutiveQuickSaveSlots;

const char *IMPSavedProfileCreateFilename(const char *nickname);
bool IMPSavedProfileDoesFileExist(const char *nickname);
SGPFile* const IMPSavedProfileOpenFileForRead(const char *nickname);
int IMPSavedProfileLoadMercProfile(const char *nickname);
void IMPSavedProfileLoadInventory(const char *nickname, SOLDIERTYPE *pSoldier);

#endif
