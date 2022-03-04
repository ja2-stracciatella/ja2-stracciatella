#ifndef _GAME_SETTINGS__H_
#define _GAME_SETTINGS__H_

#include "VanillaDataStructures.h"
#include "MessageBoxScreen.h"

#include <string_theory/string>


//If you add any options, MAKE sure you add the corresponding string to the Options Screen string array
enum
{
	TOPTION_SPEECH,
	TOPTION_MUTE_CONFIRMATIONS,
	TOPTION_SUBTITLES,
	TOPTION_KEY_ADVANCE_SPEECH,
	TOPTION_ANIMATE_SMOKE,
//	TOPTION_HIDE_BULLETS,
//	TOPTION_CONFIRM_MOVE,
	TOPTION_BLOOD_N_GORE,
	TOPTION_DONT_MOVE_MOUSE,
	TOPTION_OLD_SELECTION_METHOD,
	TOPTION_ALWAYS_SHOW_MOVEMENT_PATH,


//	TOPTION_TIME_LIMIT_TURNS,			//moved to the game init screen

	TOPTION_SHOW_MISSES,

	TOPTION_RTCONFIRM,


//	TOPTION_DISPLAY_ENEMY_INDICATOR,		//Displays the number of enemies seen by the merc, ontop of their portrait
	TOPTION_SLEEPWAKE_NOTIFICATION,

	TOPTION_USE_METRIC_SYSTEM,		//If set, uses the metric system

	TOPTION_MERC_ALWAYS_LIGHT_UP,

	TOPTION_SMART_CURSOR,

	TOPTION_SNAP_CURSOR_TO_DOOR,

	TOPTION_GLOW_ITEMS,
	TOPTION_TOGGLE_TREE_TOPS,
	TOPTION_TOGGLE_WIREFRAME,
	TOPTION_3D_CURSOR,

	NUM_GAME_OPTIONS,				//Toggle up this will be able to be Toggled by the player

	//These options will NOT be toggable by the Player
	TOPTION_MERC_CASTS_LIGHT = NUM_GAME_OPTIONS,
	TOPTION_HIDE_BULLETS,
	TOPTION_TRACKING_MODE,

	NUM_ALL_GAME_OPTIONS,
};


struct GAME_SETTINGS
{
	INT8				bLastSavedGameSlot;							// The last saved game number goes in here
	ST::string sCurrentSavedGameName; // The name of the currently loaded Savegame goes here.
	ST::string sCurrentSavedGameDescription; // The description of the currently loaded Savegame goes here.

	//The following are set from the status of the toggle boxes in the Options Screen
	UINT8				fOptions[ NUM_ALL_GAME_OPTIONS ];

	UINT32			uiMeanwhileScenesSeenFlags;

	BOOLEAN			fHideHelpInAllScreens;

	UINT8				ubSizeOfDisplayCover;
	UINT8				ubSizeOfLOS;
};


//Enums for the difficulty levels
enum
{
	DIF_LEVEL_EASY = 1,
	DIF_LEVEL_MEDIUM = 2,
	DIF_LEVEL_HARD = 3,	
	NUM_DIF_LEVELS = DIF_LEVEL_HARD
};

//Enums for the difficulty levels
enum
{
	DIF_CAN_SAVE,
	DIF_IRON_MAN,
	DIF_DEAD_IS_DEAD,
};


struct GAME_OPTIONS
{
	BOOLEAN fGunNut;
	BOOLEAN	fSciFi;
	UINT8		ubDifficultyLevel;
	BOOLEAN	fTurnTimeLimit;
	UINT8		ubGameSaveMode;
};

//This structure will contain general Ja2 settings  NOT individual game settings.
extern GAME_SETTINGS gGameSettings;


//This structure will contain the Game options set at the beginning of the game.
extern GAME_OPTIONS gGameOptions;


void SaveGameSettings(void);
void LoadGameSettings(void);

void InitGameOptions(void);

void DisplayGameSettings(void);

bool MeanwhileSceneSeen(UINT8 meanwhile_id);
void SetMeanwhileSceneSeen(UINT8 meanwhile_id);

BOOLEAN CanGameBeSaved(void);

void DoDeadIsDeadSaveIfNecessary(void);

#endif
