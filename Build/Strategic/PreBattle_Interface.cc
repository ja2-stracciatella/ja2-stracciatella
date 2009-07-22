#include "Font.h"
#include "Input.h"
#include "MessageBoxScreen.h"
#include "PreBattle_Interface.h"
#include "Button_System.h"
#include "MouseSystem.h"
#include "Map_Screen_Interface.h"
#include "JAScreens.h"
#include "GameScreen.h"
#include "StrategicMap.h"
#include "Game_Clock.h"
#include "Music_Control.h"
#include "SysUtil.h"
#include "Font_Control.h"
#include "Timer.h"
#include "Queen_Command.h"
#include "Strategic_Movement.h"
#include "Strategic_Pathing.h"
#include "Text.h"
#include "PopUpBox.h"
#include "Player_Command.h"
#include "Cursors.h"
#include "Auto_Resolve.h"
#include "Sound_Control.h"
#include "English.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Overhead.h"
#include "Tactical_Placement_GUI.h"
#include "Timer_Control.h"
#include "Town_Militia.h"
#include "Campaign.h"
#include "GameSettings.h"
#include "Random.h"
#include "Creature_Spreading.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Map_Screen_Helicopter.h"
#include "MapScreen.h"
#include "Quests.h"
#include "Map_Screen_Interface_Border.h"
#include "Cheats.h"
#include "Strategic_Status.h"
#include "Interface_Control.h"
#include "Strategic_Town_Loyalty.h"
#include "Squads.h"
#include "Assignments.h"
#include "Soldier_Macros.h"
#include "History.h"
#include "VObject.h"
#include "Vehicles.h"
#include "Video.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "Render_Dirty.h"
#include "VSurface.h"


extern BOOLEAN gfDelayAutoResolveStart;

#ifdef JA2BETAVERSION
extern BOOLEAN gfExitViewer;
#endif


BOOLEAN gfTacticalTraversal = FALSE;
GROUP *gpTacticalTraversalGroup = NULL;
SOLDIERTYPE *gpTacticalTraversalChosenSoldier = NULL;


BOOLEAN gfAutomaticallyStartAutoResolve = FALSE;
BOOLEAN gfAutoAmbush = FALSE;
BOOLEAN gfHighPotentialForAmbush = FALSE;
BOOLEAN gfGotoSectorTransition = FALSE;
BOOLEAN gfEnterAutoResolveMode = FALSE;
BOOLEAN gfEnteringMapScreenToEnterPreBattleInterface = FALSE;
BOOLEAN gfIgnoreAllInput = TRUE;

enum //GraphicIDs for the panel
{
	MAINPANEL,
	TITLE_BAR_PIECE,
	TOP_COLUMN,
	BOTTOM_COLUMN,
	UNINVOLVED_HEADER
};

//The start of the black space
#define TOP_Y							113
//The end of the black space
#define BOTTOM_Y					349
//The internal height of the uninvolved panel
#define INTERNAL_HEIGHT		27
//The actual height of the uninvolved panel
#define ACTUAL_HEIGHT			34
//The height of each row
#define ROW_HEIGHT				10

BOOLEAN gfDisplayPotentialRetreatPaths = FALSE;

GROUP *gpBattleGroup = NULL;


MOUSE_REGION PBInterfaceBlanket;
BOOLEAN gfPreBattleInterfaceActive = FALSE;
GUIButtonRef iPBButton[3];
static BUTTON_PICS* iPBButtonImage[3];
static SGPVObject* uiInterfaceImages;
BOOLEAN gfRenderPBInterface;
BOOLEAN	gfPBButtonsHidden;
BOOLEAN fDisableMapInterfaceDueToBattle = FALSE;

BOOLEAN gfBlinkHeader;

UINT32 guiNumInvolved;
UINT32 guiNumUninvolved;

//SAVE START

//Using the ESC key in the PBI will get rid of the PBI and go back to mapscreen, but
//only if the PBI isn't persistant (!gfPersistantPBI).
BOOLEAN gfPersistantPBI = FALSE;

//Contains general information about the type of encounter the player is faced with.  This
//determines whether or not you can autoresolve the battle or even retreat.  This code
//dictates the header that is used at the top of the PBI.
UINT8 gubEnemyEncounterCode = NO_ENCOUNTER_CODE;

//The autoresolve during tactical battle option needs more detailed information than the
//gubEnemyEncounterCode can provide.  The explicit version contains possibly unique codes
//for reasons not normally used in the PBI.  For example, if we were fighting the enemy
//in a normal situation, then shot at a civilian, the civilians associated with the victim
//would turn hostile, which would disable the ability to autoresolve the battle.
BOOLEAN gubExplicitEnemyEncounterCode = NO_ENCOUNTER_CODE;

//Location of the current battle (determines where the animated icon is blitted) and if the
//icon is to be blitted.
BOOLEAN gfBlitBattleSectorLocator = FALSE;

UINT8 gubPBSectorX = 0;
UINT8 gubPBSectorY = 0;
UINT8 gubPBSectorZ = 0;

BOOLEAN gfCantRetreatInPBI = FALSE;
//SAVE END

BOOLEAN gfUsePersistantPBI;


#ifdef JA2BETAVERSION

//The group is passed so we can extract the sector location
static void ValidateAndCorrectInBattleCounters(GROUP* pLocGroup)
{
	SECTORINFO *pSector;
	UINT8 ubSectorID;
	UINT8 ubInvalidGroups = 0;

	if( !pLocGroup->ubSectorZ )
	{
		FOR_ALL_ENEMY_GROUPS(pGroup)
		{
			if( pGroup->ubSectorX == pLocGroup->ubSectorX && pGroup->ubSectorY == pLocGroup->ubSectorY )
			{
				if( pGroup->pEnemyGroup->ubAdminsInBattle || pGroup->pEnemyGroup->ubTroopsInBattle || pGroup->pEnemyGroup->ubElitesInBattle )
				{
					ubInvalidGroups++;
					pGroup->pEnemyGroup->ubAdminsInBattle = 0;
					pGroup->pEnemyGroup->ubTroopsInBattle = 0;
					pGroup->pEnemyGroup->ubElitesInBattle = 0;
				}
			}
		}
	}

	ubSectorID = (UINT8)SECTOR( pLocGroup->ubSectorX, pLocGroup->ubSectorY );
	pSector = &SectorInfo[ ubSectorID ];

	if( ubInvalidGroups || pSector->ubAdminsInBattle || pSector->ubTroopsInBattle || pSector->ubElitesInBattle || pSector->ubCreaturesInBattle )
	{
		wchar_t str[512];
		swprintf(str, lengthof(str), L"Strategic info warning:  Sector 'in battle' counters are not clear when they should be.  "
									 L"If you can provide information on how a previous battle was resolved here or nearby patrol "
									 L"(auto resolve, tactical battle, cheat keys, or retreat),"
									 L"please forward that info (no data files necessary) as well as the following code (very important):  "
									 L"G(%02d:%c%d_b%d) A(%02d:%02d) T(%02d:%02d) E(%02d:%02d) C(%02d:%02d)",
									 ubInvalidGroups, pLocGroup->ubSectorY + 'A' - 1, pLocGroup->ubSectorX, pLocGroup->ubSectorZ,
									 pSector->ubNumAdmins, pSector->ubAdminsInBattle,
									 pSector->ubNumTroops, pSector->ubTroopsInBattle,
									 pSector->ubNumElites, pSector->ubElitesInBattle,
									 pSector->ubNumCreatures, pSector->ubCreaturesInBattle );
		DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
		pSector->ubAdminsInBattle = 0;
		pSector->ubTroopsInBattle = 0;
		pSector->ubElitesInBattle = 0;
		pSector->ubCreaturesInBattle = 0;
	}
}
#endif


static void MakeButton(UINT idx, INT16 x, const wchar_t* text, GUI_CALLBACK click)
{
	GUIButtonRef const btn = QuickCreateButton(iPBButtonImage[idx], x, 54, MSYS_PRIORITY_HIGHEST - 2, click);
	iPBButton[idx] = btn;

	btn->SpecifyGeneralTextAttributes(text, BLOCKFONT, FONT_BEIGE, 141);
	btn->SpecifyHilitedTextColors(FONT_WHITE, FONT_NEARBLACK);
	btn->SpecifyTextOffsets(8, 7, TRUE);
	btn->SpecifyTextWrappedWidth(51);
	btn->AllowDisabledFastHelp();
	btn->Hide();
}


static void AutoResolveBattleCallback(GUI_BUTTON* btn, INT32 reason);
static void CheckForRobotAndIfItsControlled(void);
static void DoTransitionFromMapscreenToPreBattleInterface(void);
static void GoToSectorCallback(GUI_BUTTON* btn, INT32 reason);
static void RetreatMercsCallback(GUI_BUTTON* btn, INT32 reason);


void InitPreBattleInterface( GROUP *pBattleGroup, BOOLEAN fPersistantPBI )
{
	UINT8 ubGroupID = 0;
	UINT8 ubNumStationaryEnemies = 0;
	UINT8 ubNumMobileEnemies = 0;
	UINT8 ubNumMercs;
	BOOLEAN fUsePluralVersion = FALSE;
	INT8	bBestExpLevel = 0;
	BOOLEAN fRetreatAnOption = TRUE;
	SECTORINFO *pSector;


	// ARM: Feb01/98 - Cancel out of mapscreen movement plotting if PBI subscreen is coming up
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}

	if( gfPreBattleInterfaceActive )
		return;

	gfPersistantPBI = fPersistantPBI;

	if( gfPersistantPBI )
	{
		gfBlitBattleSectorLocator = TRUE;
		gfBlinkHeader = FALSE;

		#ifdef JA2BETAVERSION
			if( pBattleGroup )
			{
				ValidateAndCorrectInBattleCounters( pBattleGroup );
			}
		#endif

		//	InitializeTacticalStatusAtBattleStart();
		// CJC, Oct 5 98: this is all we should need from InitializeTacticalStatusAtBattleStart()
		if( gubEnemyEncounterCode != BLOODCAT_AMBUSH_CODE && gubEnemyEncounterCode != ENTERING_BLOODCAT_LAIR_CODE )
		{
			if (!CheckFact(FACT_FIRST_BATTLE_FOUGHT, 0))
			{
				SetFactTrue( FACT_FIRST_BATTLE_BEING_FOUGHT );
			}
		}

		//If we are currently in the AI Viewer development utility, then remove it first.  It automatically
		//returns to the mapscreen upon removal, which is where we want to go.
		#ifdef JA2BETAVERSION
			if( guiCurrentScreen == AIVIEWER_SCREEN )
			{
				gfExitViewer = TRUE;
				gpBattleGroup = pBattleGroup;
				gfEnteringMapScreen = TRUE;
				gfEnteringMapScreenToEnterPreBattleInterface = TRUE;
				gfUsePersistantPBI = TRUE;
				return;
			}
		#endif

		// ATE: Added check for fPersistantPBI = TRUE if pBattleGroup == NULL
		// Searched code and saw that this condition only happens for creatures
			// fixing a bug
		//if( guiCurrentScreen == GAME_SCREEN && pBattleGroup )
		if( guiCurrentScreen == GAME_SCREEN && ( pBattleGroup || fPersistantPBI ) )
		{
			gpBattleGroup = pBattleGroup;
			gfEnteringMapScreen = TRUE;
			gfEnteringMapScreenToEnterPreBattleInterface = TRUE;
			gfUsePersistantPBI = TRUE;
			return;
		}

		if( gfTacticalTraversal && (pBattleGroup == gpTacticalTraversalGroup || gbWorldSectorZ > 0) )
		{
			return;
		}

		// reset the help text for mouse regions
		gMapStatusBarsRegion.SetFastHelpText(L"");

		gfDisplayPotentialRetreatPaths = FALSE;

		gpBattleGroup = pBattleGroup;

		//calc sector values
		if( gpBattleGroup )
		{
			gubPBSectorX = gpBattleGroup->ubSectorX;
			gubPBSectorY = gpBattleGroup->ubSectorY;
			gubPBSectorZ = gpBattleGroup->ubSectorZ;

			fMapPanelDirty = TRUE;
		}
		else
		{
			gubPBSectorX = (UINT8)SECTORX( gubSectorIDOfCreatureAttack );
			gubPBSectorY = (UINT8)SECTORY( gubSectorIDOfCreatureAttack );
			gubPBSectorZ = 0;
		}
	}
	else
	{ //calculate the non-persistant situation
		gfBlinkHeader = TRUE;

		if( HostileCiviliansPresent() )
		{ //There are hostile civilians, so no autoresolve allowed.
			gubExplicitEnemyEncounterCode = HOSTILE_CIVILIANS_CODE;
		}
		else if( HostileBloodcatsPresent() )
		{ //There are bloodcats in the sector, so no autoresolve allowed
			gubExplicitEnemyEncounterCode = HOSTILE_BLOODCATS_CODE;
		}
		else if( gbWorldSectorZ )
		{ //We are underground, so no autoresolve allowed
			pSector = &SectorInfo[ SECTOR( gubPBSectorX, gubPBSectorY ) ];
			if( pSector->ubCreaturesInBattle )
			{
				gubExplicitEnemyEncounterCode = FIGHTING_CREATURES_CODE;
			}
			else if( pSector->ubAdminsInBattle || pSector->ubTroopsInBattle || pSector->ubElitesInBattle )
			{
				gubExplicitEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
			}
		}
		else if( gubEnemyEncounterCode == ENTERING_ENEMY_SECTOR_CODE ||
						 gubEnemyEncounterCode == ENEMY_ENCOUNTER_CODE ||
						 gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
						 gubEnemyEncounterCode == ENEMY_INVASION_CODE ||
						 gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
						 gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE ||
						 gubEnemyEncounterCode == CREATURE_ATTACK_CODE )
		{ //use same code
			gubExplicitEnemyEncounterCode = gubEnemyEncounterCode;
		}
		else
		{
			#ifdef JA2BETAVERSION
				DoScreenIndependantMessageBox( L"Can't determine valid reason for battle indicator.  Please try to provide information as to when and why this indicator first appeared and send whatever files that may help.", MSG_BOX_FLAG_OK, NULL );
			#endif
			gfBlitBattleSectorLocator = FALSE;
			return;
		}
	}

	fMapScreenBottomDirty = TRUE;
	ChangeSelectedMapSector( gubPBSectorX, gubPBSectorY, gubPBSectorZ );
	RenderMapScreenInterfaceBottom();

	//If we are currently in tactical, then set the flag to automatically bring up the mapscreen.
	if( guiCurrentScreen == GAME_SCREEN )
	{
		gfEnteringMapScreen = TRUE;
	}

	if( !fShowTeamFlag )
	{
		ToggleShowTeamsMode();
	}

	//Define the blanket region to cover all of the other regions used underneath the panel.
	MSYS_DefineRegion( &PBInterfaceBlanket, 0, 0, 261, 359, MSYS_PRIORITY_HIGHEST - 5, 0, 0, 0 );

	//Create the panel
	const char* const ImageFile = GetMLGFilename(MLG_PREBATTLEPANEL);
	uiInterfaceImages = AddVideoObjectFromFile(ImageFile);

	//Create the 3 buttons
	iPBButtonImage[0] = LoadButtonImage( "INTERFACE/PreBattleButton.sti", -1, 0, -1, 1, -1 );
	iPBButtonImage[1] = UseLoadedButtonImage( iPBButtonImage[ 0 ], -1, 0, -1, 1, -1 );
	iPBButtonImage[2] = UseLoadedButtonImage( iPBButtonImage[ 0 ], -1, 0, -1, 1, -1 );

	MakeButton(0,  27, gpStrategicString[STR_PB_AUTORESOLVE_BTN],  AutoResolveBattleCallback);
	MakeButton(1,  98, gpStrategicString[STR_PB_GOTOSECTOR_BTN],   GoToSectorCallback);
	MakeButton(2, 169, gpStrategicString[STR_PB_RETREATMERCS_BTN], RetreatMercsCallback);

	gfPBButtonsHidden = TRUE;

	// ARM: this must now be set before any calls utilizing the GetCurrentBattleSectorXYZ() function
	gfPreBattleInterfaceActive = TRUE;


	CheckForRobotAndIfItsControlled();

	// wake everyone up
	WakeUpAllMercsInSectorUnderAttack( );

	//Count the number of players involved or not involved in this battle
	guiNumUninvolved = 0;
	guiNumInvolved = 0;
	CFOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bLife != 0 && !(s->uiStatusFlags & SOLDIER_VEHICLE))
		{
			if (PlayerMercInvolvedInThisCombat(s))
			{
				// involved
				if( !ubGroupID )
				{ //Record the first groupID.  If there are more than one group in this battle, we
					//can detect it by comparing the first value with future values.  If we do, then
					//we set a flag which determines whether to use the singular help text or plural version
					//for the retreat button.
					ubGroupID = s->ubGroupID;
					if( !gpBattleGroup )
						gpBattleGroup = GetGroup( ubGroupID );
					if (bBestExpLevel > s->bExpLevel) bBestExpLevel = s->bExpLevel;
					if (s->ubPrevSectorID == 255)
					{ //Not able to retreat (calculate it for group)
						GROUP *pTempGroup;
						pTempGroup = GetGroup( ubGroupID );
						Assert( pTempGroup );
						CalculateGroupRetreatSector( pTempGroup );
					}
				}
				else if (ubGroupID != s->ubGroupID)
				{
					fUsePluralVersion = TRUE;
				}
				guiNumInvolved ++;
			}
			else
				guiNumUninvolved++;
		}
	}

	ubNumStationaryEnemies = NumStationaryEnemiesInSector( gubPBSectorX, gubPBSectorY );
	ubNumMobileEnemies = NumMobileEnemiesInSector( gubPBSectorX, gubPBSectorY );
	ubNumMercs = PlayerMercsInSector( gubPBSectorX, gubPBSectorY, gubPBSectorZ );

	if( gfPersistantPBI )
	{
		if( !pBattleGroup )
		{ //creature's attacking!
			gubEnemyEncounterCode = CREATURE_ATTACK_CODE;
		}
		else if( gpBattleGroup->fPlayer )
		{
			if( gubEnemyEncounterCode != BLOODCAT_AMBUSH_CODE && gubEnemyEncounterCode != ENTERING_BLOODCAT_LAIR_CODE )
			{
				if( ubNumStationaryEnemies )
				{
					gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
				}
				else
				{
					gubEnemyEncounterCode = ENEMY_ENCOUNTER_CODE;

					//Don't consider ambushes until the player has reached 25% (normal) progress
					if( gfHighPotentialForAmbush )
					{
						if( Chance( 90 ) )
						{
							gubEnemyEncounterCode = ENEMY_AMBUSH_CODE;
						}
					}
					else if( gfAutoAmbush && ubNumMobileEnemies > ubNumMercs )
					{
						gubEnemyEncounterCode = ENEMY_AMBUSH_CODE;
					}
					else if( WhatPlayerKnowsAboutEnemiesInSector( gubPBSectorX, gubPBSectorY ) == KNOWS_NOTHING &&
									 CurrentPlayerProgressPercentage() >= 30 - gGameOptions.ubDifficultyLevel * 5 )
					{ //if the enemy outnumbers the players, then there is a small chance of the enemies ambushing the group
						if( ubNumMobileEnemies > ubNumMercs )
						{
							INT32 iChance;
							pSector = &SectorInfo[ SECTOR( gubPBSectorX, gubPBSectorY ) ];
							if( !(pSector->uiFlags & SF_ALREADY_VISITED) )
							{
								iChance = (UINT8)( 4 - bBestExpLevel + 2 * gGameOptions.ubDifficultyLevel + CurrentPlayerProgressPercentage() / 10 );
								if( pSector->uiFlags & SF_ENEMY_AMBUSH_LOCATION )
								{
									iChance += 20;
								}
								if( gfCantRetreatInPBI )
								{
									iChance += 20;
								}
								if( (INT32)PreRandom( 100 ) < iChance )
								{
									gubEnemyEncounterCode = ENEMY_AMBUSH_CODE;
								}
							}
						}
					}
				}
			}
		}
		else
		{ //Are enemies invading a town, or just encountered the player.
			if( GetTownIdForSector( gubPBSectorX, gubPBSectorY ) )
			{
				gubEnemyEncounterCode = ENEMY_INVASION_CODE;
			}
			else
			{
				switch( SECTOR( gubPBSectorX, gubPBSectorY ) )
				{
					case SEC_D2:
					case SEC_D15:
					case SEC_G8:
						//SAM sites not in towns will also be considered to be important
						gubEnemyEncounterCode = ENEMY_INVASION_CODE;
						break;
					default:
						gubEnemyEncounterCode = ENEMY_ENCOUNTER_CODE;
						break;
				}
			}
		}
	}

	gfHighPotentialForAmbush = FALSE;

	if( gfAutomaticallyStartAutoResolve )
	{
		DisableButton( iPBButton[1] );
		DisableButton( iPBButton[2] );
	}

	gfRenderPBInterface = TRUE;
	MSYS_SetCurrentCursor( CURSOR_NORMAL );
	StopTimeCompression();

	// hide all visible boxes
	HideAllBoxes( );
	fShowAssignmentMenu = FALSE;
	fShowContractMenu = FALSE;
	DisableTeamInfoPanels();
	if (giMapContractButton) giMapContractButton->Hide();
	if (giCharInfoButton[0]) giCharInfoButton[0]->Hide();
	if (giCharInfoButton[1]) giCharInfoButton[1]->Hide();

	if( gubEnemyEncounterCode == ENEMY_ENCOUNTER_CODE )
	{ //we know how many enemies are here, so until we leave the sector, we will continue to display the value.
		//the flag will get cleared when time advances after the fEnemyInSector flag is clear.
		pSector = &SectorInfo[ SECTOR( gubPBSectorX, gubPBSectorY ) ];

		// ALWAYS use these 2 statements together, without setting the boolean, the flag will never be cleaned up!
		pSector->uiFlags |= SF_PLAYER_KNOWS_ENEMIES_ARE_HERE;
		gfResetAllPlayerKnowsEnemiesFlags = TRUE;
	}

	//Set up fast help for buttons depending on the state of the button, and disable buttons
	//when necessary.
	if( gfPersistantPBI )
	{
		wchar_t const* autoresolve_help;
		if( gubEnemyEncounterCode == ENTERING_ENEMY_SECTOR_CODE ||
				gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE )
		{ //Don't allow autoresolve for player initiated invasion battle types
			DisableButton( iPBButton[ 0 ] );
			autoresolve_help = gpStrategicString[STR_PB_DISABLED_AUTORESOLVE_FASTHELP];
		}
		else if( gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
						 gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE )
		{ //Don't allow autoresolve for ambushes
			DisableButton( iPBButton[ 0 ] );
			autoresolve_help = gzNonPersistantPBIText[3];
		}
		else
		{
			autoresolve_help = gpStrategicString[STR_PB_AUTORESOLVE_FASTHELP];
		}
		iPBButton[0]->SetFastHelpText(autoresolve_help);
		iPBButton[1]->SetFastHelpText(gpStrategicString[STR_PB_GOTOSECTOR_FASTHELP]);
		if( gfAutomaticallyStartAutoResolve )
		{
			DisableButton( iPBButton[ 1 ] );
		}
		if( gfCantRetreatInPBI )
		{
			gfCantRetreatInPBI = FALSE;
			fRetreatAnOption = FALSE;
		}

		wchar_t const* retreat_help;
		if( gfAutomaticallyStartAutoResolve || !fRetreatAnOption ||
				gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
				gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
				gubEnemyEncounterCode == CREATURE_ATTACK_CODE )
		{
			DisableButton( iPBButton[ 2 ] );
			retreat_help = gzNonPersistantPBIText[9];
		}
		else
		{
			if( !fUsePluralVersion )
			{
				retreat_help = gpStrategicString[STR_BP_RETREATSINGLE_FASTHELP];
			}
			else
			{
				retreat_help = gpStrategicString[STR_BP_RETREATPLURAL_FASTHELP];
			}
		}
		iPBButton[2]->SetFastHelpText(retreat_help);
	}
	else
	{ //use the explicit encounter code to determine what get's disable and the associated help text that is used.

		//First of all, the retreat button is always disabled seeing a battle is in progress.
		DisableButton( iPBButton[ 2 ] );
		iPBButton[2]->SetFastHelpText(gzNonPersistantPBIText[0]);
		iPBButton[1]->SetFastHelpText(gzNonPersistantPBIText[1]);
		switch( gubExplicitEnemyEncounterCode )
		{
			case CREATURE_ATTACK_CODE:
			case ENEMY_ENCOUNTER_CODE:
			case ENEMY_INVASION_CODE:
				iPBButton[0]->SetFastHelpText(gzNonPersistantPBIText[2]);
				break;
			case ENTERING_ENEMY_SECTOR_CODE:
				DisableButton( iPBButton[ 0 ] );
				iPBButton[0]->SetFastHelpText(gzNonPersistantPBIText[3]);
				break;
			case ENEMY_AMBUSH_CODE:
				DisableButton( iPBButton[ 0 ] );
				iPBButton[0]->SetFastHelpText(gzNonPersistantPBIText[4]);
				break;
			case FIGHTING_CREATURES_CODE:
				DisableButton( iPBButton[ 0 ] );
				iPBButton[0]->SetFastHelpText(gzNonPersistantPBIText[5]);
				break;
			case HOSTILE_CIVILIANS_CODE:
				DisableButton( iPBButton[ 0 ] );
				iPBButton[0]->SetFastHelpText(gzNonPersistantPBIText[6]);
				break;
			case HOSTILE_BLOODCATS_CODE:
			case BLOODCAT_AMBUSH_CODE:
			case ENTERING_BLOODCAT_LAIR_CODE:
				DisableButton( iPBButton[ 0 ] );
				iPBButton[0]->SetFastHelpText(gzNonPersistantPBIText[7]);
				break;
		}
	}

	//Disable the options button when the auto resolve  screen comes up
	EnableDisAbleMapScreenOptionsButton( FALSE );

	SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

	DoTransitionFromMapscreenToPreBattleInterface();
}


static void DoTransitionFromMapscreenToPreBattleInterface(void)
{
	UINT32 uiStartTime, uiCurrTime;
	INT32 iPercentage, iFactor;
	UINT32 uiTimeRange;
	INT16 sStartLeft, sEndLeft, sStartTop, sEndTop;
	INT32 iLeft, iTop, iWidth, iHeight;
	BOOLEAN fEnterAutoResolveMode = FALSE;

	PauseTime( FALSE );

	iWidth = 261;
	iHeight = 359;

	uiTimeRange = 1000;
	iPercentage = 0;
	uiStartTime = GetClock();

	GetScreenXYFromMapXY( gubPBSectorX, gubPBSectorY, &sStartLeft, &sStartTop );
	sStartLeft += MAP_GRID_X / 2;
	sStartTop += MAP_GRID_Y / 2;
	sEndLeft = 131;
	sEndTop = 180;

	//save the mapscreen buffer
	BltVideoSurface(guiEXTRABUFFER, FRAME_BUFFER, 0, 0, NULL);

	if( gfEnterAutoResolveMode )
	{ //If we are intending on immediately entering autoresolve, change the global flag so that it will actually
		//render the interface once.  If gfEnterAutoResolveMode is clear, then RenderPreBattleInterface() won't do
		//anything.
		fEnterAutoResolveMode = TRUE;
		gfEnterAutoResolveMode = FALSE;
	}
	//render the prebattle interface
	RenderPreBattleInterface();

	gfIgnoreAllInput = TRUE;

	if( fEnterAutoResolveMode )
	{ //Change it back
		gfEnterAutoResolveMode = TRUE;
	}

	BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 27, 54, 209, 32 );
	RenderButtons();
	BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 27, 54, 209, 32 );
	gfRenderPBInterface = TRUE;

	//hide the prebattle interface
	BlitBufferToBuffer( guiEXTRABUFFER, FRAME_BUFFER, 0, 0, 261, 359 );
	PlayJA2SampleFromFile("SOUNDS/Laptop power up (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
	InvalidateScreen();
	RefreshScreen();

	SGPBox const PBIRect = { 0, 0, 261, 359 };
	while( iPercentage < 100  )
	{
		uiCurrTime = GetClock();
		iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
		iPercentage = MIN( iPercentage, 100 );

		//Factor the percentage so that it is modified by a gravity falling acceleration effect.
		iFactor = (iPercentage - 50) * 2;
		if( iPercentage < 50 )
			iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
		else
			iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

		//Calculate the center point.
		iLeft = sStartLeft - (sStartLeft-sEndLeft+1) * iPercentage / 100;
		if( sStartTop > sEndTop )
			iTop = sStartTop - (sStartTop-sEndTop+1) * iPercentage / 100;
		else
			iTop = sStartTop + (sEndTop-sStartTop+1) * iPercentage / 100;

		SGPBox const DstRect =
		{
			iLeft - iWidth  * iPercentage / 200,
			iTop  - iHeight * iPercentage / 200,
			MAX(1, iWidth  * iPercentage / 100),
			MAX(1, iHeight * iPercentage / 100)
		};

		BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &PBIRect, &DstRect);

		InvalidateScreen();
		RefreshScreen();

		//Restore the previous rect.
		BlitBufferToBuffer(guiEXTRABUFFER, FRAME_BUFFER, DstRect.x, DstRect.y, DstRect.w + 1, DstRect.h + 1);
	}
	BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
}

void KillPreBattleInterface()
{
	if( !gfPreBattleInterfaceActive )
		return;

	fDisableMapInterfaceDueToBattle = FALSE;
	MSYS_RemoveRegion( &PBInterfaceBlanket );

	//The panel
	DeleteVideoObject(uiInterfaceImages);

	//The 3 buttons
	RemoveButton( iPBButton[0] );
	RemoveButton( iPBButton[1] );
	RemoveButton( iPBButton[2] );
	UnloadButtonImage( iPBButtonImage[0] );
	UnloadButtonImage( iPBButtonImage[1] );
	UnloadButtonImage( iPBButtonImage[2] );

	/*
	MSYS_RemoveRegion( &InvolvedRegion );
	if( guiNumUninvolved )
		MSYS_RemoveRegion( &UninvolvedRegion );
	*/

	gfPreBattleInterfaceActive = FALSE;

	//UpdateCharRegionHelpText( );

	// re draw affected regions
	fMapPanelDirty = TRUE;
	fTeamPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;
	gfDisplayPotentialRetreatPaths = FALSE;

	//Enable the options button when the auto resolve  screen comes up
	EnableDisAbleMapScreenOptionsButton( TRUE );

	ColorFillVideoSurfaceArea( guiSAVEBUFFER, 0, 0, 261, 359, 0 );

	EnableTeamInfoPanels();
	if (giMapContractButton) giMapContractButton->Show();
	if (giCharInfoButton[0]) giCharInfoButton[0]->Show();
	if (giCharInfoButton[1]) giCharInfoButton[1]->Show();
}


static void RenderPBHeader(INT32* piX, INT32* piWidth)
{
	INT32 x, width;
	UINT8 const foreground =
		!gfBlinkHeader             ? FONT_BEIGE :
		GetJA2Clock() % 1000 < 667 ? FONT_WHITE :
		FONT_LTRED;
	SetFontAttributes(FONT10ARIALBOLD, foreground);
	const wchar_t* str; // XXX HACK000E
	if( !gfPersistantPBI )
	{
		str = gzNonPersistantPBIText[8];
	}
	else switch( gubEnemyEncounterCode )
	{
		case ENEMY_INVASION_CODE:
			str = gpStrategicString[STR_PB_ENEMYINVASION_HEADER];
			break;
		case ENEMY_ENCOUNTER_CODE:
			str = gpStrategicString[STR_PB_ENEMYENCOUNTER_HEADER];
			break;
		case ENEMY_AMBUSH_CODE:
			str = gpStrategicString[STR_PB_ENEMYAMBUSH_HEADER];
			gfBlinkHeader = TRUE;
			break;
		case ENTERING_ENEMY_SECTOR_CODE:
			str = gpStrategicString[STR_PB_ENTERINGENEMYSECTOR_HEADER];
			break;
		case CREATURE_ATTACK_CODE:
			str = gpStrategicString[STR_PB_CREATUREATTACK_HEADER];
			gfBlinkHeader = TRUE;
			break;
		case BLOODCAT_AMBUSH_CODE:
			str = gpStrategicString[STR_PB_BLOODCATAMBUSH_HEADER];
			gfBlinkHeader = TRUE;
			break;
		case ENTERING_BLOODCAT_LAIR_CODE:
			str = gpStrategicString[STR_PB_ENTERINGBLOODCATLAIR_HEADER];
			break;

		default: abort(); // HACK000E
	}
	width = StringPixLength( str, FONT10ARIALBOLD );
	x = 130 - width / 2;
	MPrint(x, 4, str);
	InvalidateRegion( 0, 0, 231, 12 );
	*piX = x;
	*piWidth = width;
}


static wchar_t const* GetSoldierConditionInfo(SOLDIERTYPE const&);


void RenderPreBattleInterface()
{
	INT32 x;
	INT32 y;
	INT32 width;
	wchar_t str[100];
	wchar_t pSectorName[ 128 ];
	UINT8 ubJunk;
	//PLAYERGROUP *pPlayer;

	//This code determines if the cursor is inside the rectangle consisting of the
	//retreat button.  If it is inside, then we set up the variables so that the retreat
	//arrows get drawn in the mapscreen.
	GUIButtonRef const retreat = iPBButton[2];
	if (retreat->uiFlags & BUTTON_ENABLED)
	{
		BOOLEAN const fMouseInRetreatButtonArea =
			retreat->X() <= gusMouseXPos && gusMouseXPos <= retreat->BottomRightX() &&
			retreat->Y() <= gusMouseYPos && gusMouseYPos <= retreat->BottomRightY();
		if( fMouseInRetreatButtonArea != gfDisplayPotentialRetreatPaths )
		{
			gfDisplayPotentialRetreatPaths = fMouseInRetreatButtonArea;
			fMapPanelDirty = TRUE;
		}
	}

	if( gfRenderPBInterface )
	{
		// set font destinanation buffer to the save buffer
		SetFontDestBuffer(guiSAVEBUFFER);

		if( gfPBButtonsHidden )
		{
			ShowButton( iPBButton[0] );
			ShowButton( iPBButton[1] );
			ShowButton( iPBButton[2] );
			gfPBButtonsHidden = FALSE;
		}
		else
		{
			MarkAButtonDirty( iPBButton[ 0 ] );
			MarkAButtonDirty( iPBButton[ 1 ] );
			MarkAButtonDirty( iPBButton[ 2 ] );
		}

		gfRenderPBInterface = FALSE;
		const SGPVObject* const hVObject = uiInterfaceImages;
		//main panel
		BltVideoObject( guiSAVEBUFFER, hVObject, MAINPANEL, 0, 0);
		//main title

		RenderPBHeader( &x, &width );
		//now draw the title bars up to the text.
		for (INT32 i = x - 12; i > 20; i -= 10)
		{
			BltVideoObject( guiSAVEBUFFER, hVObject, TITLE_BAR_PIECE, i, 6);
		}
		for (INT32 i = x + width + 2; i < 231; i += 10)
		{
			BltVideoObject( guiSAVEBUFFER, hVObject, TITLE_BAR_PIECE, i, 6);
		}

		y = BOTTOM_Y - ACTUAL_HEIGHT - ROW_HEIGHT * MAX( guiNumUninvolved, 1 );
		BltVideoObject( guiSAVEBUFFER, hVObject, UNINVOLVED_HEADER, 8, y);

		SetFont( BLOCKFONT );
		SetFontForeground( FONT_BEIGE );
		const wchar_t* Location = gpStrategicString[STR_PB_LOCATION];
		width = StringPixLength(Location, BLOCKFONT);
		if( width > 64 )
		{
			SetFont( BLOCKFONTNARROW );
			width = StringPixLength(Location, BLOCKFONTNARROW);
		}
		MPrint(65 - width, 17, Location);

		SetFont( BLOCKFONT );
		const wchar_t* Encounter;
		if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
		{
			Encounter = gpStrategicString[STR_PB_ENEMIES];
		}
		else if( gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE || gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE )
		{
			Encounter = gpStrategicString[STR_PB_BLOODCATS];
		}
		else
		{
			Encounter = gpStrategicString[STR_PB_CREATURES];
		}
		width = StringPixLength(Encounter, BLOCKFONT);
		if( width > 52 )
		{
			SetFont( BLOCKFONTNARROW );
			width = StringPixLength(Encounter, BLOCKFONTNARROW);
		}
		MPrint(54 - width, 38, Encounter);

		SetFont( BLOCKFONT );
		const wchar_t* Mercs = gpStrategicString[STR_PB_MERCS];
		width = StringPixLength(Mercs, BLOCKFONT);
		if( width > 52 )
		{
			SetFont( BLOCKFONTNARROW );
			width = StringPixLength(Mercs, BLOCKFONTNARROW);
		}
		MPrint(139 - width, 38, Mercs);

		SetFont( BLOCKFONT );
		const wchar_t* Milita = gpStrategicString[STR_PB_MILITIA];
		width = StringPixLength(Milita, BLOCKFONT);
		if( width > 52 )
		{
			SetFont( BLOCKFONTNARROW );
			width = StringPixLength(Milita, BLOCKFONTNARROW);
		}
		MPrint(224 - width, 38, Milita);

		//Draw the bottom columns
		for (INT32 i = 0; i < (INT32)MAX(guiNumUninvolved, 1); ++i)
		{
			y = BOTTOM_Y - ROW_HEIGHT * (i+1) + 1;
			BltVideoObject( guiSAVEBUFFER, hVObject, BOTTOM_COLUMN, 161, y);
		}

		for (INT32 i = 0; i < (INT32)(21 - MAX( guiNumUninvolved, 1 )); ++i)
		{
			y = TOP_Y + ROW_HEIGHT * i;
			BltVideoObject( guiSAVEBUFFER, hVObject, TOP_COLUMN, 186, y);
		}

		//location
		SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
		GetSectorIDString( gubPBSectorX, gubPBSectorY, gubPBSectorZ, pSectorName, lengthof(pSectorName), TRUE );
		mprintf( 70, 17, L"%ls %ls", gpStrategicString[ STR_PB_SECTOR ], pSectorName );

		//enemy
		SetFont( FONT14ARIAL );
		if( gubEnemyEncounterCode == CREATURE_ATTACK_CODE ||
			  gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
				gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE ||
				WhatPlayerKnowsAboutEnemiesInSector( gubPBSectorX, gubPBSectorY ) != KNOWS_HOW_MANY )
		{
			// don't know how many
			swprintf( str, lengthof(str), L"?" );
		}
		else
		{
			// know exactly how many
			const INT32 i = NumEnemiesInSector(gubPBSectorX, gubPBSectorY);
			swprintf( str, lengthof(str), L"%d", i );
		}
		x = 57 + (27 - StringPixLength( str, FONT14ARIAL )) / 2;
		y = 36;
		MPrint(x, y, str);
		//player
		swprintf( str, lengthof(str), L"%d", guiNumInvolved );
		x = 142 + (27 - StringPixLength( str, FONT14ARIAL )) / 2;
		MPrint(x, y, str);
		//militia
		swprintf( str, lengthof(str), L"%d", CountAllMilitiaInSector( gubPBSectorX, gubPBSectorY ) );
		x = 227 + (27 - StringPixLength( str, FONT14ARIAL )) / 2;
		MPrint(x, y, str);
		SetFontShadow( FONT_NEARBLACK );

		SetFont( BLOCKFONT2 );

		//print out the participants of the battle.
		// |  NAME  | ASSIGN |  COND  |   HP   |   BP   |
		y = TOP_Y + 1;
		CFOR_ALL_IN_TEAM(i, OUR_TEAM)
		{
			SOLDIERTYPE const& s = *i;
			if (s.bLife == 0)                        continue;
			if (s.uiStatusFlags & SOLDIER_VEHICLE)   continue;
			if (!PlayerMercInvolvedInThisCombat(&s)) continue;

			// Name
			wchar_t const* const name = s.name;
			x = 17 + (52-StringPixLength(name, BLOCKFONT2)) / 2;
			MPrint(x, y, name);
			// Assignment
			wchar_t const* const assignment = GetMapscreenMercAssignmentString(&s);
			x = 72 + (54 - StringPixLength(assignment, BLOCKFONT2)) / 2;
			MPrint(x, y, assignment);
			// Condition
			wchar_t const* const condition = GetSoldierConditionInfo(s);
			x = 129 + (58 - StringPixLength(condition, BLOCKFONT2)) / 2;
			MPrint(x, y, condition);
			// HP
			swprintf(str, lengthof(str), L"%d%%", s.bLife * 100 / s.bLifeMax);
			x = 189 + (25 - StringPixLength(str, BLOCKFONT2)) / 2;
			MPrint(x, y, str);
			// BP
			swprintf(str, lengthof(str), L"%d%%", s.bBreath);
			x = 217 + (25 - StringPixLength(str, BLOCKFONT2)) / 2;
			MPrint(x, y, str);

			y += ROW_HEIGHT;
		}

		//print out the uninvolved members of the battle
		// |  NAME  | ASSIGN |  LOC   |  DEST  |  DEP   |
		if( !guiNumUninvolved )
		{
			const wchar_t* None = gpStrategicString[STR_PB_NONE];
			x = 17 + (52 - StringPixLength(None, BLOCKFONT2)) / 2;
			y = BOTTOM_Y - ROW_HEIGHT + 2;
			MPrint(x, y, None);
		}
		else
		{
			y = BOTTOM_Y - ROW_HEIGHT * guiNumUninvolved + 2;
			CFOR_ALL_IN_TEAM(s, OUR_TEAM)
			{
				if (s->bLife != 0 && !(s->uiStatusFlags & SOLDIER_VEHICLE))
				{
					if (!PlayerMercInvolvedInThisCombat(s))
					{
						// uninvolved
						//NAME
						const wchar_t* const Name = s->name;
						x = 17 + (52 - StringPixLength(Name, BLOCKFONT2)) / 2;
						MPrint(x , y, Name);
						//ASSIGN
						const wchar_t* const Assignment = GetMapscreenMercAssignmentString(s);
						x = 72 + (54 - StringPixLength(Assignment, BLOCKFONT2)) / 2;
						MPrint(x, y, Assignment);
						//LOC
						GetMapscreenMercLocationString(s, str, lengthof(str));
						x = 128 + (33-StringPixLength( str, BLOCKFONT2)) / 2;
						MPrint(x, y, str);
						//DEST
						GetMapscreenMercDestinationString(s, str, lengthof(str));
						if( wcslen( str ) > 0 )
						{
							x = 164 + (41-StringPixLength( str, BLOCKFONT2)) / 2;
							MPrint(x, y, str);
						}
						//DEP
						GetMapscreenMercDepartureString(s, str, lengthof(str), &ubJunk);
						x = 208 + (34-StringPixLength( str, BLOCKFONT2)) / 2;
						MPrint(x, y, str);
						y += ROW_HEIGHT;
					}
				}
			}
		}

		// mark any and ALL pop up boxes as altered
		MarkAllBoxesAsAltered( );
		RestoreExternBackgroundRect( 0, 0, 261, 359 );

		// restore font destinanation buffer to the frame buffer
		SetFontDestBuffer(FRAME_BUFFER);
	}
	else if( gfBlinkHeader )
	{
		RenderPBHeader( &x, &width ); //the text is important enough to blink.
	}

  //InvalidateRegion( 0, 0, 261, 359 );
	if( gfEnterAutoResolveMode )
	{
		gfEnterAutoResolveMode = FALSE;
		EnterAutoResolveMode( gubPBSectorX, gubPBSectorY );
		//return;
	}

	gfIgnoreAllInput = FALSE;

}


static void AutoResolveBattleCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( !gfIgnoreAllInput )
	{
		if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			#ifdef JA2TESTVERSION
				if( _KeyDown( ALT ) )
			#else
				if( _KeyDown( ALT ) && CHEATER_CHEAT_LEVEL() )
			#endif
				{
					if( !gfPersistantPBI )
					{
						return;
					}
					PlayJA2Sample(EXPLOSION_1, HIGHVOLUME, 1, MIDDLEPAN);
					gStrategicStatus.usPlayerKills += NumEnemiesInSector( gubPBSectorX, gubPBSectorY );
					EliminateAllEnemies( gubPBSectorX, gubPBSectorY );
					SetMusicMode( MUSIC_TACTICAL_VICTORY );
					btn->uiFlags &= ~BUTTON_CLICKED_ON;
					btn->Draw();
					InvalidateRegion(btn->X(), btn->Y(), btn->BottomRightX(), btn->BottomRightY());
					ExecuteBaseDirtyRectQueue();
					EndFrameBufferRender( );
					RefreshScreen();
					KillPreBattleInterface();
					StopTimeCompression();
					SetMusicMode( MUSIC_TACTICAL_NOTHING );
					return;
				}
			gfEnterAutoResolveMode = TRUE;
		}
	}
}


static void ClearMovementForAllInvolvedPlayerGroups(void);
static void PutNonSquadMercsInBattleSectorOnSquads(BOOLEAN fExitVehicles);


static void GoToSectorCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( !gfIgnoreAllInput )
	{
		if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			#ifdef JA2TESTVERSION
				if( _KeyDown( ALT ) )
			#else
				if( _KeyDown( ALT ) && CHEATER_CHEAT_LEVEL() )
			#endif
				{
					if( !gfPersistantPBI )
					{
						return;
					}
					PlayJA2Sample(EXPLOSION_1, HIGHVOLUME, 1, MIDDLEPAN);
					gStrategicStatus.usPlayerKills += NumEnemiesInSector( gubPBSectorX, gubPBSectorY );
					EliminateAllEnemies( gubPBSectorX, gubPBSectorY );
					SetMusicMode( MUSIC_TACTICAL_VICTORY );
					btn->uiFlags &= ~BUTTON_CLICKED_ON;
					btn->Draw();
					InvalidateRegion(btn->X(), btn->Y(), btn->BottomRightX(), btn->BottomRightY());
					ExecuteBaseDirtyRectQueue();
					EndFrameBufferRender( );
					RefreshScreen();
					KillPreBattleInterface();
					StopTimeCompression();
					SetMusicMode( MUSIC_TACTICAL_NOTHING );
					return;
				}
			if( gfPersistantPBI && gpBattleGroup && gpBattleGroup->fPlayer &&
					gubEnemyEncounterCode != ENEMY_AMBUSH_CODE &&
					gubEnemyEncounterCode != CREATURE_ATTACK_CODE &&
					gubEnemyEncounterCode != BLOODCAT_AMBUSH_CODE )
			{
				gfEnterTacticalPlacementGUI = TRUE;
			}
			btn->uiFlags &= ~BUTTON_CLICKED_ON;
			btn->Draw();
			InvalidateRegion(btn->X(), btn->Y(), btn->BottomRightX(), btn->BottomRightY());
			ExecuteBaseDirtyRectQueue();
			EndFrameBufferRender( );
			RefreshScreen();
			if( gubPBSectorX == gWorldSectorX && gubPBSectorY == gWorldSectorY && !gbWorldSectorZ )
			{
				gfGotoSectorTransition = TRUE;
			}

			// first time going to the sector?
			if( gfPersistantPBI )
			{
				// put everyone on duty, and remove mercs from vehicles, too
				PutNonSquadMercsInBattleSectorOnSquads( TRUE );

				// we nuke the groups existing route & destination in advance
				ClearMovementForAllInvolvedPlayerGroups( );
			}
			else
			{ //Clear the battlegroup pointer.
				gpBattleGroup = NULL;
			}

			// must come AFTER anything that needs gpBattleGroup, as it wipes it out
			SetCurrentWorldSector( gubPBSectorX, gubPBSectorY, gubPBSectorZ );

			KillPreBattleInterface();
			SetTacticalInterfaceFlags( 0 );
		}
	}
}


static void RetreatMercsCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( !gfIgnoreAllInput )
	{
		if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			// get them outta here!
			RetreatAllInvolvedPlayerGroups();

			// NOTE: this code assumes you can never retreat while underground
			HandleLoyaltyImplicationsOfMercRetreat( RETREAT_PBI, gubPBSectorX, gubPBSectorY, 0 );
			if( CountAllMilitiaInSector( gubPBSectorX, gubPBSectorY ) )
			{ //Mercs retreat, but enemies still need to fight the militia
				gfEnterAutoResolveMode = TRUE;
				return;
			}

			//Warp time by 5 minutes so that player can't just go back into the sector he left.
			WarpGameTime( 300, WARPTIME_NO_PROCESSING_OF_EVENTS );
			ResetMovementForEnemyGroupsInLocation( gubPBSectorX, gubPBSectorY );

			btn->uiFlags &= ~BUTTON_CLICKED_ON;
			btn->Draw();
			InvalidateRegion(btn->X(), btn->Y(), btn->BottomRightX(), btn->BottomRightY());
			ExecuteBaseDirtyRectQueue();
			EndFrameBufferRender( );
			RefreshScreen();
			KillPreBattleInterface();
			StopTimeCompression();
			gpBattleGroup = NULL;
			gfBlitBattleSectorLocator = FALSE;

			SetMusicMode( MUSIC_TACTICAL_NOTHING );
		}
	}
}

enum
{
	COND_EXCELLENT,
	COND_GOOD,
	COND_FAIR,
	COND_WOUNDED,
	COND_FATIGUED,
	COND_BLEEDING,
	COND_UNCONCIOUS,
	COND_DYING,
	COND_DEAD
};


static wchar_t const* GetSoldierConditionInfo(SOLDIERTYPE const& s)
{
	// Go from the worst condition to the best
	return
		s.bLife == 0                         ? pConditionStrings[COND_DEAD]       :
		s.bLife < OKLIFE && s.bBleeding != 0 ? pConditionStrings[COND_DYING]      :
		s.bBreath < OKBREATH && s.bCollapsed ? pConditionStrings[COND_UNCONCIOUS] :
		s.bBleeding > MIN_BLEEDING_THRESHOLD ? pConditionStrings[COND_BLEEDING]   :
		s.bLife * 100 < s.bLifeMax * 50      ? pConditionStrings[COND_WOUNDED]    :
		s.bBreath < 50                       ? pConditionStrings[COND_FATIGUED]   :
		s.bLife * 100 < s.bLifeMax * 67      ? pConditionStrings[COND_FAIR]       :
		s.bLife * 100 < s.bLifeMax * 86      ? pConditionStrings[COND_GOOD]       :
		pConditionStrings[COND_EXCELLENT];
}


void ActivatePreBattleAutoresolveAction()
{
	if (iPBButton[0]->uiFlags & BUTTON_ENABLED)
	{ //Feign call the autoresolve button using the callback
		AutoResolveBattleCallback(iPBButton[0], MSYS_CALLBACK_REASON_LBUTTON_UP);
	}
}

void ActivatePreBattleEnterSectorAction()
{
	if (iPBButton[1]->uiFlags & BUTTON_ENABLED)
	{ //Feign call the enter sector button using the callback
		GoToSectorCallback(iPBButton[1], MSYS_CALLBACK_REASON_LBUTTON_UP);
	}
}

void ActivatePreBattleRetreatAction()
{
	if (iPBButton[2]->uiFlags & BUTTON_ENABLED)
	{ //Feign call the retreat button using the callback
		RetreatMercsCallback(iPBButton[2], MSYS_CALLBACK_REASON_LBUTTON_UP);
	}
}


static void ActivateAutomaticAutoResolveStart()
{
	iPBButton[0]->uiFlags |= BUTTON_CLICKED_ON;
	gfIgnoreAllInput = FALSE;
	AutoResolveBattleCallback(iPBButton[0], MSYS_CALLBACK_REASON_LBUTTON_UP);
}


void CalculateNonPersistantPBIInfo(void)
{
	//We need to set up the non-persistant PBI
	if( !gfBlitBattleSectorLocator ||
			gubPBSectorX != gWorldSectorX || gubPBSectorY != gWorldSectorY || gubPBSectorZ != gbWorldSectorZ )
	{ //Either the locator isn't on or the locator info is in a different sector

		//Calculated the encounter type
		gubEnemyEncounterCode = NO_ENCOUNTER_CODE;
		gubExplicitEnemyEncounterCode = NO_ENCOUNTER_CODE;
		if( HostileCiviliansPresent() )
		{ //There are hostile civilians, so no autoresolve allowed.
			gubExplicitEnemyEncounterCode = HOSTILE_CIVILIANS_CODE;
		}
		else if( HostileBloodcatsPresent() )
		{ //There are bloodcats in the sector, so no autoresolve allowed
			gubExplicitEnemyEncounterCode = HOSTILE_BLOODCATS_CODE;
		}
		else if( gbWorldSectorZ )
		{
			UNDERGROUND_SECTORINFO *pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
			Assert( pSector );
			if( pSector->ubCreaturesInBattle )
			{
				gubExplicitEnemyEncounterCode = FIGHTING_CREATURES_CODE;
			}
			else if( pSector->ubAdminsInBattle || pSector->ubTroopsInBattle || pSector->ubElitesInBattle )
			{
				gubExplicitEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
				gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
			}
		}
		else
		{
			SECTORINFO *pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
			Assert( pSector );
			if( pSector->ubCreaturesInBattle )
			{
				gubExplicitEnemyEncounterCode = FIGHTING_CREATURES_CODE;
			}
			else if( pSector->ubAdminsInBattle || pSector->ubTroopsInBattle || pSector->ubElitesInBattle )
			{
				gubExplicitEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
				gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
			}
		}
		if( gubExplicitEnemyEncounterCode != NO_ENCOUNTER_CODE )
		{	//Set up the location as well as turning on the blit flag.
			gubPBSectorX = (UINT8)gWorldSectorX;
			gubPBSectorY = (UINT8)gWorldSectorY;
			gubPBSectorZ = (UINT8)gbWorldSectorZ;
			gfBlitBattleSectorLocator = TRUE;
		}
	}
}


static void PutNonSquadMercsInPlayerGroupOnSquads(GROUP* pGroup, BOOLEAN fExitVehicles);


static void PutNonSquadMercsInBattleSectorOnSquads(BOOLEAN fExitVehicles)
{
	// IMPORTANT: Have to do this by group, so everyone inside vehicles gets assigned to the same squad.  Needed for
	// the tactical placement interface to work in case of simultaneous multi-vehicle arrivals!
	FOR_ALL_GROUPS_SAFE(i)
	{
		GROUP& g = *i;
		if (!PlayerGroupInvolvedInThisCombat(g)) continue;

		// the helicopter group CAN be involved, if it's on the ground, in which case everybody must get out of it
		if (IsGroupTheHelicopterGroup(g))
		{
			// only happens if chopper is on the ground...
			Assert( !fHelicopterIsAirBorne );

			// put anyone in it into movement group
			MoveAllInHelicopterToFootMovementGroup( );
		}
		else
		{
			PutNonSquadMercsInPlayerGroupOnSquads(&g, fExitVehicles);
		}
	}
}


static void PutNonSquadMercsInPlayerGroupOnSquads(GROUP* const pGroup, const BOOLEAN fExitVehicles)
{
	INT8 bUniqueVehicleSquad = -1;
	if (pGroup->fVehicle)
	{
		// put these guys on their own squad (we need to return their group ID, and can only return one, so they need a unique one
		bUniqueVehicleSquad = GetFirstEmptySquad();
	}

	PLAYERGROUP* next;
	for (PLAYERGROUP* p = pGroup->pPlayerList; p; p = next)
	{
		SOLDIERTYPE* const s = p->pSoldier;
		Assert(s);

		// store ptr to next soldier in group, once removed from group, his info will get memfree'd!
		next = p->next;

		if (!s->bActive || s->bLife == 0 || s->uiStatusFlags & SOLDIER_VEHICLE) continue;

		if (!PlayerMercInvolvedInThisCombat(s) || s->bAssignment < ON_DUTY) continue;
		// if involved, but off-duty (includes mercs inside vehicles!)

		// if in a vehicle, pull him out
		if (s->bAssignment == VEHICLE)
		{
			if (fExitVehicles)
			{
				TakeSoldierOutOfVehicle(s);

				/* put them on the unique squad assigned to people leaving this vehicle.
				 * Can't add them to existing squads, because if this is a simultaneous
				 * group attack, the mercs could be coming from different sides, and the
				 * placement screen can't handle mercs on the same squad arriving from
				 * different edges! */
				const BOOLEAN fSuccess = AddCharacterToSquad(s, bUniqueVehicleSquad);
				(void)fSuccess;
				Assert(fSuccess);
			}
		}
		else
		{
			// add him to ANY on duty foot squad
			AddCharacterToAnySquad(s);
		}

		// stand him up
		MakeSoldiersTacticalAnimationReflectAssignment(s);
	}
}


void WakeUpAllMercsInSectorUnderAttack( void )
{
	// any mercs not on duty should be added to the first avail squad
	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->bLife && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE))
		{
			// if involved, but asleep
			if (PlayerMercInvolvedInThisCombat(pSoldier) && pSoldier->fMercAsleep)
			{
				// FORCE him wake him up
				SetMercAwake( pSoldier, FALSE, TRUE );
			}
		}
	}
}


// we are entering the sector, clear out all mvt orders for grunts
static void ClearMovementForAllInvolvedPlayerGroups(void)
{
	FOR_ALL_GROUPS(i)
	{
		GROUP& g = *i;
		if (!PlayerGroupInvolvedInThisCombat(g)) continue;
		// clear their strategic movement (mercpaths and waypoints)
		ClearMercPathsAndWaypointsForAllInGroup(g);
	}
}

void RetreatAllInvolvedPlayerGroups( void )
{
	// make sure guys stop their off duty assignments, like militia training!
	// but don't exit vehicles - drive off in them!
	PutNonSquadMercsInBattleSectorOnSquads( FALSE );

	FOR_ALL_GROUPS(i)
	{
		GROUP& g = *i;
		if (!PlayerGroupInvolvedInThisCombat(g)) continue;
		// Don't retreat empty vehicle groups!
		if (g.fVehicle && !DoesVehicleGroupHaveAnyPassengers(&g)) continue;
		ClearMercPathsAndWaypointsForAllInGroup(g);
		RetreatGroupToPreviousSector(g);
	}
}


static BOOLEAN CurrentBattleSectorIs(INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ);


BOOLEAN PlayerMercInvolvedInThisCombat(const SOLDIERTYPE* s)
{
	Assert(s);
	Assert(s->bActive);

	if (!s->fBetweenSectors &&
			s->bAssignment != IN_TRANSIT &&
			s->bAssignment != ASSIGNMENT_POW &&
			s->bAssignment != ASSIGNMENT_DEAD &&
			!(s->uiStatusFlags & SOLDIER_VEHICLE) &&
			// Robot is involved if it has a valid controller with it, uninvolved otherwise
			(!AM_A_ROBOT(s) || s->robot_remote_holder != NULL) &&
			!SoldierAboardAirborneHeli(s))
	{
		if (CurrentBattleSectorIs(s->sSectorX, s->sSectorY, s->bSectorZ))
		{
			// involved
			return( TRUE );
		}
	}

	// not involved
	return( FALSE );
}


bool PlayerGroupInvolvedInThisCombat(GROUP const& g)
{
	/* Player group, non-empty, not between sectors, in the right sector, isn't a
	 * group of in transit, dead, or POW mercs, and either not the helicopter
	 * group, or the heli is on the ground */
	return
		g.fPlayer                                                 &&
		g.ubGroupSize != 0                                        &&
		!g.fBetweenSectors                                        &&
		!GroupHasInTransitDeadOrPOWMercs(g)                       &&
		(!IsGroupTheHelicopterGroup(g) || !fHelicopterIsAirBorne) &&
		CurrentBattleSectorIs(g.ubSectorX, g.ubSectorY, g.ubSectorZ);
}


static BOOLEAN CurrentBattleSectorIs(INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ)
{
	INT16 sBattleSectorX, sBattleSectorY, sBattleSectorZ;
	BOOLEAN fSuccess;

	fSuccess = GetCurrentBattleSectorXYZ( &sBattleSectorX, &sBattleSectorY, &sBattleSectorZ );
	Assert( fSuccess );

	if ( ( sSectorX == sBattleSectorX ) && ( sSectorY == sBattleSectorY ) && ( sSectorZ == sBattleSectorZ ) )
	{
		// yup!
		return( TRUE );
	}
	else
	{
		// wrong sector, no battle here
		return( FALSE );
	}
}


static void CheckForRobotAndIfItsControlled(void)
{
	// search for the robot on player's team
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bLife != 0 && AM_A_ROBOT(s))
		{
			// check whether it has a valid controller with it. This sets its robot_remote_holder field.
			UpdateRobotControllerGivenRobot(s);

			// if he has a controller, set controllers
			if (s->robot_remote_holder != NULL)
			{
				UpdateRobotControllerGivenController(s->robot_remote_holder);
			}

			break;
		}
	}
}


void LogBattleResults(const UINT8 ubVictoryCode)
{
	INT16 sSectorX;
	INT16 sSectorY;
	INT16 sSectorZ;
	GetCurrentBattleSectorXYZ(&sSectorX, &sSectorY, &sSectorZ);
	UINT8 code;
	if (ubVictoryCode == LOG_VICTORY)
	{
		switch (gubEnemyEncounterCode)
		{
			case ENEMY_INVASION_CODE:         code = HISTORY_DEFENDEDTOWNSECTOR;   break;
			case ENEMY_ENCOUNTER_CODE:        code = HISTORY_WONBATTLE;            break;
			case ENEMY_AMBUSH_CODE:           code = HISTORY_WIPEDOUTENEMYAMBUSH;  break;
			case ENTERING_ENEMY_SECTOR_CODE:  code = HISTORY_SUCCESSFULATTACK;     break;
			case CREATURE_ATTACK_CODE:        code = HISTORY_CREATURESATTACKED;    break;
			case BLOODCAT_AMBUSH_CODE:
			case ENTERING_BLOODCAT_LAIR_CODE: code = HISTORY_SLAUGHTEREDBLOODCATS; break;
			default:                          return;
		}
	}
	else
	{
		switch (gubEnemyEncounterCode)
		{
			case ENEMY_INVASION_CODE:         code = HISTORY_LOSTTOWNSECTOR;       break;
			case ENEMY_ENCOUNTER_CODE:        code = HISTORY_LOSTBATTLE;           break;
			case ENEMY_AMBUSH_CODE:           code = HISTORY_FATALAMBUSH;          break;
			case ENTERING_ENEMY_SECTOR_CODE:  code = HISTORY_UNSUCCESSFULATTACK;   break;
			case CREATURE_ATTACK_CODE:        code = HISTORY_CREATURESATTACKED;    break;
			case BLOODCAT_AMBUSH_CODE:
			case ENTERING_BLOODCAT_LAIR_CODE: code = HISTORY_KILLEDBYBLOODCATS;    break;
			default:                          return;
		}
	}
	AddHistoryToPlayersLog(code, 0, GetWorldTotalMin(), sSectorX, sSectorY);
}


void HandlePreBattleInterfaceStates()
{
	if( gfEnteringMapScreenToEnterPreBattleInterface && !gfEnteringMapScreen )
	{
		gfEnteringMapScreenToEnterPreBattleInterface = FALSE;
		if( !gfUsePersistantPBI )
		{
			InitPreBattleInterface( NULL, FALSE );
			gfUsePersistantPBI = TRUE;
		}
		else
		{
			InitPreBattleInterface( gpBattleGroup, TRUE );
		}
	}
	else if( gfDelayAutoResolveStart && gfPreBattleInterfaceActive )
	{
		gfDelayAutoResolveStart = FALSE;
		gfAutomaticallyStartAutoResolve = TRUE;
	}
	else if( gfAutomaticallyStartAutoResolve )
	{
		gfAutomaticallyStartAutoResolve = FALSE;
		ActivateAutomaticAutoResolveStart();
	}
}
