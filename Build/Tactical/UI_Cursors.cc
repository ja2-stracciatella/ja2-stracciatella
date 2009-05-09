#include "Handle_Items.h"
#include "Items.h"
#include "Soldier_Find.h"
#include "Structure.h"
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


// FUNCTIONS FOR ITEM CURSOR HANDLING
static UICursorID HandleActivatedTargetCursor(   SOLDIERTYPE*, GridNo map_pos, BOOLEAN recalc);
static UICursorID HandleNonActivatedTargetCursor(SOLDIERTYPE*, GridNo map_pos, BOOLEAN show_APs, BOOLEAN fRecalc, UINT32 uiCursorFlags);
static UICursorID HandleKnifeCursor(             SOLDIERTYPE*, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags);
static UICursorID HandlePunchCursor(             SOLDIERTYPE*, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlagsl);
static UICursorID HandleAidCursor(               SOLDIERTYPE*, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags);
static UICursorID HandleActivatedTossCursor(     SOLDIERTYPE*, UINT16 usMapPos, UINT8 ubCursor);
static UICursorID HandleNonActivatedTossCursor(  SOLDIERTYPE*, UINT16 usMapPos, BOOLEAN fRecalc, UINT32 uiCursorFlags, UINT8 ubCursor);
static UICursorID HandleWirecutterCursor(        SOLDIERTYPE*, UINT16 usMapPos, UINT32 uiCursorFlags);
static UICursorID HandleRepairCursor(            SOLDIERTYPE*, UINT16 usMapPos, UINT32 uiCursorFlags);
static UICursorID HandleRefuelCursor(            SOLDIERTYPE*, UINT16 usMapPos, UINT32 uiCursorFlags);
static UICursorID HandleRemoteCursor(            SOLDIERTYPE*, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags);
static UICursorID HandleBombCursor(              SOLDIERTYPE*, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags);
static UICursorID HandleJarCursor(               SOLDIERTYPE*, UINT16 usMapPos, UINT32 uiCursorFlags);
static UICursorID HandleTinCanCursor(            SOLDIERTYPE*, UINT16 usMapPos, UINT32 uiCursorFlags);


static BOOLEAN gfCannotGetThrough = FALSE;
static BOOLEAN gfDisplayFullCountRing = FALSE;


BOOLEAN GetMouseRecalcAndShowAPFlags( UINT32 *puiCursorFlags, BOOLEAN *pfShowAPs )
{
	static					BOOLEAN						fDoNewTile = FALSE;
	BOOLEAN						fRecalc = FALSE;
	BOOLEAN						fShowAPs = FALSE;

	// SET FLAGS FOR CERTAIN MOUSE MOVEMENTS
	const UINT32 uiCursorFlags = GetCursorMovementFlags();

	// Force if we are currently cycling guys...
	if ( gfUIForceReExamineCursorData )
  {
		fDoNewTile = TRUE;
		fRecalc    = TRUE;
		gfUIForceReExamineCursorData = FALSE;
  }

	// IF CURSOR IS MOVING
	if ( uiCursorFlags & MOUSE_MOVING )
	{
			// IF CURSOR WAS PREVIOUSLY STATIONARY, MAKE THE ADDITIONAL CHECK OF GRID POS CHANGE
			//if ( uiCursorFlags & MOUSE_MOVING_NEW_TILE )
			{
				// Reset counter
				RESETCOUNTER( PATHFINDCOUNTER );
				fDoNewTile = TRUE;
			}
	 }

	 if ( uiCursorFlags & MOUSE_STATIONARY )
	 {
			// ONLY DIPSLAY APS AFTER A DELAY
			if ( COUNTERDONE( PATHFINDCOUNTER ) )
			{
				// Don't reset counter: One when we move again do we do this!
				fShowAPs = TRUE;

				if ( fDoNewTile )
				{
					fDoNewTile = FALSE;
					fRecalc    = TRUE;
				}

			}

	 }

	 if ( puiCursorFlags )
	 {
			(*puiCursorFlags) = uiCursorFlags;
	 }

	 if ( pfShowAPs )
	 {
			(*pfShowAPs) = fShowAPs;
	 }

	 return( fRecalc );
}


// FUNCTIONS FOR CURSOR DETERMINATION!
UICursorID GetProperItemCursor(SOLDIERTYPE* const pSoldier, UINT16 usMapPos, BOOLEAN fActivated)
{
	UINT32						uiCursorFlags;
	BOOLEAN						fShowAPs = FALSE;
	BOOLEAN						fRecalc = FALSE;
	INT16							sTargetGridNo = usMapPos;
	UICursorID ubCursorID = NO_UICURSOR;

	fRecalc = GetMouseRecalcAndShowAPFlags( &uiCursorFlags, &fShowAPs );

	// ATE: Update attacking weapon!
	// CC has added this attackingWeapon stuff and I need to update it constantly for
	// CTGH algorithms
	if ( gTacticalStatus.ubAttackBusyCount == 0 && Item[ pSoldier->inv[HANDPOS].usItem ].usItemClass & IC_WEAPON )
	{
		pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;
	}

	// Calculate target gridno!
	sTargetGridNo = (gUIFullTarget != NULL ? gUIFullTarget->sGridNo : usMapPos);

	ItemCursor const ubItemCursor = GetActionModeCursor(pSoldier);
	switch( ubItemCursor )
	{
		case PUNCHCURS:

			// Determine whether gray or red!
			ubCursorID = HandlePunchCursor( pSoldier, sTargetGridNo, fActivated, uiCursorFlags );
			break;

		case KNIFECURS:

			ubCursorID = HandleKnifeCursor( pSoldier, sTargetGridNo, fActivated, uiCursorFlags );
			break;

		case AIDCURS:

			ubCursorID =  HandleAidCursor( pSoldier, usMapPos, fActivated, uiCursorFlags );
			break;

		case TARGETCURS:
			if ( fActivated )
			{
				ubCursorID = HandleActivatedTargetCursor(pSoldier, sTargetGridNo, fRecalc);
			}
			else
			{
				ubCursorID = HandleNonActivatedTargetCursor( pSoldier, sTargetGridNo, fShowAPs, fRecalc, uiCursorFlags );
			}


			// ATE: Only do this if we are in combat!
			if ( gCurrentUIMode == ACTION_MODE && ( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				// Alrighty, let's change the cursor!
				const SOLDIERTYPE* const tgt = gUIFullTarget;
				if (fRecalc &&
						tgt != NULL &&
						IsValidTargetMerc(tgt) &&
						EnoughAmmo(pSoldier, FALSE, HANDPOS) && // ATE: Check for ammo
						guiUIFullTargetFlags & ENEMY_MERC && // IF it's an ememy, goto confirm action mode
						guiUIFullTargetFlags & VISIBLE_MERC &&
						!(guiUIFullTargetFlags & DEAD_MERC) &&
						!gfCannotGetThrough)
				{
					guiPendingOverrideEvent = A_CHANGE_TO_CONFIM_ACTION;
				}
			}
			break;

		case TOSSCURS:
		case TRAJECTORYCURS:

			if ( fActivated )
			{
        if ( !gfUIHandlePhysicsTrajectory )
        {
				  ubCursorID =  HandleNonActivatedTossCursor( pSoldier, sTargetGridNo, fRecalc, uiCursorFlags, ubItemCursor );
        }
        else
        {
				  ubCursorID = HandleActivatedTossCursor( pSoldier, sTargetGridNo, ubItemCursor );
        }
			}
			else
			{
				ubCursorID =  HandleNonActivatedTossCursor( pSoldier, sTargetGridNo, fRecalc, uiCursorFlags, ubItemCursor );
			}
			break;

		case BOMBCURS:

			ubCursorID = HandleBombCursor( pSoldier, sTargetGridNo, fActivated, uiCursorFlags );
			break;

		case REMOTECURS:

			ubCursorID = HandleRemoteCursor( pSoldier, sTargetGridNo, fActivated, uiCursorFlags );
			break;

		case WIRECUTCURS:

			ubCursorID = HandleWirecutterCursor( pSoldier, sTargetGridNo, uiCursorFlags );
			break;


		case REPAIRCURS:

			ubCursorID = HandleRepairCursor( pSoldier, sTargetGridNo, uiCursorFlags );
			break;

		case JARCURS:

			ubCursorID = HandleJarCursor( pSoldier, sTargetGridNo, uiCursorFlags );
			break;

		case TINCANCURS:

			ubCursorID = HandleTinCanCursor( pSoldier, sTargetGridNo, uiCursorFlags );
			break;

		case REFUELCURS:

			ubCursorID = HandleRefuelCursor( pSoldier, sTargetGridNo, uiCursorFlags );
			break;

		case INVALIDCURS:

			ubCursorID =  INVALID_ACTION_UICURSOR;
			break;

	}

	return( ubCursorID );
}


static void DetermineCursorBodyLocation(SOLDIERTYPE* s, BOOLEAN fDisplay, BOOLEAN fRecalc);


static UICursorID HandleActivatedTargetCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const recalc)
{
	bool const is_throwing_knife = Item[s->inv[HANDPOS].usItem].usItemClass == IC_THROWING_KNIFE;
	if (is_throwing_knife)
	{
		// If we are in realtime, follow!
		if (!(gTacticalStatus.uiFlags & INCOMBAT)                  &&
				gAnimControl[s->usAnimState].uiFlags & ANIM_STATIONARY &&
				gUITargetShotWaiting)
		{
			guiPendingOverrideEvent = CA_MERC_SHOOT;
		}

		// Check if we are reloading
		if ((gTacticalStatus.uiFlags & REALTIME || !(gTacticalStatus.uiFlags & INCOMBAT)) &&
				s->fReloading)
		{
			return ACTION_TARGET_RELOADING;
		}
	}

	// Determine where we are shooting/aiming
	DetermineCursorBodyLocation(s, TRUE, TRUE);

	bool enough_points       = true;
	bool max_point_limit_hit = false;
	if (gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT)
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

	if ((gTacticalStatus.uiFlags & REALTIME || !(gTacticalStatus.uiFlags & INCOMBAT)) &&
			COUNTERDONE(TARGETREFINE))
	{
		RESETCOUNTER(TARGETREFINE);

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

	UICursorID cursor = NO_UICURSOR;
	if (max_point_limit_hit)
	{
		// Check if we're in burst mode!
		cursor =
			s->bDoBurst       ? ACTION_TARGETREDBURST_UICURSOR :
			is_throwing_knife ? RED_THROW_UICURSOR             :
			ACTION_TARGETRED_UICURSOR;
	}
	else if (s->bDoBurst)
	{
		cursor =
			s->fDoSpread ? ACTION_TARGETREDBURST_UICURSOR :
			ACTION_TARGETCONFIRMBURST_UICURSOR;
	}
	else
	{
		switch (s->bShownAimTime)
		{
			case REFINE_AIM_1:
				if (is_throwing_knife)
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW1_UICURSOR :
						enough_points          ? ACTION_THROWAIM1_UICURSOR       :
						ACTION_THROWAIMCANT1_UICURSOR;
				}
				else
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW1_UICURSOR :
						enough_points          ? ACTION_TARGETAIM1_UICURSOR       :
						ACTION_TARGETAIMCANT1_UICURSOR;
				}
				break;

			case REFINE_AIM_2:
				if (is_throwing_knife)
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW2_UICURSOR :
						enough_points          ? ACTION_THROWAIM3_UICURSOR       :
						ACTION_THROWAIMCANT2_UICURSOR;
				}
				else
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW2_UICURSOR :
						enough_points          ? ACTION_TARGETAIM3_UICURSOR       :
						ACTION_TARGETAIMCANT2_UICURSOR;
				}
				break;

			case REFINE_AIM_3:
				if (is_throwing_knife)
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW3_UICURSOR :
						enough_points          ? ACTION_THROWAIM5_UICURSOR       :
						ACTION_THROWAIMCANT3_UICURSOR;
				}
				else
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW3_UICURSOR :
						enough_points          ? ACTION_TARGETAIM5_UICURSOR       :
						ACTION_TARGETAIMCANT3_UICURSOR;
				}
				break;

			case REFINE_AIM_4:
				if (is_throwing_knife)
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_THROWAIMYELLOW4_UICURSOR :
						enough_points          ? ACTION_THROWAIM7_UICURSOR       :
						ACTION_THROWAIMCANT4_UICURSOR;
				}
				else
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_TARGETAIMYELLOW4_UICURSOR :
						enough_points          ? ACTION_TARGETAIM7_UICURSOR       :
						ACTION_TARGETAIMCANT4_UICURSOR;
				}
				break;

			case REFINE_AIM_5:
				if (is_throwing_knife)
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_THROWAIMFULL_UICURSOR :
						enough_points          ? ACTION_THROWAIM9_UICURSOR    :
						ACTION_THROWAIMCANT5_UICURSOR;
				}
				else
				{
					cursor =
						gfDisplayFullCountRing ? ACTION_TARGETAIMFULL_UICURSOR :
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
		{ // OK, make buddy flash!
			SetCursorFlags(free_cursor_name, CURSOR_TO_FLASH | CURSOR_TO_PLAY_SOUND);
		}
		else
		{
			SetCursorSpecialFrame(free_cursor_name, 0);
		}
	}

	return cursor;
}


static UICursorID HandleNonActivatedTargetCursor(SOLDIERTYPE* const s, GridNo const map_pos, BOOLEAN const show_APs, BOOLEAN const fRecalc, UINT32 const uiCursorFlags)
{
	bool const is_throwing_knife = Item[s->inv[HANDPOS].usItem].usItemClass == IC_THROWING_KNIFE;
	if (!is_throwing_knife)
	{
		if (gTacticalStatus.uiFlags & REALTIME || !(gTacticalStatus.uiFlags & INCOMBAT))
		{
			DetermineCursorBodyLocation(GetSelectedMan(), show_APs, fRecalc);
			if (s->fReloading) return ACTION_TARGET_RELOADING;
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

	if (gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT)
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
	if (uiCursorFlags & MOUSE_MOVING) gfCannotGetThrough = FALSE;

	if (gfCannotGetThrough)
	{
		return
			s->bDoBurst       ? ACTION_NOCHANCE_BURST_UICURSOR :
			is_throwing_knife ? BAD_THROW_UICURSOR             :
			ACTION_NOCHANCE_SHOOT_UICURSOR;
	}
	else if (!InRange(s, map_pos))
	{ // Flash cursor!
		return
			s->bDoBurst       ? ACTION_FLASH_BURST_UICURSOR :
			is_throwing_knife ? FLASH_THROW_UICURSOR        :
			ACTION_FLASH_SHOOT_UICURSOR;
	}
	else
	{
		return
			s->bDoBurst       ? ACTION_TARGETBURST_UICURSOR :
			is_throwing_knife ? GOOD_THROW_UICURSOR         :
			ACTION_SHOOT_UICURSOR;
	}
}


static void DetermineCursorBodyLocation(SOLDIERTYPE* const pSoldier, const BOOLEAN fDisplay, const BOOLEAN fRecalc)
{
	SOLDIERTYPE* pTargetSoldier = NULL;

	if ( gTacticalStatus.ubAttackBusyCount > 0 )
	{
		// ATE: Return if attacker busy count > 0, this
		// helps in RT with re-setting the flag to random...
		return;
	}

	if (fRecalc)
	{
		// ALWAYS SET AIM LOCATION TO NOTHING
		pSoldier->bAimShotLocation = AIM_SHOT_RANDOM;

		const GridNo usMapPos = GetMouseMapPos();
		if (usMapPos == NOWHERE) return;

		BOOLEAN	fOnGuy = FALSE;
		UINT16  usFlags;

		// Determine which body part it's on
		for (LEVELNODE* pNode = NULL;;)
		{
			pNode = GetAnimProfileFlags(usMapPos, &usFlags, &pTargetSoldier, pNode);
			if (pNode == NULL) break;

			if (pTargetSoldier == NULL) continue;

			// ATE: Check their stance - if prone - return!
			if (gAnimControl[pTargetSoldier->usAnimState].ubHeight == ANIM_PRONE)
			{
				return;
			}

			// Check if we have a half tile profile
			if (usFlags & (TILE_FLAG_NORTH_HALF | TILE_FLAG_SOUTH_HALF | TILE_FLAG_WEST_HALF | TILE_FLAG_EAST_HALF | TILE_FLAG_TOP_HALF | TILE_FLAG_BOTTOM_HALF))
			{
				INT16 sCellX;
				INT16 sCellY;
				GetMouseWorldCoords(&sCellX, &sCellY);
				// We are only interested in the sub-tile coordinates
				sCellX %= CELL_X_SIZE;
				sCellY %= CELL_Y_SIZE;

				if (usFlags & TILE_FLAG_NORTH_HALF && sCellY >  CELL_Y_SIZE / 2) continue;
				if (usFlags & TILE_FLAG_SOUTH_HALF && sCellY <= CELL_Y_SIZE / 2) continue;
				if (usFlags & TILE_FLAG_WEST_HALF  && sCellX >  CELL_X_SIZE / 2) continue;
				if (usFlags & TILE_FLAG_EAST_HALF  && sCellX <= CELL_X_SIZE / 2) continue;

				if (usFlags & TILE_FLAG_TOP_HALF)
				{
					INT16 sScreenX;
					INT16 sScreenY;
					FromCellToScreenCoordinates(sCellX, sCellY, &sScreenX, &sScreenY);

					// Check for Below...
					if (sScreenX > WORLD_TILE_Y / 2) continue;
				}

				if (usFlags & TILE_FLAG_BOTTOM_HALF)
				{
					INT16 sScreenX;
					INT16 sScreenY;
					FromCellToScreenCoordinates(sCellX, sCellY, &sScreenX, &sScreenY);

					// Check for Below...
					if (sScreenX <= WORLD_TILE_Y / 2) continue;
				}
			}

			// Check if mouse is in bounding box of soldier
			if (!IsPointInSoldierBoundingBox(pTargetSoldier, gusMouseXPos, gusMouseYPos))
			{
				continue;
			}

			fOnGuy = TRUE;
			break;
		}

		if (!fOnGuy)
		{
			// Check if we can find a soldier here
			SOLDIERTYPE* const tgt = gUIFullTarget;
			if (tgt != NULL)
			{
				pTargetSoldier = tgt;
				usFlags = FindRelativeSoldierPosition(tgt, gusMouseXPos, gusMouseYPos);
				if (usFlags != 0) fOnGuy = TRUE;
			}
		}

		if (fOnGuy && IsValidTargetMerc(pTargetSoldier))
		{
			if (usFlags & TILE_FLAG_FEET) pSoldier->bAimShotLocation = AIM_SHOT_LEGS;
			if (usFlags & TILE_FLAG_MID)  pSoldier->bAimShotLocation = AIM_SHOT_TORSO;
			if (usFlags & TILE_FLAG_HEAD) pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
		}
	}

	if ( fDisplay && ( !pSoldier->bDoBurst ) )
	{
		SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			pTargetSoldier = tgt;

				if ( pTargetSoldier->ubBodyType == CROW )
				{
					pSoldier->bAimShotLocation = AIM_SHOT_LEGS;
					SetHitLocationText(TacticalStr[CROW_HIT_LOCATION_STR]);
					return;
				}

				if ( !IS_MERC_BODY_TYPE( pTargetSoldier ) )
				{
					return;
				}

				switch( pSoldier->bAimShotLocation )
				{
					case AIM_SHOT_HEAD:

						// If we have a knife in hand, change string
						if ( Item[ pSoldier->inv[ HANDPOS ].usItem ].usItemClass == IC_BLADE )
						{
							SetHitLocationText(TacticalStr[NECK_HIT_LOCATION_STR]);
						}
						else
						{
							SetHitLocationText(TacticalStr[HEAD_HIT_LOCATION_STR]);
						}
						break;

					case AIM_SHOT_TORSO:
						SetHitLocationText(TacticalStr[TORSO_HIT_LOCATION_STR]);
						break;

					case AIM_SHOT_LEGS:
						SetHitLocationText(TacticalStr[LEGS_HIT_LOCATION_STR]);
						break;
				}
		}
	}
}


static UICursorID HandleKnifeCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags)
{
	INT16							sAPCosts;
	INT8							bFutureAim;
	BOOLEAN						fEnoughPoints = TRUE;

	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_MERCS );

	if ( fActivated )
	{
		DetermineCursorBodyLocation(pSoldier, TRUE, TRUE);

		if ( gfUIHandleShowMoveGrid )
		{
			gfUIHandleShowMoveGrid = 2;
		}

		// Calculate action points
		if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
		{
			gsCurrentActionPoints = CalcTotalAPsToAttack( pSoldier, sGridNo, TRUE, (INT8)(pSoldier->bShownAimTime / 2) );
			gfUIDisplayActionPoints = TRUE;
			gfUIDisplayActionPointsCenter = TRUE;

			// If we don't have any points and we are at the first refine, do nothing but warn!
			if ( !EnoughPoints( pSoldier, gsCurrentActionPoints, 0 , FALSE ) )
			{
				gfUIDisplayActionPointsInvalid = TRUE;

				if ( pSoldier->bShownAimTime == REFINE_KNIFE_1 )
				{
					return( KNIFE_HIT_UICURSOR );
				}
			}

			bFutureAim = (INT8)( REFINE_KNIFE_2 );

			sAPCosts = CalcTotalAPsToAttack( pSoldier, sGridNo, TRUE, (INT8)(bFutureAim / 2) );

			// Determine if we can afford!
			if ( !EnoughPoints( pSoldier, (INT16)sAPCosts, 0 , FALSE ) )
			{
				fEnoughPoints = FALSE;
			}

		}

		if ( ( ( gTacticalStatus.uiFlags & REALTIME ) || !( gTacticalStatus.uiFlags & INCOMBAT ) ) )
		{
			if (COUNTERDONE(NONGUNTARGETREFINE))
			{
				// Reset counter
				RESETCOUNTER(NONGUNTARGETREFINE);

				if (pSoldier->bShownAimTime == REFINE_KNIFE_1)
				{
					PlayJA2Sample(TARG_REFINE_BEEP, MIDVOLUME, 1, MIDDLEPAN);
				}

				pSoldier->bShownAimTime = REFINE_KNIFE_2;
			}
		}


		switch( pSoldier->bShownAimTime )
		{
			case REFINE_KNIFE_1:

				if ( gfDisplayFullCountRing )
				{
					return( KNIFE_YELLOW_AIM1_UICURSOR );
				}
				else if ( fEnoughPoints )
				{
					return( KNIFE_HIT_AIM1_UICURSOR );
				}
				else
				{
					return( KNIFE_NOGO_AIM1_UICURSOR );
				}

			case REFINE_KNIFE_2:

				if ( gfDisplayFullCountRing )
				{
					return( KNIFE_YELLOW_AIM2_UICURSOR );
				}
				else if ( fEnoughPoints )
				{
					return( KNIFE_HIT_AIM2_UICURSOR );
				}
				else
				{
					return( KNIFE_NOGO_AIM2_UICURSOR );
				}

			default:
				Assert( FALSE );
				// no return value!
				return NO_UICURSOR;
		}
	}
	else
	{
		gfUIDisplayActionPointsCenter = TRUE;

		// CHECK IF WE ARE ON A GUY ( THAT'S NOT SELECTED )!
		if (gUIFullTarget != NULL && !(guiUIFullTargetFlags & SELECTED_MERC))
		{
			DetermineCursorBodyLocation(pSoldier, TRUE, TRUE);
			return( KNIFE_HIT_UICURSOR );
		}
		else
		{
			return( KNIFE_REG_UICURSOR );
		}
	}
}


static UICursorID HandlePunchCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags)
{
	INT16							sAPCosts;
	INT8							bFutureAim;
	BOOLEAN						fEnoughPoints = TRUE;

	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_MERCS );

	if ( fActivated )
	{
		DetermineCursorBodyLocation(pSoldier, TRUE, TRUE);

		if ( gfUIHandleShowMoveGrid )
		{
			gfUIHandleShowMoveGrid = 2;
		}

		// Calculate action points
		if ( gTacticalStatus.uiFlags & TURNBASED )
		{
			gsCurrentActionPoints = CalcTotalAPsToAttack( pSoldier, sGridNo, TRUE, (INT8)(pSoldier->bShownAimTime / 2) );
			gfUIDisplayActionPoints = TRUE;
			gfUIDisplayActionPointsCenter = TRUE;

			// If we don't have any points and we are at the first refine, do nothing but warn!
			if ( !EnoughPoints( pSoldier, gsCurrentActionPoints, 0 , FALSE ) )
			{
				gfUIDisplayActionPointsInvalid = TRUE;

				if ( pSoldier->bShownAimTime == REFINE_PUNCH_1 )
				{
					return( ACTION_PUNCH_RED );
				}
			}

			bFutureAim = (INT8)( REFINE_PUNCH_2 );

			sAPCosts = CalcTotalAPsToAttack( pSoldier, sGridNo, TRUE, (INT8)(bFutureAim / 2) );

			// Determine if we can afford!
			if ( !EnoughPoints( pSoldier, (INT16)sAPCosts, 0 , FALSE ) )
			{
				fEnoughPoints = FALSE;
			}

		}

		if ( ( ( gTacticalStatus.uiFlags & REALTIME ) || !( gTacticalStatus.uiFlags & INCOMBAT ) ) )
		{
			if (COUNTERDONE(NONGUNTARGETREFINE))
			{
				// Reset counter
				RESETCOUNTER(NONGUNTARGETREFINE);

				if (pSoldier->bShownAimTime == REFINE_PUNCH_1)
				{
					PlayJA2Sample(TARG_REFINE_BEEP, MIDVOLUME, 1, MIDDLEPAN);
				}

				pSoldier->bShownAimTime = REFINE_PUNCH_2;
			}
		}

		switch( pSoldier->bShownAimTime )
		{
			case REFINE_PUNCH_1:

				if ( gfDisplayFullCountRing )
				{
					return( ACTION_PUNCH_YELLOW_AIM1_UICURSOR );
				}
				else if ( fEnoughPoints )
				{
					return( ACTION_PUNCH_RED_AIM1_UICURSOR );
				}
				else
				{
					return( ACTION_PUNCH_NOGO_AIM1_UICURSOR );
				}

			case REFINE_PUNCH_2:

				if ( gfDisplayFullCountRing )
				{
					return( ACTION_PUNCH_YELLOW_AIM2_UICURSOR );
				}
				else if ( fEnoughPoints )
				{
					return( ACTION_PUNCH_RED_AIM2_UICURSOR );
				}
				else
				{
					return( ACTION_PUNCH_NOGO_AIM2_UICURSOR );
				}

			default:
				Assert( FALSE );
				// no return value!
				return NO_UICURSOR;
		}
	}
	else
	{
		gfUIDisplayActionPointsCenter = TRUE;

		// CHECK IF WE ARE ON A GUY ( THAT'S NOT SELECTED )!
		if (gUIFullTarget != NULL && !(guiUIFullTargetFlags & SELECTED_MERC))
		{
			DetermineCursorBodyLocation(pSoldier, TRUE, TRUE);
			return( ACTION_PUNCH_RED );
		}
		else
		{
			return( ACTION_PUNCH_GRAY );
		}
	}
}


static UICursorID HandleAidCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags)
{
	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_MERCSFORAID );

	if ( fActivated )
	{
		return( ACTION_FIRSTAID_RED );
	}
	else
	{
		// CHECK IF WE ARE ON A GUY
		if (gUIFullTarget != NULL)
		{
			return( ACTION_FIRSTAID_RED );
		}
		else
		{
			return( ACTION_FIRSTAID_GRAY );
		}
	}
}


static UICursorID HandleActivatedTossCursor(SOLDIERTYPE* pSoldier, UINT16 usMapPos, UINT8 ubCursor)
{
	return( ACTION_TOSS_UICURSOR );
}


static UICursorID HandleNonActivatedTossCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, BOOLEAN fRecalc, UINT32 uiCursorFlags, UINT8 ubItemCursor)
{
	INT16 sFinalGridNo;
	static BOOLEAN fBadCTGH = FALSE;
	BOOLEAN fArmed = FALSE;
	INT8		bLevel;
	OBJECTTYPE	TempObject;
	INT8		bSlot;
	OBJECTTYPE * pObj;
	INT8				bAttachPos;


	// Check for enough ammo...
	if ( ubItemCursor == TRAJECTORYCURS )
	{
		fArmed = TRUE;

		if ( !EnoughAmmo( pSoldier, FALSE, HANDPOS ) )
		{
			// Check if ANY ammo exists.....
			if ( FindAmmoToReload( pSoldier, HANDPOS, NO_SLOT ) == NO_SLOT )
			{
				// OK, use BAD reload cursor.....
				return( BAD_RELOAD_UICURSOR );
			}
			else
			{
				// Check APs to reload...
				gsCurrentActionPoints = GetAPsToAutoReload( pSoldier );

				gfUIDisplayActionPoints = TRUE;
				//gUIDisplayActionPointsOffX = 14;
				//gUIDisplayActionPointsOffY = 7;

				// OK, use GOOD reload cursor.....
				return( GOOD_RELOAD_UICURSOR );
			}
		}
	}

	// Add APs....
	if ( gTacticalStatus.uiFlags & TURNBASED && ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		if ( ubItemCursor == TRAJECTORYCURS )
		{
		  gsCurrentActionPoints = CalcTotalAPsToAttack( pSoldier, sGridNo, TRUE, (INT8)(pSoldier->bShownAimTime / 2) );
		}
		else
		{
			gsCurrentActionPoints = MinAPsToThrow( pSoldier, sGridNo, TRUE );
		}

		gfUIDisplayActionPoints = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if ( !EnoughPoints( pSoldier, gsCurrentActionPoints, 0 , FALSE ) )
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}


	// OK, if we begin to move, reset the cursor...
	if ( uiCursorFlags & MOUSE_MOVING )
	{
		EndPhysicsTrajectoryUI( );
	}

	gfUIHandlePhysicsTrajectory = TRUE;

	if ( fRecalc )
	{
		// Calculate chance to throw here.....
		if ( sGridNo == pSoldier->sGridNo )
		{
			fBadCTGH = FALSE;
		}
		else
		{
      // ATE: Find the object to use...
      TempObject = pSoldier->inv[HANDPOS];

      // Do we have a launcable?
	    pObj = &(pSoldier->inv[HANDPOS]);
	    for (bAttachPos = 0; bAttachPos < MAX_ATTACHMENTS; bAttachPos++)
	    {
		    if (pObj->usAttachItem[ bAttachPos ] != NOTHING)
		    {
			    if ( Item[ pObj->usAttachItem[ bAttachPos ] ].usItemClass & IC_EXPLOSV )
			    {
				    break;
			    }
		    }
	    }
	    if (bAttachPos != MAX_ATTACHMENTS)
	    {
        CreateItem( pObj->usAttachItem[ bAttachPos ],	pObj->bAttachStatus[ bAttachPos ], &TempObject );
	    }


			if (pSoldier->bWeaponMode == WM_ATTACHED && FindAttachment( &(pSoldier->inv[HANDPOS]), UNDER_GLAUNCHER ) != NO_SLOT )
			{
				bSlot = FindAttachment( &(pSoldier->inv[HANDPOS]), UNDER_GLAUNCHER );

				if ( bSlot != NO_SLOT )
				{
					CreateItem( UNDER_GLAUNCHER, pSoldier->inv[HANDPOS].bAttachStatus[ bSlot ], &TempObject );

					if ( !CalculateLaunchItemChanceToGetThrough( pSoldier, &TempObject, sGridNo, (INT8)gsInterfaceLevel, (INT16)( gsInterfaceLevel * 256 ), &sFinalGridNo, fArmed, &bLevel, TRUE ) )
					{
						fBadCTGH = TRUE;
					}
					else
					{
						fBadCTGH = FALSE;
					}
    			BeginPhysicsTrajectoryUI( sFinalGridNo, bLevel, fBadCTGH );
				}
			}
			else
			{
				if ( !CalculateLaunchItemChanceToGetThrough( pSoldier, &TempObject, sGridNo, (INT8)gsInterfaceLevel, (INT16)( gsInterfaceLevel * 256 ), &sFinalGridNo, fArmed, &bLevel, TRUE ) )
				{
					fBadCTGH = TRUE;
				}
				else
				{
					fBadCTGH = FALSE;
				}
    		BeginPhysicsTrajectoryUI( sFinalGridNo, bLevel, fBadCTGH );
			}
		}
	}

	if ( fBadCTGH )
	{
		return( BAD_THROW_UICURSOR );
	}
	return( GOOD_THROW_UICURSOR );
}


static UICursorID HandleWirecutterCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, UINT32 uiCursorFlags)
{
	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_WIREFENCE );

	// Are we over a cuttable fence?
	if ( IsCuttableWireFenceAtGridNo( sGridNo ) && pSoldier->bLevel == 0 )
	{
		return( GOOD_WIRECUTTER_UICURSOR );
	}

	return( BAD_WIRECUTTER_UICURSOR );
}


static UICursorID HandleRepairCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, UINT32 uiCursorFlags)
{
	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_REPAIR );

	// Are we over a cuttable fence?
	if ( IsRepairableStructAtGridNo( sGridNo, NULL ) && pSoldier->bLevel == 0 )
	{
		return( GOOD_REPAIR_UICURSOR );
	}

	return( BAD_REPAIR_UICURSOR );
}


static UICursorID HandleRefuelCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, UINT32 uiCursorFlags)
{
	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_REFUEL );

	// Are we over a refuelable vehicle?
	if (pSoldier->bLevel == 0 && GetRefuelableStructAtGridNo(sGridNo) != NULL)
	{
		return( REFUEL_RED_UICURSOR );
	}

	return( REFUEL_GREY_UICURSOR );
}


static UICursorID HandleJarCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, UINT32 uiCursorFlags)
{
	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_JAR );

	// Are we over a cuttable fence?
	if ( IsCorpseAtGridNo( sGridNo, pSoldier->bLevel ) )
	{
		return( GOOD_JAR_UICURSOR );
	}

	return( BAD_JAR_UICURSOR );
}


static UICursorID HandleTinCanCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, UINT32 uiCursorFlags)
{
	STRUCTURE					*pStructure;
  INT16							sIntTileGridNo;
	LEVELNODE					*pIntTile;


	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_CAN );

	// Check if a door exists here.....
	pIntTile = GetCurInteractiveTileGridNoAndStructure( &sIntTileGridNo, &pStructure );

	// We should not have null here if we are given this flag...
	if ( pIntTile != NULL )
	{
		if (pStructure->fFlags & STRUCTURE_ANYDOOR)
		{
			return( PLACE_TINCAN_GREY_UICURSOR );
		}
	}

	return( PLACE_TINCAN_RED_UICURSOR );
}


static UICursorID HandleRemoteCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags)
{
	// Calculate action points
	if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
	{
		gsCurrentActionPoints = GetAPsToUseRemote( pSoldier );
		gfUIDisplayActionPoints = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if ( !EnoughPoints( pSoldier, gsCurrentActionPoints, 0 , FALSE ) )
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}

	if ( fActivated )
	{
		return( PLACE_REMOTE_RED_UICURSOR );
	}
	else
	{
		return( PLACE_REMOTE_GREY_UICURSOR );
	}
}


static UICursorID HandleBombCursor(SOLDIERTYPE* pSoldier, UINT16 sGridNo, BOOLEAN fActivated, UINT32 uiCursorFlags)
{
	// DRAW PATH TO GUY
	HandleUIMovementCursor( pSoldier, uiCursorFlags, sGridNo, MOVEUI_TARGET_BOMB );

	// Calculate action points
	if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
	{
		gsCurrentActionPoints = GetTotalAPsToDropBomb( pSoldier, sGridNo );
		gfUIDisplayActionPoints = TRUE;
		gfUIDisplayActionPointsCenter = TRUE;

		// If we don't have any points and we are at the first refine, do nothing but warn!
		if ( !EnoughPoints( pSoldier, gsCurrentActionPoints, 0 , FALSE ) )
			{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}

	if ( fActivated )
	{
		return( PLACE_BOMB_RED_UICURSOR );
	}
	else
	{
		return( PLACE_BOMB_GREY_UICURSOR );
	}
}


void HandleLeftClickCursor( SOLDIERTYPE *pSoldier )
{
	ItemCursor const ubItemCursor = GetActionModeCursor(pSoldier);

	// OK, if we are i realtime.. goto directly to shoot
	if ( ( ( gTacticalStatus.uiFlags & TURNBASED ) && !( gTacticalStatus.uiFlags & INCOMBAT ) ) && ubItemCursor != TOSSCURS && ubItemCursor != TRAJECTORYCURS )
	{
		// GOTO DIRECTLY TO USING ITEM
		// ( only if not burst mode.. )
		if ( !pSoldier->bDoBurst )
		{
			guiPendingOverrideEvent = CA_MERC_SHOOT;
		}
		return;
	}

	const GridNo sGridNo = GetMouseMapPos();
	if (sGridNo == NOWHERE) return;

	gfUIForceReExamineCursorData = TRUE;

	gfDisplayFullCountRing = FALSE;

	switch( ubItemCursor )
	{
		case TARGETCURS:

			if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				pSoldier->bShownAimTime				= REFINE_AIM_1;
			}
			else
			{
				pSoldier->bShownAimTime				= REFINE_AIM_1;
			}
			// Reset counter
			RESETCOUNTER( TARGETREFINE );
			break;

		case PUNCHCURS:

			if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				pSoldier->bShownAimTime				= REFINE_PUNCH_1;
			}
			else
			{
				pSoldier->bShownAimTime				= REFINE_PUNCH_1;
			}
			// Reset counter
			RESETCOUNTER( NONGUNTARGETREFINE );
			break;


		case KNIFECURS:

			if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				pSoldier->bShownAimTime				= REFINE_KNIFE_1;
			}
			else
			{
				pSoldier->bShownAimTime				= REFINE_KNIFE_1;
			}
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
	INT16					sAPCosts;
	INT8					bFutureAim;
	INT16					sGridNo;
	INT8					bTargetLevel;

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
				sGridNo					= usMapPos;
				bTargetLevel	  = (INT8)gsInterfaceLevel;

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
  UINT16				usInHand;

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
	ItemCursor ubCursor = Item[ usInHand ].ubCursor;

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
				if ( Item[ pSoldier->inv[ HANDPOS ].usItem ].usItemClass == IC_THROWING_KNIFE )
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
