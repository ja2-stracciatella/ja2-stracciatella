#include "Directories.h"
#include "Real_Time_Input.h"
#include "Spread_Burst.h"
#include "VObject.h"
#include "Soldier_Control.h"
#include "Soldier_Find.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "VSurface.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"

#define MAX_BURST_LOCATIONS 50


struct BURST_LOCATIONS
{
	INT16 sX;
	INT16 sY;
	INT16 sGridNo;
};


static BURST_LOCATIONS gsBurstLocations[MAX_BURST_LOCATIONS];
static INT8            gbNumBurstLocations = 0;

static SGPVObject* guiBURSTACCUM;


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
	UINT8 ubShotsPerBurst;
	FLOAT dAccululator = 0;
	FLOAT dStep = 0;
	INT32 cnt;
	UINT8 ubLocationNum;

	// OK, using the # of locations, spread them evenly between our current weapon shots per burst value

	// Get shots per burst
	ubShotsPerBurst = GCM->getWeapon( pSoldier->inv[ HANDPOS].usItem)->ubShotsPerBurst;

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
	UINT8 ubShotsPerBurst;
	FLOAT dAccululator = 0;
	FLOAT dStep = 0;
	INT32 cnt;
	UINT8 ubLocationNum;

	// OK, using the # of locations, spread them evenly between our current weapon shots per burst value

	// Get shots per burst
	ubShotsPerBurst = GCM->getWeapon( pSoldier->inv[ HANDPOS].usItem)->ubShotsPerBurst;

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


void RenderAccumulatedBurstLocations( )
{
	INT32 cnt;
	INT16 sGridNo;

	if ( !gfBeginBurstSpreadTracking )
	{
		return;
	}

	if ( gbNumBurstLocations == 0 )
	{
		return;
	}

	// Loop through each location...

	// If on screen, render

	// Check if it already exists!
	for ( cnt = 0; cnt < gbNumBurstLocations; cnt++ )
	{
		sGridNo = gsBurstLocations[ cnt ].sGridNo;

		if ( GridNoOnScreen( sGridNo ) )
		{
			INT16 dOffsetX, dOffsetY;
			INT16 dTempX_S, dTempY_S;
			INT16 sXPos, sYPos;

			dOffsetX = (FLOAT)( gsBurstLocations[ cnt ].sX - gsRenderCenterX );
			dOffsetY = (FLOAT)( gsBurstLocations[ cnt ].sY - gsRenderCenterY );

			// Calculate guy's position
			FromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

			sXPos = ( g_ui.m_tacticalMapCenterX ) + (INT16)dTempX_S;
			sYPos = ( g_ui.m_tacticalMapCenterY ) + (INT16)dTempY_S - gpWorldLevelData[ sGridNo ].sHeight;

			// Adjust for render height
			sYPos += gsRenderHeight;

			//sScreenY -= gpWorldLevelData[ sGridNo ].sHeight;

			// Center circle!
			//sXPos -= 10;
			//sYPos -= 10;

			RegisterBackgroundRectSingleFilled(sXPos, sYPos, 40, 40);

			BltVideoObject(FRAME_BUFFER, guiBURSTACCUM, 1, sXPos, sYPos);
		}
	}
}


void LoadSpreadBurstGraphics()
{
	guiBURSTACCUM = AddVideoObjectFromFile(INTERFACEDIR "/burst1.sti");
}


void DeleteSpreadBurstGraphics()
{
	DeleteVideoObject(guiBURSTACCUM);
}
