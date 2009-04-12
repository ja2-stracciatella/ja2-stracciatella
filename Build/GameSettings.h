#ifndef _GAME_SETTINGS__H_
#define _GAME_SETTINGS__H_

#include "MessageBoxScreen.h"


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

	UINT8				ubMusicVolumeSetting;
	UINT8				ubSoundEffectsVolume;
	UINT8				ubSpeechVolume;

	//The following are set from the status of the toggle boxes in the Options Screen
	UINT8				fOptions[ NUM_ALL_GAME_OPTIONS ];

	char zVersionNumber[16];

	UINT32			uiSettingsVersionNumber;
	UINT32			uiMeanwhileScenesSeenFlags;

	BOOLEAN			fHideHelpInAllScreens;

	BOOLEAN				fUNUSEDPlayerFinishedTheGame; // JA2Gold: for UB compatibility // XXX HACK000B
	UINT8				ubSizeOfDisplayCover;
	UINT8				ubSizeOfLOS;

	UINT8		ubFiller[17]; // XXX HACK000B
};
CASSERT(sizeof(GAME_SETTINGS) == 76)


//Enums for the difficulty levels
enum
{
	DIF_LEVEL_ZERO,
	DIF_LEVEL_EASY,
	DIF_LEVEL_MEDIUM,
	DIF_LEVEL_HARD,
	DIF_LEVEL_FOUR,
};


struct GAME_OPTIONS
{
	BOOLEAN fGunNut;
	BOOLEAN	fSciFi;
	UINT8		ubDifficultyLevel;
	BOOLEAN	fTurnTimeLimit;
	BOOLEAN	fIronManMode;

	UINT8		ubFiller[7]; // XXX HACK000B
};
CASSERT(sizeof(GAME_OPTIONS) == 12)

//This structure will contain general Ja2 settings  NOT individual game settings.
extern GAME_SETTINGS gGameSettings;


//This structure will contain the Game options set at the beginning of the game.
extern GAME_OPTIONS gGameOptions;


void SaveGameSettings(void);
void LoadGameSettings(void);

void InitGameOptions(void);

void DisplayGameSettings(void);

BOOLEAN MeanwhileSceneSeen( UINT8 ubMeanwhile );

BOOLEAN SetMeanwhileSceneSeen( UINT8 ubMeanwhile );

BOOLEAN CanGameBeSaved(void);

void CDromEjectionErrorMessageBoxCallBack(MessageBoxReturnValue);

#endif
