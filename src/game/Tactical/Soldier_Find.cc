#include "Turn_Based_Input.h"
#include "Debug.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Isometric_Utils.h"
#include "Interface.h"
#include "Interactive_Tiles.h"
#include "English.h"
#include "Overhead.h"
#include "Soldier_Macros.h"
#include "Soldier_Find.h"
#include "Vehicles.h"
#include "GameSettings.h"
#include "UI_Cursors.h"
#include "UILayout.h"


// This value is used to keep a small static array of uBID's which are stacked
#define MAX_STACKED_MERCS 10


static const UINT32 gScrollSlideInertiaDirection[NUM_WORLD_DIRECTIONS] =
{
	3,
	0,
	0,
	0,
	0,
	0,
	3,
	3,
};

// Struct used for cycling through multiple mercs per mouse position
struct SOLDIER_STACK_TYPE
{
	INT8         bNum;
	SOLDIERTYPE* mercs[MAX_STACKED_MERCS];
	INT8         bCur;
	BOOLEAN      fUseGridNo;
	UINT16       sUseGridNoGridNo;
};

static SOLDIER_STACK_TYPE gSoldierStack;
static BOOLEAN            gfHandleStack = FALSE;

bool IsOwnedMerc(SOLDIERTYPE const& s)
{
	return IsOnOurTeam(s) && (
		!(s.uiStatusFlags & SOLDIER_VEHICLE) ||
		GetNumberInVehicle(GetVehicle(s.bVehicleID)) != 0);
}


SoldierFindFlags GetSoldierFindFlags(SOLDIERTYPE const& s)
{
	SoldierFindFlags flags = NO_MERC;

	if (&s == GetSelectedMan()) flags |= SELECTED_MERC;
	if (IsOnOurTeam(s))
	{
		if (!(s.uiStatusFlags & SOLDIER_VEHICLE) ||
			GetNumberInVehicle(GetVehicle(s.bVehicleID)) != 0)
		{
			// It's our own merc
			flags |= OWNED_MERC;
		}
	}
	else
	{
		// Check the side, etc
		if (!s.bNeutral && s.bSide != Side::FRIENDLY)
		{
			// It's an enemy merc
			flags |= ENEMY_MERC;
		}
		else
		{
			// It's not an enemy merc
			flags |= NEUTRAL_MERC;
		}
	}

	// Check for a guy who does not have an iterrupt (when applicable!)
	if (!OK_INTERRUPT_MERC(&s)) flags |= NOINTERRUPT_MERC;
	if (s.bLife < OKLIFE)       flags |= UNCONSCIOUS_MERC;
	if (s.bLife == 0)           flags |= DEAD_MERC;

	if (s.bVisible != -1 || gTacticalStatus.uiFlags & SHOW_ALL_MERCS)
	{
		flags |= VISIBLE_MERC;
	}

	return flags;
}


static void GetSoldierScreenRect(const SOLDIERTYPE* pSoldier, SGPRect* pRect);


// This function is called fairly regularly
SOLDIERTYPE* FindSoldier(GridNo const gridno, UINT32 flags)
{
	if (_KeyDown(SHIFT)) flags = FIND_SOLDIER_GRIDNO;

	if (flags & FIND_SOLDIER_BEGINSTACK)
	{
		gSoldierStack.bNum       = 0;
		gSoldierStack.fUseGridNo = FALSE;
	}

	INT16        heighest_merc_screen_y = -32000;
	SOLDIERTYPE* best_merc              = 0;
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE& s = **i;

		if (s.uiStatusFlags & SOLDIER_DEAD) continue;
		if (s.bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) continue;

		// Ignore if soldier is a passenger
		if (s.uiStatusFlags & (SOLDIER_PASSENGER | SOLDIER_DRIVER)) continue;

		// If we want same level, skip if soldier is not on the same level
		if (flags & FIND_SOLDIER_SAMELEVEL)
		{
			if (s.bLevel != (UINT8)(flags >> 16)) continue;
		}

		if (!(flags & FIND_SOLDIER_GRIDNO))
		{
			SGPRect soldier_rect;
			GetSoldierScreenRect(&s, &soldier_rect);

			INT16 const screen_x = gusMouseXPos;
			INT16 const screen_y = gusMouseYPos;

			bool in_screen_rect = IsPointInScreenRect(screen_x, screen_y, soldier_rect);

			// ATE: If we are an enemy
			if (!gGameSettings.fOptions[TOPTION_SMART_CURSOR] && IsOnOurTeam(s))
			{
				// ATE: NOT if we are in action or comfirm action mode
				if ((gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE) || gUIActionModeChangeDueToMouseOver)
				{
					in_screen_rect = false;
				}
			}

			// Make sure we are always on guy if we are on same gridno
			if (!in_screen_rect && s.sGridNo != gridno) continue;

			// ATE: Refine this further
			if (&s == GetSelectedMan() &&
				(gCurrentUIMode == ACTION_MODE || gCurrentUIMode == CONFIRM_ACTION_MODE) &&
				GetActionModeCursor(&s) != AIDCURS)
			{
				continue;
			}

			// Check if we are a vehicle and refine if so
			if (s.uiStatusFlags & SOLDIER_VEHICLE)
			{
				UINT16 const anim_surface = GetSoldierAnimationSurface(&s);
				if (anim_surface != INVALID_ANIMATION_SURFACE)
				{
					INT32 const merc_screen_x =   screen_x - soldier_rect.iLeft;
					INT32 const merc_screen_y = -(screen_y - soldier_rect.iBottom);
					if (!CheckVideoObjectScreenCoordinateInData(gAnimSurfaceDatabase[anim_surface].hVideoObject, s.usAniFrame, merc_screen_x, merc_screen_y))
					{
						continue;
					}
				}
			}

			// Only break here if we're not creating a stack of these fellas
			if (flags & FIND_SOLDIER_BEGINSTACK)
			{
				gfHandleStack = TRUE;

				// Add this one!
				gSoldierStack.mercs[gSoldierStack.bNum++] = &s;

				// Determine if it's the current
				if (heighest_merc_screen_y < soldier_rect.iBottom)
				{
					heighest_merc_screen_y = soldier_rect.iBottom;
					gSoldierStack.bCur     = gSoldierStack.bNum - 1;
				}
			}
			else if (gfHandleStack) // Are we handling a stack right now?
			{
				// Are we the selected stack?
				if (gSoldierStack.fUseGridNo)
				{
					best_merc = 0;
					break;
				}
				else if (gSoldierStack.mercs[gSoldierStack.bCur] == &s)
				{
					return &s;
				}
			}
			else
			{
				// Determine if it's the best one
				if (heighest_merc_screen_y < soldier_rect.iBottom)
				{
					heighest_merc_screen_y = soldier_rect.iBottom;
					best_merc              = &s;
				}
				// Don't break here, find the rest
			}
		}
		else
		{
			// Otherwise, look for a bad guy by way of gridno
			// Selective means don't give out enemy mercs if they are not visible
			if (s.sGridNo == gridno && !NewOKDestination(&s, gridno, TRUE, gsInterfaceLevel))
			{
				return &s;
			}
		}
	}

	if (best_merc) return best_merc;

	// If we were handling a stack, and we have not found anybody, end
	if (gfHandleStack &&
		!(flags & FIND_SOLDIER_BEGINSTACK) &&
		(!gSoldierStack.fUseGridNo || gSoldierStack.sUseGridNoGridNo != gridno))
	{
		gfHandleStack = FALSE;
	}
	return 0;
}


BOOLEAN CycleSoldierFindStack( UINT16 usMapPos )
{
	// Have we initalized for this yet?
	if (!gfHandleStack &&
		FindSoldier(usMapPos, FINDSOLDIERSAMELEVEL(gsInterfaceLevel) | FIND_SOLDIER_BEGINSTACK) != NULL)
	{
		gfHandleStack = TRUE;
	}

	if ( gfHandleStack )
	{
		// we are cycling now?
		if ( !gSoldierStack.fUseGridNo )
		{
			gSoldierStack.bCur++;
		}

		gfUIForceReExamineCursorData = TRUE;

		if ( gSoldierStack.bCur == gSoldierStack.bNum )
		{
			if ( !gSoldierStack.fUseGridNo )
			{
				gSoldierStack.fUseGridNo = TRUE;
				gUIActionModeChangeDueToMouseOver = FALSE;
				gSoldierStack.sUseGridNoGridNo = usMapPos;
			}
			else
			{
				gSoldierStack.bCur = 0;
				gSoldierStack.fUseGridNo = FALSE;
			}
		}


		if ( !gSoldierStack.fUseGridNo )
		{
			gUIFullTarget        = gSoldierStack.mercs[gSoldierStack.bCur];
			guiUIFullTargetFlags = GetSoldierFindFlags(*gUIFullTarget);
			gUITargetSoldier     = gUIFullTarget;
		}
		else
		{
			gUIFullTarget        = NULL;
			guiUIFullTargetFlags = NO_MERC;
		}
	}

	// Return if we are in the cycle mode now...
	return( gfHandleStack );
}


BOOLEAN IsValidTargetMerc(const SOLDIERTYPE* const s)
{
	// CHECK IF ACTIVE!
	if (!s->bActive) return FALSE;

	// CHECK IF DEAD
	//if (s->bLife == 0) return FALSE;

	// IF BAD GUY - CHECK VISIVILITY
	if (s->bTeam != OUR_TEAM &&
		s->bVisible == -1 &&
		!(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
	{
		return( FALSE  );
	}

	return( TRUE );
}


static void GetSoldierScreenRect(const SOLDIERTYPE* const pSoldier, SGPRect* const pRect)
{
		INT16 sMercScreenX, sMercScreenY;

		GetSoldierScreenPos( pSoldier, &sMercScreenX, &sMercScreenY );

		pRect->iLeft = sMercScreenX;
		pRect->iTop = sMercScreenY;
		pRect->iBottom = sMercScreenY + pSoldier->sBoundingBoxHeight;
		pRect->iRight = sMercScreenX + pSoldier->sBoundingBoxWidth;
}


void GetSoldierScreenPos(const SOLDIERTYPE* const s, INT16* const psScreenX, INT16* const psScreenY)
{
	GetSoldierTRUEScreenPos(s, psScreenX, psScreenY);
	*psScreenX += s->sBoundingBoxOffsetX;
	*psScreenY += s->sBoundingBoxOffsetY;
}


// THE TRUE SCREN RECT DOES NOT TAKE THE OFFSETS OF BUDDY INTO ACCOUNT!
void GetSoldierTRUEScreenPos(const SOLDIERTYPE* const s, INT16* const psScreenX, INT16* const psScreenY)
{
	UINT16 const usAnimSurface = GetSoldierAnimationSurface(s);
	if (usAnimSurface == INVALID_ANIMATION_SURFACE)
	{
		*psScreenX = 0;
		*psScreenY = 0;
		return;
	}

	// Get 'TRUE' merc position
	FLOAT const dOffsetX = s->dXPos - gsRenderCenterX;
	FLOAT const dOffsetY = s->dYPos - gsRenderCenterY;

	FLOAT dTempX_S;
	FLOAT dTempY_S;
	FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

	INT16 sMercScreenX = g_ui.m_tacticalMapCenterX + (INT16)dTempX_S;
	INT16 sMercScreenY = g_ui.m_tacticalMapCenterY + (INT16)dTempY_S;

	// Adjust starting screen coordinates
	sMercScreenX -= gsRenderWorldOffsetX;
	sMercScreenY -= gsRenderWorldOffsetY;

	// Adjust for render height
	sMercScreenY += gsRenderHeight;
	sMercScreenY -= gpWorldLevelData[s->sGridNo].sHeight;

	sMercScreenY -= s->sHeightAdjustment;

	*psScreenX = sMercScreenX;
	*psScreenY = sMercScreenY;
}


bool GridNoOnScreen(GridNo const gridno)
{
	INT16 world_x;
	INT16 world_y;
	GetAbsoluteScreenXYFromMapPos(gridno, &world_x, &world_y);
	INT16 const allowance = gsVIEWPORT_WINDOW_START_Y == 20 ? 40 : 20;
	// ATE: Adjust the top value so that it's a tile and a bit over, because of
	// our mercs.
	return gsTopLeftWorldX <= world_x && world_x <= gsBottomRightWorldX &&
		gsTopLeftWorldY + allowance <= world_y && world_y <= gsBottomRightWorldY + 20;
}


BOOLEAN SoldierOnScreen(const SOLDIERTYPE* s)
{
	return GridNoOnScreen(s->sGridNo);
}


BOOLEAN SoldierLocationRelativeToScreen(const INT16 sGridNo, INT8* const pbDirection, UINT32* const puiScrollFlags)
{
	INT16 sWorldX;
	INT16 sWorldY;
	static BOOLEAN fCountdown = 0;
	INT16 sScreenCenterX, sScreenCenterY;
	INT16 sDistToCenterY, sDistToCenterX;

	*puiScrollFlags = 0;

	GetAbsoluteScreenXYFromMapPos(sGridNo, &sWorldX, &sWorldY);

	// Find the diustance from render center to true world center
	sDistToCenterX = gsRenderCenterX - gCenterWorldX;
	sDistToCenterY = gsRenderCenterY - gCenterWorldY;

	// From render center in world coords, convert to render center in "screen" coords
	FromCellToScreenCoordinates( sDistToCenterX , sDistToCenterY, &sScreenCenterX, &sScreenCenterY );

	// Subtract screen center
	sScreenCenterX += gsCX;
	sScreenCenterY += gsCY;

	// Adjust for offset origin!
	sScreenCenterX += 0;
	sScreenCenterY += 10;

	// Get direction
	//*pbDirection = atan8( sScreenCenterX, sScreenCenterY, sWorldX, sWorldY );
	*pbDirection = atan8(gsRenderCenterX, gsRenderCenterY, CenterX(sGridNo), CenterY(sGridNo));

	// Check values!
	if ( sWorldX > ( sScreenCenterX + 20 ) )
	{
		(*puiScrollFlags) |= SCROLL_RIGHT;
	}
	if ( sWorldX < ( sScreenCenterX - 20 ) )
	{
		(*puiScrollFlags) |= SCROLL_LEFT;
	}
	if ( sWorldY > ( sScreenCenterY + 20 ) )
	{
		(*puiScrollFlags) |= SCROLL_DOWN;
	}
	if ( sWorldY < ( sScreenCenterY - 20 ) )
	{
		(*puiScrollFlags) |= SCROLL_UP;
	}


	// If we are on screen, stop
	if (sWorldX >= gsTopLeftWorldX && sWorldX <= gsBottomRightWorldX &&
		sWorldY >= gsTopLeftWorldY && sWorldY <= (gsBottomRightWorldY + 20))
	{
		// CHECK IF WE ARE DONE...
		if ( fCountdown > gScrollSlideInertiaDirection[ *pbDirection ] )
		{
			fCountdown = 0;
			return( FALSE );
		}
		else
		{
			fCountdown++;
		}
	}

	return( TRUE );
}

BOOLEAN IsPointInSoldierBoundingBox( SOLDIERTYPE *pSoldier, INT16 sX, INT16 sY )
{
	SGPRect aRect;

	// Get Rect contained in the soldier
	GetSoldierScreenRect( pSoldier, &aRect );

	return IsPointInScreenRect(sX, sY, aRect);
}


UINT16 FindRelativeSoldierPosition(const SOLDIERTYPE* const pSoldier, const INT16 sX, const INT16 sY)
{
	SGPRect aRect;
	INT16   sRelX, sRelY;
	FLOAT   dRelPer;


	// Get Rect contained in the soldier
	GetSoldierScreenRect( pSoldier, &aRect );

	if ( IsPointInScreenRectWithRelative( sX, sY, &aRect, &sRelX, &sRelY ) )
	{
		dRelPer = (FLOAT)sRelY / ( aRect.iBottom - aRect.iTop );

		// Determine relative positions
		switch( gAnimControl[ pSoldier->usAnimState ].ubHeight )
		{
			case ANIM_STAND:
				if (dRelPer < .2) return TILE_FLAG_HEAD;
				if (dRelPer < .6) return TILE_FLAG_MID;
				return TILE_FLAG_FEET;

			case ANIM_CROUCH:
				if (dRelPer < .2) return TILE_FLAG_HEAD;
				if (dRelPer < .7) return TILE_FLAG_MID;
				return TILE_FLAG_FEET;
		}
	}

	return 0;
}


void GetGridNoScreenPos( INT16 sGridNo, UINT8 ubLevel, INT16 *psScreenX, INT16 *psScreenY )
{
	INT16 sScreenX, sScreenY;
	FLOAT dOffsetX, dOffsetY;
	FLOAT dTempX_S, dTempY_S;

	// Get 'TRUE' merc position
	dOffsetX = (FLOAT)( CenterX( sGridNo ) - gsRenderCenterX );
	dOffsetY = (FLOAT)( CenterY( sGridNo ) - gsRenderCenterY );

	// OK, DONT'T ASK... CONVERSION TO PROPER Y NEEDS THIS...
	dOffsetX -= CELL_Y_SIZE;

	FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

	sScreenX = ( g_ui.m_tacticalMapCenterX ) + (INT16)dTempX_S;
	sScreenY = ( g_ui.m_tacticalMapCenterY ) + (INT16)dTempY_S;

	// Adjust starting screen coordinates
	sScreenX -= gsRenderWorldOffsetX;
	sScreenY -= gsRenderWorldOffsetY;

	sScreenY += gsRenderHeight;

	// Adjust for world height
	sScreenY -= gpWorldLevelData[ sGridNo ].sHeight;

	// Adjust for level height
	if ( ubLevel )
	{
		sScreenY -= ROOF_LEVEL_HEIGHT;
	}

	*psScreenX = sScreenX;
	*psScreenY = sScreenY;
}
