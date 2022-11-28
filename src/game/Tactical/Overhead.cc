#include "Handle_Doors.h"
#include "Handle_Items.h"
#include "MapScreen.h"
#include "Soldier_Find.h"
#include "Spread_Burst.h"
#include "TileDef.h"
#include "VObject.h"
#include "WCheck.h"
#include "Debug.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "RenderWorld.h"
#include "Assignments.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Isometric_Utils.h"
#include "Event_Pump.h"
#include "Timer_Control.h"
#include "Render_Fun.h"
#include "Render_Dirty.h"
#include "MouseSystem.h"
#include "Interface.h"
#include "SysUtil.h"
#include "Points.h"
#include "Random.h"
#include "AI.h"
#include "Interactive_Tiles.h"
#include "Soldier_Ani.h"
#include "Overhead.h"
#include "OppList.h"
#include "Sound_Control.h"
#include "Font_Control.h"
#include "Lighting.h"
#include "PathAI.h"
#include "ScreenIDs.h"
#include "Interface_Cursors.h"
#include "Weapons.h"
#include "Rotting_Corpses.h"
#include "Structure.h"
#include "Interface_Panels.h"
#include "Message.h"
#include "Items.h"
#include "Soldier_Profile.h"
#include "FOV.h"
#include "Soldier_Macros.h"
#include "EditorMercs.h"
#include "Soldier_Tile.h"
#include "Structure_Wrap.h"
#include "Tile_Animation.h"
#include "Strategic_Merc_Handler.h"
#include "Strategic_Movement.h"
#include "Strategic_Turns.h"
#include "Squads.h"
#include "Morale.h"
#include "Campaign.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Faces.h"
#include "Dialogue_Control.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "NPC.h"
#include "StrategicMap.h"
#include "Soldier_Functions.h"
#include "Auto_Bandage.h"
#include "Game_Event_Hook.h"
#include "Explosion_Control.h"
#include "SkillCheck.h"
#include "World_Items.h"
#include "Smell.h"
#include "Player_Command.h"
#include "GameSettings.h"
#include "MessageBoxScreen.h"
#include "Game_Clock.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Mines.h"
#include "Interface_Items.h"
#include "Text.h"
#include "Keys.h"
#include "Boxing.h"
#include "Town_Militia.h"
#include "Meanwhile.h"
#include "Map_Screen_Helicopter.h"
#include "Interface_Control.h"
#include "Exit_Grids.h"
#include "Game_Init.h"
#include "JAScreens.h"
#include "Arms_Dealer_Init.h"
#include "Interface_Utils.h"
#include "Civ_Quotes.h"
#include "Drugs_And_Alcohol.h"
#include "History.h"
#include "LOS.h"
#include "Interface_Dialogue.h"
#include "Strategic_AI.h"
#include "End_Game.h"
#include "Strategic_Status.h"
#include "PreBattle_Interface.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "NewStrings.h"
#include "Soldier.h"
#include "Logger.h"

#include <string_theory/string>

#include <algorithm>
#include <iterator>

#define RT_DELAY_BETWEEN_AI_HANDLING	50
#define RT_AI_TIMESLICE			10

UINT32        guiRTAILastUpdateTime = 0;
static UINT32 guiAISlotToHandle    = 0;
#define HANDLE_OFF_MAP_MERC		0xFFFF
#define RESET_HANDLE_OF_OFF_MAP_MERCS	0xFFFF
static UINT32 guiAIAwaySlotToHandle = RESET_HANDLE_OF_OFF_MAP_MERCS;

#define PAUSE_ALL_AI_DELAY		1500

static BOOLEAN gfPauseAllAI      = FALSE;
static UINT32  guiPauseAllAITimer = 0;


BOOLEAN gfSurrendered = FALSE;


#define NEW_FADE_DELAY			60


// Soldier List used for all soldier overhead interaction
SOLDIERTYPE  Menptr[TOTAL_SOLDIERS];

SOLDIERTYPE* MercSlots[TOTAL_SOLDIERS];
UINT32       guiNumMercSlots = 0;

TacticalStatusType gTacticalStatus;

static SOLDIERTYPE* AwaySlots[TOTAL_SOLDIERS];
static UINT32       guiNumAwaySlots = 0;

// Global for current selected soldier
SOLDIERTYPE* g_selected_man;


const char* const gzActionStr[] =
{
	"NONE",

	"RANDOM PATROL",
	"SEEK FRIEND",
	"SEEK OPPONENT",
	"TAKE COVER",
	"GET CLOSER",

	"POINT PATROL",
	"LEAVE WATER GAS",
	"SEEK NOISE",
	"ESCORTED MOVE",
	"RUN AWAY",

	"KNIFE MOVE",
	"APPROACH MERC",
	"TRACK",
	"EAT",
	"PICK UP ITEM",

	"SCHEDULE MOVE",
	"WALK",
	"RUN",
	"MOVE TO CLIMB",
	"CHG FACING",

	"CHG STANCE",
	"YELLOW ALERT",
	"RED ALERT",
	"CREATURE CALL",
	"PULL TRIGGER",

	"USE DETONATOR",
	"FIRE GUN",
	"TOSS PROJECTILE",
	"KNIFE STAB",
	"THROW KNIFE",

	"GIVE AID",
	"WAIT",
	"PENDING ACTION",
	"DROP ITEM",
	"COWER",

	"STOP COWERING",
	"OPEN/CLOSE DOOR",
	"UNLOCK DOOR",
	"LOCK DOOR",
	"LOWER GUN",

	"ABSOLUTELY NONE",
	"CLIMB ROOF",
	"END TURN",
	"EC&M",
	"TRAVERSE DOWN",
	"OFFER SURRENDER"
};


struct TeamInfo
{
	UINT8    size;
	INT8     side;
	bool     human;
	COLORVAL colour;
};


// Militia guys on our side.
// Creatures are on no one's side but their own.
// NB: side 2 is used for hostile rebels.
// Rest hostile (enemies, or civilians; civs are potentially hostile but neutral)
static TeamInfo const g_default_team_info[] =
{
	{ 20,                 0, true,  FROMRGB(255, 255,   0) }, // Us
	{ 32,                 1, false, FROMRGB(255,   0,   0) }, // Enemy
	{ 32,                 3, false, FROMRGB(255,   0, 255) }, // Creature
	{ 32,                 0, false, FROMRGB(  0, 255,   0) }, // Rebels (our guys)
	{ 32,                 1, false, FROMRGB(255, 255, 255) }, // Civilians
	{ NUM_PLANNING_MERCS, 0, true,  FROMRGB(  0,   0, 255) }  // Planning soldiers
};


UINT8         gubWaitingForAllMercsToExitCode  = 0;
static INT8   gbNumMercsUntilWaitingOver       = 0;
static UINT32 guiWaitingForAllMercsToExitTimer = 0;
BOOLEAN       gfKillingGuysForLosingBattle     = FALSE;


static INT32 GetFreeMercSlot(void)
{
	for (UINT32 i = 0; i < guiNumMercSlots; ++i)
	{
		if (MercSlots[i] == NULL) return i;
	}

	if (guiNumMercSlots < TOTAL_SOLDIERS) return guiNumMercSlots++;

	return -1;
}


static void RecountMercSlots(void)
{
	// set equal to 0 as a default
	for (INT32 i = (INT32)guiNumMercSlots - 1; i >= 0; --i)
	{
		if (MercSlots[i] != NULL)
		{
			guiNumMercSlots = i + 1;
			return;
		}
	}
	// no mercs found
	guiNumMercSlots = 0;
}


void AddMercSlot(SOLDIERTYPE* pSoldier)
{
	const INT32 iMercIndex = GetFreeMercSlot();
	if (iMercIndex != -1) MercSlots[iMercIndex] = pSoldier;
}


BOOLEAN RemoveMercSlot(SOLDIERTYPE* pSoldier)
{
	CHECKF(pSoldier != NULL);

	for (UINT32 i = 0; i < guiNumMercSlots; ++i)
	{
		if (MercSlots[i] == pSoldier)
		{
			MercSlots[i] = NULL;
			RecountMercSlots();
			return TRUE;
		}
	}

	// TOLD TO DELETE NON-EXISTANT SOLDIER
	return FALSE;
}


static INT32 GetFreeAwaySlot(void)
{
	for (UINT32 i = 0; i < guiNumAwaySlots; ++i)
	{
		if (AwaySlots[i] == NULL) return i;
	}

	if (guiNumAwaySlots < TOTAL_SOLDIERS) return guiNumAwaySlots++;

	return -1;
}


static void RecountAwaySlots(void)
{
	for (INT32 i = guiNumAwaySlots - 1; i >=0; --i)
	{
		if (AwaySlots[i] != NULL)
		{
			guiNumAwaySlots = i + 1;
			return;
		}
	}
	// no mercs found
	guiNumAwaySlots = 0;
}


INT32 AddAwaySlot(SOLDIERTYPE* pSoldier)
{
	const INT32 iAwayIndex = GetFreeAwaySlot();
	if (iAwayIndex != -1) AwaySlots[iAwayIndex] = pSoldier;
	return iAwayIndex;
}


BOOLEAN RemoveAwaySlot(SOLDIERTYPE* pSoldier)
{
	CHECKF(pSoldier != NULL);

	for (UINT32 i = 0; i < guiNumAwaySlots; ++i)
	{
		if (AwaySlots[i] == pSoldier)
		{
			AwaySlots[i] = NULL;
			RecountAwaySlots();
			return TRUE;
		}
	}

	// TOLD TO DELETE NON-EXISTANT SOLDIER
	return FALSE;
}


INT32 MoveSoldierFromMercToAwaySlot(SOLDIERTYPE* pSoldier)
{
	BOOLEAN fRet = RemoveMercSlot(pSoldier);
	if (!fRet) return -1;

	if (!(pSoldier->uiStatusFlags & SOLDIER_OFF_MAP))
	{
		RemoveManFromTeam(pSoldier->bTeam);
	}

	pSoldier->bInSector      = FALSE;
	pSoldier->uiStatusFlags |= SOLDIER_OFF_MAP;
	return AddAwaySlot(pSoldier);
}


void MoveSoldierFromAwayToMercSlot(SOLDIERTYPE* const pSoldier)
{
	if (!RemoveAwaySlot(pSoldier)) return;

	AddManToTeam(pSoldier->bTeam);

	pSoldier->bInSector      = TRUE;
	pSoldier->uiStatusFlags &= ~SOLDIER_OFF_MAP;
	AddMercSlot(pSoldier);
}

static void HandleBrothelWallDestroyed(INT16 sSectorX, INT16 sSectorY, INT8 sSectorZ, INT16 sGridNo, STRUCTURE* s, UINT8 ubDamage, BOOLEAN fDestroyed);

void InitTacticalEngine()
{
	InitRenderParams(0);
	InitializeTacticalInterface();
	InitializeGameVideoObjects();
	LoadPaletteData();
	LoadDialogueControlGraphics();
	LoadFacesGraphics();
	LoadInterfacePanelGraphics();
	LoadSpreadBurstGraphics();

	LoadLockTable();
	InitPathAI();
	InitAI();
	InitOverhead();

	OnStructureDamaged.addListener("default:brothel", HandleBrothelWallDestroyed);
	BeforeStructureDamaged.addListener("default", HandleStatueDamaged);
}


void ShutdownTacticalEngine(void)
{
	DeleteSpreadBurstGraphics();
	DeleteInterfacePanelGraphics();
	DeleteFacesGraphics();
	DeleteDialogueControlGraphics();
	DeletePaletteData();
	UnloadExternalNPCFaces();
	ShutDownPathAI();
	UnLoadCarPortraits();
	ShutdownNPCQuotes();
}


void InitOverhead()
{
	std::fill(std::begin(MercSlots), std::end(MercSlots), nullptr);
	std::fill(std::begin(AwaySlots), std::end(AwaySlots), nullptr);
	std::fill(std::begin(Menptr), std::end(Menptr), SOLDIERTYPE{});

	TacticalStatusType& t = gTacticalStatus;
	t = TacticalStatusType{};
	t.uiFlags                 = 0x000000004; // TURNBASED, for save game compatibility
	t.sSlideTarget            = NOWHERE;
	t.uiTimeOfLastInput       = GetJA2Clock();
	t.uiTimeSinceDemoOn       = GetJA2Clock();
	t.bRealtimeSpeed          = MAX_REALTIME_SPEED_VAL / 2;
	FOR_EACH(INT16, i, t.sPanicTriggerGridNo) *i = NOWHERE;
	t.fDidGameJustStart       = TRUE;
	t.ubLastRequesterTargetID = NO_PROFILE;

	// Set team values
	UINT8 team_start = 0;
	for (UINT32 i = 0; i != MAXTEAMS; ++i)
	{
		TacticalTeamType& team = t.Team[i];
		TeamInfo const&   info = g_default_team_info[i];
		// For now, set hard-coded values
		team.bFirstID           = team_start;
		team_start += info.size;
		team.bLastID            = team_start - 1;
		team.RadarColor         = info.colour;
		team.bSide              = info.side;
		team.bAwareOfOpposition = FALSE;
		team.bHuman             = info.human;
		team.last_merc_to_radio = 0;
	}

	gpCustomizableTimerCallback = 0;

	// Reset cursor
	gpItemPointer        = 0;
	gpItemPointerSoldier = 0;
	std::fill(std::begin(gbInvalidPlacementSlot), std::end(gbInvalidPlacementSlot), 0);

	InitCivQuoteSystem();
	ZeroAnimSurfaceCounts();
}


void ShutdownOverhead(void)
{
	// Delete any soldiers which have been created!
	FOR_EACH_SOLDIER(i) DeleteSoldier(*i);
}


static BOOLEAN NextAIToHandle(UINT32 uiCurrAISlot)
{
	UINT32 cnt;
	if (uiCurrAISlot >= guiNumMercSlots)
	{
		// last person to handle was an off-map merc, so now we start looping at the beginning
		// again
		cnt = 0;
	}
	else
	{
		// continue on from the last person we handled
		cnt = uiCurrAISlot + 1;
	}

	for (; cnt < guiNumMercSlots; ++cnt)
	{
		if (MercSlots[cnt] &&
				(MercSlots[cnt]->bTeam != OUR_TEAM || MercSlots[cnt]->uiStatusFlags & SOLDIER_PCUNDERAICONTROL))
		{
			// aha! found an AI guy!
			guiAISlotToHandle = cnt;
			return TRUE;
		}
	}

	// set so that even if there are no off-screen mercs to handle, we will loop back to
	// the start of the array
	guiAISlotToHandle = HANDLE_OFF_MAP_MERC;

	// didn't find an AI guy to handle after the last one handled and the # of slots
	// it's time to check for an off-map merc... maybe
	if (guiNumAwaySlots > 0)
	{
		if (guiAIAwaySlotToHandle + 1 >= guiNumAwaySlots)
		{
			// start looping from the beginning
			cnt = 0;
		}
		else
		{
			// continue on from the last person we handled
			cnt = guiAIAwaySlotToHandle + 1;
		}

		for (; cnt < guiNumAwaySlots; ++cnt)
		{
			if (AwaySlots[cnt] && AwaySlots[cnt]->bTeam != OUR_TEAM)
			{
				// aha! found an AI guy!
				guiAIAwaySlotToHandle = cnt;
				return FALSE;
			}
		}

		// reset awayAISlotToHandle, but DON'T loop again, away slots not that important
		guiAIAwaySlotToHandle = RESET_HANDLE_OF_OFF_MAP_MERCS;
	}

	return FALSE;
}


void PauseAITemporarily(void)
{
	gfPauseAllAI      = TRUE;
	guiPauseAllAITimer = GetJA2Clock();
}


void PauseAIUntilManuallyUnpaused(void)
{
	gfPauseAllAI      = TRUE;
	guiPauseAllAITimer = 0;
}


void UnPauseAI(void)
{
	// overrides any timer too
	gfPauseAllAI      = FALSE;
	guiPauseAllAITimer = 0;
}


static void HandleBloodForNewGridNo(const SOLDIERTYPE* pSoldier);
static BOOLEAN HandleAtNewGridNo(SOLDIERTYPE* pSoldier, BOOLEAN* pfKeepMoving);
static void HandleCreatureTenseQuote(void);


void ExecuteOverhead(void)
{
	// Diagnostic Stuff
	static INT32 iTimerTest = 0;

	BOOLEAN fKeepMoving;

	if (COUNTERDONE(TOVERHEAD))
	{
		RESETCOUNTER(TOVERHEAD);

		// Diagnostic Stuff
		UINT32 uiTimerVal = GetJA2Clock();
		guiTimerDiag = uiTimerVal - iTimerTest;
		iTimerTest  = uiTimerVal;

		// ANIMATED TILE STUFF
		UpdateAniTiles();

		// BOMBS!!!
		HandleExplosionQueue();

		HandleCreatureTenseQuote();

		CheckHostileOrSayQuoteList();

		if (gfPauseAllAI && guiPauseAllAITimer && uiTimerVal - guiPauseAllAITimer > PAUSE_ALL_AI_DELAY)
		{
			// ok, stop pausing the AI!
			gfPauseAllAI = FALSE;
		}

		BOOLEAN fHandleAI = FALSE;
		if (!gfPauseAllAI)
		{
			// AI limiting crap
			if (!(gTacticalStatus.uiFlags & INCOMBAT))
			{
				if (uiTimerVal - guiRTAILastUpdateTime > RT_DELAY_BETWEEN_AI_HANDLING)
				{
					guiRTAILastUpdateTime = uiTimerVal;
					// figure out which AI guy to handle this time around,
					// starting with the slot AFTER the current AI guy
					fHandleAI = NextAIToHandle(guiAISlotToHandle);
				}
			}
		}

		for (UINT32 cnt = 0; cnt < guiNumMercSlots; ++cnt)
		{
			SOLDIERTYPE* pSoldier = MercSlots[cnt];
			SoldierSP soldier = GetSoldier(pSoldier);

			// Syncronize for upcoming soldier counters
			SYNCTIMECOUNTER();

			if (pSoldier != NULL)
			{
				HandlePanelFaceAnimations(pSoldier);

				// Handle damage counters
				if (pSoldier->fDisplayDamage)
				{
					if (TIMECOUNTERDONE(pSoldier->DamageCounter, DAMAGE_DISPLAY_DELAY))
					{
						pSoldier->bDisplayDamageCount++;
						pSoldier->sDamageX += 1;
						pSoldier->sDamageY -= 1;

						RESETTIMECOUNTER(pSoldier->DamageCounter, DAMAGE_DISPLAY_DELAY);
					}

					if (pSoldier->bDisplayDamageCount >= 8)
					{
						pSoldier->bDisplayDamageCount = 0;
						pSoldier->sDamage             = 0;
						pSoldier->fDisplayDamage      = FALSE;
					}
				}

				// Checkout fading
				if (pSoldier->fBeginFade &&
						TIMECOUNTERDONE(pSoldier->FadeCounter, NEW_FADE_DELAY))
				{
					RESETTIMECOUNTER(pSoldier->FadeCounter, NEW_FADE_DELAY);

					// Fade out....
					if (pSoldier->fBeginFade == 1)
					{
						INT8 bShadeLevel = pSoldier->ubFadeLevel & 0x0f;
						bShadeLevel = std::min(bShadeLevel + 1, SHADE_MIN);

						if (bShadeLevel >= SHADE_MIN - 3)
						{
							pSoldier->fBeginFade = FALSE;
							pSoldier->bVisible = -1;

							// Set levelnode shade level....
							if (pSoldier->pLevelNode)
							{
								pSoldier->pLevelNode->ubShadeLevel = bShadeLevel;
							}

							// Set Anim speed accordingly!
							SetSoldierAniSpeed(pSoldier);
						}

						bShadeLevel |= pSoldier->ubFadeLevel & 0x30;
						pSoldier->ubFadeLevel = bShadeLevel;
					}
					else if (pSoldier->fBeginFade == 2)
					{
						INT8 bShadeLevel = pSoldier->ubFadeLevel & 0x0f;
						//ubShadeLevel = std::max(ubShadeLevel-1, gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel );
						bShadeLevel = std::max(0, bShadeLevel - 1);

						if (bShadeLevel <= gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel)
						{
							bShadeLevel = gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;

							pSoldier->fBeginFade = FALSE;
							//pSoldier->bVisible = -1;
							//pSoldier->ubFadeLevel = gpWorldLevelData[ pSoldier->sGridNo ].pLandHead->ubShadeLevel;

							// Set levelnode shade level....
							if (pSoldier->pLevelNode)
							{
								pSoldier->pLevelNode->ubShadeLevel = bShadeLevel;
							}

							// Set Anim speed accordingly!
							SetSoldierAniSpeed(pSoldier);
						}

						bShadeLevel |= pSoldier->ubFadeLevel & 0x30;
						pSoldier->ubFadeLevel = bShadeLevel;
					}
				}

				// Check if we have a new visiblity and shade accordingly down
				if (pSoldier->bLastRenderVisibleValue != pSoldier->bVisible)
				{
					HandleCrowShadowVisibility(*pSoldier);

					// Check for fade out....
					if (pSoldier->bVisible == -1 && pSoldier->bLastRenderVisibleValue >= 0)
					{
						if (pSoldier->sGridNo != NOWHERE)
						{
							pSoldier->ubFadeLevel = gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;
						}
						pSoldier->fBeginFade           = TRUE;
						pSoldier->sLocationOfFadeStart = pSoldier->sGridNo;

						// OK, re-evaluate guy's roof marker
						HandlePlacingRoofMarker(*pSoldier, false, false);

						pSoldier->bVisible = -2;
					}

					// Check for fade in.....
					if (pSoldier->bVisible != -1 && pSoldier->bLastRenderVisibleValue == -1 && pSoldier->bTeam != OUR_TEAM)
					{
						pSoldier->ubFadeLevel          = SHADE_MIN - 3;
						pSoldier->fBeginFade           = 2;
						pSoldier->sLocationOfFadeStart = pSoldier->sGridNo;

						// OK, re-evaluate guy's roof marker
						HandlePlacingRoofMarker(*pSoldier, true, false);
					}

				}
				pSoldier->bLastRenderVisibleValue = pSoldier->bVisible;


				// Handle stationary polling...
				if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY || pSoldier->fNoAPToFinishMove)
				{
					// Are are stationary....
					// Were we once moving...?
					if (pSoldier->fSoldierWasMoving && pSoldier->bVisible > -1)
					{
						pSoldier->fSoldierWasMoving = FALSE;

						HandlePlacingRoofMarker(*pSoldier, true, false);

						if (!gGameSettings.fOptions[TOPTION_MERC_ALWAYS_LIGHT_UP])
						{
							DeleteSoldierLight(pSoldier);
							PositionSoldierLight(pSoldier);
						}
					}
				}
				else
				{
					// We are moving....
					// Were we once stationary?
					if (!pSoldier->fSoldierWasMoving)
					{
						pSoldier->fSoldierWasMoving = TRUE;
						HandlePlacingRoofMarker(*pSoldier, false, false);
					}
				}

				// Handle animation update counters
				// ATE: Added additional check here for special value of anispeed that pauses all updates
#ifndef BOUNDS_CHECKER
				if (TIMECOUNTERDONE(pSoldier->UpdateCounter, pSoldier->sAniDelay) &&
					pSoldier->sAniDelay != 10000)
#endif
				{
					// Check if we need to look for items
					if (pSoldier->uiStatusFlags & SOLDIER_LOOKFOR_ITEMS)
					{
						RevealRoofsAndItems(pSoldier, FALSE);
						pSoldier->uiStatusFlags &= ~SOLDIER_LOOKFOR_ITEMS;
					}

					RESETTIMECOUNTER(pSoldier->UpdateCounter, pSoldier->sAniDelay);

					BOOLEAN fNoAPsForPendingAction = FALSE;

					// Check if we are moving and we deduct points and we have no points
					if ((!(gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_MOVING | ANIM_SPECIALMOVE))
						|| !pSoldier->fNoAPToFinishMove) &&
						!pSoldier->fPauseAllAnimation)
					{
						if (!AdjustToNextAnimationFrame(pSoldier))
						{
							continue;
						}

						if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE))
						{
							// Check if we are waiting for an opened path
							HandleNextTileWaiting(pSoldier);
						}

						// Handle situation when Darrent goes through the ring to give money.
						// He jumps out of the ring right beside the player.
						// Sutiation when he walks to the player is handled the usual way.
						if((pSoldier->ubProfile == DARREN)
							&& (pSoldier->sFinalDestination == pSoldier->sGridNo)
							&& (pSoldier->usAnimState == STANDING)
							&& (pSoldier->ubPendingAction == MERC_GIVEITEM))
						{
							soldier->handlePendingAction(gTacticalStatus.uiFlags & INCOMBAT);
						}
						// Update world data with new position, etc
						// Determine gameworld cells corrds of guy
						else if (gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_MOVING | ANIM_SPECIALMOVE) && !(pSoldier->uiStatusFlags & SOLDIER_PAUSEANIMOVE))
						{
							fKeepMoving = TRUE;

							pSoldier->fPausedMove = FALSE;

							// CHECK TO SEE IF WE'RE ON A MIDDLE TILE
							if (pSoldier->fPastXDest && pSoldier->fPastYDest)
							{
								pSoldier->fPastXDest = pSoldier->fPastYDest = FALSE;
								// assign X/Y values back to make sure we are at the center of the tile
								// (to prevent mercs from going through corners of tiles and producing
								// structure data complaints)

								//pSoldier->dXPos = pSoldier->sDestXPos;
								//pSoldier->dYPos = pSoldier->sDestYPos;

								HandleBloodForNewGridNo(pSoldier);

								if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE) ||
									pSoldier->sGridNo == pSoldier->sFinalDestination)
								{
									//OK, we're at the MIDDLE of a new tile...
									HandleAtNewGridNo(pSoldier, &fKeepMoving);
								}

								if (gTacticalStatus.bBoxingState != NOT_BOXING &&
									(gTacticalStatus.bBoxingState == BOXING_WAITING_FOR_PLAYER ||
									gTacticalStatus.bBoxingState == PRE_BOXING ||
									gTacticalStatus.bBoxingState == BOXING))
								{
									BoxingMovementCheck(pSoldier);
								}

								// Are we at our final destination?
								if (pSoldier->sFinalDestination == pSoldier->sGridNo)
								{
									// Cancel path....
									pSoldier->ubPathIndex = pSoldier->ubPathDataSize = 0;

									// Cancel reverse
									pSoldier->bReverse = FALSE;

									// OK, if we are the selected soldier, refresh some UI stuff
									if (pSoldier == GetSelectedMan()) gfUIRefreshArrows = TRUE;

									// ATE: Play landing sound.....
									if (pSoldier->usAnimState == JUMP_OVER_BLOCKING_PERSON)
									{
										PlaySoldierFootstepSound(pSoldier);
									}

									// If we are a robot, play stop sound...
									if (pSoldier->uiStatusFlags & SOLDIER_ROBOT)
									{
										PlaySoldierJA2Sample(pSoldier, ROBOT_STOP, HIGHVOLUME, 1, TRUE);
									}

									// Update to middle if we're on destination
									const float dXPos = pSoldier->sDestXPos;
									const float dYPos = pSoldier->sDestYPos;
									EVENT_SetSoldierPositionXY(pSoldier, dXPos, dYPos, SSP_NONE);

									// CHECK IF WE HAVE A PENDING ANIMATION
									if (pSoldier->usPendingAnimation != NO_PENDING_ANIMATION)
									{
										ChangeSoldierState(pSoldier, pSoldier->usPendingAnimation, 0, FALSE);
										pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;

										if (pSoldier->ubPendingDirection != NO_PENDING_DIRECTION)
										{
											EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->ubPendingDirection);
											pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
										}

									}

									// CHECK IF WE HAVE A PENDING ACTION
									if (pSoldier->ubWaitActionToDo)
									{
										if (pSoldier->ubWaitActionToDo == 2)
										{
											pSoldier->ubWaitActionToDo = 1;

											if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN)
											{
												// ATE wanted this line here...
												pSoldier->ubPathIndex--;
												AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->sGridNo, (UINT8)pSoldier->uiPendingActionData1);
												continue;
											}
										}
										else if (pSoldier->ubWaitActionToDo == 1)
										{
											pSoldier->ubWaitActionToDo = 0;

											gbNumMercsUntilWaitingOver--;

											SoldierGotoStationaryStance(pSoldier);

											// If we are at an exit-grid, make disappear.....
											if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO)
											{
												RemoveSoldierFromTacticalSector(*pSoldier);
											}
										}
									}
									else if (soldier->hasPendingAction())
									{
										SLOGD("We are inside the IF PENDING Animation with soldier #{}", pSoldier->ubID);

										if (pSoldier->ubPendingAction == MERC_OPENDOOR ||
											pSoldier->ubPendingAction == MERC_OPENSTRUCT)
										{
											const INT16 sGridNo = pSoldier->sPendingActionData2;
											//usStructureID = (UINT16)pSoldier->uiPendingActionData1;
											//pStructure = FindStructureByID( sGridNo, usStructureID );

											// LOOK FOR STRUCT OPENABLE
											STRUCTURE* const pStructure = FindStructure(sGridNo, STRUCTURE_OPENABLE);
											if (pStructure == NULL)
											{
												SLOGW("Told to open struct at {} and none was found", sGridNo);
												fKeepMoving = FALSE;
											}
											else
											{
												if (EnoughPoints(pSoldier, AP_OPEN_DOOR, BP_OPEN_DOOR, TRUE))
												{
													// avoid several problems due to a lack of global action queueing
													if (DialogueQueueIsEmptyAndNobodyIsTalking() && gCurrentUIMode != LOCKUI_MODE && !soldier->anyoneHasPendingAction(MERC_GIVEITEM) && !gTacticalStatus.fAutoBandageMode && !gTacticalStatus.fAutoBandagePending)
													{
														InteractWithOpenableStruct(*pSoldier, *pStructure, pSoldier->bPendingActionData3);
													}
													else
													{
														SLOGD("Aborting pending action due to other ongoing activities!");
														fKeepMoving = FALSE;
														soldier->removePendingAnimation();
													}
												}
												else
												{
													fNoAPsForPendingAction = TRUE;
												}
											}
										}

										if(soldier->handlePendingAction(gTacticalStatus.uiFlags & INCOMBAT))
										{
											continue;
										}

										if (fNoAPsForPendingAction)
										{
											// Change status of guy to waiting
											HaltMoveForSoldierOutOfPoints(*pSoldier);
											fKeepMoving = FALSE;
											pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
											pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
										}
									}
									else
									{
										// OK, ADJUST TO STANDING, WE ARE DONE
										// DO NOTHING IF WE ARE UNCONSCIOUS
										if (pSoldier->bLife >= OKLIFE)
										{
											if (pSoldier->ubBodyType == CROW)
											{
												// If we are flying, don't stop!
												if (pSoldier->sHeightAdjustment == 0)
												{
													SoldierGotoStationaryStance(pSoldier);
												}
											}
											else
											{
												UnSetUIBusy(pSoldier);
												SoldierGotoStationaryStance(pSoldier);
											}
										}
									}

									// RESET MOVE FAST FLAG
									if (pSoldier->ubProfile == NO_PROFILE)
									{
										pSoldier->fUIMovementFast = FALSE;
									}

									// if AI moving and waiting to process something at end of
									// move, have them handled the very next frame
									if (pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST)
									{
										pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;
									}

									fKeepMoving = FALSE;
								}
								else if (!pSoldier->fNoAPToFinishMove)
								{
									// Increment path....
									pSoldier->ubPathIndex++;
									if (pSoldier->ubPathIndex > pSoldier->ubPathDataSize)
									{
										pSoldier->ubPathIndex = pSoldier->ubPathDataSize;
									}

									// Are we at the end?
									if (pSoldier->ubPathIndex == pSoldier->ubPathDataSize)
									{
										// ATE: Pop up warning....
										if (pSoldier->ubPathDataSize != MAX_PATH_LIST_SIZE)
										{
											SLOGD("Path for {} ( {} ) did not make merc get to dest.",
												pSoldier->name, pSoldier->ubID);
										}

										// In case this is an AI person with the path-stored flag set,
										// turn it OFF since we have exhausted our stored path
										pSoldier->bPathStored = FALSE;
										if (pSoldier->sAbsoluteFinalDestination != NOWHERE)
										{
											// We have not made it to our dest... but it's better to let the AI handle this itself,
											// on the very next fram
											pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;
										}
										else
										{
											// ATE: Added this to fcalilitate the fact
											// that our final dest may now have people on it....
											if (FindBestPath(pSoldier, pSoldier->sFinalDestination, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, PATH_THROUGH_PEOPLE) != 0)
											{
												const INT16 sNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(guiPathingData[0]));
												SetDelayedTileWaiting(pSoldier, sNewGridNo, 1);
											}

											// We have not made it to our dest... set flag that we are waiting....
											if (!EVENT_InternalGetNewSoldierPath(pSoldier, pSoldier->sFinalDestination, pSoldier->usUIMovementMode, 2, FALSE))
											{
												// ATE: To do here.... we could not get path, so we have to stop
												SoldierGotoStationaryStance(pSoldier);
												continue;
											}
										}
									}
									else
									{
										// OK, Now find another dest grindo....
										if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE))
										{
											// OK, now we want to see if we can continue to another tile...
											if (!HandleGotoNewGridNo(pSoldier, &fKeepMoving, FALSE, pSoldier->usAnimState))
											{
												continue;
											}
										}
										else
										{
											// Change desired direction
											// Just change direction
											EVENT_InternalSetSoldierDestination(pSoldier, pSoldier->ubPathingData[pSoldier->ubPathIndex], FALSE, pSoldier->usAnimState);
										}

										if (gTacticalStatus.bBoxingState != NOT_BOXING &&
											(gTacticalStatus.bBoxingState == BOXING_WAITING_FOR_PLAYER ||
											gTacticalStatus.bBoxingState == PRE_BOXING ||
											gTacticalStatus.bBoxingState == BOXING))
										{
											BoxingMovementCheck(pSoldier);
										}
									}
								}
							}

							if (pSoldier->uiStatusFlags & SOLDIER_PAUSEANIMOVE)
							{
								fKeepMoving = FALSE;
							}

							// DO WALKING
							if (!pSoldier->fPausedMove && fKeepMoving)
							{
								// Determine deltas
								//dDeltaX = pSoldier->sDestXPos - pSoldier->dXPos;
								//dDeltaY = pSoldier->sDestYPos - pSoldier->dYPos;

								// Determine angle
								//dAngle = (FLOAT)atan2( dDeltaX, dDeltaY );

								static const float gdRadiansForAngle[] =
								{
									(float)PI,
									(float)(PI * 3 / 4),
									(float)(PI     / 2),
									(float)(PI     / 4),

									0,
									(float)(-PI     / 4),
									(float)(-PI     / 2),
									(float)(-PI * 3 / 4),
								};
								const float dAngle = gdRadiansForAngle[pSoldier->bMovementDirection];

								// For walking, base it on body type!
								if (pSoldier->usAnimState == WALKING)
								{
									MoveMerc(pSoldier, gubAnimWalkSpeeds[pSoldier->ubBodyType].dMovementChange, dAngle, TRUE);
								}
								else
								{
									MoveMerc(pSoldier, gAnimControl[pSoldier->usAnimState].dMovementChange, dAngle, TRUE);
								}
							}
						}

						// Check for direction change
						if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_TURNING)
						{
							TurnSoldier(pSoldier);
						}
					}
				}

				if (!gfPauseAllAI && (
					(gTacticalStatus.uiFlags & INCOMBAT) ||
					(fHandleAI && guiAISlotToHandle == cnt) ||
					pSoldier->fAIFlags & AI_HANDLE_EVERY_FRAME ||
					gTacticalStatus.fAutoBandageMode))
				{
					HandleSoldierAI(pSoldier);
					if (!(gTacticalStatus.uiFlags & INCOMBAT))
					{
						if (GetJA2Clock() - uiTimerVal > RT_AI_TIMESLICE)
						{
							// don't do any more AI this time!
							fHandleAI = FALSE;
						}
						else
						{
							// we still have time to handle AI; skip to the next person
							fHandleAI = NextAIToHandle(guiAISlotToHandle);
						}
					}
				}
			}
		}

		if (guiNumAwaySlots > 0 &&
				!gfPauseAllAI &&
				!(gTacticalStatus.uiFlags & INCOMBAT) &&
				guiAISlotToHandle == HANDLE_OFF_MAP_MERC
				&& guiAIAwaySlotToHandle != RESET_HANDLE_OF_OFF_MAP_MERCS)
		{
			// Syncronize for upcoming soldier counters
			SYNCTIMECOUNTER();

			// the ONLY thing to do with away soldiers is process their schedule if they have one
			// and there is an action for them to do (like go on-sector)
			SOLDIERTYPE* const pSoldier = AwaySlots[guiAIAwaySlotToHandle];
			if (pSoldier != NULL && pSoldier->fAIFlags & AI_CHECK_SCHEDULE)
			{
				HandleSoldierAI(pSoldier);
			}
		}

		// Turn off auto bandage if we need to...
		if (gTacticalStatus.fAutoBandageMode && !CanAutoBandage(TRUE))
		{
			SetAutoBandageComplete();
		}

		// Check if we should be doing a special event once guys get to a location...
		if (gubWaitingForAllMercsToExitCode != 0)
		{
			// Check if we have gone past our time...
			if (GetJA2Clock() - guiWaitingForAllMercsToExitTimer > 2500)
			{
				// OK, set num waiting to 0
				SLOGD("Waiting too long for Mercs to exit...forcing entry.");
				gbNumMercsUntilWaitingOver = 0;

				// Reset all waitng codes
				FOR_EACH_MERC(i) (*i)->ubWaitActionToDo = 0;
			}

			if (gbNumMercsUntilWaitingOver == 0)
			{
				// ATE: Unset flag to ignore sight...
				gTacticalStatus.uiFlags &= ~DISALLOW_SIGHT;

				// OK cheif, do something here....
				switch (gubWaitingForAllMercsToExitCode)
				{
					case WAIT_FOR_MERCS_TO_WALKOFF_SCREEN:
						if (gTacticalStatus.ubCurrentTeam == OUR_TEAM)
						{
							guiPendingOverrideEvent = LU_ENDUILOCK;
							HandleTacticalUI();
						}
						AllMercsHaveWalkedOffSector();
						break;

					case WAIT_FOR_MERCS_TO_WALKON_SCREEN:
						// OK, unset UI
						if (gTacticalStatus.ubCurrentTeam == OUR_TEAM)
						{
							guiPendingOverrideEvent = LU_ENDUILOCK;
							HandleTacticalUI();
						}
						break;

					case WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO:
						// OK, unset UI
						if (gTacticalStatus.ubCurrentTeam == OUR_TEAM)
						{
							guiPendingOverrideEvent = LU_ENDUILOCK;
							HandleTacticalUI();
						}
						AllMercsWalkedToExitGrid();
						break;
				}

				// ATE; Turn off tactical status flag...
				gTacticalStatus.uiFlags         &= ~IGNORE_ALL_OBSTACLES;
				gubWaitingForAllMercsToExitCode  = 0;
			}
		}
	}

	// reset these AI-related global variables to 0 to ensure they don't interfere with the UI
	gubNPCAPBudget  = 0;
	gubNPCDistLimit = 0;
}


static void HaltGuyFromNewGridNoBecauseOfNoAPs(SOLDIERTYPE& s)
{
	SoldierSP soldier = GetSoldier(&s);

	HaltMoveForSoldierOutOfPoints(s);
	soldier->removePendingAnimation();

	UnMarkMovementReserved(s);

	// Display message if our merc
	if (s.bTeam == OUR_TEAM && gTacticalStatus.uiFlags & INCOMBAT)
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[GUY_HAS_RUN_OUT_OF_APS_STR], s.name));
	}

	UnSetUIBusy(&s);
	// Unset engaged in CONV, something changed
	UnSetEngagedInConvFromPCAction(&s);
}


static void HandleLocateToGuyAsHeWalks(SOLDIERTYPE* pSoldier)
{
	// Our guys if option set,
	if (pSoldier->bTeam == OUR_TEAM)
	{
		// IF tracking on, center on guy....
		if (gGameSettings.fOptions[TOPTION_TRACKING_MODE])
		{
			LocateSoldier(pSoldier, FALSE);
		}
	}
	else
	{
		// Others if visible...
		if (pSoldier->bVisible != -1)
		{
			// ATE: If we are visible, and have not already removed roofs, goforit
			if (pSoldier->bLevel > 0)
			{
				if (!(gTacticalStatus.uiFlags & SHOW_ALL_ROOFS))
				{
					gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
					SetRenderFlags(RENDER_FLAG_FULL);
				}
			}
			LocateSoldier(pSoldier, FALSE);
		}
	}
}


static void CheckIfNearbyGroundSeemsWrong(SOLDIERTYPE* const s, UINT16 const gridno, BOOLEAN const check_around, BOOLEAN* const keep_moving)
{
	INT16 mine_gridno;
	if (!NearbyGroundSeemsWrong(s, gridno, check_around, &mine_gridno)) return;

	if (s->uiStatusFlags & SOLDIER_PC)
	{
		// NearbyGroundSeemsWrong() returns true with gridno NOWHERE if we find
		// something by metal detector.  We should definitely stop but we won't
		// place a locator or say anything
		if (gTacticalStatus.uiFlags & INCOMBAT)
		{
			EVENT_StopMerc(s);
		}
		else
		{
			// Not in combat, stop them all
			for (INT32 i = gTacticalStatus.Team[OUR_TEAM].bLastID; i >= gTacticalStatus.Team[OUR_TEAM].bFirstID; i--)
			{
				SOLDIERTYPE& s2 = GetMan(i);
				if (!s2.bActive) continue;
				EVENT_StopMerc(&s2);
			}
		}

		*keep_moving = FALSE;

		if (mine_gridno == NOWHERE) return;

		// We reuse the boobytrap gridno variable here
		gpBoobyTrapSoldier = s;
		gsBoobyTrapGridNo  = mine_gridno;
		LocateGridNo(mine_gridno);
		SetStopTimeQuoteCallback(MineSpottedDialogueCallBack);
		TacticalCharacterDialogue(s, QUOTE_SUSPICIOUS_GROUND);
	}
	else
	{
		if (mine_gridno == NOWHERE) return;

		EVENT_StopMerc(s);
		*keep_moving = FALSE;

		gpWorldLevelData[mine_gridno].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;

		// Better stop and reconsider what to do
		SetNewSituation(s);
		ActionDone(s);
	}
}


BOOLEAN HandleGotoNewGridNo(SOLDIERTYPE* pSoldier, BOOLEAN* pfKeepMoving, BOOLEAN fInitialMove, UINT16 usAnimState)
{
	if (gTacticalStatus.uiFlags & INCOMBAT && fInitialMove)
	{
		HandleLocateToGuyAsHeWalks(pSoldier);
	}

	// Default to TRUE
	*pfKeepMoving = TRUE;

	// Check for good breath....
	// if ( pSoldier->bBreath < OKBREATH && !fInitialMove )
	if (pSoldier->bBreath < OKBREATH)
	{
		// OK, first check for b== 0
		// If our currentd state is moving already....( this misses the first tile, so the user
		// Sees some change in their click, but just one tile
		if (pSoldier->bBreath == 0)
		{
			// Collapse!
			pSoldier->bBreathCollapsed = TRUE;
			pSoldier->bEndDoorOpenCode = FALSE;

			if (fInitialMove) UnSetUIBusy(pSoldier);

			SLOGD("HandleGotoNewGridNo() Failed: Out of Breath");
			return FALSE;
		}

		// OK, if we are collapsed now, check for OK breath instead...
		if (pSoldier->bCollapsed)
		{
			// Collapse!
			SLOGD("HandleGotoNewGridNo() Failed: Has Collapsed");
			pSoldier->bBreathCollapsed = TRUE;
			pSoldier->bEndDoorOpenCode = FALSE;
			return FALSE;
		}

	}

	const UINT16 usNewGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(pSoldier->ubPathingData[pSoldier->ubPathIndex]));

	// OK, check if this is a fence cost....
	if (gubWorldMovementCosts[usNewGridNo][pSoldier->ubPathingData[pSoldier->ubPathIndex]][pSoldier->bLevel] == TRAVELCOST_FENCE)
	{
		// We have been told to jump fence....

		// Do we have APs?
		const INT16 sAPCost = AP_JUMPFENCE;
		const INT16 sBPCost = BP_JUMPFENCE;

		if (EnoughPoints(pSoldier, sAPCost, sBPCost, FALSE))
		{
			// ATE: Check for tile being clear....
			const UINT16 sOverFenceGridNo = NewGridNo(usNewGridNo, DirectionInc(pSoldier->ubPathingData[pSoldier->ubPathIndex + 1]));

			if (HandleNextTile(pSoldier, (INT8)pSoldier->ubPathingData[pSoldier->ubPathIndex + 1], sOverFenceGridNo, pSoldier->sFinalDestination))
			{
				// We do, adjust path data....
				pSoldier->ubPathIndex++;
				// We go two, because we really want to start moving towards the NEXT gridno,
				// if we have any...

				// LOCK PENDING ACTION COUNTER
				pSoldier->uiStatusFlags |= SOLDIER_LOCKPENDINGACTIONCOUNTER;

				SoldierGotoStationaryStance(pSoldier);

				// OK, jump!
				BeginSoldierClimbFence(pSoldier);

				pSoldier->fContinueMoveAfterStanceChange = 2;
			}
		}
		else
		{
			HaltGuyFromNewGridNoBecauseOfNoAPs(*pSoldier);
			*pfKeepMoving = FALSE;
		}

		return FALSE;
	}
	else if (InternalDoorTravelCost(pSoldier, usNewGridNo, gubWorldMovementCosts[usNewGridNo][pSoldier->ubPathingData[pSoldier->ubPathIndex]][pSoldier->bLevel], pSoldier->bTeam == OUR_TEAM, NULL, TRUE) == TRAVELCOST_DOOR)
	{
		// OK, if we are here, we have been told to get a pth through a door.

		// No need to check if for AI

		// No need to check for right key ( since the path checks for that? )

		// Just for now play the $&&% animation
		const INT8 bDirection = pSoldier->ubPathingData[pSoldier->ubPathIndex];

		// OK, based on the direction, get door gridno
		INT16 sDoorGridNo;
		if (bDirection == NORTH || bDirection == WEST)
		{
			sDoorGridNo = NewGridNo(pSoldier->sGridNo, DirectionInc(pSoldier->ubPathingData[pSoldier->ubPathIndex]));
		}
		else if (bDirection == SOUTH || bDirection == EAST)
		{
			sDoorGridNo = pSoldier->sGridNo;
		}
		else
		{
			SLOGD("HandleGotoNewGridNo() Failed: Open door - invalid approach direction");

			HaltGuyFromNewGridNoBecauseOfNoAPs(*pSoldier);
			pSoldier->bEndDoorOpenCode = FALSE;
			(*pfKeepMoving ) = FALSE;
			return( FALSE );
		}

		// Get door
		STRUCTURE* const pStructure = FindStructure(sDoorGridNo, STRUCTURE_ANYDOOR);
		if (pStructure == NULL)
		{
			SLOGD("HandleGotoNewGridNo() Failed: Door does not exist");
			HaltGuyFromNewGridNoBecauseOfNoAPs(*pSoldier);
			pSoldier->bEndDoorOpenCode = FALSE;
			*pfKeepMoving = FALSE;
			return FALSE;
		}

		// OK, open!
		StartInteractiveObject(sDoorGridNo, *pStructure, *pSoldier, bDirection);
		InteractWithOpenableStruct(*pSoldier, *pStructure, bDirection);

		// One needs to walk after....
		if (pSoldier->bTeam != OUR_TEAM || gTacticalStatus.fAutoBandageMode || pSoldier->uiStatusFlags & SOLDIER_PCUNDERAICONTROL)
		{
			pSoldier->bEndDoorOpenCode     = 1;
			pSoldier->sEndDoorOpenCodeData = sDoorGridNo;
		}
		*pfKeepMoving = FALSE;
		return FALSE;
	}

	// Find out how much it takes to move here!
	const INT16 sAPCost = ActionPointCost(    pSoldier, usNewGridNo, (INT8)pSoldier->ubPathingData[pSoldier->ubPathIndex], usAnimState);
	const INT16 sBPCost = TerrainBreathPoints(pSoldier, usNewGridNo, (INT8)pSoldier->ubPathingData[pSoldier->ubPathIndex], usAnimState);

	// CHECK IF THIS TILE IS A GOOD ONE!
	if (!HandleNextTile(pSoldier, (INT8)pSoldier->ubPathingData[pSoldier->ubPathIndex], usNewGridNo, pSoldier->sFinalDestination))
	{
		SLOGD("HandleGotoNewGridNo() Failed: Tile {} Was blocked", usNewGridNo);

		// ATE: If our own guy and an initial move.. display message
		//if ( fInitialMove && pSoldier->bTeam == OUR_TEAM  )
		//{
		//	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NO_PATH_FOR_MERC ], pSoldier->name) );
		//}

		pSoldier->bEndDoorOpenCode = FALSE;
		// GO on to next guy!
		return FALSE;
	}

	// just check the tile we're going to walk into
	CheckIfNearbyGroundSeemsWrong(pSoldier, usNewGridNo, FALSE, pfKeepMoving);

	// ATE: Check if we have sighted anyone, if so, don't do anything else...
	// IN other words, we have stopped from sighting...
	if (pSoldier->fNoAPToFinishMove && !fInitialMove)
	{
		SLOGD("HandleGotoNewGridNo() Failed: No APs to finish move set");
		pSoldier->bEndDoorOpenCode = FALSE;
		*pfKeepMoving = FALSE;
	}
	else if (pSoldier->ubPathIndex == pSoldier->ubPathDataSize && pSoldier->ubPathDataSize == 0)
	{
		SLOGD("HandleGotoNewGridNo() Failed: No Path");
		pSoldier->bEndDoorOpenCode = FALSE;
		*pfKeepMoving = FALSE;
	}
	//else if ( gTacticalStatus.fEnemySightingOnTheirTurn )
	//{
	//	// Hault guy!
	//	AdjustNoAPToFinishMove( pSoldier, TRUE );
	//	(*pfKeepMoving ) = FALSE;
	//}
	else if (EnoughPoints(pSoldier, sAPCost, 0, FALSE))
	{
		BOOLEAN fDontContinue = FALSE;

		if (pSoldier->ubPathIndex > 0)
		{
			// check for running into gas

			// note: this will have to use the minimum types of structures for tear/creature gas
			// since there isn't a way to retrieve the smoke effect structure
			if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] & ANY_SMOKE_EFFECT && PreRandom(5) == 0)
			{
				INT8 bPosOfMask;
				if (pSoldier->inv[HEAD1POS].usItem == GASMASK && pSoldier->inv[HEAD1POS].bStatus[0] >= GASMASK_MIN_STATUS)
				{
					bPosOfMask = HEAD1POS;
				}
				else if (pSoldier->inv[HEAD2POS].usItem == GASMASK && pSoldier->inv[HEAD2POS].bStatus[0] >= GASMASK_MIN_STATUS)
				{
					bPosOfMask = HEAD2POS;
				}
				else
				{
					bPosOfMask = NO_SLOT;
				}

				EXPLOSIVETYPE const* pExplosive = 0;
				if (!AM_A_ROBOT(pSoldier))
				{
					if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] & MAPELEMENT_EXT_TEARGAS)
					{
						if (!(pSoldier->fHitByGasFlags & HIT_BY_TEARGAS) && bPosOfMask == NO_SLOT)
						{
							// check for gas mask
							pExplosive = &Explosive[GCM->getItem(TEARGAS_GRENADE)->getClassIndex()];
						}
					}
					if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] & MAPELEMENT_EXT_MUSTARDGAS)
					{
						if (!(pSoldier->fHitByGasFlags & HIT_BY_MUSTARDGAS) && bPosOfMask == NO_SLOT)
						{
							pExplosive = &Explosive[GCM->getItem(MUSTARD_GRENADE)->getClassIndex()];
						}
					}
				}
				if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] & MAPELEMENT_EXT_CREATUREGAS)
				{
					if (!(pSoldier->fHitByGasFlags & HIT_BY_CREATUREGAS)) // gas mask doesn't help vs creaturegas
					{
						pExplosive = &Explosive[GCM->getItem(SMALL_CREATURE_GAS)->getClassIndex()];
					}
				}
				if (pExplosive)
				{
					EVENT_StopMerc(pSoldier);
					fDontContinue = TRUE;
					DishOutGasDamage(pSoldier, pExplosive, TRUE, FALSE, pExplosive->ubDamage + PreRandom(pExplosive->ubDamage), 100 * (pExplosive->ubStunDamage + PreRandom(pExplosive->ubStunDamage / 2)), NULL);
				}
			}

			if (!fDontContinue)
			{

				if ((pSoldier->bOverTerrainType == FLAT_FLOOR || pSoldier->bOverTerrainType == PAVED_ROAD) &&
					pSoldier->bLevel == 0)
				{
					INT32 iMarblesIndex;
					if (ItemTypeExistsAtLocation(pSoldier->sGridNo, MARBLES, 0, &iMarblesIndex))
					{
						// Slip on marbles!
						DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
						if (pSoldier->bTeam == OUR_TEAM)
						{
							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(g_langRes->Message[STR_SLIPPED_MARBLES], pSoldier->name));
						}
						RemoveItemFromPool(GetWorldItem(iMarblesIndex));
						SoldierCollapse(pSoldier);
						if (pSoldier->bActionPoints > 0)
						{
							pSoldier->bActionPoints -= (INT8)(Random(pSoldier->bActionPoints) + 1);
						}
						return FALSE;
					}
				}

				if (pSoldier->bBlindedCounter > 0 &&
					pSoldier->usAnimState == RUNNING &&
					Random(5) == 0 &&
					OKFallDirection(pSoldier, pSoldier->sGridNo + DirectionInc(pSoldier->bDirection), pSoldier->bLevel, pSoldier->bDirection, pSoldier->usAnimState))
				{
					// 20% chance of falling over!
					DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_37], pSoldier->name));
					SoldierCollapse(pSoldier);
					if (pSoldier->bActionPoints > 0)
					{
						pSoldier->bActionPoints -= (INT8)(Random(pSoldier->bActionPoints) + 1);
					}
					return FALSE;
				}
				else if (GetDrunkLevel(pSoldier) == DRUNK &&
					(Random(5) == 0) &&
					OKFallDirection(pSoldier, pSoldier->sGridNo + DirectionInc(pSoldier->bDirection), pSoldier->bLevel, pSoldier->bDirection, pSoldier->usAnimState))
				{
					// 20% chance of falling over!
					DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_37], pSoldier->name));
					SoldierCollapse(pSoldier);
					if (pSoldier->bActionPoints > 0)
					{
						pSoldier->bActionPoints -= (INT8)(Random(pSoldier->bActionPoints) + 1);
					}
					return FALSE;
				}
				else if (pSoldier->bTeam == OUR_TEAM &&
					pSoldier->ubProfile != NO_PROFILE &&
					gMercProfiles[pSoldier->ubProfile].bPersonalityTrait == FORGETFUL)
				{
					// ATE; First check for profile
					// Forgetful guy might forget his path
					if (pSoldier->ubNumTilesMovesSinceLastForget < 255)
					{
						pSoldier->ubNumTilesMovesSinceLastForget++;
					}

					if (pSoldier->ubPathIndex > 2 && Random(100 ) == 0 && pSoldier->ubNumTilesMovesSinceLastForget > 200)
					{
						pSoldier->ubNumTilesMovesSinceLastForget = 0;

						TacticalCharacterDialogue(pSoldier, QUOTE_PERSONALITY_TRAIT);
						EVENT_StopMerc(pSoldier);
						if (pSoldier->bActionPoints > 0)
						{
							pSoldier->bActionPoints -= (INT8)(Random(pSoldier->bActionPoints) + 1);
						}

						fDontContinue = TRUE;
						UnSetUIBusy(pSoldier);
					}
				}
			}
		}

		if (!fDontContinue)
		{
			// Don't apply the first deduction in points...
			if (usAnimState == CRAWLING && pSoldier->fTurningFromPronePosition > 1)
			{
			}
			else
			{
				// Adjust AP/Breathing points to move
				DeductPoints(pSoldier, sAPCost, sBPCost);
			}

			// OK, let's check for monsters....
			if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
			{
				if (!ValidCreatureTurn(pSoldier, (INT8)pSoldier->ubPathingData[pSoldier->ubPathIndex]))
				{
					if (!pSoldier->bReverse)
					{
						pSoldier->bReverse = TRUE;

						if (pSoldier->ubBodyType == INFANT_MONSTER)
						{
							ChangeSoldierState(pSoldier, WALK_BACKWARDS, 1, TRUE);
						}
						else
						{
							ChangeSoldierState(pSoldier, MONSTER_WALK_BACKWARDS, 1, TRUE);
						}
					}
				}
				else
				{
					pSoldier->bReverse = FALSE;
				}
			}

			// OK, let's check for monsters....
			if (pSoldier->ubBodyType == BLOODCAT)
			{
				if (!ValidCreatureTurn(pSoldier, (INT8)pSoldier->ubPathingData[pSoldier->ubPathIndex]))
				{
					if (!pSoldier->bReverse)
					{
						pSoldier->bReverse = TRUE;
						ChangeSoldierState(pSoldier, BLOODCAT_WALK_BACKWARDS, 1, TRUE);
					}
				}
				else
				{
					pSoldier->bReverse = FALSE;
				}
			}

			// Change desired direction
			EVENT_InternalSetSoldierDestination(pSoldier, pSoldier->ubPathingData[pSoldier->ubPathIndex], fInitialMove, usAnimState);

			// CONTINUE
			// IT'S SAVE TO GO AGAIN, REFRESH flag
			AdjustNoAPToFinishMove(pSoldier, FALSE);
		}
	}
	else
	{
		// HALT GUY HERE
		SLOGD("HandleGotoNewGridNo() Failed: No APs {} {}", sAPCost, pSoldier->bActionPoints);
		HaltGuyFromNewGridNoBecauseOfNoAPs(*pSoldier);
		pSoldier->bEndDoorOpenCode = FALSE;
		*pfKeepMoving = FALSE;
	}

	return TRUE;
}


static void HandleMaryArrival(SOLDIERTYPE* pSoldier)
{
	if (!pSoldier)
	{
		pSoldier = FindSoldierByProfileIDOnPlayerTeam(MARY);
		if (!pSoldier) return;
	}

	if (CheckFact( FACT_JOHN_ALIVE, 0)) return;

	// new requirements: player close by
	else if (PythSpacesAway(pSoldier->sGridNo, 8228) < 40)
	{
		INT16 sDist;
		if (ClosestPC(pSoldier, &sDist) != NOWHERE && sDist > NPC_TALK_RADIUS * 2)
		{
			// too far away
			return;
		}

		// Mary has arrived
		SetFactTrue(FACT_MARY_OR_JOHN_ARRIVED);
		EVENT_StopMerc(pSoldier);
		TriggerNPCRecord(MARY, 13);
	}
}


static void HandleJohnArrival(SOLDIERTYPE* pSoldier)
{
	if (!pSoldier)
	{
		pSoldier = FindSoldierByProfileIDOnPlayerTeam(JOHN);
		if (!pSoldier) return;
	}

	if (PythSpacesAway(pSoldier->sGridNo, 8228) < 40)
	{
		INT16 sDist;
		if (ClosestPC(pSoldier, &sDist) != NOWHERE && sDist > NPC_TALK_RADIUS * 2)
		{
			// too far away
			return;
		}

		SOLDIERTYPE* pSoldier2 = NULL;
		if (CheckFact(FACT_MARY_ALIVE, 0))
		{
			pSoldier2 = FindSoldierByProfileID(MARY);
			if (pSoldier2)
			{
				if (PythSpacesAway(pSoldier->sGridNo, pSoldier2->sGridNo) > 8)
				{
					// too far away!
					return;
				}
			}
		}

		SetFactTrue(FACT_MARY_OR_JOHN_ARRIVED);

		EVENT_StopMerc(pSoldier);

		// if Mary is alive/dead
		if (pSoldier2)
		{
			EVENT_StopMerc(pSoldier2);
			TriggerNPCRecord(JOHN, 13);
		}
		else
		{
			TriggerNPCRecord(JOHN, 12);
		}
	}
}


static BOOLEAN HandleAtNewGridNo(SOLDIERTYPE* pSoldier, BOOLEAN* pfKeepMoving)
{
	SoldierSP soldier = GetSoldier(pSoldier);

	// ATE; Handle bad guys, as they fade, to cancel it if
	// too long...
	// ONLY if fading IN!
	if (pSoldier->fBeginFade == 1)
	{
		if (pSoldier->sLocationOfFadeStart != pSoldier->sGridNo)
		{
			// Turn off
			pSoldier->fBeginFade = FALSE;

			if (pSoldier->bLevel > 0 && gpWorldLevelData[pSoldier->sGridNo].pRoofHead != NULL)
			{
				pSoldier->ubFadeLevel = gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubShadeLevel;
			}
			else
			{
				pSoldier->ubFadeLevel = gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;
			}

			// Set levelnode shade level....
			if (pSoldier->pLevelNode)
			{
				pSoldier->pLevelNode->ubShadeLevel = pSoldier->ubFadeLevel;
			}
			pSoldier->bVisible = -1;
		}
	}

	if (gTacticalStatus.uiFlags & INCOMBAT) HandleLocateToGuyAsHeWalks(pSoldier);

	// Default to TRUE
	*pfKeepMoving = TRUE;

	pSoldier->bTilesMoved++;
	if (pSoldier->usAnimState == RUNNING)
	{
		// count running as double
		pSoldier->bTilesMoved++;
	}

	// First if we are in noncombat (AKA realtime)
	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		// Update value for RT breath update
		pSoldier->ubTilesMovedPerRTBreathUpdate++;
		// Update last anim
		pSoldier->usLastMovementAnimPerRTBreathUpdate = pSoldier->usAnimState;
	}

	// Update path if showing path in RT
	if (gGameSettings.fOptions[TOPTION_ALWAYS_SHOW_MOVEMENT_PATH] &&
		!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		gfPlotNewMovement = TRUE;
	}

	// ATE: Put some stuff in here to not handle certain things if we are
	// trversing...
	if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN ||
		gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO)
	{
		return TRUE;
	}

	// Check if they are out of breath
	if (CheckForBreathCollapse(*pSoldier))
	{
		*pfKeepMoving = TRUE;
		return FALSE;
	}

	// see if a mine gets set off...
	if (SetOffBombsInGridNo(pSoldier, pSoldier->sGridNo, FALSE, pSoldier->bLevel))
	{
		*pfKeepMoving = FALSE;
		EVENT_StopMerc(pSoldier);
		return FALSE;
	}


	// Set "interrupt occurred" flag to false so that we will know whether *this
	// particular call* to HandleSight caused an interrupt
	gTacticalStatus.fInterruptOccurred = FALSE;

	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		const UINT8 ubVolume = MovementNoise(pSoldier);
		if (ubVolume > 0)
		{
			MakeNoise(pSoldier, pSoldier->sGridNo, pSoldier->bLevel, ubVolume, NOISE_MOVEMENT);
			if (pSoldier->uiStatusFlags & SOLDIER_PC && pSoldier->bStealthMode)
			{
				PlayStealthySoldierFootstepSound(pSoldier);
			}
		}
	}

	// ATE: Make sure we don't make another interrupt...
	if (!gTacticalStatus.fInterruptOccurred)
	{
		// Handle New sight
		HandleSight(*pSoldier, SIGHT_LOOK | SIGHT_RADIO | SIGHT_INTERRUPT);
	}

	// ATE: Check if we have sighted anyone, if so, don't do anything else...
	// IN other words, we have stopped from sighting...
	if (gTacticalStatus.fInterruptOccurred)
	{
		// Unset no APs value
		AdjustNoAPToFinishMove(pSoldier, TRUE);

		*pfKeepMoving = FALSE;
		pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
		pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;

		// ATE: Cancel only if our final destination
		if (pSoldier->sGridNo == pSoldier->sFinalDestination)
		{
			soldier->removePendingAction();
		}

		// this flag is set only to halt the currently moving guy; reset it now
		gTacticalStatus.fInterruptOccurred = FALSE;

		// ATE: Remove this if we were stopped....
		if (gTacticalStatus.fEnemySightingOnTheirTurn)
		{
			if (gTacticalStatus.enemy_sighting_on_their_turn_enemy == pSoldier)
			{
				pSoldier->fPauseAllAnimation = FALSE;
				gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
			}
		}
	}
	else if (pSoldier->fNoAPToFinishMove)
	{
		*pfKeepMoving = FALSE;
	}
	else if (pSoldier->ubPathIndex == pSoldier->ubPathDataSize && pSoldier->ubPathDataSize == 0)
	{
		*pfKeepMoving = FALSE;
	}
	else if (gTacticalStatus.fEnemySightingOnTheirTurn)
	{
		// Hault guy!
		AdjustNoAPToFinishMove(pSoldier, TRUE);
		*pfKeepMoving = FALSE;
	}

	// OK, check for other stuff like mines...
	CheckIfNearbyGroundSeemsWrong(pSoldier, pSoldier->sGridNo, TRUE, pfKeepMoving);

	HandleSystemNewAISituation(pSoldier);

	if (pSoldier->bTeam == OUR_TEAM)
	{
		static const SGPSector drassen(13, MAP_ROW_B);
		if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC)
		{
			// are we there yet?
			static const SGPSector drassenAirport(13, MAP_ROW_B);
			static const SGPSector sanMona(6, MAP_ROW_C);
			static const SGPSector cambria(8, MAP_ROW_G);
			if (pSoldier->sSector == drassenAirport)
			{
				switch (pSoldier->ubProfile)
				{
					case SKYRIDER:
						if (PythSpacesAway(pSoldier->sGridNo, 8842) < 11)
						{
							// Skyrider has arrived!
							EVENT_StopMerc(pSoldier);
							SetFactTrue(FACT_SKYRIDER_CLOSE_TO_CHOPPER);
							TriggerNPCRecord(SKYRIDER, 15);
							SetUpHelicopterForPlayer(drassenAirport);
						}
						break;

					case MARY: HandleMaryArrival(pSoldier); break;
					case JOHN: HandleJohnArrival(pSoldier); break;
				}
			}
			else if (pSoldier->ubProfile == MARIA &&
				pSoldier->sSector == sanMona &&
				CheckFact(FACT_MARIA_ESCORTED_AT_LEATHER_SHOP, MARIA) == TRUE)
			{
				// check that Angel is there!
				if (NPCInRoom(ANGEL, 2)) // room 2 is leather shop
				{
					//UnRecruitEPC( MARIA );
					TriggerNPCRecord(ANGEL, 12);
				}
			}
			else if (pSoldier->ubProfile == JOEY && pSoldier->sSector == cambria)
			{
				// if Joey walks near Martha then trigger Martha record 7
				if (CheckFact(FACT_JOEY_NEAR_MARTHA, 0))
				{
					EVENT_StopMerc(pSoldier);
					TriggerNPCRecord(JOEY, 9);
				}
			}

		}
		// Drassen stuff for John & Mary
		else if (gubQuest[QUEST_ESCORT_TOURISTS] == QUESTINPROGRESS &&
			pSoldier->sSector == drassen)
		{
			if (CheckFact(FACT_JOHN_ALIVE, 0))
			{
				HandleJohnArrival(NULL);
			}
			else
			{
				HandleMaryArrival(NULL);
			}
		}

	}
	else if (pSoldier->bTeam == CIV_TEAM && pSoldier->ubProfile != NO_PROFILE && pSoldier->bNeutral)
	{
		switch (pSoldier->ubProfile)
		{
			case JIM:
			case JACK:
			case OLAF:
			case RAY:
			case OLGA:
			case TYRONE:
			{
				INT16 sDesiredMercDist;
				if (ClosestPC(pSoldier, &sDesiredMercDist) != NOWHERE)
				{
					if (sDesiredMercDist <= NPC_TALK_RADIUS * 2)
					{
						// stop
						CancelAIAction(pSoldier);
						// aaaaaaaaaaaaaaaaaaaaatttaaaack!!!!
						AddToShouldBecomeHostileOrSayQuoteList(pSoldier);
						//MakeCivHostile( pSoldier, 2 );
						//TriggerNPCWithIHateYouQuote( pSoldier->ubProfile );
					}
				}
			}
				break;

			default:
				break;
		}
	}
	return( TRUE );
}


void SelectNextAvailSoldier(const SOLDIERTYPE* const last)
{
	// IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
	FOR_EACH_IN_TEAM(s, last->bTeam)
	{
		if (OkControllableMerc(s))
		{
			SelectSoldier(s, SELSOLDIER_NONE);
			return;
		}
	}

	SetSelectedMan(NULL);
	// Change UI mode to reflact that we are selected
	guiPendingOverrideEvent = I_ON_TERRAIN;
}


void SelectSoldier(SOLDIERTYPE* const s, const SelSoldierFlags flags)
{
	// ARM: can't call SelectSoldier() in mapscreen, that will initialize interface panels!!!
	// ATE: Adjusted conditions a bit ( sometimes were not getting selected )
	if (guiCurrentScreen == LAPTOP_SCREEN || guiCurrentScreen == MAP_SCREEN) return;

	//if we are in the shop keeper interface
	if (guiCurrentScreen == SHOPKEEPER_SCREEN)
	{
		//dont allow the player to change the selected merc
		return;
	}

	// If we are dead, ignore
	if (!OK_CONTROLLABLE_MERC(s)) return;

	// Don't do it if we don't have an interrupt
	if (!OK_INTERRUPT_MERC(s))
	{
		// OK, we want to display message that we can't....
		if (flags & SELSOLDIER_FROM_UI)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(TacticalStr[MERC_IS_UNAVAILABLE_STR], s->name));
		}
		return;
	}

	SOLDIERTYPE* const old_sel = GetSelectedMan();
	if (s == old_sel && !(flags & SELSOLDIER_FORCE_RESELECT)) return;

	// Unselect old selected guy
	if (old_sel != NULL)
	{
		old_sel->fShowLocator  = FALSE;
		old_sel->fFlashLocator = FALSE;

		// DB This used to say "s"... I fixed it
		if (old_sel->bLevel == 0)
		{
			//ApplyTranslucencyToWalls((INT16)(old_sel->dXPos / CELL_X_SIZE), (INT16)(old_sel->dYPos / CELL_Y_SIZE));
		}
		//DeleteSoldierLight(old_sel);

		UpdateForContOverPortrait(old_sel, FALSE);
	}

	SetSelectedMan(s);

	// find which squad this guy is, then set selected squad to this guy
	SetCurrentSquad(s->bAssignment, FALSE);

	if (s->bLevel == 0)
	{
		//CalcTranslucentWalls( s->dXPos / CELL_X_SIZE, s->dYPos / CELL_Y_SIZE);
	}

	// Set interface to reflect new selection!
	SetCurrentTacticalPanelCurrentMerc(s);

	// PLay ATTN SOUND
	if (flags & SELSOLDIER_ACKNOWLEDGE && !gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS])
	{
		DoMercBattleSound(s, BATTLE_SOUND_ATTN1);
	}

	// Change UI mode to reflact that we are selected
	// NOT if we are locked inthe UI
	if (gTacticalStatus.ubCurrentTeam == OUR_TEAM && gCurrentUIMode != LOCKUI_MODE && gCurrentUIMode != LOCKOURTURN_UI_MODE)
	{
		guiPendingOverrideEvent = M_ON_TERRAIN;
	}

	ChangeInterfaceLevel(s->bLevel);

	if (s->fMercAsleep) PutMercInAwakeState(s);

	// possibly say personality quote
	if (s->bTeam == OUR_TEAM &&
		s->ubProfile != NO_PROFILE &&
		s->ubWhatKindOfMercAmI != MERC_TYPE__PLAYER_CHARACTER &&
		!(s->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY))
	{
		switch (gMercProfiles[s->ubProfile].bPersonalityTrait)
		{
			case PSYCHO:
				if (Random(50) == 0)
				{
					TacticalCharacterDialogue(s, QUOTE_PERSONALITY_TRAIT);
					s->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
				}
				break;

			default:
				break;
		}
	}

	UpdateForContOverPortrait(s, TRUE);

	// Remove any interactive tiles we could be over!
	BeginCurInteractiveTileCheck();
}


void LocateSoldier(SOLDIERTYPE* s, BOOLEAN fSetLocator)
{
	//if (!bCenter && SoldierOnScreen(s)) return;

	// do we need to move the screen?
	//ATE: Force this baby to locate if told to
	if (!SoldierOnScreen(s) || fSetLocator == 10)
	{
		// Center on guy
		const INT16 sNewCenterWorldX = (INT16)s->dXPos;
		const INT16 sNewCenterWorldY = (INT16)s->dYPos;
		SetRenderCenter(sNewCenterWorldX, sNewCenterWorldY);

		// Plot new path!
		gfPlotNewMovement = TRUE;
	}

	// do we flash the name & health bars/health string above?
	switch (fSetLocator)
	{
		case DONTSETLOCATOR: break;
		case 10:
		case SETLOCATOR:     ShowRadioLocator(s, SHOW_LOCATOR_NORMAL); break;
		default:             ShowRadioLocator(s, SHOW_LOCATOR_FAST);   break;
	}
}


void InternalLocateGridNo(UINT16 sGridNo, BOOLEAN fForce)
{
	INT16 sNewCenterWorldX;
	INT16 sNewCenterWorldY;
	ConvertGridNoToCenterCellXY(sGridNo, &sNewCenterWorldX, &sNewCenterWorldY);

	// FIRST CHECK IF WE ARE ON SCREEN
	if (GridNoOnScreen(sGridNo) && !fForce) return;

	SetRenderCenter(sNewCenterWorldX, sNewCenterWorldY);
}


void LocateGridNo(UINT16 sGridNo)
{
	InternalLocateGridNo(sGridNo, FALSE);
}


void SlideTo(SOLDIERTYPE* const tgt, const BOOLEAN fSetLocator)
{
	if (fSetLocator == SETANDREMOVEPREVIOUSLOCATOR)
	{
		FOR_EACH_SOLDIER(s)
		{
			if (s->bInSector)
			{
				// Remove all existing locators...
				s->fFlashLocator = FALSE;
			}
		}
	}

	// Locate even if on screen
	if (fSetLocator) ShowRadioLocator(tgt, SHOW_LOCATOR_NORMAL);

	// FIRST CHECK IF WE ARE ON SCREEN
	if (GridNoOnScreen(tgt->sGridNo)) return;

	// sGridNo here for DG compatibility
	gTacticalStatus.sSlideTarget = tgt->sGridNo;

	// Plot new path!
	gfPlotNewMovement = TRUE;
}


void SlideToLocation(const INT16 sDestGridNo)
{
	if (sDestGridNo == NOWHERE) return;

	// FIRST CHECK IF WE ARE ON SCREEN
	if (GridNoOnScreen(sDestGridNo)) return;

	// sGridNo here for DG compatibility
	gTacticalStatus.sSlideTarget = sDestGridNo;

	// Plot new path!
	gfPlotNewMovement = TRUE;
}


void RebuildAllSoldierShadeTables(void)
{
	FOR_EACH_SOLDIER(i) CreateSoldierPalettes(*i);
}


void HandlePlayerTeamMemberDeath(SOLDIERTYPE* pSoldier)
{
	VerifyPublicOpplistDueToDeath(pSoldier);
	ReceivingSoldierCancelServices(pSoldier);

	// look for all mercs on the same team,
	SOLDIERTYPE* new_selected_soldier = NULL;
	FOR_EACH_IN_TEAM(s, pSoldier->bTeam)
	{
		if (s->bLife >= OKLIFE && s->bInSector)
		{
			new_selected_soldier = s;
			break;
		}
	}

	if (new_selected_soldier != NULL)
	{
		if (gTacticalStatus.fAutoBandageMode &&
				pSoldier->auto_bandaging_medic != NULL)
		{
			CancelAIAction(pSoldier->auto_bandaging_medic);
		}

		// see if this was the friend of a living merc
		FOR_EACH_IN_TEAM(s, pSoldier->bTeam)
		{
			if (s->bInSector && s->bLife >= OKLIFE)
			{
				const INT8 bBuddyIndex = WhichBuddy(s->ubProfile, pSoldier->ubProfile);
				switch (bBuddyIndex)
				{
					case 0:  TacticalCharacterDialogue(s, QUOTE_BUDDY_ONE_KILLED);            break;
					case 1:  TacticalCharacterDialogue(s, QUOTE_BUDDY_TWO_KILLED);            break;
					case 2:  TacticalCharacterDialogue(s, QUOTE_LEARNED_TO_LIKE_MERC_KILLED); break;
					default: break;
				}
			}
		}

		switch (pSoldier->ubProfile)
		{
			case SLAY:
			{
				// handle stuff for Carmen if Slay is killed
				const SOLDIERTYPE* const s = FindSoldierByProfileID(CARMEN);
				if (s && s->bAttitude == ATTACKSLAYONLY && ClosestPC(s, NULL) != NOWHERE)
				{
					// Carmen now becomes friendly again
					TriggerNPCRecord(CARMEN, 29);
				}
				break;
			}

			case ROBOT:
				if (!CheckFact(FACT_FIRST_ROBOT_DESTROYED, 0))
				{
					SetFactTrue(FACT_FIRST_ROBOT_DESTROYED);
					SetFactFalse(FACT_ROBOT_READY);
				}
				else
				{
					SetFactTrue(FACT_SECOND_ROBOT_DESTROYED);
				}
				break;
		}
	}

	//Make a call to handle the strategic things, such as Life Insurance, record it in history file etc.
	StrategicHandlePlayerTeamMercDeath(*pSoldier);

	CheckForEndOfBattle(FALSE);


	if (GetSelectedMan() == pSoldier)
	{
		if (new_selected_soldier)
		{
			SelectSoldier(new_selected_soldier, SELSOLDIER_NONE);
		}
		else
		{
			SetSelectedMan(NULL);
			// Change UI mode to reflact that we are selected
			guiPendingOverrideEvent = I_ON_TERRAIN;
		}
	}
}


void HandleNPCTeamMemberDeath(SOLDIERTYPE* const pSoldierOld)
{
	pSoldierOld->uiStatusFlags |= SOLDIER_DEAD;
	const INT8 bVisible = pSoldierOld->bVisible;

	VerifyPublicOpplistDueToDeath(pSoldierOld);

	SOLDIERTYPE* const killer = pSoldierOld->attacker;

	if (pSoldierOld->ubProfile != NO_PROFILE)
	{
		// mark as dead!
		gMercProfiles[pSoldierOld->ubProfile].bMercStatus = MERC_IS_DEAD;
		gMercProfiles[pSoldierOld->ubProfile].bLife       = 0;

		if (!(pSoldierOld->uiStatusFlags & SOLDIER_VEHICLE) && !TANK(pSoldierOld))
		{
			const UINT8 code = (killer && killer->bTeam == OUR_TEAM ? HISTORY_MERC_KILLED_CHARACTER : HISTORY_NPC_KILLED);
			AddHistoryToPlayersLog(code, pSoldierOld->ubProfile, GetWorldTotalMin(), gWorldSector);
		}
	}

	if (pSoldierOld->bTeam == CIV_TEAM)
	{
		// ATE: Added string to player
		if (bVisible != -1 && pSoldierOld->ubProfile != NO_PROFILE)
		{
			ScreenMsg(FONT_RED, MSG_INTERFACE, st_format_printf(pMercDeadString, pSoldierOld->name));
		}

		switch (pSoldierOld->ubProfile)
		{
			case BRENDA:
				SetFactTrue(FACT_BRENDA_DEAD);
			{
				const SOLDIERTYPE* const pOther = FindSoldierByProfileID(HANS);
				if (pOther && pOther->bLife >= OKLIFE &&
					pOther->bNeutral && SpacesAway(pSoldierOld->sGridNo, pOther->sGridNo) <= 12)
				{
					TriggerNPCRecord(HANS, 10);
				}
			}
				break;

			case PABLO:
				AddFutureDayStrategicEvent(EVENT_SECOND_AIRPORT_ATTENDANT_ARRIVED, 480 + Random(60), 0, 1);
				break;

			case ROBOT:
				if (!CheckFact(FACT_FIRST_ROBOT_DESTROYED, 0))
				{
					SetFactTrue(FACT_FIRST_ROBOT_DESTROYED);
				}
				else
				{
					SetFactTrue(FACT_SECOND_ROBOT_DESTROYED);
				}
				break;

			case DRUGGIST:
			case SLAY:
			case ANNIE:
			case CHRIS:
			case TIFFANY:
			case T_REX:
				MakeRemainingTerroristsTougher();
				if (pSoldierOld->ubProfile == DRUGGIST)
				{
					SOLDIERTYPE* const pOther = FindSoldierByProfileID(MANNY);
					if (pOther && pOther->bInSector && pOther->bLife >= OKLIFE)
					{
						// try to make sure he isn't cowering etc
						pOther->sNoiseGridno = NOWHERE;
						pOther->bAlertStatus = STATUS_GREEN;
						TriggerNPCRecord(MANNY, 10);
					}
				}
				break;

			case JIM:
			case JACK:
			case OLAF:
			case RAY:
			case OLGA:
			case TYRONE:
				MakeRemainingAssassinsTougher();
				break;

			case ELDIN:
				// the security guard...  Results in an extra loyalty penalty for Balime (in addition to civilian murder)
				DecrementTownLoyalty(BALIME, LOYALTY_PENALTY_ELDIN_KILLED);
				break;

			case JOEY:
			{
				// check to see if Martha can see this
				const SOLDIERTYPE* const pOther = FindSoldierByProfileID(MARTHA);
				if (pOther && (PythSpacesAway(pOther->sGridNo, pSoldierOld->sGridNo ) < 10 ||
					SoldierToSoldierLineOfSightTest(pOther, pSoldierOld, MaxDistanceVisible(), TRUE) != 0))
				{
					// Martha has a heart attack and croaks
					TriggerNPCRecord(MARTHA, 17);
					DecrementTownLoyalty(CAMBRIA, LOYALTY_PENALTY_MARTHA_HEART_ATTACK);
				}
				else // Martha doesn't see it.  She lives, but Joey is found a day or so later anyways
				{
					DecrementTownLoyalty(CAMBRIA, LOYALTY_PENALTY_JOEY_KILLED);
				}
				break;
			}

			case DYNAMO:
				// check to see if dynamo quest is on
				if (gubQuest[QUEST_FREE_DYNAMO] == QUESTINPROGRESS)
				{
					EndQuest(QUEST_FREE_DYNAMO, pSoldierOld->sSector);
				}
				break;

			case KINGPIN:
				// check to see if Kingpin money quest is on
				if (gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS)
				{
					EndQuest(QUEST_KINGPIN_MONEY, pSoldierOld->sSector);
					HandleNPCDoAction(KINGPIN, NPC_ACTION_GRANT_EXPERIENCE_3, 0);
				}
				SetFactTrue(FACT_KINGPIN_DEAD);
				ExecuteStrategicAIAction(STRATEGIC_AI_ACTION_KINGPIN_DEAD, nullptr);
				break;

			case DOREEN:
				// Doreen's dead
				if (CheckFact(FACT_DOREEN_HAD_CHANGE_OF_HEART, 0))
				{
					// tsk tsk, player killed her after getting her to reconsider, lose the bonus for sparing her
					DecrementTownLoyalty(DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED);
				}
				// then get the points for freeing the kids though killing her
				IncrementTownLoyalty(DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_KILLED);
				// set the fact true so we have a universal check for whether the kids can go
				SetFactTrue(FACT_DOREEN_HAD_CHANGE_OF_HEART);
				EndQuest(QUEST_FREE_CHILDREN, gWorldSector);
				if (!CheckFact(FACT_KIDS_ARE_FREE, 0))
				{
					HandleNPCDoAction(DOREEN, NPC_ACTION_FREE_KIDS, 0);
				}
				break;
		}

		// Are we looking at the queen?
		if (pSoldierOld->ubProfile == QUEEN)
		{
			BeginHandleDeidrannaDeath(killer, pSoldierOld->sGridNo, pSoldierOld->bLevel);
		}

		// crows/cows are on the civilian team, but none of the following applies to them
		if (pSoldierOld->ubBodyType != CROW && pSoldierOld->ubBodyType != COW)
		{
			// handle death of civilian..and if it was intentional
			HandleMurderOfCivilian(pSoldierOld);
		}
	}
	else if (pSoldierOld->bTeam == MILITIA_TEAM)
	{
		const INT8 bMilitiaRank = SoldierClassToMilitiaRank(pSoldierOld->ubSoldierClass);
		if (bMilitiaRank != -1)
		{
			// remove this militia from the strategic records
			StrategicRemoveMilitiaFromSector(gWorldSector, bMilitiaRank, 1);
		}

		// also treat this as murder - but player will never be blamed for militia death he didn't cause
		HandleMurderOfCivilian(pSoldierOld);

		HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_NATIVE_KILLED, gWorldSector);
	}
	else // enemies and creatures... should any of this stuff not be called if a creature dies?
	{
		if (pSoldierOld->ubBodyType == QUEENMONSTER && killer != NULL)
		{
			BeginHandleQueenBitchDeath(killer, pSoldierOld->sGridNo, pSoldierOld->bLevel);
		}

		if (pSoldierOld->bTeam == ENEMY_TEAM)
		{
			gTacticalStatus.ubArmyGuysKilled++;
			TrackEnemiesKilled(ENEMY_KILLED_IN_TACTICAL, pSoldierOld->ubSoldierClass);
		}
		// If enemy guy was killed by the player, give morale boost to player's team!
		if (killer != NULL && killer->bTeam == OUR_TEAM)
		{
			HandleMoraleEvent(killer, MORALE_KILLED_ENEMY, gWorldSector);
		}

		HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ENEMY_KILLED, gWorldSector);

		CheckForAlertWhenEnemyDies(pSoldierOld);

		if (gTacticalStatus.the_chosen_one == pSoldierOld)
		{
			// reset the chosen one!
			gTacticalStatus.the_chosen_one = NULL;
		}

		if (pSoldierOld->ubProfile == QUEEN)
		{
			HandleMoraleEvent(NULL, MORALE_DEIDRANNA_KILLED, gWorldSector);
			MaximizeLoyaltyForDeidrannaKilled();
		}
		else if (pSoldierOld->ubBodyType == QUEENMONSTER)
		{
			HandleMoraleEvent(NULL, MORALE_MONSTER_QUEEN_KILLED, gWorldSector);
			IncrementTownLoyaltyEverywhere(LOYALTY_BONUS_KILL_QUEEN_MONSTER);

			// Grant experience gain.....
			HandleNPCDoAction(0, NPC_ACTION_GRANT_EXPERIENCE_5, 0);
		}
	}

	// killing crows/cows is not worth any experience!
	if (pSoldierOld->ubBodyType != CROW &&
		pSoldierOld->ubBodyType != COW  &&
		pSoldierOld->ubLastDamageReason != TAKE_DAMAGE_BLOODLOSS)
	{
		// if it was a kill by a player's merc
		if (killer != NULL && killer->bTeam == OUR_TEAM)
		{
			// EXPERIENCE CLASS GAIN:  Earned a kill
			StatChange(*killer, EXPERAMT, 10 * pSoldierOld->bExpLevel, FROM_SUCCESS);
		}

		// JA2 Gold: if previous and current attackers are the same, the next-to-previous attacker gets the assist
		SOLDIERTYPE* assister = pSoldierOld->previous_attacker;
		if (assister == killer) assister = pSoldierOld->next_to_previous_attacker;

		// if it was assisted by a player's merc
		if (assister != NULL && assister->bActive && assister->bTeam == OUR_TEAM)
		{
			// EXPERIENCE CLASS GAIN:  Earned an assist
			StatChange(*assister, EXPERAMT, 5 * pSoldierOld->bExpLevel, FROM_SUCCESS);
		}
	}

	if (killer != NULL && killer->bTeam == MILITIA_TEAM)
	{
		killer->ubMilitiaKills++;
	}

	//if the NPC is a dealer, add the dealers items to the ground
	AddDeadArmsDealerItemsToWorld(pSoldierOld);

	//The queen AI layer must process the event by subtracting forces, etc.
	ProcessQueenCmdImplicationsOfDeath(pSoldierOld);

	// OK, check for existence of any more badguys!
	CheckForEndOfBattle(FALSE);
}


void CheckForPotentialAddToBattleIncrement(SOLDIERTYPE* pSoldier)
{
	// Check if we are a threat!
	if (pSoldier->bNeutral || pSoldier->bSide == OUR_TEAM) return;

	if (pSoldier->bTeam == CIV_TEAM)
	{
		// maybe increment num enemy attacked
		switch (pSoldier->ubCivilianGroup)
		{
			case REBEL_CIV_GROUP:
			case KINGPIN_CIV_GROUP:
			case HICKS_CIV_GROUP:
				// We need to exclude cases where a kid is not neutral anymore, but is
				// defenseless!
				if (FindObjClass(pSoldier, IC_WEAPON) != NO_SLOT)
				{
					gTacticalStatus.bNumFoughtInBattle[pSoldier->bTeam]++;
				}
				break;

			default: break;
		}
	}
	else
	{
		// Increment num enemy attacked
		gTacticalStatus.bNumFoughtInBattle[pSoldier->bTeam]++;
	}
}


// internal function for turning neutral to FALSE
void SetSoldierNonNeutral(SOLDIERTYPE* pSoldier)
{
	pSoldier->bNeutral = FALSE;
	if (gTacticalStatus.bBoxingState == NOT_BOXING)
	{
		// Special code for strategic implications
		CalculateNonPersistantPBIInfo();
	}
}


// internal function for turning neutral to TRUE
void SetSoldierNeutral(SOLDIERTYPE* pSoldier)
{
	pSoldier->bNeutral = TRUE;
	if (gTacticalStatus.bBoxingState == NOT_BOXING)
	{
		// Special code for strategic implications
		// search through civ team looking for non-neutral civilian!
		if (!HostileCiviliansPresent())
		{
			CalculateNonPersistantPBIInfo();
		}
	}
}


void MakeCivHostile(SOLDIERTYPE* pSoldier, INT8 bNewSide)
{
	if (pSoldier->ubBodyType == COW) return;

	// override passed-in value; default is hostile to player, allied to army
	bNewSide = 1;

	switch (pSoldier->ubProfile)
	{
		case IRA:
		case DIMITRI:
		case MIGUEL:
		case CARLOS:
		case MADLAB:
		case DYNAMO:
		case SHANK:
			// rebels and rebel sympathizers become hostile to player and enemy
			bNewSide = 2;
			break;

		case MARIA:
		case ANGEL:
			if (gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS ||
				gubQuest[QUEST_RESCUE_MARIA] == QUESTDONE)
			{
				bNewSide = 2;
			}
			break;

		default:
			switch (pSoldier->ubCivilianGroup)
			{
				case REBEL_CIV_GROUP: bNewSide = 2; break;
				default:              break;
			}
			break;
	}

	if (!pSoldier->bNeutral && bNewSide == pSoldier->bSide)
	{
		// already hostile!
		return;
	}

	if (pSoldier->ubProfile == CONRAD || pSoldier->ubProfile == GENERAL)
	{
		// change to enemy team
		SetSoldierNonNeutral(pSoldier);
		pSoldier->bSide = bNewSide;
		pSoldier = ChangeSoldierTeam(pSoldier, ENEMY_TEAM);
	}
	else
	{
		if (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP)
		{
			// if Maria is in the sector and escorted, set fact that the escape has
			// been noticed
			if (gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS &&
				gTacticalStatus.bBoxingState == NOT_BOXING)
			{
				const SOLDIERTYPE* const pMaria = FindSoldierByProfileID(MARIA);
				if (pMaria && pMaria->bInSector)
				{
					SetFactTrue(FACT_MARIA_ESCAPE_NOTICED);
				}
			}
		}
		if (pSoldier->ubProfile == BILLY) pSoldier->bOrders = FARPATROL;
		if (bNewSide != -1) pSoldier->bSide = bNewSide;
		if (pSoldier->bNeutral)
		{
			SetSoldierNonNeutral(pSoldier);
			RecalculateOppCntsDueToNoLongerNeutral(pSoldier);
		}
	}

	// If we are already in combat...
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		CheckForPotentialAddToBattleIncrement(pSoldier);
	}
}


UINT8 CivilianGroupMembersChangeSidesWithinProximity(SOLDIERTYPE* pAttacked)
{
	if (pAttacked->ubCivilianGroup == NON_CIV_GROUP) return pAttacked->ubProfile;

	UINT8 ubFirstProfile = NO_PROFILE;
	FOR_EACH_IN_TEAM(s, CIV_TEAM)
	{
		if (!s->bInSector || s->bLife == 0 || !s->bNeutral) continue;
		if (s->ubCivilianGroup != pAttacked->ubCivilianGroup || s->ubBodyType == COW) continue;

		// if in LOS of this guy's attacker
		const SOLDIERTYPE* const attacker = pAttacked->attacker;
		if ((attacker != NULL && s->bOppList[attacker->ubID] == SEEN_CURRENTLY) ||
			(PythSpacesAway(s->sGridNo, pAttacked->sGridNo) < MaxDistanceVisible()) ||
			(attacker != NULL && PythSpacesAway(s->sGridNo, attacker->sGridNo) < MaxDistanceVisible()))
		{
			MakeCivHostile(s, 2);
			if (s->bOppCnt > 0)
			{
				AddToShouldBecomeHostileOrSayQuoteList(s);
			}

			if (s->ubProfile != NO_PROFILE &&
				s->bOppCnt > 0 &&
				(ubFirstProfile == NO_PROFILE || Random(2)))
			{
				ubFirstProfile = s->ubProfile;
			}
		}
	}

	return ubFirstProfile;
}


SOLDIERTYPE * CivilianGroupMemberChangesSides( SOLDIERTYPE * pAttacked )
{
	SOLDIERTYPE* pNewAttacked = pAttacked;

	if ( pAttacked->ubCivilianGroup == NON_CIV_GROUP )
	{
		// abort
		return( pNewAttacked );
	}

	// remove anyone (rebels) on our team and put them back in the civ team
	UINT8 ubFirstProfile = NO_PROFILE;
	FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->bInSector && pSoldier->bLife != 0)
		{
			if (pSoldier->ubCivilianGroup == pAttacked->ubCivilianGroup)
			{
				// should become hostile
				if ( pSoldier->ubProfile != NO_PROFILE &&
					( ubFirstProfile == NO_PROFILE || Random( 2 ) ) )
				{
					ubFirstProfile = pSoldier->ubProfile;
				}

				SOLDIERTYPE* const pNew = ChangeSoldierTeam(pSoldier, CIV_TEAM);
				if (pSoldier == pAttacked) pNewAttacked = pNew;
			}
		}
	}

	// now change sides for anyone on the civ team within proximity
	if ( ubFirstProfile == NO_PROFILE )
	{
		// get first profile value
		ubFirstProfile = CivilianGroupMembersChangeSidesWithinProximity( pNewAttacked );
	}
	else
	{
		// just call
		CivilianGroupMembersChangeSidesWithinProximity( pNewAttacked );
	}

	/*
	if ( ubFirstProfile != NO_PROFILE )
	{
		TriggerFriendWithHostileQuote( ubFirstProfile );
	}*/

	if ( gTacticalStatus.fCivGroupHostile[ pNewAttacked->ubCivilianGroup ] == CIV_GROUP_NEUTRAL )
	{
		// if the civilian group turning hostile is the Rebels
		if (pAttacked->ubCivilianGroup == REBEL_CIV_GROUP)
		{
			// we haven't already reduced the loyalty back when we first set the flag to BECOME hostile
			ReduceLoyaltyForRebelsBetrayed();
		}

		AddStrategicEvent( EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE, GetWorldTotalMin() + 300, pNewAttacked->ubCivilianGroup );
		gTacticalStatus.fCivGroupHostile[ pNewAttacked->ubCivilianGroup ] = CIV_GROUP_WILL_EVENTUALLY_BECOME_HOSTILE;
	}

	return( pNewAttacked );
}

void CivilianGroupChangesSides( UINT8 ubCivilianGroup )
{
	// change civ group side due to external event (wall blowing up)
	//UINT8 ubFirstProfile = NO_PROFILE;

	gTacticalStatus.fCivGroupHostile[ ubCivilianGroup ] = CIV_GROUP_HOSTILE;

	// now change sides for anyone on the civ team
	FOR_EACH_IN_TEAM(pSoldier, CIV_TEAM)
	{
		if (pSoldier->bInSector && pSoldier->bLife && pSoldier->bNeutral)
		{
			if ( pSoldier->ubCivilianGroup == ubCivilianGroup && pSoldier->ubBodyType != COW )
			{
				MakeCivHostile( pSoldier, 2 );
				if ( pSoldier->bOppCnt > 0 )
				{
					AddToShouldBecomeHostileOrSayQuoteList(pSoldier);
				}
				/*
				if ( (pSoldier->ubProfile != NO_PROFILE) && (pSoldier->bOppCnt > 0) && ( ubFirstProfile == NO_PROFILE || Random( 2 ) ) )
				{
					ubFirstProfile = pSoldier->ubProfile;
				}
				*/
			}
		}
	}

	/*
	if ( ubFirstProfile != NO_PROFILE )
	{
		TriggerFriendWithHostileQuote( ubFirstProfile );
	}
	*/
}


static void HickCowAttacked(SOLDIERTYPE* pNastyGuy, SOLDIERTYPE* pTarget)
{
	// now change sides for anyone on the civ team
	FOR_EACH_IN_TEAM(pSoldier, CIV_TEAM)
	{
		if (pSoldier->bInSector && pSoldier->bLife && pSoldier->bNeutral && pSoldier->ubCivilianGroup == HICKS_CIV_GROUP)
		{
			if ( SoldierToSoldierLineOfSightTest( pSoldier, pNastyGuy, (UINT8) MaxDistanceVisible(), TRUE ) )
			{

				CivilianGroupMemberChangesSides( pSoldier );
				break;
			}
		}
	}
}


static void MilitiaChangesSides(void)
{
	// make all the militia change sides
	if (!IsTeamActive(MILITIA_TEAM)) return;

	// remove anyone (rebels) on our team and put them back in the civ team
	FOR_EACH_IN_TEAM(s, MILITIA_TEAM)
	{
		if (s->bInSector && s->bLife != 0)
		{
			MakeCivHostile(s, 2);
			RecalculateOppCntsDueToNoLongerNeutral(s);
		}
	}
}


static SOLDIERTYPE* FindActiveAndAliveMerc(const SOLDIERTYPE* const curr, const UINT step, const BOOLEAN fGoodForLessOKLife, const BOOLEAN fOnlyRegularMercs)
{
	const TacticalTeamType* const t = &gTacticalStatus.Team[curr->bTeam];

	SOLDIERTYPE* s;
	SoldierID    i = curr->ubID;
	do
	{
		UINT di = step;
		if (i > t->bLastID - di) di -= t->bLastID - t->bFirstID + 1; // modulo, subtract span
		i += di;
		s = &GetMan(i);
		if (!s->bActive) continue;

		if (fOnlyRegularMercs && (AM_AN_EPC(s) || AM_A_ROBOT(s))) continue;
		if (s->bAssignment != curr->bAssignment)                  continue;
		if (!OK_INTERRUPT_MERC(s))                                continue;
		if (!s->bInSector)                                        continue;
		if (s->bLife < (fGoodForLessOKLife ? 1 : OKLIFE))         continue;
		break;
	}
	while (s != curr);
	return s;
}


SOLDIERTYPE* FindNextActiveAndAliveMerc(const SOLDIERTYPE* const curr, const BOOLEAN fGoodForLessOKLife, const BOOLEAN fOnlyRegularMercs)
{
	return FindActiveAndAliveMerc(curr, 1, fGoodForLessOKLife, fOnlyRegularMercs);
}


SOLDIERTYPE* FindPrevActiveAndAliveMerc(const SOLDIERTYPE* const curr, const BOOLEAN fGoodForLessOKLife, const BOOLEAN fOnlyRegularMercs)
{
	const TacticalTeamType* const t    = &gTacticalStatus.Team[curr->bTeam];
	UINT                    const step = t->bLastID - t->bFirstID; // modulo: -1
	return FindActiveAndAliveMerc(curr, step, fGoodForLessOKLife, fOnlyRegularMercs);
}


static SOLDIERTYPE* FindNextActiveSquadRange(INT8 begin, INT8 end)
{
	for (INT32 i = begin; i != end; ++i)
	{
		FOR_EACH_IN_SQUAD(j, i)
		{
			SOLDIERTYPE* const s = *j;
			if (!s->bInSector)                      continue;
			if (!OK_INTERRUPT_MERC(s))              continue;
			if (!OK_CONTROLLABLE_MERC(s))           continue;
			if (s->uiStatusFlags & SOLDIER_VEHICLE) continue;
			return s;
		}
	}
	return NULL;
}


SOLDIERTYPE* FindNextActiveSquad(SOLDIERTYPE* s)
{
	const INT8 assignment = s->bAssignment;
	SOLDIERTYPE* res;

	res = FindNextActiveSquadRange(assignment + 1, NUMBER_OF_SQUADS);
	if (res != NULL) return res;

	// none found, now loop back
	res = FindNextActiveSquadRange(0, assignment);
	if (res != NULL) return res;

	// IF we are here, keep as we always were!
	return s;
}


static bool IsDestinationBlocked(GridNo const grid_no, INT8 const level, SOLDIERTYPE const& s)
{
	// ATE: If we are trying to get a path to an exit grid, still allow this
	if (gfPlotPathToExitGrid) return false;

	// Check obstruction in future
	INT16 const desired_level = level == 0 ? STRUCTURE_ON_GROUND : STRUCTURE_ON_ROOF;
	FOR_EACH_STRUCTURE(i, grid_no, STRUCTURE_BLOCKSMOVES)
	{
		if (i->fFlags & STRUCTURE_PASSABLE) continue;

		// Check if this is a multi-tile and check IDs with soldier's ID
		if (i->fFlags & STRUCTURE_MOBILE &&
			s.uiStatusFlags & SOLDIER_MULTITILE &&
			s.pLevelNode &&
			s.pLevelNode->pStructureData &&
			s.pLevelNode->pStructureData->usStructureID == i->usStructureID)
		{
			continue;
		}

		if (i->sCubeOffset == desired_level)
			return true;
	}

	return false;
}


// NB if making changes don't forget to update NewOKDestinationAndDirection
INT16 NewOKDestination(const SOLDIERTYPE* pCurrSoldier, INT16 sGridNo, BOOLEAN fPeopleToo, INT8 bLevel)
{
	if (!GridNoOnVisibleWorldTile(sGridNo)) // grid is outside visible world.. it's NOT OK
		return FALSE;

	if (fPeopleToo)
	{
		const SOLDIERTYPE* const person = WhoIsThere2(sGridNo, bLevel);
		if (person != NULL)
		{
			// we could be multitiled... if the person there is us, and the gridno is not
			// our base gridno, skip past these checks
			if (person != pCurrSoldier || sGridNo == pCurrSoldier->sGridNo)
			{
				if (pCurrSoldier->bTeam != OUR_TEAM ||
					person->bVisible >= 0 ||
					gTacticalStatus.uiFlags & SHOW_ALL_MERCS)
				{
					return FALSE; // if someone there it's NOT OK
				}
			}
		}
	}

	// Check structure database
	if (pCurrSoldier->uiStatusFlags & SOLDIER_MULTITILE && !gfEstimatePath)
	{
		// this could be kinda slow...

		// Get animation surface...
		const UINT16 usAnimSurface = DetermineSoldierAnimationSurface(pCurrSoldier, pCurrSoldier->usUIMovementMode);
		// Get structure ref...
		const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pCurrSoldier, usAnimSurface, pCurrSoldier->usUIMovementMode);

		// opposite directions should be mirrors, so only check 4
		if (pStructureFileRef)
		{
			// if ANY direction is valid, consider moving here valid
			for (INT8 i = 0; i < NUM_WORLD_DIRECTIONS; ++i)
			{
				// ATE: Only if we have a levelnode...
				UINT16 const usStructureID = GetStructureID(pCurrSoldier);

				if (InternalOkayToAddStructureToWorld(sGridNo, bLevel, &pStructureFileRef->pDBStructureRef[i], usStructureID, !fPeopleToo))
				{
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	if (IsDestinationBlocked(sGridNo, bLevel, *pCurrSoldier))
		return FALSE;

	return TRUE;
}


// NB if making changes don't forget to update NewOKDestination
static INT16 NewOKDestinationAndDirection(const SOLDIERTYPE* pCurrSoldier, INT16 sGridNo, INT8 bDirection, BOOLEAN fPeopleToo, INT8 bLevel)
{
	if (fPeopleToo)
	{
		const SOLDIERTYPE* const person = WhoIsThere2(sGridNo, bLevel);
		if (person != NULL)
		{
			// we could be multitiled... if the person there is us, and the gridno is
			// not our base gridno, skip past these checks
			if (person != pCurrSoldier || sGridNo == pCurrSoldier->sGridNo)
			{
				if (pCurrSoldier->bTeam != OUR_TEAM ||
						person->bVisible >= 0 ||
						gTacticalStatus.uiFlags & SHOW_ALL_MERCS)
				{
					return FALSE; // if someone there it's NOT OK
				}
			}
		}
	}

	// Check structure database
	if (pCurrSoldier->uiStatusFlags & SOLDIER_MULTITILE && !gfEstimatePath)
	{
		// this could be kinda slow...

		// Get animation surface...
		const UINT16 usAnimSurface = DetermineSoldierAnimationSurface(pCurrSoldier, pCurrSoldier->usUIMovementMode);
		// Get structure ref...
		const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pCurrSoldier, usAnimSurface, pCurrSoldier->usUIMovementMode);
		if (pStructureFileRef)
		{
			// use the specified direction for checks
			const INT8 bLoop = bDirection;
			// ATE: Only if we have a levelnode...
			UINT16 const usStructureID = GetStructureID(pCurrSoldier);

			if (InternalOkayToAddStructureToWorld(sGridNo, pCurrSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(bLoop)], usStructureID, !fPeopleToo))
			{
				return TRUE;
			}
		}
		return FALSE;
	}

	if (IsDestinationBlocked(sGridNo, bLevel, *pCurrSoldier)) return FALSE;

	return TRUE;
}


//Kris:
BOOLEAN FlatRoofAboveGridNo(INT32 iMapIndex)
{
	for (const LEVELNODE* i = gpWorldLevelData[iMapIndex].pRoofHead; i; i = i->pNext)
	{
		if (i->usIndex == NO_TILE) continue;

		const UINT32 uiTileType = GetTileType(i->usIndex);
		if (uiTileType >= FIRSTROOF && uiTileType <= LASTROOF) return TRUE;
	}
	return FALSE;
}


// Kris:
// ASSUMPTION:  This function assumes that we are checking on behalf of a single
//              tiled merc.  This function should not be used for checking on
//              behalf of multi-tiled "things".
// I wrote this function for editor use.  I don't personally care about
// multi-tiled stuff.  All I want to know is whether or not I can put a merc
// here.  In most cases, I won't be dealing with multi-tiled mercs, and the
// rarity doesn't justify the needs.  I just wrote this to be quick and dirty,
// and I don't expect it to perform perfectly in all situations.
BOOLEAN IsLocationSittable( INT32 iMapIndex, BOOLEAN fOnRoof )
{
	INT16 sDesiredLevel;
	if (WhoIsThere2(iMapIndex, 0) != NULL) return FALSE;
	//Locations on roofs without a roof is not possible, so
	//we convert the onroof intention to ground.
	if( fOnRoof && !FlatRoofAboveGridNo( iMapIndex ) )
		fOnRoof = FALSE;
	// Check structure database
	if( gpWorldLevelData[ iMapIndex ].pStructureHead )
	{
		// Something is here, check obstruction in future
		sDesiredLevel = fOnRoof ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
		FOR_EACH_STRUCTURE(pStructure, (INT16)iMapIndex, STRUCTURE_BLOCKSMOVES)
		{
			if( !(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel )
				return FALSE;
		}
	}
	return TRUE;
}


BOOLEAN IsLocationSittableExcludingPeople( INT32 iMapIndex, BOOLEAN fOnRoof )
{
	INT16 sDesiredLevel;

	//Locations on roofs without a roof is not possible, so
	//we convert the onroof intention to ground.
	if( fOnRoof && !FlatRoofAboveGridNo( iMapIndex ) )
		fOnRoof = FALSE;
	// Check structure database
	if( gpWorldLevelData[ iMapIndex ].pStructureHead )
	{
		// Something is here, check obstruction in future
		sDesiredLevel = fOnRoof ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
		FOR_EACH_STRUCTURE(pStructure, (INT16)iMapIndex, STRUCTURE_BLOCKSMOVES)
		{
			if( !(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel )
				return FALSE;
		}
	}
	return TRUE;
}


BOOLEAN TeamMemberNear(INT8 bTeam, INT16 sGridNo, INT32 iRange)
{
	CFOR_EACH_IN_TEAM(s, bTeam)
	{
		if (s->bInSector &&
			s->bLife >= OKLIFE &&
			!(s->uiStatusFlags & SOLDIER_GASSED) &&
			PythSpacesAway(s->sGridNo,sGridNo) <= iRange)
		{
			return TRUE;
		}
	}
	return FALSE;
}


INT16 FindAdjacentGridEx(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8* pubDirection, INT16* psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor)
{
	// psAdjustedGridNo gets the original gridno or the new one if updated
	// It will ONLY be updated IF we were over a merc, ( it's updated to their gridno )
	// pubDirection gets the direction to the final gridno
	// fForceToPerson: forces the grid under consideration to be the one occupiedby any target
	// in that location, because we could be passed a gridno based on the overlap of soldier's graphic
	// fDoor determines whether special door-handling code should be used (for interacting with doors)

	INT16 sDistance = 0;
	INT16 sDirs[4] = { NORTH, EAST, SOUTH, WEST };
	INT32 cnt;
	INT16 sClosest=NOWHERE, sSpot;
	INT16 sCloseGridNo=NOWHERE;
	UINT8 ubDir;
	STRUCTURE *pDoor;
	//STRUCTURE *pWall;
	UINT8    ubWallOrientation;
	BOOLEAN  fCheckGivenGridNo = TRUE;
	UINT8    ubTestDirection;
	EXITGRID ExitGrid;

	// Set default direction
	if (pubDirection)
	{
		*pubDirection = pSoldier->bDirection;
	}

	// CHECK IF WE WANT TO FORCE GRIDNO TO PERSON
	if (psAdjustedGridNo != NULL)
	{
		*psAdjustedGridNo = sGridNo;
	}

	// CHECK IF IT'S THE SAME ONE AS WE'RE ON, IF SO, RETURN THAT!
	if (pSoldier->sGridNo == sGridNo && !FindStructure(sGridNo, STRUCTURE_SWITCH))
	{
		// OK, if we are looking for a door, it may be in the same tile as us, so find
		// the direction we have to face to get to the door, not just our initial
		// direction...
		// If we are in the same tile as a switch, we can NEVER pull it....
		if (fDoor)
		{
			// This can only happen if a door was to the south to east of us!

			// Do south!
			//sSpot = NewGridNo( sGridNo, DirectionInc( SOUTH ) );

			// ATE: Added: Switch behave EXACTLY like doors
			pDoor = FindStructure(sGridNo, STRUCTURE_ANYDOOR);

			if (pDoor != NULL)
			{
				// Get orinetation
				ubWallOrientation = pDoor->ubWallOrientation;

				if (ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT)
				{
					// To the south!
					sSpot = NewGridNo(sGridNo, DirectionInc(SOUTH));
					if (pubDirection)
					{
						*pubDirection = GetDirectionFromGridNo(sSpot, pSoldier);
					}
				}

				if (ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT)
				{
					// TO the east!
					sSpot = NewGridNo(sGridNo, DirectionInc(EAST));
					if (pubDirection)
					{
						*pubDirection = GetDirectionFromGridNo(sSpot, pSoldier);
					}
				}
			}
		}

		// Use soldier's direction
		return sGridNo;
	}

	// Look for a door!
	if (fDoor)
	{
		pDoor = FindStructure(sGridNo, STRUCTURE_ANYDOOR | STRUCTURE_SWITCH);
	}
	else
	{
		pDoor = NULL;
	}

	if (fForceToPerson)
	{
		const SOLDIERTYPE* const s = FindSoldier(sGridNo, FIND_SOLDIER_GRIDNO);
		if (s != NULL)
		{
			sGridNo = s->sGridNo;
			if (psAdjustedGridNo != NULL)
			{
				*psAdjustedGridNo = sGridNo;

				// Use direction to this guy!
				if (pubDirection)
				{
					*pubDirection = GetDirectionFromGridNo(sGridNo, pSoldier);
				}
			}
		}
	}


	if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) > 0) // no problem going there! nobody on it!
	{
		// OK, if we are looking to goto a switch, ignore this....
		if (pDoor)
		{
			if (pDoor->fFlags & STRUCTURE_SWITCH)
			{
				// Don't continuel
				fCheckGivenGridNo = FALSE;
			}
		}

		// If there is an exit grid....
		if (GetExitGrid(sGridNo, &ExitGrid))
		{
			// Don't continuel
			fCheckGivenGridNo = FALSE;
		}


		if ( fCheckGivenGridNo )
		{
			sDistance = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
			if (sDistance > 0 && sDistance < sClosest)
			{
				sClosest     = sDistance;
				sCloseGridNo = sGridNo;
			}
		}
	}


	for (cnt = 0; cnt < 4; cnt++)
	{
		// MOVE OUT TWO DIRECTIONS
		sSpot = NewGridNo(sGridNo, DirectionInc(sDirs[cnt]));

		ubTestDirection = sDirs[cnt];

		// For switches, ALLOW them to walk through walls to reach it....
		if (pDoor && pDoor->fFlags & STRUCTURE_SWITCH)
		{
			ubTestDirection = OppositeDirection(ubTestDirection);
		}

		if (fDoor)
		{
			if ( gubWorldMovementCosts[ sSpot ][ ubTestDirection ][ pSoldier->bLevel ] >= TRAVELCOST_BLOCKED )
			{
				// obstacle or wall there!
				continue;
			}
		}
		else
		{
			// this function returns original MP cost if not a door cost
			if ( DoorTravelCost( pSoldier, sSpot, gubWorldMovementCosts[ sSpot ][ ubTestDirection ][ pSoldier->bLevel ], FALSE, NULL ) >= TRAVELCOST_BLOCKED )
			{
			// obstacle or wall there!
			continue;
			}
		}

		// Eliminate some directions if we are looking at doors!
		if ( pDoor != NULL )
		{
			// Get orinetation
			ubWallOrientation = pDoor->ubWallOrientation;

			// Refuse the south and north and west  directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == SOUTH )
					continue;
			}

			// Refuse the north and west and east directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == EAST )
					continue;
			}
		}

		// If this spot is our soldier's gridno use that!
		if ( sSpot == pSoldier->sGridNo )
		{
			// Use default diurection ) soldier's direction )

			// OK, at least get direction to face......
			// Defaults to soldier's facing dir unless we change it!
			//if ( pDoor != NULL )
			{
				// Use direction to the door!
				if (pubDirection)
				{
					*pubDirection = GetDirectionFromGridNo(sGridNo, pSoldier);
				}
			}
			return sSpot;
		}

		// don't store path, just measure it
		ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sSpot, sGridNo );

		if (NewOKDestinationAndDirection(pSoldier, sSpot, ubDir, TRUE, pSoldier->bLevel) > 0 &&
				(sDistance = PlotPath(pSoldier, sSpot, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints)) > 0)
		{
			if (sDistance < sClosest)
			{
				sClosest     = sDistance;
				sCloseGridNo = sSpot;
			}
		}
	}

	if (sClosest == NOWHERE) return -1;

	// Take last direction and use opposite!
	// This will be usefull for ours and AI mercs

	// If our gridno is the same ( which can be if we are look at doors )
	if (sGridNo == sCloseGridNo)
	{
		if (pubDirection)
		{
			// ATE: Only if we have a valid door!
			if (pDoor)
			{
				switch (pDoor->pDBStructureRef->pDBStructure->ubWallOrientation)
				{
					case OUTSIDE_TOP_LEFT:
					case INSIDE_TOP_LEFT:   *pubDirection = SOUTH; break;
					case OUTSIDE_TOP_RIGHT:
					case INSIDE_TOP_RIGHT:  *pubDirection = EAST; break;
				}
			}
		}
	}
	else
	{
		// Calculate direction if our gridno is different....
		ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
		if (pubDirection)
		{
			*pubDirection = ubDir;
		}
	}
	//if ( psAdjustedGridNo != NULL )
	//{
	//		(*psAdjustedGridNo) = sCloseGridNo;
	//}
	if (sCloseGridNo == NOWHERE) return -1;
	return sCloseGridNo;
}


INT16 FindNextToAdjacentGridEx( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 *pubDirection, INT16 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor )
{
	// This function works in a similar way as FindAdjacentGridEx, but looks for a location 2 tiles away

	// psAdjustedGridNo gets the original gridno or the new one if updated
	// pubDirection gets the direction to the final gridno
	// fForceToPerson: forces the grid under consideration to be the one occupiedby any target
	// in that location, because we could be passed a gridno based on the overlap of soldier's graphic
	// fDoor determines whether special door-handling code should be used (for interacting with doors)

	INT16     sDistance=0;
	INT16     sDirs[4] = { NORTH, EAST, SOUTH, WEST };
	INT32     cnt;
	INT16     sClosest=WORLD_MAX, sSpot, sSpot2;
	INT16     sCloseGridNo=NOWHERE;
	UINT8     ubDir;
	STRUCTURE *pDoor;
	UINT8     ubWallOrientation;
	BOOLEAN   fCheckGivenGridNo = TRUE;
	UINT8     ubTestDirection;

	// CHECK IF WE WANT TO FORCE GRIDNO TO PERSON
	if (psAdjustedGridNo != NULL) *psAdjustedGridNo = sGridNo;

	// CHECK IF IT'S THE SAME ONE AS WE'RE ON, IF SO, RETURN THAT!
	if (pSoldier->sGridNo == sGridNo)
	{
		if (pubDirection != NULL) *pubDirection = pSoldier->bDirection;
		return sGridNo;
	}

	// Look for a door!
	if (fDoor)
	{
		pDoor = FindStructure(sGridNo, STRUCTURE_ANYDOOR | STRUCTURE_SWITCH);
	}
	else
	{
		pDoor = NULL;
	}

	if (fForceToPerson)
	{
		const SOLDIERTYPE* const s = FindSoldier(sGridNo, FIND_SOLDIER_GRIDNO);
		if (s != NULL)
		{
			sGridNo = s->sGridNo;
			if (psAdjustedGridNo != NULL) *psAdjustedGridNo = sGridNo;
		}
	}


	if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) > 0) // no problem going there! nobody on it!
	{
		// OK, if we are looking to goto a switch, ignore this....
		if (pDoor)
		{
			if (pDoor->fFlags & STRUCTURE_SWITCH)
			{
				// Don't continuel
				fCheckGivenGridNo = FALSE;
			}
		}

		if ( fCheckGivenGridNo )
		{
			sDistance = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
			if (sDistance > 0 && sDistance < sClosest)
			{
				sClosest     = sDistance;
				sCloseGridNo = sGridNo;
			}
		}
	}


	for (cnt = 0; cnt < 4; cnt++)
	{
		// MOVE OUT TWO DIRECTIONS
		sSpot = NewGridNo(sGridNo, DirectionInc(sDirs[cnt]));

		ubTestDirection = sDirs[cnt];

		if (pDoor && pDoor->fFlags & STRUCTURE_SWITCH)
		{
			ubTestDirection = OppositeDirection(ubTestDirection);
		}

		if (gubWorldMovementCosts[sSpot][ubTestDirection][pSoldier->bLevel] >= TRAVELCOST_BLOCKED)
		{
			// obstacle or wall there!
			continue;
		}

		const SOLDIERTYPE* const tgt = WhoIsThere2(sSpot, pSoldier->bLevel);
		if (tgt != NULL && tgt != pSoldier)
		{
			// skip this direction b/c it's blocked by another merc!
			continue;
		}

		// Eliminate some directions if we are looking at doors!
		if (pDoor != NULL)
		{
			// Get orinetation
			ubWallOrientation = pDoor->ubWallOrientation;

			// Refuse the south and north and west  directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == SOUTH )
					continue;
			}

			// Refuse the north and west and east directions if our orientation is top-right
			if ( ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT )
			{
				if ( sDirs[ cnt ] == NORTH || sDirs[ cnt ] == WEST || sDirs[ cnt ] == EAST )
					continue;
			}
		}

		// first tile is okay, how about the second?
		sSpot2 = NewGridNo( sSpot, DirectionInc( sDirs[ cnt ] ) );
		if (gubWorldMovementCosts[sSpot2][sDirs[cnt]][pSoldier->bLevel] >= TRAVELCOST_BLOCKED ||
			DoorTravelCost(pSoldier, sSpot2, gubWorldMovementCosts[sSpot2][sDirs[cnt]][pSoldier->bLevel], pSoldier->bTeam == OUR_TEAM, NULL) == TRAVELCOST_DOOR) // closed door blocks!
		{
			// obstacle or wall there!
			continue;
		}

		const SOLDIERTYPE* const tgt2 = WhoIsThere2(sSpot2, pSoldier->bLevel);
		if (tgt2 != NULL && tgt2 != pSoldier)
		{
			// skip this direction b/c it's blocked by another merc!
			continue;
		}

		sSpot = sSpot2;

		// If this spot is our soldier's gridno use that!
		if (sSpot == pSoldier->sGridNo)
		{
			if (pubDirection)
			{
				*pubDirection = (UINT8)GetDirectionFromGridNo(sGridNo, pSoldier);
			}
			//*pubDirection = pSoldier->bDirection;
			return( sSpot );
		}

		ubDir = GetDirectionToGridNoFromGridNo(sSpot, sGridNo);

		// don't store path, just measure it
		if (NewOKDestinationAndDirection(pSoldier, sSpot, ubDir, TRUE, pSoldier->bLevel) > 0 &&
			(sDistance = PlotPath(pSoldier, sSpot, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints)) > 0)
		{
			if (sDistance < sClosest)
			{
				sClosest     = sDistance;
				sCloseGridNo = sSpot;
			}
		}
	}

	if (sClosest == NOWHERE) return -1;

	// Take last direction and use opposite!
	// This will be usefull for ours and AI mercs

	// If our gridno is the same ( which can be if we are look at doors )
	if (sGridNo == sCloseGridNo)
	{
		if (pubDirection)
		{
			// ATE: Only if we have a valid door!
			if (pDoor)
			{
				switch (pDoor->pDBStructureRef->pDBStructure->ubWallOrientation)
				{
					case OUTSIDE_TOP_LEFT:
					case INSIDE_TOP_LEFT:
						*pubDirection = SOUTH;
						break;

					case OUTSIDE_TOP_RIGHT:
					case INSIDE_TOP_RIGHT:
						*pubDirection = EAST;
						break;
				}
			}
		}
	}
	else
	{
		// Calculate direction if our gridno is different....
		ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
		if (pubDirection)
		{
			*pubDirection = ubDir;
		}
	}

	if (sCloseGridNo == NOWHERE) return -1;
	return sCloseGridNo;

	/*
	if (sCloseGridNo == NOWHERE) return -1;

	// Take last direction and use opposite!
	// This will be usefull for ours and AI mercs

	// If our gridno is the same ( which can be if we are look at doors )
	if ( sGridNo == sCloseGridNo )
	{
		switch( pDoor->pDBStructureRef->pDBStructure->ubWallOrientation )
		{
			case OUTSIDE_TOP_LEFT:
			case INSIDE_TOP_LEFT:
				*pubDirection = SOUTH;
				break;

			case OUTSIDE_TOP_RIGHT:
			case INSIDE_TOP_RIGHT:
				*pubDirection = EAST;
				break;
		}
	}
	else
	{
		// Calculate direction if our gridno is different....
		ubDir = (UINT8)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
		*pubDirection = ubDir;
	}
	return sCloseGridNo;
	*/
}


INT16 FindAdjacentPunchTarget(const SOLDIERTYPE* const pSoldier, const SOLDIERTYPE* const pTargetSoldier, INT16* const psAdjustedTargetGridNo)
{
	Assert(pTargetSoldier != NULL);

	for (UINT8 i = 0; i < NUM_WORLD_DIRECTIONS; ++i)
	{
		const INT16 sSpot = NewGridNo(pSoldier->sGridNo, DirectionInc(i));

		if (DoorTravelCost(pSoldier, sSpot, gubWorldMovementCosts[sSpot][i][pSoldier->bLevel], FALSE, NULL) >= TRAVELCOST_BLOCKED)
		{
			// blocked!
			continue;
		}

		// Check for who is there...
		if (pTargetSoldier == WhoIsThere2(sSpot, pSoldier->bLevel))
		{
			// We've got a guy here....
			// Who is the one we want......
			*psAdjustedTargetGridNo = pTargetSoldier->sGridNo;
			return sSpot;
		}
	}

	return NOWHERE;
}


BOOLEAN UIOKMoveDestination(const SOLDIERTYPE* pSoldier, UINT16 usMapPos)
{
	if ( !NewOKDestination( pSoldier, usMapPos, FALSE, (INT8) gsInterfaceLevel ) )
	{
		return( FALSE );
	}

	// ATE: If we are a robot, see if we are being validly controlled...
	if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
	{
		if ( ! CanRobotBeControlled( pSoldier ) )
		{
			// Display message that robot cannot be controlled....
			return( 2 );
		}
	}

	// ATE: Experiment.. take out
	//else if ( IsRoofVisible( usMapPos ) && gsInterfaceLevel == 0 )
	//{
	//	return( FALSE );
	//}


	return( TRUE);
}


void HandleTeamServices( UINT8 ubTeamNum )
{
	FOR_EACH_IN_TEAM(i, ubTeamNum)
	{
		SOLDIERTYPE& s = *i;
		if (s.bInSector) HandlePlayerServices(s);
	}
}


void HandlePlayerServices(SOLDIERTYPE& s)
{
	if (s.bLife < OKLIFE)            return;
	if (s.usAnimState != GIVING_AID) return;

	SOLDIERTYPE* const tgt = WhoIsThere2(s.sTargetGridNo, s.bLevel);
	if (!tgt) return;
	if (tgt->ubServiceCount == 0) return;

	OBJECTTYPE&  in_hand     = s.inv[HANDPOS];
	UINT16 const kit_pts     = TotalPoints(&in_hand);
	UINT32 const points_used = SoldierDressWound(&s, tgt, kit_pts, kit_pts);

	// Determine if they are all banagded
	bool done = FALSE;
	if (tgt->bBleeding == 0 && tgt->bLife >= OKLIFE)
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[MERC_IS_ALL_BANDAGED_STR], tgt->name));
		// Cancel all services for this guy!
		ReceivingSoldierCancelServices(tgt);
		done = TRUE;
	}

	UseKitPoints(in_hand, points_used, s);

	// Whether or not recipient is all bandaged, check if we've used them up
	if (TotalPoints(&in_hand) > 0) return;
	// No more bandages

	INT8 slot;
	if (done)
	{
		// Don't swap if we're done
		slot = NO_SLOT;
	}
	else
	{
		slot = FindObj(&s, FIRSTAIDKIT);
		if (slot == NO_SLOT)
		{
			slot = FindObj(&s, MEDICKIT);
		}
	}

	if (slot != NO_SLOT)
	{
		SwapObjs(&in_hand, &s.inv[slot]);
	}
	else
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[MERC_IS_OUT_OF_BANDAGES_STR], s.name));
		GivingSoldierCancelServices(&s);
		if (!gTacticalStatus.fAutoBandageMode)
		{
			DoMercBattleSound(&s, BATTLE_SOUND_CURSE1);
		}
	}
}


void CommonEnterCombatModeCode( )
{
	gTacticalStatus.uiFlags |= INCOMBAT;

	//gTacticalStatus.ubAttackBusyCount = 0;

	// Reset num enemies fought flag...
	std::fill_n(gTacticalStatus.bNumFoughtInBattle, MAXTEAMS, 0);
	gTacticalStatus.ubLastBattleSector = gWorldSector;
	gTacticalStatus.fLastBattleWon      = FALSE;
	gTacticalStatus.fItemsSeenOnAttack  = FALSE;


	// ATE: If we have an item pointer end it!
	CancelItemPointer( );

	ResetInterfaceAndUI( );
	ResetMultiSelection( );

	// OK, loop thorugh all guys and stop them!
	// Loop through all mercs and make go
	FOR_EACH_SOLDIER(pSoldier)
	{
		if ( pSoldier->bInSector && pSoldier->ubBodyType != CROW )
		{
			// Set some flags for quotes
			pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_IN_SHIT );
			pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES);

			// Hault!
			EVENT_StopMerc(pSoldier);

			// END AI actions
			CancelAIAction(pSoldier);

			// turn off AI controlled flag
			pSoldier->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;

			// Check if this guy is an enemy....
			CheckForPotentialAddToBattleIncrement( pSoldier );

			// If guy is sleeping, wake him up!
			if (pSoldier->fMercAsleep)
			{
				ChangeSoldierState( pSoldier, WKAEUP_FROM_SLEEP, 1, TRUE );
			}

			// ATE: Refresh APs
			CalcNewActionPoints( pSoldier );

			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				if ( pSoldier->bTeam == CIV_TEAM && pSoldier->bNeutral )
				{
					// only set precombat gridno if unset
					if ( gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo == 0 ||
						gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo == NOWHERE )
					{
						gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo = pSoldier->sGridNo;
					}
				}
				else
				{
					gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo = NOWHERE;
				}
			}

			if ( !gTacticalStatus.fHasEnteredCombatModeSinceEntering )
			{
				// ATE: reset player's movement mode at the very start of
				// combat
				//if ( pSoldier->bTeam == OUR_TEAM )
				//{
				//	pSoldier->usUIMovementMode = RUNNING;
				//}
			}
		}
	}

	gTacticalStatus.fHasEnteredCombatModeSinceEntering = TRUE;

	SyncStrategicTurnTimes( );

	// Play tune..
	PlayJA2Sample(ENDTURN_1, MIDVOLUME, 1, MIDDLEPAN);

	// Say quote.....

	// Change music modes
	SetMusicMode( MUSIC_TACTICAL_BATTLE );

}


void EnterCombatMode( UINT8 ubStartingTeam )
{
	if ( gTacticalStatus.uiFlags & INCOMBAT )
	{
		SLOGD("Can't enter combat when already in combat" );
		// we're already in combat!
		return;
	}

	// Alrighty, don't do this if no enemies in sector
	if ( NumCapableEnemyInSector( ) == 0 )
	{
		SLOGD("Can't enter combat when no capable enemies" );
		return;
	}

	SLOGD("Entering combat mode" );

	// ATE: Added here to guarentee we have fEnemyInSector
	// Mostly this was not getting set if:
	// 1 ) we see bloodcats ( which makes them hostile )
	// 2 ) we make civs hostile
	// only do this once they are seen.....
	SetEnemyPresence();

	CommonEnterCombatModeCode( );

	if (ubStartingTeam == OUR_TEAM)
	{
		// OK, make sure we have a selected guy
		const SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel == NULL || sel->bOppCnt == 0)
		{
			// OK, look through and find one....
			FOR_EACH_IN_TEAM(s, OUR_TEAM)
			{
				if (OkControllableMerc(s) && s->bOppCnt > 0)
				{
					SelectSoldier(s, SELSOLDIER_FORCE_RESELECT);
				}
			}
		}

		StartPlayerTeamTurn( FALSE, TRUE );
	}
	else
	{
		// have to call EndTurn so that we freeze the interface etc
		EndTurn( ubStartingTeam );
	}


}



void ExitCombatMode( )
{
	SLOGD("Exiting combat mode" );

	// Leave combat mode
	gTacticalStatus.uiFlags &= (~INCOMBAT);

	EndTopMessage( );

	// OK, we have exited combat mode.....
	// Reset some flags for no aps to move, etc

	// Set virgin sector to true....
	gTacticalStatus.fVirginSector = TRUE;

	FOR_EACH_SOLDIER(pSoldier)
	{
		SoldierSP soldier = GetSoldier(pSoldier);
		if ( pSoldier->bInSector )
		{
			// Reset some flags
			if ( pSoldier->fNoAPToFinishMove && pSoldier->bLife >= OKLIFE )
			{
				AdjustNoAPToFinishMove( pSoldier, FALSE );
				SoldierGotoStationaryStance( pSoldier );
			}

			soldier->removePendingAnimation();

			// Reset moved flag
			pSoldier->bMoved = FALSE;

			// Set final destination
			pSoldier->sFinalDestination = pSoldier->sGridNo;

			// remove AI controlled flag
			pSoldier->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
		}
	}

	// Change music modes
	gfForceMusicToTense = TRUE;

	SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

	BetweenTurnsVisibilityAdjustments();

	// pause the AI for a bit
	PauseAITemporarily();

	// reset muzzle flashes
	TurnOffEveryonesMuzzleFlashes();

	// zap interrupt list
	ClearIntList();

	fInterfacePanelDirty = DIRTYLEVEL2;

	// ATE: If we are IN_CONV - DONT'T DO THIS!
	if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
	{
		HandleStrategicTurnImplicationsOfExitingCombatMode();
	}

	// Make sure next opplist decay DOES happen right after we go to RT
	// since this would be the same as what would happen at the end of the turn
	gTacticalStatus.uiTimeSinceLastOpplistDecay = std::max(UINT32(0), GetWorldTotalSeconds() - TIME_BETWEEN_RT_OPPLIST_DECAYS);
	NonCombatDecayPublicOpplist( GetWorldTotalSeconds() );
}


void SetEnemyPresence( )
{
	// We have an ememy present....

	// Check if we previously had no enemys present and we are in a virgin secotr ( no enemys spotted yet )
	if ( !gTacticalStatus.fEnemyInSector && gTacticalStatus.fVirginSector )
	{
		// If we have a guy selected, say quote!
		// For now, display ono status message
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ ENEMY_IN_SECTOR_STR ] );

		// Change music modes..

		// If we are just starting game, don't do this!
		if ( !DidGameJustStart() && !AreInMeanwhile( ) )
		{
			SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );
		}

		// Say quote...
		//SayQuoteFromAnyBodyInSector( QUOTE_ENEMY_PRESENCE );

		gTacticalStatus.fEnemyInSector = TRUE;
	}
}


static bool SoldierHasSeenEnemiesLastFewTurns(SOLDIERTYPE const& s)
{
	for (INT32 team = 0; team != MAXTEAMS; ++team)
	{
		if (gTacticalStatus.Team[team].bSide == s.bSide) continue;

		// Check this team for possible enemies
		CFOR_EACH_IN_TEAM(i, team)
		{
			SOLDIERTYPE const& other = *i;
			if (!other.bInSector)                                   continue;
			if (other.bTeam != OUR_TEAM && other.bLife < OKLIFE) continue;
			if (CONSIDERED_NEUTRAL(&s, &other))                     continue;
			if (s.bSide == other.bSide)                             continue;
			// Have we not seen this guy
			INT8 const seen = s.bOppList[other.ubID];
			if (seen < SEEN_CURRENTLY || SEEN_THIS_TURN < seen)     continue;

			gTacticalStatus.bConsNumTurnsNotSeen = 0;
			return true;
		}
	}

	return false;
}


static BOOLEAN WeSeeNoOne(void)
{
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const s = *i;
		if (s->bTeam == OUR_TEAM && s->bOppCnt > 0) return FALSE;
	}
	return TRUE;
}


static BOOLEAN NobodyAlerted(void)
{
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const s = *i;
		if (s->bTeam != OUR_TEAM &&
			!s->bNeutral &&
			s->bLife >= OKLIFE &&
			s->bAlertStatus >= STATUS_RED)
		{
			return FALSE;
		}
	}
	return( TRUE );
}


static BOOLEAN WeSawSomeoneThisTurn(void)
{
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const s = *i;
		if (s->bTeam != OUR_TEAM) continue;

		for (UINT32 uiLoop2 = gTacticalStatus.Team[ENEMY_TEAM].bFirstID; uiLoop2 < MAX_NUM_SOLDIERS; ++uiLoop2)
		{
			if (s->bOppList[uiLoop2] == SEEN_THIS_TURN) return TRUE;
		}
	}
	return FALSE;
}


static void SayBattleSoundFromAnyBodyInSector(BattleSound const iBattleSnd)
{
	UINT8	ubNumMercs = 0;

	// Loop through all our guys and randomly say one from someone in our sector
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Add guy if he's a candidate...
		if (OkControllableMerc(s) &&
			!AM_AN_EPC(s) &&
			!(s->uiStatusFlags & SOLDIER_GASSED) &&
			!AM_A_ROBOT(s) &&
			!s->fMercAsleep)
		{
			mercs_in_sector[ubNumMercs++] = s;
		}
	}

	if ( ubNumMercs > 0 )
	{
		SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];
		DoMercBattleSound(chosen, iBattleSnd);
	}
}


static UINT8 NumBloodcatsInSectorNotDeadOrDying();
static UINT8 NumEnemyInSectorNotDeadOrDying();


BOOLEAN CheckForEndOfCombatMode( BOOLEAN fIncrementTurnsNotSeen )
{
	BOOLEAN	fWeSeeNoOne, fNobodyAlerted;
	BOOLEAN	fSayQuote = FALSE;
	BOOLEAN fWeSawSomeoneRecently = FALSE, fSomeoneSawSomeoneRecently = FALSE;

	// We can only check for end of combat if in combat mode
	if ( ! ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		return( FALSE );
	}

	// if we're boxing NEVER end combat mode
	if ( gTacticalStatus.bBoxingState == BOXING )
	{
		return( FALSE );
	}

	// First check for end of battle....
	// If there are no enemies at all in the sector
	// Battle end should take presedence!
	if ( CheckForEndOfBattle( FALSE ) )
	{
		return( TRUE );
	}

	fWeSeeNoOne = WeSeeNoOne();
	fNobodyAlerted = NobodyAlerted();
	if ( fWeSeeNoOne && fNobodyAlerted )
	{
		// hack!!!
		gTacticalStatus.bConsNumTurnsNotSeen = 5;
	}
	else
	{
		// we have to loop through EVERYONE to see if anyone sees a hostile... if so, stay in turnbased...
		FOR_EACH_MERC(i)
		{
			SOLDIERTYPE const& s = **i;
			if (s.bLife >= OKLIFE &&
				!s.bNeutral &&
				SoldierHasSeenEnemiesLastFewTurns(s))
			{
				gTacticalStatus.bConsNumTurnsNotSeen = 0;
				fSomeoneSawSomeoneRecently = TRUE;
				if (s.bTeam == OUR_TEAM ||
					(s.bTeam == MILITIA_TEAM && s.bSide == 0)) // or friendly militia
				{
					fWeSawSomeoneRecently = TRUE;
					break;
				}
			}
		}

		if ( fSomeoneSawSomeoneRecently )
		{
			if ( fWeSawSomeoneRecently )
			{
				gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 0;
			}
			else
			{
				// start tracking this
				gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes++;
			}
			return( FALSE );
		}

		// IF here, we don;t see anybody.... increment count for end check
		if( fIncrementTurnsNotSeen )
		{
			gTacticalStatus.bConsNumTurnsNotSeen++;
		}
	}

	gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 0;

	// If we have reach a point where a cons. number of turns gone by....
	if ( gTacticalStatus.bConsNumTurnsNotSeen > 1 )
	{
		gTacticalStatus.bConsNumTurnsNotSeen = 0;

		// Exit mode!
		ExitCombatMode();


		if ( fNobodyAlerted )
		{
			// if we don't see anyone currently BUT we did see someone this turn, THEN don't
			// say quote
			if ( fWeSeeNoOne && WeSawSomeoneThisTurn() )
			{
				// don't say quote
			}
			else
			{
				fSayQuote = TRUE;
			}
		}
		else
		{
			fSayQuote = TRUE;
		}

		// ATE: Are there creatures here? If so, say another quote...
		if ( fSayQuote && ( gTacticalStatus.uiFlags & IN_CREATURE_LAIR ) )
		{
			SayQuoteFromAnyBodyInSector( QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE );
		}
		// Are we fighting bloodcats?
		else if ( NumBloodcatsInSectorNotDeadOrDying( ) > 0 )
		{
		}
		else
		{
			if (fSayQuote)
			{
				// Double check by seeing if we have any active enemies in sector!
				if (NumEnemyInSectorNotDeadOrDying() > 0)
				{
					SayQuoteFromAnyBodyInSector(QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT);
				}
			}
		}
		/*
		if ( (!fWeSeeNoOne || !fNobodyAlerted) && WeSawSomeoneThisTurn() )
		{
			// Say quote to the effect that the battle has lulled
			SayQuoteFromAnyBodyInSector( QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT );
		}*/

		// Begin tense music....
		gfForceMusicToTense = TRUE;
		SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

		return( TRUE );
	}

	return( FALSE );
}


static void DeathNoMessageTimerCallback(void)
{
	CheckAndHandleUnloadingOfCurrentWorld();
}


static BOOLEAN CheckForLosingEndOfBattle(void);
static bool  KillIncompacitatedEnemyInSector();
static UINT8 NumEnemyInSectorExceptCreatures();


//!!!!
//IMPORTANT NEW NOTE:
//Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
BOOLEAN CheckForEndOfBattle( BOOLEAN fAnEnemyRetreated )
{
	BOOLEAN fBattleWon = TRUE;
	BOOLEAN fBattleLost = FALSE;
	UINT16  usAnimState;

	if ( gTacticalStatus.bBoxingState == BOXING )
	{
		// no way are we going to exit boxing prematurely, thank you! :-)
		return( FALSE );
	}

	// We can only check for end of battle if in combat mode or there are enemies
	// present (they might bleed to death or run off the map!)
	if ( ! ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		if ( ! (gTacticalStatus.fEnemyInSector) )
		{
			return( FALSE );
		}
	}

	// ATE: If attack busy count.. get out...
	if ( (gTacticalStatus.ubAttackBusyCount > 0 ) )
	{
		return( FALSE );
	}


	// OK, this is to releave infinate looping...becasue we can kill guys in this function
	if ( gfKillingGuysForLosingBattle )
	{
		return( FALSE );
	}

	// Check if the battle is won!
	if ( NumCapableEnemyInSector( ) > 0 )
	{
		fBattleWon = FALSE;
	}

	if (CheckForLosingEndOfBattle())
	{
		fBattleLost = TRUE;
	}

	//NEW (Nov 24, 98)  by Kris
	if (!gWorldSector.z && fBattleWon)
	{
		//Check to see if more enemy soldiers exist in the strategic layer
		//It is possible to have more than 20 enemies in a sector.  By failing here,
		//it gives the engine a chance to add these soldiers as reinforcements.  This
		//is naturally handled.
		if( gfPendingEnemies )
		{
			fBattleWon = FALSE;
		}
	}

	// We should NEVER have a battle lost and won at the same time...

	if ( fBattleLost )
	{
		// CJC: End AI's turn here.... first... so that UnSetUIBusy will succeed if militia win
		// battle for us
		EndAllAITurns( );

		// Set enemy presence to false
		// This is safe 'cause we're about to unload the friggen sector anyway....
		gTacticalStatus.fEnemyInSector = FALSE;

		// If here, the battle has been lost!
		UnSetUIBusy(GetSelectedMan());

		if ( gTacticalStatus.uiFlags & INCOMBAT )
		{
			// Exit mode!
			ExitCombatMode();
		}

		HandleMoraleEvent(NULL, MORALE_HEARD_BATTLE_LOST, gWorldSector);
		HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_BATTLE_LOST, gWorldSector);

		// Play death music
		SetMusicMode( MUSIC_TACTICAL_DEFEAT );
		SetCustomizableTimerCallbackAndDelay( 10000, DeathNoMessageTimerCallback, FALSE );

		if ( CheckFact( FACT_FIRST_BATTLE_BEING_FOUGHT, 0 ) )
		{
			// this is our first battle... and we lost it!
			SetFactTrue( FACT_FIRST_BATTLE_FOUGHT );
			SetFactFalse( FACT_FIRST_BATTLE_BEING_FOUGHT );
			SetTheFirstBattleSector(gWorldSector.AsStrategicIndex());
			HandleFirstBattleEndingWhileInTown(gWorldSector, FALSE);
		}

		if( NumEnemyInSectorExceptCreatures() )
		{
			SetThisSectorAsEnemyControlled(gWorldSector);
		}

		// ATE: Important! THis is delayed until music ends so we can have proper effect!
		// CheckAndHandleUnloadingOfCurrentWorld();

		//Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
		LogBattleResults( LOG_DEFEAT );
		gfBlitBattleSectorLocator = FALSE;
		return( TRUE );
	}


	// If battle won, do stuff right away!
	if ( fBattleWon )
	{

		if ( gTacticalStatus.bBoxingState == NOT_BOXING ) // if boxing don't do any of this stuff
		{
			gTacticalStatus.fLastBattleWon = TRUE;

			// OK, KILL any enemies that are incompacitated
			if ( KillIncompacitatedEnemyInSector( ) )
			{
				return( FALSE );
			}
		}

		// If here, the battle has been won!
		// hurray! a glorious victory!

		// Set enemy presence to false
		gTacticalStatus.fEnemyInSector = FALSE;

		// CJC: End AI's turn here.... first... so that UnSetUIBusy will succeed if militia win
		// battle for us
		EndAllAITurns( );

		UnSetUIBusy(GetSelectedMan());

		// ATE:
		// If we ended battle in any team other than the player's
		// we need to end the UI lock using this method....
		guiPendingOverrideEvent = LU_ENDUILOCK;
		HandleTacticalUI( );

		if ( gTacticalStatus.uiFlags & INCOMBAT )
		{
			// Exit mode!
			ExitCombatMode();
		}

		if ( gTacticalStatus.bBoxingState == NOT_BOXING ) // if boxing don't do any of this stuff
		{

			// Only do some stuff if we actually faught a battle
			if ( gTacticalStatus.bNumFoughtInBattle[ ENEMY_TEAM ] +
				gTacticalStatus.bNumFoughtInBattle[ CREATURE_TEAM ] +
				gTacticalStatus.bNumFoughtInBattle[ CIV_TEAM ] > 0 )
			//if ( gTacticalStatus.bNumEnemiesFoughtInBattle > 0 )
			{
				FOR_EACH_IN_TEAM(pTeamSoldier, OUR_TEAM)
				{
					if (pTeamSoldier->bInSector &&
						pTeamSoldier->bTeam == OUR_TEAM)
					{
						gMercProfiles[pTeamSoldier->ubProfile].usBattlesFought++;

						// If this guy is OKLIFE & not standing, make stand....
						if (pTeamSoldier->bLife >= OKLIFE &&
							!pTeamSoldier->bCollapsed &&
							pTeamSoldier->bAssignment < ON_DUTY)
						{
							// Reset some quote flags....
							pTeamSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_BUDDY_1_WITNESSED;
							pTeamSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_BUDDY_2_WITNESSED;
							pTeamSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_BUDDY_3_WITNESSED;

							// toggle stealth mode....
							gfUIStanceDifferent = TRUE;
							pTeamSoldier->bStealthMode = FALSE;
							fInterfacePanelDirty = DIRTYLEVEL2;

							if (gAnimControl[pTeamSoldier->usAnimState].ubHeight != ANIM_STAND)
							{
								ChangeSoldierStance(pTeamSoldier, ANIM_STAND);
							}
							else
							{
								// If they are aiming, end aim!
								usAnimState = PickSoldierReadyAnimation(pTeamSoldier, TRUE);

								if (usAnimState != INVALID_ANIMATION)
								{
									EVENT_InitNewSoldierAnim(pTeamSoldier, usAnimState, 0, FALSE);
								}
							}
						}
					}
				}

				HandleMoraleEvent(nullptr, MORALE_BATTLE_WON, gWorldSector);
				HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_BATTLE_WON, gWorldSector);

				// Change music modes
				if (gLastMercTalkedAboutKilling == NULL ||
					!(gLastMercTalkedAboutKilling->uiStatusFlags & SOLDIER_MONSTER))
				{
					SetMusicMode(MUSIC_TACTICAL_VICTORY);
				}
				ShouldBeginAutoBandage();

				// Say battle end quote....

				if (fAnEnemyRetreated)
				{
					SayQuoteFromAnyBodyInSector( QUOTE_ENEMY_RETREATED );
				}
				else
				{
					// OK, If we have just finished a battle with creatures........ play killed creature quote...
					//
					if (gLastMercTalkedAboutKilling != NULL &&
						gLastMercTalkedAboutKilling->uiStatusFlags & SOLDIER_MONSTER)
					{

					}
					if (gLastMercTalkedAboutKilling != NULL &&
						gLastMercTalkedAboutKilling->ubBodyType == BLOODCAT)
					{
						SayBattleSoundFromAnyBodyInSector(BATTLE_SOUND_COOL1);
					}
					else
					{
						SayQuoteFromAnyBodyInSector(QUOTE_SECTOR_SAFE);
					}
				}

			}
			else
			{
				// Change to nothing music...
				SetMusicMode(MUSIC_TACTICAL_NOTHING);
				ShouldBeginAutoBandage();
			}

			// Loop through all militia and restore them to peaceful status
			FOR_EACH_IN_TEAM(pTeamSoldier, MILITIA_TEAM)
			{
				if (pTeamSoldier->bInSector)
				{
					pTeamSoldier->bAlertStatus = STATUS_GREEN;
					CheckForChangingOrders( pTeamSoldier );
					pTeamSoldier->sNoiseGridno = NOWHERE;
					pTeamSoldier->ubNoiseVolume = 0;
					pTeamSoldier->bNewSituation = FALSE;
					pTeamSoldier->bOrders = STATIONARY;
					if ( pTeamSoldier->bLife >= OKLIFE )
					{
						pTeamSoldier->bBleeding = 0;
					}
				}
			}
			gTacticalStatus.Team[ MILITIA_TEAM ].bAwareOfOpposition = FALSE;

			// Loop through all civs and restore them to peaceful status
			FOR_EACH_IN_TEAM(pTeamSoldier, CIV_TEAM)
			{
				if (pTeamSoldier->bInSector)
				{
					pTeamSoldier->bAlertStatus = STATUS_GREEN;
					pTeamSoldier->sNoiseGridno = NOWHERE;
					pTeamSoldier->ubNoiseVolume = 0;
					pTeamSoldier->bNewSituation = FALSE;
					CheckForChangingOrders( pTeamSoldier );
				}
			}
			gTacticalStatus.Team[ CIV_TEAM ].bAwareOfOpposition = FALSE;

		}

		fInterfacePanelDirty = DIRTYLEVEL2;

		if ( gTacticalStatus.bBoxingState == NOT_BOXING ) // if boxing don't do any of this stuff
		{
			LogBattleResults( LOG_VICTORY );

			SetThisSectorAsPlayerControlled(gWorldSector, TRUE);
			HandleVictoryInNPCSector(gWorldSector);
			if ( CheckFact( FACT_FIRST_BATTLE_BEING_FOUGHT, 0 ) )
			{
				// ATE: Need to trigger record for this event .... for NPC scripting
				TriggerNPCRecord( PACOS, 18 );

				// this is our first battle... and we won!
				SetFactTrue( FACT_FIRST_BATTLE_FOUGHT );
				SetFactTrue( FACT_FIRST_BATTLE_WON );
				SetFactFalse( FACT_FIRST_BATTLE_BEING_FOUGHT );
				SetTheFirstBattleSector(gWorldSector.AsStrategicIndex());
				HandleFirstBattleEndingWhileInTown(gWorldSector, FALSE);
			}
		}

		SetCustomizableTimerCallbackAndDelay(3000, HandleThePlayerBeNotifiedOfSomeoneElseInSector, FALSE);

		//Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
		gfBlitBattleSectorLocator = FALSE;
		return( TRUE );
	}

	return( FALSE );
}


void CycleThroughKnownEnemies( )
{
	// static to indicate last position we were at:
	static BOOLEAN fFirstTime = TRUE;
	static UINT16  usStartToLook;
	BOOLEAN        fEnemyBehindStartLook = FALSE;
	BOOLEAN        fEnemiesFound = FALSE;

	if ( fFirstTime )
	{
		fFirstTime = FALSE;

		usStartToLook = gTacticalStatus.Team[ OUR_TEAM ].bLastID;
	}

	FOR_EACH_NON_PLAYER_SOLDIER(s)
	{
		// try to find first active, OK enemy
		if (s->bInSector &&
			!s->bNeutral &&
			s->bSide != OUR_TEAM &&
			s->bLife > 0 &&
			s->bVisible != -1)
		{
			fEnemiesFound = TRUE;

			// If we are > ok start, this is the one!
			if (s->ubID > usStartToLook)
			{
				usStartToLook = s->ubID;
				SlideTo(s, SETANDREMOVEPREVIOUSLOCATOR);
				return;
			}
			else
			{
				fEnemyBehindStartLook = TRUE;
			}
		}
	}

	if ( !fEnemiesFound )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_ENEMIES_IN_SIGHT_STR ] );
	}

	// If here, we found nobody, but there may be someone behind
	// If to, recurse!
	if ( fEnemyBehindStartLook )
	{
		usStartToLook = gTacticalStatus.Team[ OUR_TEAM ].bLastID;

		CycleThroughKnownEnemies( );
	}
}


void CycleVisibleEnemies( SOLDIERTYPE *pSrcSoldier )
{
	FOR_EACH_NON_PLAYER_SOLDIER(s)
	{
		// try to find first active, OK enemy
		if (s->bInSector &&
			!s->bNeutral &&
			s->bSide != OUR_TEAM &&
			s->bLife > 0 &&
			pSrcSoldier->bOppList[s->ubID] == SEEN_CURRENTLY &&
			s->ubID > pSrcSoldier->ubLastEnemyCycledID)
		{
			pSrcSoldier->ubLastEnemyCycledID = s->ubID;
			SlideTo(s, SETANDREMOVEPREVIOUSLOCATOR);
			ChangeInterfaceLevel(s->bLevel);
			return;
		}
	}

	// If here.. reset to zero...
	pSrcSoldier->ubLastEnemyCycledID = 0;

	FOR_EACH_NON_PLAYER_SOLDIER(s)
	{
		// try to find first active, OK enemy
		if (s->bInSector &&
			!s->bNeutral &&
			s->bSide != OUR_TEAM &&
			s->bLife > 0 &&
			pSrcSoldier->bOppList[s->ubID] == SEEN_CURRENTLY &&
			s->ubID > pSrcSoldier->ubLastEnemyCycledID)
		{
			pSrcSoldier->ubLastEnemyCycledID = s->ubID;
			SlideTo(s, SETANDREMOVEPREVIOUSLOCATOR);
			ChangeInterfaceLevel(s->bLevel);
			return;
		}
	}
}


UINT NumberOfMercsOnPlayerTeam(void)
{
	INT8 bNumber = 0;
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (!(s->uiStatusFlags & SOLDIER_VEHICLE)) bNumber++;
	}
	return bNumber;
}


BOOLEAN PlayerTeamFull( )
{
	// last ID for the player team is 19, so long as we have at most 17 non-vehicles...
	if (NumberOfMercsOnPlayerTeam() <= gTacticalStatus.Team[OUR_TEAM].bLastID - 2u)
	{
		return( FALSE );
	}

	return( TRUE );
}


UINT8 NumPCsInSector(void)
{
	UINT8 ubNumPlayers = 0;
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const s = *i;
		if (s->bTeam == OUR_TEAM && s->bLife > 0) ++ubNumPlayers;
	}
	return ubNumPlayers;
}


UINT8 NumEnemyInSector()
{
	UINT8 n_enemies = 0;
	CFOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE const& s = *i;
		if (!s.bInSector) continue;
		if (s.bLife <= 0) continue;
		if (s.bNeutral)   continue;
		if (s.bSide == 0) continue;
		++n_enemies;
	}
	return n_enemies;
}


static UINT8 NumEnemyInSectorExceptCreatures()
{
	UINT8 n_enemies = 0;
	CFOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE const& s = *i;
		if (!s.bInSector)             continue;
		if (s.bLife <= 0)             continue;
		if (s.bNeutral)               continue;
		if (s.bSide == 0)             continue;
		if (s.bTeam == CREATURE_TEAM) continue;
		++n_enemies;
	}
	return n_enemies;
}


static UINT8 NumEnemyInSectorNotDeadOrDying()
{
	UINT8 n_enemies = 0;
	CFOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE const& s = *i;
		if (!s.bInSector)                   continue;
		if (s.uiStatusFlags & SOLDIER_DEAD) continue;
		// Also, we want to pick up unconcious guys as NOT being capable, but we
		// want to make sure we don't get those ones that are in the process of
		// dying
		if (s.bLife < OKLIFE) continue;
		if (s.bNeutral)       continue;
		if (s.bSide == 0)     continue;
		++n_enemies;
	}
	return n_enemies;
}


static UINT8 NumBloodcatsInSectorNotDeadOrDying()
{
	UINT8 n_enemies = 0;
	CFOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE const& s = *i;
		if (!s.bInSector)                   continue;
		if (s.ubBodyType != BLOODCAT)       continue;
		if (s.uiStatusFlags & SOLDIER_DEAD) continue;
		// Also, we want to pick up unconcious guys as NOT being capable, but we
		// want to make sure we don't get those ones that are in the process of
		// dying
		if (s.bLife < OKLIFE) continue;
		if (s.bNeutral)       continue;
		if (s.bSide == 0)     continue;
		++n_enemies;
	}
	return n_enemies;
}


UINT8 NumCapableEnemyInSector()
{
	UINT8 n_enemies = 0;
	CFOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE const& s = *i;
		if (!s.bInSector)                   continue;
		if (s.uiStatusFlags & SOLDIER_DEAD) continue;
		// Also, we want to pick up unconcious guys as NOT being capable, but we
		// want to make sure we don't get those ones that are in the process of
		// dying
		if (s.bLife < OKLIFE && s.bLife != 0) continue;
		if (s.bNeutral)   continue;
		if (s.bSide == 0) continue;
		++n_enemies;
	}
	return n_enemies;
}


static void DeathTimerCallback(void);


static BOOLEAN CheckForLosingEndOfBattle(void)
{
	INT8    bNumDead = 0, bNumNotOK = 0, bNumInBattle = 0, bNumNotOKRealMercs = 0;
	BOOLEAN fMadeCorpse;
	BOOLEAN fDoCapture = FALSE;
	BOOLEAN fOnlyEPCsLeft = TRUE;
	BOOLEAN fMilitiaInSector = FALSE;

	// ATE: Check for MILITIA - we won't lose if we have some.....
	CFOR_EACH_IN_TEAM(s, MILITIA_TEAM)
	{
		if (s->bInSector && s->bSide == OUR_TEAM && s->bLife >= OKLIFE)
		{
			// We have at least one poor guy who will still fight....
			// we have not lost ( yet )!
			fMilitiaInSector = TRUE;
		}
	}

	CFOR_EACH_IN_TEAM(pTeamSoldier, OUR_TEAM)
	{
		// Are we in sector.....
		if (pTeamSoldier->bInSector && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE))
		{
			bNumInBattle++;

			if ( pTeamSoldier->bLife == 0 )
			{
				bNumDead++;
			}
			else if ( pTeamSoldier->bLife < OKLIFE )
			{
				bNumNotOK++;

				if ( !AM_AN_EPC( pTeamSoldier ) && !AM_A_ROBOT( pTeamSoldier ) )
				{
					bNumNotOKRealMercs++;
				}
			}
			else
			{
				if ( !AM_A_ROBOT( pTeamSoldier ) || !AM_AN_EPC( pTeamSoldier ) )
				{
					fOnlyEPCsLeft = FALSE;
				}
			}
		}
	}


	// OK< check ALL in battle, if that matches SUM of dead, incompacitated, we're done!
	if ( ( bNumDead + bNumNotOK ) == bNumInBattle || fOnlyEPCsLeft )
	{
		// Are there militia in sector?
		if ( fMilitiaInSector )
		{
			if( guiCurrentScreen != AUTORESOLVE_SCREEN )
			{
				// if here, check if we should autoresolve.
				// if we have at least one guy unconscious, call below function...
				if ( HandlePotentialBringUpAutoresolveToFinishBattle( ) )
				{
					// return false here as we are autoresolving...
					return( FALSE );
				}
			}
			else
			{
				return( FALSE );
			}
		}


		// Bring up box if we have ANY guy incompaciteded.....
		if ( bNumDead != bNumInBattle )
		{
			// If we get captured...
			// Your unconscious mercs are captured!

			// Check if we should get captured....
			if ( bNumNotOKRealMercs < 4 && bNumNotOKRealMercs > 1 )
			{
				// Check if any enemies exist....
				if (IsTeamActive(ENEMY_TEAM))
				{
					//if( GetWorldDay() > STARTDAY_ALLOW_PLAYER_CAPTURE_FOR_RESCUE && !( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE ))
					{
						if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED || ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE && gubQuest[ QUEST_INTERROGATION ] == QUESTNOTSTARTED ) )
						{
							fDoCapture = TRUE;
							// CJC Dec 1 2002: fix capture sequences
							BeginCaptureSquence();
						}
					}
				}
			}

			gfKillingGuysForLosingBattle = TRUE;

			// Kill them now...
			FOR_EACH_IN_TEAM(i, OUR_TEAM)
			{
				SOLDIERTYPE& s = *i;
				// Are we in sector.....
				if (s.bInSector)
				{
					if ((s.bLife != 0 && s.bLife < OKLIFE) || AM_AN_EPC(&s) || AM_A_ROBOT(&s))
					{
						// Captured EPCs or ROBOTS will be kiiled in capture routine....
						if ( !fDoCapture )
						{
							// Kill!
							s.bLife = 0;

							HandleSoldierDeath(&s, &fMadeCorpse);
						}
					}

					// ATE: if we are told to do capture....
					if (s.bLife != 0 && fDoCapture)
					{
						EnemyCapturesPlayerSoldier(&s);
						RemoveSoldierFromTacticalSector(s);
					}
				}
			}

			gfKillingGuysForLosingBattle = FALSE;

			if ( fDoCapture )
			{
				EndCaptureSequence( );
				SetCustomizableTimerCallbackAndDelay( 3000, CaptureTimerCallback, FALSE );
			}
			else
			{
				SetCustomizableTimerCallbackAndDelay( 10000, DeathTimerCallback, FALSE );
			}

		}
		return( TRUE );
	}

	return( FALSE );
}


static bool KillIncompacitatedEnemyInSector()
{
	bool ret = false;
	FOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE& s = *i;
		if (!s.bInSector)                   continue;
		if (s.bLife >= OKLIFE)              continue;
		if (s.uiStatusFlags & SOLDIER_DEAD) continue;
		if (s.bNeutral)                     continue;
		if (s.bSide == OUR_TEAM)         continue;
		// Kill
		SoldierTakeDamage(&s, s.bLife, 100, TAKE_DAMAGE_BLOODLOSS, 0);
		ret = true;
	}
	return ret;
}


static INT8 CalcSuppressionTolerance(SOLDIERTYPE* pSoldier)
{
	INT8 bTolerance;

	// Calculate basic tolerance value
	bTolerance = pSoldier->bExpLevel * 2;
	if (pSoldier->uiStatusFlags & SOLDIER_PC)
	{
		// give +1 for every 10% morale from 50, for a maximum bonus/penalty of 5.
		bTolerance += ( pSoldier->bMorale - 50 ) / 10;
	}
	else
	{
		// give +2 for every morale category from normal, for a max change of 4
		bTolerance += ( pSoldier->bAIMorale - MORALE_NORMAL ) * 2;
	}

	if ( pSoldier->ubProfile != NO_PROFILE )
	{
		// change tolerance based on attitude
		switch ( gMercProfiles[ pSoldier->ubProfile ].bAttitude )
		{
			case ATT_AGGRESSIVE:
				bTolerance +=  2;
				break;
			case ATT_COWARD:
				bTolerance += -2;
				break;
			default:
				break;
		}
	}
	else
	{
		// generic NPC/civvie; change tolerance based on attitude
		switch ( pSoldier->bAttitude )
		{
			case BRAVESOLO:
			case BRAVEAID:
				bTolerance +=  2;
				break;
			case AGGRESSIVE:
				bTolerance +=  1;
				break;
			case DEFENSIVE:
				bTolerance += -1;
				break;
			default:
				break;
		}
	}

	if (bTolerance < 0)
	{
		bTolerance = 0;
	}

	return( bTolerance );
}


#define MAX_APS_SUPPRESSED 8


static void HandleSuppressionFire(const SOLDIERTYPE* const targeted_merc, SOLDIERTYPE* const caused_attacker)
{
	INT8  bTolerance;
	INT16 sClosestOpponent, sClosestOppLoc;
	UINT8 ubPointsLost, ubTotalPointsLost, ubNewStance;
	UINT8 ubLoop2;

	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pSoldier = *i;
		if (IS_MERC_BODY_TYPE(pSoldier) && pSoldier->bLife >= OKLIFE && pSoldier->ubSuppressionPoints > 0)
		{
			bTolerance = CalcSuppressionTolerance( pSoldier );

			// multiply by 2, add 1 and divide by 2 to round off to nearest whole number
			ubPointsLost = ( ((pSoldier->ubSuppressionPoints * 6) / (bTolerance + 6)) * 2 + 1 ) / 2;

			// reduce loss of APs based on stance
			// ATE: Taken out because we can possibly supress ourselves...
			//switch (gAnimControl[ pSoldier->usAnimState ].ubEndHeight)
			//{
			//	case ANIM_PRONE:
			//		ubPointsLost = ubPointsLost * 2 / 4;
			//		break;
			//	case ANIM_CROUCH:
			//		ubPointsLost = ubPointsLost * 3 / 4;
			//		break;
			//	default:
			//		break;
			//}

			// cap the # of APs we can lose
			if (ubPointsLost > MAX_APS_SUPPRESSED)
			{
				ubPointsLost = MAX_APS_SUPPRESSED;
			}

			ubTotalPointsLost = ubPointsLost;

			// Subtract off the APs lost before this point to find out how many points are lost now
			if (pSoldier->ubAPsLostToSuppression >= ubPointsLost)
			{
				continue;
			}

			// morale modifier
			if (ubTotalPointsLost / 2 > pSoldier->ubAPsLostToSuppression / 2)
			{
				for ( ubLoop2 = 0; ubLoop2 < (ubTotalPointsLost / 2) - (pSoldier->ubAPsLostToSuppression / 2); ubLoop2++ )
				{
					HandleMoraleEvent(pSoldier, MORALE_SUPPRESSED, pSoldier->sSector);
				}
			}

			ubPointsLost -= pSoldier->ubAPsLostToSuppression;
			ubNewStance = 0;

			// merc may get to react
			if ( pSoldier->ubSuppressionPoints >= ( 130 / (6 + bTolerance) ) )
			{
				// merc gets to use APs to react!
				switch (gAnimControl[ pSoldier->usAnimState ].ubEndHeight)
				{
					case ANIM_PRONE:
						// can't change stance below prone!
						break;
					case ANIM_CROUCH:
						if (ubTotalPointsLost >= AP_PRONE && IsValidStance( pSoldier, ANIM_PRONE ) )
						{
							sClosestOpponent = ClosestKnownOpponent( pSoldier, &sClosestOppLoc, NULL );
							if (sClosestOpponent == NOWHERE || SpacesAway( pSoldier->sGridNo, sClosestOppLoc ) > 8)
							{
								if (ubPointsLost < AP_PRONE)
								{
									// Have to give APs back so that we can change stance without
									// losing more APs
									pSoldier->bActionPoints += (AP_PRONE - ubPointsLost);
									ubPointsLost = 0;
								}
								else
								{
									ubPointsLost -= AP_PRONE;
								}
								ubNewStance = ANIM_PRONE;
							}
						}
						break;
					default: // standing!
						if (pSoldier->bOverTerrainType == LOW_WATER || pSoldier->bOverTerrainType == DEEP_WATER)
						{
							// can't change stance here!
							break;
						}
						else if (ubTotalPointsLost >= (AP_CROUCH + AP_PRONE) && ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_PRONE ) && IsValidStance( pSoldier, ANIM_PRONE ) )
						{
							sClosestOpponent = ClosestKnownOpponent( pSoldier, &sClosestOppLoc, NULL );
							if ( sClosestOpponent == NOWHERE || SpacesAway( pSoldier->sGridNo, sClosestOppLoc ) > 8 )
							{
								if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND )
								{
									// can only crouch for now
									ubNewStance = ANIM_CROUCH;
								}
								else
								{
									// lie prone!
									ubNewStance = ANIM_PRONE;
								}
							}
							else if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND && IsValidStance( pSoldier, ANIM_CROUCH ) )
							{
								// crouch, at least!
								ubNewStance = ANIM_CROUCH;
							}
						}
						else if ( ubTotalPointsLost >= AP_CROUCH && ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH ) && IsValidStance( pSoldier, ANIM_CROUCH ) )
						{
							// crouch!
							ubNewStance = ANIM_CROUCH;
						}
						break;
				}
			}

			// Reduce action points!
			pSoldier->bActionPoints -= ubPointsLost;
			pSoldier->ubAPsLostToSuppression = ubTotalPointsLost;

			if (pSoldier->uiStatusFlags & SOLDIER_PC && pSoldier->ubSuppressionPoints > 8 && pSoldier == targeted_merc)
			{
				if ( !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_BEING_PUMMELED ) )
				{
					pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_BEING_PUMMELED;
					// say we're under heavy fire!

					// ATE: For some reason, we forgot #53!
					if ( pSoldier->ubProfile != 53 )
					{
						TacticalCharacterDialogue( pSoldier, QUOTE_UNDER_HEAVY_FIRE );
					}
				}
			}

			if ( ubNewStance != 0 )
			{
				// This person is going to change stance

				// This person will be busy while they crouch or go prone
				if (gTacticalStatus.uiFlags & INCOMBAT)
				{
					SLOGD("Starting suppression, on {}", pSoldier->ubID);

					gTacticalStatus.ubAttackBusyCount++;

					// make sure supressor ID is the same!
					pSoldier->suppressor = caused_attacker;
				}
				pSoldier->fChangingStanceDueToSuppression = TRUE;
				pSoldier->fDontChargeAPsForStanceChange = TRUE;

				// AI people will have to have their actions cancelled
				if (!(pSoldier->uiStatusFlags & SOLDIER_PC))
				{
					CancelAIAction(pSoldier);
					pSoldier->bAction = AI_ACTION_CHANGE_STANCE;
					pSoldier->usActionData = ubNewStance;
					pSoldier->bActionInProgress = TRUE;
				}

				// go for it!
				// ATE: Cancel any PENDING ANIMATIONS...
				pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
				// ATE: Turn off non-interrupt flag ( this NEEDS to be done! )
				pSoldier->fInNonintAnim = FALSE;
				pSoldier->fRTInNonintAnim = FALSE;

				ChangeSoldierStance( pSoldier, ubNewStance );
			}

		} // end of examining one soldier
	} // end of loop
}


BOOLEAN ProcessImplicationsOfPCAttack(SOLDIERTYPE* const pSoldier, SOLDIERTYPE* const pTarget, const INT8 bReason)
{
	BOOLEAN	fEnterCombat = TRUE;

	if ( pTarget->fAIFlags & AI_ASLEEP )
	{
		// waaaaaaaaaaaaake up!
		pTarget->fAIFlags &= (~AI_ASLEEP);
	}

	if (pTarget->ubProfile == PABLO && CheckFact( FACT_PLAYER_FOUND_ITEMS_MISSING, 0 ) )
	{
		SetFactTrue( FACT_PABLO_PUNISHED_BY_PLAYER );
	}

	if ( gTacticalStatus.bBoxingState == BOXING )
	{
		// should have a check for "in boxing ring", no?
		if ( ( pSoldier->usAttackingWeapon != NOTHING &&
			pSoldier->usAttackingWeapon != BRASS_KNUCKLES ) || !( pSoldier->uiStatusFlags & SOLDIER_BOXER ) )
		{
			// someone's cheating!
			if ( (GCM->getItem(pSoldier->usAttackingWeapon)->getItemClass() == IC_BLADE ||
				GCM->getItem(pSoldier->usAttackingWeapon)->getItemClass() == IC_PUNCH) && (pTarget->uiStatusFlags & SOLDIER_BOXER) )
			{
				// knife or brass knuckles disqualify the player!
				BoxingPlayerDisqualified( pSoldier, BAD_ATTACK );
			}
			else
			{
				// anything else is open war!
				//gTacticalStatus.bBoxingState = NOT_BOXING;
				SetBoxingState( NOT_BOXING );
				// if we are attacking a boxer we should set them to neutral (temporarily) so that the rest of the civgroup code works...
				if ( (pTarget->bTeam == CIV_TEAM) && (pTarget->uiStatusFlags & SOLDIER_BOXER) )
				{
					SetSoldierNeutral( pTarget );
				}
			}

		}
	}

	if ( (pTarget->bTeam == MILITIA_TEAM) && (pTarget->bSide == OUR_TEAM) )
	{
		// rebel militia attacked by the player!
		MilitiaChangesSides();
	}
	// JA2 Gold: fix Slay
	else if (pTarget->bTeam == CIV_TEAM &&
		pTarget->bNeutral &&
		pTarget->ubProfile == SLAY &&
		pTarget->bLife >= OKLIFE &&
		!CheckFact(FACT_155, 0))
	{
		TriggerNPCRecord( SLAY, 1 );
	}
	else if ( (pTarget->bTeam == CIV_TEAM) &&
		(pTarget->ubCivilianGroup == 0) &&
		(pTarget->bNeutral) && !( pTarget->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		static const SGPSector hicksFarm(10, MAP_ROW_F);
		if (pTarget->ubBodyType == COW && gWorldSector == hicksFarm)
		{
			// hicks could get mad!!!
			HickCowAttacked( pSoldier, pTarget );
		}
		else if (pTarget->ubProfile == PABLO && pTarget->bLife >= OKLIFE && CheckFact(FACT_PABLO_PUNISHED_BY_PLAYER, 0) && !CheckFact(FACT_38, 0))
		{
			TriggerNPCRecord( PABLO, 3 );
		}
		else
		{
			// regular civ attacked, turn non-neutral
			AddToShouldBecomeHostileOrSayQuoteList(pTarget);

			if ( pTarget->ubProfile == NO_PROFILE || !(gMercProfiles[ pTarget->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH) )
			{
				// militia, if any, turn hostile
				MilitiaChangesSides();
			}
		}
	}
	else
	{
		if (pTarget->ubProfile == CARMEN)// Carmen
		{
			// Special stuff for Carmen the bounty hunter
			if (pSoldier->ubProfile != SLAY) // attacked by someone other than Slay
			{
				// change attitude
				pTarget->bAttitude = AGGRESSIVE;
			}
		}

		if ( pTarget->ubCivilianGroup && ( (pTarget->bTeam == OUR_TEAM) || pTarget->bNeutral ) )
		{
			if (pTarget->uiStatusFlags & SOLDIER_PC)
			{
				SLOGD("{} is changing teams", pTarget->name);
			}
			// member of a civ group, either recruited or neutral, so should
			// change sides individually or all together

			CivilianGroupMemberChangesSides( pTarget );

			if (pTarget->ubProfile != NO_PROFILE && pTarget->bLife >= OKLIFE && pTarget->bVisible == TRUE)
			{
				// trigger quote!
				PauseAITemporarily();
				AddToShouldBecomeHostileOrSayQuoteList(pTarget);
				//TriggerNPCWithIHateYouQuote( pTarget->ubProfile );
			}
		}
		else if ( pTarget->ubCivilianGroup != NON_CIV_GROUP && !( pTarget->uiStatusFlags & SOLDIER_BOXER ) )
		{
			// Firing at a civ in a civ group who isn't hostile... if anyone in that civ group can see this
			// going on they should become hostile.
			CivilianGroupMembersChangeSidesWithinProximity( pTarget );
		}
		else if ( pTarget->bTeam == OUR_TEAM && !(gTacticalStatus.uiFlags & INCOMBAT) )
		{
			// firing at one of our own guys who is not a rebel etc
			if ( pTarget->bLife >= OKLIFE && !(pTarget->bCollapsed) && !AM_A_ROBOT( pTarget ) && (bReason == REASON_NORMAL_ATTACK ) )
			{
				// OK, sturn towards the prick
				// Change to fire ready animation

				pTarget->fDontChargeReadyAPs = TRUE;
				SoldierReadyWeapon(pTarget, pSoldier->sGridNo, FALSE);

				// ATE: Depending on personality, fire back.....

				// Do we have a gun in a\hand?
				if ( GCM->getItem(pTarget->inv[ HANDPOS ].usItem)->getItemClass() == IC_GUN )
				{
					// Toggle burst capable...
					if ( !pTarget->bDoBurst )
					{
						if (IsGunBurstCapable(pTarget, HANDPOS))
						{
							ChangeWeaponMode( pTarget );
						}
					}

					// Fire back!
					HandleItem( pTarget, pSoldier->sGridNo, pSoldier->bLevel, pTarget->inv[ HANDPOS ].usItem, FALSE );

				}

			}

			// don't enter combat on attack on one of our own mercs
			fEnterCombat = FALSE;
		}

		// if we've attacked a miner
		if ( IsProfileAHeadMiner( pTarget->ubProfile ) )
		{
			PlayerAttackedHeadMiner( pTarget->ubProfile );
		}
	}

	return( fEnterCombat );
}


static SOLDIERTYPE* InternalReduceAttackBusyCount(SOLDIERTYPE* const pSoldier, const BOOLEAN fCalledByAttacker, SOLDIERTYPE* pTarget)
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	BOOLEAN fEnterCombat = FALSE;

	if (pSoldier == NULL)
	{
		pTarget = NULL;
	}
	else
	{
		if (pTarget == NULL)
		{
			SLOGD("Target ptr is null!");
		}
	}

	if (fCalledByAttacker)
	{
		if (pSoldier && GCM->getItem(pSoldier->inv[HANDPOS].usItem)->isGun())
		{
			if (pSoldier->bBulletsLeft > 0)
			{
				return( pTarget );
			}
		}
	}

	//if (gTacticalStatus.uiFlags & INCOMBAT)
	//{

	if ( gTacticalStatus.ubAttackBusyCount == 0 )
	{
		// ATE: We have a problem here... if testversion, report error......
		// But for all means.... DON'T wrap!
		if ( (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			SLOGD("Problem with attacker busy count decrementing past 0.... preventing wrap-around.");
		}
	}
	else
	{
		gTacticalStatus.ubAttackBusyCount--;
	}

	SLOGD("Ending attack, attack count now {}", gTacticalStatus.ubAttackBusyCount);
	//}

	if (gTacticalStatus.ubAttackBusyCount > 0)
	{
		return( pTarget );
	}

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		// Check to see if anyone was suppressed
		const SOLDIERTYPE* const target = (pSoldier == NULL ? NULL : pSoldier->target);
		HandleSuppressionFire(target, pSoldier);

		//HandleAfterShootingGuy( pSoldier, pTarget );

		// suppression fire might cause the count to be increased, so check it again
		if (gTacticalStatus.ubAttackBusyCount > 0)
		{
			SLOGD("Starting suppression, attack count now {}", gTacticalStatus.ubAttackBusyCount);
			return( pTarget );
		}
	}

	// ATE: IN MEANWHILES, we have 'combat' in realtime....
	// this is so we DON'T call freeupattacker() which will cancel
	// the AI guy's meanwhile NPC stuff.
	// OK< let's NOT do this if it was the queen attacking....
	if ( AreInMeanwhile( ) && pSoldier != NULL && pSoldier->ubProfile != QUEEN )
	{
		return( pTarget );
	}

	if (pTarget)
	{
		// reset # of shotgun pellets hit by
		pTarget->bNumPelletsHitBy = 0;
		// reset flag for making "ow" sound on being shot
	}

	if (pSoldier)
	{
		// reset attacking hand
		pSoldier->ubAttackingHand = HANDPOS;

		// if there is a valid target, and our attack was noticed
		if ( pTarget && (pSoldier->uiStatusFlags & SOLDIER_ATTACK_NOTICED) )
		{
			// stuff that only applies to when we attack
			if ( pTarget->ubBodyType != CROW )
			{
				if ( pSoldier->bTeam == OUR_TEAM )
				{
					fEnterCombat = ProcessImplicationsOfPCAttack(pSoldier, pTarget, REASON_NORMAL_ATTACK);
					if ( !fEnterCombat )
					{
						SLOGD("Not entering combat as a result of PC attack" );
					}
				}
			}

			// global

			// ATE: If we are an animal, etc, don't change to hostile...
			// ( and don't go into combat )
			if ( pTarget->ubBodyType == CROW )
			{
				// Loop through our team, make guys who can see this fly away....
				const UINT8 ubTeam = pTarget->bTeam;
				FOR_EACH_IN_TEAM(s, ubTeam)
				{
					if (s->bInSector &&
						s->ubBodyType == CROW &&
						s->bOppList[pSoldier->ubID] == SEEN_CURRENTLY)
					{
						//ZEROTIMECOUNTER(s->AICounter);
						//MakeCivHostile(s, 2);
						HandleCrowFlyAway(s);
					}
				}

				// Don't enter combat...
				fEnterCombat = FALSE;
			}

			if ( gTacticalStatus.bBoxingState == BOXING )
			{
				if ( pTarget && pTarget->bLife <= 0 )
				{
					// someone has won!
					EndBoxingMatch( pTarget );
				}
			}


			// if soldier and target were not both players and target was not under fire before...
			if ( ( pSoldier->bTeam != OUR_TEAM || pTarget->bTeam != OUR_TEAM ) )
			{
				if (pTarget->bOppList[ pSoldier->ubID ] != SEEN_CURRENTLY )
				{
					NoticeUnseenAttacker( pSoldier, pTarget, 0 );
				}
				// "under fire" lasts for 2 turns
				pTarget->bUnderFire = 2;
			}

		}
		else if ( pTarget )
		{
			// something is wrong here!
			if ( !pTarget->bActive || !pTarget->bInSector )
			{
				SLOGD("Invalid target attacked!" );
			}
			else if ( ! (pSoldier->uiStatusFlags & SOLDIER_ATTACK_NOTICED) )
			{
				SLOGD("Attack not noticed" );
			}
		}
		else
		{
			// no target, don't enter combat
			fEnterCombat = FALSE;
		}

		if ( pSoldier->fSayAmmoQuotePending )
		{
			pSoldier->fSayAmmoQuotePending = FALSE;
			TacticalCharacterDialogue( pSoldier, QUOTE_OUT_OF_AMMO );
		}

		if (pSoldier->uiStatusFlags & SOLDIER_PC)
		{
			UnSetUIBusy(pSoldier);
		}
		else
		{
			FreeUpNPCFromAttacking(pSoldier);
		}

		if ( !fEnterCombat )
		{
			SLOGD("Not to enter combat from this attack" );
		}


		if ( fEnterCombat && !( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			// Go into combat!

			// If we are in a meanwhile... don't enter combat here...
			if ( !AreInMeanwhile( ) )
			{
				EnterCombatMode( pSoldier->bTeam );
			}
		}

		pSoldier->uiStatusFlags &= (~SOLDIER_ATTACK_NOTICED);
	}

	TacticalStatusType* const ts = &gTacticalStatus;
	if (ts->fKilledEnemyOnAttack)
	{
		// Check for death quote...
		HandleKilledQuote(ts->enemy_killed_on_attack, ts->enemy_killed_on_attack_killer, ts->ubEnemyKilledOnAttackLocation, ts->bEnemyKilledOnAttackLevel);
		ts->fKilledEnemyOnAttack = FALSE;
	}

	// ATE: Check for stat changes....
	HandleAnyStatChangesAfterAttack( );


	if ( gTacticalStatus.fItemsSeenOnAttack  && gTacticalStatus.ubCurrentTeam == OUR_TEAM )
	{
		gTacticalStatus.fItemsSeenOnAttack = FALSE;

		// Display quote!
		SOLDIERTYPE* const s = gTacticalStatus.items_seen_on_attack_soldier;
		if (!AM_AN_EPC(s))
		{
			MakeCharacterDialogueEventSignalItemLocatorStart(*s, gTacticalStatus.usItemsSeenOnAttackGridNo);
		}
		else
		{
			// Turn off item lock for locators...
			gTacticalStatus.fLockItemLocators = FALSE;
			// Slide to location!
			SlideToLocation(gTacticalStatus.usItemsSeenOnAttackGridNo);
		}
	}

	if ( gTacticalStatus.uiFlags & CHECK_SIGHT_AT_END_OF_ATTACK )
	{
		AllTeamsLookForAll( FALSE );

		// call fov code
		FOR_EACH_IN_TEAM(pSightSoldier, OUR_TEAM)
		{
			if (pSightSoldier->bInSector)
			{
				RevealRoofsAndItems(pSightSoldier, FALSE);
			}
		}
		gTacticalStatus.uiFlags &= ~CHECK_SIGHT_AT_END_OF_ATTACK;
	}

	DequeueAllDemandGameEvents();

	CheckForEndOfBattle( FALSE );

	// if we're in realtime, turn off the attacker's muzzle flash at this point
	if ( !(gTacticalStatus.uiFlags & INCOMBAT) && pSoldier )
	{
		EndMuzzleFlash( pSoldier );
	}

	if ( pSoldier && pSoldier->bWeaponMode == WM_ATTACHED )
	{
		// change back to single shot
		pSoldier->bWeaponMode = WM_NORMAL;
	}

	// record last target
	// Check for valid target!
	if ( pSoldier )
	{
		pSoldier->sLastTarget = pSoldier->sTargetGridNo;
	}

	return( pTarget );
}


SOLDIERTYPE* ReduceAttackBusyCount(SOLDIERTYPE* const attacker, const BOOLEAN fCalledByAttacker)
{
	SOLDIERTYPE* const target = (attacker == NULL ? NULL : attacker->target);
	SLOGD("Reducing Attack Busy Count of {}", attacker != NULL ? attacker->ubID : -1);
	return InternalReduceAttackBusyCount(attacker, fCalledByAttacker, target);
}


SOLDIERTYPE* FreeUpAttacker(SOLDIERTYPE* const attacker)
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	return ReduceAttackBusyCount(attacker, TRUE);
}


SOLDIERTYPE* FreeUpAttackerGivenTarget(SOLDIERTYPE* const target)
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	return InternalReduceAttackBusyCount(target->attacker, TRUE, target);
}


SOLDIERTYPE* ReduceAttackBusyGivenTarget(SOLDIERTYPE* const target)
{
	// Strange as this may seem, this function returns a pointer to
	// the *target* in case the target has changed sides as a result
	// of being attacked
	return InternalReduceAttackBusyCount(target->attacker, FALSE, target);
}


void ResetAllMercSpeeds(void)
{
	FOR_EACH_SOLDIER(s)
	{
		if (s->bInSector) SetSoldierAniSpeed(s);
	}
}


void SetActionToDoOnceMercsGetToLocation(UINT8 ubActionCode, INT8 bNumMercsWaiting)
{
	gubWaitingForAllMercsToExitCode = ubActionCode;
	gbNumMercsUntilWaitingOver      = bNumMercsWaiting;

	// Setup timer counter and report back if too long....
	guiWaitingForAllMercsToExitTimer = GetJA2Clock();

	// ATE: Set flag to ignore sight...
	gTacticalStatus.uiFlags |= DISALLOW_SIGHT;
}


static void HandleBloodForNewGridNo(const SOLDIERTYPE* pSoldier)
{
	// Handle bleeding...
	if ( ( pSoldier->bBleeding > MIN_BLEEDING_THRESHOLD ) )
	{
		INT8 bBlood = (pSoldier->bBleeding - MIN_BLEEDING_THRESHOLD) / BLOODDIVISOR;
		if (bBlood > MAXBLOODQUANTITY) bBlood = MAXBLOODQUANTITY;

		// now, he shouldn't ALWAYS bleed the same amount; LOWER it perhaps. If it
		// goes less than zero, then no blood!
		bBlood -= (INT8)Random(7);

		if (bBlood >= 0)
		{
			// this handles all soldiers' dropping blood during movement
			DropBlood(*pSoldier, bBlood);
		}
	}
}

void CencelAllActionsForTimeCompression( void )
{
	FOR_EACH_SOLDIER(s)
	{
		if (!s->bInSector) continue;

		// Hault!
		EVENT_StopMerc(s);

		// END AI actions
		CancelAIAction(s);
	}
}

void AddManToTeam( INT8 bTeam )
{
	// ATE: If not loading game!
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// Increment men in sector number!
		gTacticalStatus.Team[ bTeam ].bMenInSector++;
	}
}


void RemoveManFromTeam(const INT8 bTeam)
{
	// ATE; if not loading game!
	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;
	if (!IsTeamActive(bTeam))
	{
		SLOGD("Number of people on team {} dropped to {}", bTeam, gTacticalStatus.Team[bTeam].bMenInSector);
		return;
	}
	--gTacticalStatus.Team[bTeam].bMenInSector;
}


void RemoveSoldierFromTacticalSector(SOLDIERTYPE& s)
{
	// reset merc's opplist
	InitSoldierOppList(s);

	// Remove!
	RemoveSoldierFromGridNo(s);

	RemoveMercSlot(&s);

	s.bInSector = FALSE;

	// Select next avialiable guy....
	if (guiCurrentScreen == GAME_SCREEN)
	{
		if (GetSelectedMan() == &s)
		{
			SOLDIERTYPE* const next = FindNextActiveAndAliveMerc(&s, FALSE, FALSE);
			if (next != &s)
			{
				SelectSoldier(next, SELSOLDIER_NONE);
			}
			else
			{
				// OK - let's look for another squad...
				SOLDIERTYPE* const pNewSoldier = FindNextActiveSquad(&s);
				if (pNewSoldier != &s)
				{
					// Good squad found!
					SelectSoldier(pNewSoldier, SELSOLDIER_NONE);
				}
				else
				{
					// if here, make nobody
					SetSelectedMan(NULL);
				}
			}
		}
		UpdateTeamPanelAssignments();
	}
	else
	{
		SetSelectedMan(NULL);
	}
}

static void EndBattleWithUnconsciousGuysCallback(MessageBoxReturnValue const bExitValue)
{
	// Enter mapscreen.....
	CheckAndHandleUnloadingOfCurrentWorld();
}


void InitializeTacticalStatusAtBattleStart(void)
{
	gTacticalStatus.ubArmyGuysKilled = 0;

	gTacticalStatus.fPanicFlags = 0;
	gTacticalStatus.fEnemyFlags = 0;
	for (INT8 i = 0; i < NUM_PANIC_TRIGGERS; ++i)
	{
		gTacticalStatus.sPanicTriggerGridNo[i]  = NOWHERE;
		gTacticalStatus.bPanicTriggerIsAlarm[i] = FALSE;
		gTacticalStatus.ubPanicTolerance[i]     = 0;
	}

	for (INT32 i = 0; i < MAXTEAMS; ++i)
	{
		gTacticalStatus.Team[i].last_merc_to_radio = NULL;
		gTacticalStatus.Team[i].bAwareOfOpposition = FALSE;
	}

	gTacticalStatus.the_chosen_one = NULL;

	ClearIntList();

	// make sure none of our guys have leftover shock values etc
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		s->bShock      = 0;
		s->bTilesMoved = 0;
	}

	// loop through everyone; clear misc flags
	FOR_EACH_SOLDIER(s)
	{
		s->ubMiscSoldierFlags = 0;
	}
}


static void DeathTimerCallback(void)
{
	ST::string text;
	if (gTacticalStatus.Team[CREATURE_TEAM].bMenInSector > gTacticalStatus.Team[ENEMY_TEAM].bMenInSector)
	{
		text = LargeTacticalStr[LARGESTR_NOONE_LEFT_CAPABLE_OF_BATTLE_AGAINST_CREATURES_STR];
	}
	else
	{
		text = LargeTacticalStr[LARGESTR_NOONE_LEFT_CAPABLE_OF_BATTLE_STR];
	}
	DoMessageBox(MSG_BOX_BASIC_STYLE, text, GAME_SCREEN, MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL);
}


void CaptureTimerCallback(void)
{
	ST::string text;
	if (gfSurrendered)
	{
		text = LargeTacticalStr[3];
	}
	else
	{
		text = LargeTacticalStr[LARGESTR_HAVE_BEEN_CAPTURED];
	}
	DoMessageBox(MSG_BOX_BASIC_STYLE, text, GAME_SCREEN, MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL);
	gfSurrendered = FALSE;
}


void DoPOWPathChecks(void)
{
	// loop through all mercs on our team and if they are POWs in sector, do POW
	// path check and put on a squad if available
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (!s->bInSector || s->bAssignment != ASSIGNMENT_POW) continue;

		// check to see if POW has been freed!
		// this will be true if a path can be made from the POW to either of 3 gridnos
		// 10492 (hallway) or 10482 (outside), or 9381 (outside)
		if (!FindBestPath(s, 10492, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE) &&
				!FindBestPath(s, 10482, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE) &&
				!FindBestPath(s,  9381, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE))
		{
			continue;
		}
		// free! free!
		// put them on any available squad
		s->bNeutral = FALSE;
		AddCharacterToAnySquad(s);
		DoMercBattleSound(s, BATTLE_SOUND_COOL1);
	}
}


BOOLEAN HostileCiviliansPresent( void )
{
	if (!IsTeamActive(CIV_TEAM)) return FALSE;

	CFOR_EACH_IN_TEAM(s, CIV_TEAM)
	{
		if (s->bInSector && s->bLife > 0 && !s->bNeutral)
		{
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN HostileBloodcatsPresent(void)
{
	if (!IsTeamActive(CREATURE_TEAM)) return FALSE;

	CFOR_EACH_IN_TEAM(s, CREATURE_TEAM)
	{
		// KM : Aug 11, 1999 -- Patch fix:  Removed the check for bNeutral.
		// Bloodcats automatically become hostile on sight.  Because the check used
		// to be there, it was possible to get into a 2nd battle elsewhere which is
		// BAD BAD BAD!
		if (s->bInSector && s->bLife > 0 && s->ubBodyType == BLOODCAT)
		{
			return TRUE;
		}
	}

	return FALSE;
}


static void DoCreatureTensionQuote(SOLDIERTYPE* pSoldier);


static void HandleCreatureTenseQuote(void)
{
	// Check for quote seeing creature attacks....
	if (gubQuest[QUEST_CREATURES] == QUESTDONE) return;
	if (!(gTacticalStatus.uiFlags & IN_CREATURE_LAIR)) return;
	if (gTacticalStatus.uiFlags & INCOMBAT) return;

	UINT32 uiTime = GetJA2Clock();
	if (uiTime - gTacticalStatus.uiCreatureTenseQuoteLastUpdate > (UINT32)(gTacticalStatus.sCreatureTenseQuoteDelay * 1000))
	{
		gTacticalStatus.uiCreatureTenseQuoteLastUpdate = uiTime;

		// run through list
		UINT8	ubNumMercs = 0;
		SOLDIERTYPE* mercs_in_sector[20];
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			// Add guy if he's a candidate...
			if (OkControllableMerc(s) &&
				!AM_AN_EPC(s) &&
				!(s->uiStatusFlags & SOLDIER_GASSED) &&
				!AM_A_ROBOT(s) &&
				!s->fMercAsleep)
			{
				mercs_in_sector[ubNumMercs++] = s;
			}
		}

		if (ubNumMercs > 0)
		{
			SOLDIERTYPE* const chosen = mercs_in_sector[Random(ubNumMercs)];
			DoCreatureTensionQuote(chosen);
		}

		// Adjust delay....
		gTacticalStatus.sCreatureTenseQuoteDelay = 60 + Random(60);
	}
}


static void DoCreatureTensionQuote(SOLDIERTYPE* s)
{
	// Check for playing smell quote....
	INT32  quote;
	UINT16 quote_flag;
	const INT32 iRandomQuote = Random(3);
	switch (iRandomQuote)
	{
		case 0:
			quote      = QUOTE_SMELLED_CREATURE;
			quote_flag = SOLDIER_QUOTE_SAID_SMELLED_CREATURE;
			break;

		case 1:
			quote      = QUOTE_TRACES_OF_CREATURE_ATTACK;
			quote_flag = SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK;
			break;

		case 2:
			quote      = QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE;
			quote_flag = SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES;
			break;

		default: abort(); // HACK000E
	}
	if (s->usQuoteSaidFlags & quote_flag) return;
	s->usQuoteSaidFlags |= quote_flag;
	TacticalCharacterDialogue(s, quote);
}


void MakeCharacterDialogueEventSignalItemLocatorStart(SOLDIERTYPE& s, GridNo const location)
{
	class CharacterDialogueEventSignalItemLocatorStart : public CharacterDialogueEvent
	{
		public:
			CharacterDialogueEventSignalItemLocatorStart(SOLDIERTYPE& s, GridNo const location) :
				CharacterDialogueEvent(s),
				location_(location)
			{}

			bool Execute()
			{
				if (!MayExecute()) return true;

				// Turn off item lock for locators
				gTacticalStatus.fLockItemLocators = FALSE;

				SlideToLocation(location_);

				SOLDIERTYPE& s = soldier_;
				ExecuteCharacterDialogue(s.ubProfile, QUOTE_SPOTTED_SOMETHING_ONE + Random(2), s.face, DIALOGUE_TACTICAL_UI, TRUE, false);

				return false;
			}

		private:
			GridNo const location_;
	};

	DialogueEvent::Add(new CharacterDialogueEventSignalItemLocatorStart(s, location));
}

void HandleThePlayerBeNotifiedOfSomeoneElseInSector(void)
{
	//Is someone important is in this sector
	if (!WildernessSectorWithAllProfiledNPCsNotSpokenWith(gWorldSector))
	{
		return;
	}

	//if something else is going on, come back later
	if (gTacticalStatus.fAutoBandageMode ||
		DialogueActive() ||
		gTacticalStatus.fAutoBandagePending ||
		guiCurrentScreen == MSG_BOX_SCREEN ||
		AreWeInAUIMenu()
		)
	{
		SetCustomizableTimerCallbackAndDelay(2000, HandleThePlayerBeNotifiedOfSomeoneElseInSector, FALSE);
		return;
	}

	DoMessageBox(MSG_BOX_BASIC_STYLE, *(GCM->getNewString(NS_SOMEONE_ELSE_IN_SECTOR)), GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
}

static void HandleBrothelWallDestroyed(INT16 const sSectorX, INT16 const sSectorY, INT8 const sSectorZ, INT16 const sGridNo, STRUCTURE* const s, UINT8 const ubDamage, BOOLEAN const fDestroyed)
{
    // CJC, Sept 16: if we destroy any wall of the brothel, make Kingpin's men hostile!
    if (sSectorX == 5 && sSectorY == MAP_ROW_C && sSectorZ == 0 && s->fFlags & STRUCTURE_WALLSTUFF && fDestroyed)
    {
        UINT8 room = GetRoom(sGridNo);
        if (room == NO_ROOM) room = GetRoom(sGridNo + DirectionInc(SOUTH));
        if (room == NO_ROOM) room = GetRoom(sGridNo + DirectionInc(EAST));
        if (room != NO_ROOM && IN_BROTHEL(room))
        {
            CivilianGroupChangesSides(KINGPIN_CIV_GROUP);
        }
    }
}

#ifdef WITH_UNITTESTS
#undef FAIL
#include "gtest/gtest.h"

TEST(Overhead, asserts)
{
	EXPECT_EQ(lengthof(g_default_team_info), static_cast<size_t>(MAXTEAMS));
}

#endif
