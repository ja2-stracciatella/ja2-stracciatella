#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include <string.h>
	#include "WCheck.h"
	#include "stdlib.h"
	#include "Debug.h"
	#include "Soldier_Control.h"
	#include "Weapons.h"
	#include "Cursor_Control.h"
	#include "Cursors.h"
	#include "Soldier_Find.h"
	#include "Isometric_Utils.h"
	#include "RenderWorld.h"
	#include "Render_Dirty.h"
	#include "Interface.h"
#endif


#define		MAX_BURST_LOCATIONS		50


typedef struct
{
	INT16 sX;
	INT16 sY;
	INT16 sGridNo;

} BURST_LOCATIONS;



BURST_LOCATIONS			gsBurstLocations[ MAX_BURST_LOCATIONS ];
INT8								gbNumBurstLocations = 0;

extern BOOLEAN gfBeginBurstSpreadTracking;


void ResetBurstLocations( )
{
	gbNumBurstLocations = 0;
}

void AccumulateBurstLocation( INT16 sGridNo )
{
	INT32 cnt;

	if ( gbNumBurstLocations < MAX_BURST_LOCATIONS )
	{
		// Check if it already exists!
		for ( cnt = 0; cnt < gbNumBurstLocations; cnt++ )
		{
			if ( gsBurstLocations[ cnt ].sGridNo == sGridNo )
			{
				return;
			}
		}

		gsBurstLocations[ gbNumBurstLocations ].sGridNo = sGridNo;

		// Get cell X, Y from mouse...
		GetMouseWorldCoords( &( gsBurstLocations[ gbNumBurstLocations ].sX ), &( gsBurstLocations[ gbNumBurstLocations ].sY ) );

		gbNumBurstLocations++;
	}
}



void PickBurstLocations( SOLDIERTYPE *pSoldier )
{
	UINT8		ubShotsPerBurst;
	FLOAT		dAccululator = 0;
	FLOAT		dStep = 0;
	INT32		cnt;
	UINT8		ubLocationNum;

	// OK, using the # of locations, spread them evenly between our current weapon shots per burst value

	// Get shots per burst
	ubShotsPerBurst = Weapon[ pSoldier->inv[ HANDPOS ].usItem ].ubShotsPerBurst;

	// Use # gridnos accululated and # burst shots to determine accululator
	dStep = gbNumBurstLocations / (FLOAT)ubShotsPerBurst;

	//Loop through our shots!
	for ( cnt = 0; cnt < ubShotsPerBurst; cnt++ )
	{
		// Get index into list
		ubLocationNum = (UINT8)( dAccululator );

		// Add to merc location
		pSoldier->sSpreadLocations[ cnt ] = gsBurstLocations[ ubLocationNum ].sGridNo;

		// Acculuate index value
		dAccululator += dStep;
	}

	// OK, they have been added
}

void AIPickBurstLocations( SOLDIERTYPE *pSoldier, INT8 bTargets, SOLDIERTYPE *pTargets[5] )
{
	UINT8		ubShotsPerBurst;
	FLOAT		dAccululator = 0;
	FLOAT		dStep = 0;
	INT32		cnt;
	UINT8		ubLocationNum;

	// OK, using the # of locations, spread them evenly between our current weapon shots per burst value

	// Get shots per burst
	ubShotsPerBurst = Weapon[ pSoldier->inv[ HANDPOS ].usItem ].ubShotsPerBurst;

	// Use # gridnos accululated and # burst shots to determine accululator
	//dStep = gbNumBurstLocations / (FLOAT)ubShotsPerBurst;
	// CJC: tweak!
	dStep = bTargets / (FLOAT)ubShotsPerBurst;

	//Loop through our shots!
	for ( cnt = 0; cnt < ubShotsPerBurst; cnt++ )
	{
		// Get index into list
		ubLocationNum = (UINT8)( dAccululator );

		// Add to merc location
		pSoldier->sSpreadLocations[ cnt ] = pTargets[ubLocationNum]->sGridNo;

		// Acculuate index value
		dAccululator += dStep;
	}

	// OK, they have been added
}


extern HVOBJECT GetCursorFileVideoObject( UINT32 uiCursorFile );


void RenderAccumulatedBurstLocations( )
{
	INT32			cnt;
	INT16			sGridNo;
	HVOBJECT	hVObject;

	if ( !gfBeginBurstSpreadTracking )
	{
		return;
	}

	if ( gbNumBurstLocations == 0 )
	{
		return;
	}

	// Loop through each location...
	GetVideoObject( &hVObject, guiBURSTACCUM );

	// If on screen, render

	// Check if it already exists!
	for ( cnt = 0; cnt < gbNumBurstLocations; cnt++ )
	{
		sGridNo = gsBurstLocations[ cnt ].sGridNo;

		if ( GridNoOnScreen( sGridNo ) )
		{
			FLOAT				dOffsetX, dOffsetY;
			FLOAT				dTempX_S, dTempY_S;
			INT16				sXPos, sYPos;
			INT32				iBack;

			dOffsetX = (FLOAT)( gsBurstLocations[ cnt ].sX - gsRenderCenterX );
			dOffsetY = (FLOAT)( gsBurstLocations[ cnt ].sY - gsRenderCenterY );

			// Calculate guy's position
			FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

			sXPos = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)dTempX_S;
			sYPos = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)dTempY_S - gpWorldLevelData[ sGridNo ].sHeight;

			// Adjust for offset position on screen
			sXPos -= gsRenderWorldOffsetX;
			sYPos -= gsRenderWorldOffsetY;

			// Adjust for render height
			sYPos += gsRenderHeight;

			//sScreenY -= gpWorldLevelData[ sGridNo ].sHeight;

			// Center circle!
			//sXPos -= 10;
			//sYPos -= 10;

			iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, sXPos, sYPos, (INT16)(sXPos +40 ), (INT16)(sYPos + 40 ) );
			if ( iBack != -1 )
			{
				SetBackgroundRectFilled( iBack );
			}

			BltVideoObject(  FRAME_BUFFER, hVObject, 1, sXPos, sYPos, VO_BLT_SRCTRANSPARENCY, NULL );
		}
	}
}


