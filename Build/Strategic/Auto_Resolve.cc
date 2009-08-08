#include <stdexcept>

#include "Font.h"
#include "HImage.h"
#include "Isometric_Utils.h"
#include "MessageBoxScreen.h"
#include "Timer.h"
#include "Timer_Control.h"
#include "Types.h"
#include "Auto_Resolve.h"
#include "Local.h"
#include "Strategic_Movement.h"
#include "Queen_Command.h"
#include "Music_Control.h"
#include "PreBattle_Interface.h"
#include "Player_Command.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "GameLoop.h"
#include "MapScreen.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "Input.h"
#include "GameScreen.h"
#include "Render_Dirty.h"
#include "VObject_Blitters.h"
#include "SysUtil.h"
#include "Font_Control.h"
#include "Soldier_Create.h"
#include "Overhead.h"
#include "Items.h"
#include "Weapons.h"
#include "Sound_Control.h"
#include "Game_Clock.h"
#include "Soldier_Profile.h"
#include "Campaign.h"
#include "Tactical_Save.h"
#include "Strategic_Status.h"
#include "Text.h"
#include "WordWrap.h"
#include "Random.h"
#include "Line.h"
#include "English.h"
#include "Strategic_Pathing.h"
#include "Strategic_Merc_Handler.h"
#include "Strategic.h"
#include "Message.h"
#include "Town_Militia.h"
#include "Animation_Data.h"
#include "Creature_Spreading.h"
#include "Strategic_AI.h"
#include "SkillCheck.h"
#include "RT_Time_Defines.h"
#include "Morale.h"
#include "Strategic_Town_Loyalty.h"
#include "Soldier_Macros.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Meanwhile.h"
#include "Inventory_Choosing.h"
#include "Game_Event_Hook.h"
#include "Map_Information.h"
#include "MemMan.h"
#include "Debug.h"

#ifdef JA2BETAVERSION
#	include "Cheats.h"
#endif


//#define INVULNERABILITY

BOOLEAN gfTransferTacticalOppositionToAutoResolve = FALSE;

//button images
enum
{
	PAUSE_BUTTON,
	PLAY_BUTTON,
	FAST_BUTTON,
	FINISH_BUTTON,
	YES_BUTTON,
	NO_BUTTON,
	BANDAGE_BUTTON,
	RETREAT_BUTTON,
	DONEWIN_BUTTON,
	DONELOSE_BUTTON,
	NUM_AR_BUTTONS
};


struct SOLDIERCELL
{
	SOLDIERTYPE *pSoldier;
	MouseRegion* pRegion; // only used for player mercs.
	SGPVObject* uiVObjectID;
	UINT16 usIndex;
	UINT32 uiFlags;
	UINT16 usFrame;
	INT16 xp, yp;
	UINT16 usAttack, usDefence;
	UINT16 usNextAttack;
	UINT16 usNextHit[3];
	UINT16 usHitDamage[3];
	SOLDIERCELL* pAttacker[3];
	UINT32 uiFlashTime;
	INT8 bWeaponSlot;
};

struct AUTORESOLVE_STRUCT
{
	SOLDIERCELL *pRobotCell;

	//IDs into the graphic images
	SGPVObject* iPanelImages;
	GUIButtonRef iButton[NUM_AR_BUTTONS];
	BUTTON_PICS* iButtonImage[NUM_AR_BUTTONS];
	SGPVObject* iFaces; //for generic civs and enemies
	INT32 iMercFaces[20]; //for each merc face
	SGPVObject* iIndent;
	SGPVSurface* iInterfaceBuffer;
	INT32 iNumMercFaces;
	INT32 iActualMercFaces; //this represents the real number of merc faces.  Because
													 //my debug mode allows to freely add and subtract mercs, we
													 //can add/remove temp mercs, but we don't want to remove the
													 //actual mercs.
	UINT32 uiTimeSlice;
	UINT32 uiTotalElapsedBattleTimeInMilliseconds;
	UINT32 uiPrevTime, uiCurrTime;
	UINT32 uiPreRandomIndex;

	SGPBox rect;

	UINT16 usPlayerAttack;
	UINT16 usPlayerDefence;
	UINT16 usEnemyAttack;
	UINT16 usEnemyDefence;
	INT16 sCenterStartX;

	UINT8 ubEnemyLeadership;
	UINT8 ubPlayerLeadership;
	UINT8 ubMercs, ubCivs, ubEnemies;
	UINT8 ubAdmins, ubTroops, ubElites;
	UINT8 ubYMCreatures, ubYFCreatures, ubAMCreatures, ubAFCreatures;
	UINT8 ubAliveMercs, ubAliveCivs, ubAliveEnemies;
	UINT8 ubMercCols, ubMercRows;
	UINT8 ubEnemyCols, ubEnemyRows;
	UINT8 ubCivCols, ubCivRows;
	UINT8 ubTimeModifierPercentage;
	UINT8 ubSectorX, ubSectorY;
	INT8 bVerticalOffset;

	BOOLEAN fRenderAutoResolve;
	BOOLEAN fExitAutoResolve;
	BOOLEAN fPaused;
	BOOLEAN fDebugInfo;
	BOOLEAN ubBattleStatus;
	BOOLEAN fUnlimitedAmmo;
	BOOLEAN fSound;
	BOOLEAN ubPlayerDefenceAdvantage;
	BOOLEAN ubEnemyDefenceAdvantage;
	BOOLEAN fInstantFinish;
	BOOLEAN fAllowCapture;
	BOOLEAN fPlayerRejectedSurrenderOffer;
	BOOLEAN fPendingSurrender;
	BOOLEAN fShowInterface;
	BOOLEAN fEnteringAutoResolve;
	BOOLEAN fMoraleEventsHandled;
	BOOLEAN fCaptureNotPermittedDueToEPCs;

	MOUSE_REGION AutoResolveRegion;
};

//Classifies the type of soldier the soldier cell is
#define CELL_MERC							0x00000001
#define CELL_MILITIA					0x00000002
#define CELL_ELITE						0x00000004
#define CELL_TROOP						0x00000008
#define	CELL_ADMIN						0x00000010
#define CELL_AF_CREATURE			0x00000020
#define CELL_AM_CREATURE			0x00000040
#define CELL_YF_CREATURE			0x00000080
#define CELL_YM_CREATURE			0x00000100
//The team leader is the one with the highest leadership.
//There can only be one teamleader per side (mercs/civs and enemies)
#define CELL_TEAMLEADER				0x00000200
//Combat flags
#define CELL_FIREDATTARGET		0x00000400
#define CELL_DODGEDATTACK			0x00000800
#define CELL_HITBYATTACKER		0x00001000
#define CELL_HITLASTFRAME			0x00002000
//Cell statii
#define CELL_SHOWRETREATTEXT  0x00004000
#define CELL_RETREATING				0x00008000
#define CELL_RETREATED				0x00010000
#define CELL_DIRTY						0x00020000
#define CELL_PROCESSED				0x00040000
#define CELL_ASSIGNED					0x00080000
#define CELL_EPC							0x00100000
#define CELL_ROBOT						0x00200000

//Combined flags
#define CELL_PLAYER						( CELL_MERC | CELL_MILITIA )
#define CELL_ENEMY						( CELL_ELITE | CELL_TROOP | CELL_ADMIN )
#define CELL_CREATURE					( CELL_AF_CREATURE | CELL_AM_CREATURE | CELL_YF_CREATURE | CELL_YM_CREATURE )
#define CELL_FEMALECREATURE	  ( CELL_AF_CREATURE | CELL_YF_CREATURE )
#define CELL_MALECREATURE			( CELL_AM_CREATURE | CELL_YM_CREATURE )
#define CELL_YOUNGCREATURE		( CELL_YF_CREATURE | CELL_YM_CREATURE )
#define CELL_INVOLVEDINCOMBAT ( CELL_FIREDATTARGET | CELL_DODGEDATTACK | CELL_HITBYATTACKER )

enum
{
	BATTLE_IN_PROGRESS,
	BATTLE_VICTORY,
	BATTLE_DEFEAT,
	BATTLE_RETREAT,
	BATTLE_SURRENDERED,
	BATTLE_CAPTURED
};

//panel pieces
enum
{
	TL_BORDER,
	T_BORDER,
	TR_BORDER,
	L_BORDER,
	C_TEXTURE,
	R_BORDER,
	BL_BORDER,
	B_BORDER,
	BR_BORDER,
	TOP_MIDDLE,
	AUTO_MIDDLE,
	BOT_MIDDLE,
	MERC_PANEL,
	OTHER_PANEL,
};

//generic face images
enum
{
	ADMIN_FACE,
	TROOP_FACE,
	ELITE_FACE,
	MILITIA1_FACE,
	MILITIA2_FACE,
	MILITIA3_FACE,
	YM_CREATURE_FACE,
	AM_CREATURE_FACE,
	YF_CREATURE_FACE,
	AF_CREATURE_FACE,
	HUMAN_SKULL,
	CREATURE_SKULL,
	ELITEF_FACE,
	MILITIA1F_FACE,
	MILITIA2F_FACE,
	MILITIA3F_FACE,
};


//Autoresolve sets this variable which defaults to -1 when not needed.
INT16 gsEnemyGainedControlOfSectorID = -1;
INT16 gsCiviliansEatenByMonsters = -1;

//Dynamic globals -- to conserve memory, all global variables are allocated upon entry
//and deleted before we leave.
static AUTORESOLVE_STRUCT* gpAR;
static SOLDIERCELL*        gpMercs;
static SOLDIERCELL*        gpCivs;
static SOLDIERCELL*        gpEnemies;


#define FOR_ALL_AR_MERCS(iter) \
	for (SOLDIERCELL* iter = gpMercs, *const iter##__end = &gpMercs[gpAR->ubMercs]; iter != iter##__end; ++iter)

#define FOR_ALL_AR_CIVS(iter) \
	for (SOLDIERCELL* iter = gpCivs, *const iter##__end = &gpCivs[gpAR->ubCivs]; iter != iter##__end; ++iter)

#define FOR_ALL_AR_ENEMIES(iter) \
	for (SOLDIERCELL* iter = gpEnemies, *const iter##__end = &gpEnemies[gpAR->ubEnemies]; iter != iter##__end; ++iter)


//Simple wrappers for autoresolve sounds that are played.
static void PlayAutoResolveSample(SoundID const usNum, UINT32 const ubVolume, UINT32 const ubLoops, UINT32 const uiPan)
{
	if( gpAR->fSound )
	{
		PlayJA2Sample(usNum, ubVolume, ubLoops, uiPan);
	}
}


static void EliminateAllMercs(void)
{
	SOLDIERCELL *pAttacker = NULL;
	if( gpAR )
	{
		FOR_ALL_AR_ENEMIES(i)
		{
			if (i->pSoldier->bLife == 0) continue;
			pAttacker = i;
			break;
		}
		if( pAttacker )
		{
			INT32 iNum = 0;
			FOR_ALL_AR_MERCS(i)
			{
				SOLDIERTYPE& s = *i->pSoldier;
				if (s.bLife == 0) continue;

				s.bLife           = 1;
				i->usNextHit[0]   = 250 * ++iNum;
				i->usHitDamage[0] = 100;
				i->pAttacker[0]   = pAttacker;
			}
		}
	}
}


static void EliminateAllFriendlies(void)
{
	if( gpAR )
	{
		FOR_ALL_AR_MERCS(i)
		{
			i->pSoldier->bLife = 0;
		}
		gpAR->ubAliveMercs = 0;
		FOR_ALL_AR_CIVS(i)
		{
			i->pSoldier->bLife = 0;
		}
		gpAR->ubAliveCivs = 0;
	}
}

void EliminateAllEnemies( UINT8 ubSectorX, UINT8 ubSectorY )
{
	SECTORINFO *pSector;
	UINT8 ubNumEnemies[ NUM_ENEMY_RANKS ];
	UINT8 ubRankIndex;

	//Clear any possible battle locator
	gfBlitBattleSectorLocator = FALSE;

	pSector = &SectorInfo[ SECTOR( ubSectorX, ubSectorY ) ];


	// if we're doing this from the Pre-Battle interface, gpAR is NULL, and RemoveAutoResolveInterface(0 won't happen, so
	// we must process the enemies killed right here & give out loyalty bonuses as if the battle had been fought & won
	if( !gpAR )
	{
		GetNumberOfEnemiesInSector( ubSectorX, ubSectorY, &ubNumEnemies[ 0 ], &ubNumEnemies[ 1 ], &ubNumEnemies[ 2 ] );

		for ( ubRankIndex = 0; ubRankIndex < NUM_ENEMY_RANKS; ubRankIndex++ )
		{
			for (INT32 i = 0; i < ubNumEnemies[ubRankIndex]; ++i)
			{
				HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_ENEMY_KILLED, ubSectorX, ubSectorY, 0 );
				TrackEnemiesKilled( ENEMY_KILLED_IN_AUTO_RESOLVE, RankIndexToSoldierClass( ubRankIndex ) );
			}
		}

		HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_BATTLE_WON, ubSectorX, ubSectorY, 0 );
	}

	if( !gpAR || gpAR->ubBattleStatus != BATTLE_IN_PROGRESS )
	{
		//Remove the defend force here.
		pSector->ubNumTroops = 0;
		pSector->ubNumElites = 0;
		pSector->ubNumAdmins = 0;
		pSector->ubNumCreatures = 0;
		//Remove the mobile forces here, but only if battle is over.
		FOR_ALL_GROUPS_SAFE(i)
		{
			GROUP& g = *i;
			if (g.fPlayer)                continue;
			if (g.ubSectorX != ubSectorX) continue;
			if (g.ubSectorY != ubSectorY) continue;
			RemoveGroupFromStrategicAILists(g);
			if (gpBattleGroup == &g) gpBattleGroup = 0;
			RemoveGroup(g);
		}
		if( gpBattleGroup )
		{
			CalculateNextMoveIntention( gpBattleGroup );
		}
		// set this sector as taken over
		SetThisSectorAsPlayerControlled( ubSectorX, ubSectorY, 0, TRUE );
		RecalculateSectorWeight( (UINT8)SECTOR( ubSectorX, ubSectorY ) );

		// dirty map panel
		fMapPanelDirty = TRUE;
	}

	if( gpAR )
	{
		FOR_ALL_AR_ENEMIES(i)
		{
			i->pSoldier->bLife = 0;
		}
		gpAR->ubAliveEnemies = 0;
	}
	gpBattleGroup = NULL;
}


static void RenderAutoResolve(void);


static void DoTransitionFromPreBattleInterfaceToAutoResolve(void)
{
	UINT32 uiStartTime, uiCurrTime;
	INT32 iPercentage, iFactor;
	UINT32 uiTimeRange;

	PauseTime( FALSE );

	gpAR->fShowInterface = TRUE;

	uiTimeRange = 1000;
	iPercentage = 0;
	uiStartTime = GetClock();

	INT32 const x = gpAR->rect.x;
	INT32 const y = gpAR->rect.y;
	INT32 const w = gpAR->rect.w;
	INT32 const h = gpAR->rect.h;

	INT16 const sStartLeft = 59;
	INT16 const sStartTop  = 69;
	INT16 const sEndLeft   = x + w / 2;
	INT16 const sEndTop    = y + h / 2;

	//save the prebattle/mapscreen interface background
	BltVideoSurface(guiEXTRABUFFER, FRAME_BUFFER, 0, 0, NULL);

	//render the autoresolve panel
	RenderAutoResolve();
	RenderButtons();
	RenderButtonsFastHelp();
	//save it
	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, x, y, w, h);

	//hide the autoresolve
	BlitBufferToBuffer(guiEXTRABUFFER, FRAME_BUFFER, x, y, w, h);

	PlayJA2SampleFromFile("SOUNDS/Laptop power up (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
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
		INT32 const iLeft = sStartLeft + (sEndLeft - sStartLeft + 1) * iPercentage / 100;
		INT32 const iTop  = sStartTop  + (sEndTop  - sStartTop  + 1) * iPercentage / 100;

		SGPBox const DstRect =
		{
			iLeft - w * iPercentage / 200,
			iTop  - h * iPercentage / 200,
			MAX(1, w * iPercentage / 100),
			MAX(1, h * iPercentage / 100)
		};

		BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &gpAR->rect, &DstRect);
		InvalidateScreen();
		RefreshScreen();

		//Restore the previous rect.
		BlitBufferToBuffer(guiEXTRABUFFER, FRAME_BUFFER, DstRect.x, DstRect.y, DstRect.w + 1, DstRect.h + 1);
	}
}

void EnterAutoResolveMode( UINT8 ubSectorX, UINT8 ubSectorY )
{
	#ifdef JA2BETAVERSION
		CountRandomCalls( TRUE );
	#endif

	//Set up mapscreen for removal
	SetPendingNewScreen( AUTORESOLVE_SCREEN );
	CreateDestroyMapInvButton();
	RenderButtons();

	//Allocate memory for all the globals while we are in this mode.
	gpAR = MALLOCZ(AUTORESOLVE_STRUCT);
	//Mercs -- 20 max
	gpMercs = MALLOCNZ(SOLDIERCELL, 20);
	//Militia -- MAX_ALLOWABLE_MILITIA_PER_SECTOR max
	gpCivs = MALLOCNZ(SOLDIERCELL, MAX_ALLOWABLE_MILITIA_PER_SECTOR);
	//Enemies -- 32 max
	gpEnemies = MALLOCNZ(SOLDIERCELL, 32);

	//Set up autoresolve
	gpAR->fEnteringAutoResolve = TRUE;
	gpAR->ubSectorX = ubSectorX;
	gpAR->ubSectorY = ubSectorY;
	gpAR->ubBattleStatus = BATTLE_IN_PROGRESS;
	gpAR->uiTimeSlice = 1000;
	gpAR->uiTotalElapsedBattleTimeInMilliseconds = 0;
	gpAR->fSound = TRUE;
	gpAR->fMoraleEventsHandled = FALSE;
	gpAR->uiPreRandomIndex = guiPreRandomIndex;

	//Determine who gets the defensive advantage
	switch( gubEnemyEncounterCode )
	{
		case ENEMY_ENCOUNTER_CODE:
			gpAR->ubPlayerDefenceAdvantage = 21; //Skewed to the player's advantage for convenience purposes.
			break;
		case ENEMY_INVASION_CODE:
			gpAR->ubPlayerDefenceAdvantage = 0;
			break;
		case CREATURE_ATTACK_CODE:
			gpAR->ubPlayerDefenceAdvantage = 0;
			break;
		default:
			//shouldn't happen
			#ifdef JA2BETAVERSION
				ScreenMsg( FONT_RED, MSG_ERROR, L"Autoresolving with entering enemy sector code %d -- illegal KM:1", gubEnemyEncounterCode );
			#endif
			break;
	}
}


static void CalculateAttackValues(void);
static void CalculateAutoResolveInfo(void);
static void CalculateSoldierCells(BOOLEAN fReset);
static void CreateAutoResolveInterface(void);
static void DetermineTeamLeader(BOOLEAN fFriendlyTeam);
static void HandleAutoResolveInput(void);
static void ProcessBattleFrame(void);
static void RemoveAutoResolveInterface(BOOLEAN fDeleteForGood);


ScreenID AutoResolveScreenHandle()
{
	RestoreBackgroundRects();

	if( !gpAR )
	{
		gfEnteringMapScreen = TRUE;
		return MAP_SCREEN;
	}
	if( gpAR->fEnteringAutoResolve )
	{
		gpAR->fEnteringAutoResolve = FALSE;
		//Take the framebuffer, shade it, and save it to the SAVEBUFFER.
		FRAME_BUFFER->ShadowRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
		KillPreBattleInterface();
		CalculateAutoResolveInfo();
		CalculateSoldierCells( FALSE );
		CreateAutoResolveInterface();
		DetermineTeamLeader( TRUE ); //friendly team
		DetermineTeamLeader( FALSE ); //enemy team
		CalculateAttackValues();
		DoTransitionFromPreBattleInterfaceToAutoResolve();
		gpAR->fRenderAutoResolve = TRUE;
	}
	if( gpAR->fExitAutoResolve )
	{
		gfEnteringMapScreen = TRUE;
		RemoveAutoResolveInterface( TRUE );
		#ifdef JA2BETAVERSION
		{
			UINT32 uiRandoms, uiPreRandoms;
			GetRandomCalls( &uiRandoms, &uiPreRandoms );
		}
		#endif
		return MAP_SCREEN;
	}
	if( gpAR->fPendingSurrender )
	{
		gpAR->uiPrevTime = gpAR->uiCurrTime = GetJA2Clock();

	}
	else if (gpAR->ubBattleStatus == BATTLE_IN_PROGRESS)
	{
		ProcessBattleFrame();
	}
	HandleAutoResolveInput();
	RenderAutoResolve();


	SaveBackgroundRects();
	RenderButtons();
	RenderButtonsFastHelp();
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
	return AUTORESOLVE_SCREEN;
}


static void RefreshMerc(SOLDIERTYPE* pSoldier)
{
	pSoldier->bLife = pSoldier->bLifeMax;
	pSoldier->bBleeding = 0;
	pSoldier->bBreath = pSoldier->bBreathMax = 100;
	pSoldier->sBreathRed = 0;
	if( gpAR->pRobotCell)
	{
		UpdateRobotControllerGivenRobot( gpAR->pRobotCell->pSoldier );
	}
	//gpAR->fUnlimitedAmmo = TRUE;
}


//Now assign the pSoldier->ubGroupIDs for the enemies, so we know where to remove them.  Start with
//stationary groups first.
static void AssociateEnemiesWithStrategicGroups(void)
{
	SECTORINFO *pSector;
	UINT8 ubNumAdmins, ubNumTroops, ubNumElites;
	UINT8 ubNumElitesInGroup, ubNumTroopsInGroup, ubNumAdminsInGroup;

	if( gubEnemyEncounterCode == CREATURE_ATTACK_CODE )
		return;

	pSector = &SectorInfo[ SECTOR( gpAR->ubSectorX, gpAR->ubSectorY ) ];

	//grab the number of each type in the stationary sector
	ubNumAdmins = pSector->ubNumAdmins;
	ubNumTroops = pSector->ubNumTroops;
	ubNumElites = pSector->ubNumElites;

	//Now go through our enemies in the autoresolve array, and assign the ubGroupID to the soldier
	//Stationary groups have a group ID of 0
	FOR_ALL_AR_ENEMIES(i)
	{
		if (i->uiFlags & CELL_ELITE && ubNumElites != 0)
		{
			--ubNumElites;
		}
		else if (i->uiFlags & CELL_TROOP && ubNumTroops != 0)
		{
			--ubNumTroops;
		}
		else if (i->uiFlags & CELL_ADMIN && ubNumAdmins != 0)
		{
			--ubNumAdmins;
		}
		else
		{
			continue;
		}
		i->uiFlags             |= CELL_ASSIGNED;
		i->pSoldier->ubGroupID  = 0;
	}

	ubNumAdmins = gpAR->ubAdmins - pSector->ubNumAdmins;
	ubNumTroops = gpAR->ubTroops - pSector->ubNumTroops;
	ubNumElites = gpAR->ubElites - pSector->ubNumElites;

	if( !ubNumElites && !ubNumTroops && !ubNumAdmins )
	{ //All troops accounted for.
		return;
	}

	//Now assign the rest of the soldiers to groups
	CFOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (pGroup->ubSectorX == gpAR->ubSectorX &&
				pGroup->ubSectorY == gpAR->ubSectorY)
		{
			ubNumElitesInGroup = pGroup->pEnemyGroup->ubNumElites;
			ubNumTroopsInGroup = pGroup->pEnemyGroup->ubNumTroops;
			ubNumAdminsInGroup = pGroup->pEnemyGroup->ubNumAdmins;
			FOR_ALL_AR_ENEMIES(i)
			{
				if (i->uiFlags & CELL_ASSIGNED) continue;

				if (ubNumElites != 0 && ubNumElitesInGroup != 0)
				{
					--ubNumElites;
					--ubNumElitesInGroup;
				}
				else if (ubNumTroops != 0 && ubNumTroopsInGroup != 0)
				{
					--ubNumTroops;
					--ubNumTroopsInGroup;
				}
				else if (ubNumAdmins != 0 && ubNumAdminsInGroup != 0)
				{
					--ubNumAdmins;
					--ubNumAdminsInGroup;
				}
				else
				{
					continue;
				}
				i->uiFlags             |= CELL_ASSIGNED;
				i->pSoldier->ubGroupID  = pGroup->ubGroupID;
			}
		}
	}
}


static void MercCellMouseClickCallback(MOUSE_REGION* reg, INT32 reason);
static void MercCellMouseMoveCallback(MOUSE_REGION* reg, INT32 reason);


static void CalculateSoldierCells(BOOLEAN fReset)
{
	INT32 i, x, y;
	INT32 index, iStartY, iTop, gapStartRow;
	INT32 iMaxTeamSize;

	gpAR->ubAliveMercs = gpAR->ubMercs;
	gpAR->ubAliveCivs = gpAR->ubCivs;
	gpAR->ubAliveEnemies = gpAR->ubEnemies;

	iMaxTeamSize = MAX( gpAR->ubMercs + gpAR->ubCivs, gpAR->ubEnemies );

	if( iMaxTeamSize > 12 )
	{
		gpAR->ubTimeModifierPercentage = (UINT8)(118 - iMaxTeamSize*1.5);
	}
	else
	{
		gpAR->ubTimeModifierPercentage = 100;
	}
	gpAR->uiTimeSlice = gpAR->uiTimeSlice * gpAR->ubTimeModifierPercentage / 100;

	iTop = (SCREEN_HEIGHT - gpAR->rect.h) / 2;
	if( iTop > 120 )
		iTop -= 40;

	if( gpAR->ubMercs )
	{
		iStartY = iTop + (gpAR->rect.h - ((gpAR->ubMercRows + gpAR->ubCivRows) * 47 + 7)) / 2 + 6;
		y = gpAR->ubMercRows;
		x = gpAR->ubMercCols;
		i = gpAR->ubMercs;
		gapStartRow = gpAR->ubMercRows - gpAR->ubMercRows * gpAR->ubMercCols + gpAR->ubMercs;
		for( x = 0; x < gpAR->ubMercCols; x++ ) for( y = 0; i && y < gpAR->ubMercRows; y++, i-- )
		{
			index = y * gpAR->ubMercCols + gpAR->ubMercCols - x - 1;
			if( y >= gapStartRow )
				index -= y - gapStartRow + 1;
			Assert( index >= 0 && index < gpAR->ubMercs );
			gpMercs[ index ].xp = gpAR->sCenterStartX + 3 - 55*(x+1);
			gpMercs[ index ].yp = iStartY + y*47;
			gpMercs[ index ].uiFlags = CELL_MERC;
			if( AM_AN_EPC( gpMercs[ index ].pSoldier ) )
			{
				if( AM_A_ROBOT( gpMercs[ index ].pSoldier ) )
				{ //treat robot as a merc for the purpose of combat.
					gpMercs[ index ].uiFlags |= CELL_ROBOT;
				}
				else
				{
					gpMercs[ index ].uiFlags |= CELL_EPC;
				}
			}
			SOLDIERCELL& c = gpMercs[index];
			c.pRegion = new MouseRegion(c.xp, c.yp, 50, 44, MSYS_PRIORITY_HIGH, 0, MercCellMouseMoveCallback, MercCellMouseClickCallback);
			if( fReset )
				RefreshMerc( gpMercs[ index ].pSoldier );
			if( !gpMercs[ index ].pSoldier->bLife )
				gpAR->ubAliveMercs--;
		}
	}
	if( gpAR->ubCivs )
	{
		iStartY = iTop + (gpAR->rect.h - ((gpAR->ubMercRows + gpAR->ubCivRows) * 47 + 7)) / 2 + gpAR->ubMercRows * 47 + 5;
		y = gpAR->ubCivRows;
		x = gpAR->ubCivCols;
		i = gpAR->ubCivs;
		gapStartRow = gpAR->ubCivRows - gpAR->ubCivRows * gpAR->ubCivCols + gpAR->ubCivs;
		for( x = 0; x < gpAR->ubCivCols; x++ ) for( y = 0; i && y < gpAR->ubCivRows; y++, i-- )
		{
			index = y * gpAR->ubCivCols + gpAR->ubCivCols - x - 1;
			if( y >= gapStartRow )
				index -= y - gapStartRow + 1;
			Assert( index >= 0 && index < gpAR->ubCivs );
			gpCivs[ index ].xp = gpAR->sCenterStartX + 3 - 55*(x+1);
			gpCivs[ index ].yp = iStartY + y*47;
			gpCivs[ index ].uiFlags |= CELL_MILITIA;
		}
	}
	if( gpAR->ubEnemies )
	{
		iStartY = iTop + (gpAR->rect.h - (gpAR->ubEnemyRows * 47 + 7)) / 2 + 5;
		y = gpAR->ubEnemyRows;
		x = gpAR->ubEnemyCols;
		i = gpAR->ubEnemies;
		gapStartRow = gpAR->ubEnemyRows - gpAR->ubEnemyRows * gpAR->ubEnemyCols + gpAR->ubEnemies;
		for( x = 0; x < gpAR->ubEnemyCols; x++ ) for( y = 0; i && y < gpAR->ubEnemyRows; y++, i-- )
		{
			index = y * gpAR->ubEnemyCols + x;
			if( y > gapStartRow )
				index -= y - gapStartRow;
			Assert( index >= 0 && index < gpAR->ubEnemies );
			gpEnemies[ index ].xp = (UINT16)(gpAR->sCenterStartX + 141 + 55*x);
			gpEnemies[ index ].yp = iStartY + y*47;
			if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
			{
				if( index < gpAR->ubElites )
					gpEnemies[ index ].uiFlags = CELL_ELITE;
				else if( index < gpAR->ubElites + gpAR->ubTroops )
					gpEnemies[ index ].uiFlags = CELL_TROOP;
				else
					gpEnemies[ index ].uiFlags = CELL_ADMIN;
			}
			else
			{
				if( index < gpAR->ubAFCreatures )
					gpEnemies[ index ].uiFlags = CELL_AF_CREATURE;
				else if( index < gpAR->ubAMCreatures + gpAR->ubAFCreatures )
					gpEnemies[ index ].uiFlags = CELL_AM_CREATURE;
				else if( index < gpAR->ubYFCreatures + gpAR->ubAMCreatures + gpAR->ubAFCreatures )
					gpEnemies[ index ].uiFlags = CELL_YF_CREATURE;
				else
					gpEnemies[ index ].uiFlags = CELL_YM_CREATURE;
			}
		}
	}
}


static void DrawDebugText(SOLDIERCELL* pCell);
static void RenderSoldierCellBars(SOLDIERCELL* pCell);
static void RenderSoldierCellHealth(SOLDIERCELL* pCell);


static void RenderSoldierCell(SOLDIERCELL* const c)
{
	SGPVSurface* const buf = FRAME_BUFFER;
	INT16        const dx  = c->xp;
	INT16        const dy  = c->yp;

	INT32       x = dx + 3;
	INT32 const y = dy + 3;
	if (c->uiFlags & CELL_MERC)
	{
		ColorFillVideoSurfaceArea(buf, dx + 36, dy + 2, dx + 44, dy + 30, 0);
		BltVideoObject(buf, gpAR->iPanelImages, MERC_PANEL, dx, dy);
		RenderSoldierCellBars(c);
	}
	else
	{
		BltVideoObject(buf, gpAR->iPanelImages, OTHER_PANEL, dx, dy);
		x += 6;
	}

	if (c->pSoldier->bLife == 0)
	{
		UINT16 const skull = c->uiFlags & CELL_CREATURE ? CREATURE_SKULL : HUMAN_SKULL;
		BltVideoObject(buf, gpAR->iFaces, skull, x, y);
	}
	else
	{
		if (c->uiFlags & CELL_HITBYATTACKER)
		{
			ColorFillVideoSurfaceArea(buf, x, y, x + 30, y + 26, 65535);
		}
		else
		{
			size_t shade = 0;
			if (c->uiFlags & CELL_HITLASTFRAME)
			{
				ColorFillVideoSurfaceArea(buf, x, y, x + 30, y + 26, 0);
				shade = 1;
			}
			SGPVObject* const vo = c->uiVObjectID;
			vo->CurrentShade(shade);
			BltVideoObject(buf, vo, c->usIndex, x, y);
		}

		if (c->pSoldier->bLife < OKLIFE && !(c->uiFlags & (CELL_HITBYATTACKER | CELL_HITLASTFRAME | CELL_CREATURE)))
		{ // Merc is unconcious (and not taking damage), so darken his portrait.
			buf->ShadowRect(x, y, x + 30, y + 26);
		}
	}

	RenderSoldierCellHealth(c);
	DrawDebugText(c);

	InvalidateRegion(dx, dy, dx + 50, dy + 44);

	// Adjust flags accordingly
	if (c->uiFlags & CELL_HITBYATTACKER)
	{
		c->uiFlashTime  = GetJA2Clock() + 150;
		c->uiFlags     &= ~CELL_HITBYATTACKER;
		c->uiFlags     |= CELL_HITLASTFRAME | CELL_DIRTY;
	}
	else if (c->uiFlags & CELL_HITLASTFRAME)
	{
		if (c->uiFlashTime < GetJA2Clock()) c->uiFlags &= ~CELL_HITLASTFRAME;
		c->uiFlags |= CELL_DIRTY;
	}
	else if (!(c->uiFlags & CELL_RETREATING))
	{
		c->uiFlags &= ~CELL_DIRTY;
	}
}


static void RenderSoldierCellBars(SOLDIERCELL* pCell)
{
	INT32 iStartY;
	//HEALTH BAR
	if( !pCell->pSoldier->bLife )
		return;
	//yellow one for bleeding
	iStartY = pCell->yp + 29 - 25*pCell->pSoldier->bLifeMax/100;
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+37, iStartY, pCell->xp+38, pCell->yp+29, Get16BPPColor( FROMRGB( 107, 107, 57 ) ) );
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+38, iStartY, pCell->xp+39, pCell->yp+29, Get16BPPColor( FROMRGB( 222, 181, 115 ) ) );
	//pink one for bandaged.
	iStartY += 25*pCell->pSoldier->bBleeding/100;
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+37, iStartY, pCell->xp+38, pCell->yp+29, Get16BPPColor( FROMRGB( 156, 57, 57 ) ) );
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+38, iStartY, pCell->xp+39, pCell->yp+29, Get16BPPColor( FROMRGB( 222, 132, 132 ) ) );
	//red one for actual health
	iStartY = pCell->yp + 29 - 25*pCell->pSoldier->bLife/100;
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+37, iStartY, pCell->xp+38, pCell->yp+29, Get16BPPColor( FROMRGB( 107, 8, 8 ) ) );
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+38, iStartY, pCell->xp+39, pCell->yp+29, Get16BPPColor( FROMRGB( 206, 0, 0 ) ) );
	//BREATH BAR
	iStartY = pCell->yp + 29 - 25*pCell->pSoldier->bBreathMax/100;
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+41, iStartY, pCell->xp+42, pCell->yp+29, Get16BPPColor( FROMRGB( 8, 8, 132 ) ) );
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+42, iStartY, pCell->xp+43, pCell->yp+29, Get16BPPColor( FROMRGB( 8, 8, 107 ) ) );
	//MORALE BAR
	iStartY = pCell->yp + 29 - 25*pCell->pSoldier->bMorale/100;
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+45, iStartY, pCell->xp+46, pCell->yp+29, Get16BPPColor( FROMRGB( 8, 156, 8 ) ) );
	ColorFillVideoSurfaceArea( FRAME_BUFFER, pCell->xp+46, iStartY, pCell->xp+47, pCell->yp+29, Get16BPPColor( FROMRGB( 8, 107, 8 ) ) );
}


static void BuildInterfaceBuffer(void)
{
	SGPRect					ClipRect;
	SGPRect					DestRect;
	INT32						x,y;

	//Setup the blitting clip regions, so we don't draw outside of the region (for excess panelling)
	gpAR->rect.x = (SCREEN_WIDTH  - gpAR->rect.w) / 2;
	gpAR->rect.y = (SCREEN_HEIGHT - gpAR->rect.h) / 2;
	if (gpAR->rect.y > 120) gpAR->rect.y -= 40;

	DestRect.iLeft			= 0;
	DestRect.iTop				= 0;
	DestRect.iRight			= gpAR->rect.w;
	DestRect.iBottom		= gpAR->rect.h;

	gpAR->iInterfaceBuffer = AddVideoSurface(gpAR->rect.w, gpAR->rect.h, PIXEL_DEPTH);

	GetClippingRect( &ClipRect );
	SetClippingRect( &DestRect );

	//Blit the back panels...
	for( y = DestRect.iTop; y < DestRect.iBottom; y += 40 )
	{
		for( x = DestRect.iLeft; x < DestRect.iRight; x += 50 )
		{
			BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, C_TEXTURE, x, y);
		}
	}
	//Blit the left and right edges
	for( y = DestRect.iTop; y < DestRect.iBottom; y += 40 )
	{
		x = DestRect.iLeft;
		BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, L_BORDER, x, y);
		x = DestRect.iRight - 3;
		BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, R_BORDER, x, y);
	}
	//Blit the top and bottom edges
	for( x = DestRect.iLeft; x < DestRect.iRight; x += 50 )
	{
		y = DestRect.iTop;
		BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, T_BORDER, x, y);
		y = DestRect.iBottom - 3;
		BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, B_BORDER, x, y);
	}
	//Blit the 4 corners
	BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, TL_BORDER, DestRect.iLeft, DestRect.iTop);
	BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, TR_BORDER, DestRect.iRight-10, DestRect.iTop);
	BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, BL_BORDER, DestRect.iLeft, DestRect.iBottom-9);
	BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, BR_BORDER, DestRect.iRight-10, DestRect.iBottom-9);

	//Blit the center pieces
	x = gpAR->sCenterStartX - gpAR->rect.x;
	y = 0;
	//Top
	BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, TOP_MIDDLE, x, y);
	//Middle
	for (y = 40; y < gpAR->rect.h - 40; y += 40)
	{
		BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, AUTO_MIDDLE, x, y);
	}
	y = gpAR->rect.h - 40;
	BltVideoObject( gpAR->iInterfaceBuffer, gpAR->iPanelImages, BOT_MIDDLE, x, y);

	SetClippingRect( &ClipRect );
}


UINT32 VirtualSoldierDressWound(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pVictim, OBJECTTYPE* pKit, INT16 sKitPts, INT16 sStatus)
{
	UINT32 uiDressSkill, uiPossible, uiActual, uiMedcost, uiDeficiency, uiAvailAPs, uiUsedAPs;
	UINT8 bBelowOKlife, bPtsLeft;

	if( pVictim->bBleeding < 1 )
		return 0;		// nothing to do, shouldn't have even been called!
	if ( pVictim->bLife == 0 )
		return 0;

	// calculate wound-dressing skill (3x medical,  2x equip,1x level, 1x dex)
	uiDressSkill = ( ( 3 * EffectiveMedical( pSoldier ) ) +		// medical knowledge
									( 2 * sStatus) + 													// state of medical kit
									(10 * EffectiveExpLevel( pSoldier ) ) +		// battle injury experience
									EffectiveDexterity( pSoldier ) )	/ 7;		// general "handiness"

	// try to use every AP that the merc has left
	uiAvailAPs = pSoldier->bActionPoints;

	// OK, If we are in real-time, use another value...
	if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT ) )
	{	// Set to a value which looks good based on out tactical turns duration
		uiAvailAPs = RT_FIRST_AID_GAIN_MODIFIER;
	}

	// calculate how much bandaging CAN be done this turn
	uiPossible = ( uiAvailAPs * uiDressSkill ) / 50;	// max rate is 2 * fullAPs

	// if no healing is possible (insufficient APs or insufficient dressSkill)
	if (!uiPossible)
		return 0;

	if (pSoldier->inv[0].usItem == MEDICKIT )		// using the GOOD medic stuff
		uiPossible += ( uiPossible / 2);			// add extra 50 %

	uiActual = uiPossible;		// start by assuming maximum possible

	// figure out how far below OKLIFE the victim is
	if (pVictim->bLife >= OKLIFE)
		bBelowOKlife = 0;
	else
		bBelowOKlife = OKLIFE - pVictim->bLife;

	// figure out how many healing pts we need to stop dying (2x cost)
	uiDeficiency = (2 * bBelowOKlife );

	// if, after that, the patient will still be bleeding
	if ( (pVictim->bBleeding - bBelowOKlife ) > 0)
	{ // then add how many healing pts we need to stop bleeding (1x cost)
		uiDeficiency += ( pVictim->bBleeding - bBelowOKlife );
	}

	// now, make sure we weren't going to give too much
	if ( uiActual > uiDeficiency)	// if we were about to apply too much
		uiActual = uiDeficiency;	// reduce actual not to waste anything

	// now make sure we HAVE that much
	if( pKit->usItem == MEDICKIT )
	{
		uiMedcost = uiActual / 2;		// cost is only half
		if ( uiMedcost == 0 && uiActual > 0)
			uiMedcost = 1;
		if ( uiMedcost > (UINT32)sKitPts )     		// if we can't afford this
		{
			uiMedcost = sKitPts;		// what CAN we afford?
			uiActual = uiMedcost * 2;		// give double this as aid
		}
	}
	else
	{
		uiMedcost = uiActual;
		if ( uiMedcost == 0 && uiActual > 0)
			uiMedcost = 1;
		if ( uiMedcost > (UINT32)sKitPts)		// can't afford it
			uiMedcost = uiActual = sKitPts;   	// recalc cost AND aid
	}

	bPtsLeft = (INT8)uiActual;
	// heal real life points first (if below OKLIFE) because we don't want the
	// patient still DYING if bandages run out, or medic is disabled/distracted!
	// NOTE: Dressing wounds for life below OKLIFE now costs 2 pts/life point!
	if ( bPtsLeft && pVictim->bLife < OKLIFE)
	{
		// if we have enough points to bring him all the way to OKLIFE this turn
		if ( bPtsLeft >= (2 * bBelowOKlife ) )
		{ // raise life to OKLIFE
			pVictim->bLife = OKLIFE;
			// reduce bleeding by the same number of life points healed up
			pVictim->bBleeding -= bBelowOKlife;
			// use up appropriate # of actual healing points
			bPtsLeft -= (2 * bBelowOKlife);
		}
		else
		{
			pVictim->bLife += ( bPtsLeft / 2);
			pVictim->bBleeding -= ( bPtsLeft / 2);
			bPtsLeft = bPtsLeft % 2;	// if ptsLeft was odd, ptsLeft = 1
		}

		// this should never happen any more, but make sure bleeding not negative
		if (pVictim->bBleeding < 0)
			pVictim->bBleeding = 0;

		// if this healing brought the patient out of the worst of it, cancel dying
		if (pVictim->bLife >= OKLIFE )
		{ // turn off merc QUOTE flags
			pVictim->fDyingComment = FALSE;
		}

		if ( pVictim->bBleeding <= MIN_BLEEDING_THRESHOLD )
		{
			pVictim->fWarnedAboutBleeding = FALSE;
		}
	}

	// if any healing points remain, apply that to any remaining bleeding (1/1)
	// DON'T spend any APs/kit pts to cure bleeding until merc is no longer dying
	//if ( bPtsLeft && pVictim->bBleeding && !pVictim->dying)
	if ( bPtsLeft && pVictim->bBleeding )
	{ // if we have enough points to bandage all remaining bleeding this turn
		if (bPtsLeft >= pVictim->bBleeding )
		{
			bPtsLeft -= pVictim->bBleeding;
			pVictim->bBleeding = 0;
		}
		else		// bandage what we can
		{
			pVictim->bBleeding -= bPtsLeft;
			bPtsLeft = 0;
		}
	}
	// if there are any ptsLeft now, then we didn't actually get to use them
	uiActual -= bPtsLeft;

	// usedAPs equals (actionPts) * (%of possible points actually used)
	uiUsedAPs = ( uiActual * uiAvailAPs ) / uiPossible;

	if (pSoldier->inv[0].usItem == MEDICKIT)	// using the GOOD medic stuff
		uiUsedAPs = ( uiUsedAPs * 2) / 3;	// reverse 50% bonus by taking 2/3rds

	if ( uiActual / 2)
		// MEDICAL GAIN (actual / 2):  Helped someone by giving first aid
		StatChange(*pSoldier, MEDICALAMT, uiActual / 2, FROM_SUCCESS);

	if ( uiActual / 4)
		// DEXTERITY GAIN (actual / 4):  Helped someone by giving first aid
		StatChange(*pSoldier, DEXTAMT, uiActual / 4, FROM_SUCCESS);

	return uiMedcost;
}


static OBJECTTYPE* FindMedicalKit(void)
{
	FOR_ALL_AR_MERCS(i)
	{
		SOLDIERTYPE& s = *i->pSoldier;
		INT32 const slot = FindObjClass(&s, IC_MEDKIT);
		if (slot == NO_SLOT) continue;
		return &s.inv[slot];
	}
	return 0;
}


static void AutoBandageFinishedCallback(MessageBoxReturnValue);


static void AutoBandageMercs(void)
{
	OBJECTTYPE* kit = 0;

	// Do we have any doctors?  If so, bandage selves first.
	UINT32       max_points_used = 0; // XXX write-only, should probably be assigned to parallel_points_used
	SOLDIERTYPE* best            = gpMercs[0].pSoldier;
	FOR_ALL_AR_MERCS(i)
	{
		SOLDIERTYPE& s = *i->pSoldier;
		if (s.bLife < OKLIFE) continue;
		if (s.bCollapsed)     continue;
		if (s.bMedical == 0)  continue;

		// Find the best rated doctor to do all of the further bandaging.
		if (best->bMedical < s.bMedical) best = &s;

		INT8 slot = FindObjClass(&s, IC_MEDKIT);
		if (slot == NO_SLOT) continue;

		// Bandage self first!
		UINT32 curr_points_used = 0;
		INT8   cnt              = 0;
		while (s.bBleeding)
		{
			kit = &s.inv[slot];
			UINT16 const kit_pts = TotalPoints(kit);
			if (kit_pts == 0)
			{ // Attempt to find another kit before stopping
				slot = FindObjClass(&s, IC_MEDKIT);
				if (slot == NO_SLOT) break;
				continue;
			}
			UINT32 const points_used = VirtualSoldierDressWound(&s, &s, kit, kit_pts, kit_pts);
			UseKitPoints(kit, points_used, &s);
			curr_points_used += points_used;
			if (++cnt > 50) break;
		}
		if (max_points_used < curr_points_used) max_points_used = curr_points_used;
		if (!kit) break;
	}

	UINT32 parallel_points_used = 0;
	bool   complete             = true;
	FOR_ALL_AR_MERCS(i)
	{
		SOLDIERTYPE& s = *i->pSoldier;
		while (s.bBleeding != 0 && s.bLife != 0)
		{ // This merc needs medical attention
			if (!kit)
			{
				kit = FindMedicalKit();
				if (!kit)
				{
					complete = false;
					break;
				}
			}
			UINT16 const kit_pts = TotalPoints(kit);
			if (kit_pts == 0)
			{
				kit = 0;
				continue;
			}
			UINT32 const points_used = VirtualSoldierDressWound(best, &s, kit, kit_pts, kit_pts);
			UseKitPoints(kit, points_used, &s);
			parallel_points_used += points_used;
		}
	}
	wchar_t const* const msg = complete ? gzLateLocalizedString[STR_LATE_13] : gzLateLocalizedString[STR_LATE_10];
	DoScreenIndependantMessageBox(msg, MSG_BOX_FLAG_OK, AutoBandageFinishedCallback);

	gpAR->uiTotalElapsedBattleTimeInMilliseconds += parallel_points_used * 200;
}


static void RenderAutoResolve(void)
{
	INT32 xp, yp;
	wchar_t str[100];
	UINT8 ubGood, ubBad;

	if (gpAR->fShowInterface)
	{ //After expanding the window, we now show the interface
		if( gpAR->ubBattleStatus == BATTLE_IN_PROGRESS && !gpAR->fPendingSurrender )
		{
			for (INT32 i = 0 ; i < DONEWIN_BUTTON; ++i)
				ShowButton( gpAR->iButton[ i ] );
			HideButton( gpAR->iButton[ BANDAGE_BUTTON ] );
			HideButton( gpAR->iButton[ YES_BUTTON ] );
			HideButton( gpAR->iButton[ NO_BUTTON ] );
			gpAR->fShowInterface = FALSE;
		}
		else if( gpAR->ubBattleStatus == BATTLE_VICTORY )
		{
			ShowButton( gpAR->iButton[ DONEWIN_BUTTON ] );
			ShowButton( gpAR->iButton[ BANDAGE_BUTTON ] );
		}
		else
		{
			ShowButton( gpAR->iButton[ DONELOSE_BUTTON ] );
		}
	}

	if( !gpAR->fRenderAutoResolve && !gpAR->fDebugInfo )
	{ //update the dirty cells only
		FOR_ALL_AR_MERCS(i)
		{
			if (!(i->uiFlags & CELL_DIRTY)) continue;
			RenderSoldierCell(i);
		}
		FOR_ALL_AR_CIVS(i)
		{
			if (!(i->uiFlags & CELL_DIRTY)) continue;
			RenderSoldierCell(i);
		}
		FOR_ALL_AR_ENEMIES(i)
		{
			if (!(i->uiFlags & CELL_DIRTY)) continue;
			RenderSoldierCell(i);
		}
		return;
	}
	gpAR->fRenderAutoResolve = FALSE;

	BltVideoSurface(FRAME_BUFFER, gpAR->iInterfaceBuffer, gpAR->rect.x, gpAR->rect.y, 0);

	FOR_ALL_AR_MERCS(i)
	{
		RenderSoldierCell(i);
	}
	FOR_ALL_AR_CIVS(i)
	{
		RenderSoldierCell(i);
	}
	FOR_ALL_AR_ENEMIES(i)
	{
		RenderSoldierCell(i);
	}

	//Render the titles
	SetFontAttributes(FONT10ARIALBOLD, FONT_WHITE);

	const wchar_t* EncounterType; // XXX HACK000E
	switch( gubEnemyEncounterCode )
	{
		case ENEMY_ENCOUNTER_CODE:
			EncounterType = gpStrategicString[STR_AR_ENCOUNTER_HEADER];
			break;
		case ENEMY_INVASION_CODE:
		case CREATURE_ATTACK_CODE:
			EncounterType = gpStrategicString[STR_AR_DEFEND_HEADER];
			break;

		default: abort(); // HACK000E
	}

	xp = gpAR->sCenterStartX + 70 - StringPixLength(EncounterType, FONT10ARIALBOLD) / 2;
	yp = gpAR->rect.y + 15;
	MPrint(xp, yp, EncounterType);

	SetFontAttributes(FONT10ARIAL, FONT_GRAY2);

	GetSectorIDString( gpAR->ubSectorX, gpAR->ubSectorY, 0, str, lengthof(str), TRUE );
	xp = gpAR->sCenterStartX + 70 - StringPixLength( str, FONT10ARIAL )/2;
	yp += 11;
	MPrint(xp, yp, str);

	//Display the remaining forces
	ubGood = (UINT8)(gpAR->ubAliveMercs + gpAR->ubAliveCivs);
	ubBad = gpAR->ubAliveEnemies;
	swprintf(str, lengthof(str), gzLateLocalizedString[STR_LATE_17], ubGood, ubBad);

	SetFont( FONT14ARIAL );
	if( ubGood * 3 <= ubBad * 2 )
	{
		SetFontForeground( FONT_LTRED );
	}
	else if( ubGood * 2 >= ubBad * 3 )
	{
		SetFontForeground( FONT_LTGREEN );
	}
	else
	{
		SetFontForeground( FONT_YELLOW );
	}

	xp = gpAR->sCenterStartX + 70 - StringPixLength( str, FONT14ARIAL )/2;
	yp += 11;
	MPrint(xp, yp, str);

#ifdef JA2BETAVERSION
	if (gpAR->fAllowCapture) MPrint(2, 2, L"Enemy capture enabled.");
#endif

	if( gpAR->fPendingSurrender )
	{
		DisplayWrappedString(gpAR->sCenterStartX + 16, 230 + gpAR->bVerticalOffset, 108, 2, FONT10ARIAL, FONT_YELLOW, gpStrategicString[STR_ENEMY_SURRENDER_OFFER], FONT_BLACK, LEFT_JUSTIFIED);
	}

	if( gpAR->ubBattleStatus != BATTLE_IN_PROGRESS )
	{
		// Handle merc morale, Global loyalty, and change of sector control
		if( !gpAR->fMoraleEventsHandled )
		{
			gpAR->uiTotalElapsedBattleTimeInMilliseconds *= 3;
			gpAR->fMoraleEventsHandled = TRUE;
			if (!CheckFact(FACT_FIRST_BATTLE_FOUGHT, 0))
			{
				// this was the first battle against the army
				SetFactTrue( FACT_FIRST_BATTLE_FOUGHT );
				if ( gpAR->ubBattleStatus == BATTLE_VICTORY)
				{
					SetFactTrue( FACT_FIRST_BATTLE_WON );
				}
				SetTheFirstBattleSector( ( INT16 ) (gpAR->ubSectorX + gpAR->ubSectorY * MAP_WORLD_X ) );
				HandleFirstBattleEndingWhileInTown( gpAR->ubSectorX, gpAR->ubSectorY, 0, TRUE );
			}

			switch( gpAR->ubBattleStatus )
			{
				case BATTLE_VICTORY:
					HandleMoraleEvent( NULL, MORALE_BATTLE_WON, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
					HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_BATTLE_WON, gpAR->ubSectorX, gpAR->ubSectorY, 0 );

					SetThisSectorAsPlayerControlled( gpAR->ubSectorX, gpAR->ubSectorY, 0, TRUE );

					SetMusicMode( MUSIC_TACTICAL_VICTORY );
					LogBattleResults( LOG_VICTORY );
					break;

				case BATTLE_SURRENDERED:
				case BATTLE_CAPTURED:
					FOR_ALL_IN_TEAM(i, OUR_TEAM)
					{
						SOLDIERTYPE& s = *i;
						if (s.bLife != 0 && !IsMechanical(s))
						{ //Merc is alive and not a vehicle or robot
							if (PlayerMercInvolvedInThisCombat(s))
							{
								// This morale event is PER INDIVIDUAL SOLDIER
								HandleMoraleEvent(&s, MORALE_MERC_CAPTURED, gpAR->ubSectorX, gpAR->ubSectorY, 0);
							}
						}
					}
					HandleMoraleEvent( NULL, MORALE_HEARD_BATTLE_LOST, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
					HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_BATTLE_LOST, gpAR->ubSectorX, gpAR->ubSectorY, 0 );

					SetMusicMode( MUSIC_TACTICAL_DEATH );
					gsEnemyGainedControlOfSectorID = (INT16)SECTOR( gpAR->ubSectorX, gpAR->ubSectorY );
					break;
				case BATTLE_DEFEAT:
					HandleMoraleEvent( NULL, MORALE_HEARD_BATTLE_LOST, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
					HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_BATTLE_LOST, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
					if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
					{
						gsEnemyGainedControlOfSectorID = (INT16)SECTOR( gpAR->ubSectorX, gpAR->ubSectorY );
					}
					else
					{
						gsEnemyGainedControlOfSectorID = (INT16)SECTOR( gpAR->ubSectorX, gpAR->ubSectorY );
						gsCiviliansEatenByMonsters = gpAR->ubAliveEnemies;
					}
					SetMusicMode( MUSIC_TACTICAL_DEATH );
					LogBattleResults( LOG_DEFEAT );
					break;

				case BATTLE_RETREAT:

					//Tack on 5 minutes for retreat.
					gpAR->uiTotalElapsedBattleTimeInMilliseconds += 300000;

					HandleLoyaltyImplicationsOfMercRetreat( RETREAT_AUTORESOLVE, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
					if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
					{
						gsEnemyGainedControlOfSectorID = (INT16)SECTOR( gpAR->ubSectorX, gpAR->ubSectorY );
					}
					else if( gpAR->ubAliveEnemies )
					{
						gsEnemyGainedControlOfSectorID = (INT16)SECTOR( gpAR->ubSectorX, gpAR->ubSectorY );
						gsCiviliansEatenByMonsters = gpAR->ubAliveEnemies;
					}
					break;
			}
		}
		//Render the end battle condition.
			const wchar_t* BattleResult; // XXX HACK000E
			switch( gpAR->ubBattleStatus )
			{
				case BATTLE_VICTORY:
					SetFontForeground( FONT_LTGREEN );
					BattleResult = gpStrategicString[STR_AR_OVER_VICTORY];
					break;
				case BATTLE_SURRENDERED:
				case BATTLE_CAPTURED:
					if( gpAR->ubBattleStatus == BATTLE_SURRENDERED )
					{
						BattleResult = gpStrategicString[STR_AR_OVER_SURRENDERED];
					}
					else
					{
						DisplayWrappedString(gpAR->sCenterStartX + 16, 310, 108, 2, FONT10ARIAL, FONT_YELLOW, gpStrategicString[STR_ENEMY_CAPTURED], FONT_BLACK, LEFT_JUSTIFIED);
						BattleResult = gpStrategicString[STR_AR_OVER_CAPTURED];
					}
					SetFontForeground( FONT_RED );
					break;
				case BATTLE_DEFEAT:
					SetFontForeground( FONT_RED );
					BattleResult = gpStrategicString[STR_AR_OVER_DEFEAT];
					break;
				case BATTLE_RETREAT:
					SetFontForeground( FONT_YELLOW );
					BattleResult = gpStrategicString[STR_AR_OVER_RETREATED];
					break;

				default: abort(); // HACK000E
			}
			//Render the results of the battle.
			SetFont( BLOCKFONT2 );
			xp = gpAR->sCenterStartX + 12;
			yp = 218 + gpAR->bVerticalOffset;
			BltVideoObject( FRAME_BUFFER, gpAR->iIndent, 0, xp, yp);
			xp = gpAR->sCenterStartX + 70 - StringPixLength(BattleResult, BLOCKFONT2) / 2;
			yp = 227 + gpAR->bVerticalOffset;
			MPrint(xp, yp, BattleResult);

			//Render the total battle time elapsed.
			SetFont( FONT10ARIAL );
			swprintf(str, lengthof(str), L"%ls:  %d%ls %02d%ls",
				gpStrategicString[ STR_AR_TIME_ELAPSED ],
				gpAR->uiTotalElapsedBattleTimeInMilliseconds/60000,
				gsTimeStrings[1],
				gpAR->uiTotalElapsedBattleTimeInMilliseconds % 60000 / 1000,
				gsTimeStrings[2]);
			xp = gpAR->sCenterStartX + 70 - StringPixLength( str, FONT10ARIAL )/2;
			yp = 290 + gpAR->bVerticalOffset;
			SetFontForeground( FONT_YELLOW );
			MPrint(xp, yp, str);
	}

	MarkButtonsDirty();
	InvalidateScreen();
}


static void MakeButton(UINT idx, INT16 x, INT16 y, GUI_CALLBACK click, BOOLEAN hide, const wchar_t* text)
{
	GUIButtonRef const btn = QuickCreateButton(gpAR->iButtonImage[idx], x, y, MSYS_PRIORITY_HIGH, click);
	gpAR->iButton[idx] = btn;
	if (text != NULL) btn->SpecifyGeneralTextAttributes(text, BLOCKFONT2, 169, FONT_NEARBLACK);
	if (hide) btn->Hide();
}


static SOLDIERCELL* MakeEnemyTroops(SOLDIERCELL* cell, size_t n, AUTORESOLVE_STRUCT* const ar, SoldierClass const sc, UINT16 const face, wchar_t const* const name)
{
	for (; n != 0; --n, ++cell)
	{
		SOLDIERTYPE* const s = TacticalCreateEnemySoldier(sc);
		cell->pSoldier       = s;
		cell->uiVObjectID    = ar->iFaces;
		// Only elite troops have women
		cell->usIndex        = s->ubBodyType == REGFEMALE ? ELITEF_FACE : face;
		s->sSectorX          = ar->ubSectorX;
		s->sSectorY          = ar->ubSectorY;
		wcslcpy(s->name, name, lengthof(s->name));
	}
	return cell;
}


static SOLDIERCELL* MakeCreatures(SOLDIERCELL* cell, size_t n, AUTORESOLVE_STRUCT* const ar, INT8 const body_type, UINT16 const face)
{
	for (; n != 0; --n, ++cell)
	{
		SOLDIERTYPE* const s = TacticalCreateCreature(body_type);
		cell->pSoldier       = s;
		cell->uiVObjectID    = ar->iFaces;
		cell->usIndex        = face;
		s->sSectorX          = ar->ubSectorX;
		s->sSectorY          = ar->ubSectorY;
		wcslcpy(s->name, gpStrategicString[STR_AR_CREATURE_NAME], lengthof(s->name));
	}
	return cell;
}


static void AcceptSurrenderCallback(GUI_BUTTON* btn, INT32 reason);
static void BandageButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void DoneButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void FastButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void FinishButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void PauseButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void PlayButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void RejectSurrenderCallback(GUI_BUTTON* btn, INT32 reason);
static void RetreatButtonCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateAutoResolveInterface(void)
{
	AUTORESOLVE_STRUCT* const ar = gpAR;

	// Setup new autoresolve blanket interface.
	MSYS_DefineRegion(&ar->AutoResolveRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH - 1, 0, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
	ar->fRenderAutoResolve = TRUE;
	ar->fExitAutoResolve   = FALSE;

	//Load the general panel image pieces, to be combined to make the dynamically sized window.
	ar->iPanelImages = AddVideoObjectFromFile("Interface/AutoResolve.sti");

	// Load the button images file, and assign it to the first button.
	BUTTON_PICS* const btn_pics = LoadButtonImage("Interface/AutoBtns.sti", 0, 7);
	ar->iButtonImage[PAUSE_BUTTON]    = btn_pics;
	// Have the other buttons hook into the first button containing the images.
	ar->iButtonImage[PLAY_BUTTON]     = UseLoadedButtonImage(btn_pics,  1,  8);
	ar->iButtonImage[FAST_BUTTON]     = UseLoadedButtonImage(btn_pics,  2,  9);
	ar->iButtonImage[FINISH_BUTTON]   = UseLoadedButtonImage(btn_pics,  3, 10);
	ar->iButtonImage[YES_BUTTON]      = UseLoadedButtonImage(btn_pics,  4, 11);
	ar->iButtonImage[NO_BUTTON]       = UseLoadedButtonImage(btn_pics,  5, 12);
	ar->iButtonImage[BANDAGE_BUTTON]  = UseLoadedButtonImage(btn_pics,  6, 13);
	ar->iButtonImage[RETREAT_BUTTON]  = UseLoadedButtonImage(btn_pics, 14, 15);
	ar->iButtonImage[DONEWIN_BUTTON]  = UseLoadedButtonImage(btn_pics, 14, 15);
	ar->iButtonImage[DONELOSE_BUTTON] = UseLoadedButtonImage(btn_pics, 16, 17);

	// Load the generic faces for civs and enemies
	SGPVObject* const faces = AddVideoObjectFromFile("Interface/SmFaces.sti");
	ar->iFaces = faces;
	SGPPaletteEntry const* const pal = faces->Palette();
	faces->pShades[0] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);
	faces->pShades[1] = Create16BPPPaletteShaded(pal, 250,  25,  25, TRUE);

	// Add the battle over panels
	ar->iIndent = AddVideoObjectFromFile("Interface/indent.sti");

	// Add all the faces now
	FOR_ALL_AR_MERCS(cell)
	{
		//Load the face
		SGPFILENAME ImageFile;
		sprintf(ImageFile, "Faces/65Face/%02d.sti", GetProfile(cell->pSoldier->ubProfile).ubFaceIndex);
		SGPVObject* face;
		try
		{
			face = AddVideoObjectFromFile(ImageFile);
		}
		catch (...)
		{
			face = AddVideoObjectFromFile("Faces/65Face/speck.sti");
		}
		cell->uiVObjectID = face;
		SGPPaletteEntry const* const pal = face->Palette();
		face->pShades[0] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);
		face->pShades[1] = Create16BPPPaletteShaded(pal, 250,  25,  25, TRUE);
	}

	UINT8 n_militia_elite = MilitiaInSectorOfRank(ar->ubSectorX, ar->ubSectorY, ELITE_MILITIA);
	UINT8 n_militia_reg   = MilitiaInSectorOfRank(ar->ubSectorX, ar->ubSectorY, REGULAR_MILITIA);
	UINT8 n_militia_green = MilitiaInSectorOfRank(ar->ubSectorX, ar->ubSectorY, GREEN_MILITIA);
	while (n_militia_elite + n_militia_reg + n_militia_green < ar->ubCivs)
	{
		switch (PreRandom(3))
		{
			case 0: ++n_militia_elite; break;
			case 1:	++n_militia_reg;   break;
			case 2:	++n_militia_green; break;
		}
	}
	for (INT32 i = 0; i < ar->ubCivs; ++i)
	{
		// reset counter of how many mortars this team has rolled
		ResetMortarsOnTeamCount();

		SOLDIERTYPE*       s;
		UINT16             idx;
		if (i < n_militia_elite)
		{
			s   = TacticalCreateMilitia(SOLDIER_CLASS_ELITE_MILITIA);
			idx = s->ubBodyType == REGFEMALE ? MILITIA3F_FACE : MILITIA3_FACE;
		}
		else if (i < n_militia_reg + n_militia_elite)
		{
			s   = TacticalCreateMilitia(SOLDIER_CLASS_REG_MILITIA);
			idx = s->ubBodyType == REGFEMALE ? MILITIA2F_FACE : MILITIA2_FACE;
		}
		else if (i < n_militia_green + n_militia_reg + n_militia_elite)
		{
			s   = TacticalCreateMilitia(SOLDIER_CLASS_GREEN_MILITIA);
			idx = s->ubBodyType == REGFEMALE ? MILITIA1F_FACE : MILITIA1_FACE;
		}
		else
		{
			AssertMsg(0, "Attempting to illegally create a militia soldier.");
			s   = 0;
			idx = 0;
		}
		SOLDIERCELL* const cell = &gpCivs[i];
		cell->pSoldier    = s;
		cell->usIndex     = idx;
		cell->uiVObjectID = ar->iFaces;

		AssertMsg(s, "Failed to create militia soldier for autoresolve.");
		s->sSectorX = ar->ubSectorX;
		s->sSectorY = ar->ubSectorY;
		wcslcpy(s->name, gpStrategicString[STR_AR_MILITIA_NAME], lengthof(s->name));
	}

	if (gubEnemyEncounterCode != CREATURE_ATTACK_CODE)
	{
		SOLDIERCELL* cell = gpEnemies;
		cell = MakeEnemyTroops(cell, ar->ubElites, ar, SOLDIER_CLASS_ELITE,         ELITE_FACE, gpStrategicString[STR_AR_ELITE_NAME]);
		cell = MakeEnemyTroops(cell, ar->ubTroops, ar, SOLDIER_CLASS_ARMY,          TROOP_FACE, gpStrategicString[STR_AR_TROOP_NAME]);
		cell = MakeEnemyTroops(cell, ar->ubAdmins, ar, SOLDIER_CLASS_ADMINISTRATOR, ADMIN_FACE, gpStrategicString[STR_AR_ADMINISTRATOR_NAME]);
		AssociateEnemiesWithStrategicGroups();
	}
	else
	{
		SOLDIERCELL* cell = gpEnemies;
		cell = MakeCreatures(cell, ar->ubAFCreatures, ar, ADULTFEMALEMONSTER, AF_CREATURE_FACE);
		cell = MakeCreatures(cell, ar->ubAMCreatures, ar, AM_MONSTER,         AM_CREATURE_FACE);
		cell = MakeCreatures(cell, ar->ubYFCreatures, ar, YAF_MONSTER,        YF_CREATURE_FACE);
		cell = MakeCreatures(cell, ar->ubYMCreatures, ar, YAM_MONSTER,        YM_CREATURE_FACE);
	}

	if (ar->ubSectorX  == gWorldSectorX &&
			ar->ubSectorY  == gWorldSectorY &&
			gbWorldSectorZ == 0)
	{
		CheckAndHandleUnloadingOfCurrentWorld();
	}
	else
	{
		gfBlitBattleSectorLocator = FALSE;
	}

	/* Build the interface buffer, and blit the "shaded" background.  This info
	 * won't change from now on, but will be used to restore text. */
	BuildInterfaceBuffer();
	BltVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 0);

	/* If we are bumping up the interface, then also use that piece of info to
	 * move the buttons up by the same amount. */
	ar->bVerticalOffset = (SCREEN_HEIGHT - ar->rect.h) / 2 > 120 ? -40 : 0;

	const INT16 dx = ar->sCenterStartX;
	const INT16 dy = ar->bVerticalOffset + SCREEN_HEIGHT / 2;

	// Create the buttons -- subject to relocation
	MakeButton(PLAY_BUTTON,     dx + 11, dy,      PlayButtonCallback,      FALSE, 0);
	MakeButton(FAST_BUTTON,     dx + 51, dy,      FastButtonCallback,      FALSE, 0);
	MakeButton(FINISH_BUTTON,   dx + 91, dy,      FinishButtonCallback,    FALSE, 0);
	MakeButton(PAUSE_BUTTON,    dx + 11, dy + 34, PauseButtonCallback,     FALSE, 0);
	MakeButton(RETREAT_BUTTON,  dx + 51, dy + 34, RetreatButtonCallback,   FALSE, gpStrategicString[STR_AR_RETREAT_BUTTON]);
	if (!ar->ubMercs) DisableButton(ar->iButton[RETREAT_BUTTON]);

	MakeButton(BANDAGE_BUTTON,  dx + 11, dy +  5, BandageButtonCallback,   TRUE,  0);
	MakeButton(DONEWIN_BUTTON,  dx + 51, dy +  5, DoneButtonCallback,      TRUE,  gpStrategicString[STR_AR_DONE_BUTTON]);
	MakeButton(DONELOSE_BUTTON, dx + 25, dy +  5, DoneButtonCallback,      TRUE,  gpStrategicString[STR_AR_DONE_BUTTON]);
	MakeButton(YES_BUTTON,      dx + 21, dy + 17, AcceptSurrenderCallback, TRUE,  0);
	MakeButton(NO_BUTTON,       dx + 81, dy + 17, RejectSurrenderCallback, TRUE,  0);
	ar->iButton[PLAY_BUTTON]->uiFlags |= BUTTON_CLICKED_ON;
}


static bool IsAnybodyWounded()
{
	FOR_ALL_AR_MERCS(i)
	{
		SOLDIERTYPE const& s = *i->pSoldier;
		if (s.bBleeding == 0 || s.bLife == 0) continue;
		return true;
	}
	return false;
}


static void RemoveAutoResolveInterface(BOOLEAN fDeleteForGood)
{
	INT32 i;
	UINT8 ubCurrentRank;
	UINT8 ubCurrentGroupID = 0;
	BOOLEAN fFirstGroup = TRUE;

	MSYS_RemoveRegion( &gpAR->AutoResolveRegion );
	DeleteVideoObject(gpAR->iPanelImages);
	DeleteVideoObject(gpAR->iFaces);
	DeleteVideoObject(gpAR->iIndent);
	DeleteVideoSurface(gpAR->iInterfaceBuffer);

	if( fDeleteForGood )
	{ //Delete the soldier instances -- done when we are completely finished.

		//KM: By request of AM, I have added this bleeding event in cases where autoresolve is
		//	  complete and there are bleeding mercs remaining.  AM coded the internals
		//    of the strategic event.
		if (IsAnybodyWounded())
		{
			// ARM: only one event is needed regardless of how many are bleeding
			AddStrategicEvent(EVENT_BANDAGE_BLEEDING_MERCS, GetWorldTotalMin() + 1, 0);
		}

		// ARM: Update assignment flashing: Doctors may now have new patients or lost them all, etc.
		gfReEvaluateEveryonesNothingToDo = TRUE;


		if( gpAR->pRobotCell)
		{
			UpdateRobotControllerGivenRobot( gpAR->pRobotCell->pSoldier );
		}
		for( i = 0; i < gpAR->iNumMercFaces; i++ )
		{
			if( i >= gpAR->iActualMercFaces )
				TacticalRemoveSoldier(*gpMercs[i].pSoldier);
			else
			{ //Record finishing information for our mercs
				if( !gpMercs[ i ].pSoldier->bLife )
				{
					StrategicHandlePlayerTeamMercDeath( gpMercs[ i ].pSoldier );
				}
				else if( gpAR->ubBattleStatus == BATTLE_SURRENDERED || gpAR->ubBattleStatus == BATTLE_CAPTURED )
				{
					EnemyCapturesPlayerSoldier( gpMercs[ i ].pSoldier );
				}
				else if( gpAR->ubBattleStatus == BATTLE_VICTORY )
				{ //merc is alive, so group them at the center gridno.
					gpMercs[ i ].pSoldier->ubStrategicInsertionCode = INSERTION_CODE_CENTER;
				}
				gMercProfiles[ gpMercs[ i ].pSoldier->ubProfile ].usBattlesFought++;
			}
		}
		for( i = 0; i < gpAR->iNumMercFaces; i++ )
		{
			if( gpAR->ubBattleStatus == BATTLE_VICTORY && gpMercs[ i ].pSoldier->bLife >= OKLIFE )
			{
				if( gpMercs[ i ].pSoldier->ubGroupID != ubCurrentGroupID )
				{
					ubCurrentGroupID = gpMercs[ i ].pSoldier->ubGroupID;

					// look for NPCs to stop for, anyone is too tired to keep going, if all OK rebuild waypoints & continue movement
					// NOTE: Only the first group found will stop for NPCs, it's just too much hassle to stop them all
					PlayerGroupArrivedSafelyInSector(*GetGroup(gpMercs[i].pSoldier->ubGroupID), fFirstGroup);
					fFirstGroup = FALSE;
				}
			}
			gpMercs[ i ].pSoldier = NULL;
		}

		// End capture squence....
		if( gpAR->ubBattleStatus == BATTLE_SURRENDERED || gpAR->ubBattleStatus == BATTLE_CAPTURED )
		{
			EndCaptureSequence( );
		}

	}
	//Delete all of the faces.
	for( i = 0; i < gpAR->iNumMercFaces; i++ )
	{
		if (gpMercs[i].uiVObjectID)
		{
			DeleteVideoObject(gpMercs[i].uiVObjectID);
			gpMercs[i].uiVObjectID = 0;
		}
		MouseRegion*& r = gpMercs[i].pRegion;
		delete r;
		r = 0;
	}
	//Delete all militia
	gbGreenToElitePromotions = 0;
	gbGreenToRegPromotions = 0;
	gbRegToElitePromotions = 0;
	gbMilitiaPromotions = 0;
	for( i = 0; i < MAX_ALLOWABLE_MILITIA_PER_SECTOR; i++ )
	{
		if( gpCivs[ i ].pSoldier )
		{
			ubCurrentRank = 255;
			switch( gpCivs[ i ].pSoldier->ubSoldierClass )
			{
				case SOLDIER_CLASS_GREEN_MILITIA:		ubCurrentRank = GREEN_MILITIA;		break;
				case SOLDIER_CLASS_REG_MILITIA:			ubCurrentRank = REGULAR_MILITIA;	break;
				case SOLDIER_CLASS_ELITE_MILITIA:		ubCurrentRank = ELITE_MILITIA;		break;
				default:
					#ifdef JA2BETAVERSION
						ScreenMsg( FONT_RED, MSG_ERROR, L"Removing autoresolve militia with invalid ubSoldierClass %d.",gpCivs[ i ].pSoldier->ubSoldierClass );
					#endif
					break;
			}
			if( fDeleteForGood && gpCivs[ i ].pSoldier->bLife < OKLIFE/2 )
			{
				AddDeadSoldierToUnLoadedSector( gpAR->ubSectorX, gpAR->ubSectorY, 0, gpCivs[ i ].pSoldier, RandomGridNo(), ADD_DEAD_SOLDIER_TO_SWEETSPOT );
				StrategicRemoveMilitiaFromSector( gpAR->ubSectorX, gpAR->ubSectorY, ubCurrentRank, 1 );
				HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_NATIVE_KILLED, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
			}
			else
			{
				UINT8 ubPromotions;
				// this will check for promotions and handle them for you
				if( fDeleteForGood && ( gpCivs[ i ].pSoldier->ubMilitiaKills > 0) && ( ubCurrentRank < ELITE_MILITIA ) )
				{
					ubPromotions = CheckOneMilitiaForPromotion( gpAR->ubSectorX, gpAR->ubSectorY, ubCurrentRank, gpCivs[ i ].pSoldier->ubMilitiaKills );
					if( ubPromotions )
					{
						if( ubPromotions == 2 )
						{
							gbGreenToElitePromotions++;
							gbMilitiaPromotions++;
						}
						else if( gpCivs[ i ].pSoldier->ubSoldierClass == SOLDIER_CLASS_GREEN_MILITIA )
						{
							gbGreenToRegPromotions++;
							gbMilitiaPromotions++;
						}
						else if( gpCivs[ i ].pSoldier->ubSoldierClass == SOLDIER_CLASS_REG_MILITIA )
						{
							gbRegToElitePromotions++;
							gbMilitiaPromotions++;
						}
					}
				}
			}
			TacticalRemoveSoldier(*gpCivs[i].pSoldier);
			memset( &gpCivs[ i ], 0, sizeof( SOLDIERCELL ) );
		}
	}

	//Record and process all enemy deaths
	for( i = 0; i < 32; i++ )
	{
		if( gpEnemies[ i ].pSoldier )
		{
			if( fDeleteForGood && gpEnemies[ i ].pSoldier->bLife < OKLIFE )
			{
				TrackEnemiesKilled( ENEMY_KILLED_IN_AUTO_RESOLVE, gpEnemies[ i ].pSoldier->ubSoldierClass );
				HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_ENEMY_KILLED, gpAR->ubSectorX, gpAR->ubSectorY, 0 );
				ProcessQueenCmdImplicationsOfDeath( gpEnemies[ i ].pSoldier );
				AddDeadSoldierToUnLoadedSector( gpAR->ubSectorX, gpAR->ubSectorY, 0, gpEnemies[ i ].pSoldier, RandomGridNo(), ADD_DEAD_SOLDIER_TO_SWEETSPOT );
			}
		}
	}
	//Eliminate all excess soldiers (as more than 32 can exist in the same battle.
	//Autoresolve only processes 32, so the excess is slaughtered as the player never
	//knew they existed.
	if( fDeleteForGood )
	{ //Warp the game time accordingly
		if( gpAR->ubBattleStatus == BATTLE_VICTORY )
		{	//Get rid of any extra enemies that could be here.  It is possible for the number of total enemies to exceed 32, but
			//autoresolve can only process 32.  We basically cheat by eliminating the rest of them.
			EliminateAllEnemies( gpAR->ubSectorX, gpAR->ubSectorY );
		}
		else
		{ //The enemy won, so repoll movement.
			ResetMovementForEnemyGroupsInLocation( gpAR->ubSectorX, gpAR->ubSectorY );
		}
	}
	//Physically delete the soldiers now.
	for( i = 0; i < 32; i++ )
	{
		if( gpEnemies[ i ].pSoldier )
		{
			TacticalRemoveSoldier(*gpEnemies[i].pSoldier);
			memset( &gpEnemies[ i ], 0, sizeof( SOLDIERCELL ) );
		}
	}

	for( i = 0; i < NUM_AR_BUTTONS; i++ )
	{
		UnloadButtonImage( gpAR->iButtonImage[ i ] );
		RemoveButton( gpAR->iButton[ i ] );
	}
	if( fDeleteForGood )
	{ //Warp the game time accordingly

		WarpGameTime( gpAR->uiTotalElapsedBattleTimeInMilliseconds/1000, WARPTIME_NO_PROCESSING_OF_EVENTS );

		//Deallocate all of the global memory.
		//Everything internal to them, should have already been deleted.
		MemFree( gpAR );
		gpAR = NULL;

		MemFree( gpMercs );
		gpMercs = NULL;

		MemFree( gpCivs );
		gpCivs = NULL;

		MemFree( gpEnemies );
		gpEnemies = NULL;

	}

	//KM : Aug 09, 1999 Patch fix -- Would break future dialog while time compressing
	gTacticalStatus.ubCurrentTeam = gbPlayerNum;

	gpBattleGroup = NULL;

	if( gubEnemyEncounterCode == CREATURE_ATTACK_CODE )
	{
		gubNumCreaturesAttackingTown = 0;
		gubSectorIDOfCreatureAttack = 0;
	}
}


static void DepressAutoButton(UINT btn)
{
	gpAR->iButton[PAUSE_BUTTON ]->uiFlags &= ~BUTTON_CLICKED_ON;
	gpAR->iButton[PLAY_BUTTON  ]->uiFlags &= ~BUTTON_CLICKED_ON;
	gpAR->iButton[FAST_BUTTON  ]->uiFlags &= ~BUTTON_CLICKED_ON;
	gpAR->iButton[FINISH_BUTTON]->uiFlags &= ~BUTTON_CLICKED_ON;
	gpAR->iButton[btn          ]->uiFlags |=  BUTTON_CLICKED_ON;
	gpAR->fPaused = (btn == PAUSE_BUTTON);
}


static void PauseButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		DepressAutoButton(PAUSE_BUTTON);
	}
}


static void PlayButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		DepressAutoButton(PLAY_BUTTON);
		gpAR->uiTimeSlice = 1000 * gpAR->ubTimeModifierPercentage / 100;
	}
}


static void FastButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		DepressAutoButton(FAST_BUTTON);
		gpAR->uiTimeSlice = 4000;
	}
}


static void FinishButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		DepressAutoButton(FINISH_BUTTON);
		gpAR->uiTimeSlice = 0xffffffff;
		gpAR->fSound = FALSE;
		PlayJA2StreamingSample(AUTORESOLVE_FINISHFX, HIGHVOLUME, 1, MIDDLEPAN);
	}
}


static void RetreatButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		FOR_ALL_AR_MERCS(i)
		{
			if (i->uiFlags & (CELL_RETREATING | CELL_RETREATED)) continue;
			i->uiFlags           |= CELL_RETREATING | CELL_DIRTY;
			// Gets to retreat after a total of 2 attacks.
			i->usNextAttack       = (1000 + i->usNextAttack * 2 + PreRandom(2000 - i->usAttack)) * 2;
			gpAR->usPlayerAttack -= i->usAttack;
			i->usAttack           = 0;
		}
		if( gpAR->pRobotCell )
		{ //if robot is retreating, set the retreat time to be the same as the robot's controller.
			const SOLDIERTYPE* const robot_controller = gpAR->pRobotCell->pSoldier->robot_remote_holder;
			if (robot_controller == NULL)
			{
				gpAR->pRobotCell->uiFlags &= ~CELL_RETREATING;
				gpAR->pRobotCell->uiFlags |= CELL_DIRTY;
				gpAR->pRobotCell->usNextAttack = 0xffff;
				return;
			}
			FOR_ALL_AR_MERCS(i)
			{
				if (robot_controller != i->pSoldier) continue;
				// Found the controller, make the robot's retreat time match the contollers.
				gpAR->pRobotCell->usNextAttack = i->usNextAttack;
				return;
			}
		}
	}
}


static bool CanAnybodyBandage()
{
	FOR_ALL_AR_MERCS(i)
	{
		SOLDIERTYPE const& s = *i->pSoldier;
		if (s.bLife < OKLIFE || s.bCollapsed || s.bMedical == 0) continue;
		return true;
	}
	return false;
}


static void DetermineBandageButtonState(void)
{
	wchar_t const* help;
	bool           enable = false;
	if (!IsAnybodyWounded()) // Does anyone need bandaging?
	{
		help = gzLateLocalizedString[STR_LATE_11];
	}
	else if (!CanAnybodyBandage()) // Do we have any doctors?
	{	// No doctors
		help = gzLateLocalizedString[STR_LATE_08];
	}
	else if (!FindMedicalKit()) // Do have a kit?
	{ // No kits
		help = gzLateLocalizedString[STR_LATE_09];
	}
	else
	{ // Allow bandaging
		help   = gzLateLocalizedString[STR_LATE_12];
		enable = true;
	}
	GUI_BUTTON& b = *gpAR->iButton[BANDAGE_BUTTON];
	EnableButton(&b, enable);
	b.SetFastHelpText(help);
}


static void SetupDoneInterface(void);


static void BandageButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		AutoBandageMercs();
		SetupDoneInterface();
	}
}


static void DoneButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gpAR->fExitAutoResolve = TRUE;
	}
}


// Find the merc with the same region.
static SOLDIERCELL* GetCell(MOUSE_REGION const* const reg)
{
	FOR_ALL_AR_MERCS(i)
	{
		if (&i->pRegion->Base() != reg) continue;
		return i;
	}
	throw std::logic_error("Region does not belong to a soldier cell");
}


static void MercCellMouseMoveCallback(MOUSE_REGION* reg, INT32 reason)
{
	SOLDIERCELL* const pCell = GetCell(reg);
	if( gpAR->fPendingSurrender )
	{ //Can't setup retreats when pending surrender.
		pCell->uiFlags &= ~CELL_SHOWRETREATTEXT;
		pCell->uiFlags |= CELL_DIRTY;
		return;
	}
	if( reg->uiFlags & MSYS_MOUSE_IN_AREA )
	{
		if( !(pCell->uiFlags & CELL_SHOWRETREATTEXT) )
			pCell->uiFlags |= CELL_SHOWRETREATTEXT | CELL_DIRTY;
	}
	else
	{
		if( pCell->uiFlags & CELL_SHOWRETREATTEXT )
		{
			pCell->uiFlags &= ~CELL_SHOWRETREATTEXT;
			pCell->uiFlags |= CELL_DIRTY;
		}
	}
}


static void MercCellMouseClickCallback(MOUSE_REGION* reg, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( gpAR->fPendingSurrender )
		{ //Can't setup retreats when pending surrender.
			return;
		}

		SOLDIERCELL* const pCell = GetCell(reg);

		if( pCell->uiFlags & ( CELL_RETREATING | CELL_RETREATED ) )
		{ //already retreated/retreating.
			return;
		}

		if( pCell == gpAR->pRobotCell )
		{ //robot retreats only when controller retreats
			return;
		}

		pCell->uiFlags |= CELL_RETREATING | CELL_DIRTY;
		//Gets to retreat after a total of 2 attacks.
		pCell->usNextAttack = (UINT16)((1000 + pCell->usNextAttack * 5 + PreRandom( 2000 - pCell->usAttack ))*2);
		gpAR->usPlayerAttack -= pCell->usAttack;
		pCell->usAttack = 0;

		if( gpAR->pRobotCell )
		{ //if controller is retreating, make the robot retreat too.
			const SOLDIERTYPE* const robot_controller = gpAR->pRobotCell->pSoldier->robot_remote_holder;
			if (robot_controller == NULL)
			{
				gpAR->pRobotCell->uiFlags &= ~CELL_RETREATING;
				gpAR->pRobotCell->uiFlags |= CELL_DIRTY;
				gpAR->pRobotCell->usNextAttack = 0xffff;
			}
			else if (robot_controller == pCell->pSoldier)
			{ //Found the controller, make the robot's retreat time match the contollers.
				gpAR->pRobotCell->uiFlags |= CELL_RETREATING | CELL_DIRTY;
				gpAR->pRobotCell->usNextAttack = pCell->usNextAttack;
				gpAR->usPlayerAttack -= gpAR->pRobotCell->usAttack;
				gpAR->pRobotCell->usAttack = 0;
				return;
			}
		}
	}
}


static void CalculateRowsAndColumns(void);


//Determine how many players, militia, and enemies that are going at it, and use these values
//to figure out how many rows and columns we can use.  The will effect the size of the panel.
static void CalculateAutoResolveInfo(void)
{
	Assert( gpAR->ubSectorX >= 1 && gpAR->ubSectorX <= 16 );
	Assert( gpAR->ubSectorY >= 1 && gpAR->ubSectorY <= 16 );

	if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
	{
		GetNumberOfEnemiesInSector( gpAR->ubSectorX, gpAR->ubSectorY,
																&gpAR->ubAdmins, &gpAR->ubTroops, &gpAR->ubElites );
		gpAR->ubEnemies = (UINT8)MIN( gpAR->ubAdmins + gpAR->ubTroops + gpAR->ubElites, 32 );
	}
	else
	{
		if( gfTransferTacticalOppositionToAutoResolve )
		{
			DetermineCreatureTownCompositionBasedOnTacticalInformation( &gubNumCreaturesAttackingTown,
																				&gpAR->ubYMCreatures, &gpAR->ubYFCreatures,
																				&gpAR->ubAMCreatures, &gpAR->ubAFCreatures );
		}
		else
		{
			DetermineCreatureTownComposition( gubNumCreaturesAttackingTown,
																				&gpAR->ubYMCreatures, &gpAR->ubYFCreatures,
																				&gpAR->ubAMCreatures, &gpAR->ubAFCreatures );
		}
		gpAR->ubEnemies = (UINT8)MIN( gpAR->ubYMCreatures + gpAR->ubYFCreatures + gpAR->ubAMCreatures + gpAR->ubAFCreatures, 32 );
	}
	gfTransferTacticalOppositionToAutoResolve = FALSE;
	gpAR->ubCivs = CountAllMilitiaInSector( gpAR->ubSectorX, gpAR->ubSectorY );
	gpAR->ubMercs = 0;
	CFOR_ALL_GROUPS(i)
	{
		GROUP const& g = *i;
		if (!PlayerGroupInvolvedInThisCombat(g)) continue;

		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, &g)
		{
			SOLDIERTYPE& s = *pPlayer->pSoldier;
			// NOTE: Must check each merc individually, e.g. Robot without controller is an uninvolved merc on an involved group!
			if (PlayerMercInvolvedInThisCombat(s))
			{
				gpMercs[gpAR->ubMercs].pSoldier = &s;

				//!!! CLEAR OPPCOUNT HERE.  All of these soldiers are guaranteed to not be in tactical anymore.
				//ClearOppCount(&s);

				gpAR->ubMercs++;
				if (AM_AN_EPC(&s))
				{
					gpAR->fCaptureNotPermittedDueToEPCs = TRUE;
				}
				if (AM_A_ROBOT(&s))
				{
					gpAR->pRobotCell = &gpMercs[ gpAR->ubMercs - 1 ];
					UpdateRobotControllerGivenRobot( gpAR->pRobotCell->pSoldier );
				}
			}
		}
	}
	gpAR->iNumMercFaces = gpAR->ubMercs;
	gpAR->iActualMercFaces = gpAR->ubMercs;

	CalculateRowsAndColumns();
}


static void CreateTempPlayerMerc(void);


// Debug utilities
static void ResetAutoResolveInterface(void)
{
	guiPreRandomIndex = gpAR->uiPreRandomIndex;

	RemoveAutoResolveInterface( FALSE );

	gpAR->ubBattleStatus = BATTLE_IN_PROGRESS;

	if( !gpAR->ubCivs && !gpAR->ubMercs )
		gpAR->ubCivs = 1;

	//Make sure the number of enemy portraits is the same as needed.
	//The debug keypresses may add or remove more than one at a time.
	while( gpAR->ubElites + gpAR->ubAdmins + gpAR->ubTroops > gpAR->ubEnemies )
	{
		switch( PreRandom( 5 ) )
		{
			case 0:					if( gpAR->ubElites ) { gpAR->ubElites--; break; }
			case 1: case 2: if( gpAR->ubAdmins ) { gpAR->ubAdmins--; break; }
			case 3: case 4: if( gpAR->ubTroops ) { gpAR->ubTroops--; break; }
		}
	}
	while( gpAR->ubElites + gpAR->ubAdmins + gpAR->ubTroops < gpAR->ubEnemies )
	{
		switch( PreRandom( 5 ) )
		{
			case 0:				  gpAR->ubElites++; break;
			case 1: case 2: gpAR->ubAdmins++; break;
			case 3: case 4: gpAR->ubTroops++; break;
		}
	}


	//Do the same for the player mercs.
	while( gpAR->iNumMercFaces > gpAR->ubMercs && gpAR->iNumMercFaces > gpAR->iActualMercFaces )
	{ //Removing temp mercs
		gpAR->iNumMercFaces--;
		TacticalRemoveSoldier(*gpMercs[gpAR->iNumMercFaces].pSoldier);
		gpMercs[gpAR->iNumMercFaces].pSoldier = NULL;
	}
	while( gpAR->iNumMercFaces < gpAR->ubMercs && gpAR->iNumMercFaces >= gpAR->iActualMercFaces )
	{
		CreateTempPlayerMerc();
	}

	if( gpAR->uiTimeSlice == 0xffffffff )
	{
		gpAR->fSound = TRUE;
	}
	gpAR->uiTimeSlice = 1000;
	gpAR->uiTotalElapsedBattleTimeInMilliseconds	 = 0;
	gpAR->uiCurrTime = 0;
	gpAR->fPlayerRejectedSurrenderOffer = FALSE;
	gpAR->fPendingSurrender = FALSE;
	CalculateRowsAndColumns();
	CalculateSoldierCells( TRUE);
	CreateAutoResolveInterface();
	DetermineTeamLeader( TRUE ); //friendly team
	DetermineTeamLeader( FALSE ); //enemy team
	CalculateAttackValues();
}


static void CalculateRowsAndColumns(void)
{
	//now that we have the number on each team, calculate the number of rows and columns to be used on
	//the player's sides.  NOTE:  Militia won't appear on the same row as mercs.
	if( !gpAR->ubMercs )
	{ //0
		gpAR->ubMercCols = gpAR->ubMercRows = 0;
	}
	else if( gpAR->ubMercs < 5 )
	{ //1-4
		gpAR->ubMercCols = 1;
		gpAR->ubMercRows = gpAR->ubMercs;
	}
	else if( gpAR->ubMercs < 9 || gpAR->ubMercs == 10 )
	{ //5-8, 10
		gpAR->ubMercCols = 2;
		gpAR->ubMercRows = (gpAR->ubMercs+1)/2;
	}
	else if( gpAR->ubMercs < 16 )
	{ //9, 11-15
		gpAR->ubMercCols = 3;
		gpAR->ubMercRows = (gpAR->ubMercs+2)/3;
	}
	else
	{ //16-MAX_STRATEGIC_TEAM_SIZE
		gpAR->ubMercCols = 4;
		gpAR->ubMercRows = (gpAR->ubMercs+3)/4;
	}

	if( !gpAR->ubCivs )
	{
		gpAR->ubCivCols = gpAR->ubCivRows = 0;
	}
	else if( gpAR->ubCivs < 5 )
	{ //1-4
		gpAR->ubCivCols = 1;
		gpAR->ubCivRows = gpAR->ubCivs;
	}
	else if( gpAR->ubCivs < 9 || gpAR->ubCivs == 10 )
	{ //5-8, 10
		gpAR->ubCivCols = 2;
		gpAR->ubCivRows = (gpAR->ubCivs+1)/2;
	}
	else if( gpAR->ubCivs < 16 )
	{ //9, 11-15
		gpAR->ubCivCols = 3;
		gpAR->ubCivRows = (gpAR->ubCivs+2)/3;
	}
	else
	{ //16-MAX_ALLOWABLE_MILITIA_PER_SECTOR
		gpAR->ubCivCols = 4;
		gpAR->ubCivRows = (gpAR->ubCivs+3)/4;
	}

	if( !gpAR->ubEnemies )
	{
		gpAR->ubEnemyCols = gpAR->ubEnemyRows = 0;
	}
	else if( gpAR->ubEnemies < 5 )
	{ //1-4
		gpAR->ubEnemyCols = 1;
		gpAR->ubEnemyRows = gpAR->ubEnemies;
	}
	else if( gpAR->ubEnemies < 9 || gpAR->ubEnemies == 10 )
	{ //5-8, 10
		gpAR->ubEnemyCols = 2;
		gpAR->ubEnemyRows = (gpAR->ubEnemies+1)/2;
	}
	else if( gpAR->ubEnemies < 16 )
	{ //9, 11-15
		gpAR->ubEnemyCols = 3;
		gpAR->ubEnemyRows = (gpAR->ubEnemies+2)/3;
	}
	else
	{ //16-32
		gpAR->ubEnemyCols = 4;
		gpAR->ubEnemyRows = (gpAR->ubEnemies+3)/4;
	}


	//Now, that we have the number of mercs, militia, and enemies, it is possible that there
	//may be some conflicts.  Our goal is to make the window as small as possible.  Bumping up
	//the number of columns to 5 or rows to 10 will force one or both axes to go full screen.  If we
	//have high numbers of both, then we will have to.

	//Step one:  equalize the number of columns for both the mercs and civs.
	if( gpAR->ubMercs && gpAR->ubCivs && gpAR->ubMercCols != gpAR->ubCivCols )
	{
		if( gpAR->ubMercCols < gpAR->ubCivCols )
		{
			gpAR->ubMercCols = gpAR->ubCivCols;
			gpAR->ubMercRows = (gpAR->ubMercs+gpAR->ubMercCols-1)/gpAR->ubMercCols;
		}
		else
		{
			gpAR->ubCivCols = gpAR->ubMercCols;
			gpAR->ubCivRows = (gpAR->ubCivs+gpAR->ubCivCols-1)/gpAR->ubCivCols;
		}
	}
	//If we have both mercs and militia, we must make sure that the height to width ratio is never higher than
	//a factor of two.
	if( gpAR->ubMercs && gpAR->ubCivs && gpAR->ubMercRows + gpAR->ubCivRows > 4 )
	{
		if( gpAR->ubMercCols * 2 < gpAR->ubMercRows + gpAR->ubCivRows )
		{
			gpAR->ubMercCols++;
			gpAR->ubMercRows = (gpAR->ubMercs+gpAR->ubMercCols-1)/gpAR->ubMercCols;
			gpAR->ubCivCols++;
			gpAR->ubCivRows = (gpAR->ubCivs+gpAR->ubCivCols-1)/gpAR->ubCivCols;
		}
	}


	if( gpAR->ubMercRows + gpAR->ubCivRows > 9 )
	{
		if( gpAR->ubMercCols < 5 )
		{ //bump it up
			gpAR->ubMercCols++;
			gpAR->ubMercRows = (gpAR->ubMercs+gpAR->ubMercCols-1)/gpAR->ubMercCols;
		}
		if( gpAR->ubCivCols < 5 )
		{ //match it up with the mercs
			gpAR->ubCivCols = gpAR->ubMercCols;
			gpAR->ubCivRows = (gpAR->ubCivs+gpAR->ubCivCols-1)/gpAR->ubCivCols;
		}
	}

	if( gpAR->ubMercCols + gpAR->ubEnemyCols == 9 )
		gpAR->rect.w = SCREEN_WIDTH;
	else
		gpAR->rect.w = 146 + 55 * (MAX(MAX(gpAR->ubMercCols, gpAR->ubCivCols), 2) + MAX(gpAR->ubEnemyCols, 2));

	gpAR->sCenterStartX = 323 - gpAR->rect.w / 2 + MAX(MAX(gpAR->ubMercCols, 2), MAX(gpAR->ubCivCols, 2)) * 55;

	//Anywhere from 48*3 to 48*10
	gpAR->rect.h = 48 * MAX(3, MAX(gpAR->ubMercRows + gpAR->ubCivRows, gpAR->ubEnemyRows));
	//Make it an even multiple of 40 (rounding up).
	gpAR->rect.h += 39;
	gpAR->rect.h /= 40;
	gpAR->rect.h *= 40;

	//Here is a extremely bitchy case.  The formulae throughout this module work for most cases.
	//However, when combining mercs and civs, the column must be the same.  However, we retract that
	//in cases where there are less mercs than available to fill up *most* of the designated space.
	if( gpAR->ubMercs && gpAR->ubCivs )
	{
		if( gpAR->ubMercRows * gpAR->ubMercCols > gpAR->ubMercs + gpAR->ubMercRows )
			gpAR->ubMercCols--;
		if( gpAR->ubCivRows * gpAR->ubCivCols > gpAR->ubCivs + gpAR->ubCivRows )
			gpAR->ubCivCols--;
	}
}


static void HandleAutoResolveInput(void)
{
	InputAtom InputEvent;
	BOOLEAN fResetAutoResolve = FALSE;
	while( DequeueEvent( &InputEvent ) )
	{
		if( InputEvent.usEvent == KEY_DOWN || InputEvent.usEvent == KEY_REPEAT )
		{
			switch( InputEvent.usParam )
			{
				case SDLK_SPACE:
					DepressAutoButton(gpAR->fPaused ? PLAY_BUTTON : PAUSE_BUTTON);
					break;

				case 'x':
					if( _KeyDown( ALT ) )
					{
						HandleShortCutExitState( );
					}
					break;
				#ifdef JA2BETAVERSION
				case 'c':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->fAllowCapture ^= TRUE;
						gpAR->fPlayerRejectedSurrenderOffer = FALSE;
						gStrategicStatus.uiFlags &= ~STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;
						gpAR->fRenderAutoResolve = TRUE;
					}
					break;

				case SDLK_F5: if (CHEATER_CHEAT_LEVEL()) gpAR->fDebugInfo     ^= TRUE; break;
				case SDLK_F6: if (CHEATER_CHEAT_LEVEL()) gpAR->fSound         ^= TRUE; break;
				case SDLK_F7: if (CHEATER_CHEAT_LEVEL()) gpAR->fInstantFinish ^= TRUE; break;

				case SDLK_BACKSPACE: if (CHEATER_CHEAT_LEVEL()) fResetAutoResolve = TRUE; break;

				case 'd':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( _KeyDown( ALT ) )
						{
							if ( gpAR->ubBattleStatus == BATTLE_IN_PROGRESS )
							{
								PlayAutoResolveSample(EXPLOSION_1, HIGHVOLUME, 1, MIDDLEPAN);
								EliminateAllFriendlies();
							}
						}
					}
					break;
				case 'z':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( _KeyDown( ALT ) )
						{
							if ( gpAR->ubBattleStatus == BATTLE_IN_PROGRESS )
							{
								PlayAutoResolveSample(EXPLOSION_1, HIGHVOLUME, 1, MIDDLEPAN);
								EliminateAllMercs();
							}
						}
					}
					break;
				case 'o':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( _KeyDown( ALT ) )
						{
							if ( gpAR->ubBattleStatus == BATTLE_IN_PROGRESS )
							{
								PlayAutoResolveSample(EXPLOSION_1, HIGHVOLUME, 1, MIDDLEPAN);
								// this is not very accurate, any enemies already dead will be counted as killed twice
								gStrategicStatus.usPlayerKills += NumEnemiesInSector( gpAR->ubSectorX, gpAR->ubSectorY );
								EliminateAllEnemies( gpAR->ubSectorX, gpAR->ubSectorY );
							}
						}
					}
					break;
				case '{':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubMercs = 0;
						fResetAutoResolve = TRUE;
					}
					break;
				case '}':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubMercs = MAX_STRATEGIC_TEAM_SIZE;
						fResetAutoResolve = TRUE;
					}
					break;
				case '[':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( gpAR->ubMercs )
						{
							gpAR->ubMercs--;
							fResetAutoResolve = TRUE;
						}
					}
					break;
				case ']':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( gpAR->ubMercs < MAX_STRATEGIC_TEAM_SIZE )
						{
							gpAR->ubMercs++;
							fResetAutoResolve = TRUE;
						}
					}
					break;
				case ':':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubCivs = 0;
						fResetAutoResolve = TRUE;
					}
					break;
				case '"':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubCivs = MAX_ALLOWABLE_MILITIA_PER_SECTOR;
						fResetAutoResolve = TRUE;
					}
					break;
				case ';':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( gpAR->ubCivs )
						{
							gpAR->ubCivs--;
							fResetAutoResolve = TRUE;
						}
					}
					break;
				case 39: // ' quote
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( gpAR->ubCivs < MAX_ALLOWABLE_MILITIA_PER_SECTOR )
						{
							gpAR->ubCivs++;
							fResetAutoResolve = TRUE;
						}
					}
					break;
				case '<':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubEnemies = 1;
						fResetAutoResolve = TRUE;
					}
					break;
				case '>':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubEnemies = 32;
						fResetAutoResolve = TRUE;
					}
					break;
				case ',':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( gpAR->ubEnemies > 1 )
						{
							gpAR->ubEnemies--;
							fResetAutoResolve = TRUE;
						}
					}
					break;
				case '.':
					if( CHEATER_CHEAT_LEVEL() )
					{
						if( gpAR->ubEnemies < 32 )
						{
							gpAR->ubEnemies++;
							fResetAutoResolve = TRUE;
						}
					}
					break;
				case '/':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubMercs = 1;
						gpAR->ubCivs = 0;
						gpAR->ubEnemies = 1;
						fResetAutoResolve = TRUE;
					}
					break;
				case '?':
					if( CHEATER_CHEAT_LEVEL() )
					{
						gpAR->ubMercs = 20;
						gpAR->ubCivs = MAX_ALLOWABLE_MILITIA_PER_SECTOR;
						gpAR->ubEnemies = 32;
						fResetAutoResolve = TRUE;
					}
					break;
				#endif
			}
		}
	}
	if( fResetAutoResolve )
	{
		ResetAutoResolveInterface();
	}
}


static void RenderSoldierCellHealth(SOLDIERCELL* pCell)
{
	INT32 cnt, cntStart;
	INT32 xp, yp;
	const wchar_t *pStr;
	wchar_t str[20];
	UINT16 usColor;

	SetFont( SMALLCOMPFONT );
	//Restore the background before drawing text.
	xp = pCell->xp +  2;
	yp = pCell->yp + 32;
	SGPBox const r = { xp - gpAR->rect.x, yp - gpAR->rect.y, 46, 10 };
	BltVideoSurface(FRAME_BUFFER, gpAR->iInterfaceBuffer, xp, yp, &r);

	if( pCell->pSoldier->bLife )
	{
		if( pCell->pSoldier->bLife == pCell->pSoldier->bLifeMax )
		{
			cntStart = 4;
		}
		else
		{
			cntStart = 0;
		}
		for( cnt = cntStart; cnt < 6; cnt ++ )
		{
			if( pCell->pSoldier->bLife < bHealthStrRanges[ cnt ] )
			{
				break;
			}
		}
		switch( cnt )
		{
			case 0: //DYING
			case 1: //CRITICAL
				usColor = FONT_RED;
				break;
			case 2: //WOUNDED
			case 3: //POOR
				usColor = FONT_YELLOW;
				break;
			default: //REST
				usColor = FONT_GRAY1;
				break;
		}
		if( cnt > 3 && pCell->pSoldier->bLife != pCell->pSoldier->bLifeMax )
		{ //Merc has taken damage, even though his life if good.
			usColor = FONT_YELLOW;
		}
		if( pCell->pSoldier->bLife == pCell->pSoldier->bLifeMax )
			usColor = FONT_GRAY1;
		pStr = zHealthStr[ cnt ];
	}
	else
	{
		wcscpy( str, pCell->pSoldier->name );
		#if 0 /* XXX */
		pStr = _wcsupr( str );
		#else
		pStr = str;
		#endif
		usColor = FONT_BLACK;
	}

	//Draw the retreating text, if applicable
	if( pCell->uiFlags & CELL_RETREATED && gpAR->ubBattleStatus != BATTLE_VICTORY )
	{
		usColor = FONT_LTGREEN;
		pStr = gpStrategicString[STR_AR_MERC_RETREATED];
	}
	else if( pCell->uiFlags & CELL_RETREATING && gpAR->ubBattleStatus == BATTLE_IN_PROGRESS )
	{
		if( pCell->pSoldier->bLife >= OKLIFE )
		{ //Retreating is shared with the status string.  Alternate between the
			//two every 450 milliseconds
			if( GetJA2Clock() % 900 < 450 )
			{ //override the health string with the retreating string.
				usColor = FONT_LTRED;
				pStr = gpStrategicString[STR_AR_MERC_RETREATING];
			}
		}
	}
	else if( pCell->uiFlags & CELL_SHOWRETREATTEXT && gpAR->ubBattleStatus == BATTLE_IN_PROGRESS )
	{
		if( pCell->pSoldier->bLife >= OKLIFE )
		{
			SetFontForeground( FONT_YELLOW );
			const wchar_t* Retreat = gpStrategicString[STR_AR_MERC_RETREAT];
			xp = pCell->xp + 25 - StringPixLength(Retreat, SMALLCOMPFONT) / 2;
			yp = pCell->yp + 12;
			MPrint(xp, yp, Retreat);
		}
	}
	SetFontForeground( (UINT8)usColor );
	xp = pCell->xp + 25 - StringPixLength( pStr, SMALLCOMPFONT ) / 2;
	yp = pCell->yp + 33;
	MPrint(xp, yp, pStr);
}


static UINT8 GetUnusedMercProfileID(void)
{
	for (;;)
	{
		const ProfileID pid = PreRandom(40);
		if (FindSoldierByProfileIDOnPlayerTeam(pid) == NULL) return pid;
	}
}


static void CreateTempPlayerMerc(void)
{
	SOLDIERCREATE_STRUCT		MercCreateStruct;

	//Init the merc create structure with basic information
	memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
	MercCreateStruct.bTeam									= OUR_TEAM;
	MercCreateStruct.ubProfile							= GetUnusedMercProfileID();
	MercCreateStruct.sSectorX								= gpAR->ubSectorX;
	MercCreateStruct.sSectorY								= gpAR->ubSectorY;
	MercCreateStruct.bSectorZ								= 0;
	MercCreateStruct.fCopyProfileItemsOver	= TRUE;

	//Create the player soldier
	gpMercs[gpAR->iNumMercFaces].pSoldier = TacticalCreateSoldier(MercCreateStruct);
	if( gpMercs[ gpAR->iNumMercFaces ].pSoldier )
	{
		gpAR->iNumMercFaces++;
	}
}


static void DetermineTeamLeader(BOOLEAN fFriendlyTeam)
{
	SOLDIERCELL *pBestLeaderCell = NULL;
	//For each team (civs and players count as same team), find the merc with the best
	//leadership ability.
	if( fFriendlyTeam )
	{
		gpAR->ubPlayerLeadership = 0;
		FOR_ALL_AR_MERCS(i)
		{
			if (gpAR->ubPlayerLeadership >= i->pSoldier->bLeadership) continue;
			gpAR->ubPlayerLeadership = i->pSoldier->bLeadership;
			pBestLeaderCell          = i;
		}
		FOR_ALL_AR_CIVS(i)
		{
			if (gpAR->ubPlayerLeadership >= i->pSoldier->bLeadership) continue;
			gpAR->ubPlayerLeadership = i->pSoldier->bLeadership;
			pBestLeaderCell          = i;
		}

		if( pBestLeaderCell )
		{
			//Assign the best leader the honour of team leader.
			pBestLeaderCell->uiFlags |= CELL_TEAMLEADER;
		}
		return;
	}
	//ENEMY TEAM
	gpAR->ubEnemyLeadership = 0;
	FOR_ALL_AR_ENEMIES(i)
	{
		if (gpAR->ubEnemyLeadership >= i->pSoldier->bLeadership) continue;
		gpAR->ubEnemyLeadership = i->pSoldier->bLeadership;
		pBestLeaderCell         = i;
	}
	if( pBestLeaderCell )
	{
		//Assign the best enemy leader the honour of team leader
		pBestLeaderCell->uiFlags |= CELL_TEAMLEADER;
	}
}


static void ResetNextAttackCounter(SOLDIERCELL* pCell)
{
	pCell->usNextAttack = MIN( 1000 - pCell->usAttack, 800 );
	pCell->usNextAttack = (UINT16)(1000 + pCell->usNextAttack * 5 + PreRandom( 2000 - pCell->usAttack ) );
	if( pCell->uiFlags & CELL_CREATURE )
	{
		pCell->usNextAttack = pCell->usNextAttack * 8 / 10;
	}
}


static void CalculateAttackValues(void)
{
	SOLDIERCELL *pCell;
	SOLDIERTYPE *pSoldier;
	UINT16 usBonus;
	UINT16 usBestAttack = 0xffff;
	UINT16 usBreathStrengthPercentage;
	//INT16 sOutnumberBonus = 0;
	//PLAYER TEAM
	gpAR->usPlayerAttack = 0;
	gpAR->usPlayerDefence = 0;

	//if( gpAR->ubEnemies )
	//{
	//	//bonus equals 20 if good guys outnumber bad guys 2 to 1.
	//	const INT16 sMaxBonus = 20;
	//	sOutnumberBonus = (INT16)(gpAR->ubMercs + gpAR->ubCivs) * sMaxBonus / gpAR->ubEnemies - sMaxBonus;
	//	sOutnumberBonus = (INT16)MIN( sOutnumberBonus, MAX( sMaxBonus, 0 ) );
	//}

	for (INT32 i = 0; i < gpAR->ubMercs; i++)
	{
		pCell = &gpMercs[ i ];
		pSoldier = pCell->pSoldier;
		if( !pSoldier->bLife )
			continue;
		pCell->usAttack =		pSoldier->bStrength +
												pSoldier->bDexterity +
												pSoldier->bWisdom +
												pSoldier->bMarksmanship +
												pSoldier->bMorale;
		//Give player controlled mercs a significant bonus to compensate for lack of control
		//as the player would typically do much better in tactical.
		if( pCell->usAttack < 1000 )
		{ //A player with 500 attack will be augmented to 625
			//A player with 600 attack will be augmented to 700
			pCell->usAttack = (UINT16)(pCell->usAttack + (1000 - pCell->usAttack) / 4);
		}
		usBreathStrengthPercentage = 100 - ( 100 - pCell->pSoldier->bBreathMax ) / 3;
		pCell->usAttack =		pCell->usAttack * usBreathStrengthPercentage / 100;
		pCell->usDefence =	pSoldier->bAgility +
												pSoldier->bWisdom +
												pSoldier->bBreathMax +
												pSoldier->bMedical +
												pSoldier->bMorale;
		//100 team leadership adds a bonus of 10%,
		usBonus = 100 + gpAR->ubPlayerLeadership/10;// + sOutnumberBonus;

		//bExpLevel adds a bonus of 7% per level after 2, level 1 soldiers get a 7% decrease
		//usBonus += 7 * (pSoldier->bExpLevel-2);
		usBonus += gpAR->ubPlayerDefenceAdvantage;
		pCell->usAttack = pCell->usAttack * usBonus / 100;
		pCell->usDefence = pCell->usDefence * usBonus / 100;

		if( pCell->uiFlags & CELL_EPC )
		{ //strengthen the defense (seeing the mercs will keep them safe).
			pCell->usAttack = 0;
			pCell->usDefence = 1000;
		}

		pCell->usAttack = MIN( pCell->usAttack, 1000 );
		pCell->usDefence = MIN( pCell->usDefence, 1000 );

		gpAR->usPlayerAttack += pCell->usAttack;
		gpAR->usPlayerDefence += pCell->usDefence;
		ResetNextAttackCounter( pCell );
		if( i > 8 )
		{ //Too many mercs, delay attack entry of extra mercs.
			pCell->usNextAttack += (UINT16)( ( i - 8 ) * 2000 );
		}
		if( pCell->usNextAttack < usBestAttack )
			usBestAttack = pCell->usNextAttack;
	}
	//CIVS
	for (INT32 i = 0; i < gpAR->ubCivs; i++)
	{
		pCell = &gpCivs[ i ];
		pSoldier = pCell->pSoldier;
		pCell->usAttack =		pSoldier->bStrength +
												pSoldier->bDexterity +
												pSoldier->bWisdom +
												pSoldier->bMarksmanship +
												pSoldier->bMorale;
		pCell->usAttack =		pCell->usAttack * pSoldier->bBreath / 100;
		pCell->usDefence =	pSoldier->bAgility +
												pSoldier->bWisdom +
												pSoldier->bBreathMax +
												pSoldier->bMedical +
												pSoldier->bMorale;
		//100 team leadership adds a bonus of 10%
		usBonus = 100 + gpAR->ubPlayerLeadership/10;// + sOutnumberBonus;
		//bExpLevel adds a bonus of 7% per level after 2, level 1 soldiers get a 7% decrease
		//usBonus += 7 * (pSoldier->bExpLevel-2);
		usBonus += gpAR->ubPlayerDefenceAdvantage;
		pCell->usAttack = pCell->usAttack * usBonus / 100;
		pCell->usDefence = pCell->usDefence * usBonus / 100;

		pCell->usAttack = MIN( pCell->usAttack, 1000 );
		pCell->usDefence = MIN( pCell->usDefence, 1000 );

		gpAR->usPlayerAttack += pCell->usAttack;
		gpAR->usPlayerDefence += pCell->usDefence;
		ResetNextAttackCounter( pCell );
		if( i > 6 )
		{ //Too many militia, delay attack entry of extra mercs.
			pCell->usNextAttack += (UINT16)(( i - 4 ) * 2000 );
		}
		if( pCell->usNextAttack < usBestAttack )
			usBestAttack = pCell->usNextAttack;
	}
	//ENEMIES
	gpAR->usEnemyAttack = 0;
	gpAR->usEnemyDefence = 0;
	//if( gpAR->ubMercs + gpAR->ubCivs )
	//{
	//	//bonus equals 20 if good guys outnumber bad guys 2 to 1.
	//	const INT16 sMaxBonus = 20;
	//	sOutnumberBonus = (INT16)gpAR->ubEnemies * sMaxBonus / (gpAR->ubMercs + gpAR->ubCivs) - sMaxBonus;
	//	sOutnumberBonus = (INT16)MIN( sOutnumberBonus, MAX( sMaxBonus, 0 ) );
	//}

	for (INT32 i = 0; i < gpAR->ubEnemies; i++)
	{
		pCell = &gpEnemies[ i ];
		pSoldier = pCell->pSoldier;
		pCell->usAttack =		pSoldier->bStrength +
												pSoldier->bDexterity +
												pSoldier->bWisdom +
												pSoldier->bMarksmanship +
												pSoldier->bMorale;
		pCell->usAttack =		pCell->usAttack * pSoldier->bBreath / 100;
		pCell->usDefence =	pSoldier->bAgility +
												pSoldier->bWisdom +
												pSoldier->bBreathMax +
												pSoldier->bMedical +
												pSoldier->bMorale;
		//100 team leadership adds a bonus of 10%
		usBonus = 100 + gpAR->ubPlayerLeadership/10;// + sOutnumberBonus;
		//bExpLevel adds a bonus of 7% per level after 2, level 1 soldiers get a 7% decrease
		//usBonus += 7 * (pSoldier->bExpLevel-2);
		usBonus += gpAR->ubEnemyDefenceAdvantage;
		pCell->usAttack = pCell->usAttack * usBonus / 100;
		pCell->usDefence = pCell->usDefence * usBonus / 100;

		pCell->usAttack = MIN( pCell->usAttack, 1000 );
		pCell->usDefence = MIN( pCell->usDefence, 1000 );

		gpAR->usEnemyAttack += pCell->usAttack;
		gpAR->usEnemyDefence += pCell->usDefence;
		ResetNextAttackCounter( pCell );

		if( i > 4 && !(pCell->uiFlags & CELL_CREATURE) )
		{ //Too many enemies, delay attack entry of extra mercs.
			pCell->usNextAttack += (UINT16)( ( i - 4 ) * 1000 );
		}

		if( pCell->usNextAttack < usBestAttack )
			usBestAttack = pCell->usNextAttack;
	}
	//Now, because we are starting a new battle, we want to get the ball rolling a bit earlier.  So,
	//we will take the usBestAttack value and subtract 60% of it from everybody's next attack.
	usBestAttack = usBestAttack * 60 / 100;
	FOR_ALL_AR_MERCS(i)
	{
		i->usNextAttack -= usBestAttack;
	}
	FOR_ALL_AR_CIVS(i)
	{
		i->usNextAttack -= usBestAttack;
	}
	FOR_ALL_AR_ENEMIES(i)
	{
		i->usNextAttack -= usBestAttack;
	}
}


static void DrawDebugText(SOLDIERCELL* pCell)
{
	INT32 xp, yp;
	if( !gpAR->fDebugInfo )
		return;
	SetFont( SMALLCOMPFONT );
	SetFontForeground( FONT_WHITE );
	xp = pCell->xp + 4;
	yp = pCell->yp + 4;
	if( pCell->uiFlags & CELL_TEAMLEADER )
	{
		//debug str
		MPrint(xp, yp, L"LEADER");
		yp += 9;
	}
	mprintf( xp, yp, L"AT: %d", pCell->usAttack );
	yp += 9;
	mprintf( xp, yp, L"DF: %d", pCell->usDefence );
	yp += 9;

	xp = pCell->xp;
	yp = pCell->yp - 4;
	SetFont( LARGEFONT1 );
	SetFontShadow( FONT_NEARBLACK );
	if( pCell->uiFlags & CELL_FIREDATTARGET )
	{
		SetFontForeground( FONT_YELLOW );
		MPrint(xp, yp, L"FIRE");
		pCell->uiFlags &= ~CELL_FIREDATTARGET;
		yp += 13;
	}
	if( pCell->uiFlags & CELL_DODGEDATTACK )
	{
		SetFontForeground( FONT_BLUE );
		MPrint(xp, yp, L"MISS");
		pCell->uiFlags &= ~CELL_DODGEDATTACK;
		yp += 13;
	}
	if( pCell->uiFlags & CELL_HITBYATTACKER )
	{
		SetFontForeground( FONT_RED );
		MPrint(xp, yp, L"HIT");
		pCell->uiFlags &= ~CELL_HITBYATTACKER;
		yp += 13;
	}
}


static BOOLEAN IsBattleOver(void);


static SOLDIERCELL* ChooseTarget(SOLDIERCELL* pAttacker)
{
	INT32 iAvailableTargets;
	INT32 index;
	INT32 iRandom = -1;
	SOLDIERCELL *pTarget = NULL;
	UINT16 usSavedDefence;
	//Determine what team we are attacking
	if( pAttacker->uiFlags & (CELL_ENEMY | CELL_CREATURE) )
	{ //enemy team attacking a player
		iAvailableTargets = gpAR->ubMercs + gpAR->ubCivs;
		index = 0;
		usSavedDefence = gpAR->usPlayerDefence;
		while( iAvailableTargets )
		{
			pTarget = ( index < gpAR->ubMercs ) ? &gpMercs[ index ] : &gpCivs[ index - gpAR->ubMercs ];
			if( !pTarget->pSoldier->bLife || pTarget->uiFlags & CELL_RETREATED )
			{
				index++;
				iAvailableTargets--;
				continue;
			}
			iRandom = PreRandom( gpAR->usPlayerDefence );
			gpAR->usPlayerDefence -= pTarget->usDefence;
			if( iRandom < pTarget->usDefence )
			{
				gpAR->usPlayerDefence = usSavedDefence;
				return pTarget;
			}
			index++;
			iAvailableTargets--;
		}
		if( !IsBattleOver() )
		{
			AssertMsg( 0, String("***Please send PRIOR save and screenshot of this message***  iAvailableTargets %d, index %d, iRandom %d, defence %d. ",
				iAvailableTargets, index, iRandom, gpAR->usPlayerDefence) );
		}
	}
	else
	{ //player team attacking an enemy
		iAvailableTargets = gpAR->ubEnemies;
		index = 0;
		usSavedDefence = gpAR->usEnemyDefence;
		while( iAvailableTargets )
		{
			pTarget = &gpEnemies[ index ];
			if( !pTarget->pSoldier->bLife )
			{
				index++;
				iAvailableTargets--;
				continue;
			}
			iRandom = PreRandom( gpAR->usEnemyDefence );
			gpAR->usEnemyDefence -= pTarget->usDefence;
			if( iRandom < pTarget->usDefence )
			{
				gpAR->usEnemyDefence = usSavedDefence;
				return pTarget;
			}
			index++;
			iAvailableTargets--;
		}
	}
	AssertMsg( 0, "Error in ChooseTarget logic for choosing enemy target." );
	return NULL;
}


static BOOLEAN FireAShot(SOLDIERCELL* pAttacker)
{
	OBJECTTYPE *pItem;
	SOLDIERTYPE *pSoldier;
	INT32 i;

	pSoldier = pAttacker->pSoldier;

	if( pAttacker->uiFlags & CELL_MALECREATURE )
	{
		PlayAutoResolveSample(ACR_SPIT, 50, 1, MIDDLEPAN);
		pAttacker->bWeaponSlot = SECONDHANDPOS;
		return TRUE;
	}
	for( i = 0; i < NUM_INV_SLOTS; i++ )
	{
		pItem = &pSoldier->inv[ i ];

		if( Item[ pItem->usItem ].usItemClass == IC_GUN )
		{
			pAttacker->bWeaponSlot = (INT8)i;
			if( gpAR->fUnlimitedAmmo )
			{
				PlayAutoResolveSample(Weapon[pItem->usItem].sSound, 50, 1, MIDDLEPAN);
				return TRUE;
			}
			if( !pItem->ubGunShotsLeft )
			{
				AutoReload( pSoldier );
				if (pItem->ubGunShotsLeft && Weapon[pItem->usItem].sLocknLoadSound != NO_SOUND)
				{
					PlayAutoResolveSample(Weapon[pItem->usItem].sLocknLoadSound, 50, 1, MIDDLEPAN);
				}
			}
			if( pItem->ubGunShotsLeft )
			{
				PlayAutoResolveSample(Weapon[pItem->usItem].sSound, 50, 1, MIDDLEPAN);
				if( pAttacker->uiFlags & CELL_MERC )
				{
					gMercProfiles[ pAttacker->pSoldier->ubProfile ].usShotsFired++;
					// MARKSMANSHIP GAIN: Attacker fires a shot
					StatChange(*pAttacker->pSoldier, MARKAMT, 3, FROM_SUCCESS);
				}
				pItem->ubGunShotsLeft--;
				return TRUE;
			}
		}
	}
	pAttacker->bWeaponSlot = -1;
	return FALSE;
}


static BOOLEAN AttackerHasKnife(SOLDIERCELL* const attacker)
{
	INT8 const slot = FindObjClass(attacker->pSoldier, IC_BLADE);
	attacker->bWeaponSlot = slot;
	return slot != NO_SLOT;
}


static BOOLEAN TargetHasLoadedGun(SOLDIERTYPE* pSoldier)
{
	CFOR_ALL_SOLDIER_INV_SLOTS(pItem, *pSoldier)
	{
		if( Item[ pItem->usItem ].usItemClass == IC_GUN )
		{
			if( gpAR->fUnlimitedAmmo )
			{
				return TRUE;
			}
			if( pItem->ubGunShotsLeft )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


static void AttackTarget(SOLDIERCELL* pAttacker, SOLDIERCELL* pTarget)
{
	UINT16 usAttack;
	UINT16 usDefence;
	UINT8 ubImpact;
	UINT8 ubLocation;
	UINT8 ubAccuracy;
	INT32 iRandom;
	INT32 iImpact;
	INT32 iNewLife;
	BOOLEAN fMelee = FALSE;
	BOOLEAN fKnife = FALSE;
	BOOLEAN fClaw = FALSE;
	INT8	bAttackIndex = -1;

	pAttacker->uiFlags |= CELL_FIREDATTARGET | CELL_DIRTY;
	if( pAttacker->usAttack < 950 )
		usAttack = (UINT16)(pAttacker->usAttack + PreRandom(1000 - pAttacker->usAttack ));
	else
		usAttack = (UINT16)(950 + PreRandom( 50 ));
	if( pTarget->uiFlags & CELL_RETREATING && !(pAttacker->uiFlags & CELL_FEMALECREATURE) )
	{ //Attacking a retreating merc is harder.  Modify the attack value to 70% of it's value.
		//This allows retreaters to have a better chance of escaping.
		usAttack = usAttack * 7 / 10;
	}
	if( pTarget->usDefence < 950 )
		usDefence = (UINT16)(pTarget->usDefence + PreRandom(1000 - pTarget->usDefence ));
	else
		usDefence = (UINT16)(950 + PreRandom( 50 ));
	if( pAttacker->uiFlags & CELL_FEMALECREATURE )
	{
		pAttacker->bWeaponSlot = HANDPOS;
		fMelee = TRUE;
		fClaw = TRUE;
	}
	else if( !FireAShot( pAttacker ) )
	{ //Maybe look for a weapon, such as a knife or grenade?
		fMelee = TRUE;
		fKnife = AttackerHasKnife( pAttacker );
		if( TargetHasLoadedGun( pTarget->pSoldier ) )
		{ //Penalty to attack with melee weapons against target with loaded gun.
			if( !(pAttacker->uiFlags & CELL_CREATURE ) )
			{ //except for creatures
				if( fKnife )
					usAttack = usAttack * 6 / 10;
				else
					usAttack = usAttack * 4 / 10;
			}
		}
	}
	//Set up a random delay for the hit or miss.
	if( !fMelee )
	{
		if( !pTarget->usNextHit[0] )
		{
			bAttackIndex = 0;
		}
		else if( !pTarget->usNextHit[1] )
		{
			bAttackIndex = 1;
		}
		else if( !pTarget->usNextHit[2] )
		{
			bAttackIndex = 2;
		}
		if ( bAttackIndex != -1 )
		{
			pTarget->usNextHit[ bAttackIndex ] = (UINT16)( 50 + PreRandom( 400 ) );
			pTarget->pAttacker[ bAttackIndex ] = pAttacker;
		}
	}
	if( usAttack < usDefence )
	{
		if( pTarget->pSoldier->bLife >= OKLIFE || !PreRandom( 5 ) )
		{	//Attacker misses -- use up a round of ammo.  If target is unconcious, then 80% chance of hitting.
			pTarget->uiFlags |= CELL_DODGEDATTACK | CELL_DIRTY;
			if( fMelee )
			{
				if( fKnife )
					PlayAutoResolveSample(MISS_KNIFE, 50, 1, MIDDLEPAN);
				else if( fClaw )
				{
					if( Chance( 50 ) )
					{
						PlayAutoResolveSample(ACR_SWIPE, 50, 1, MIDDLEPAN);
					}
					else
					{
						PlayAutoResolveSample(ACR_LUNGE, 50, 1, MIDDLEPAN);
					}
				}
				else
					PlayAutoResolveSample(SoundRange<SWOOSH_1, SWOOSH_6>(), 50, 1, MIDDLEPAN);
				if( pTarget->uiFlags & CELL_MERC )
					// AGILITY GAIN: Target "dodged" an attack
					StatChange(*pTarget->pSoldier, AGILAMT, 5, FROM_SUCCESS);
			}
			return;
		}
	}
	//Attacker hits
	if( !fMelee )
	{
		ubImpact = Weapon[ pAttacker->pSoldier->inv[ pAttacker->bWeaponSlot ].usItem ].ubImpact;
		iRandom = PreRandom( 100 );
		if( iRandom < 15 )
			ubLocation = AIM_SHOT_HEAD;
		else if( iRandom < 30 )
			ubLocation = AIM_SHOT_LEGS;
		else
			ubLocation = AIM_SHOT_TORSO;
		ubAccuracy = (UINT8)((usAttack - usDefence + PreRandom( usDefence - pTarget->usDefence ) )/10);
		iImpact = BulletImpact( pAttacker->pSoldier, pTarget->pSoldier, ubLocation, ubImpact, ubAccuracy, NULL );

		if ( bAttackIndex == -1 )
		{
			// tack damage on to end of last hit
			pTarget->usHitDamage[2] += (UINT16) iImpact;
		}
		else
		{
			pTarget->usHitDamage[ bAttackIndex ] = (UINT16) iImpact;
		}

	}
	else
	{
		OBJECTTYPE *pItem;
		PlayAutoResolveSample(SoundRange<BULLET_IMPACT_1, BULLET_IMPACT_3>(), 50, 1, MIDDLEPAN);
		if( !pTarget->pSoldier->bLife )
		{ //Soldier already dead (can't kill him again!)
			return;
		}

		ubAccuracy = (UINT8)((usAttack - usDefence + PreRandom( usDefence - pTarget->usDefence ) )/10);

		//Determine attacking weapon.
		pAttacker->pSoldier->usAttackingWeapon = 0;
		if( pAttacker->bWeaponSlot != -1 )
		{
			pItem = &pAttacker->pSoldier->inv[ pAttacker->bWeaponSlot ];
			if( Item[ pItem->usItem ].usItemClass & IC_WEAPON )
				pAttacker->pSoldier->usAttackingWeapon = pAttacker->pSoldier->inv[ pAttacker->bWeaponSlot ].usItem;
		}

		iImpact = HTHImpact(pAttacker->pSoldier, pTarget->pSoldier, ubAccuracy, fKnife | fClaw);

		iNewLife = pTarget->pSoldier->bLife - iImpact;

		if( pAttacker->uiFlags & CELL_MERC )
		{ //Attacker is a player, so increment the number of shots that hit.
			gMercProfiles[ pAttacker->pSoldier->ubProfile ].usShotsHit++;
			// MARKSMANSHIP GAIN: Attacker's shot hits
			StatChange(*pAttacker->pSoldier, MARKAMT, 6, FROM_SUCCESS); // in addition to 3 for taking a shot
		}
		if( pTarget->uiFlags & CELL_MERC )
		{ //Target is a player, so increment the times he has been wounded.
			gMercProfiles[ pTarget->pSoldier->ubProfile ].usTimesWounded++;
			// EXPERIENCE GAIN: Took some damage
			StatChange(*pTarget->pSoldier, EXPERAMT, 5 * (iImpact / 10), FROM_SUCCESS);
		}
		if( pTarget->pSoldier->bLife >= CONSCIOUSNESS || pTarget->uiFlags & CELL_CREATURE )
		{
			if( gpAR->fSound )
				DoMercBattleSound(pTarget->pSoldier, BATTLE_SOUND_HIT1);
		}
		if( !(pTarget->uiFlags & CELL_CREATURE) && iNewLife < OKLIFE && pTarget->pSoldier->bLife >= OKLIFE )
		{ //the hit caused the merc to fall.  Play the falling sound
			PlayAutoResolveSample(FALL_1, 50, 1, MIDDLEPAN);
			pTarget->uiFlags &= ~CELL_RETREATING;
		}
		if( iNewLife <= 0 )
		{ //soldier has been killed
			if( pAttacker->uiFlags & CELL_MERC )
			{ //Player killed the enemy soldier -- update his stats as well as any assisters.
				gMercProfiles[ pAttacker->pSoldier->ubProfile ].usKills++;
				gStrategicStatus.usPlayerKills++;
			}
			else if( pAttacker->uiFlags & CELL_MILITIA )
			{
				pAttacker->pSoldier->ubMilitiaKills += 2;
			}
			if( pTarget->uiFlags & CELL_MERC && gpAR->fSound )
			{
				PlayAutoResolveSample(DOORCR_1, HIGHVOLUME, 1, MIDDLEPAN);
				PlayAutoResolveSample(HEADCR_1, HIGHVOLUME, 1, MIDDLEPAN);
			}
		}
		//Adjust the soldiers stats based on the damage.
		pTarget->pSoldier->bLife = (INT8)MAX( iNewLife, 0 );
		if( pTarget->uiFlags & CELL_MERC && gpAR->pRobotCell)
		{
			UpdateRobotControllerGivenRobot( gpAR->pRobotCell->pSoldier );
		}
		if( fKnife || fClaw )
		{
			if( pTarget->pSoldier->bLifeMax - pTarget->pSoldier->bBleeding - iImpact >= pTarget->pSoldier->bLife )
				pTarget->pSoldier->bBleeding += (INT8)iImpact;
			else
				pTarget->pSoldier->bBleeding = (INT8)(pTarget->pSoldier->bLifeMax - pTarget->pSoldier->bLife);
		}
		if( !pTarget->pSoldier->bLife )
		{
			gpAR->fRenderAutoResolve = TRUE;
			#ifdef INVULNERABILITY
			if( 1 )
				RefreshMerc( pTarget->pSoldier );
			else
			#endif
			if( pTarget->uiFlags & CELL_MERC )
			{
				gpAR->usPlayerAttack -= pTarget->usAttack;
				gpAR->usPlayerDefence -= pTarget->usDefence;
				gpAR->ubAliveMercs--;
				pTarget->usAttack = 0;
				pTarget->usDefence = 0;
			}
			else if( pTarget->uiFlags & CELL_MILITIA )
			{
				gpAR->usPlayerAttack -= pTarget->usAttack;
				gpAR->usPlayerDefence -= pTarget->usDefence;
				gpAR->ubAliveCivs--;
				pTarget->usAttack = 0;
				pTarget->usDefence = 0;
			}
			else if( pTarget->uiFlags & (CELL_ENEMY|CELL_CREATURE) )
			{
				gpAR->usEnemyAttack -= pTarget->usAttack;
				gpAR->usEnemyDefence -= pTarget->usDefence;
				gpAR->ubAliveEnemies--;
				pTarget->usAttack = 0;
				pTarget->usDefence = 0;
			}
		}
		pTarget->uiFlags |= CELL_HITBYATTACKER | CELL_DIRTY;
	}
}


static void TargetHitCallback(SOLDIERCELL* pTarget, INT32 index)
{
	INT32 iNewLife;
	SOLDIERCELL *pAttacker;
	if( !pTarget->pSoldier->bLife )
	{ //Soldier already dead (can't kill him again!)
		return;
	}
	pAttacker = pTarget->pAttacker[ index ];

	//creatures get damage reduction bonuses
	switch( pTarget->pSoldier->ubBodyType )
	{
		case LARVAE_MONSTER:
		case INFANT_MONSTER:
			break;
		case YAF_MONSTER:
		case YAM_MONSTER:
			pTarget->usHitDamage[index] = (pTarget->usHitDamage[index] + 2) / 4;
			break;
		case ADULTFEMALEMONSTER:
		case AM_MONSTER:
			pTarget->usHitDamage[index] = (pTarget->usHitDamage[index] + 3) / 6;
			break;
		case QUEENMONSTER:
			pTarget->usHitDamage[index] = (pTarget->usHitDamage[index] + 4) / 8;
			break;
	}

	iNewLife = pTarget->pSoldier->bLife - pTarget->usHitDamage[index];
	if( !pTarget->usHitDamage[index] )
	{ //bullet missed -- play a ricochet sound.
		if( pTarget->uiFlags & CELL_MERC )
			// AGILITY GAIN: Target "dodged" an attack
			StatChange(*pTarget->pSoldier, AGILAMT, 5, FROM_SUCCESS);
		PlayAutoResolveSample(SoundRange<MISS_1, MISS_8>(), 50, 1, MIDDLEPAN);
		return;
	}

	if( pAttacker->uiFlags & CELL_MERC )
	{ //Attacker is a player, so increment the number of shots that hit.
		gMercProfiles[ pAttacker->pSoldier->ubProfile ].usShotsHit++;
		// MARKSMANSHIP GAIN: Attacker's shot hits
		StatChange(*pAttacker->pSoldier, MARKAMT, 6, FROM_SUCCESS); // in addition to 3 for taking a shot
	}
	if( pTarget->uiFlags & CELL_MERC && pTarget->usHitDamage[ index ] )
	{ //Target is a player, so increment the times he has been wounded.
		gMercProfiles[ pTarget->pSoldier->ubProfile ].usTimesWounded++;
		// EXPERIENCE GAIN: Took some damage
		StatChange(*pTarget->pSoldier, EXPERAMT, 5 * (pTarget->usHitDamage[index] / 10), FROM_SUCCESS);
	}

	//bullet hit -- play an impact sound and a merc hit sound
	PlayAutoResolveSample(SoundRange<BULLET_IMPACT_1, BULLET_IMPACT_3>(), 50, 1, MIDDLEPAN);

	if( pTarget->pSoldier->bLife >= CONSCIOUSNESS )
	{
		if( gpAR->fSound )
			DoMercBattleSound(pTarget->pSoldier, BATTLE_SOUND_HIT1);
	}
	if( iNewLife < OKLIFE && pTarget->pSoldier->bLife >= OKLIFE )
	{ //the hit caused the merc to fall.  Play the falling sound
		PlayAutoResolveSample(FALL_1, 50, 1, MIDDLEPAN);
		pTarget->uiFlags &= ~CELL_RETREATING;
	}
	if( iNewLife <= 0 )
	{ //soldier has been killed
		if( pTarget->pAttacker[ index ]->uiFlags & CELL_PLAYER )
		{ //Player killed the enemy soldier -- update his stats as well as any assisters.
			SOLDIERCELL *pKiller;
			SOLDIERCELL *pAssister1, *pAssister2;
			pKiller = pTarget->pAttacker[ index ];
			pAssister1 = pTarget->pAttacker[ index < 2 ? index + 1 : 0 ];
			pAssister2 = pTarget->pAttacker[ index > 0 ? index - 1 : 2 ];
			if( pKiller == pAssister1 )
				pAssister1 = NULL;
			if( pKiller == pAssister2 )
				pAssister2 = NULL;
			if( pAssister1 == pAssister2 )
				pAssister2 = NULL;
			if( pKiller )
			{
				if( pKiller->uiFlags & CELL_MERC )
				{
					gMercProfiles[ pKiller->pSoldier->ubProfile ].usKills++;
					gStrategicStatus.usPlayerKills++;
					// EXPERIENCE CLASS GAIN:  Earned a kill
					StatChange(*pKiller->pSoldier, EXPERAMT, 10 * pTarget->pSoldier->bLevel, FROM_SUCCESS);
					HandleMoraleEvent( pKiller->pSoldier, MORALE_KILLED_ENEMY, gpAR->ubSectorX, gpAR->ubSectorY, 0  );
				}
				else if( pKiller->uiFlags & CELL_MILITIA )
					pKiller->pSoldier->ubMilitiaKills += 2;
			}
			if( pAssister1 )
			{
				if( pAssister1->uiFlags & CELL_MERC )
				{
					gMercProfiles[ pAssister1->pSoldier->ubProfile ].usAssists++;
					// EXPERIENCE CLASS GAIN:  Earned an assist
					StatChange(*pAssister1->pSoldier, EXPERAMT, 5 * pTarget->pSoldier->bLevel, FROM_SUCCESS);
				}
				else if( pAssister1->uiFlags & CELL_MILITIA )
					pAssister1->pSoldier->ubMilitiaKills++;
			}
			else if( pAssister2 )
			{
				if( pAssister2->uiFlags & CELL_MERC )
				{
					gMercProfiles[ pAssister2->pSoldier->ubProfile ].usAssists++;
					// EXPERIENCE CLASS GAIN:  Earned an assist
					StatChange(*pAssister2->pSoldier, EXPERAMT, 5 * pTarget->pSoldier->bLevel, FROM_SUCCESS);
				}
				else if( pAssister2->uiFlags & CELL_MILITIA )
					pAssister2->pSoldier->ubMilitiaKills++;
			}
		}
		if( pTarget->uiFlags & CELL_MERC && gpAR->fSound )
		{
			PlayAutoResolveSample(DOORCR_1, HIGHVOLUME, 1, MIDDLEPAN);
			PlayAutoResolveSample(HEADCR_1, HIGHVOLUME, 1, MIDDLEPAN);
		}
		if( iNewLife < -60 && !(pTarget->uiFlags & CELL_CREATURE) )
		{ //High damage death
			if( gpAR->fSound )
			{
				if( PreRandom( 3 ) )
					PlayAutoResolveSample(BODY_SPLAT_1, 50, 1, MIDDLEPAN);
				else
					PlayAutoResolveSample(HEADSPLAT_1, 50, 1, MIDDLEPAN);
			}
		}
		else
		{ //Normal death
			if( gpAR->fSound )
			{
				DoMercBattleSound( pTarget->pSoldier, BATTLE_SOUND_DIE1 );
			}
		}
		#ifdef INVULNERABILITY
			RefreshMerc( pTarget->pSoldier );
			return;
		#endif
	}
	//Adjust the soldiers stats based on the damage.
	pTarget->pSoldier->bLife = (INT8)MAX( iNewLife, 0 );
	if( pTarget->uiFlags & CELL_MERC && gpAR->pRobotCell)
	{
		UpdateRobotControllerGivenRobot( gpAR->pRobotCell->pSoldier );
	}

	if( pTarget->pSoldier->bLifeMax - pTarget->pSoldier->bBleeding - pTarget->usHitDamage[index] >= pTarget->pSoldier->bLife )
		pTarget->pSoldier->bBleeding += (INT8)pTarget->usHitDamage[index];
	else
		pTarget->pSoldier->bBleeding = (INT8)(pTarget->pSoldier->bLifeMax - pTarget->pSoldier->bLife);
	if( !pTarget->pSoldier->bLife )
	{
		gpAR->fRenderAutoResolve = TRUE;
		if( pTarget->uiFlags & CELL_MERC )
		{
			gpAR->usPlayerAttack -= pTarget->usAttack;
			gpAR->usPlayerDefence -= pTarget->usDefence;
			gpAR->ubAliveMercs--;
			pTarget->usAttack = 0;
			pTarget->usDefence = 0;
		}
		else if( pTarget->uiFlags & CELL_MILITIA )
		{
			gpAR->usPlayerAttack -= pTarget->usAttack;
			gpAR->usPlayerDefence -= pTarget->usDefence;
			gpAR->ubAliveCivs--;
			pTarget->usAttack = 0;
			pTarget->usDefence = 0;
		}
		else if( pTarget->uiFlags & (CELL_ENEMY|CELL_CREATURE) )
		{
			gpAR->usEnemyAttack -= pTarget->usAttack;
			gpAR->usEnemyDefence -= pTarget->usDefence;
			gpAR->ubAliveEnemies--;
			pTarget->usAttack = 0;
			pTarget->usDefence = 0;
		}
	}
	pTarget->uiFlags |= CELL_HITBYATTACKER | CELL_DIRTY;
}


static BOOLEAN IsBattleOver(void)
{
	INT32 iNumInvolvedMercs = 0;
	INT32 iNumMercsRetreated = 0;
	BOOLEAN fOnlyEPCsLeft = TRUE;
	if( gpAR->ubBattleStatus != BATTLE_IN_PROGRESS )
		return TRUE;
	FOR_ALL_AR_MERCS(i)
	{
		if (i->uiFlags & CELL_RETREATED)
		{
			++iNumMercsRetreated;
		}
		else if (i->pSoldier->bLife != 0 && !(i->uiFlags & CELL_EPC))
		{
			fOnlyEPCsLeft = FALSE;
			iNumInvolvedMercs++;
		}
	}
	if( gpAR->pRobotCell )
	{ //Do special robot checks
		SOLDIERTYPE *pRobot;
		pRobot = gpAR->pRobotCell->pSoldier;
		if (pRobot->robot_remote_holder == NULL)
		{ //Robot can't fight anymore.
			gpAR->usPlayerAttack -= gpAR->pRobotCell->usAttack;
			gpAR->pRobotCell->usAttack = 0;
			if( iNumInvolvedMercs == 1 && !gpAR->ubAliveCivs )
			{ //Robot is the only one left in battle, so instantly kill him.
				DoMercBattleSound( pRobot, BATTLE_SOUND_DIE1 );
				pRobot->bLife = 0;
				gpAR->ubAliveMercs--;
				iNumInvolvedMercs = 0;
			}
		}
	}
	if( !gpAR->ubAliveCivs && !iNumInvolvedMercs && iNumMercsRetreated )
	{ //RETREATED
		gpAR->ubBattleStatus = BATTLE_RETREAT;

		// wake everyone up
		WakeUpAllMercsInSectorUnderAttack( );

		RetreatAllInvolvedPlayerGroups( );
	}
	else if( !gpAR->ubAliveCivs && !iNumInvolvedMercs )
	{ //DEFEAT
		if( fOnlyEPCsLeft )
		{ //Kill the EPCs.
			FOR_ALL_AR_MERCS(i)
			{
				if (!(i->uiFlags & CELL_EPC)) continue;
				DoMercBattleSound(i->pSoldier, BATTLE_SOUND_DIE1);
				i->pSoldier->bLife = 0;
				gpAR->ubAliveMercs--;
			}
		}
		FOR_ALL_AR_ENEMIES(i)
		{
			if (i->pSoldier->bLife == 0) continue;
			if (gubEnemyEncounterCode != CREATURE_ATTACK_CODE)
			{
				DoMercBattleSound(i->pSoldier, BATTLE_SOUND_LAUGH1);
			}
			else
			{
				PlayJA2Sample(ACR_EATFLESH, 50, 1, MIDDLEPAN);
			}
			break;
		}
		gpAR->ubBattleStatus = BATTLE_DEFEAT;
	}
	else if( !gpAR->ubAliveEnemies )
	{ //VICTORY
		gpAR->ubBattleStatus = BATTLE_VICTORY;
	}
	else
	{
		return FALSE;
	}
	SetupDoneInterface();
	return TRUE;
}


//#define TESTSURRENDER


static void SetupSurrenderInterface(void);


static BOOLEAN AttemptPlayerCapture(void)
{
	BOOLEAN fConcious;
	INT32 iConciousEnemies;

#ifndef TESTSURRENDER

	//Only attempt capture if day is less than four.
	if( GetWorldDay() < STARTDAY_ALLOW_PLAYER_CAPTURE_FOR_RESCUE && !gpAR->fAllowCapture )
	{
		return FALSE;
	}
	if( gpAR->fPlayerRejectedSurrenderOffer )
	{
		return FALSE;
	}
	if( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE )
	{
		return FALSE;
	}
	if( gpAR->fCaptureNotPermittedDueToEPCs )
	{ //EPCs make things much more difficult when considering capture.  Simply don't allow it.
		return FALSE;
	}
	//Only attempt capture of mercs if there are 2 or 3 of them alive
	if( gpAR->ubAliveCivs || gpAR->ubAliveMercs < 2 || gpAR->ubAliveMercs > 3 )
	{
		return FALSE;
	}
	//if the number of alive enemies doesn't double the number of alive mercs, don't offer surrender.
	if( gpAR->ubAliveEnemies < gpAR->ubAliveMercs*2 )
	{
		return FALSE;
	}
	//make sure that these enemies are actually concious!
	iConciousEnemies = 0;
	FOR_ALL_AR_ENEMIES(i)
	{
		if (i->pSoldier->bLife < OKLIFE) continue;
		++iConciousEnemies;
	}
	if( iConciousEnemies < gpAR->ubAliveMercs * 2 )
	{
		return FALSE;
	}

	//So far, the conditions are right.  Now, we will determine if the the remaining players are
	//wounded and/or unconcious.  If any are concious, we will prompt for a surrender, otherwise,
	//it is automatic.
	fConcious = FALSE;
	FOR_ALL_AR_MERCS(i)
	{
		//if any of the 2 or 3 mercs has more than 60% life, then return.
		if (i->uiFlags & CELL_ROBOT) return FALSE;
		SOLDIERTYPE const& s = *i->pSoldier;
		if (s.bLife * 100 > s.bLifeMax * 60) return FALSE;
		if (s.bLife >= OKLIFE) fConcious = TRUE;
	}
	if( fConcious )
	{
		if( PreRandom( 100 ) < 2 )
		{
			SetupSurrenderInterface();
		}
	}
	else if( PreRandom( 100 ) < 25 )
#endif
	{
		BeginCaptureSquence( );

		gpAR->ubBattleStatus = BATTLE_CAPTURED;
		gpAR->fRenderAutoResolve = TRUE;
		SetupDoneInterface();
	}
	return TRUE;
}


static void SetupDoneInterface(void)
{
	gpAR->fRenderAutoResolve = TRUE;

	HideButton( gpAR->iButton[ PAUSE_BUTTON ] );
	HideButton( gpAR->iButton[ PLAY_BUTTON ] );
	HideButton( gpAR->iButton[ FAST_BUTTON ] );
	HideButton( gpAR->iButton[ FINISH_BUTTON ] );
	HideButton( gpAR->iButton[ RETREAT_BUTTON ] );
	HideButton( gpAR->iButton[ YES_BUTTON ] );
	HideButton( gpAR->iButton[ NO_BUTTON ] );
	if( gpAR->ubBattleStatus == BATTLE_VICTORY && gpAR->ubAliveMercs )
	{
		ShowButton( gpAR->iButton[ DONEWIN_BUTTON ] );
		ShowButton( gpAR->iButton[ BANDAGE_BUTTON ] );
	}
	else
	{
		ShowButton( gpAR->iButton[ DONELOSE_BUTTON ] );
	}
	DetermineBandageButtonState();
	FOR_ALL_AR_MERCS(i)
	{ //So they can't retreat!
		i->pRegion->Disable();
	}
}


static void SetupSurrenderInterface(void)
{
	HideButton( gpAR->iButton[ PAUSE_BUTTON ] );
	HideButton( gpAR->iButton[ PLAY_BUTTON ] );
	HideButton( gpAR->iButton[ FAST_BUTTON ] );
	HideButton( gpAR->iButton[ FINISH_BUTTON ] );
	HideButton( gpAR->iButton[ RETREAT_BUTTON ] );
	HideButton( gpAR->iButton[ BANDAGE_BUTTON ] );
	HideButton( gpAR->iButton[ DONEWIN_BUTTON ] );
	HideButton( gpAR->iButton[ DONELOSE_BUTTON ] );
	ShowButton( gpAR->iButton[ YES_BUTTON ] );
	ShowButton( gpAR->iButton[ NO_BUTTON ] );
	gpAR->fRenderAutoResolve = TRUE;
	gpAR->fPendingSurrender = TRUE;
}


static void HideSurrenderInterface(void)
{
	HideButton( gpAR->iButton[ PAUSE_BUTTON ] );
	HideButton( gpAR->iButton[ PLAY_BUTTON ] );
	HideButton( gpAR->iButton[ FAST_BUTTON ] );
	HideButton( gpAR->iButton[ FINISH_BUTTON ] );
	HideButton( gpAR->iButton[ RETREAT_BUTTON ] );
	HideButton( gpAR->iButton[ BANDAGE_BUTTON ] );
	HideButton( gpAR->iButton[ DONEWIN_BUTTON ] );
	HideButton( gpAR->iButton[ DONELOSE_BUTTON ] );
	HideButton( gpAR->iButton[ YES_BUTTON ] );
	HideButton( gpAR->iButton[ NO_BUTTON ] );
	gpAR->fPendingSurrender = FALSE;
	gpAR->fRenderAutoResolve = TRUE;
}


static void AcceptSurrenderCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		BeginCaptureSquence( );

		gpAR->ubBattleStatus = BATTLE_SURRENDERED;
		gpAR->fPendingSurrender = FALSE;
		SetupDoneInterface();
	}
}


static void RejectSurrenderCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gpAR->fPlayerRejectedSurrenderOffer = TRUE;
		HideSurrenderInterface();
	}
}


static void ProcessBattleFrame(void)
{
	INT32 iRandom;
	SOLDIERCELL *pAttacker, *pTarget;
	UINT32 uiDiff;
	static INT32 iTimeSlice = 0;
	static BOOLEAN fContinue = FALSE;
	static UINT32 uiSlice = 0;
	static INT32 iTotal = 0;
	static INT32 iMercs = 0;
	static INT32 iCivs = 0;
	static INT32 iEnemies = 0;
	static INT32 iMercsLeft = 0;
	static INT32 iCivsLeft = 0;
	static INT32 iEnemiesLeft = 0;
	BOOLEAN found = FALSE;
	INT32 iTime, iAttacksThisFrame;

	pAttacker = NULL;
	iAttacksThisFrame = 0;

	if( fContinue )
	{
		gpAR->uiCurrTime = GetJA2Clock();
		fContinue = FALSE;
		goto CONTINUE_BATTLE;
	}
	//determine how much real-time has passed since the last frame
	if( gpAR->uiCurrTime )
	{
		gpAR->uiPrevTime = gpAR->uiCurrTime;
		gpAR->uiCurrTime = GetJA2Clock();
	}
	else
	{
		gpAR->uiCurrTime = GetJA2Clock();
		return;
	}
	if( gpAR->fPaused )
		return;

	uiDiff = gpAR->uiCurrTime - gpAR->uiPrevTime;
	if( gpAR->uiTimeSlice < 0xffffffff )
	{
		iTimeSlice = uiDiff*gpAR->uiTimeSlice/1000;
	}
	else
	{ //largest positive signed value
		iTimeSlice = 0x7fffffff;
	}

	while( iTimeSlice > 0 )
	{
		uiSlice = MIN( iTimeSlice, 1000 );
		if( gpAR->ubBattleStatus == BATTLE_IN_PROGRESS )
			gpAR->uiTotalElapsedBattleTimeInMilliseconds += uiSlice;

		//Now process each of the players
		iTotal = gpAR->ubMercs + gpAR->ubCivs + gpAR->ubEnemies + 1;
		iMercs	 = iMercsLeft		= gpAR->ubMercs;
		iCivs		 = iCivsLeft		= gpAR->ubCivs;
		iEnemies = iEnemiesLeft = gpAR->ubEnemies;
		FOR_ALL_AR_MERCS(i)
			i->uiFlags &= ~CELL_PROCESSED;
		FOR_ALL_AR_CIVS(i)
			i->uiFlags &= ~CELL_PROCESSED;
		FOR_ALL_AR_ENEMIES(i)
			i->uiFlags &= ~CELL_PROCESSED;
		while( --iTotal )
		{
			INT32 cnt;
			if( iTimeSlice != 0x7fffffff && GetJA2Clock() > gpAR->uiCurrTime+17 ||
				!gpAR->fInstantFinish && iAttacksThisFrame > (gpAR->ubMercs+gpAR->ubCivs+gpAR->ubEnemies)/4 )
			{ //We have spent too much time in here.  In order to maintain 60FPS, we will
				//leave now, which will allow for updating of the graphics (and mouse cursor),
				//and all of the necessary locals are saved via static variables.  It'll check
				//the fContinue flag, and goto the CONTINUE_BATTLE label the next time this function
				//is called.
				fContinue = TRUE;
				return;
			}
			CONTINUE_BATTLE:
			if( IsBattleOver() || gubEnemyEncounterCode != CREATURE_ATTACK_CODE && AttemptPlayerCapture() )
				return;

			iRandom = PreRandom( iTotal );
			found = FALSE;
			if( iMercs && iRandom < iMercsLeft )
			{
				iMercsLeft--;
				while( !found )
				{
					iRandom = PreRandom( iMercs );
					pAttacker = &gpMercs[ iRandom ];
					if( !(pAttacker->uiFlags & CELL_PROCESSED ) )
					{
						pAttacker->uiFlags |= CELL_PROCESSED;
						found = TRUE;
					}
				}
			}
			else if( iCivs && iRandom < iMercsLeft + iCivsLeft )
			{
				iCivsLeft--;
				while( !found )
				{
					iRandom = PreRandom( iCivs );
					pAttacker = &gpCivs[ iRandom ];
					if( !(pAttacker->uiFlags & CELL_PROCESSED ) )
					{
						pAttacker->uiFlags |= CELL_PROCESSED;
						found = TRUE;
					}
				}
			}
			else if( iEnemies && iEnemiesLeft )
			{
				iEnemiesLeft--;
				while( !found )
				{
					iRandom = PreRandom( iEnemies );
					pAttacker = &gpEnemies[ iRandom ];
					if( !(pAttacker->uiFlags & CELL_PROCESSED ) )
					{
						pAttacker->uiFlags |= CELL_PROCESSED;
						found = TRUE;
					}
				}
			}
			else
				AssertMsg( 0, "Logic error in ProcessBattleFrame()" );
			//Apply damage and play miss/hit sounds if delay between firing and hit has expired.
			if( !(pAttacker->uiFlags & CELL_RETREATED ) )
			{
				for( cnt = 0; cnt < 3; cnt++ )
				{ //Check if any incoming bullets have hit the target.
					if( pAttacker->usNextHit[ cnt ] )
					{
						iTime = pAttacker->usNextHit[ cnt ];
						iTime -= uiSlice;
						if( iTime >= 0 )
						{ //Bullet still on route.
							pAttacker->usNextHit[ cnt ] = (UINT16)iTime;
						}
						else
						{ //Bullet is going to hit/miss.
							TargetHitCallback( pAttacker, cnt );
							pAttacker->usNextHit[ cnt ] = 0;
						}
					}
				}
			}
			if( pAttacker->pSoldier->bLife < OKLIFE || pAttacker->uiFlags & CELL_RETREATED )
			{
				if( !(pAttacker->uiFlags & CELL_CREATURE) || !pAttacker->pSoldier->bLife )
					continue; //can't attack if you are unconcious or not around (Or a live creature)
			}
			iTime = pAttacker->usNextAttack;
			iTime -= uiSlice;
			if( iTime > 0 )
			{
				pAttacker->usNextAttack = (UINT16)iTime;
				continue;
			}
			else
			{
				if( pAttacker->uiFlags & CELL_RETREATING )
				{ //The merc has successfully retreated.  Remove the stats, and continue on.
					if( pAttacker == gpAR->pRobotCell )
					{
						if (gpAR->pRobotCell->pSoldier->robot_remote_holder == NULL)
						{
							gpAR->pRobotCell->uiFlags &= ~CELL_RETREATING;
							gpAR->pRobotCell->uiFlags |= CELL_DIRTY;
							gpAR->pRobotCell->usNextAttack = 0xffff;
							continue;
						}
					}
					gpAR->usPlayerDefence -= pAttacker->usDefence;
					pAttacker->usDefence = 0;
					pAttacker->uiFlags |= CELL_RETREATED;
					continue;
				}
				if( pAttacker->usAttack )
				{
					pTarget = ChooseTarget( pAttacker );
					if( pAttacker->uiFlags & CELL_CREATURE && PreRandom( 100 ) < 7 )
						PlayAutoResolveSample(SoundRange<ACR_SMELL_THREAT, ACR_SMELL_PREY>(), 50, 1, MIDDLEPAN);
					else
						AttackTarget( pAttacker, pTarget );
					ResetNextAttackCounter( pAttacker );
					pAttacker->usNextAttack += (UINT16)iTime; //tack on the remainder
					iAttacksThisFrame++;
				}
			}
		}
		if( iTimeSlice != 0x7fffffff )//|| !gpAR->fInstantFinish )
		{
			iTimeSlice -= 1000;
		}
	}
}


BOOLEAN IsAutoResolveActive()
{
	//is the autoresolve up or not?
	if( gpAR )
	{
		return TRUE;
	}
	return FALSE;
}

UINT8 GetAutoResolveSectorID()
{
	if( gpAR )
	{
		return (UINT8)SECTOR( gpAR->ubSectorX, gpAR->ubSectorY );
	}
	return 0xff;
}

//Returns TRUE if a battle is happening or sector is loaded
BOOLEAN GetCurrentBattleSectorXYZ( INT16 *psSectorX, INT16 *psSectorY, INT16 *psSectorZ )
{
	if( gpAR )
	{
		*psSectorX = gpAR->ubSectorX;
		*psSectorY = gpAR->ubSectorY;
		*psSectorZ = 0;
		return TRUE;
	}
	else if( gfPreBattleInterfaceActive )
	{
		*psSectorX = gubPBSectorX;
		*psSectorY = gubPBSectorY;
		*psSectorZ = gubPBSectorZ;
		return TRUE;
	}
	else if( gfWorldLoaded )
	{
		*psSectorX = gWorldSectorX;
		*psSectorY = gWorldSectorY;
		*psSectorZ = gbWorldSectorZ;
		return TRUE;
	}
	else
	{
		*psSectorX = 0;
		*psSectorY = 0;
		*psSectorZ = -1;
		return FALSE;
	}
}


BOOLEAN GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(INT16* psSectorX, INT16* psSectorY, INT16* psSectorZ)
{
	if( gpAR )
	{
		*psSectorX = gpAR->ubSectorX;
		*psSectorY = gpAR->ubSectorY;
		*psSectorZ = 0;
		return TRUE;
	}
	else if( gfPreBattleInterfaceActive )
	{
		*psSectorX = gubPBSectorX;
		*psSectorY = gubPBSectorY;
		*psSectorZ = gubPBSectorZ;
		return TRUE;
	}
	else if( gfWorldLoaded )
	{
		*psSectorX = gWorldSectorX;
		*psSectorY = gWorldSectorY;
		*psSectorZ = gbWorldSectorZ;
		if( gTacticalStatus.fEnemyInSector )
		{
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		*psSectorX = 0;
		*psSectorY = 0;
		*psSectorZ = -1;
		return FALSE;
	}
}


static void AutoBandageFinishedCallback(MessageBoxReturnValue const ubResult)
{
	SetupDoneInterface();
}
