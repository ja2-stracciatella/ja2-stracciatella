#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include <stdio.h>
	#include <stdarg.h>
	#include "SGP.h"
	#include "HImage.h"
	#include "VObject.h"
	#include "Interface_Utils.h"
	#include "Render_Dirty.h"
	#include "Interface.h"
	#include "Interface_Control.h"
	#include "SysUtil.h"
	#include "Faces.h"
	#include "Render_Dirty.h"
	#include "Weapons.h"
	#include "Overhead.h"
	#include "Soldier_Macros.h"
	#include "Line.h"
	#include "WCheck.h"
	#include "Vehicles.h"
	#include "JAScreens.h"
	#include "Video.h"
	#include "VSurface.h"
	#include "ScreenIDs.h"
#endif

#define			LIFE_BAR_SHADOW							FROMRGB( 108, 12, 12 )
#define			LIFE_BAR										FROMRGB( 200, 0, 0 )
#define			BANDAGE_BAR_SHADOW					FROMRGB( 156, 60, 60 )
#define			BANDAGE_BAR									FROMRGB( 222, 132, 132 )
#define			BLEEDING_BAR_SHADOW					FROMRGB( 128, 128, 60 )
#define			BLEEDING_BAR								FROMRGB( 240,  240, 20 )
#define			CURR_BREATH_BAR_SHADOW			FROMRGB( 8,		12, 118 ) // the MAX max breatth, always at 100%
#define			CURR_BREATH_BAR							FROMRGB( 8,		12, 160 )
#define     CURR_MAX_BREATH							FROMRGB( 0,		0,	0		) // the current max breath, black
#define     CURR_MAX_BREATH_SHADOW			FROMRGB( 0,		0,	0		)
#define			MORALE_BAR_SHADOW						FROMRGB( 8,		112, 12 )
#define			MORALE_BAR									FROMRGB( 8,		180, 12 )
#define			BREATH_BAR_SHADOW						FROMRGB( 60,	108, 108 ) // the lt blue current breath
#define			BREATH_BAR									FROMRGB( 113,	178, 218 )
#define			BREATH_BAR_SHAD_BACK				FROMRGB( 1,1,1 )
#define			FACE_WIDTH									48
#define			FACE_HEIGHT									43


// backgrounds for breath max background
extern UINT32 guiBrownBackgroundForTeamPanel;
extern UINT32 guiGoldBackgroundForTeamPanel;

// selected grunt
extern UINT16 gusSelectedSoldier;

// car portraits
enum{
	ELDORADO_PORTRAIT =0,
	HUMMER_PORTRAIT,
	ICE_CREAM_TRUCT_PORTRAIT,
	JEEP_PORTRAIT,
	NUMBER_CAR_PORTRAITS,
};

// the ids for the car portraits
INT32 giCarPortraits[ 4 ] = { -1, -1, -1, -1 };

// the car portrait file names
const char *pbCarPortraitFileNames[] = {
	"INTERFACE\\eldorado.sti",
	"INTERFACE\\Hummer.sti",
	"INTERFACE\\ice Cream Truck.sti",
	"INTERFACE\\Jeep.sti",
};


// load int he portraits for the car faces that will be use in mapscreen
BOOLEAN LoadCarPortraitValues( void )
{
	INT32 iCounter = 0;
	VOBJECT_DESC     VObjectDesc;

	if( giCarPortraits[ 0 ] != -1 )
	{
		return FALSE;
	}
	for( iCounter = 0; iCounter < NUMBER_CAR_PORTRAITS; iCounter++ )
	{
		VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
		sprintf( VObjectDesc.ImageFile, pbCarPortraitFileNames[ iCounter ] );
		CHECKF( AddVideoObject( &VObjectDesc, &giCarPortraits[ iCounter ] ) );
	}
	return( TRUE );
}

// get rid of the images we loaded for the mapscreen car portraits
void UnLoadCarPortraits( void )
{
	INT32 iCounter = 0;

	// car protraits loaded?
	if( giCarPortraits[ 0 ] == -1 )
	{
		return;
	}
	for( iCounter = 0; iCounter < NUMBER_CAR_PORTRAITS; iCounter++ )
	{
		DeleteVideoObjectFromIndex( giCarPortraits[ iCounter ] );
		giCarPortraits[ iCounter ] = -1;
	}
	return;
}



void DrawLifeUIBarEx( SOLDIERTYPE *pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight, BOOLEAN fErase, UINT32 uiBuffer )
{
	FLOAT											 dStart, dEnd, dPercentage;
	//UINT16										 usLineColor;

	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	UINT16										 usLineColor;
	INT8											 bBandage;

	// Erase what was there
	if( fErase )
	{
		RestoreExternBackgroundRect( sXPos, (INT16)(sYPos - sHeight), sWidth, (INT16)(sHeight + 1 ) );
	}

	if( pSoldier->bLife == 0 )
	{
		// are they dead?
		return;
	}

	pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );


	// FIRST DO MAX LIFE
	dPercentage = (FLOAT)pSoldier->bLife / (FLOAT)100;
	dEnd				=	dPercentage * sHeight;
	dStart			= sYPos;

	usLineColor = Get16BPPColor( LIFE_BAR_SHADOW );
	RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( LIFE_BAR );
	RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( LIFE_BAR_SHADOW );
	RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

	// NOW DO BANDAGE

	// Calculate bandage
	bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

	if ( bBandage )
	{
		dPercentage = (FLOAT)bBandage / (FLOAT)100;
		dStart			= (FLOAT)( sYPos - dEnd );
		dEnd				=	( dPercentage * sHeight );

		usLineColor = Get16BPPColor( BANDAGE_BAR_SHADOW );
		RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

		usLineColor = Get16BPPColor( BANDAGE_BAR );
		RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

		usLineColor = Get16BPPColor( BANDAGE_BAR_SHADOW );
		RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );
	}

	// NOW DO BLEEDING
	if ( pSoldier->bBleeding )
	{
		dPercentage = (FLOAT)pSoldier->bBleeding / (FLOAT)100;
		dStart			= (FLOAT)( dStart - dEnd );
		dEnd				=	( dPercentage * sHeight );

		usLineColor = Get16BPPColor( BLEEDING_BAR_SHADOW );
		RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

		usLineColor = Get16BPPColor( BLEEDING_BAR );
		RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

		usLineColor = Get16BPPColor( BLEEDING_BAR_SHADOW );
		RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );
	}

	UnLockVideoSurface( uiBuffer );

}


void DrawBreathUIBarEx( SOLDIERTYPE *pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight, BOOLEAN fErase, UINT32 uiBuffer )
{
	FLOAT											 dStart, dEnd, dPercentage;
	//UINT16										 usLineColor;

	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	UINT16										 usLineColor;
	HVOBJECT hHandle;

	// Erase what was there
	if ( fErase )
	{
		RestoreExternBackgroundRect( sXPos, (INT16)(sYPos - sHeight), sWidth, (INT16)(sHeight + 1 ) );
	}

	if( pSoldier->bLife == 0 )
	{
		// are they dead?
		return;
	}



	//  DO MAX MAX BREATH
	dPercentage = 1.;
	dEnd				=	dPercentage * sHeight;
	dStart			= sYPos;

	// brown guy
	GetVideoObject( &hHandle, guiBrownBackgroundForTeamPanel );

	// DO MAX BREATH
	if( guiCurrentScreen != MAP_SCREEN )
	{
		if( gusSelectedSoldier == pSoldier->ubID && gTacticalStatus.ubCurrentTeam == OUR_TEAM && OK_INTERRUPT_MERC( pSoldier ) )
		{
			// gold, the second entry in the .sti
		  BltVideoObject( uiBuffer , hHandle, 1, sXPos, ( INT16 )( sYPos - sHeight ));

		}
		else
		{
			// brown, first entry
		  BltVideoObject( uiBuffer , hHandle, 0, sXPos, ( INT16 )( sYPos - sHeight ));

		}
	}
	else
	{
		// brown, first entry
	  BltVideoObject( uiBuffer , hHandle, 0, sXPos, ( INT16 )( sYPos - sHeight ));

	}

	pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );

	if( pSoldier->bBreathMax <= 97 )
	{
		dPercentage = (FLOAT)( ( pSoldier->bBreathMax + 3 ) )/ (FLOAT)100;
		dEnd				=	dPercentage * sHeight;
		dStart			= sYPos;

		// the old background colors for breath max diff
		usLineColor = Get16BPPColor( BREATH_BAR_SHAD_BACK );
		RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

		usLineColor = Get16BPPColor( BREATH_BAR_SHAD_BACK );
		RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

		usLineColor = Get16BPPColor( BREATH_BAR_SHAD_BACK );
		RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );
	}




	dPercentage = (FLOAT)pSoldier->bBreathMax / (FLOAT)100;
	dEnd				=	dPercentage * sHeight;
	dStart			= sYPos;

	usLineColor = Get16BPPColor( CURR_MAX_BREATH_SHADOW );
	RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( CURR_MAX_BREATH );
	RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( CURR_MAX_BREATH_SHADOW );
	RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );


	// NOW DO BREATH
	dPercentage = (FLOAT)pSoldier->bBreath / (FLOAT)100;
	dEnd				=	dPercentage * sHeight;
	dStart			= sYPos;

	usLineColor = Get16BPPColor( CURR_BREATH_BAR_SHADOW );
	RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( CURR_BREATH_BAR );
	RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( CURR_BREATH_BAR_SHADOW );
	RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );


	UnLockVideoSurface( uiBuffer );

}

void DrawMoraleUIBarEx( SOLDIERTYPE *pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight, BOOLEAN fErase, UINT32 uiBuffer )
{
	FLOAT											 dStart, dEnd, dPercentage;
	//UINT16										 usLineColor;

	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	UINT16										 usLineColor;

	// Erase what was there
	if ( fErase )
	{
		RestoreExternBackgroundRect( sXPos, (INT16)(sYPos - sHeight), sWidth, (INT16)(sHeight + 1 ) );
	}

	if( pSoldier->bLife == 0 )
	{
		// are they dead?
		return;
	}

	pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );


	// FIRST DO BREATH
	dPercentage = (FLOAT)pSoldier->bMorale / (FLOAT)100;
	dEnd				=	dPercentage * sHeight;
	dStart			= sYPos;

	usLineColor = Get16BPPColor( MORALE_BAR_SHADOW );
	RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( MORALE_BAR );
	RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );

	usLineColor = Get16BPPColor( MORALE_BAR_SHADOW );
	RectangleDraw( TRUE, sXPos+ 2, (INT32)dStart, sXPos + 2, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );


	UnLockVideoSurface( uiBuffer );

}


void DrawItemUIBarEx( OBJECTTYPE *pObject, UINT8 ubStatus, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight, INT16 sColor1, INT16 sColor2, BOOLEAN fErase, UINT32 uiBuffer )
{
	FLOAT											 dStart, dEnd, dPercentage;
	//UINT16										 usLineColor;

	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	UINT16										 usLineColor;
	INT16											 sValue;


	if ( ubStatus >= DRAW_ITEM_STATUS_ATTACHMENT1 )
	{
		sValue = pObject->bAttachStatus[ ubStatus - DRAW_ITEM_STATUS_ATTACHMENT1 ];
	}
	else
	{
		sValue = pObject->bStatus[ ubStatus ];
	}

	// Adjust for ammo, other thingys..
	if( Item[ pObject->usItem ].usItemClass & IC_AMMO )
	{
		sValue = sValue * 100 / Magazine[ Item[ pObject->usItem ].ubClassIndex ].ubMagSize;

		if ( sValue > 100 )
		{
			sValue = 100;
		}

	}

	if( Item[ pObject->usItem ].usItemClass & IC_KEY )
	{
		sValue =100;
	}

  // ATE: Subtract 1 to exagerate bad status
  if ( sValue < 100 && sValue > 1 )
  {
    sValue--;
  }

	// Erase what was there
	if ( fErase )
	{
		//RestoreExternBackgroundRect( sXPos, (INT16)(sYPos - sHeight), sWidth, (INT16)(sHeight + 1 ) );
	}

	pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );


	// FIRST DO BREATH
	dPercentage = (FLOAT)sValue / (FLOAT)100;
	dEnd				=	dPercentage * sHeight;
	dStart			= sYPos;

	//usLineColor = Get16BPPColor( STATUS_BAR );
	usLineColor = sColor1;
	RectangleDraw( TRUE, sXPos, (INT32)dStart, sXPos, (INT32)( dStart - dEnd ) , usLineColor, pDestBuf );

	usLineColor = sColor2;
	RectangleDraw( TRUE, sXPos+ 1, (INT32)dStart, sXPos + 1, (INT32)( dStart - dEnd ), usLineColor, pDestBuf );


	UnLockVideoSurface( uiBuffer );

	if ( uiBuffer == guiSAVEBUFFER )
	{
		RestoreExternBackgroundRect( sXPos, (INT16)(sYPos - sHeight), sWidth, (INT16)(sHeight + 1 ) );
	}
	else
	{
		InvalidateRegion( sXPos, (INT16)(sYPos - sHeight), sXPos + sWidth, (INT16)(sYPos + 1 ) );
	}

}

void RenderSoldierFace( SOLDIERTYPE *pSoldier, INT16 sFaceX, INT16 sFaceY, BOOLEAN fAutoFace )
{
	BOOLEAN fDoFace = FALSE;
	UINT8 ubVehicleType = 0;


	if ( pSoldier->bActive )
	{

		if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{
			// get the type of vehicle
			ubVehicleType = pVehicleList[ pSoldier->bVehicleID ].ubVehicleType;

			// just draw the vehicle
			BltVideoObjectFromIndex( guiSAVEBUFFER, giCarPortraits[ ubVehicleType ], 0, sFaceX, sFaceY);
			RestoreExternBackgroundRect( sFaceX, sFaceY, FACE_WIDTH, FACE_HEIGHT );

			return;
		}

		if ( fAutoFace )
		{
			// OK, check if this face actually went active...
			if ( gFacesData[ pSoldier->iFaceIndex ].uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE )
			{
				// Render as an extern face...
				fAutoFace = FALSE;
			}
			else
			{
				SetAutoFaceActiveFromSoldier( FRAME_BUFFER, guiSAVEBUFFER, pSoldier->ubID , sFaceX, sFaceY );
			//	SetAutoFaceActiveFromSoldier( FRAME_BUFFER, FACE_AUTO_RESTORE_BUFFER, pSoldier->ubID , sFaceX, sFaceY );
			}
		}

		fDoFace = TRUE;

		if ( fDoFace )
		{
			if ( fAutoFace )
			{
				RenderAutoFaceFromSoldier( pSoldier->ubID );
			}
			else
			{
				ExternRenderFaceFromSoldier( guiSAVEBUFFER, pSoldier->ubID, sFaceX, sFaceY );
			}
		}
	}
	else
	{
		// Put close panel there
		BltVideoObjectFromIndex(guiSAVEBUFFER, guiCLOSE, 5, sFaceX, sFaceY);
		RestoreExternBackgroundRect( sFaceX, sFaceY, FACE_WIDTH, FACE_HEIGHT );
	}

}
