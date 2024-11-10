#ifndef _SAVE_LOAD_GAME_H_
#define _SAVE_LOAD_GAME_H_

#include "GameSettings.h"
#include "JA2Types.h"
#include "ScreenIDs.h"

#include <string_theory/string>


#define BYTESINMEGABYTE				1048576 //1024*1024
#define REQUIRED_FREE_SPACE				(20 * BYTESINMEGABYTE)

#define NUM_SAVE_GAME_BACKUPS				2


struct SAVED_GAME_HEADER
{
	static constexpr size_t GAME_VERSION_LENGTH{ 16 };
	static constexpr size_t ON_DISK_SIZE{ 432 }; // Size of SAVED_GAME_HEADER on disk in Vanilla and Stracciatella Windows
	static constexpr size_t ON_DISK_SIZE_STRAC_LIN{ 688 }; // Size of SAVED_GAME_HEADER on disk in Stracciatella Linux
	static constexpr size_t SIZE_OF_SAVE_GAME_DESC{ 128 }; // Number of UTF-16 characters reserved for the description string

	UINT32	uiSavedGameVersion;
	char zGameVersionNumber[GAME_VERSION_LENGTH];

	ST::string sSavedGameDesc;

	/* (vanilla) UINT32	uiFlags; */

	//The following will be used to quickly access info to display in the save/load screen
	UINT32	uiDay;
	UINT8		ubHour;
	UINT8		ubMin;
	SGPSector	sSector;
	UINT8		ubNumOfMercsOnPlayersTeam;
	INT32		iCurrentBalance;

	UINT32	uiCurrentScreen;

	BOOLEAN	fAlternateSector;

	BOOLEAN	fWorldLoaded;

	UINT8		ubLoadScreenID;		//The load screen that should be used when loading the saved game

	GAME_OPTIONS	sInitialGameOptions;	//need these in the header so we can get the info from it on the save load screen.

	UINT32	uiRandom;
	UINT32  uiSaveStateSize;

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

ST::string GetSaveGamePath(const ST::string &saveName);
BOOLEAN HasSaveGameExtension(const ST::string &fileName);

ST::string GetAutoSaveName(uint32_t index);
BOOLEAN IsAutoSaveName(const ST::string &saveName);

ST::string GetQuickSaveName();
BOOLEAN IsQuickSaveName(const ST::string &saveName);

ST::string GetErrorSaveName();
BOOLEAN IsErrorSaveName(const ST::string &saveName);

BOOLEAN SaveGame(const ST::string &saveName, const ST::string& gameDesc);
void    LoadSavedGame(const ST::string &saveName);
void BackupSavedGame(const ST::string &saveName);

void SaveFilesToSavedGame(ST::string const& SrcFileName, HWFILE);
void LoadFilesFromSavedGame(ST::string const& SrcFileName, HWFILE);

void GetBestPossibleSectorXYZValues(SGPSector& sSector);

void SaveMercPath(HWFILE, PathSt const* head);
void LoadMercPath(HWFILE, PathSt** head);

extern UINT32 guiLastSaveGameNum;
INT8		GetNextIndexForAutoSave();

extern UINT32 guiJA2EncryptionSet;

// IMP save and import profile
ST::string IMPSavedProfileCreateFilename(const ST::string& nickname);
bool IMPSavedProfileDoesFileExist(const ST::string& nickname);
SGPFile* IMPSavedProfileOpenFileForRead(const ST::string& nickname);
int IMPSavedProfileLoadMercProfile(const ST::string& nickname);
void IMPSavedProfileLoadInventory(const ST::string& nickname, SOLDIERTYPE *pSoldier);

class SaveGameInfo {
	public:
		SaveGameInfo() {
			this->savedGameHeader = SAVED_GAME_HEADER{};
		};
		SaveGameInfo(ST::string name, HWFILE file);
		const SAVED_GAME_HEADER& header() const {
			return savedGameHeader;
		};
		const std::vector<std::pair<ST::string, ST::string>>& mods() const {
			return enabledMods;
		};
		const ST::string& name() const {
			return saveName;
		}
	private:
		ST::string saveName;
		SAVED_GAME_HEADER savedGameHeader;
		std::vector<std::pair<ST::string, ST::string>> enabledMods;
};

#endif
