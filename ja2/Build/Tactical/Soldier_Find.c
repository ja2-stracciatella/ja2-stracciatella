#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include <string.h>
	#include "wcheck.h"
	#include "stdlib.h"
	#include "debug.h"
	#include "math.h"
	#include "worlddef.h"
	#include "worldman.h"
	#include "renderworld.h"
	#include "Assignments.h"
	#include "Soldier Control.h"
	#include "Animation Control.h"
	#include "Animation Data.h"
	#include "Isometric Utils.h"
	#include "Event Pump.h"
	#include "Timer Control.h"
	#include "Render Fun.h"
	#include "Render Dirty.h"
	#include "mousesystem.h"
	#include "interface.h"
	#include "sysutil.h"
	#include "FileMan.h"
	#include "points.h"
	#include "Random.h"
	#include "ai.h"
	#include "Interactive Tiles.h"
	#include "soldier ani.h"
	#include "english.h"
	#include "overhead.h"
	#include "opplist.h"
	#include "Sound Control.h"
	#include "Font Control.h"
	#include "lighting.h"
	#include "pathai.h"
	#include "screenids.h"
	#include "interface cursors.h"
	#include "weapons.h"
	#include "lighting.h"
	#include "Handle UI Plan.h"
	#include "structure.h"
	#include "interface panels.h"
	#include "message.h"
	#include "items.h"
	#include "Soldier Profile.h"
	#include "fov.h"
	#include "Soldier macros.h"
	#include "EditorMercs.h"
	#include "soldier tile.h"
	#include "soldier find.h"
#endif


BOOLEAN IsGridNoInScreenRect( INT16 sGridNo, SGPRect *pRect );
BOOLEAN IsPointInScreenRect( INT16 sXPos, INT16 sYPos, SGPRect *pRect );
void GetSoldierScreenRect( SOLDIERTYPE *pSoldier, SGPRect *pRect );

// This value is used to keep a small static array of uBID's which are stacked
#define				MAX_STACKED_MERCS		10


UINT32							gScrollSlideInertiaDirection[ NUM_WORLD_DIRECTIONS ] =
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
typedef struct
{
	INT8			bNum;
	UINT8			ubIDs[ MAX_STACKED_MERCS ];
	INT8			bCur;
	BOOLEAN		fUseGridNo;
	UINT16		sUseGridNoGridNo;

} SOLDIER_STACK_TYPE;

SOLDIER_STACK_TYPE		gSoldierStack;
BOOLEAN								gfHandleStack = FALSE;

extern								BOOLEAN	gUIActionModeChangeDueToMouseOver;
extern								UINT32	guiUITargetSoldierId;


BOOLEAN FindSoldierFromMouse( UINT16 *pusSoldierIndex, UINT32 *pMercFlags )
{
	INT16							sMapPos;

	*pMercFlags = 0;

	 if ( GetMouseMapPos( &sMapPos ) )
	 {
			 if ( FindSoldier( sMapPos, pusSoldierIndex, pMercFlags ,FINDSOLDIERSAMELEVEL( gsInterfaceLevel ) ) )
			 {
				 return( TRUE );
			 }
	 }

	 return( FALSE );
}

BOOLEAN SelectiveFindSoldierFromMouse( UINT16 *pusSoldierIndex, UINT32 *pMercFlags )
{
	INT16							sMapPos;

	*pMercFlags = 0;

	 if ( GetMouseMapPos( &sMapPos ) )
	 {
			 if ( FindSoldier( sMapPos, pusSoldierIndex, pMercFlags , FINDSOLDIERSAMELEVEL( gsInterfaceLevel ) ) )
			 {
				 return( TRUE );
			 }
	 }

	 return( FALSE );
}


UINT32 GetSoldierFindFlags( UINT16 ubID )
{
	UINT32 MercFlags = 0;
	SOLDIERTYPE *pSoldier;

 // Get pSoldier!
 pSoldier = MercPtrs[ ubID ];

 // FInd out and set flags
 if ( ubID == gusSelectedSoldier )
 {
		MercFlags |= SELECTED_MERC;
 }
 if ( ubID >= gTacticalStatus.Team[ gbPlayerNum ].bFirstID && ubID <= gTacticalStatus.Team[ gbPlayerNum ].bLastID )
 {
	 if ( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && !GetNumberInVehicle( pSoldier->bVehicleID ) )
	 {
		 // Don't do anything!
	 }
	 else
	 {
			// It's our own merc
		 MercFlags  |= OWNED_MERC;

		 if ( pSoldier->bAssignment < ON_DUTY )
		 {
				MercFlags |= ONDUTY_MERC;
		 }
	 }
 }
 else
 {
	 // Check the side, etc
	 if ( !pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum ) )
	 {
			// It's an enemy merc
		 MercFlags  |= ENEMY_MERC;
	 }
	 else
	 {
			// It's not an enemy merc
		 MercFlags  |= NEUTRAL_MERC;
	 }
 }

	// Check for a guy who does not have an iterrupt ( when applicable! )
	if ( !OK_INTERRUPT_MERC( pSoldier ) )
	{
			MercFlags  |=	NOINTERRUPT_MERC;
	}

	if ( pSoldier->bLife < OKLIFE )
	{
		MercFlags  |=	UNCONSCIOUS_MERC;
	}

	if ( pSoldier->bLife == 0 )
	{
		MercFlags  |=	DEAD_MERC;
	}

	if ( pSoldier->bVisible != -1 || (gTacticalStatus.uiFlags&SHOW_ALL_MERCS) )
	{
		MercFlags  |=	VISIBLE_MERC;
	}

	return( MercFlags );
}

extern BOOLEAN CheckVideoObjectScreenCoordinateInData( HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iTextX, INT32 iTestY );


// THIS FUNCTION IS CALLED FAIRLY REGULARLY
BOOLEAN FindSoldier( INT16 sGridNo, UINT16 *pusSoldierIndex, UINT32 *pMercFlags, UINT32 uiFlags )
{
	UINT32				cnt;
	SOLDIERTYPE		*pSoldier;
	SGPRect				aRect;
	BOOLEAN				fSoldierFound = FALSE;
	INT16					sXMapPos, sYMapPos, sScreenX, sScreenY;
	INT16					sMaxScreenMercY, sHeighestMercScreenY = -32000;
	BOOLEAN				fDoFull;
	UINT8					ubBestMerc = NOBODY;
	UINT16				usAnimSurface;
	INT32					iMercScreenX, iMercScreenY;
	BOOLEAN				fInScreenRect = FALSE;
	BOOLEAN				fInGridNo			= FALSE;


	*pusSoldierIndex = NOBODY;
	*pMercFlags			 = 0;

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
	for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
	{
		pSoldier			= MercSlots[ cnt ];
		fInScreenRect = FALSE;
		fInGridNo			= FALSE;

		if ( pSoldier != NULL )
		{

			if ( pSoldier->bActive && !( pSoldier->uiStatusFlags & SOLDIER_DEAD ) && ( pSoldier->bVisible != -1 || (gTacticalStatus.uiFlags&SHOW_ALL_MERCS) ) )
			{
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


				// If we are selective.... do our own guys FULL and other with gridno!
				// First look for owned soldiers, by way of the full method
				if ( uiFlags & FIND_SOLDIER_GRIDNO )
				{
					fDoFull = FALSE;
				}
				else if ( uiFlags & FIND_SOLDIER_SELECTIVE )
				{
					if ( pSoldier->ubID >= gTacticalStatus.Team[ gbPlayerNum ].bFirstID && pSoldier->ubID <= gTacticalStatus.Team[ gbPlayerNum ].bLastID )
					{
						fDoFull = TRUE;
					}
					else
					{
						fDoFull = FALSE;
					}
				}
				else
				{
					fDoFull = TRUE;
				}

				if ( fDoFull )
				{
					// Get Rect contained in the soldier
					GetSoldierScreenRect( pSoldier, &aRect );

					// Get XY From gridno
					ConvertGridNoToXY( sGridNo, &sXMapPos, &sYMapPos );

					// Get screen XY pos from map XY
					// Be carefull to convert to cell cords
					//CellXYToScreenXY( (INT16)((sXMapPos*CELL_X_SIZE)), (INT16)((sYMapPos*CELL_Y_SIZE)), &sScreenX, &sScreenY);

					// Set mouse stuff
					sScreenX = gusMouseXPos;
					sScreenY = gusMouseYPos;

					if ( IsPointInScreenRect( sScreenX, sScreenY, &aRect ) )
					{
						fInScreenRect = TRUE;
					}

					if ( pSoldier->sGridNo == sGridNo )
					{
						fInGridNo			= TRUE;
					}

					// ATE: If we are an enemy....
					if ( !gGameSettings.fOptions[ TOPTION_SMART_CURSOR ] )
					{
						if ( pSoldier->ubID >= gTacticalStatus.Team[ gbPlayerNum ].bFirstID && pSoldier->ubID <= gTacticalStatus.Team[ gbPlayerNum ].bLastID )
						{
							// ATE: NOT if we are in action or comfirm action mode
							if ( gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE || gUIActionModeChangeDueToMouseOver )
							{
								fInScreenRect = FALSE;
							}
						}
					}

					// ATE: Refine this further....
					// Check if this is the selected guy....
					if ( pSoldier->ubID == gusSelectedSoldier )
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
							usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

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
							 gSoldierStack.ubIDs[ gSoldierStack.bNum ] = pSoldier->ubID;
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
								else if ( gSoldierStack.ubIDs[ gSoldierStack.bCur ] == pSoldier->ubID )
								{
									 // Set it!
									 ubBestMerc = pSoldier->ubID;

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

									// Set it!
									ubBestMerc = pSoldier->ubID;
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
						// Set it!
						ubBestMerc = pSoldier->ubID;

						fSoldierFound = TRUE;
						break;
					}
				}
			}
		}
	}

	if ( fSoldierFound && ubBestMerc != NOBODY )
	{
		 *pusSoldierIndex = (UINT16)ubBestMerc;

		 (*pMercFlags) = GetSoldierFindFlags( ubBestMerc );

 		  return( TRUE );

	}
	else
	{
		// If we were handling a stack, and we have not found anybody, end
		if ( gfHandleStack && !( uiFlags & ( FIND_SOLDIER_BEGINSTACK | FIND_SOLDIER_SELECTIVE ) ) )
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
	return( FALSE );
}

BOOLEAN CycleSoldierFindStack( UINT16 usMapPos )
{
	UINT16  usSoldierIndex;
	UINT32	uiMercFlags;

	// Have we initalized for this yet?
	if ( !gfHandleStack )
	{
		if ( FindSoldier( usMapPos, &usSoldierIndex, &uiMercFlags , FINDSOLDIERSAMELEVEL( gsInterfaceLevel ) | FIND_SOLDIER_BEGINSTACK ) )
		{
			gfHandleStack = TRUE;
		}

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
			gusUIFullTargetID = gSoldierStack.ubIDs[ gSoldierStack.bCur ];
			guiUIFullTargetFlags = GetSoldierFindFlags( gusUIFullTargetID );
			guiUITargetSoldierId = gusUIFullTargetID;
			gfUIFullTargetFound			 = TRUE;
		}
		else
		{
			gfUIFullTargetFound			 = FALSE;
		}


	}

	// Return if we are in the cycle mode now...
	return( gfHandleStack );
}


SOLDIERTYPE * SimpleFindSoldier( INT16 sGridNo, INT8 bLevel )
{
	UINT8 ubID;

	ubID = WhoIsThere2( sGridNo, bLevel );
	if ( ubID == NOBODY )
	{
		return( NULL );
	}
	else
	{
		return( MercPtrs[ ubID ] );
	}
}

BOOLEAN IsValidTargetMerc( UINT8 ubSoldierID )
{
	SOLDIERTYPE *pSoldier = MercPtrs[ ubSoldierID ];


	// CHECK IF ACTIVE!
	if ( !pSoldier->bActive )
	{
		return( FALSE );
	}

	// CHECK IF DEAD
	if( pSoldier->bLife == 0 )
	{
		//return( FALSE );
	}

	// IF BAD GUY - CHECK VISIVILITY
	if ( pSoldier->bTeam != gbPlayerNum )
	{
		if ( pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS) )
		{
			return( FALSE  );
		}
	}

	return( TRUE );
}


BOOLEAN IsGridNoInScreenRect( INT16 sGridNo, SGPRect *pRect )
{
	INT32 iXTrav, iYTrav;
	INT16	sMapPos;

	// Start with top left corner
	iXTrav = pRect->iLeft;
	iYTrav = pRect->iTop;

	do
	{
		do
		{
			GetScreenXYGridNo( (INT16)iXTrav, (INT16)iYTrav, &sMapPos );

			if ( sMapPos == sGridNo )
			{
				return( TRUE );
			}

			iXTrav += WORLD_TILE_X;

		} while( iXTrav < pRect->iRight );

		iYTrav += WORLD_TILE_Y;
		iXTrav = pRect->iLeft;

	} while( iYTrav < pRect->iBottom );

	return( FALSE );
}


void GetSoldierScreenRect( SOLDIERTYPE *pSoldier, SGPRect *pRect )
{
		INT16 sMercScreenX, sMercScreenY;
		UINT16	usAnimSurface;
//		ETRLEObject *pTrav;
//		UINT32 usHeight, usWidth;

		GetSoldierScreenPos( pSoldier, &sMercScreenX, &sMercScreenY );

		usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );
		if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
		{
			pRect->iLeft		= sMercScreenX;
			pRect->iTop			= sMercScreenY;
			pRect->iBottom	= sMercScreenY + 5;
			pRect->iRight		= sMercScreenX + 5;

			return;
		}

		//pTrav = &(gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject->pETRLEObject[ pSoldier->usAniFrame ] );
		//usHeight				= (UINT32)pTrav->usHeight;
		//usWidth					= (UINT32)pTrav->usWidth;

		pRect->iLeft		= sMercScreenX;
		pRect->iTop			= sMercScreenY;
		pRect->iBottom	= sMercScreenY + pSoldier->sBoundingBoxHeight;
		pRect->iRight		= sMercScreenX + pSoldier->sBoundingBoxWidth;
}

void GetSoldierAnimDims( SOLDIERTYPE *pSoldier, INT16 *psHeight, INT16 *psWidth )
{
	UINT16											 usAnimSurface;

	usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

	if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
	{
		*psHeight					= (INT16)5;
		*psWidth					= (INT16)5;

		return;
	}

	// OK, noodle here on what we should do... If we take each frame, it will be different slightly
	// depending on the frame and the value returned here will vary thusly. However, for the
	// uses of this function, we should be able to use just the first frame...

	if ( pSoldier->usAniFrame >= gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject->usNumberOfObjects )
	{
		int i = 0;
	}

	*psHeight					= (INT16)pSoldier->sBoundingBoxHeight;
	*psWidth					= (INT16)pSoldier->sBoundingBoxWidth;
}

void GetSoldierAnimOffsets( SOLDIERTYPE *pSoldier, INT16 *sOffsetX, INT16 *sOffsetY )
{
	UINT16											 usAnimSurface;

	usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

	if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
	{
		*sOffsetX					= (INT16)0;
		*sOffsetY					= (INT16)0;

		return;
	}

	*sOffsetX					= (INT16)pSoldier->sBoundingBoxOffsetX;
	*sOffsetY					= (INT16)pSoldier->sBoundingBoxOffsetY;
}

void GetSoldierScreenPos( SOLDIERTYPE *pSoldier, INT16 *psScreenX, INT16 *psScreenY )
{
		INT16 sMercScreenX, sMercScreenY;
		FLOAT dOffsetX, dOffsetY;
		FLOAT dTempX_S, dTempY_S;
		UINT16											 usAnimSurface;
//		ETRLEObject *pTrav;

		usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

		if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
		{
			*psScreenX = 0;
			*psScreenY = 0;
			return;
		}

		// Get 'TRUE' merc position
		dOffsetX = pSoldier->dXPos - gsRenderCenterX;
		dOffsetY = pSoldier->dYPos - gsRenderCenterY;

		FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

		//pTrav = &(gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject->pETRLEObject[ pSoldier->usAniFrame ] );

		sMercScreenX = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)dTempX_S;
		sMercScreenY = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)dTempY_S;

		// Adjust starting screen coordinates
		sMercScreenX	-= gsRenderWorldOffsetX;
		sMercScreenY	-= gsRenderWorldOffsetY;
		sMercScreenY	-= gpWorldLevelData[ pSoldier->sGridNo ].sHeight;

		// Adjust for render height
		sMercScreenY += gsRenderHeight;

		// Add to start position of dest buffer
		//sMercScreenX += pTrav->sOffsetX;
		//sMercScreenY += pTrav->sOffsetY;
		sMercScreenX += pSoldier->sBoundingBoxOffsetX;
		sMercScreenY += pSoldier->sBoundingBoxOffsetY;


		sMercScreenY -= pSoldier->sHeightAdjustment;

		*psScreenX = sMercScreenX;
		*psScreenY = sMercScreenY;
}

// THE TRUE SCREN RECT DOES NOT TAKE THE OFFSETS OF BUDDY INTO ACCOUNT!
void GetSoldierTRUEScreenPos( SOLDIERTYPE *pSoldier, INT16 *psScreenX, INT16 *psScreenY )
{
		INT16 sMercScreenX, sMercScreenY;
		FLOAT dOffsetX, dOffsetY;
		FLOAT dTempX_S, dTempY_S;
		UINT16											 usAnimSurface;

		usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

		if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
		{
			*psScreenX = 0;
			*psScreenY = 0;
			return;
		}

		// Get 'TRUE' merc position
		dOffsetX = pSoldier->dXPos - gsRenderCenterX;
		dOffsetY = pSoldier->dYPos - gsRenderCenterY;

		FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

		sMercScreenX = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)dTempX_S;
		sMercScreenY = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)dTempY_S;

		// Adjust starting screen coordinates
		sMercScreenX	-= gsRenderWorldOffsetX;
		sMercScreenY	-= gsRenderWorldOffsetY;

		// Adjust for render height
		sMercScreenY += gsRenderHeight;
		sMercScreenY	-= gpWorldLevelData[ pSoldier->sGridNo ].sHeight;

		sMercScreenY -= pSoldier->sHeightAdjustment;


		*psScreenX = sMercScreenX;
		*psScreenY = sMercScreenY;
}


BOOLEAN GridNoOnScreen( INT16 sGridNo )
{
	INT16 sNewCenterWorldX, sNewCenterWorldY;
	INT16 sWorldX;
	INT16 sWorldY;
  INT16 sAllowance = 20;

  if ( gsVIEWPORT_WINDOW_START_Y == 20 )
  {
    sAllowance = 40;
  }

	ConvertGridNoToXY( sGridNo, &sNewCenterWorldX, &sNewCenterWorldY );

	// Get screen coordinates for current position of soldier
	GetWorldXYAbsoluteScreenXY( (INT16)(sNewCenterWorldX), (INT16)(sNewCenterWorldY), &sWorldX, &sWorldY);

	// ATE: OK, here, adjust the top value so that it's a tile and a bit over, because of our mercs!
	if ( sWorldX >= gsTopLeftWorldX && sWorldX <= gsBottomRightWorldX &&
			 sWorldY >= ( gsTopLeftWorldY + sAllowance )	&& sWorldY <= ( gsBottomRightWorldY + 20 ) )
	{
		return( TRUE );
	}
	return( FALSE );
}


BOOLEAN SoldierOnScreen( UINT16 usID )
{
	SOLDIERTYPE *pSoldier;

	// Get pointer of soldier
	pSoldier = MercPtrs[ usID ];

	return( GridNoOnScreen( pSoldier->sGridNo ) );
}


BOOLEAN SoldierOnVisibleWorldTile( SOLDIERTYPE *pSoldier )
{
	return( GridNoOnVisibleWorldTile( pSoldier->sGridNo ) );
}



BOOLEAN SoldierLocationRelativeToScreen( INT16 sGridNo, UINT16 usReasonID, INT8 *pbDirection, UINT32 *puiScrollFlags )
{
	INT16 sWorldX;
	INT16 sWorldY;
	INT16 sY, sX;
	static BOOLEAN fCountdown = 0;
	INT16 sScreenCenterX, sScreenCenterY;
	INT16 sDistToCenterY, sDistToCenterX;

	*puiScrollFlags = 0;

	sX = CenterX( sGridNo );
	sY = CenterY( sGridNo );

	// Get screen coordinates for current position of soldier
	GetWorldXYAbsoluteScreenXY( (INT16)(sX/CELL_X_SIZE), (INT16)(sY/CELL_Y_SIZE), &sWorldX, &sWorldY);

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
	*pbDirection = atan8( gsRenderCenterX, gsRenderCenterY, (INT16)(sX), (INT16)(sY) );

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

	if ( IsPointInScreenRect( sX, sY, &aRect ) )
	{
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN FindRelativeSoldierPosition( SOLDIERTYPE *pSoldier, UINT16 *usFlags, INT16 sX, INT16 sY )
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

				if ( dRelPer < .2 )
				{
					(*usFlags )= TILE_FLAG_HEAD;
					return( TRUE );
				}
				else if ( dRelPer < .6 )
				{
					(*usFlags )= TILE_FLAG_MID;
					return( TRUE );
				}
				else
				{
					(*usFlags )= TILE_FLAG_FEET;
					return( TRUE );
				}
				break;

			case ANIM_CROUCH:

				if ( dRelPer < .2 )
				{
					(*usFlags )= TILE_FLAG_HEAD;
					return( TRUE );
				}
				else if ( dRelPer < .7 )
				{
					(*usFlags )= TILE_FLAG_MID;
					return( TRUE );
				}
				else
				{
					(*usFlags )= TILE_FLAG_FEET;
					return( TRUE );
				}
				break;

		}
	}

	return( FALSE );
}

// VERY quickly finds a soldier at gridno , ( that is visible )
UINT8 QuickFindSoldier( INT16 sGridNo )
{
	UINT32 cnt;
	SOLDIERTYPE *pSoldier = NULL;

	// Loop through all mercs and make go
	for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
	{
		pSoldier = MercSlots[ cnt ];

		if ( pSoldier != NULL )
		{
			if ( pSoldier->sGridNo == sGridNo && pSoldier->bVisible != -1 )
			{
				return( (UINT8)cnt );
			}
		}

	}

	return( NOBODY );
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

