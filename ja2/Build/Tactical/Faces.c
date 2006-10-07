#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include "math.h"
	#include <stdio.h>
	#include <errno.h>

	#include "WorldDef.h"
	#include "RenderWorld.h"
	#include "VSurface.h"
	#include "Render_Dirty.h"
	#include "SysUtil.h"
	#include "Container.h"
	#include "WCheck.h"
	#include "Video.h"
	#include "VObject_Blitters.h"
	#include "Faces.h"
	#include "Utilities.h"
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
#endif

// Defines
#define		NUM_FACE_SLOTS					50

#define		END_FACE_OVERLAY_DELAY	2000


// GLOBAL FOR FACES LISTING
FACETYPE	gFacesData[ NUM_FACE_SLOTS ];
UINT32 guiNumFaces = 0;

// LOCAL FUNCTIONS
void NewEye( FACETYPE *pFace );
void NewMouth( FACETYPE *pFace );
INT32 GetFreeFace(void);
void RecountFaces(void);
void HandleRenderFaceAdjustments( FACETYPE *pFace, BOOLEAN fDisplayBuffer, BOOLEAN fUseExternBuffer, UINT32 uiBuffer, INT16 sFaceX, INT16 sFaceY, UINT16 usEyesX, UINT16 usEyesY );

extern BOOLEAN	gfInItemPickupMenu;



typedef struct
{
	INT8		bEyesX;
	INT8		bEyesY;
	INT8		bMouthX;
	INT8		bMouthY;

} RPC_SMALL_FACE_VALUES;


RPC_SMALL_FACE_VALUES gRPCSmallFaceValues[ ] =
{
	9,  8,  8,  24,		// MIGUEL		( 57 )
	8,  8,  7,  24,		// CARLOS		( 58 )
	10,	8,	8,	26,		// IRA			( 59 )
	 7, 8,  7,  26,		// DIMITRI	( 60 )
	 6,	7,	7,	23,		// DEVIN		( 61 )
	 0,	0,	0,	0,		// THE RAT	( 62 )
	 8,	7,	8,	23,		//					( 63 )
	 8,	8,	8,	22,		// SLAY			( 64 )
	 0,	0,	0,	0,		//					( 65 )
	 9,	4,	7,	22,		// DYNAMO		( 66 )
	 8,	8,	8,	25,		// SHANK		( 67 )
	 4,	6,	5,	22,		// IGGY			( 68 )
	 8,	9,	7,	25,		// VINCE		( 69 )
	 4,	7,	5,	25,		// CONRAD		( 70 )
	 9,	7,	8,	22,		// CARL			( 71 )
	 9,	7,	9,	25,		// MADDOG		( 72 )
	 0,	0,	0,	0,		//					( 73 )
	 0,	0,	0,	0,		//					( 74 )

	 9,	3,	8,	23,		// MARIA		( 88 )

	 9,	3,	8,	25,		// JOEY			( 90 )

	11,	7,	9,	24,		// SKYRIDER	( 97 )
	9,	5,	7,	23,		// Miner	( 106 )

	 6,	4,	6,	24,		// JOHN					( 118 )
	 12,4,	10,	24,			//					( 119 )
	8,	6,	8,	23,		// Miner	( 148 )
	6,	5,	6,	23,		// Miner	( 156 )
	13,	7,	11,	24,		// Miner	( 157 )
	9,	7,	8,	22,		// Miner	( 158 )

};

UINT8	gubRPCSmallFaceProfileNum[] =
{
	57, // entry 0
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66, // entry 9
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	88,
	90, // entry 19
	97,
	106,
	118,
	119,
	148, // entry 24
	156,
	157,
	158,

};

UINT8	ubRPCNumSmallFaceValues = 28;


extern BOOLEAN	gfSMDisableForItems;
extern INT16		gsCurInterfacePanel;
extern UINT16		gusSMCurrentMerc;
extern BOOLEAN	gfRerenderInterfaceFromHelpText;
extern	BOOLEAN	gfInItemPickupMenu;

BOOLEAN FaceRestoreSavedBackgroundRect( INT32 iFaceIndex, INT16 sDestLeft, INT16 sDestTop, INT16 sSrcLeft, INT16 sSrcTop, INT16 sWidth, INT16 sHeight );
void SetupFinalTalkingDelay( FACETYPE *pFace );


INT32 GetFreeFace(void)
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

void RecountFaces(void)
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


INT32	InitSoldierFace( SOLDIERTYPE *pSoldier )
{
	INT32							iFaceIndex;

	// Check if we have a face init already
	iFaceIndex = pSoldier->iFaceIndex;

	if ( iFaceIndex != -1 )
	{
		return( iFaceIndex );
	}

	return( InitFace( pSoldier->ubProfile, pSoldier->ubID, 0) );
}


INT32	InitFace( UINT8 usMercProfileID, UINT8 ubSoldierID, UINT32 uiInitFlags )
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

	return( InternalInitFace( usMercProfileID, ubSoldierID, uiInitFlags, gMercProfiles[ usMercProfileID ].ubFaceIndex, uiBlinkFrequency, uiExpressionFrequency ) );

}


INT32	InternalInitFace( UINT8 usMercProfileID, UINT8 ubSoldierID, UINT32 uiInitFlags, INT32 iFaceFileID, UINT32 uiBlinkFrequency, UINT32 uiExpressionFrequency )
{
	FACETYPE					*pFace;
  VOBJECT_DESC			VObjectDesc;
	UINT32						uiVideoObject;
	INT32							iFaceIndex;
	ETRLEObject				ETRLEObject;
	HVOBJECT					hVObject;
	UINT32						uiCount;
	SGPPaletteEntry		Pal[256];

	if( ( iFaceIndex = GetFreeFace() )==(-1) )
		return(-1);

	// Load face file
	VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;

	// ATE: If we are merc profile ID #151-154, all use 151's protrait....
	if ( usMercProfileID >= 151 && usMercProfileID <= 154 )
	{
		iFaceFileID = 151;
	}


	// Check if we are a big-face....
	if ( uiInitFlags & FACE_BIGFACE )
	{
		// The filename is the profile ID!
		if( iFaceFileID < 100 )
		{
			sprintf( VObjectDesc.ImageFile, "FACES\\b%02d.sti", iFaceFileID );
		}
		else
		{
			sprintf( VObjectDesc.ImageFile, "FACES\\b%03d.sti", iFaceFileID );
		}

    // ATE: Check for profile - if elliot , use special face :)
    if ( usMercProfileID == ELLIOT )
    {
      if ( gMercProfiles[ ELLIOT ].bNPCData > 3 && gMercProfiles[ ELLIOT ].bNPCData < 7 )
      {
			  sprintf( VObjectDesc.ImageFile, "FACES\\b%02da.sti", iFaceFileID );
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData > 6 && gMercProfiles[ ELLIOT ].bNPCData < 10 )
      {
			  sprintf( VObjectDesc.ImageFile, "FACES\\b%02db.sti", iFaceFileID );
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData > 9 && gMercProfiles[ ELLIOT ].bNPCData < 13 )
      {
			  sprintf( VObjectDesc.ImageFile, "FACES\\b%02dc.sti", iFaceFileID );
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData > 12 && gMercProfiles[ ELLIOT ].bNPCData < 16 )
      {
			  sprintf( VObjectDesc.ImageFile, "FACES\\b%02dd.sti", iFaceFileID );
      }
      else if ( gMercProfiles[ ELLIOT ].bNPCData == 17 )
      {
			  sprintf( VObjectDesc.ImageFile, "FACES\\b%02de.sti", iFaceFileID );
      }
    }
	}
	else
	{

		if( iFaceFileID < 100 )
		{
			// The filename is the profile ID!
			sprintf( VObjectDesc.ImageFile, "FACES\\%02d.sti", iFaceFileID );
		}
		else
		{
			sprintf( VObjectDesc.ImageFile, "FACES\\%03d.sti", iFaceFileID );
		}
	}

	// Load
	if( AddVideoObject( &VObjectDesc, &uiVideoObject ) == FALSE )
	{
		// If we are a big face, use placeholder...
		if ( uiInitFlags & FACE_BIGFACE )
		{
			sprintf( VObjectDesc.ImageFile, "FACES\\placeholder.sti" );

			if( AddVideoObject( &VObjectDesc, &uiVideoObject ) == FALSE )
			{
				return( -1 );
			}
		}
		else
		{
			return( -1 );
		}
	}

	memset(&gFacesData[ iFaceIndex ], 0, sizeof( FACETYPE ) );

	pFace = &gFacesData[ iFaceIndex ];

	// Get profile data and set into face data
	pFace->ubSoldierID		= ubSoldierID;

	pFace->iID						= iFaceIndex;
	pFace->fAllocated			= TRUE;

	//Default to off!
	pFace->fDisabled			= TRUE;
	pFace->iVideoOverlay	= -1;
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
	if( GetVideoObject( &hVObject, uiVideoObject ) )
	{
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

	}


	// Get FACE height, width
	if( GetVideoObjectETRLEPropertiesFromIndex( uiVideoObject, &ETRLEObject, 0 ) == FALSE )
	{
		return( -1 );
	}
	pFace->usFaceWidth = ETRLEObject.usWidth;
	pFace->usFaceHeight = ETRLEObject.usHeight;


	// OK, check # of items
	if ( hVObject->usNumberOfObjects == 8 )
	{
		pFace->fInvalidAnim = FALSE;

		// Get EYE height, width
		if( GetVideoObjectETRLEPropertiesFromIndex( uiVideoObject, &ETRLEObject, 1 ) == FALSE )
		{
			return( -1 );
		}
		pFace->usEyesWidth = ETRLEObject.usWidth;
		pFace->usEyesHeight = ETRLEObject.usHeight;


		// Get Mouth height, width
		if( GetVideoObjectETRLEPropertiesFromIndex( uiVideoObject, &ETRLEObject, 5 ) == FALSE )
		{
			return( -1 );
		}
		pFace->usMouthWidth = ETRLEObject.usWidth;
		pFace->usMouthHeight = ETRLEObject.usHeight;
	}
	else
	{
		pFace->fInvalidAnim = TRUE;
	}

	// Set id
	pFace->uiVideoObject = uiVideoObject;

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
	CHECKV( pFace->fAllocated != FALSE );

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

void	SetAutoFaceActiveFromSoldier( UINT32 uiDisplayBuffer, UINT32 uiRestoreBuffer, UINT8 ubSoldierID , UINT16 usFaceX, UINT16 usFaceY )
{
	if( ubSoldierID == NOBODY )
	{
		return;
	}

	SetAutoFaceActive( uiDisplayBuffer, uiRestoreBuffer, MercPtrs[ ubSoldierID ]->iFaceIndex, usFaceX, usFaceY );

}

void GetFaceRelativeCoordinates( FACETYPE *pFace, UINT16 *pusEyesX, UINT16 *pusEyesY, UINT16 *pusMouthX, UINT16 *pusMouthY )
{
	UINT16						usMercProfileID;
	UINT16						usEyesX;
	UINT16						usEyesY;
	UINT16						usMouthX;
	UINT16						usMouthY;
	INT32							cnt;

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
			// Loop through all values of availible merc IDs to find ours!
			for ( cnt = 0; cnt < ubRPCNumSmallFaceValues; cnt++ )
			{
				// We've found one!
				if ( gubRPCSmallFaceProfileNum[ cnt ] == usMercProfileID )
				{
					usEyesX				= gRPCSmallFaceValues[ cnt ].bEyesX;
					usEyesY				= gRPCSmallFaceValues[ cnt ].bEyesY;
					usMouthY			=	gRPCSmallFaceValues[ cnt ].bMouthY;
					usMouthX			= gRPCSmallFaceValues[ cnt ].bMouthX;
				}
			}

		}
	}

	(*pusEyesX)		= usEyesX;
	(*pusEyesY)		= usEyesY;
	(*pusMouthX)	= usMouthX;
	(*pusMouthY)	= usMouthY;

}


void SetAutoFaceActive( UINT32 uiDisplayBuffer, UINT32 uiRestoreBuffer, INT32 iFaceIndex , UINT16 usFaceX, UINT16 usFaceY )
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

	InternalSetAutoFaceActive( uiDisplayBuffer, uiRestoreBuffer, iFaceIndex , usFaceX, usFaceY, usEyesX,  usEyesY, usMouthX, usMouthY );

}


void InternalSetAutoFaceActive( UINT32 uiDisplayBuffer, UINT32 uiRestoreBuffer, INT32 iFaceIndex , UINT16 usFaceX, UINT16 usFaceY, UINT16 usEyesX, UINT16 usEyesY, UINT16 usMouthX, UINT16 usMouthY )
{
	UINT16						usMercProfileID;
	FACETYPE					*pFace;
	VSURFACE_DESC			vs_desc;
	UINT16						usWidth;
	UINT16						usHeight;
	UINT8							ubBitDepth;

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

	if ( uiRestoreBuffer == FACE_AUTO_RESTORE_BUFFER )
	{
		// BUILD A BUFFER
		GetCurrentVideoSettings( &usWidth, &usHeight, &ubBitDepth );
		// OK, ignore screen widths, height, only use BPP
		vs_desc.fCreateFlags = VSURFACE_CREATE_DEFAULT | VSURFACE_SYSTEM_MEM_USAGE;
		vs_desc.usWidth = pFace->usFaceWidth;
		vs_desc.usHeight = pFace->usFaceHeight;
		vs_desc.ubBitDepth = ubBitDepth;

		pFace->fAutoRestoreBuffer  = TRUE;

		CHECKV( AddVideoSurface( &vs_desc, &(pFace->uiAutoRestoreBuffer)  ) );
	}
	else
	{
		pFace->fAutoRestoreBuffer  = FALSE;
		pFace->uiAutoRestoreBuffer = uiRestoreBuffer;
	}

	if ( uiDisplayBuffer == FACE_AUTO_DISPLAY_BUFFER )
	{
		// BUILD A BUFFER
		GetCurrentVideoSettings( &usWidth, &usHeight, &ubBitDepth );
		// OK, ignore screen widths, height, only use BPP
		vs_desc.fCreateFlags = VSURFACE_CREATE_DEFAULT | VSURFACE_SYSTEM_MEM_USAGE;
		vs_desc.usWidth = pFace->usFaceWidth;
		vs_desc.usHeight = pFace->usFaceHeight;
		vs_desc.ubBitDepth = ubBitDepth;

		pFace->fAutoDisplayBuffer  = TRUE;

		CHECKV( AddVideoSurface( &vs_desc, &(pFace->uiAutoDisplayBuffer)  ) );
	}
	else
	{
		pFace->fAutoDisplayBuffer  = FALSE;
		pFace->uiAutoDisplayBuffer = uiDisplayBuffer;
	}


	usMercProfileID = pFace->ubCharacterNum;

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
	pFace->fStartFrame			=  TRUE;

	// Are we a soldier?
	if ( pFace->ubSoldierID != NOBODY )
	{
		 pFace->bOldSoldierLife = MercPtrs[ pFace->ubSoldierID ]->bLife;
	}
}


void SetAutoFaceInActiveFromSoldier( UINT8 ubSoldierID )
{
	// Check for valid soldier
	CHECKV( ubSoldierID != NOBODY );

	SetAutoFaceInActive( MercPtrs[ ubSoldierID ]->iFaceIndex );
}


void SetAutoFaceInActive(INT32 iFaceIndex )
{
	FACETYPE				*pFace;
	SOLDIERTYPE			*pSoldier;

	// Check face index
	CHECKV( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// Check for a valid slot!
	CHECKV( pFace->fAllocated != FALSE );


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
		//
		if ( pFace->ubSoldierID != NOBODY )
		{
			pSoldier = MercPtrs[ pFace->ubSoldierID ];

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

	if ( pFace->iVideoOverlay != -1 )
	{
		RemoveVideoOverlay( pFace->iVideoOverlay );
		pFace->iVideoOverlay = -1;
	}

	// Turn off some flags
	pFace->uiFlags &= ( ~FACE_INACTIVE_HANDLED_ELSEWHERE );

	// Disable!
	pFace->fDisabled = TRUE;

}


void SetAllAutoFacesInactive(  )
{
	UINT32 uiCount;
	FACETYPE				*pFace;

	for ( uiCount = 0; uiCount < guiNumFaces; uiCount++ )
	{
		if ( gFacesData[ uiCount ].fAllocated )
		{
			pFace = &gFacesData[ uiCount ];

			SetAutoFaceInActive( uiCount );
		}
	}
}



void BlinkAutoFace( INT32 iFaceIndex )
{
	FACETYPE				*pFace;
	INT16						sFrame;
	BOOLEAN					fDoBlink = FALSE;

	if ( gFacesData[ iFaceIndex ].fAllocated && !gFacesData[ iFaceIndex ].fDisabled && !gFacesData[ iFaceIndex ].fInvalidAnim )
	{
		pFace = &gFacesData[ iFaceIndex ];

		// CHECK IF BUDDY IS DEAD, UNCONSCIOUS, ASLEEP, OR POW!
		if ( pFace->ubSoldierID != NOBODY )
		{
			if ( ( MercPtrs[ pFace->ubSoldierID ]->bLife < OKLIFE ) ||
					 ( MercPtrs[ pFace->ubSoldierID ]->fMercAsleep == TRUE ) ||
					 ( MercPtrs[ pFace->ubSoldierID ]->bAssignment == ASSIGNMENT_POW ) )
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
			if ( pFace->fStartFrame )
			{
				if ( ( GetJA2Clock() - pFace->uiEyelast ) > pFace->uiEyeDelay ) //> Random( 10000 ) )
				{
					fDoBlink						= TRUE;
					pFace->fStartFrame	= FALSE;
				}
			}
			else
			{
				if ( ( GetJA2Clock() - pFace->uiEyelast ) > pFace->uiEyeDelay )
				{
					fDoBlink = TRUE;
				}
			}

			// Are we going to blink?
			if ( fDoBlink )
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
					BltVideoObjectFromIndex( pFace->uiAutoDisplayBuffer, pFace->uiVideoObject, (INT16)( sFrame ), pFace->usEyesX, pFace->usEyesY, VO_BLT_SRCTRANSPARENCY);

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

				HandleRenderFaceAdjustments( pFace, TRUE, FALSE, 0, pFace->usFaceX, pFace->usFaceY, pFace->usEyesX, pFace->usEyesY );

			}
		}

	}

}


void HandleFaceHilights( FACETYPE *pFace, UINT32 uiBuffer, INT16 sFaceX, INT16 sFaceY )
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

		    SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );

		    UnLockVideoSurface( uiBuffer );
	    }
	    else if ( ( pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT  ) )
	    {
		    if ( pFace->ubSoldierID != NOBODY )
		    {
			    if ( MercPtrs[ pFace->ubSoldierID ]->bLife >= OKLIFE )
			    {
				    // Lock buffer
				    pDestBuf = LockVideoSurface( uiBuffer, &uiDestPitchBYTES );
				    SetClippingRegionAndImageWidth( uiDestPitchBYTES, sFaceX-2, sFaceY-1, sFaceX + pFace->usFaceWidth + 4, sFaceY + pFace->usFaceHeight + 4 );

				    if ( MercPtrs[ pFace->ubSoldierID ]->bStealthMode )
				    {
					    usLineColor = Get16BPPColor( FROMRGB( 158, 158, 12 ) );
				    }
				    else
				    {
					    usLineColor = Get16BPPColor( FROMRGB( 8, 12, 118 ) );
 				    }
    		    RectangleDraw( TRUE, (sFaceX - 2 ), (sFaceY - 1),sFaceX + pFace->usFaceWidth + 1, sFaceY + pFace->usFaceHeight , usLineColor, pDestBuf );

				    SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );

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

		    SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );

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

		SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );

		UnLockVideoSurface( uiBuffer );
	}

}


void MouthAutoFace( INT32 iFaceIndex )
{
	FACETYPE				*pFace;
	INT16						sFrame;

	if ( gFacesData[ iFaceIndex ].fAllocated  )
	{
		pFace = &gFacesData[ iFaceIndex ];

		// Remove video overlay is present....
		if ( pFace->uiFlags & FACE_DESTROY_OVERLAY )
		{
			//if ( pFace->iVideoOverlay != -1 )
			//{
			//	if ( pFace->uiStopOverlayTimer != 0 )
			//	{
			//		if ( ( GetJA2Clock( ) - pFace->uiStopOverlayTimer ) > END_FACE_OVERLAY_DELAY )
			//		{
					//	RemoveVideoOverlay( pFace->iVideoOverlay );
			//			pFace->iVideoOverlay = -1;
			//		}
			//	}
			//}
		}

		if ( pFace->fTalking )
		{
			if ( !gFacesData[ iFaceIndex ].fDisabled && !gFacesData[ iFaceIndex ].fInvalidAnim )
			{
				if ( pFace->fAnimatingTalking )
				{
					PollAudioGap( pFace->uiSoundID, &(pFace->GapList ) );

					// Check if we have an audio gap
					if ( pFace->GapList.audio_gap_active )
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
								BltVideoObjectFromIndex( pFace->uiAutoDisplayBuffer, pFace->uiVideoObject, (INT16)( sFrame + 4 ), pFace->usMouthX, pFace->usMouthY, VO_BLT_SRCTRANSPARENCY);

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

							HandleRenderFaceAdjustments( pFace, TRUE, FALSE, 0, pFace->usFaceX, pFace->usFaceY, pFace->usEyesX, pFace->usEyesY );

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


void HandleTalkingAutoFace( INT32 iFaceIndex )
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

						// Remove video overlay is present....
						if ( pFace->iVideoOverlay != -1 )
						{
							//if ( pFace->uiStopOverlayTimer == 0 )
							//{
							//	pFace->uiStopOverlayTimer = GetJA2Clock();
							//}
						}

						// Call dialogue handler function
						HandleDialogueEnd( pFace );
					}
			}
		}
	}
}


// Local function - uses these variables because they have already been validated
void SetFaceShade( SOLDIERTYPE *pSoldier, FACETYPE *pFace, BOOLEAN fExternBlit )
{
	// Set to default
	SetObjectHandleShade( pFace->uiVideoObject, FLASH_PORTRAIT_NOSHADE );

	if ( pFace->iVideoOverlay == -1 && !fExternBlit )
	{
		if ( ( pSoldier->bActionPoints == 0 ) && !( gTacticalStatus.uiFlags & REALTIME ) && (gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			SetObjectHandleShade( pFace->uiVideoObject, FLASH_PORTRAIT_LITESHADE );
		}
	}

	if ( pSoldier->bLife < OKLIFE  )
	{
		SetObjectHandleShade( pFace->uiVideoObject, FLASH_PORTRAIT_DARKSHADE );
	}

	// ATE: Don't shade for damage if blitting extern face...
	if ( !fExternBlit )
	{
		if ( pSoldier->fFlashPortrait == FLASH_PORTRAIT_START )
		{
			SetObjectHandleShade( pFace->uiVideoObject, pSoldier->bFlashPortraitFrame );
		}
	}
}

BOOLEAN RenderAutoFaceFromSoldier( UINT8 ubSoldierID )
{
	// Check for valid soldier
	CHECKF( ubSoldierID != NOBODY );

	return( RenderAutoFace( MercPtrs[ ubSoldierID ]->iFaceIndex ) );
}

void GetXYForIconPlacement( FACETYPE *pFace, UINT16 ubIndex, INT16 sFaceX, INT16 sFaceY, INT16 *psX, INT16 *psY )
{
	INT16 sX, sY;
	UINT16 usWidth, usHeight;
  ETRLEObject						*pTrav;
	HVOBJECT							hVObject;


	// Get height, width of icon...
	GetVideoObject( &hVObject, guiPORTRAITICONS );
	pTrav = &(hVObject->pETRLEObject[ ubIndex ] );
	usHeight				= pTrav->usHeight;
	usWidth					= pTrav->usWidth;

	sX = sFaceX + pFace->usFaceWidth - usWidth - 1;
	sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}

void GetXYForRightIconPlacement( FACETYPE *pFace, UINT16 ubIndex, INT16 sFaceX, INT16 sFaceY, INT16 *psX, INT16 *psY, INT8 bNumIcons )
{
	INT16 sX, sY;
	UINT16 usWidth, usHeight;
  ETRLEObject						*pTrav;
	HVOBJECT							hVObject;


	// Get height, width of icon...
	GetVideoObject( &hVObject, guiPORTRAITICONS );
	pTrav = &(hVObject->pETRLEObject[ ubIndex ] );
	usHeight				= pTrav->usHeight;
	usWidth					= pTrav->usWidth;

	sX = sFaceX + ( usWidth * bNumIcons ) + 1;
	sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}



void DoRightIcon( UINT32 uiRenderBuffer, FACETYPE *pFace, INT16 sFaceX, INT16 sFaceY, INT8 bNumIcons, INT8 sIconIndex )
{
	INT16						sIconX, sIconY;

	// Find X, y for placement
	GetXYForRightIconPlacement( pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY, bNumIcons );
	BltVideoObjectFromIndex( uiRenderBuffer, guiPORTRAITICONS, sIconIndex, sIconX, sIconY, VO_BLT_SRCTRANSPARENCY);
}


void HandleRenderFaceAdjustments( FACETYPE *pFace, BOOLEAN fDisplayBuffer, BOOLEAN fUseExternBuffer, UINT32 uiBuffer, INT16 sFaceX, INT16 sFaceY, UINT16 usEyesX, UINT16 usEyesY )
{
	INT16						sIconX, sIconY;
	INT16						sIconIndex=-1;
	BOOLEAN					fDoIcon = FALSE;
	UINT32					uiRenderBuffer;
	INT16						sPtsAvailable = 0;
	UINT16 					usMaximumPts = 0;
	CHAR16					sString[ 32 ];
	UINT16					usTextWidth;
	BOOLEAN					fAtGunRange = FALSE;
	BOOLEAN					fShowNumber = FALSE;
	BOOLEAN					fShowMaximum = FALSE;
	SOLDIERTYPE			*pSoldier;
	INT16						sFontX, sFontY;
	INT16						sX1, sY1, sY2, sX2;
	UINT32					uiDestPitchBYTES;
	UINT8						*pDestBuf;
	UINT16					usLineColor;
  INT8            bNumRightIcons = 0;

	// If we are using an extern buffer...
	if ( fUseExternBuffer )
	{
		uiRenderBuffer = uiBuffer;
	}
	else
	{
		if ( fDisplayBuffer )
		{
			uiRenderBuffer = pFace->uiAutoDisplayBuffer;
		}
		else
		{
			uiRenderBuffer = pFace->uiAutoRestoreBuffer;

			if ( pFace->uiAutoRestoreBuffer == FACE_NO_RESTORE_BUFFER )
			{
				return;
			}
		}
	}

	// BLIT HATCH
	if ( pFace->ubSoldierID != NOBODY )
	{
		pSoldier = MercPtrs[ pFace->ubSoldierID ];

		if ( ( MercPtrs[ pFace->ubSoldierID ]->bLife < CONSCIOUSNESS || MercPtrs[ pFace->ubSoldierID ]->fDeadPanel ) )
		{
			// Blit Closed eyes here!
			BltVideoObjectFromIndex( uiRenderBuffer, pFace->uiVideoObject, 1, usEyesX, usEyesY, VO_BLT_SRCTRANSPARENCY);

			// Blit hatch!
			BltVideoObjectFromIndex( uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY, VO_BLT_SRCTRANSPARENCY);
		}

		if( MercPtrs[ pFace->ubSoldierID ]->fMercAsleep == TRUE )
		{
			// blit eyes closed
			BltVideoObjectFromIndex( uiRenderBuffer, pFace->uiVideoObject, 1, usEyesX, usEyesY, VO_BLT_SRCTRANSPARENCY);
		}

		if ( ( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
		{
			// IF we are in the process of doing any deal/close animations, show face, not skill...
			if ( !pSoldier->fClosePanel && !pSoldier->fDeadPanel && !pSoldier->fUIdeadMerc && !pSoldier->fUICloseMerc )
			{
				// Put close panel there
				BltVideoObjectFromIndex( uiRenderBuffer, guiDEAD, 5, sFaceX, sFaceY, VO_BLT_SRCTRANSPARENCY);

				// Blit hatch!
				BltVideoObjectFromIndex( uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY, VO_BLT_SRCTRANSPARENCY);
			}
		}

    // ATE: If talking in popup, don't do the other things.....
    if ( pFace->fTalking && gTacticalStatus.uiFlags & IN_ENDGAME_SEQUENCE )
    {
      return;
    }

		// ATE: Only do this, because we can be talking during an interrupt....
		if ( ( pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE ) && !fUseExternBuffer )
		{
			// Don't do this if we are being handled elsewhere and it's not an extern buffer...
		}
		else
		{
			HandleFaceHilights( pFace, uiRenderBuffer, sFaceX, sFaceY );

#ifdef JA2BETAVERSION
			if ( pSoldier->bOppCnt != 0 )
#else
			if ( pSoldier->bOppCnt > 0 )
#endif
			{
				SetFontDestBuffer( uiRenderBuffer, 0, 0, 640, 480, FALSE );

				swprintf( sString, lengthof(sString), L"%d", pSoldier->bOppCnt );

				SetFont( TINYFONT1 );
				SetFontForeground( FONT_DKRED );
				SetFontBackground( FONT_NEARBLACK );

				sX1 = (INT16)( sFaceX );
				sY1 = (INT16)( sFaceY );

				sX2 = sX1 + StringPixLength( sString, TINYFONT1 ) + 1;
				sY2 = sY1 + GetFontHeight( TINYFONT1 ) - 1;

				mprintf( (INT16)( sX1 + 1), (INT16)( sY1 - 1 ), sString );
				SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );

				// Draw box
				pDestBuf = LockVideoSurface( uiRenderBuffer, &uiDestPitchBYTES );
				SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );

				usLineColor = Get16BPPColor( FROMRGB( 105, 8, 9 ) );
				RectangleDraw( TRUE, sX1, sY1, sX2, sY2, usLineColor, pDestBuf );

				UnLockVideoSurface( uiRenderBuffer );

			}

			if ( MercPtrs[ pFace->ubSoldierID ]->bInSector && ( ( ( gTacticalStatus.ubCurrentTeam != OUR_TEAM ) || !OK_INTERRUPT_MERC(  MercPtrs[ pFace->ubSoldierID ] ) ) && !gfHiddenInterrupt ) || ( ( gfSMDisableForItems && !gfInItemPickupMenu ) && gusSMCurrentMerc == pFace->ubSoldierID && gsCurInterfacePanel == SM_PANEL ) )
			{
				// Blit hatch!
				BltVideoObjectFromIndex( uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY, VO_BLT_SRCTRANSPARENCY);
			}

			if ( !pFace->fDisabled && !pFace->fInvalidAnim )
			{
				// Render text above here if that's what was asked for
				if ( pFace->fDisplayTextOver != FACE_NO_TEXT_OVER  )
				{
					SetFont( TINYFONT1 );
					SetFontBackground( FONT_MCOLOR_BLACK );
					SetFontForeground( FONT_MCOLOR_WHITE );

					SetFontDestBuffer( uiRenderBuffer, 0, 0, 640, 480, FALSE );

					VarFindFontCenterCoordinates( sFaceX, sFaceY, pFace->usFaceWidth, pFace->usFaceHeight, TINYFONT1, &sFontX, &sFontY, pFace->zDisplayText );

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

					SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );

				}
			}

		}

    // Check if a robot and is not controlled....
	  if ( MercPtrs[ pFace->ubSoldierID ]->uiStatusFlags & SOLDIER_ROBOT )
	  {
		  if ( !CanRobotBeControlled( MercPtrs[ pFace->ubSoldierID ] ) )
      {
        // Not controlled robot
			  sIconIndex = 5;
			  fDoIcon		 = TRUE;
      }
    }

    if ( ControllingRobot( MercPtrs[ pFace->ubSoldierID ] ) )
    {
      // controlling robot
			sIconIndex = 4;
			fDoIcon		 = TRUE;
    }

    // If blind...
    if ( MercPtrs[ pFace->ubSoldierID ]->bBlindedCounter > 0 )
    {
      DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 6 );
      bNumRightIcons++;
    }

    if ( MercPtrs[ pFace->ubSoldierID ]->bDrugEffect[ DRUG_TYPE_ADRENALINE ] )
    {
      DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 7 );
      bNumRightIcons++;
    }

	  if ( GetDrunkLevel( MercPtrs[ pFace->ubSoldierID ] ) != SOBER )
	  {
      DoRightIcon( uiRenderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 8 );
      bNumRightIcons++;
    }


		switch( pSoldier->bAssignment )
		{
			case DOCTOR:

				sIconIndex = 1;
				fDoIcon		 = TRUE;
				sPtsAvailable = CalculateHealingPointsForDoctor( MercPtrs[ pFace->ubSoldierID ], &usMaximumPts, FALSE );
				fShowNumber = TRUE;
				fShowMaximum = TRUE;

				// divide both amounts by 10 to make the displayed numbers a little more user-palatable (smaller)
				sPtsAvailable = ( sPtsAvailable + 5 ) / 10;
				usMaximumPts  = ( usMaximumPts + 5 ) / 10;
				break;

			case PATIENT:

				sIconIndex = 2;
				fDoIcon		 = TRUE;
				// show current health / maximum health
				sPtsAvailable = MercPtrs[ pFace->ubSoldierID ]->bLife;
				usMaximumPts  = MercPtrs[ pFace->ubSoldierID ]->bLifeMax;
				fShowNumber = TRUE;
				fShowMaximum = TRUE;
				break;

			case TRAIN_SELF:
			case TRAIN_TOWN:
			case TRAIN_TEAMMATE:
			case TRAIN_BY_OTHER:
				sIconIndex = 3;
				fDoIcon		 = TRUE;
				fShowNumber = TRUE;
				fShowMaximum = TRUE;
				// there could be bonus pts for training at gun range
				if ( ( MercPtrs[ pFace->ubSoldierID ]->sSectorX == 13) && (MercPtrs[ pFace->ubSoldierID ]->sSectorY == MAP_ROW_H) && (MercPtrs[ pFace->ubSoldierID ]->bSectorZ == 0) )
				{
					fAtGunRange = TRUE;
				}

				switch( MercPtrs[ pFace->ubSoldierID ]->bAssignment )
				{
					case( TRAIN_SELF ):
						sPtsAvailable = GetSoldierTrainingPts( MercPtrs[ pFace->ubSoldierID ], MercPtrs[ pFace->ubSoldierID ]->bTrainStat, fAtGunRange, &usMaximumPts );
						break;
					case( TRAIN_BY_OTHER ):
						sPtsAvailable = GetSoldierStudentPts( MercPtrs[ pFace->ubSoldierID ], MercPtrs[ pFace->ubSoldierID ]->bTrainStat, fAtGunRange, &usMaximumPts );
						break;
					case( TRAIN_TOWN ):
						sPtsAvailable = GetTownTrainPtsForCharacter( MercPtrs[ pFace->ubSoldierID ], &usMaximumPts );
						// divide both amounts by 10 to make the displayed numbers a little more user-palatable (smaller)
						sPtsAvailable = ( sPtsAvailable + 5 ) / 10;
						usMaximumPts  = ( usMaximumPts + 5 ) / 10;
						break;
					case( TRAIN_TEAMMATE ):
						sPtsAvailable = GetBonusTrainingPtsDueToInstructor( MercPtrs[ pFace->ubSoldierID ], NULL , MercPtrs[ pFace->ubSoldierID ]->bTrainStat, fAtGunRange, &usMaximumPts );
						break;
				}
				break;

			case REPAIR:

				sIconIndex = 0;
				fDoIcon		 = TRUE;
				sPtsAvailable = CalculateRepairPointsForRepairman( MercPtrs[ pFace->ubSoldierID ], &usMaximumPts, FALSE );
				fShowNumber = TRUE;
				fShowMaximum = TRUE;

				// check if we are repairing a vehicle
				if ( Menptr[ pFace->ubSoldierID ].bVehicleUnderRepairID != -1 )
				{
					// reduce to a multiple of VEHICLE_REPAIR_POINTS_DIVISOR.  This way skill too low will show up as 0 repair pts.
					sPtsAvailable -= ( sPtsAvailable % VEHICLE_REPAIR_POINTS_DIVISOR );
					usMaximumPts  -= ( usMaximumPts  % VEHICLE_REPAIR_POINTS_DIVISOR );
				}

				break;
		}

		// Check for being serviced...
		if ( MercPtrs[ pFace->ubSoldierID ]->ubServicePartner != NOBODY )
		{
			// Doctor...
			sIconIndex = 1;
			fDoIcon		 = TRUE;
		}

		if ( MercPtrs[ pFace->ubSoldierID ]->ubServiceCount != 0 )
		{
			// Patient
			sIconIndex = 2;
			fDoIcon		 = TRUE;
		}


		if ( fDoIcon )
		{
			// Find X, y for placement
			GetXYForIconPlacement( pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY );
			BltVideoObjectFromIndex( uiRenderBuffer, guiPORTRAITICONS, sIconIndex, sIconX, sIconY, VO_BLT_SRCTRANSPARENCY);

      // ATE: Show numbers only in mapscreen
			if( fShowNumber )
			{
				SetFontDestBuffer( uiRenderBuffer, 0, 0, 640, 480, FALSE );

				if ( fShowMaximum )
				{
					swprintf( sString, lengthof(sString), L"%d/%d", sPtsAvailable, usMaximumPts );
				}
				else
				{
					swprintf( sString, lengthof(sString), L"%d", sPtsAvailable );
				}

				usTextWidth = StringPixLength( sString, FONT10ARIAL );
				usTextWidth += 1;

				SetFont( FONT10ARIAL );
				SetFontForeground( FONT_YELLOW );
				SetFontBackground( FONT_BLACK );

				mprintf(  sFaceX + pFace->usFaceWidth - usTextWidth, ( INT16 )( sFaceY + 3 ), sString );
				SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );
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
	CHECKF( pFace->fAllocated != FALSE );

	// Check for disabled guy!
	CHECKF( pFace->fDisabled != TRUE );

	// Set shade
	if ( pFace->ubSoldierID != NOBODY )
	{
		SetFaceShade( MercPtrs[ pFace->ubSoldierID ], pFace, FALSE );
	}

	// Blit face to save buffer!
	if ( pFace->uiAutoRestoreBuffer != FACE_NO_RESTORE_BUFFER )
	{
		if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
		{
			BltVideoObjectFromIndex( pFace->uiAutoRestoreBuffer, pFace->uiVideoObject, 0, pFace->usFaceX, pFace->usFaceY, VO_BLT_SRCTRANSPARENCY);
		}
		else
		{
			BltVideoObjectFromIndex( pFace->uiAutoRestoreBuffer, pFace->uiVideoObject, 0, 0, 0, VO_BLT_SRCTRANSPARENCY);
		}
	}

	HandleRenderFaceAdjustments( pFace, FALSE, FALSE, 0, pFace->usFaceX, pFace->usFaceY, pFace->usEyesX, pFace->usEyesY );

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


BOOLEAN ExternRenderFaceFromSoldier( UINT32 uiBuffer, UINT8 ubSoldierID, INT16 sX, INT16 sY )
{
	// Check for valid soldier
	CHECKF( ubSoldierID != NOBODY );

	return( ExternRenderFace( uiBuffer, MercPtrs[ ubSoldierID ]->iFaceIndex, sX, sY ) );
}


BOOLEAN ExternRenderFace( UINT32 uiBuffer, INT32 iFaceIndex, INT16 sX, INT16 sY )
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
	CHECKF( pFace->fAllocated != FALSE );

	// Here, any face can be rendered, even if disabled

	// Set shade
	if ( pFace->ubSoldierID != NOBODY )
	{
		SetFaceShade( MercPtrs[ pFace->ubSoldierID ], pFace , TRUE );
	}

	// Blit face to save buffer!
	BltVideoObjectFromIndex( uiBuffer, pFace->uiVideoObject, 0, sX, sY, VO_BLT_SRCTRANSPARENCY);

	GetFaceRelativeCoordinates( pFace, &usEyesX, &usEyesY, &usMouthX, &usMouthY );

	HandleRenderFaceAdjustments( pFace, FALSE, TRUE, uiBuffer, sX, sY, ( UINT16)( sX + usEyesX ), ( UINT16)( sY + usEyesY ) );

	// Restore extern rect
	if ( uiBuffer == guiSAVEBUFFER )
	{
		RestoreExternBackgroundRect( sX, sY, pFace->usFaceWidth, pFace->usFaceWidth );
	}

	return( TRUE );
}



void NewEye( FACETYPE *pFace )
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


void NewMouth( FACETYPE *pFace )
{
 BOOLEAN OK		= FALSE;
 UINT16 sOld	= pFace->sMouthFrame;

// if (audio_gap_active == 1)
 //  {
 //   Talk.mouth = 0;
 //   return;
 //  }

 do
 {
  //Talk.mouth = random(4);

  pFace->sMouthFrame = (INT16)Random(6);

  if ( pFace->sMouthFrame > 3)
	{
    pFace->sMouthFrame = 0;
	}

  switch( sOld)
  {
    case 0 : if ( pFace->sMouthFrame != 0 )
	     OK = TRUE;
	     break;
    case 1 : if ( pFace->sMouthFrame != 1 )
			 OK = TRUE;
	     break;
    case 2 : if ( pFace->sMouthFrame != 2 )
		   OK = TRUE;
	     break;
    case 3 : if ( pFace->sMouthFrame != 3 )
		   OK = TRUE;
	     break;
  }

 } while (!OK);

}


void HandleAutoFaces( )
{
	UINT32 uiCount;
	FACETYPE				*pFace;
	INT8	bLife;
	INT8	bInSector;
	INT8	bAPs;
	BOOLEAN	fRerender = FALSE;
	BOOLEAN	fHandleFace;
	BOOLEAN	fHandleUIHatch;
	SOLDIERTYPE *pSoldier;


	for ( uiCount = 0; uiCount < guiNumFaces; uiCount++ )
	{
		fRerender = FALSE;
		fHandleFace = TRUE;
		fHandleUIHatch = FALSE;

		// OK, NOW, check if our bLife status has changed, re-render if so!
		if ( gFacesData[ uiCount ].fAllocated  )
		{
			pFace = &gFacesData[ uiCount ];

			// Are we a soldier?
			if ( pFace->ubSoldierID != NOBODY )
			{
				 // Get Life now
				 pSoldier  = MercPtrs[ pFace->ubSoldierID ];
				 bLife		 = pSoldier->bLife;
				 bInSector = pSoldier->bInSector;
				 bAPs      = pSoldier->bActionPoints;

				 if ( pSoldier->ubID == gsSelectedGuy && gfUIHandleSelectionAboveGuy )
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

				 pFace->fOldHandleUIHatch = fHandleUIHatch;
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
	FACETYPE				*pFace;

	for ( uiCount = 0; uiCount < guiNumFaces; uiCount++ )
	{
		// OK, NOW, check if our bLife status has changed, re-render if so!
		if ( gFacesData[ uiCount ].fAllocated  )
		{
			pFace = &gFacesData[ uiCount ];

			HandleTalkingAutoFace( uiCount );

		}
	}
}



BOOLEAN FaceRestoreSavedBackgroundRect( INT32 iFaceIndex, INT16 sDestLeft, INT16 sDestTop, INT16 sSrcLeft, INT16 sSrcTop, INT16 sWidth, INT16 sHeight )
{
	FACETYPE					*pFace;
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;

	// Check face index
	CHECKF( iFaceIndex != -1 );

	pFace = &gFacesData[ iFaceIndex ];

	// DOn't continue if we do not want the resotre to happen ( ei blitting entrie thing every frame...
	if ( pFace->uiAutoRestoreBuffer == FACE_NO_RESTORE_BUFFER )
	{
		return( FALSE );
	}

	pDestBuf = LockVideoSurface(pFace->uiAutoDisplayBuffer, &uiDestPitchBYTES);
	pSrcBuf = LockVideoSurface( pFace->uiAutoRestoreBuffer, &uiSrcPitchBYTES);

	Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES,
				(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
				sDestLeft , sDestTop,
				sSrcLeft , sSrcTop,
				sWidth, sHeight);

	UnLockVideoSurface(pFace->uiAutoDisplayBuffer);
	UnLockVideoSurface(pFace->uiAutoRestoreBuffer);

	// Add rect to frame buffer queue
	if ( pFace->uiAutoDisplayBuffer == FRAME_BUFFER )
	{
		InvalidateRegionEx(sDestLeft - 2, sDestTop - 2, sDestLeft + sWidth + 3, sDestTop + sHeight + 2);
	}
	return(TRUE);
}


BOOLEAN SetFaceTalking( INT32 iFaceIndex, CHAR8 *zSoundFile, STR16 zTextString, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan )
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
		pFace->uiSoundID = PlayJA2GapSample( zSoundFile, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN, &(pFace->GapList ) );
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

void SetupFinalTalkingDelay( FACETYPE *pFace )
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
