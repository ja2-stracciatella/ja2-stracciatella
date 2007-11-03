#include "Local.h"
#include "HImage.h"
#include "VObject.h"
#include "Interface_Utils.h"
#include "Render_Dirty.h"
#include "Interface.h"
#include "Interface_Control.h"
#include "SysUtil.h"
#include "Faces.h"
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
	"INTERFACE/eldorado.sti",
	"INTERFACE/Hummer.sti",
	"INTERFACE/ice Cream Truck.sti",
	"INTERFACE/Jeep.sti",
};


// load int he portraits for the car faces that will be use in mapscreen
BOOLEAN LoadCarPortraitValues( void )
{
	INT32 iCounter = 0;

	if( giCarPortraits[ 0 ] != -1 )
	{
		return FALSE;
	}
	for( iCounter = 0; iCounter < NUMBER_CAR_PORTRAITS; iCounter++ )
	{
		giCarPortraits[iCounter] = AddVideoObjectFromFile(pbCarPortraitFileNames[iCounter]);
		CHECKF(giCarPortraits[iCounter] != NO_VOBJECT);
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
}


static void DrawBar(UINT32 XPos, UINT32 YPos, UINT32 Height, UINT16 Color, UINT16 ShadowColor, UINT8* DestBuf)
{
	LineDraw(TRUE, XPos + 0, YPos, XPos + 0, YPos - Height, ShadowColor, DestBuf);
	LineDraw(TRUE, XPos + 1, YPos, XPos + 1, YPos - Height, Color,       DestBuf);
	LineDraw(TRUE, XPos + 2, YPos, XPos + 2, YPos - Height, ShadowColor, DestBuf);
}


static void DrawLifeUIBar(const SOLDIERTYPE* pSoldier, UINT32 XPos, UINT32 YPos, UINT32 MaxHeight, UINT8* pDestBuf)
{
	UINT32 Height;

	// FIRST DO MAX LIFE
	Height = MaxHeight * pSoldier->bLife / 100;
	DrawBar(XPos, YPos, Height, Get16BPPColor(LIFE_BAR), Get16BPPColor(LIFE_BAR_SHADOW), pDestBuf);

	// NOW DO BANDAGE
	// Calculate bandage
	UINT32 Bandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;
	if (Bandage != 0)
	{
		YPos   -= Height;
		Height  = MaxHeight * Bandage / 100;
		DrawBar(XPos, YPos, Height, Get16BPPColor(BANDAGE_BAR), Get16BPPColor(BANDAGE_BAR_SHADOW), pDestBuf);
	}

	// NOW DO BLEEDING
	if (pSoldier->bBleeding != 0)
	{
		YPos   -= Height;
		Height  = MaxHeight * pSoldier->bBleeding / 100;
		DrawBar(XPos, YPos, Height, Get16BPPColor(BLEEDING_BAR), Get16BPPColor(BLEEDING_BAR_SHADOW), pDestBuf);
	}
}


static void DrawBreathUIBar(const SOLDIERTYPE* pSoldier, UINT32 XPos, UINT32 sYPos, UINT32 MaxHeight, UINT8* pDestBuf)
{
	UINT32 Height;

	if (pSoldier->bBreathMax <= 97)
	{
		Height = MaxHeight * (pSoldier->bBreathMax + 3) / 100;
		// the old background colors for breath max diff
		DrawBar(XPos, sYPos, Height, Get16BPPColor(BREATH_BAR_SHAD_BACK), Get16BPPColor(BREATH_BAR_SHAD_BACK), pDestBuf);
	}

	Height = MaxHeight * pSoldier->bBreathMax / 100;
	DrawBar(XPos, sYPos, Height, Get16BPPColor(CURR_MAX_BREATH), Get16BPPColor(CURR_MAX_BREATH_SHADOW), pDestBuf);

	// NOW DO BREATH
	Height = MaxHeight * pSoldier->bBreath / 100;
	DrawBar(XPos, sYPos, Height, Get16BPPColor(CURR_BREATH_BAR), Get16BPPColor(CURR_BREATH_BAR_SHADOW), pDestBuf);
}


static void DrawMoraleUIBar(const SOLDIERTYPE* pSoldier, UINT32 XPos, UINT32 YPos, UINT32 MaxHeight, UINT8* pDestBuf)
{
	UINT32 Height = MaxHeight * pSoldier->bMorale / 100;
	DrawBar(XPos, YPos, Height, Get16BPPColor(MORALE_BAR), Get16BPPColor(MORALE_BAR_SHADOW), pDestBuf);
}


void DrawSoldierUIBars(const SOLDIERTYPE* pSoldier, INT16 sXPos, INT16 sYPos, BOOLEAN fErase, UINT32 uiBuffer)
{
	const UINT32 BarWidth  =  3;
	const UINT32 BarHeight = 42;
	const UINT32 BreathOff =  6;
	const UINT32 MoraleOff = 12;

	// Erase what was there
	if (fErase)
	{
		RestoreExternBackgroundRect(sXPos, sYPos - BarHeight, MoraleOff + BarWidth, BarHeight + 1);
	}

	if (pSoldier->bLife == 0) return;

	if (!(pSoldier->uiStatusFlags & SOLDIER_ROBOT))
	{
		// DO MAX BREATH
		// brown guy
		HVOBJECT hHandle = GetVideoObject(guiBrownBackgroundForTeamPanel);
		UINT16 Region;
		if (guiCurrentScreen != MAP_SCREEN &&
				gusSelectedSoldier == pSoldier->ubID &&
				gTacticalStatus.ubCurrentTeam == OUR_TEAM &&
				OK_INTERRUPT_MERC(pSoldier))
		{
			Region = 1; // gold, the second entry in the .sti
		}
		else
		{
			Region = 0; // brown, first entry
		}
		BltVideoObject(uiBuffer, hHandle, Region, sXPos + BreathOff, sYPos - BarHeight);
	}

	UINT32 uiDestPitchBYTES;
	UINT8* pDestBuf = LockVideoSurface(uiBuffer, &uiDestPitchBYTES);
	SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	DrawLifeUIBar(pSoldier, sXPos, sYPos, BarHeight, pDestBuf);
	if (!(pSoldier->uiStatusFlags & SOLDIER_ROBOT))
	{
		DrawBreathUIBar(pSoldier, sXPos + BreathOff, sYPos, BarHeight, pDestBuf);
		if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE))
		{
			DrawMoraleUIBar(pSoldier, sXPos + MoraleOff, sYPos, BarHeight, pDestBuf);
		}
	}

	UnLockVideoSurface(uiBuffer);
}


void DrawItemUIBarEx(const OBJECTTYPE* pObject, UINT8 ubStatus, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight, INT16 sColor1, INT16 sColor2, BOOLEAN fErase, UINT32 uiBuffer)
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
	SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


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


void RenderSoldierFace(const SOLDIERTYPE* pSoldier, INT16 sFaceX, INT16 sFaceY, BOOLEAN fAutoFace)
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
				ExternRenderFaceFromSoldier(guiSAVEBUFFER, pSoldier, sFaceX, sFaceY);
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
