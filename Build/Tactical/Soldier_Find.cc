#include "Turn_Based_Input.h"
#include "VObject.h"
#include "Debug.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "RenderWorld.h"
#include "Assignments.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Isometric_Utils.h"
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
#include "English.h"
#include "Overhead.h"
#include "OppList.h"
#include "PathAI.h"
#include "Interface_Cursors.h"
#include "Weapons.h"
#include "Items.h"
#include "Soldier_Profile.h"
#include "FOV.h"
#include "Soldier_Macros.h"
#include "EditorMercs.h"
#include "Soldier_Tile.h"
#include "Soldier_Find.h"
#include "Vehicles.h"
#include "GameSettings.h"
#include "UI_Cursors.h"


// This value is used to keep a small static array of uBID's which are stacked
#define				MAX_STACKED_MERCS		10


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


SOLDIERTYPE* FindSoldierFromMouse(void)
{
	const GridNo pos = GetMouseMapPos();
	if (pos == NOWHERE) return NULL;
	return FindSoldier(pos, FINDSOLDIERSAMELEVEL(gsInterfaceLevel));
}


bool IsOwnedMerc(SOLDIERTYPE const& s)
{
	return
		IsOnOurTeam(&s) &&
		(
			!(s.uiStatusFlags & SOLDIER_VEHICLE) ||
			GetNumberInVehicle(GetVehicle(s.bVehicleID)) != 0
		);
}


UINT32 GetSoldierFindFlags(SOLDIERTYPE const& s)
{
	UINT32 flags = 0;

	if (&s == GetSelectedMan()) flags |= SELECTED_MERC;
	if (IsOnOurTeam(&s))
	{
		if (!(s.uiStatusFlags & SOLDIER_VEHICLE) ||
				GetNumberInVehicle(GetVehicle(s.bVehicleID)) != 0)
		{ // It's our own merc
			flags |= OWNED_MERC;
		}
	}
	else
	{ // Check the side, etc
		if (!s.bNeutral && s.bSide != gbPlayerNum)
		{ // It's an enemy merc
			flags |= ENEMY_MERC;
		}
		else
		{ // It's not an enemy merc
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


// THIS FUNCTION IS CALLED FAIRLY REGULARLY
SOLDIERTYPE* FindSoldier(INT16 sGridNo, UINT32 uiFlags)
{
	SGPRect				aRect;
	BOOLEAN				fSoldierFound = FALSE;
	INT16					sScreenX, sScreenY;
	INT16					sMaxScreenMercY, sHeighestMercScreenY = -32000;
	SOLDIERTYPE* best_merc = NULL;
	UINT16				usAnimSurface;
	INT32					iMercScreenX, iMercScreenY;

	if ( _KeyDown( SHIFT ) )
	{
		uiFlags = FIND_SOLDIER_GRIDNO;
	}

	// Set some values
  if ( uiFlags & FIND_SOLDIER_BEGINSTACK )
	{
		gSoldierStack.bNum = 0;
		gSoldierStack.fUseGridNo = FALSE;
	}


	// Loop through all mercs and make go
	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE* const pSoldier = *i;

		BOOLEAN fInScreenRect = FALSE;
		BOOLEAN fInGridNo     = FALSE;

		if (pSoldier->uiStatusFlags & SOLDIER_DEAD) continue;
		if (pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) continue;

		// OK, ignore if we are a passenger...
		if ( pSoldier->uiStatusFlags & ( SOLDIER_PASSENGER | SOLDIER_DRIVER ) )
		{
			continue;
		}

		// If we want same level, skip if buggy's not on the same level!
		if ( uiFlags & FIND_SOLDIER_SAMELEVEL )
		{
			if ( pSoldier->bLevel != (UINT8)( uiFlags >> 16 ) )
			{
				continue;
			}
		}

		if (!(uiFlags & FIND_SOLDIER_GRIDNO))
		{
			// Get Rect contained in the soldier
			GetSoldierScreenRect( pSoldier, &aRect );

			// Set mouse stuff
			sScreenX = gusMouseXPos;
			sScreenY = gusMouseYPos;

			if (IsPointInScreenRect(sScreenX, sScreenY, aRect))
			{
				fInScreenRect = TRUE;
			}

			if ( pSoldier->sGridNo == sGridNo )
			{
				fInGridNo			= TRUE;
			}

			// ATE: If we are an enemy....
			if (!gGameSettings.fOptions[TOPTION_SMART_CURSOR] &&
					IsOnOurTeam(pSoldier))
			{
				// ATE: NOT if we are in action or comfirm action mode
				if ( gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE || gUIActionModeChangeDueToMouseOver )
				{
					fInScreenRect = FALSE;
				}
			}

			// ATE: Refine this further....
			// Check if this is the selected guy....
			if (pSoldier == GetSelectedMan())
			{
				// Are we in action mode...
				if ( gCurrentUIMode == ACTION_MODE || gCurrentUIMode == CONFIRM_ACTION_MODE )
				{
					// Are we in medic mode?
					if ( GetActionModeCursor( pSoldier ) != AIDCURS )
					{
						fInScreenRect = FALSE;
						fInGridNo			= FALSE;
					}
				}
			}

			// Make sure we are always on guy if we are on same gridno
			if ( fInScreenRect || fInGridNo )
			{
				// Check if we are a vehicle and refine if so....
				if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
				{
					usAnimSurface = GetSoldierAnimationSurface(pSoldier);

					if ( usAnimSurface != INVALID_ANIMATION_SURFACE )
					{
						iMercScreenX = (INT32)( sScreenX - aRect.iLeft );
						iMercScreenY = (INT32)( -1 * ( sScreenY - aRect.iBottom ) );

						if ( !CheckVideoObjectScreenCoordinateInData( gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject, pSoldier->usAniFrame, iMercScreenX, iMercScreenY ) )
						{
							continue;
						}
					}
				}

				// If thgis is from a gridno, use mouse pos!
				if ( pSoldier->sGridNo == sGridNo )
				{

				}

				 // Only break here if we're not creating a stack of these fellas
				 if ( uiFlags & FIND_SOLDIER_BEGINSTACK )
				 {
					 gfHandleStack = TRUE;

						// Add this one!
					 gSoldierStack.mercs[gSoldierStack.bNum] = pSoldier;
					 gSoldierStack.bNum++;

					 // Determine if it's the current
					 if ( aRect.iBottom > sHeighestMercScreenY )
					 {
							sMaxScreenMercY = (UINT16)aRect.iBottom;
							sHeighestMercScreenY = sMaxScreenMercY;

							gSoldierStack.bCur = gSoldierStack.bNum - 1;
					 }
				 }
				 //Are we handling a stack right now?
				 else if ( gfHandleStack )
				 {
						// Are we the selected stack?
						if ( gSoldierStack.fUseGridNo )
						{
							 fSoldierFound = FALSE;
							 break;
						}
						else if (gSoldierStack.mercs[gSoldierStack.bCur] == pSoldier)
						{
							 best_merc = pSoldier;
							 fSoldierFound = TRUE;
							 break;
						}
				 }
				 else
				 {
					 // Determine if it's the best one
					 if ( aRect.iBottom > sHeighestMercScreenY )
					 {
							sMaxScreenMercY = (UINT16)aRect.iBottom;
							sHeighestMercScreenY = sMaxScreenMercY;
							best_merc = pSoldier;
					 }

					 fSoldierFound = TRUE;
					 // Don't break here, find the rest!

				 }
			}

		}
		else
		{
			//Otherwise, look for a bad guy by way of gridno]
			// Selective means don't give out enemy mercs if they are not visible

			///&& !NewOKDestination( pSoldier, sGridNo, TRUE, (INT8)gsInterfaceLevel )
			if ( pSoldier->sGridNo == sGridNo && !NewOKDestination( pSoldier, sGridNo, TRUE, (INT8)gsInterfaceLevel ) )
			{
				best_merc = pSoldier;
				fSoldierFound = TRUE;
				break;
			}
		}
	}

	if (fSoldierFound && best_merc != NULL)
	{
		return best_merc;
	}
	else
	{
		// If we were handling a stack, and we have not found anybody, end
		if (gfHandleStack && !(uiFlags & FIND_SOLDIER_BEGINSTACK))
		{
			if ( gSoldierStack.fUseGridNo )
			{
				if ( gSoldierStack.sUseGridNoGridNo != sGridNo )
				{
					gfHandleStack = FALSE;
				}
			}
			else
			{
				gfHandleStack = FALSE;
			}
		}
	}
	return NULL;
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
			guiUIFullTargetFlags = 0;
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
	if (s->bTeam != gbPlayerNum &&
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

		pRect->iLeft		= sMercScreenX;
		pRect->iTop			= sMercScreenY;
		pRect->iBottom	= sMercScreenY + pSoldier->sBoundingBoxHeight;
		pRect->iRight		= sMercScreenX + pSoldier->sBoundingBoxWidth;
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

	INT16 sMercScreenX = (gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2 + (INT16)dTempX_S;
	INT16 sMercScreenY = (gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2 + (INT16)dTempY_S;

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


BOOLEAN GridNoOnScreen( INT16 sGridNo )
{
	INT16 sWorldX;
	INT16 sWorldY;
  INT16 sAllowance = 20;

  if ( gsVIEWPORT_WINDOW_START_Y == 20 )
  {
    sAllowance = 40;
  }

	GetAbsoluteScreenXYFromMapPos(sGridNo, &sWorldX, &sWorldY);

	// ATE: OK, here, adjust the top value so that it's a tile and a bit over, because of our mercs!
	if ( sWorldX >= gsTopLeftWorldX && sWorldX <= gsBottomRightWorldX &&
			 sWorldY >= ( gsTopLeftWorldY + sAllowance )	&& sWorldY <= ( gsBottomRightWorldY + 20 ) )
	{
		return( TRUE );
	}
	return( FALSE );
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
	if ( sWorldX >= gsTopLeftWorldX && sWorldX <= gsBottomRightWorldX &&
			 sWorldY >= gsTopLeftWorldY	&& sWorldY <= ( gsBottomRightWorldY + 20 ) )
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
	SGPRect				aRect;

	// Get Rect contained in the soldier
	GetSoldierScreenRect( pSoldier, &aRect );

	return IsPointInScreenRect(sX, sY, aRect);
}


UINT16 FindRelativeSoldierPosition(const SOLDIERTYPE* const pSoldier, const INT16 sX, const INT16 sY)
{
	SGPRect				aRect;
	INT16					sRelX, sRelY;
	FLOAT					dRelPer;


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

		sScreenX = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)dTempX_S;
		sScreenY = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)dTempY_S;

		// Adjust starting screen coordinates
		sScreenX	-= gsRenderWorldOffsetX;
		sScreenY	-= gsRenderWorldOffsetY;

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
