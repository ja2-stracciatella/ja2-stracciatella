#include "Handle_Items.h"
#include "ItemModel.h"
#include "Items.h"
#include "Soldier_Find.h"
#include "Structure_Internals.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "Weapons.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Handle_UI.h"
#include "Animation_Control.h"
#include "Points.h"
#include "Sound_Control.h"
#include "Isometric_Utils.h"
#include "Animation_Data.h"
#include "UI_Cursors.h"
#include "LOS.h"
#include "Interface.h"
#include "Cursors.h"
#include "Cursor_Control.h"
#include "Structure_Wrap.h"
#include "Physics.h"
#include "Soldier_Macros.h"
#include "Text.h"
#include "Interactive_Tiles.h"
#include "PathAI.h"
#include "Debug.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"

#include <string_theory/string>


// FUNCTIONS FOR ITEM CURSOR HANDLING
static UICursorID HandleActivatedTargetCursor(   SOLDIERTYPE*, GridNo map_pos, BOOLEAN recalc);
static UICursorID HandleNonActivatedTargetCursor(SOLDIERTYPE*, GridNo map_pos, BOOLEAN show_APs, BOOLEAN fRecalc, MouseMoveState);
static UICursorID HandleKnifeCursor(             SOLDIERTYPE*, GridNo map_pos, BOOLEAN activated, MouseMoveState);
static UICursorID HandlePunchCursor(             SOLDIERTYPE*, GridNo map_pos, BOOLEAN activated, MouseMoveState);
static UICursorID HandleAidCursor(               SOLDIERTYPE*, GridNo map_pos, BOOLEAN activated, MouseMoveState);
static UICursorID HandleActivatedTossCursor();
static UICursorID HandleNonActivatedTossCursor(  SOLDIERTYPE*, GridNo map_pos, BOOLEAN recalc, MouseMoveState, ItemCursor);
static UICursorID HandleWirecutterCursor(        SOLDIERTYPE*, GridNo map_pos, MouseMoveState);
static UICursorID HandleRepairCursor(            SOLDIERTYPE*, GridNo map_pos, MouseMoveState);
static UICursorID HandleRefuelCursor(            SOLDIERTYPE*, GridNo map_pos, MouseMoveState);
static UICursorID HandleRemoteCursor(            SOLDIERTYPE*, BOOLEAN activated, MouseMoveState);
static UICursorID HandleBombCursor(              SOLDIERTYPE*, GridNo map_pos, BOOLEAN activated, MouseMoveState);
static UICursorID HandleJarCursor(               SOLDIERTYPE*, GridNo map_pos, MouseMoveState);
static UICursorID HandleTinCanCursor(            SOLDIERTYPE*, GridNo map_pos, MouseMoveState);


static BOOLEAN gfCannotGetThrough = FALSE;
static BOOLEAN gfDisplayFullCountRing = FALSE;
static int giHitChance = 0; // If value is -1 then we skip showing hit chance
static int giLastBodyLocationTargeted = 0;
static int giLastAimTime = 0;

BOOLEAN GetMouseRecalcAndShowAPFlags(MouseMoveState* const puiCursorFlags, BOOLEAN* const pfShowAPs)
{
	static bool do_new_tile = false;

	// Set flags for certain mouse movements
	MouseMoveState const cursor_flags = GetCursorMovementFlags();
	bool                 recalc       = false;

	// Force if we are currently cycling guys
	if (gfUIForceReExamineCursorData)
	{
		do_new_tile                  = true;
		recalc                       = true;
		gfUIForceReExamineCursorData = FALSE;
	}

	bool show_APs = false;
	if (cursor_flags != MOUSE_STATIONARY)
	{
		// If cursor was previously stationary, make the additional check of grid
		// pos change
		RESETCOUNTER(PATHFINDCOUNTER);
		do_new_tile = true;
	}
	else if (COUNTERDONE(PATHFINDCOUNTER, false)) // Only dipslay aps after a delay
	{
		// Don't reset counter: One when we move again do we do this!
		show_APs = true;

		if (do_new_tile)
		{
			do_new_tile = false;
			recalc      = true;
		}
	}

	if (puiCursorFlags) *puiCursorFlags = cursor_flags;
	if (pfShowAPs)      *pfShowAPs      = show_APs;
	return recalc;
}


// Functions for cursor determination
UICursorID GetProperItemCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const activated)
{
	MouseMoveState cursor_flags;
	BOOLEAN        show_APs;
	BOOLEAN const  recalc = GetMouseRecalcAndShowAPFlags(&cursor_flags, &show_APs);

	// ATE: Update attacking weapon!
	// CC has added this attackingWeapon stuff and I need to update it constantly
	// for CTGH algorithms
	if (gTacticalStatus.ubAttackBusyCount == 0)
	{
		UINT16 const in_hand = s->inv[HANDPOS].usItem;
		if (GCM->getItem(in_hand)->isWeapon()) s->usAttackingWeapon = in_hand;
	}

	UICursorID               cursor      = NO_UICURSOR;
	SOLDIERTYPE const* const tgt         = gUIFullTarget;
	GridNo             const tgt_grid_no = tgt ? tgt->sGridNo : map_pos;
	ItemCursor         const item_cursor = GetActionModeCursor(s);
	switch (item_cursor)
	{
		case TARGETCURS:
			cursor =
				activated ? HandleActivatedTargetCursor(s, tgt_grid_no, recalc) :
				HandleNonActivatedTargetCursor(s, tgt_grid_no, show_APs, recalc, cursor_flags);

			if (gCurrentUIMode == ACTION_MODE       &&
				gTacticalStatus.uiFlags & INCOMBAT  &&
				recalc                              &&
				tgt                                 &&
				IsValidTargetMerc(tgt)              &&
				EnoughAmmo(s, FALSE, HANDPOS)       && // ATE: Check for ammo
				guiUIFullTargetFlags & ENEMY_MERC   && // IF it's an ememy, goto confirm action mode
				guiUIFullTargetFlags & VISIBLE_MERC &&
				!(guiUIFullTargetFlags & DEAD_MERC) &&
				!gfCannotGetThrough)
			{
				guiPendingOverrideEvent = A_CHANGE_TO_CONFIM_ACTION;
			}
			break;

		case TOSSCURS:
		case TRAJECTORYCURS:
			cursor = activated && gfUIHandlePhysicsTrajectory ? HandleActivatedTossCursor() :
					HandleNonActivatedTossCursor(s, tgt_grid_no, recalc, cursor_flags, item_cursor);
			break;

		case PUNCHCURS:   cursor = HandlePunchCursor(     s, tgt_grid_no, activated, cursor_flags); break;
		case KNIFECURS:   cursor = HandleKnifeCursor(     s, tgt_grid_no, activated, cursor_flags); break;
		case AIDCURS:     cursor = HandleAidCursor(       s, map_pos,     activated, cursor_flags); break;
		case BOMBCURS:    cursor = HandleBombCursor(      s, tgt_grid_no, activated, cursor_flags); break;
		case REMOTECURS:  cursor = HandleRemoteCursor(    s,              activated, cursor_flags); break;
		case WIRECUTCURS: cursor = HandleWirecutterCursor(s, tgt_grid_no,            cursor_flags); break;
		case REPAIRCURS:  cursor = HandleRepairCursor(    s, tgt_grid_no,            cursor_flags); break;
		case JARCURS:     cursor = HandleJarCursor(       s, tgt_grid_no,            cursor_flags); break;
		case TINCANCURS:  cursor = HandleTinCanCursor(    s, tgt_grid_no,            cursor_flags); break;
		case REFUELCURS:  cursor = HandleRefuelCursor(    s, tgt_grid_no,            cursor_flags); break;
		case INVALIDCURS: cursor = INVALID_ACTION_UICURSOR; break;
		default:
			break;
	}

	return cursor;
}


static void DetermineCursorBodyLocation(SOLDIERTYPE*, BOOLEAN fDisplay, BOOLEAN fRecalc);


static UICursorID HandleActivatedTargetCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const recalc)
{
	bool const is_throwing_knife = GCM->getItem(s->inv[HANDPOS].usItem)->getItemClass() == IC_THROWING_KNIFE;
	if (is_throwing_knife)
	{
		// If we are in realtime, follow!
		if (!(gTacticalStatus.uiFlags & INCOMBAT) &&
			gAnimControl[s->usAnimState].uiFlags & ANIM_STATIONARY &&
			gUITargetShotWaiting)
		{
			guiPendingOverrideEvent = CA_MERC_SHOOT;
		}
	}

	// Determine where we are shooting/aiming
	DetermineCursorBodyLocation(s, TRUE, TRUE);

	bool enough_points       = true;
	bool max_point_limit_hit = false;
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		gsCurrentActionPoints         = CalcTotalAPsToAttack(s, map_pos, TRUE, s->bShownAimTime / 2);
		gfUIDisplayActionPoints       = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if (!EnoughPoints(s, gsCurrentActionPoints, 0 , FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
			max_point_limit_hit            = true;
		}
		else
		{
			UINT8 const future_aim = s->bShownAimTime + 2;
			if (future_aim <= REFINE_AIM_5)
			{
				INT16 const AP_costs = MinAPsToAttack(s, map_pos, TRUE) + future_aim / 2;
				if (!EnoughPoints(s, AP_costs, 0, FALSE))
				{
					enough_points = false;
				}
			}
		}
	}

	if (!(gTacticalStatus.uiFlags & INCOMBAT) && COUNTERDONE(TARGETREFINE))
	{
		if (s->bDoBurst)
		{
			s->bShownAimTime = REFINE_AIM_BURST;
		}
		else
		{
			++s->bShownAimTime;
			if (s->bShownAimTime > REFINE_AIM_5)
			{
				s->bShownAimTime = REFINE_AIM_5;
			}
			else if (s->bShownAimTime % 2 != 0)
			{
				PlayJA2Sample(TARG_REFINE_BEEP, MIDVOLUME, 1, MIDDLEPAN);
			}
		}
	}

	if (recalc)
	{
		SOLDIERTYPE const* const tgt    = gUIFullTarget;
		UINT8              const chance =
			tgt ? SoldierToSoldierBodyPartChanceToGetThrough(s, tgt, s->bAimShotLocation) :
			SoldierToLocationChanceToGetThrough(s, map_pos, gsInterfaceLevel, s->bTargetCubeLevel, 0);
		gfCannotGetThrough = chance < OK_CHANCE_TO_GET_THROUGH;
	}

	if(gamepolicy(show_hit_chance))
	{
		// Calculate chance to hit
		if (recalc || giLastBodyLocationTargeted != s->bAimShotLocation || giLastAimTime != s->bShownAimTime)
		{
			GridNo targetTile = gUIFullTarget ? gUIFullTarget->sGridNo : map_pos;
			giHitChance = is_throwing_knife ? CalcThrownChanceToHit(s, targetTile, s->bShownAimTime / 2, s->bAimShotLocation) :
				CalcChanceToHitGun(s, targetTile, s->bShownAimTime / 2, s->bAimShotLocation, false);
			giHitChance *= SoldierToLocationChanceToGetThrough(s, targetTile, gsInterfaceLevel, s->bTargetCubeLevel, 0) / 100.0f;
		}

		// Attach chance-to-hit to mouse cursor
		if(giHitChance != -1)
		{
			SetChanceToHitText(st_format_printf("%d%%", giHitChance));
			giLastBodyLocationTargeted = s->bAimShotLocation;
			giLastAimTime = s->bShownAimTime;
		}
	}

	UICursorID cursor = NO_UICURSOR;
	if (max_point_limit_hit)
	{
		// Check if we're in burst mode!
		cursor = s->bDoBurst       ? ACTION_TARGETREDBURST_UICURSOR :
			is_throwing_knife ? RED_THROW_UICURSOR             :
			ACTION_TARGETRED_UICURSOR;
	}
	else if (s->bDoBurst)
	{
		cursor = s->fDoSpread ? ACTION_TARGETREDBURST_UICURSOR :
			ACTION_TARGETCONFIRMBURST_UICURSOR;
	}
	else
	{
		switch (s->bShownAimTime)
		{
			case REFINE_AIM_1:
				if (is_throwing_knife)
				{
					cursor = gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW1_UICURSOR :
						enough_points          ? ACTION_THROWAIM1_UICURSOR       :
						ACTION_THROWAIMCANT1_UICURSOR;
				}
				else
				{
					cursor = gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW1_UICURSOR :
						enough_points          ? ACTION_TARGETAIM1_UICURSOR       :
						ACTION_TARGETAIMCANT1_UICURSOR;
				}
				break;

			case REFINE_AIM_2:
				if (is_throwing_knife)
				{
					cursor = gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW2_UICURSOR :
						enough_points          ? ACTION_THROWAIM3_UICURSOR       :
						ACTION_THROWAIMCANT2_UICURSOR;
				}
				else
				{
					cursor = gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW2_UICURSOR :
						enough_points          ? ACTION_TARGETAIM3_UICURSOR       :
						ACTION_TARGETAIMCANT2_UICURSOR;
				}
				break;

			case REFINE_AIM_3:
				if (is_throwing_knife)
				{
					cursor = gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW3_UICURSOR :
						enough_points          ? ACTION_THROWAIM5_UICURSOR       :
						ACTION_THROWAIMCANT3_UICURSOR;
				}
				else
				{
					cursor = gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW3_UICURSOR :
						enough_points          ? ACTION_TARGETAIM5_UICURSOR       :
						ACTION_TARGETAIMCANT3_UICURSOR;
				}
				break;

			case REFINE_AIM_4:
				if (is_throwing_knife)
				{
					cursor = gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW4_UICURSOR :
						enough_points          ? ACTION_THROWAIM7_UICURSOR       :
						ACTION_THROWAIMCANT4_UICURSOR;
				}
				else
				{
					cursor = gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW4_UICURSOR :
						enough_points          ? ACTION_TARGETAIM7_UICURSOR       :
						ACTION_TARGETAIMCANT4_UICURSOR;
				}
				break;

			case REFINE_AIM_5:
				if (is_throwing_knife)
				{
					cursor = gfDisplayFullCountRing ? ACTION_THROWAIMFULL_UICURSOR :
						enough_points          ? ACTION_THROWAIM9_UICURSOR    :
						ACTION_THROWAIMCANT5_UICURSOR;
				}
				else
				{
					cursor = gfDisplayFullCountRing ? ACTION_TARGETAIMFULL_UICURSOR :
						enough_points          ? ACTION_TARGETAIM9_UICURSOR    :
						ACTION_TARGETAIMCANT5_UICURSOR;
				}
				break;

			case REFINE_AIM_MID1: cursor = ACTION_TARGETAIM2_UICURSOR; break;
			case REFINE_AIM_MID2: cursor = ACTION_TARGETAIM4_UICURSOR; break;
			case REFINE_AIM_MID3: cursor = ACTION_TARGETAIM6_UICURSOR; break;
			case REFINE_AIM_MID4: cursor = ACTION_TARGETAIM8_UICURSOR; break;
		}
	}

	if (!max_point_limit_hit)
	{
		UINT16 const free_cursor_name = gUICursors[cursor].usFreeCursorName;
		RemoveCursorFlags(free_cursor_name, CURSOR_TO_FLASH | CURSOR_TO_PLAY_SOUND);
		if (gfCannotGetThrough)
		{
			SetCursorSpecialFrame(free_cursor_name, 1);
		}
		else if (!InRange(s, map_pos))
		{
			// OK, make buddy flash!
			SetCursorFlags(free_cursor_name, CURSOR_TO_FLASH | CURSOR_TO_PLAY_SOUND);
		}
		else
		{
			SetCursorSpecialFrame(free_cursor_name, 0);
		}
	}

	return cursor;
}


static UICursorID HandleNonActivatedTargetCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const show_APs, BOOLEAN const fRecalc, MouseMoveState const uiCursorFlags)
{
	bool const is_throwing_knife = GCM->getItem(s->inv[HANDPOS].usItem)->getItemClass() == IC_THROWING_KNIFE;
	if (!is_throwing_knife)
	{
		if (!(gTacticalStatus.uiFlags & INCOMBAT))
		{
			DetermineCursorBodyLocation(GetSelectedMan(), show_APs, fRecalc);
		}

		if (!EnoughAmmo(s, FALSE, HANDPOS))
		{
			// Check if ANY ammo exists
			if (FindAmmoToReload(s, HANDPOS, NO_SLOT) == NO_SLOT) return BAD_RELOAD_UICURSOR;

			gsCurrentActionPoints   = GetAPsToAutoReload(s);
			gfUIDisplayActionPoints = TRUE;
			return GOOD_RELOAD_UICURSOR;
		}
	}

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		DetermineCursorBodyLocation(GetSelectedMan(), show_APs, fRecalc);

		gsCurrentActionPoints = CalcTotalAPsToAttack(s, map_pos, TRUE, s->bShownAimTime / 2);

		gfUIDisplayActionPoints       = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		if (!show_APs)
		{
			gfUIDisplayActionPoints = FALSE;
		}
		else if (!EnoughPoints(s, gsCurrentActionPoints, 0 , FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}

	if (fRecalc)
	{
		gfCannotGetThrough = SoldierToLocationChanceToGetThrough(s, map_pos, gsInterfaceLevel, s->bTargetCubeLevel, 0) < OK_CHANCE_TO_GET_THROUGH;
	}

	// If we begin to move, reset the cursor
	if (uiCursorFlags != MOUSE_STATIONARY) gfCannotGetThrough = FALSE;

	if (gfCannotGetThrough)
	{
		return s->bDoBurst       ? ACTION_NOCHANCE_BURST_UICURSOR :
			is_throwing_knife ? BAD_THROW_UICURSOR             :
			ACTION_NOCHANCE_SHOOT_UICURSOR;
	}
	else if (!InRange(s, map_pos))
	{
		// Flash cursor!
		return s->bDoBurst       ? ACTION_FLASH_BURST_UICURSOR :
			is_throwing_knife ? FLASH_THROW_UICURSOR        :
			ACTION_FLASH_SHOOT_UICURSOR;
	}
	else
	{
		return s->bDoBurst       ? ACTION_TARGETBURST_UICURSOR :
			is_throwing_knife ? GOOD_THROW_UICURSOR         :
			ACTION_SHOOT_UICURSOR;
	}
}


static void DetermineCursorBodyLocation(SOLDIERTYPE* const s, BOOLEAN const display, BOOLEAN const recalc)
{
	if (gTacticalStatus.ubAttackBusyCount > 0)
	{
		// ATE: Return if attacker busy count > 0, this helps in RT with re-setting
		// the flag to random
		return;
	}

	if (recalc)
	{
		SGPPoint cursorPosition{};
		GetCursorPos(cursorPosition);

		// Always set aim location to nothing
		s->bAimShotLocation = AIM_SHOT_RANDOM;

		GridNo const map_pos = guiCurrentCursorGridNo;
		if (map_pos == NOWHERE) return;

		SOLDIERTYPE* tgt = 0;
		UINT16       flags;

		// Determine which body part it's on
		for (LEVELNODE* n = gpWorldLevelData[map_pos].pMercHead; n; n = n->pNext)
		{
			if (!(n->uiFlags & LEVELNODE_MERCPLACEHOLDER)) continue;

			SOLDIERTYPE* const potential_tgt = n->pSoldier;
			if (!potential_tgt) continue;

			// ATE: Check their stance - if prone - return!
			if (gAnimControl[potential_tgt->usAnimState].ubHeight == ANIM_PRONE)
			{
				return;
			}

			// Check if we have a half tile profile
			flags = n->uiAnimHitLocationFlags;
			if (flags & (TILE_FLAG_NORTH_HALF | TILE_FLAG_SOUTH_HALF | TILE_FLAG_WEST_HALF |
				TILE_FLAG_EAST_HALF | TILE_FLAG_TOP_HALF | TILE_FLAG_BOTTOM_HALF))
			{
				INT16 sCellX;
				INT16 sCellY;
				GetMouseWorldCoords(&sCellX, &sCellY);
				// We are only interested in the sub-tile coordinates
				sCellX %= CELL_X_SIZE;
				sCellY %= CELL_Y_SIZE;

				if (flags & TILE_FLAG_NORTH_HALF && sCellY >  CELL_Y_SIZE / 2) continue;
				if (flags & TILE_FLAG_SOUTH_HALF && sCellY <= CELL_Y_SIZE / 2) continue;
				if (flags & TILE_FLAG_WEST_HALF  && sCellX >  CELL_X_SIZE / 2) continue;
				if (flags & TILE_FLAG_EAST_HALF  && sCellX <= CELL_X_SIZE / 2) continue;

				if (flags & TILE_FLAG_TOP_HALF)
				{
					INT16 sScreenX;
					INT16 sScreenY;
					FromCellToScreenCoordinates(sCellX, sCellY, &sScreenX, &sScreenY);

					// Check for Below
					if (sScreenX > WORLD_TILE_Y / 2) continue;
				}

				if (flags & TILE_FLAG_BOTTOM_HALF)
				{
					INT16 sScreenX;
					INT16 sScreenY;
					FromCellToScreenCoordinates(sCellX, sCellY, &sScreenX, &sScreenY);

					// Check for Below
					if (sScreenX <= WORLD_TILE_Y / 2) continue;
				}
			}

			// Check if mouse is in bounding box of soldier
			if (!IsPointInSoldierBoundingBox(potential_tgt, cursorPosition.iX, cursorPosition.iY))
			{
				continue;
			}

			tgt = potential_tgt;
			break;
		}

		if (!tgt)
		{
			// Check if we can find a soldier here
			SOLDIERTYPE* const potential_tgt = gUIFullTarget;
			if (potential_tgt)
			{
				flags = FindRelativeSoldierPosition(potential_tgt, cursorPosition.iX, cursorPosition.iY);
				if (flags != 0) tgt = potential_tgt;
			}
		}

		if (tgt && IsValidTargetMerc(tgt))
		{
			if (flags & TILE_FLAG_FEET) s->bAimShotLocation = AIM_SHOT_LEGS;
			if (flags & TILE_FLAG_MID)  s->bAimShotLocation = AIM_SHOT_TORSO;
			if (flags & TILE_FLAG_HEAD) s->bAimShotLocation = AIM_SHOT_HEAD;
		}
	}

	if (!display)    return;
	if (s->bDoBurst) return;

	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (!tgt) return;

	ST::string hit_location;
	if (tgt->ubBodyType == CROW)
	{
		s->bAimShotLocation = AIM_SHOT_LEGS;
		hit_location = TacticalStr[CROW_HIT_LOCATION_STR];
	}
	else
	{
		if (!IS_MERC_BODY_TYPE(tgt)) return;

		switch (s->bAimShotLocation)
		{
			case AIM_SHOT_HEAD:
				hit_location = // If we have a knife in hand, change string
					GCM->getItem(s->inv[HANDPOS].usItem)->getItemClass() == IC_BLADE ?
					TacticalStr[NECK_HIT_LOCATION_STR] :
					TacticalStr[HEAD_HIT_LOCATION_STR];
				break;

			case AIM_SHOT_TORSO: hit_location = TacticalStr[TORSO_HIT_LOCATION_STR]; break;
			case AIM_SHOT_LEGS:  hit_location = TacticalStr[LEGS_HIT_LOCATION_STR];  break;

			default: return;
		}
	}
	SetHitLocationText(hit_location);
}


static UICursorID HandleKnifeCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const activated, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_MERCS);

	if (activated)
	{
		DetermineCursorBodyLocation(s, TRUE, TRUE);

		// Calculate chance to hit
		if (gamepolicy(show_hit_chance) && gUIFullTarget)
		{
			UINT32 uiHitChance = CalcChanceToStab(s, gUIFullTarget, s->bShownAimTime / 2);
			SetChanceToHitText(st_format_printf("%d%%", uiHitChance));
		}

		if (gfUIHandleShowMoveGrid) gfUIHandleShowMoveGrid = 2;

		// Calculate action points
		bool enough_points = true;
		if (gTacticalStatus.uiFlags & INCOMBAT)
		{
			gsCurrentActionPoints         = CalcTotalAPsToAttack(s, map_pos, TRUE, s->bShownAimTime / 2);
			gfUIDisplayActionPoints       = TRUE;
			gfUIDisplayActionPointsCenter = TRUE;

			// If we don't have any points and we are at the first refine, do nothing but warn!
			if (!EnoughPoints(s, gsCurrentActionPoints, 0, FALSE))
			{
				gfUIDisplayActionPointsInvalid = TRUE;
				if (s->bShownAimTime == REFINE_KNIFE_1) return KNIFE_HIT_UICURSOR;
			}

			INT8  const future_aim = REFINE_KNIFE_2;
			INT16 const ap_costs   = CalcTotalAPsToAttack(s, map_pos, TRUE, future_aim / 2);
			if (!EnoughPoints(s, ap_costs, 0, FALSE)) enough_points = false;
		}

		if (!(gTacticalStatus.uiFlags & INCOMBAT) && COUNTERDONE(NONGUNTARGETREFINE))
		{
			if (s->bShownAimTime == REFINE_KNIFE_1)
			{
				PlayJA2Sample(TARG_REFINE_BEEP, MIDVOLUME, 1, MIDDLEPAN);
			}

			s->bShownAimTime = REFINE_KNIFE_2;
		}

		switch (s->bShownAimTime)
		{
			case REFINE_KNIFE_1:
				return gfDisplayFullCountRing ? KNIFE_YELLOW_AIM1_UICURSOR :
					enough_points          ? KNIFE_HIT_AIM1_UICURSOR    :
					KNIFE_NOGO_AIM1_UICURSOR;

			case REFINE_KNIFE_2:
				return gfDisplayFullCountRing ? KNIFE_YELLOW_AIM2_UICURSOR :
					enough_points          ? KNIFE_HIT_AIM2_UICURSOR    :
					KNIFE_NOGO_AIM2_UICURSOR;

			default:
				Assert(FALSE);
				// no return value!
				return NO_UICURSOR;
		}
	}
	else
	{
		gfUIDisplayActionPointsCenter = TRUE;

		// Check if we are on a guy (who's not selected)!
		if (gUIFullTarget && !(guiUIFullTargetFlags & SELECTED_MERC))
		{
			DetermineCursorBodyLocation(s, TRUE, TRUE);
			return KNIFE_HIT_UICURSOR;
		}
		else
		{
			return KNIFE_REG_UICURSOR;
		}
	}
}


static UICursorID HandlePunchCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const activated, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_MERCS);

	if (activated)
	{
		DetermineCursorBodyLocation(s, TRUE, TRUE);

		// Calculate chance to hit
		if (gamepolicy(show_hit_chance) && gUIFullTarget)
		{
			UINT32 uiHitChance = CalcChanceToPunch(s, gUIFullTarget, s->bShownAimTime / 2, true);
			SetChanceToHitText(st_format_printf("%d%%", uiHitChance));
		}

		if (gfUIHandleShowMoveGrid) gfUIHandleShowMoveGrid = 2;

		// Calculate action points
		bool enough_points = true;

		gsCurrentActionPoints         = CalcTotalAPsToAttack(s, map_pos, TRUE, s->bShownAimTime / 2);
		gfUIDisplayActionPoints       = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if (!EnoughPoints(s, gsCurrentActionPoints, 0, FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
			if (s->bShownAimTime == REFINE_PUNCH_1) return ACTION_PUNCH_RED;
		}

		INT8  const future_aim = REFINE_PUNCH_2;
		INT16 const ap_costs   = CalcTotalAPsToAttack(s, map_pos, TRUE, future_aim / 2);
		if (!EnoughPoints(s, ap_costs, 0, FALSE)) enough_points = false;

		if (!(gTacticalStatus.uiFlags & INCOMBAT) && COUNTERDONE(NONGUNTARGETREFINE))
		{
			if (s->bShownAimTime == REFINE_PUNCH_1)
			{
				PlayJA2Sample(TARG_REFINE_BEEP, MIDVOLUME, 1, MIDDLEPAN);
			}

			s->bShownAimTime = REFINE_PUNCH_2;
		}

		switch (s->bShownAimTime)
		{
			case REFINE_PUNCH_1:
				return gfDisplayFullCountRing ? ACTION_PUNCH_YELLOW_AIM1_UICURSOR :
					enough_points          ? ACTION_PUNCH_RED_AIM1_UICURSOR    :
					ACTION_PUNCH_NOGO_AIM1_UICURSOR;

			case REFINE_PUNCH_2:
				return gfDisplayFullCountRing ? ACTION_PUNCH_YELLOW_AIM2_UICURSOR :
					enough_points          ? ACTION_PUNCH_RED_AIM2_UICURSOR    :
					ACTION_PUNCH_NOGO_AIM2_UICURSOR;

			default:
				Assert(FALSE);
				// no return value!
				return NO_UICURSOR;
		}
	}
	else
	{
		gfUIDisplayActionPointsCenter = TRUE;

		// Check if we are on a guy (who's not selected)!
		if (gUIFullTarget && !(guiUIFullTargetFlags & SELECTED_MERC))
		{
			DetermineCursorBodyLocation(s, TRUE, TRUE);
			return ACTION_PUNCH_RED;
		}
		else
		{
			return ACTION_PUNCH_GRAY;
		}
	}
}


static UICursorID HandleAidCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const activated, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_MERCSFORAID);
	return activated || gUIFullTarget ? ACTION_FIRSTAID_RED : ACTION_FIRSTAID_GRAY;
}


static UICursorID HandleActivatedTossCursor()
{
	return ACTION_TOSS_UICURSOR;
}


static UICursorID HandleNonActivatedTossCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const recalc, MouseMoveState const uiCursorFlags, ItemCursor const ubItemCursor)
{
	static bool bad_ctgh = false;

	// Check for enough ammo
	BOOLEAN armed = FALSE;
	if (ubItemCursor == TRAJECTORYCURS)
	{
		if (!EnoughAmmo(s, FALSE, HANDPOS))
		{
			// Check if ANY ammo exists
			if (FindAmmoToReload(s, HANDPOS, NO_SLOT) == NO_SLOT) return BAD_RELOAD_UICURSOR;

			gsCurrentActionPoints   = GetAPsToAutoReload(s);
			gfUIDisplayActionPoints = TRUE;
			return GOOD_RELOAD_UICURSOR;
		}

		armed = TRUE;
	}

	// Add APs
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		gsCurrentActionPoints =
			ubItemCursor == TRAJECTORYCURS ? CalcTotalAPsToAttack(s, map_pos, TRUE, s->bShownAimTime / 2) :
			MinAPsToThrow(*s, map_pos, TRUE);

		gfUIDisplayActionPoints       = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing
		// but warn!
		if (!EnoughPoints(s, gsCurrentActionPoints, 0, FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}

	// If we begin to move, reset the cursor
	if (uiCursorFlags != MOUSE_STATIONARY)
	{
		giHitChance = -1;
		EndPhysicsTrajectoryUI();
	}

	gfUIHandlePhysicsTrajectory = TRUE;

	if (recalc)
	{
		INT16 final_grid_no = 0;

		// Calculate chance to throw here
		if (map_pos == s->sGridNo)
		{
			bad_ctgh = false;
		}
		else
		{
			OBJECTTYPE const& o = s->inv[HANDPOS];
			// ATE: Find the object to use
			OBJECTTYPE TempObject = o;

			// Do we have a launchable?
			for (INT8 i = 0; i != MAX_ATTACHMENTS; ++i)
			{
				UINT16 const attach_item = o.usAttachItem[i];
				if (attach_item == NOTHING) continue;
				if (!(GCM->getItem(attach_item)->isExplosive())) continue;
				CreateItem(attach_item, o.bAttachStatus[i], &TempObject);
				break;
			}

			if (s->bWeaponMode == WM_ATTACHED)
			{
				INT8 const slot = FindAttachment(&o, UNDER_GLAUNCHER);
				if (slot != NO_SLOT)
				{
					CreateItem(UNDER_GLAUNCHER, o.bAttachStatus[slot], &TempObject);
				}
			}

			INT8  level;
			bad_ctgh = !CalculateLaunchItemChanceToGetThrough(s, &TempObject, map_pos, gsInterfaceLevel, gsInterfaceLevel * 256, &final_grid_no, armed, &level, TRUE);
			BeginPhysicsTrajectoryUI(final_grid_no, level, bad_ctgh);
		}

		// Calculate chance to hit
		if (gamepolicy(show_hit_chance))
		{
			if (bad_ctgh)
				giHitChance = 0;
			else
				giHitChance = CalcThrownChanceToHit(s, final_grid_no, s->bShownAimTime / 2, AIM_SHOT_TORSO);
		}
	}

	// Attach chance-to-hit to mouse cursor
	if (gamepolicy(show_hit_chance) && giHitChance != -1)
		SetChanceToHitText(st_format_printf("%d%%", giHitChance));

	return bad_ctgh ? BAD_THROW_UICURSOR : GOOD_THROW_UICURSOR;
}


static UICursorID HandleWirecutterCursor(SOLDIERTYPE* const s, GridNo const map_pos, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_WIREFENCE);
	return s->bLevel == 0 && IsCuttableWireFenceAtGridNo(map_pos) ? GOOD_WIRECUTTER_UICURSOR :
		BAD_WIRECUTTER_UICURSOR;
}


static UICursorID HandleRepairCursor(SOLDIERTYPE* const s, GridNo const map_pos, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_REPAIR);
	return s->bLevel == 0 && IsRepairableStructAtGridNo(map_pos, 0) ? GOOD_REPAIR_UICURSOR :
		BAD_REPAIR_UICURSOR;
}


static UICursorID HandleRefuelCursor(SOLDIERTYPE* const s, GridNo const map_pos, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_REFUEL);
	return s->bLevel == 0 && GetRefuelableStructAtGridNo(map_pos) ? REFUEL_RED_UICURSOR :
		REFUEL_GREY_UICURSOR;
}


static UICursorID HandleJarCursor(SOLDIERTYPE* const s, GridNo const map_pos, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_JAR);
	return IsCorpseAtGridNo(map_pos, s->bLevel) ? GOOD_JAR_UICURSOR :
		BAD_JAR_UICURSOR;
}


static UICursorID HandleTinCanCursor(SOLDIERTYPE* const s, GridNo const map_pos, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_CAN);

	// Check if a door exists here
	STRUCTURE*       structure;
	INT16            int_tile_grid_no;
	LEVELNODE* const int_tile = GetCurInteractiveTileGridNoAndStructure(&int_tile_grid_no, &structure);
	return int_tile && structure->fFlags & STRUCTURE_ANYDOOR ? PLACE_TINCAN_GREY_UICURSOR :
		PLACE_TINCAN_RED_UICURSOR;
}


static UICursorID HandleRemoteCursor(SOLDIERTYPE* const s, BOOLEAN const activated, MouseMoveState const uiCursorFlags)
{
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		gsCurrentActionPoints         = GetAPsToUseRemote(s);
		gfUIDisplayActionPoints       = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if (!EnoughPoints(s, gsCurrentActionPoints, 0, FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}

	return activated ? PLACE_REMOTE_RED_UICURSOR : PLACE_REMOTE_GREY_UICURSOR;
}


static UICursorID HandleBombCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const activated, MouseMoveState const uiCursorFlags)
{
	HandleUIMovementCursor(s, uiCursorFlags, map_pos, MOVEUI_TARGET_BOMB);

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		gsCurrentActionPoints         = GetTotalAPsToDropBomb(s, map_pos);
		gfUIDisplayActionPoints       = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if (!EnoughPoints(s, gsCurrentActionPoints, 0, FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}

	return activated ? PLACE_BOMB_RED_UICURSOR : PLACE_BOMB_GREY_UICURSOR;
}


void HandleLeftClickCursor( SOLDIERTYPE *pSoldier )
{
	ItemCursor const ubItemCursor = GetActionModeCursor(pSoldier);

	// OK, if we are i realtime.. goto directly to shoot
	if (!(gTacticalStatus.uiFlags & INCOMBAT) &&
		ubItemCursor != TOSSCURS && ubItemCursor != TRAJECTORYCURS)
	{
		// GOTO DIRECTLY TO USING ITEM
		// ( only if not burst mode.. )
		if ( !pSoldier->bDoBurst )
		{
			guiPendingOverrideEvent = CA_MERC_SHOOT;
		}
		return;
	}

	const GridNo sGridNo = guiCurrentCursorGridNo;
	if (sGridNo == NOWHERE) return;

	gfUIForceReExamineCursorData = TRUE;

	gfDisplayFullCountRing = FALSE;

	switch( ubItemCursor )
	{
		case TARGETCURS:
			pSoldier->bShownAimTime = REFINE_AIM_1;

			// Reset counter
			RESETCOUNTER( TARGETREFINE );
			break;

		case PUNCHCURS:
			pSoldier->bShownAimTime = REFINE_PUNCH_1;

			// Reset counter
			RESETCOUNTER( NONGUNTARGETREFINE );
			break;


		case KNIFECURS:
			pSoldier->bShownAimTime = REFINE_KNIFE_1;

			// Reset counter
			RESETCOUNTER( NONGUNTARGETREFINE );
			break;

		default:
			// GOTO DIRECTLY TO USING ITEM
			guiPendingOverrideEvent = CA_MERC_SHOOT;
	}
}




void HandleRightClickAdjustCursor( SOLDIERTYPE *pSoldier, INT16 usMapPos )
{
	INT16 sAPCosts;
	INT8  bFutureAim;
	INT16 sGridNo;
	INT8  bTargetLevel;

	ItemCursor const ubCursor = GetActionModeCursor(pSoldier);

	// 'snap' cursor to target tile....
	if (gUIFullTarget != NULL) usMapPos = gUIFullTarget->sGridNo;

	switch( ubCursor )
	{
		case TARGETCURS:

			// CHECK IF GUY HAS IN HAND A WEAPON
			if ( pSoldier->bDoBurst )
			{
				// Do nothing!
				// pSoldier->bShownAimTime = REFINE_AIM_BURST;
			}
			else
			{
				// Look for a target here...
				const SOLDIERTYPE* const tgt = gUIFullTarget;
				if (tgt != NULL)
				{
					// Get target soldier, if one exists
					sGridNo = tgt->sGridNo;
					bTargetLevel = pSoldier->bLevel;

					if (!HandleCheckForBadChangeToGetThrough(pSoldier, tgt, sGridNo, bTargetLevel))
					{
						return;
					}
				}

				bFutureAim = (INT8)( pSoldier->bShownAimTime + 2 );

				if ( bFutureAim <= REFINE_AIM_5 )
				{
					sAPCosts = CalcTotalAPsToAttack( pSoldier, usMapPos, TRUE, (INT8)(bFutureAim / 2) );

					// Determine if we can afford!
					if ( EnoughPoints( pSoldier, sAPCosts, 0, FALSE ) )
					{
						pSoldier->bShownAimTime+= 2;
						if ( pSoldier->bShownAimTime > REFINE_AIM_5 )
						{
							pSoldier->bShownAimTime = REFINE_AIM_5;
						}
					}
					// Else - goto first level!
					else
					{
						if ( !gfDisplayFullCountRing )
						{
							gfDisplayFullCountRing = TRUE;
						}
						else
						{
							pSoldier->bShownAimTime = REFINE_AIM_1;
							gfDisplayFullCountRing = FALSE;
						}
					}
				}
				else
				{
					if ( !gfDisplayFullCountRing )
					{
						gfDisplayFullCountRing = TRUE;
					}
					else
					{
						pSoldier->bShownAimTime = REFINE_AIM_1;
						gfDisplayFullCountRing = FALSE;
					}
				}
			}
			break;


		case PUNCHCURS:

			bFutureAim = (INT8)( pSoldier->bShownAimTime + REFINE_PUNCH_2 );

			if ( bFutureAim <= REFINE_PUNCH_2 )
			{
				sAPCosts = CalcTotalAPsToAttack( pSoldier, usMapPos, TRUE, (INT8)(bFutureAim / 2) );

				// Determine if we can afford!
				if ( EnoughPoints( pSoldier, sAPCosts, 0, FALSE ) )
				{
					pSoldier->bShownAimTime+= REFINE_PUNCH_2;

					if ( pSoldier->bShownAimTime > REFINE_PUNCH_2 )
					{
						pSoldier->bShownAimTime = REFINE_PUNCH_2;
					}
				}
				// Else - goto first level!
				else
				{
					if ( !gfDisplayFullCountRing )
					{
						gfDisplayFullCountRing = TRUE;
					}
					else
					{
						pSoldier->bShownAimTime = REFINE_PUNCH_1;
						gfDisplayFullCountRing = FALSE;
					}
				}
			}
			else
			{
				if ( !gfDisplayFullCountRing )
				{
					gfDisplayFullCountRing = TRUE;
				}
				else
				{
					pSoldier->bShownAimTime = REFINE_PUNCH_1;
					gfDisplayFullCountRing = FALSE;
				}
			}
			break;


		case KNIFECURS:

			bFutureAim = (INT8)( pSoldier->bShownAimTime + REFINE_KNIFE_2 );

			if ( bFutureAim <= REFINE_KNIFE_2 )
			{
				sAPCosts = CalcTotalAPsToAttack( pSoldier, usMapPos, TRUE, (INT8)(bFutureAim / 2) );

				// Determine if we can afford!
				if ( EnoughPoints( pSoldier, sAPCosts, 0, FALSE ) )
				{
					pSoldier->bShownAimTime+= REFINE_KNIFE_2;

					if ( pSoldier->bShownAimTime > REFINE_KNIFE_2 )
					{
						pSoldier->bShownAimTime = REFINE_KNIFE_2;
					}
				}
				// Else - goto first level!
				else
				{
					if ( !gfDisplayFullCountRing )
					{
						gfDisplayFullCountRing = TRUE;
					}
					else
					{
						pSoldier->bShownAimTime = REFINE_KNIFE_1;
						gfDisplayFullCountRing = FALSE;
					}
				}
			}
			else
			{
				if ( !gfDisplayFullCountRing )
				{
					gfDisplayFullCountRing = TRUE;
				}
				else
				{
					pSoldier->bShownAimTime = REFINE_KNIFE_1;
					gfDisplayFullCountRing = FALSE;
				}
			}
			break;

		case TOSSCURS: break;

		default:

			ErasePath();

	}

}


ItemCursor GetActionModeCursor(SOLDIERTYPE const* const pSoldier)
{
	UINT16 usInHand;

	// If we are an EPC, do nothing....
	//if ( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	//{
	//	return( INVALIDCURS );
	//}

	// AN EPC is always not - attackable unless they are a robot!
	if ( AM_AN_EPC( pSoldier ) && !( pSoldier->uiStatusFlags & SOLDIER_ROBOT ) )
	{
		return( INVALIDCURS );
	}

	// ATE: if a vehicle.... same thing
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		return( INVALIDCURS );
	}

	// If we can't be controlled, returninvalid...
	if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
	{
		if ( !CanRobotBeControlled( pSoldier ) )
		{
			// Display message that robot cannot be controlled....
			return( INVALIDCURS );
		}
	}


	// If we are in attach shoot mode, use toss cursor...
	if (pSoldier->bWeaponMode == WM_ATTACHED )
	{
		return( TRAJECTORYCURS );
	}

	usInHand = pSoldier->inv[HANDPOS].usItem;

	// Start off with what is in our hand
	ItemCursor ubCursor = GCM->getItem(usInHand)->getCursor();

	// OK, check if what is in our hands has a detonator attachment...
	// Detonators can only be on invalidcurs things...
	if ( ubCursor == INVALIDCURS )
	{
		if ( FindAttachment( &(pSoldier->inv[HANDPOS]), DETONATOR) != ITEM_NOT_FOUND )
		{
			ubCursor = BOMBCURS;
		}
		else if ( FindAttachment( &(pSoldier->inv[HANDPOS]), REMDETONATOR) != ITEM_NOT_FOUND )
		{
			ubCursor = BOMBCURS;
		}
	}

	// Now check our terrain to see if we cannot do the action now...
	if ( pSoldier->bOverTerrainType == DEEP_WATER )
	{
		ubCursor = INVALIDCURS;
	}

	// If we are out of breath, no cursor...
	if ( pSoldier->bBreath < OKBREATH && pSoldier->bCollapsed )
	{
		ubCursor = INVALIDCURS;
	}

	return( ubCursor );
}

// Switch on item, display appropriate feedback cursor for a click....
void HandleUICursorRTFeedback( SOLDIERTYPE *pSoldier )
{
	ItemCursor const ubItemCursor = GetActionModeCursor(pSoldier);
	switch( ubItemCursor )
	{
		case TARGETCURS:

			if ( pSoldier->bDoBurst )
			{
				//BeginDisplayTimedCursor( ACTION_TARGETREDBURST_UICURSOR, 500 );
			}
			else
			{
				if ( GCM->getItem(pSoldier->inv[ HANDPOS ].usItem)->getItemClass() == IC_THROWING_KNIFE )
				{
					BeginDisplayTimedCursor( RED_THROW_UICURSOR, 500 );
				}
				else
				{
					BeginDisplayTimedCursor( ACTION_TARGETRED_UICURSOR, 500 );
				}
			}
			break;

		default:

			break;
	}

}
