#include "Font.h"
#include "HImage.h"
#include "Interface_Control.h"
#include "Interface_Panels.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "WorldDef.h"
#include "VSurface.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "WCheck.h"
#include "Video.h"
#include "Faces.h"
#include "Overhead.h"
#include "Gap.h"
#include "Soldier_Profile.h"
#include "Sound_Control.h"
#include "TeamTurns.h"
#include "Soldier_Macros.h"
#include "Dialogue_Control.h"
#include "Font_Control.h"
#include "Assignments.h"
#include "Random.h"
#include "Line.h"
#include "GameSettings.h"
#include "Squads.h"
#include "Interface.h"
#include "Quests.h"
#include "Animation_Control.h"
#include "Drugs_And_Alcohol.h"
#include "Interface_Items.h"
#include "Meanwhile.h"
#include "Map_Screen_Interface.h"
#include "SoundMan.h"
#include "JAScreens.h"
#include "ScreenIDs.h"


// Defines
#define		NUM_FACE_SLOTS					50


// GLOBAL FOR FACES LISTING
FACETYPE	gFacesData[ NUM_FACE_SLOTS ];
static UINT32 guiNumFaces = 0;


typedef struct RPC_SMALL_FACE_VALUES
{
	ProfileID profile;
	INT8      bEyesX;
	INT8      bEyesY;
	INT8      bMouthX;
	INT8      bMouthY;
} RPC_SMALL_FACE_VALUES;


// TODO0013
static const RPC_SMALL_FACE_VALUES gRPCSmallFaceValues[] =
{
	{ MIGUEL,    9, 8,  8, 24 },
	{ CARLOS,    8, 8,  7, 24 },
	{ IRA,      10, 8,  8, 26 },
	{ DIMITRI,   7, 8,  7, 26 },
	{ DEVIN,     6, 7,  7, 23 },
	{ ROBOT,     0, 0,  0,  0 }, // THE RAT  (  62 )
	{ HAMOUS,    8, 7,  8, 23 },
	{ SLAY,      8, 8,  8, 22 },
	{ RPC65,     0, 0,  0,  0 },
	{ DYNAMO,    9, 4,  7, 22 },
	{ SHANK,     8, 8,  8, 25 },
	{ IGGY,      4, 6,  5, 22 },
	{ VINCE,     8, 9,  7, 25 },
	{ CONRAD,    4, 7,  5, 25 },
	{ RPC71,     9, 7,  8, 22 }, // CARL     (  71 )
	{ MADDOG,    9, 7,  9, 25 },
	{ DARREL,    0, 0,  0,  0 },
	{ PERKO,     0, 0,  0,  0 },

	{ MARIA,     9, 3,  8, 23 },

	{ JOEY,      9, 3,  8, 25 },

	{ SKYRIDER, 11, 7,  9, 24 },
	{ FRED,      9, 5,  7, 23 }, // Miner    ( 106 )

	{ JOHN,      6, 4,  6, 24 },
	{ MARY,     12, 4, 10, 24 },
	{ MATT,      8, 6,  8, 23 }, // Miner    ( 148 )
	{ OSWALD,    6, 5,  6, 23 }, // Miner    ( 156 )
	{ CALVIN,   13, 7, 11, 24 }, // Miner    ( 157 )
	{ CARL,      9, 7,  8, 22 }  // Miner    ( 158 )
};


static INT32 GetFreeFace(void)
{
	UINT32 uiCount;

	for(uiCount=0; uiCount < guiNumFaces; uiCount++)
	{
		if((gFacesData[uiCount].fAllocated==FALSE) )
			return((INT32)uiCount);
	}

	if(guiNumFaces < NUM_FACE_SLOTS )
		return((INT32)guiNumFaces++);

	return(-1);
}


static void RecountFaces(void)
{
INT32 uiCount;

	for(uiCount=guiNumFaces-1; (uiCount >=0) ; uiCount--)
	{
		if( ( gFacesData[uiCount].fAllocated ) )
		{
			guiNumFaces=(UINT32)(uiCount+1);
			break;
		}
	}
}


void InitSoldierFace(SOLDIERTYPE* const s)
{
	// Check if we have a face init already
	if (s->iFaceIndex != -1) return;

	s->iFaceIndex = InitFace(s->ubProfile, s, 0);
}


static INT32 InternalInitFace(UINT8 usMercProfileID, SOLDIERTYPE* s, UINT32 uiInitFlags, INT32 iFaceFileID, UINT32 uiBlinkFrequency, UINT32 uiExpressionFrequency);


INT32	InitFace(const UINT8 usMercProfileID, SOLDIERTYPE* const s, const UINT32 uiInitFlags)
{
	UINT32	uiBlinkFrequency;
	UINT32	uiExpressionFrequency;

	if ( usMercProfileID == NO_PROFILE )
	{
		return( -1 );
	}

	uiBlinkFrequency			= gMercProfiles[ usMercProfileID ].uiBlinkFrequency;
	uiExpressionFrequency	= gMercProfiles[ usMercProfileID ].uiExpressionFrequency;

	if ( Random( 2 ) )
	{
		uiBlinkFrequency		+= Random( 2000 );
	}
	else
	{
		uiBlinkFrequency		-= Random( 2000 );
	}

	return InternalInitFace(usMercProfileID, s, uiInitFlags, gMercProfiles[usMercProfileID].ubFaceIndex, uiBlinkFrequency, uiExpressionFrequency);
}


static INT32 InternalInitFace(const UINT8 usMercProfileID, SOLDIERTYPE* const s, const UINT32 uiInitFlags, INT32 iFaceFileID, const UINT32 uiBlinkFrequency, const UINT32 uiExpressionFrequency)
{
	FACETYPE					*pFace;
	INT32							iFaceIndex;
	UINT32						uiCount;
	SGPPaletteEntry		Pal[256];

	if( ( iFaceIndex = GetFreeFace() )==(-1) )
		return(-1);

	// ATE: If we are merc profile ID #151-154, all use 151's protrait....
	if ( usMercProfileID >= 151 && usMercProfileID <= 154 )
	{
		iFaceFileID = 151;
	}


	// Check if we are a big-face....
	SGPFILENAME ImageFile;
	if ( uiInitFlags & FACE_BIGFACE )
	{
		// The filename is the profile ID!
		sprintf(ImageFile, "FACES/b%02d.sti", iFaceFileID);

    // ATE: Check for profile - if elliot , use special face :)
    if ( usMercProfileID == ELLIOT )
    {
      if ( gMercProfiles[ ELLIOT ].bNPCData > 3 && gMercProfiles[ ELLIOT ].bNPCData < 7 )
      {
			  sprintf(ImageFile, "FACES/b%02da.sti", iFaceFileID);
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData > 6 && gMercProfiles[ ELLIOT ].bNPCData < 10 )
      {
			  sprintf(ImageFile, "FACES/b%02db.sti", iFaceFileID);
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData > 9 && gMercProfiles[ ELLIOT ].bNPCData < 13 )
      {
			  sprintf(ImageFile, "FACES/b%02dc.sti", iFaceFileID);
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData > 12 && gMercProfiles[ ELLIOT ].bNPCData < 16 )
      {
			  sprintf(ImageFile, "FACES/b%02dd.sti", iFaceFileID);
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData == 17 )
      {
			  sprintf(ImageFile, "FACES/b%02de.sti", iFaceFileID);
      }
    }
	}
	else
	{
		sprintf(ImageFile, "FACES/%02d.sti", iFaceFileID);
	}

	// Load
	SGPVObject* hVObject = AddVideoObjectFromFile(ImageFile);
	if (hVObject == NO_VOBJECT)
	{
		// If we are a big face, use placeholder...
		if ( uiInitFlags & FACE_BIGFACE )
		{
			hVObject = AddVideoObjectFromFile("FACES/placeholder.sti");
			if (hVObject == NO_VOBJECT) return -1;
		}
		else
		{
			return( -1 );
		}
	}

	memset(&gFacesData[ iFaceIndex ], 0, sizeof( FACETYPE ) );

	pFace = &gFacesData[ iFaceIndex ];

	// Get profile data and set into face data
	pFace->soldier = s;

	pFace->iID						= iFaceIndex;
	pFace->fAllocated			= TRUE;

	//Default to off!
	pFace->fDisabled			= TRUE;
	pFace->video_overlay  = NULL;
	//pFace->uiEyeDelay			=	gMercProfiles[ usMercProfileID ].uiEyeDelay;
	//pFace->uiMouthDelay		= gMercProfiles[ usMercProfileID ].uiMouthDelay;
	pFace->uiEyeDelay			=	50 + Random( 30 );
	pFace->uiMouthDelay		= 120;
	pFace->ubCharacterNum = usMercProfileID;


	pFace->uiBlinkFrequency			 = uiBlinkFrequency;
	pFace->uiExpressionFrequency = uiExpressionFrequency;

	pFace->sEyeFrame		=		0;
	pFace->sMouthFrame	=		0;
	pFace->uiFlags			=  uiInitFlags;


	// Set palette
	// Build a grayscale palette! ( for testing different looks )
	for(uiCount=0; uiCount < 256; uiCount++)
	{
		Pal[uiCount].peRed=255;
		Pal[uiCount].peGreen=255;
		Pal[uiCount].peBlue=255;
	}

	hVObject->pShades[ FLASH_PORTRAIT_NOSHADE ]		  = Create16BPPPaletteShaded( hVObject->pPaletteEntry, 255, 255, 255, FALSE );
	hVObject->pShades[ FLASH_PORTRAIT_STARTSHADE ]  = Create16BPPPaletteShaded( Pal, 255, 255, 255, FALSE );
	hVObject->pShades[ FLASH_PORTRAIT_ENDSHADE ]		= Create16BPPPaletteShaded( hVObject->pPaletteEntry, 250, 25, 25, TRUE );
	hVObject->pShades[ FLASH_PORTRAIT_DARKSHADE ]		= Create16BPPPaletteShaded( hVObject->pPaletteEntry, 100, 100, 100, TRUE );
	hVObject->pShades[ FLASH_PORTRAIT_LITESHADE ]		= Create16BPPPaletteShaded( hVObject->pPaletteEntry, 100, 100, 100, FALSE );

	for(uiCount=0; uiCount < 256; uiCount++)
	{
		Pal[uiCount].peRed=(UINT8)(uiCount%128)+128;
		Pal[uiCount].peGreen=(UINT8)(uiCount%128)+128;
		Pal[uiCount].peBlue=(UINT8)(uiCount%128)+128;
	}
	hVObject->pShades[ FLASH_PORTRAIT_GRAYSHADE ]		= Create16BPPPaletteShaded( Pal, 255, 255, 255, FALSE );


	// Get FACE height, width
	const ETRLEObject* ETRLEProps;
	ETRLEProps = GetVideoObjectETRLESubregionProperties(hVObject, 0);
	if (ETRLEProps == NULL) return -1;
	pFace->usFaceWidth  = ETRLEProps->usWidth;
	pFace->usFaceHeight = ETRLEProps->usHeight;

	// OK, check # of items
	if ( hVObject->usNumberOfObjects == 8 )
	{
		pFace->fInvalidAnim = FALSE;

		// Get EYE height, width
		ETRLEProps = GetVideoObjectETRLESubregionProperties(hVObject, 1);
		if (ETRLEProps == NULL) return -1;
		pFace->usEyesWidth  = ETRLEProps->usWidth;
		pFace->usEyesHeight = ETRLEProps->usHeight;

		// Get Mouth height, width
		ETRLEProps = GetVideoObjectETRLESubregionProperties(hVObject, 5);
		if (ETRLEProps == NULL) return -1;
		pFace->usMouthWidth  = ETRLEProps->usWidth;
		pFace->usMouthHeight = ETRLEProps->usHeight;
	}
	else
	{
		pFace->fInvalidAnim = TRUE;
	}

	// Set id
	pFace->uiVideoObject = hVObject;

	return( iFaceIndex );

}


void DeleteSoldierFace( SOLDIERTYPE *pSoldier )
{
	DeleteFace( pSoldier->iFaceIndex );

	pSoldier->iFaceIndex = -1;
}


void DeleteFace( INT32 iFaceIndex )
{
	FACETYPE				*pFace;

	// Check face index
	CHECKV( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// Check for a valid slot!
	CHECKV(pFace->fAllocated);

  pFace->fCanHandleInactiveNow = TRUE;

	if ( !pFace->fDisabled )
	{
		SetAutoFaceInActive( iFaceIndex );
	}

	// If we are still talking, stop!
	if ( pFace->fTalking )
	{
		// Call dialogue handler function
		pFace->fTalking = FALSE;

		HandleDialogueEnd( pFace );
	}

	// Delete vo
	DeleteVideoObjectFromIndex( pFace->uiVideoObject );

	// Set uncallocated
	pFace->fAllocated = FALSE;

	RecountFaces( );
}


void SetAutoFaceActiveFromSoldier(SGPVSurface* const display, SGPVSurface* const restore, const SOLDIERTYPE* const s, const UINT16 usFaceX, const UINT16 usFaceY)
{
	CHECKV(s != NULL);
	SetAutoFaceActive(display, restore, s->iFaceIndex, usFaceX, usFaceY);
}


static void GetFaceRelativeCoordinates(const FACETYPE* pFace, UINT16* pusEyesX, UINT16* pusEyesY, UINT16* pusMouthX, UINT16* pusMouthY)
{
	UINT16						usMercProfileID;
	UINT16						usEyesX;
	UINT16						usEyesY;
	UINT16						usMouthX;
	UINT16						usMouthY;

	usMercProfileID = pFace->ubCharacterNum;

	//Take eyes x,y from profile unless we are an RPC and we are small faced.....
	usEyesX				= gMercProfiles[ usMercProfileID ].usEyesX;
	usEyesY				= gMercProfiles[ usMercProfileID ].usEyesY;
	usMouthY			=	gMercProfiles[ usMercProfileID ].usMouthY;
	usMouthX			= gMercProfiles[ usMercProfileID ].usMouthX;

	// Use some other values for x,y, base on if we are a RPC!
	if ( !( pFace->uiFlags & FACE_BIGFACE ) ||( pFace->uiFlags & FACE_FORCE_SMALL ))
	{
		// Are we a recruited merc? .. or small?
		if( ( gMercProfiles[ usMercProfileID ].ubMiscFlags & ( PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE ) ) ||( pFace->uiFlags & FACE_FORCE_SMALL ) )
		{
			// Loop through all values of availible profiles to find ours!
			for (const RPC_SMALL_FACE_VALUES* i = gRPCSmallFaceValues; i != endof(gRPCSmallFaceValues); ++i)
			{
				if (i->profile != usMercProfileID) continue;
				usEyesX  = i->bEyesX;
				usEyesY  = i->bEyesY;
				usMouthX = i->bMouthX;
				usMouthY = i->bMouthY;
				break;
			}
		}
	}

	(*pusEyesX)		= usEyesX;
	(*pusEyesY)		= usEyesY;
	(*pusMouthX)	= usMouthX;
	(*pusMouthY)	= usMouthY;

}


static void InternalSetAutoFaceActive(SGPVSurface* display, SGPVSurface* restore, INT32 iFaceIndex, UINT16 usFaceX, UINT16 usFaceY, UINT16 usEyesX, UINT16 usEyesY, UINT16 usMouthX, UINT16 usMouthY);


void SetAutoFaceActive(SGPVSurface* const display, SGPVSurface* const restore, const INT32 iFaceIndex, const UINT16 usFaceX, const UINT16 usFaceY)
{
	UINT16						usEyesX;
	UINT16						usEyesY;
	UINT16						usMouthX;
	UINT16						usMouthY;
	FACETYPE					*pFace;

	// Check face index
	CHECKV( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	GetFaceRelativeCoordinates( pFace, &usEyesX, &usEyesY, &usMouthX, &usMouthY );

	InternalSetAutoFaceActive(display, restore, iFaceIndex, usFaceX, usFaceY, usEyesX, usEyesY, usMouthX, usMouthY);
}


static void InternalSetAutoFaceActive(SGPVSurface* const display, SGPVSurface* const restore, const INT32 iFaceIndex, const UINT16 usFaceX, const UINT16 usFaceY, const UINT16 usEyesX, const UINT16 usEyesY, const UINT16 usMouthX, const UINT16 usMouthY)
{
	FACETYPE					*pFace;

	// Check face index
	CHECKV( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// IF we are already being contained elsewhere, return without doing anything!

	// ATE: Don't allow another activity from setting active....
	if ( pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE )
	{
		return;
	}

	// Check if we are active already, remove if so!
	if ( pFace->fDisabled )
	{
		SetAutoFaceInActive( iFaceIndex );
	}

	if (restore == FACE_AUTO_RESTORE_BUFFER)
	{
		pFace->fAutoRestoreBuffer  = TRUE;
		pFace->uiAutoRestoreBuffer = AddVideoSurface(pFace->usFaceWidth, pFace->usFaceHeight, PIXEL_DEPTH);
		CHECKV(pFace->uiAutoRestoreBuffer != NO_VSURFACE);
	}
	else
	{
		pFace->fAutoRestoreBuffer  = FALSE;
		pFace->uiAutoRestoreBuffer = restore;
	}

	if (display == FACE_AUTO_DISPLAY_BUFFER)
	{
		pFace->fAutoDisplayBuffer  = TRUE;
		pFace->uiAutoDisplayBuffer = AddVideoSurface(pFace->usFaceWidth, pFace->usFaceHeight, PIXEL_DEPTH);
		CHECKV(pFace->uiAutoDisplayBuffer != NO_VSURFACE);
	}
	else
	{
		pFace->fAutoDisplayBuffer  = FALSE;
		pFace->uiAutoDisplayBuffer = display;
	}


	pFace->usFaceX				= usFaceX;
	pFace->usFaceY				= usFaceY;
	pFace->fCanHandleInactiveNow = FALSE;


	//Take eyes x,y from profile unless we are an RPC and we are small faced.....
	pFace->usEyesX				= usEyesX + usFaceX;
	pFace->usEyesY				= usEyesY + usFaceY;
	pFace->usMouthY				=	usMouthY + usFaceY;
	pFace->usMouthX				= usMouthX + usFaceX;

	// Save offset values
	pFace->usEyesOffsetX				= usEyesX;
	pFace->usEyesOffsetY				= usEyesY;
	pFace->usMouthOffsetY				=	usMouthY;
	pFace->usMouthOffsetX				= usMouthX;


	if ( pFace->usEyesY == usFaceY || pFace->usMouthY == usFaceY )
	{
		pFace->fInvalidAnim = TRUE;
	}

	pFace->fDisabled			=	FALSE;
	pFace->uiLastBlink			=		GetJA2Clock();
	pFace->uiLastExpression	=		GetJA2Clock();
	pFace->uiEyelast				=		GetJA2Clock();

	// Are we a soldier?
	if (pFace->soldier != NULL)
	{
		pFace->bOldSoldierLife = pFace->soldier->bLife;
	}
}


void SetAutoFaceInActiveFromSoldier(const SOLDIERTYPE* const s)
{
	CHECKV(s != NULL);
	SetAutoFaceInActive(s->iFaceIndex);
}


void SetAutoFaceInActive(INT32 iFaceIndex )
{
	FACETYPE				*pFace;

	// Check face index
	CHECKV( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// Check for a valid slot!
	CHECKV(pFace->fAllocated);


	// Turn off some flags
	if ( pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE )
	{
		if ( !pFace->fCanHandleInactiveNow )
		{
			return;
		}
	}

	if ( pFace->uiFlags & FACE_MAKEACTIVE_ONCE_DONE )
	{
		const SOLDIERTYPE* const pSoldier = pFace->soldier;
		if (pSoldier != NULL)
		{
			// IF we are in tactical
			if ( pSoldier->bAssignment == iCurrentTacticalSquad && guiCurrentScreen == GAME_SCREEN )
			{
				// Make the interfac panel dirty..
				// This will dirty the panel next frame...
				gfRerenderInterfaceFromHelpText = TRUE;
			}

		}

	}

	if ( pFace->fAutoRestoreBuffer )
	{
		DeleteVideoSurfaceFromIndex( pFace->uiAutoRestoreBuffer );
	}

	if ( pFace->fAutoDisplayBuffer )
	{
		DeleteVideoSurfaceFromIndex( pFace->uiAutoDisplayBuffer );
	}

	if (pFace->video_overlay != NULL)
	{
		RemoveVideoOverlay(pFace->video_overlay);
		pFace->video_overlay = NULL;
	}

	// Turn off some flags
	pFace->uiFlags &= ( ~FACE_INACTIVE_HANDLED_ELSEWHERE );

	// Disable!
	pFace->fDisabled = TRUE;

}


void SetAllAutoFacesInactive(  )
{
	UINT32 uiCount;

	for ( uiCount = 0; uiCount < guiNumFaces; uiCount++ )
	{
		if ( gFacesData[ uiCount ].fAllocated )
		{
			SetAutoFaceInActive( uiCount );
		}
	}
}


static void NewEye(FACETYPE* pFace);
static void HandleRenderFaceAdjustments(FACETYPE* pFace, BOOLEAN fDisplayBuffer, SGPVSurface* buffer, INT16 sFaceX, INT16 sFaceY, UINT16 usEyesX, UINT16 usEyesY);
static BOOLEAN FaceRestoreSavedBackgroundRect(INT32 iFaceIndex, INT16 sDestLeft, INT16 sDestTop, INT16 sSrcLeft, INT16 sSrcTop, INT16 sWidth, INT16 sHeight);


static void BlinkAutoFace(INT32 iFaceIndex)
{
	FACETYPE				*pFace;
	INT16						sFrame;

	if ( gFacesData[ iFaceIndex ].fAllocated && !gFacesData[ iFaceIndex ].fDisabled && !gFacesData[ iFaceIndex ].fInvalidAnim )
	{
		pFace = &gFacesData[ iFaceIndex ];

		// CHECK IF BUDDY IS DEAD, UNCONSCIOUS, ASLEEP, OR POW!
		const SOLDIERTYPE* const s = pFace->soldier;
		if (s != NULL)
		{
			if (s->bLife < OKLIFE ||
					s->fMercAsleep == TRUE ||
					s->bAssignment == ASSIGNMENT_POW)
			{
				return;
			}
		}

		if ( pFace->ubExpression == NO_EXPRESSION )
		{
			// Get Delay time, if the first frame, use a different delay
			if ( ( GetJA2Clock() - pFace->uiLastBlink ) > pFace->uiBlinkFrequency )
			{
				pFace->uiLastBlink = GetJA2Clock();
				pFace->ubExpression = BLINKING;
				pFace->uiEyelast = GetJA2Clock();
			}

			if ( pFace->fAnimatingTalking )
			{
				if ( ( GetJA2Clock() - pFace->uiLastExpression ) > pFace->uiExpressionFrequency )
				{
					pFace->uiLastExpression = GetJA2Clock();

					if ( Random( 2 ) == 0 )
					{
						pFace->ubExpression = ANGRY;
					}
					else
					{
						pFace->ubExpression = SURPRISED;
					}
				}

			}

		}

		if ( pFace->ubExpression != NO_EXPRESSION )
		{
			// Are we going to blink?
			if (GetJA2Clock() - pFace->uiEyelast > pFace->uiEyeDelay)
			{
				pFace->uiEyelast = GetJA2Clock();

				// Adjust
				NewEye( pFace );

				sFrame = pFace->sEyeFrame;

				if ( sFrame >= 5 )
				{
					sFrame = 4;
				}

				if ( sFrame > 0 )
				{
					// Blit Accordingly!
					BltVideoObject(pFace->uiAutoDisplayBuffer, pFace->uiVideoObject, sFrame, pFace->usEyesX, pFace->usEyesY);

					if ( pFace->uiAutoDisplayBuffer == FRAME_BUFFER )
					{
						InvalidateRegion( pFace->usEyesX, pFace->usEyesY, pFace->usEyesX + pFace->usEyesWidth, pFace->usEyesY + pFace->usEyesHeight );
					}
				}
				else
				{
					//RenderFace( uiDestBuffer , uiCount );
					pFace->ubExpression = NO_EXPRESSION;
					// Update rects just for eyes

					if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
					{
						FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usEyesX, pFace->usEyesY, pFace->usEyesX, pFace->usEyesY, pFace->usEyesWidth, pFace->usEyesHeight );
					}
					else
					{
						FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usEyesX, pFace->usEyesY, pFace->usEyesOffsetX, pFace->usEyesOffsetY, pFace->usEyesWidth, pFace->usEyesHeight );
					}

				}

				HandleRenderFaceAdjustments(pFace, TRUE, NO_VSURFACE, pFace->usFaceX, pFace->usFaceY, pFace->usEyesX, pFace->usEyesY);
			}
		}

	}

}


static void HandleFaceHilights(FACETYPE* const pFace, SGPVSurface* const uiBuffer, const INT16 sFaceX, const INT16 sFaceY)
{
	UINT32					uiDestPitchBYTES;
	UINT8						*pDestBuf;
	UINT16					usLineColor;
	INT32						iFaceIndex;

	iFaceIndex = pFace->iID;

  if ( !gFacesData[ iFaceIndex ].fDisabled )
  {
		if ( pFace->uiAutoDisplayBuffer == FRAME_BUFFER && guiCurrentScreen == GAME_SCREEN )
    {
	    // If we are highlighted, do this now!
	    if ( ( pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT ) )
	    {
		    // Lock buffer
		    pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
		    SetClippingRegionAndImageWidth( uiDestPitchBYTES, sFaceX-2, sFaceY-1, sFaceX + pFace->usFaceWidth + 4, sFaceY + pFace->usFaceHeight + 4 );

		    usLineColor = Get16BPPColor( FROMRGB( 255, 255, 255 ) );
		    RectangleDraw( TRUE, (sFaceX - 2 ), (sFaceY - 1),sFaceX + pFace->usFaceWidth + 1, sFaceY + pFace->usFaceHeight , usLineColor, pDestBuf );

		    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		    UnLockVideoSurface( uiBuffer );
	    }
	    else if ( ( pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT  ) )
	    {
		    const SOLDIERTYPE* const s = pFace->soldier;
		    if (s != NULL)
		    {
			    if (s->bLife >= OKLIFE)
			    {
				    // Lock buffer
				    pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
				    SetClippingRegionAndImageWidth( uiDestPitchBYTES, sFaceX-2, sFaceY-1, sFaceX + pFace->usFaceWidth + 4, sFaceY + pFace->usFaceHeight + 4 );

				    if (s->bStealthMode)
				    {
					    usLineColor = Get16BPPColor( FROMRGB( 158, 158, 12 ) );
				    }
				    else
				    {
					    usLineColor = Get16BPPColor( FROMRGB( 8, 12, 118 ) );
 				    }
    		    RectangleDraw( TRUE, (sFaceX - 2 ), (sFaceY - 1),sFaceX + pFace->usFaceWidth + 1, sFaceY + pFace->usFaceHeight , usLineColor, pDestBuf );

				    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				    UnLockVideoSurface( uiBuffer );
			    }
		    }
	    }
      else
      {
        // ATE: Zero out any highlight boxzes....
		    // Lock buffer
		    pDestBuf = LockVideoSurface( pFace->uiAutoDisplayBuffer, &uiDestPitchBYTES );
		    SetClippingRegionAndImageWidth( uiDestPitchBYTES, pFace->usFaceX-2, pFace->usFaceY-1, pFace->usFaceX + pFace->usFaceWidth + 4, pFace->usFaceY + pFace->usFaceHeight + 4 );

		    usLineColor = Get16BPPColor( FROMRGB( 0, 0, 0 ) );
		    RectangleDraw( TRUE, (pFace->usFaceX - 2 ), (pFace->usFaceY - 1), pFace->usFaceX + pFace->usFaceWidth + 1, pFace->usFaceY + pFace->usFaceHeight , usLineColor, pDestBuf );

		    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		    UnLockVideoSurface( pFace->uiAutoDisplayBuffer );
      }
    }
  }


	if ( ( pFace->fCompatibleItems && !gFacesData[ iFaceIndex ].fDisabled ) )
	{
		// Lock buffer
		pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
		SetClippingRegionAndImageWidth( uiDestPitchBYTES, sFaceX-2, sFaceY-1, sFaceX + pFace->usFaceWidth+ 4, sFaceY + pFace->usFaceHeight + 4 );

		usLineColor = Get16BPPColor( FROMRGB( 255, 0, 0 ) );
		RectangleDraw( TRUE, (sFaceX - 2), (sFaceY - 1), sFaceX + pFace->usFaceWidth + 1, sFaceY + pFace->usFaceHeight , usLineColor, pDestBuf );

		SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		UnLockVideoSurface( uiBuffer );
	}

}


static void NewMouth(FACETYPE* pFace);


static void MouthAutoFace(INT32 iFaceIndex)
{
	FACETYPE				*pFace;
	INT16						sFrame;

	if ( gFacesData[ iFaceIndex ].fAllocated  )
	{
		pFace = &gFacesData[ iFaceIndex ];

		if ( pFace->fTalking )
		{
			if ( !gFacesData[ iFaceIndex ].fDisabled && !gFacesData[ iFaceIndex ].fInvalidAnim )
			{
				if ( pFace->fAnimatingTalking )
				{
					// Check if we have an audio gap
					if (PollAudioGap(pFace->uiSoundID, &pFace->GapList))
					{
						pFace->sMouthFrame = 0;

						if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
						{
							FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight );
						}
						else
						{
							FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usMouthX, pFace->usMouthY, pFace->usMouthOffsetX, pFace->usMouthOffsetY, pFace->usMouthWidth, pFace->usMouthHeight );
						}

					}
					else
					{
						// Get Delay time
						if ( ( GetJA2Clock() - pFace->uiMouthlast ) > pFace->uiMouthDelay )
						{
							pFace->uiMouthlast = GetJA2Clock();

							// Adjust
							NewMouth( pFace );

							sFrame = pFace->sMouthFrame;

							if ( sFrame > 0 )
							{
								// Blit Accordingly!
								BltVideoObject(pFace->uiAutoDisplayBuffer, pFace->uiVideoObject, sFrame + 4, pFace->usMouthX, pFace->usMouthY);

								// Update rects
								if ( pFace->uiAutoDisplayBuffer == FRAME_BUFFER )
								{
									InvalidateRegion( pFace->usMouthX, pFace->usMouthY, pFace->usMouthX + pFace->usMouthWidth, pFace->usMouthY + pFace->usMouthHeight );
								}
							}
							else
							{
								//RenderFace( uiDestBuffer , uiCount );
								//pFace->fTaking = FALSE;
								// Update rects just for Mouth
								if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
								{
									FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight );
								}
								else
								{
									FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usMouthX, pFace->usMouthY, pFace->usMouthOffsetX, pFace->usMouthOffsetY, pFace->usMouthWidth, pFace->usMouthHeight );
								}

							}

							HandleRenderFaceAdjustments(pFace, TRUE, NO_VSURFACE, pFace->usFaceX, pFace->usFaceY, pFace->usEyesX, pFace->usEyesY);
						}
					}
				}
			}
		}

		if  ( !( pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE ) )
		{
			HandleFaceHilights( pFace, pFace->uiAutoDisplayBuffer, pFace->usFaceX, pFace->usFaceY );
		}
	}
}


static void SetupFinalTalkingDelay(FACETYPE* pFace);


static void HandleTalkingAutoFace(INT32 iFaceIndex)
{
	FACETYPE				*pFace;

	if ( gFacesData[ iFaceIndex ].fAllocated  )
	{
		pFace = &gFacesData[ iFaceIndex ];

		if ( pFace->fTalking )
		{
			// Check if we are done!	( Check this first! )
			if ( pFace->fValidSpeech )
			{
				// Check if we have finished, set some flags for the final delay down if so!
				if ( !SoundIsPlaying( pFace->uiSoundID ) && !pFace->fFinishTalking )
				{
					SetupFinalTalkingDelay( pFace );
				}
			}
			else
			{
				// Check if our delay is over
				if ( !pFace->fFinishTalking )
				{
					if ( ( GetJA2Clock() - pFace->uiTalkingTimer ) > pFace->uiTalkingDuration )
					{
						// If here, setup for last delay!
						SetupFinalTalkingDelay( pFace );

					}
				}
			}

			// Now check for end of talking
			if ( pFace->fFinishTalking )
			{
					if ( ( GetJA2Clock() - pFace->uiTalkingTimer ) > pFace->uiTalkingDuration )
					{
						pFace->fTalking    = FALSE;
						pFace->fAnimatingTalking = FALSE;

						// Remove gap info
						AudioGapListDone( &(pFace->GapList) );

						// Call dialogue handler function
						HandleDialogueEnd( pFace );
					}
			}
		}
	}
}


// Local function - uses these variables because they have already been validated
static void SetFaceShade(FACETYPE* pFace, BOOLEAN fExternBlit)
{
	const SOLDIERTYPE* const s = pFace->soldier;
	if (s == NULL) return;

	// Set to default
	SetObjectShade(pFace->uiVideoObject, FLASH_PORTRAIT_NOSHADE);

	if (pFace->video_overlay == NULL && !fExternBlit)
	{
		if (s->bActionPoints == 0 && !(gTacticalStatus.uiFlags & REALTIME) && gTacticalStatus.uiFlags & INCOMBAT)
		{
			SetObjectShade(pFace->uiVideoObject, FLASH_PORTRAIT_LITESHADE);
		}
	}

	if (s->bLife < OKLIFE)
	{
		SetObjectShade(pFace->uiVideoObject, FLASH_PORTRAIT_DARKSHADE);
	}

	// ATE: Don't shade for damage if blitting extern face...
	if ( !fExternBlit )
	{
		if (s->fFlashPortrait == FLASH_PORTRAIT_START)
		{
			SetObjectShade(pFace->uiVideoObject, s->bFlashPortraitFrame);
		}
	}
}


BOOLEAN RenderAutoFaceFromSoldier(const SOLDIERTYPE* s)
{
	// Check for valid soldier
	CHECKF(s != NULL);
	return RenderAutoFace(s->iFaceIndex);
}


static void GetXYForIconPlacement(const FACETYPE* pFace, UINT16 ubIndex, INT16 sFaceX, INT16 sFaceY, INT16* psX, INT16* psY)
{
	// Get height, width of icon...
	const ETRLEObject* pTrav = GetVideoObjectETRLESubregionProperties(guiPORTRAITICONS, ubIndex);
	UINT16 usHeight = pTrav->usHeight;
	UINT16 usWidth  = pTrav->usWidth;

	INT16 sX = sFaceX + pFace->usFaceWidth  - usWidth  - 1;
	INT16 sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}


static void GetXYForRightIconPlacement(const FACETYPE* pFace, UINT16 ubIndex, INT16 sFaceX, INT16 sFaceY, INT16* psX, INT16* psY, INT8 bNumIcons)
{
	// Get height, width of icon...
	const ETRLEObject* pTrav = GetVideoObjectETRLESubregionProperties(guiPORTRAITICONS, ubIndex);
	UINT16 usHeight = pTrav->usHeight;
	UINT16 usWidth  = pTrav->usWidth;

	INT16 sX = sFaceX + usWidth * bNumIcons + 1;
	INT16 sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}


static void DoRightIcon(SGPVSurface* const dst, FACETYPE* const pFace, const INT16 sFaceX, const INT16 sFaceY, const INT8 bNumIcons, const INT8 sIconIndex)
{
	INT16						sIconX, sIconY;

	// Find X, y for placement
	GetXYForRightIconPlacement( pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY, bNumIcons );
	BltVideoObject(dst, guiPORTRAITICONS, sIconIndex, sIconX, sIconY);
}


static void HandleRenderFaceAdjustments(FACETYPE* const pFace, const BOOLEAN fDisplayBuffer, SGPVSurface* const buffer, const INT16 sFaceX, const INT16 sFaceY, const UINT16 usEyesX, const UINT16 usEyesY)
{
	INT16						sIconX, sIconY;
	INT16						sIconIndex=-1;
	BOOLEAN					fDoIcon = FALSE;
	INT16						sPtsAvailable = 0;
	UINT16 					usMaximumPts = 0;
	CHAR16					sString[ 32 ];
	UINT16					usTextWidth;
	BOOLEAN					fAtGunRange = FALSE;
	BOOLEAN					fShowNumber = FALSE;
	INT16						sFontX, sFontY;
	INT16						sX1, sY1, sY2, sX2;
	UINT32					uiDestPitchBYTES;
	UINT8						*pDestBuf;
	UINT16					usLineColor;
  INT8            bNumRightIcons = 0;

	// If we are using an extern buffer...
	SGPVSurface* uiRenderBuffer = buffer;
	if (uiRenderBuffer == NO_VSURFACE)
	{
		if ( fDisplayBuffer )
		{
			uiRenderBuffer = pFace->uiAutoDisplayBuffer;
		}
		else
		{
			uiRenderBuffer = pFace->uiAutoRestoreBuffer;
		}
	}

	// BLIT HATCH
	SOLDIERTYPE* const s = pFace->soldier;
	if (s != NULL)
	{
		if (s->bLife < CONSCIOUSNESS || s->fDeadPanel)
		{
			// Blit Closed eyes here!
			BltVideoObject(uiRenderBuffer, pFace->uiVideoObject, 1, usEyesX, usEyesY);

			// Blit hatch!
			BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
		}

		if (s->fMercAsleep == TRUE)
		{
			// blit eyes closed
			BltVideoObject(uiRenderBuffer, pFace->uiVideoObject, 1, usEyesX, usEyesY);
		}

		if (s->uiStatusFlags & SOLDIER_DEAD)
		{
			// IF we are in the process of doing any deal/close animations, show face, not skill...
			if (!s->fClosePanel && !s->fDeadPanel && !s->fUIdeadMerc && !s->fUICloseMerc)
			{
				// Put close panel there
				BltVideoObject(uiRenderBuffer, guiDEAD, 5, sFaceX, sFaceY);

				// Blit hatch!
				BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
			}
		}

    // ATE: If talking in popup, don't do the other things.....
    if ( pFace->fTalking && gTacticalStatus.uiFlags & IN_ENDGAME_SEQUENCE )
    {
      return;
    }

		// ATE: Only do this, because we can be talking during an interrupt....
		if (pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE && buffer == NO_VSURFACE)
		{
			// Don't do this if we are being handled elsewhere and it's not an extern buffer...
		}
		else
		{
			HandleFaceHilights( pFace, uiRenderBuffer, sFaceX, sFaceY );

#ifdef JA2BETAVERSION
			if (s->bOppCnt != 0)
#else
			if (s->bOppCnt > 0)
#endif
			{
				SetFontDestBuffer(uiRenderBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				swprintf(sString, lengthof(sString), L"%d", s->bOppCnt);

				SetFont( TINYFONT1 );
				SetFontForeground( FONT_DKRED );
				SetFontBackground( FONT_NEARBLACK );

				sX1 = (INT16)( sFaceX );
				sY1 = (INT16)( sFaceY );

				sX2 = sX1 + StringPixLength( sString, TINYFONT1 ) + 1;
				sY2 = sY1 + GetFontHeight( TINYFONT1 ) - 1;

				mprintf( (INT16)( sX1 + 1), (INT16)( sY1 - 1 ), sString );
				SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				// Draw box
				pDestBuf = LockVideoSurface( uiRenderBuffer, &uiDestPitchBYTES );
				SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				usLineColor = Get16BPPColor( FROMRGB( 105, 8, 9 ) );
				RectangleDraw( TRUE, sX1, sY1, sX2, sY2, usLineColor, pDestBuf );

				UnLockVideoSurface( uiRenderBuffer );

			}

			if ((s->bInSector && (gTacticalStatus.ubCurrentTeam != OUR_TEAM || !OK_INTERRUPT_MERC(s)) && !gfHiddenInterrupt) ||
					(gfSMDisableForItems && !gfInItemPickupMenu && gpSMCurrentMerc == s && gsCurInterfacePanel == SM_PANEL))
			{
				// Blit hatch!
				BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
			}

			if ( !pFace->fDisabled && !pFace->fInvalidAnim )
			{
				// Render text above here if that's what was asked for
				if ( pFace->fDisplayTextOver != FACE_NO_TEXT_OVER  )
				{
					SetFont( TINYFONT1 );
					SetFontBackground( FONT_MCOLOR_BLACK );
					SetFontForeground( FONT_MCOLOR_WHITE );

					SetFontDestBuffer(uiRenderBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

					FindFontCenterCoordinates(sFaceX, sFaceY, pFace->usFaceWidth, pFace->usFaceHeight, pFace->zDisplayText, TINYFONT1, &sFontX, &sFontY);

					if ( pFace->fDisplayTextOver == FACE_DRAW_TEXT_OVER )
					{
						gprintfinvalidate( sFontX, sFontY, pFace->zDisplayText );
						mprintf( sFontX, sFontY, pFace->zDisplayText );
					}
					else if ( pFace->fDisplayTextOver == FACE_ERASE_TEXT_OVER )
					{
						gprintfRestore( sFontX, sFontY, pFace->zDisplayText );
						pFace->fDisplayTextOver = FACE_NO_TEXT_OVER;
					}

					SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				}
			}
		}

    // Check if a robot and is not controlled....
	  if (s->uiStatusFlags & SOLDIER_ROBOT)
	  {
		  if (!CanRobotBeControlled(s))
      {
        // Not controlled robot
			  sIconIndex = 5;
			  fDoIcon		 = TRUE;
      }
    }

    if (ControllingRobot(s))
    {
      // controlling robot
			sIconIndex = 4;
			fDoIcon		 = TRUE;
    }

    // If blind...
    if (s->bBlindedCounter > 0)
    {
      DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 6 );
      bNumRightIcons++;
    }

    if (s->bDrugEffect[DRUG_TYPE_ADRENALINE])
    {
      DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 7 );
      bNumRightIcons++;
    }

	  if (GetDrunkLevel(s) != SOBER)
	  {
      DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 8 );
      bNumRightIcons++;
    }

		switch (s->bAssignment)
		{
			case DOCTOR:

				sIconIndex = 1;
				fDoIcon		 = TRUE;
				sPtsAvailable = CalculateHealingPointsForDoctor(s, &usMaximumPts, FALSE);
				fShowNumber = TRUE;

				// divide both amounts by 10 to make the displayed numbers a little more user-palatable (smaller)
				sPtsAvailable = ( sPtsAvailable + 5 ) / 10;
				usMaximumPts  = ( usMaximumPts + 5 ) / 10;
				break;

			case PATIENT:

				sIconIndex = 2;
				fDoIcon		 = TRUE;
				// show current health / maximum health
				sPtsAvailable = s->bLife;
				usMaximumPts  = s->bLifeMax;
				fShowNumber = TRUE;
				break;

			case TRAIN_SELF:
			case TRAIN_TOWN:
			case TRAIN_TEAMMATE:
			case TRAIN_BY_OTHER:
			{
				sIconIndex = 3;
				fDoIcon		 = TRUE;
				fShowNumber = TRUE;
				// there could be bonus pts for training at gun range
				if (s->sSectorX == 13 && s->sSectorY == MAP_ROW_H && s->bSectorZ == 0)
				{
					fAtGunRange = TRUE;
				}

				switch (s->bAssignment)
				{
					case( TRAIN_SELF ):
						sPtsAvailable = GetSoldierTrainingPts(s, s->bTrainStat, fAtGunRange, &usMaximumPts);
						break;
					case( TRAIN_BY_OTHER ):
						sPtsAvailable = GetSoldierStudentPts(s, s->bTrainStat, fAtGunRange, &usMaximumPts);
						break;
					case( TRAIN_TOWN ):
						sPtsAvailable = GetTownTrainPtsForCharacter(s, &usMaximumPts );
						// divide both amounts by 10 to make the displayed numbers a little more user-palatable (smaller)
						sPtsAvailable = ( sPtsAvailable + 5 ) / 10;
						usMaximumPts  = ( usMaximumPts + 5 ) / 10;
						break;
					case( TRAIN_TEAMMATE ):
						sPtsAvailable = GetBonusTrainingPtsDueToInstructor(s, NULL , s->bTrainStat, fAtGunRange, &usMaximumPts );
						break;
				}
				break;
			}

			case REPAIR:

				sIconIndex = 0;
				fDoIcon		 = TRUE;
				sPtsAvailable = CalculateRepairPointsForRepairman(s, &usMaximumPts, FALSE);
				fShowNumber = TRUE;

				// check if we are repairing a vehicle
				if (s->bVehicleUnderRepairID != -1)
				{
					// reduce to a multiple of VEHICLE_REPAIR_POINTS_DIVISOR.  This way skill too low will show up as 0 repair pts.
					sPtsAvailable -= ( sPtsAvailable % VEHICLE_REPAIR_POINTS_DIVISOR );
					usMaximumPts  -= ( usMaximumPts  % VEHICLE_REPAIR_POINTS_DIVISOR );
				}

				break;
		}

		// Check for being serviced...
		if (s->ubServicePartner != NOBODY)
		{
			// Doctor...
			sIconIndex = 1;
			fDoIcon		 = TRUE;
		}

		if (s->ubServiceCount != 0)
		{
			// Patient
			sIconIndex = 2;
			fDoIcon		 = TRUE;
		}


		if ( fDoIcon )
		{
			// Find X, y for placement
			GetXYForIconPlacement( pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY );
			BltVideoObject(uiRenderBuffer, guiPORTRAITICONS, sIconIndex, sIconX, sIconY);

      // ATE: Show numbers only in mapscreen
			if( fShowNumber )
			{
				SetFontDestBuffer(uiRenderBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				swprintf(sString, lengthof(sString), L"%d/%d", sPtsAvailable, usMaximumPts);

				usTextWidth = StringPixLength( sString, FONT10ARIAL );
				usTextWidth += 1;

				SetFont( FONT10ARIAL );
				SetFontForeground( FONT_YELLOW );
				SetFontBackground( FONT_BLACK );

				mprintf(  sFaceX + pFace->usFaceWidth - usTextWidth, ( INT16 )( sFaceY + 3 ), sString );
				SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			}
		}
	}
  else
  {
		if ( pFace->ubCharacterNum == FATHER || pFace->ubCharacterNum == MICKY )
    {
      if ( gMercProfiles[ pFace->ubCharacterNum ].bNPCData >= 5 )
      {
        DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, 0, 8 );
      }
    }
  }
}


BOOLEAN RenderAutoFace( INT32 iFaceIndex )
{
	FACETYPE				*pFace;

	// Check face index
	CHECKF( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// Check for a valid slot!
	CHECKF(pFace->fAllocated);

	// Check for disabled guy!
	CHECKF( pFace->fDisabled != TRUE );

	SetFaceShade(pFace, FALSE);

	// Blit face to save buffer!
	if (pFace->uiAutoRestoreBuffer == guiSAVEBUFFER)
	{
		BltVideoObject(pFace->uiAutoRestoreBuffer, pFace->uiVideoObject, 0, pFace->usFaceX, pFace->usFaceY);
	}
	else
	{
		BltVideoObject(pFace->uiAutoRestoreBuffer, pFace->uiVideoObject, 0, 0, 0);
	}

	HandleRenderFaceAdjustments(pFace, FALSE, NO_VSURFACE, pFace->usFaceX, pFace->usFaceY, pFace->usEyesX, pFace->usEyesY);

	// Restore extern rect
	if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
	{
		FaceRestoreSavedBackgroundRect( iFaceIndex, (INT16)( pFace->usFaceX ), (INT16)( pFace->usFaceY ), (INT16)( pFace->usFaceX ), (INT16)( pFace->usFaceY ), ( INT16)( pFace->usFaceWidth ), (INT16)( pFace->usFaceHeight ) );
	}
	else
	{
		FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usFaceX, pFace->usFaceY, 0, 0, pFace->usFaceWidth, pFace->usFaceHeight );
	}

	return( TRUE );
}


static BOOLEAN ExternRenderFace(SGPVSurface* buffer, INT32 iFaceIndex, INT16 sX, INT16 sY);


BOOLEAN ExternRenderFaceFromSoldier(SGPVSurface* const buffer, const SOLDIERTYPE* s, const INT16 sX, const INT16 sY)
{
	// Check for valid soldier
	CHECKF(s != NULL);
	return ExternRenderFace(buffer, s->iFaceIndex, sX, sY);
}


/* To render an allocated face, but one that is independent of its active
 * status and does not require eye blinking or mouth movements, call */
static BOOLEAN ExternRenderFace(SGPVSurface* const buffer, const INT32 iFaceIndex, const INT16 sX, const INT16 sY)
{
	UINT16						usEyesX;
	UINT16						usEyesY;
	UINT16						usMouthX;
	UINT16						usMouthY;
	FACETYPE				*pFace;

	// Check face index
	CHECKF( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// Check for a valid slot!
	CHECKF(pFace->fAllocated);

	// Here, any face can be rendered, even if disabled

	SetFaceShade(pFace, TRUE);

	// Blit face to save buffer!
	BltVideoObject(buffer, pFace->uiVideoObject, 0, sX, sY);

	GetFaceRelativeCoordinates( pFace, &usEyesX, &usEyesY, &usMouthX, &usMouthY );

	HandleRenderFaceAdjustments(pFace, FALSE, buffer, sX, sY, sX + usEyesX, sY + usEyesY);

	// Restore extern rect
	if (buffer == guiSAVEBUFFER)
	{
		RestoreExternBackgroundRect( sX, sY, pFace->usFaceWidth, pFace->usFaceWidth );
	}

	return( TRUE );
}


static void NewEye(FACETYPE* pFace)
{

 switch(pFace->sEyeFrame)
 {
  case 0 : //pFace->sEyeFrame = (INT16)Random(2);	// normal - can blink or frown
			if ( pFace->ubExpression == ANGRY )
			{
				pFace->ubEyeWait = 0;
				pFace->sEyeFrame = 3;
			}
			else if ( pFace->ubExpression == SURPRISED )
			{
				pFace->ubEyeWait = 0;
				pFace->sEyeFrame = 4;
			}
			else
	   //if (pFace->sEyeFrame && Talk.talking && Talk.expression != DYING)
	   ///    pFace->sEyeFrame = 3;
	   //else
	       pFace->sEyeFrame = 1;
	   break;
  case 1 : // starting to blink  - has to finish unless dying
	   //if (Talk.expression == DYING)
	   //    pFace->sEyeFrame = 1;
	   //else
	       pFace->sEyeFrame = 2;
	   break;
  case 2 : //pFace->sEyeFrame = (INT16)Random(2);	// finishing blink - can go normal or frown
	   //if (pFace->sEyeFrame && Talk.talking)
	   //    pFace->sEyeFrame = 3;
	   //else
	   //   if (Talk.expression == ANGRY)
		 // pFace->sEyeFrame = 3;
	   //   else
		  pFace->sEyeFrame = 0;
	   break;

  case 3 : //pFace->sEyeFrame = 4; break;	// frown

			pFace->ubEyeWait++;

			if ( pFace->ubEyeWait > 6 )
			{
				pFace->sEyeFrame = 0;
			}
			break;

  case 4 :

			pFace->ubEyeWait++;

			if ( pFace->ubEyeWait > 6 )
			{
				pFace->sEyeFrame = 0;
			}
			break;

  case 5 : pFace->sEyeFrame = 6;

		pFace->sEyeFrame = 0;
	   break;

  case 6 : pFace->sEyeFrame = 7; break;
  case 7 : pFace->sEyeFrame = (INT16)Random(2);	// can stop frowning or continue
	   //if (pFace->sEyeFrame && Talk.expression != DYING)
	    //   pFace->sEyeFrame = 8;
	   //else
	   //    pFace->sEyeFrame = 0;
	   //break;
  case 8 : pFace->sEyeFrame =  9; break;
  case 9 : pFace->sEyeFrame = 10; break;
  case 10: pFace->sEyeFrame = 11; break;
  case 11: pFace->sEyeFrame = 12; break;
  case 12: pFace->sEyeFrame =  0; break;
 }
}


static void NewMouth(FACETYPE* pFace)
{
	const UINT16 old_frame = pFace->sMouthFrame;
	UINT16       new_frame;
	do
	{
		new_frame = Random(6);
		if (new_frame > 3) new_frame = 0;
	}
	while (new_frame == old_frame);
	pFace->sMouthFrame = new_frame;
}


void HandleAutoFaces( )
{
	UINT32 uiCount;
	FACETYPE				*pFace;
	INT8	bLife;
	INT8	bAPs;
	BOOLEAN	fRerender = FALSE;
	BOOLEAN	fHandleFace;

	for ( uiCount = 0; uiCount < guiNumFaces; uiCount++ )
	{
		fRerender = FALSE;
		fHandleFace = TRUE;

		// OK, NOW, check if our bLife status has changed, re-render if so!
		if ( gFacesData[ uiCount ].fAllocated  )
		{
			pFace = &gFacesData[ uiCount ];

			// Are we a soldier?
			SOLDIERTYPE* const pSoldier = pFace->soldier;
			if (pSoldier != NULL)
			{
				 // Get Life now
				 bLife		 = pSoldier->bLife;
				 bAPs      = pSoldier->bActionPoints;

				if (pSoldier == gSelectedGuy)
				 {
					 pFace->uiFlags |= FACE_SHOW_WHITE_HILIGHT;
				 }
				 else
				 {
					 pFace->uiFlags &= ( ~FACE_SHOW_WHITE_HILIGHT );
				 }

				 if ( pSoldier->sGridNo != pSoldier->sFinalDestination && pSoldier->sGridNo != NOWHERE )
				 {
					 pFace->uiFlags |= FACE_SHOW_MOVING_HILIGHT;
				 }
				 else
				 {
					 pFace->uiFlags &= ( ~FACE_SHOW_MOVING_HILIGHT );
				 }

				 if ( pSoldier->bStealthMode != pFace->bOldStealthMode )
				 {
					 fRerender = TRUE;
				 }

				 // Check if we have fallen below OKLIFE...
				 if ( bLife < OKLIFE && pFace->bOldSoldierLife >= OKLIFE )
				 {
					 fRerender = TRUE;
				 }

				 if ( bLife >= OKLIFE && pFace->bOldSoldierLife < OKLIFE )
				 {
					 fRerender = TRUE;
				 }

				 // Check if we have fallen below CONSCIOUSNESS
				 if ( bLife < CONSCIOUSNESS && pFace->bOldSoldierLife >= CONSCIOUSNESS )
				 {
					 fRerender = TRUE;
				 }

				 if ( bLife >= CONSCIOUSNESS && pFace->bOldSoldierLife < CONSCIOUSNESS )
				 {
					 fRerender = TRUE;
				 }

				 if ( pSoldier->bOppCnt != pFace->bOldOppCnt )
				 {
					 fRerender = TRUE;
				 }

				 // Check if assignment is idfferent....
				 if ( pSoldier->bAssignment != pFace->bOldAssignment )
				 {
						pFace->bOldAssignment = pSoldier->bAssignment;
						fRerender = TRUE;
				 }

				 // Check if we have fallen below CONSCIOUSNESS
				 if ( bAPs == 0 && pFace->bOldActionPoints > 0 )
				 {
					 fRerender = TRUE;
				 }

				 if ( bAPs > 0  && pFace->bOldActionPoints == 0 )
				 {
					 fRerender = TRUE;
				 }

				 if ( !( pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT ) && pFace->fOldShowHighlight )
				 {
					 fRerender = TRUE;
				 }

				 if ( ( pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT ) && !( pFace->fOldShowHighlight ) )
				 {
					 fRerender = TRUE;
				 }

				 if ( !( pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT ) && pFace->fOldShowMoveHilight )
				 {
					 fRerender = TRUE;
				 }

				 if ( ( pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT ) && !( pFace->fOldShowMoveHilight ) )
				 {
					 fRerender = TRUE;
				 }

				 if ( pFace->ubOldServiceCount != pSoldier->ubServiceCount )
				 {
					 fRerender = TRUE;
					 pFace->ubOldServiceCount = pSoldier->ubServiceCount;
				 }

				 if ( pFace->fOldCompatibleItems != pFace->fCompatibleItems || gfInItemPickupMenu || gpItemPointer != NULL )
				 {
					 fRerender = TRUE;
					 pFace->fOldCompatibleItems = pFace->fCompatibleItems;
				 }


				 if ( pFace->ubOldServicePartner != pSoldier->ubServicePartner )
				 {
					 fRerender = TRUE;
					 pFace->ubOldServicePartner = pSoldier->ubServicePartner;
				 }

				 pFace->bOldSoldierLife		= bLife;
				 pFace->bOldActionPoints	= bAPs;
				 pFace->bOldStealthMode		= pSoldier->bStealthMode;
				 pFace->bOldOppCnt				= pSoldier->bOppCnt;

				 if ( pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT )
				 {
						pFace->fOldShowHighlight = TRUE;
				 }
				 else
				 {
						pFace->fOldShowHighlight = FALSE;
				 }

				 if ( pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT )
				 {
						pFace->fOldShowMoveHilight = TRUE;
				 }
				 else
				 {
						pFace->fOldShowMoveHilight = FALSE;
				 }


					if ( pSoldier->fGettingHit && pSoldier->fFlashPortrait == FLASH_PORTRAIT_STOP )
					{
						pSoldier->fFlashPortrait = TRUE;
						pSoldier->bFlashPortraitFrame = FLASH_PORTRAIT_STARTSHADE;
						RESETTIMECOUNTER( pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY );
						fRerender = TRUE;
					}
					if ( pSoldier->fFlashPortrait == FLASH_PORTRAIT_START )
					{
						// Loop through flash values
						if ( TIMECOUNTERDONE( pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY ) )
						{
							RESETTIMECOUNTER( pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY );
							pSoldier->bFlashPortraitFrame++;

							if ( pSoldier->bFlashPortraitFrame > FLASH_PORTRAIT_ENDSHADE )
							{
								pSoldier->bFlashPortraitFrame = FLASH_PORTRAIT_ENDSHADE;

								if ( pSoldier->fGettingHit )
								{
									pSoldier->fFlashPortrait = FLASH_PORTRAIT_WAITING;
								}
								else
								{
									// Render face again!
									pSoldier->fFlashPortrait = FLASH_PORTRAIT_STOP;
								}

								fRerender = TRUE;
							}
						}
					}
					// CHECK IF WE WERE WAITING FOR GETTING HIT TO FINISH!
					if ( !pSoldier->fGettingHit && pSoldier->fFlashPortrait == FLASH_PORTRAIT_WAITING )
					{
						pSoldier->fFlashPortrait = FALSE;
						fRerender = TRUE;
					}

					if ( pSoldier->fFlashPortrait == FLASH_PORTRAIT_START )
					{
						fRerender = TRUE;
					}

				 if( pFace->uiFlags & FACE_REDRAW_WHOLE_FACE_NEXT_FRAME )
				 {
					 pFace->uiFlags &= ~FACE_REDRAW_WHOLE_FACE_NEXT_FRAME;

					 fRerender = TRUE;
				 }

         if (	fInterfacePanelDirty == DIRTYLEVEL2 && guiCurrentScreen == GAME_SCREEN )
         {
					 fRerender = TRUE;
         }

				 if ( fRerender )
				 {
						RenderAutoFace( uiCount );
				 }

				 if ( bLife < CONSCIOUSNESS )
				 {
					 fHandleFace = FALSE;
				 }
			}

			if ( fHandleFace )
			{
				BlinkAutoFace( uiCount );
			}

			MouthAutoFace( uiCount );

		}

	}

}

void HandleTalkingAutoFaces( )
{
	UINT32 uiCount;

	for ( uiCount = 0; uiCount < guiNumFaces; uiCount++ )
	{
		// OK, NOW, check if our bLife status has changed, re-render if so!
		if ( gFacesData[ uiCount ].fAllocated  )
		{
			HandleTalkingAutoFace( uiCount );

		}
	}
}


static BOOLEAN FaceRestoreSavedBackgroundRect(INT32 iFaceIndex, INT16 sDestLeft, INT16 sDestTop, INT16 sSrcLeft, INT16 sSrcTop, INT16 sWidth, INT16 sHeight)
{
	FACETYPE					*pFace;

	// Check face index
	CHECKF( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	const SGPRect r = { sSrcLeft, sSrcTop, sSrcLeft + sWidth, sSrcTop + sHeight };
	BltVideoSurface(pFace->uiAutoDisplayBuffer, pFace->uiAutoRestoreBuffer, sDestLeft, sDestTop, &r);

	// Add rect to frame buffer queue
	if ( pFace->uiAutoDisplayBuffer == FRAME_BUFFER )
	{
		InvalidateRegionEx(sDestLeft - 2, sDestTop - 2, sDestLeft + sWidth + 3, sDestTop + sHeight + 2);
	}
	return(TRUE);
}


BOOLEAN SetFaceTalking(INT32 iFaceIndex, CHAR8* zSoundFile, STR16 zTextString)
{
	FACETYPE			*pFace;

	pFace = &gFacesData[ iFaceIndex ];

	// Set face to talking
	pFace->fTalking = TRUE;
	pFace->fAnimatingTalking = TRUE;
	pFace->fFinishTalking = FALSE;

  if ( !AreInMeanwhile( ) )
  {
    TurnOnSectorLocator( pFace->ubCharacterNum );
  }

	// Play sample
	if( gGameSettings.fOptions[ TOPTION_SPEECH ] )
		pFace->uiSoundID = PlayJA2GapSample(zSoundFile, HIGHVOLUME, 1, MIDDLEPAN, &pFace->GapList);
	else
		pFace->uiSoundID = SOUND_ERROR;

	if ( pFace->uiSoundID != SOUND_ERROR )
	{
		pFace->fValidSpeech	= TRUE;

		pFace->uiTalkingFromVeryBeginningTimer = GetJA2Clock( );
	}
	else
	{
		pFace->fValidSpeech	= FALSE;

		// Set delay based on sound...
		pFace->uiTalkingTimer = pFace->uiTalkingFromVeryBeginningTimer = GetJA2Clock( );

		pFace->uiTalkingDuration = FindDelayForString( zTextString );
	}

	return( TRUE );
}


BOOLEAN ExternSetFaceTalking( INT32 iFaceIndex, UINT32 uiSoundID )
{
	FACETYPE			*pFace;

	pFace = &gFacesData[ iFaceIndex ];

	// Set face to talki	ng
	pFace->fTalking = TRUE;
	pFace->fAnimatingTalking = TRUE;
	pFace->fFinishTalking = FALSE;
	pFace->fValidSpeech	= TRUE;

	pFace->uiSoundID = uiSoundID;

	return( TRUE );
}



void InternalShutupaYoFace( INT32 iFaceIndex, BOOLEAN fForce )
{
	FACETYPE			*pFace;

	// Check face index
	CHECKV( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	if ( pFace->fTalking )
	{
		// OK, only do this if we have been talking for a min. amount fo time...
		if ( ( GetJA2Clock( ) - pFace->uiTalkingFromVeryBeginningTimer ) < 500 && !fForce )
		{
			return;
		}

		if ( pFace->uiSoundID != SOUND_ERROR )
		{
			SoundStop( pFace->uiSoundID );
		}

		// Remove gap info
		AudioGapListDone( &(pFace->GapList) );

		// Shutup mouth!
		pFace->sMouthFrame = 0;

		// ATE: Only change if active!
		if ( !pFace->fDisabled )
		{
			if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
			{
				FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight );
			}
			else
			{
				FaceRestoreSavedBackgroundRect( iFaceIndex, pFace->usMouthX, pFace->usMouthY, pFace->usMouthOffsetX, pFace->usMouthOffsetY,  pFace->usMouthWidth, pFace->usMouthHeight );
			}
		}
		// OK, smart guy, make sure this guy has finished talking,
		// before attempting to end dialogue UI.
		pFace->fTalking = FALSE;

		// Call dialogue handler function
		HandleDialogueEnd( pFace );

		pFace->fTalking = FALSE;
		pFace->fAnimatingTalking = FALSE;

		gfUIWaitingForUserSpeechAdvance = FALSE;

	}

}

void ShutupaYoFace( INT32 iFaceIndex )
{
	InternalShutupaYoFace( iFaceIndex, TRUE );
}


static void SetupFinalTalkingDelay(FACETYPE* pFace)
{
	pFace->fFinishTalking = TRUE;

	pFace->fAnimatingTalking = FALSE;

	pFace->uiTalkingTimer = GetJA2Clock( );

	if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ]  )
	{
		//pFace->uiTalkingDuration = FINAL_TALKING_DURATION;
		pFace->uiTalkingDuration = 300;
	}
	else
	{
		pFace->uiTalkingDuration = 300;
	}

	pFace->sMouthFrame = 0;

	// Close mouth!
	if ( !pFace->fDisabled )
	{
		if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
		{
			FaceRestoreSavedBackgroundRect( pFace->iID, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight );
		}
		else
		{
			FaceRestoreSavedBackgroundRect( pFace->iID, pFace->usMouthX, pFace->usMouthY, pFace->usMouthOffsetX, pFace->usMouthOffsetY, pFace->usMouthWidth, pFace->usMouthHeight );
		}
	}

	// Setup flag to wait for advance ( because we have no text! )
	if ( gGameSettings.fOptions[ TOPTION_KEY_ADVANCE_SPEECH ] && ( pFace->uiFlags & FACE_POTENTIAL_KEYWAIT ) )
	{

		// Check if we have had valid speech!
		if ( !pFace->fValidSpeech || gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
		{
			// Set false!
			pFace->fFinishTalking = FALSE;
			// Set waiting for advance to true!
			gfUIWaitingForUserSpeechAdvance = TRUE;
		}
	}

	// Set final delay!
	pFace->fValidSpeech = FALSE;


}
