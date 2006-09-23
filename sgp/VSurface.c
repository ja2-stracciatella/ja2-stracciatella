#include "Debug.h"
#include "DirectDraw_Calls.h"
#include "Local.h"
#include "MemMan.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "VSurface_Private.h"
#include "Video.h"
#include "WCheck.h"
#include <string.h>

extern void SetClippingRect(SGPRect *clip);
extern void GetClippingRect(SGPRect *clip);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface SGP Module
//
// Second Revision: Dec 10, 1996, Andrew Emmons
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Defines
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// This define is sent to CreateList SGP function. It dynamically re-sizes if
// the list gets larger
//

#define DEFAULT_NUM_REGIONS		5
#define DEFAULT_VIDEO_SURFACE_LIST_SIZE	10

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// LOCAL functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN UpdateBackupSurface( HVSURFACE hVSurface );
BOOLEAN ClipReleatedSrcAndDestRectangles( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, RECT *DestRect, RECT *SrcRect );
BOOLEAN FillSurface( HVSURFACE hDestVSurface, blt_vs_fx *pBltFx );
BOOLEAN FillSurfaceRect( HVSURFACE hDestVSurface, blt_vs_fx *pBltFx );
BOOLEAN BltVSurfaceUsingDD( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, RECT *SrcRect );
BOOLEAN BltVSurfaceUsingDDBlt( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, RECT *SrcRect, RECT *DestRect );
BOOLEAN GetVSurfaceRect( HVSURFACE hVSurface, RECT *pRect);

void DeletePrimaryVideoSurfaces( );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// LOCAL global variables
//
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct VSURFACE_NODE
{
	HVSURFACE hVSurface;
	UINT32 uiIndex;
  struct VSURFACE_NODE *next, *prev;

	#ifdef SGP_VIDEO_DEBUGGING
		UINT8									*pName;
		UINT8									*pCode;
	#endif

}VSURFACE_NODE;

VSURFACE_NODE  *gpVSurfaceHead = NULL;
VSURFACE_NODE  *gpVSurfaceTail = NULL;
UINT32				guiVSurfaceIndex = 0;
UINT32				guiVSurfaceSize = 0;
UINT32				guiVSurfaceTotalAdded = 0;

#ifdef _DEBUG
enum
{
	DEBUGSTR_NONE,
	DEBUGSTR_SETVIDEOSURFACETRANSPARENCY,
	DEBUGSTR_ADDVIDEOSURFACEREGION,
	DEBUGSTR_GETVIDEOSURFACEDESCRIPTION,
	DEBUGSTR_BLTVIDEOSURFACE_DST,
	DEBUGSTR_BLTVIDEOSURFACE_SRC,
	DEBUGSTR_COLORFILLVIDEOSURFACEAREA,
	DEBUGSTR_SHADOWVIDEOSURFACERECT,
	DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST,
	DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC,
	DEBUGSTR_DELETEVIDEOSURFACEFROMINDEX
};

UINT8 gubVSDebugCode = 0;

void CheckValidVSurfaceIndex( UINT32 uiIndex );
#endif

INT32				giMemUsedInSurfaces;


//OBSOLETE!!!!!!!!!
HLIST				ghVideoSurfaces = NULL;
//OBSOLETE!!!!!!!!!


HVSURFACE		ghPrimary = NULL;
HVSURFACE		ghBackBuffer = NULL;
HVSURFACE   ghFrameBuffer = NULL;
HVSURFACE   ghMouseBuffer = NULL;


static BOOLEAN SetPrimaryVideoSurfaces(void);


BOOLEAN InitializeVideoSurfaceManager( )
{
	//Shouldn't be calling this if the video surface manager already exists.
	//Call shutdown first...
	Assert( !gpVSurfaceHead );
	Assert( !gpVSurfaceTail );
	RegisterDebugTopic(TOPIC_VIDEOSURFACE, "Video Surface Manager");
	gpVSurfaceHead = gpVSurfaceTail = NULL;

	giMemUsedInSurfaces = 0;

	// Create primary and backbuffer from globals
	if ( !SetPrimaryVideoSurfaces( ) )
	{
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_1, String( "Could not create primary surfaces" ) );
		return FALSE;
	}

	return TRUE ;
}

BOOLEAN ShutdownVideoSurfaceManager( )
{
	VSURFACE_NODE *curr;

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Shutting down the Video Surface manager");

	// Delete primary viedeo surfaces
	DeletePrimaryVideoSurfaces( );

	while( gpVSurfaceHead )
	{
		curr = gpVSurfaceHead;
		gpVSurfaceHead = gpVSurfaceHead->next;
		DeleteVideoSurface( curr->hVSurface );
		#ifdef SGP_VIDEO_DEBUGGING
			if( curr->pName )
				MemFree( curr->pName );
			if( curr->pCode )
				MemFree( curr->pCode );
		#endif
		MemFree( curr );
	}
	gpVSurfaceHead = NULL;
	gpVSurfaceTail = NULL;
	guiVSurfaceIndex = 0;
	guiVSurfaceSize = 0;
	guiVSurfaceTotalAdded = 0;
	UnRegisterDebugTopic(TOPIC_VIDEOSURFACE, "Video Objects");
	return TRUE;
}


BOOLEAN RestoreVideoSurfaces( )
{
	VSURFACE_NODE *curr;

  //
	// Loop through Video Surfaces and Restore
  //
	curr = gpVSurfaceTail;
	while( curr )
	{
		if( !RestoreVideoSurface( curr->hVSurface ) )
		{
			return FALSE;
		}
		curr = curr->prev;
	}
	return TRUE;
}


BOOLEAN AddStandardVideoSurface( VSURFACE_DESC *pVSurfaceDesc, UINT32 *puiIndex )
{

	HVSURFACE hVSurface;

	// Assertions
	Assert( puiIndex );
	Assert( pVSurfaceDesc );

	// Create video object
	hVSurface = CreateVideoSurface( pVSurfaceDesc );

	if( !hVSurface )
	{
		// Video Object will set error condition.
		return FALSE ;
	}

	// Set transparency to default
	SetVideoSurfaceTransparencyColor( hVSurface, FROMRGB( 0, 0, 0 ) );

	// Set into video object list
	if( gpVSurfaceHead )
	{ //Add node after tail
		gpVSurfaceTail->next = (VSURFACE_NODE*)MemAlloc( sizeof( VSURFACE_NODE ) );
		Assert( gpVSurfaceTail->next ); //out of memory?
		gpVSurfaceTail->next->prev = gpVSurfaceTail;
		gpVSurfaceTail->next->next = NULL;
		gpVSurfaceTail = gpVSurfaceTail->next;
	}
	else
	{ //new list
		gpVSurfaceHead = (VSURFACE_NODE*)MemAlloc( sizeof( VSURFACE_NODE ) );
		Assert( gpVSurfaceHead ); //out of memory?
		gpVSurfaceHead->prev = gpVSurfaceHead->next = NULL;
		gpVSurfaceTail = gpVSurfaceHead;
	}
	#ifdef SGP_VIDEO_DEBUGGING
		gpVSurfaceTail->pName = NULL;
		gpVSurfaceTail->pCode = NULL;
	#endif
	//Set the hVSurface into the node.
	gpVSurfaceTail->hVSurface = hVSurface;
	gpVSurfaceTail->uiIndex = guiVSurfaceIndex+=2;
	*puiIndex = gpVSurfaceTail->uiIndex;
	Assert( guiVSurfaceIndex < 0xfffffff0 ); //unlikely that we will ever use 2 billion VSurfaces!
	//We would have to create about 70 VSurfaces per second for 1 year straight to achieve this...
	guiVSurfaceSize++;
	guiVSurfaceTotalAdded++;

	return TRUE ;
}


BYTE *LockVideoSurface( UINT32 uiVSurface, UINT32 *puiPitch )
{
	VSURFACE_NODE *curr;

  //
  // Check if given backbuffer or primary buffer
  //
#ifdef JA2
  if ( uiVSurface == PRIMARY_SURFACE )
  {
    return LockPrimarySurface( puiPitch );
  }

  if ( uiVSurface == BACKBUFFER )
  {
    return LockBackBuffer( puiPitch );
  }
#endif

  if ( uiVSurface == FRAME_BUFFER )
  {
    return LockFrameBuffer( puiPitch );
  }

  if ( uiVSurface == MOUSE_BUFFER )
  {
    return LockMouseBuffer( puiPitch );
  }

  //
  // Otherwise, use list
  //

	curr = gpVSurfaceHead;
	while( curr )
	{
		if( curr->uiIndex == uiVSurface )
		{
			break;
		}
		curr = curr->next;
	}
	if( !curr )
	{
		return FALSE;
	}

  //
	// Lock buffer
  //

	return LockVideoSurfaceBuffer( curr->hVSurface, puiPitch );

}

void UnLockVideoSurface( UINT32 uiVSurface )
{
	VSURFACE_NODE *curr;

  //
  // Check if given backbuffer or primary buffer
  //
#ifdef JA2
  if ( uiVSurface == PRIMARY_SURFACE )
  {
    UnlockPrimarySurface();
    return;
  }

  if ( uiVSurface == BACKBUFFER )
  {
    UnlockBackBuffer();
    return;
  }
#endif

  if ( uiVSurface == FRAME_BUFFER )
  {
    UnlockFrameBuffer();
    return;
  }

  if ( uiVSurface == MOUSE_BUFFER )
  {
    UnlockMouseBuffer();
    return;
  }

	curr = gpVSurfaceHead;
	while( curr )
	{
		if( curr->uiIndex == uiVSurface )
		{
			break;
		}
		curr = curr->next;
	}
	if( !curr )
	{
    return;
  }

  //
	// unlock buffer
  //

	UnLockVideoSurfaceBuffer( curr->hVSurface );
}

BOOLEAN SetVideoSurfaceTransparency( UINT32 uiIndex, COLORVAL TransColor )
{
	HVSURFACE hVSurface;

  //
	// Get Video Surface
  //

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_SETVIDEOSURFACETRANSPARENCY;
	#endif
	CHECKF( GetVideoSurface( &hVSurface, uiIndex ) );

  //
	// Set transparency
  //

	SetVideoSurfaceTransparencyColor( hVSurface, TransColor );

	return( TRUE );
}

BOOLEAN AddVideoSurfaceRegion( UINT32 uiIndex, VSURFACE_REGION *pNewRegion )
{
	HVSURFACE hVSurface;

  //
	// Get Video Surface
  //

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_ADDVIDEOSURFACEREGION;
	#endif
	CHECKF( GetVideoSurface( &hVSurface, uiIndex ) );

  //
	// Add Region
  //

	CHECKF( AddVSurfaceRegion( hVSurface, pNewRegion ) );

	return( TRUE );

}

BOOLEAN GetVideoSurfaceDescription( UINT32 uiIndex, UINT16 *usWidth, UINT16 *usHeight, UINT8 *ubBitDepth )
{
	HVSURFACE hVSurface;

	Assert( usWidth != NULL );
	Assert( usHeight != NULL );
	Assert( ubBitDepth != NULL );

  //
	// Get Video Surface
  //

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_GETVIDEOSURFACEDESCRIPTION;
	#endif
	CHECKF( GetVideoSurface( &hVSurface, uiIndex ) );

	*usWidth = hVSurface->usWidth;
	*usHeight = hVSurface->usHeight;
	*ubBitDepth = hVSurface->ubBitDepth;

	return TRUE;
}

BOOLEAN GetVideoSurface( HVSURFACE *hVSurface, UINT32 uiIndex )
{
	VSURFACE_NODE *curr;

	#ifdef _DEBUG
		CheckValidVSurfaceIndex( uiIndex );
	#endif

  if ( uiIndex == PRIMARY_SURFACE )
  {
    *hVSurface = ghPrimary;
		return TRUE;
  }

  if ( uiIndex == BACKBUFFER )
  {
    *hVSurface = ghBackBuffer;
		return TRUE;
  }

  if ( uiIndex == FRAME_BUFFER )
  {
    *hVSurface = ghFrameBuffer;
		return TRUE;
  }

  if ( uiIndex == MOUSE_BUFFER )
  {
    *hVSurface = ghMouseBuffer;
		return TRUE;
  }

	curr = gpVSurfaceHead;
	while( curr )
	{
		if( curr->uiIndex == uiIndex )
		{
			*hVSurface = curr->hVSurface;
			return TRUE;
		}
		curr = curr->next;
	}
  return FALSE;
}


static HVSURFACE CreateVideoSurfaceFromDDSurface(SDL_Surface* surface);


static BOOLEAN SetPrimaryVideoSurfaces(void)
{
	SDL_Surface* pSurface;

	// Delete surfaces if they exist
	DeletePrimaryVideoSurfaces( );

  //
	// Get Primary surface
  //
#ifdef JA2
	pSurface = GetPrimarySurfaceObject();
	CHECKF( pSurface != NULL );

	ghPrimary = CreateVideoSurfaceFromDDSurface( pSurface );
	CHECKF( ghPrimary != NULL );

  //
	// Get Backbuffer surface
  //

	pSurface = GetBackBufferObject( );
	CHECKF( pSurface != NULL );

	ghBackBuffer = CreateVideoSurfaceFromDDSurface( pSurface );
	CHECKF( ghBackBuffer != NULL );

  //
	// Get mouse buffer surface
  //
	pSurface = GetMouseBufferObject( );
	CHECKF( pSurface != NULL );

	ghMouseBuffer = CreateVideoSurfaceFromDDSurface( pSurface );
	CHECKF( ghMouseBuffer != NULL );

#endif

  //
	// Get frame buffer surface
  //

	pSurface = GetFrameBufferObject( );
	CHECKF( pSurface != NULL );

	ghFrameBuffer = CreateVideoSurfaceFromDDSurface( pSurface );
	CHECKF( ghFrameBuffer != NULL );

	return TRUE;
}

void DeletePrimaryVideoSurfaces( )
{
  //
	// If globals are not null, delete them
  //

	if ( ghPrimary != NULL )
	{
		DeleteVideoSurface( ghPrimary );
		ghPrimary = NULL;
	}

	if ( ghBackBuffer != NULL )
	{
		DeleteVideoSurface( ghBackBuffer );
		ghBackBuffer = NULL;
	}

  if ( ghFrameBuffer != NULL )
	{
		DeleteVideoSurface( ghFrameBuffer );
		ghFrameBuffer = NULL;
	}

	if ( ghMouseBuffer != NULL )
	{
		DeleteVideoSurface( ghMouseBuffer );
		ghMouseBuffer = NULL;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Given an index to the dest and src vobject contained in our private VSurface list
// Based on flags, blit accordingly
// There are two types, a BltFast and a Blt. BltFast is 10% faster, uses no
// clipping lists
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_vs_fx *pBltFx )
{

	HVSURFACE	hDestVSurface;
	HVSURFACE	hSrcVSurface;

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_BLTVIDEOSURFACE_DST;
	#endif
	if( !GetVideoSurface( &hDestVSurface, uiDestVSurface ) )
	{
		return FALSE;
	}
	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_BLTVIDEOSURFACE_SRC;
	#endif
	if( !GetVideoSurface( &hSrcVSurface, uiSrcVSurface ) )
	{
		return FALSE;
	}
	if ( !BltVideoSurfaceToVideoSurface( hDestVSurface, hSrcVSurface, usRegionIndex, iDestX, iDestY, fBltFlags, pBltFx ) )
	{ // VO Blitter will set debug messages for error conditions
		return FALSE ;
	}
	return TRUE ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Fills an rectangular area with a specified color value.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP)
{
	blt_vs_fx BltFx;
	HVSURFACE	hDestVSurface;
	SGPRect Clip;

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_COLORFILLVIDEOSURFACEAREA;
	#endif
	if( !GetVideoSurface( &hDestVSurface, uiDestVSurface ) )
	{
		return FALSE;
	}

	BltFx.ColorFill = Color16BPP;
	BltFx.DestRegion = 0;

  //
	// Clip fill region coords
  //

	GetClippingRect(&Clip);

	if(iDestX1 < Clip.iLeft)
		iDestX1 = Clip.iLeft;

	if(iDestX1 > Clip.iRight)
		return(FALSE);

	if(iDestX2 > Clip.iRight)
		iDestX2 = Clip.iRight;

	if(iDestX2 < Clip.iLeft)
		return(FALSE);

	if(iDestY1 < Clip.iTop)
		iDestY1 = Clip.iTop;

	if(iDestY1 > Clip.iBottom)
		return(FALSE);

	if(iDestY2 > Clip.iBottom)
		iDestY2 = Clip.iBottom;

	if(iDestY2 < Clip.iTop)
		return(FALSE);

	if((iDestX2 <= iDestX1) || (iDestY2 <= iDestY1))
		return(FALSE);

	BltFx.SrcRect.iLeft =	BltFx.FillRect.iLeft = iDestX1;
	BltFx.SrcRect.iTop = BltFx.FillRect.iTop = iDestY1;
	BltFx.SrcRect.iRight = BltFx.FillRect.iRight = iDestX2;
	BltFx.SrcRect.iBottom = BltFx.FillRect.iBottom = iDestY2;

	return( FillSurfaceRect( hDestVSurface, &BltFx ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Fills an rectangular area with a specified image value.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN ImageFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, HVOBJECT BkgrndImg, UINT16 Index, INT16 Ox, INT16 Oy)
{
	INT16 xc,yc,hblits,wblits,aw,pw,ah,ph,w,h,xo,yo;
	ETRLEObject		*pTrav;
	SGPRect NewClip,OldClip;

	pTrav = &(BkgrndImg->pETRLEObject[Index]);
	ph = (INT16)(pTrav->usHeight+pTrav->sOffsetY);
	pw = (INT16)(pTrav->usWidth+pTrav->sOffsetX);

	ah = (INT16)(iDestY2-iDestY1);
	aw = (INT16)(iDestX2-iDestX1);

	Ox %= pw;
	Oy %= ph;

	if(Ox>0)
		Ox -= pw;
	xo = (-Ox)%pw;

	if(Oy>0)
		Oy -= ph;
	yo = (-Oy)%ph;

	if(Ox<0)
		xo = (-Ox)%pw;
	else
	{
		xo = pw-(Ox%pw);
		Ox -= pw;
	}

	if(Oy<0)
		yo = (-Oy)%ph;
	else
	{
		yo = ph-(Oy%pw);
		Oy -= ph;
	}

	hblits = ((ah+yo)/ph) + (((ah+yo)%ph)?1:0);
	wblits = ((aw+xo)/pw) + (((aw+xo)%pw)?1:0);

	if((hblits==0) || (wblits==0))
		return(FALSE);

  //
	// Clip fill region coords
  //

	GetClippingRect(&OldClip);

	NewClip.iLeft=iDestX1;
	NewClip.iTop=iDestY1;
	NewClip.iRight=iDestX2;
	NewClip.iBottom=iDestY2;

	if(NewClip.iLeft < OldClip.iLeft)
		NewClip.iLeft = OldClip.iLeft;

	if(NewClip.iLeft > OldClip.iRight)
		return(FALSE);

	if(NewClip.iRight > OldClip.iRight)
		NewClip.iRight = OldClip.iRight;

	if(NewClip.iRight < OldClip.iLeft)
		return(FALSE);

	if(NewClip.iTop < OldClip.iTop)
		NewClip.iTop = OldClip.iTop;

	if(NewClip.iTop > OldClip.iBottom)
		return(FALSE);

	if(NewClip.iBottom > OldClip.iBottom)
		NewClip.iBottom = OldClip.iBottom;

	if(NewClip.iBottom < OldClip.iTop)
		return(FALSE);

	if((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop))
		return(FALSE);

	SetClippingRect(&NewClip);

	yc = (INT16)iDestY1;
	for(h=0;h<hblits;h++)
	{
		xc = (INT16)iDestX1;
		for(w=0;w<wblits;w++)
		{
			BltVideoObject(uiDestVSurface, BkgrndImg, Index, xc+Ox, yc+Oy, VO_BLT_SRCTRANSPARENCY, NULL);
			xc += pw;
		}
		yc += ph;
	}

	SetClippingRect(&OldClip);
	return(TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manipulation Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

HVSURFACE CreateVideoSurface( VSURFACE_DESC *VSurfaceDesc )
{
	HVSURFACE						hVSurface;
	HIMAGE							hImage;
	SGPRect							tempRect;
	UINT16							usHeight;
	UINT16							usWidth;
	UINT8								ubBitDepth;
	UINT32							uiRBitMask;
	UINT32							uiGBitMask;
	UINT32							uiBBitMask;
	SDL_Surface* surface;
	Uint32 surface_flags;

	// Check creation options
	do
	{
		if (VSurfaceDesc->fCreateFlags & VSURFACE_CREATE_FROMFILE)
		{
			hImage = CreateImage(VSurfaceDesc->ImageFile, IMAGE_ALLIMAGEDATA);

			if (hImage == NULL)
			{
					DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid Image Filename given");
					return NULL;
			}

			// Set values from himage
			usHeight   = hImage->usHeight;
			usWidth    = hImage->usWidth;
			ubBitDepth = hImage->ubBitDepth;
			break;
		}

		// If here, no special options given,
		// Set values from given description structure
		usHeight   = VSurfaceDesc->usHeight;
		usWidth    = VSurfaceDesc->usWidth;
		ubBitDepth = VSurfaceDesc->ubBitDepth;
	}	while (FALSE);

	Assert(usHeight > 0);
	Assert(usWidth  > 0);

	switch (ubBitDepth)
	{
		case 8:
			uiRBitMask = 0;
			uiGBitMask = 0;
			uiBBitMask = 0;
			break;

		case 16:
		{
			const SDL_Surface* screen = SDL_GetVideoSurface();
			uiRBitMask = screen->format->Rmask;
			uiGBitMask = screen->format->Gmask;
			uiBBitMask = screen->format->Bmask;
			break;
		}

		default:
			// If Here, an invalid format was given
			DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid BPP value, can only be 8 or 16.");
			return FALSE;
	}

	// Do memory description, based on specified flags
	do
	{
		if (VSurfaceDesc->fCreateFlags & VSURFACE_DEFAULT_MEM_USAGE)
		{
			surface_flags = 0;
			break;
		}
		if (VSurfaceDesc->fCreateFlags & VSURFACE_VIDEO_MEM_USAGE)
		{
			surface_flags = SDL_HWSURFACE;
			break;
		}

		if (VSurfaceDesc->fCreateFlags & VSURFACE_SYSTEM_MEM_USAGE)
		{
			surface_flags = SDL_SWSURFACE;
			break;
		}

		// Once here, no mem flags were given, use default
		surface_flags = 0;
	} while (FALSE);

	surface = SDL_CreateRGBSurface(
		surface_flags,
		usWidth, usHeight, ubBitDepth,
		uiRBitMask, uiGBitMask, uiBBitMask, 0
	);

	// Allocate memory for Video Surface data and initialize
	hVSurface = MemAlloc(sizeof(SGPVSurface));
	memset(hVSurface, 0, sizeof(SGPVSurface));
	CHECKF(hVSurface != NULL);

	hVSurface->surface            = surface;
	hVSurface->usHeight           = usHeight;
	hVSurface->usWidth            = usWidth;
	hVSurface->ubBitDepth         = ubBitDepth;
	hVSurface->pSurfaceData1      = NULL; // XXX remove
	hVSurface->pSurfaceData       = NULL; // XXX remove
	hVSurface->pSavedSurfaceData1 = NULL;
	hVSurface->pSavedSurfaceData  = NULL;
	hVSurface->pPalette           = NULL;
	hVSurface->p16BPPPalette      = NULL;
	hVSurface->TransparentColor   = FROMRGB(0, 0, 0);
	hVSurface->RegionList         = CreateList(DEFAULT_NUM_REGIONS, sizeof(VSURFACE_REGION));
	hVSurface->fFlags             = 0;
	hVSurface->pClipper           = NULL;

	// Determine memory and other attributes of newly created surface

	// Fail if create tried for video but it's in system
	if (VSurfaceDesc->fCreateFlags & VSURFACE_VIDEO_MEM_USAGE &&
			!(surface->flags & SDL_HWSURFACE))
	{
		DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String("Failed to create Video Surface in video memory"));
		SDL_FreeSurface(surface);
		MemFree(hVSurface);
		return NULL;
	}

	// Look for system memory
	if (surface->flags & SDL_SWSURFACE)
	{
		hVSurface->fFlags |= VSURFACE_SYSTEM_MEM_USAGE;
	}

	// Look for video memory
	if (surface->flags & SDL_HWSURFACE)
	{
		hVSurface->fFlags |= VSURFACE_VIDEO_MEM_USAGE;
	}

	// If in video memory, create backup surface
	if (hVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE)
	{
#if 1
		UNIMPLEMENTED();
#else
		SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
		SurfaceDescription.dwWidth = usWidth;
		SurfaceDescription.dwHeight = usHeight;
		SurfaceDescription.ddpfPixelFormat = PixelFormat;

    //
		// Create Surface
    //

		DDCreateSurface (	lpDD2Object, &SurfaceDescription, &lpDDS, &lpDDS2 );

    //
		// Save surface to backup
    //

		hVSurface->pSavedSurfaceData1 = lpDDS;
		hVSurface->pSavedSurfaceData = lpDDS2;
#endif
	}

  //
	// Initialize surface with hImage , if given
  //

	if ( VSurfaceDesc->fCreateFlags & VSURFACE_CREATE_FROMFILE )
	{
		tempRect.iLeft = 0;
		tempRect.iTop = 0;
		tempRect.iRight = hImage->usWidth-1;
		tempRect.iBottom = hImage->usHeight-1;
		SetVideoSurfaceDataFromHImage( hVSurface, hImage, 0, 0, &tempRect );

    //
		// Set palette from himage
    //

		if ( hImage->ubBitDepth == 8 )
		{
			SetVideoSurfacePalette( hVSurface, hImage->pPalette );
		}

    //
		// Delete himage object
    //

		DestroyImage( hImage );
	}

  //
	// All is well
  //

	hVSurface->usHeight					= usHeight;
	hVSurface->usWidth						= usWidth;
	hVSurface->ubBitDepth				= ubBitDepth;

	giMemUsedInSurfaces += ( hVSurface->usHeight * hVSurface->usWidth * ( hVSurface->ubBitDepth / 8 ) );

  DbgMessage( TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Success in Creating Video Surface" ) );

	return( hVSurface );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Called when surface is lost, for the most part called by utility functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN RestoreVideoSurface( HVSURFACE hVSurface )
{
	LPDIRECTDRAWSURFACE2		lpDDSurface;
	LPDIRECTDRAWSURFACE2		lpBackupDDSurface;
	RECT										aRect;

	Assert( hVSurface != NULL );

  //
	// Restore is only for VIDEO MEMORY - should check if VIDEO and QUIT if not
  //

	if ( !( hVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE ) )
	{
    //
		// No second surfaace has been allocated, return failure
    //

	  DbgMessage( TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String("Failed to restore Video Surface surface" ) );
		return( FALSE );
	}

  //
	// Check for valid secondary surface
  //

	if ( hVSurface->pSavedSurfaceData1 == NULL )
	{
    //
		// No secondary surface available
    //

	  DbgMessage( TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String("Failure in retoring- no secondary surface found" ) );
		return( FALSE );
	}

	// Restore primary surface
	lpDDSurface = ( LPDIRECTDRAWSURFACE2 )hVSurface->pSurfaceData;
	DDRestoreSurface( lpDDSurface );

	// Blit backup surface data into primary
	lpBackupDDSurface = ( LPDIRECTDRAWSURFACE2 )hVSurface->pSavedSurfaceData;

	aRect.top = (int)0;
	aRect.left = (int)0;
	aRect.bottom = (int)hVSurface->usHeight;
	aRect.right = (int)hVSurface->usWidth;

	DDBltFastSurface( (LPDIRECTDRAWSURFACE2)hVSurface->pSavedSurfaceData, 0, 0, (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData, &aRect, DDBLTFAST_NOCOLORKEY );

	return( TRUE );
}


// Lock must be followed by release
// Pitch MUST be used for all width calculations ( Pitch is in bytes )
// The time between Locking and unlocking must be minimal
BYTE *LockVideoSurfaceBuffer( HVSURFACE hVSurface, UINT32 *pPitch )
{
	Assert( hVSurface != NULL );
	Assert( pPitch != NULL );

	SDL_LockSurface(hVSurface->surface); // XXX necessary?
	*pPitch = hVSurface->surface->pitch;
	return hVSurface->surface->pixels;
}

void UnLockVideoSurfaceBuffer( HVSURFACE hVSurface )
{
	Assert( hVSurface != NULL );

	SDL_UnlockSurface(hVSurface->surface); // XXX necessary?

	// Copy contents if surface is in video
	if ( hVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE && !hVSurface->fFlags & VSURFACE_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hVSurface );
	}
}

// Given an HIMAGE object, blit imagery into existing Video Surface. Can be from 8->16 BPP
BOOLEAN SetVideoSurfaceDataFromHImage( HVSURFACE hVSurface, HIMAGE hImage, UINT16 usX, UINT16 usY, SGPRect *pSrcRect )
{
	BYTE		*pDest;
	UINT32	fBufferBPP = 0;
	UINT32  uiPitch;
	UINT16  usEffectiveWidth;
	SGPRect	aRect;

	// Assertions
	Assert( hVSurface != NULL );
	Assert( hImage != NULL );

	// Get Size of hImage and determine if it can fit
	CHECKF( hImage->usWidth  >= hVSurface->usWidth );
	CHECKF( hImage->usHeight >= hVSurface->usHeight );

	// Check BPP and see if they are the same
	if ( hImage->ubBitDepth != hVSurface->ubBitDepth )
	{
		// They are not the same, but we can go from 8->16 without much cost
		if ( hImage->ubBitDepth == 8 && hVSurface->ubBitDepth == 16 )
		{
			fBufferBPP = BUFFER_16BPP;
		}
	}
	else
	{
		// Set buffer BPP
		switch ( hImage->ubBitDepth )
		{
			case 8:

				fBufferBPP = BUFFER_8BPP;
				break;

			case 16:

				fBufferBPP = BUFFER_16BPP;
				break;
		}

	}

	Assert( fBufferBPP != 0 );

	// Get surface buffer data
	pDest = LockVideoSurfaceBuffer( hVSurface, &uiPitch );

	// Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
	usEffectiveWidth = (UINT16)( uiPitch / ( hVSurface->ubBitDepth / 8 ) );

	CHECKF( pDest != NULL );

	// Blit Surface
	// If rect is NULL, use entrie image size
	if ( pSrcRect == NULL )
	{
		aRect.iLeft = 0;
		aRect.iTop = 0;
		aRect.iRight = hImage->usWidth;
		aRect.iBottom = hImage->usHeight;
	}
	else
	{
		aRect.iLeft = pSrcRect->iLeft;
		aRect.iTop = pSrcRect->iTop;
		aRect.iRight = pSrcRect->iRight;
		aRect.iBottom = pSrcRect->iBottom;
	}

	// This HIMAGE function will transparently copy buffer
	if ( !CopyImageToBuffer( hImage, fBufferBPP, pDest, usEffectiveWidth, hVSurface->usHeight, usX, usY, &aRect ) )
	{
    DbgMessage( TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Error Occured Copying HIMAGE to HVSURFACE" ));
		UnLockVideoSurfaceBuffer( hVSurface );
		return( FALSE );
	}

	// All is OK
	UnLockVideoSurfaceBuffer( hVSurface );

	return( TRUE );

}

// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
BOOLEAN SetVideoSurfacePalette( HVSURFACE hVSurface, SGPPaletteEntry *pSrcPalette )
{
	UINT32 i;

	Assert( hVSurface != NULL );

	// Create palette object if not already done so
	if ( hVSurface->pPalette == NULL )
	{
		hVSurface->pPalette = MemAlloc(sizeof(*hVSurface->pPalette) * 256);
	}
	for (i = 0; i < 256; i++)
	{
		hVSurface->pPalette[i].r = pSrcPalette[i].peRed;
		hVSurface->pPalette[i].g = pSrcPalette[i].peGreen;
		hVSurface->pPalette[i].b = pSrcPalette[i].peBlue;
	}

	// Delete 16BPP Palette if one exists
	if ( hVSurface->p16BPPPalette != NULL )
	{
		MemFree( hVSurface->p16BPPPalette );
		hVSurface->p16BPPPalette = NULL;
	}

	// Create 16BPP Palette
	hVSurface->p16BPPPalette = Create16BPPPalette( pSrcPalette );

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Video Surface Palette change successfull" ));
	return( TRUE );
}

// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
BOOLEAN SetVideoSurfaceTransparencyColor( HVSURFACE hVSurface, COLORVAL TransColor )
{
#if 1
	FIXME
#else
  DDCOLORKEY		ColorKey;
  DWORD					fFlags;
	LPDIRECTDRAWSURFACE2	lpDDSurface;

	// Assertions
	Assert( hVSurface != NULL );

	//Set trans color into Video Surface
	hVSurface->TransparentColor = TransColor;

	// Get surface pointer
	lpDDSurface = (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData;
	CHECKF( lpDDSurface != NULL );

	// Get right pixel format, based on bit depth

	switch( hVSurface->ubBitDepth )
	{
			case 8:

				// Use color directly
				ColorKey.dwColorSpaceLowValue  = TransColor;
				ColorKey.dwColorSpaceHighValue = TransColor;
				break;

			case 16:

				fFlags = Get16BPPColor( TransColor );

				//fFlags now contains our closest match
				ColorKey.dwColorSpaceLowValue  = fFlags;
				ColorKey.dwColorSpaceHighValue = ColorKey.dwColorSpaceLowValue;
				break;

	}

	DDSetSurfaceColorKey( lpDDSurface, DDCKEY_SRCBLT, &ColorKey);
#endif

	return( TRUE );
}


BOOLEAN GetVSurfacePaletteEntries( HVSURFACE hVSurface, SGPPaletteEntry *pPalette )
{
	UINT32 i;

	CHECKF( hVSurface->pPalette != NULL );

	for (i = 0; i < 256; i++)
	{
		pPalette[i].peRed = hVSurface->pPalette[i].r;
		pPalette[i].peRed = hVSurface->pPalette[i].g;
		pPalette[i].peRed = hVSurface->pPalette[i].b;
	}

	return( TRUE );
}


BOOLEAN DeleteVideoSurfaceFromIndex( UINT32 uiIndex )
{
	VSURFACE_NODE *curr;

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_DELETEVIDEOSURFACEFROMINDEX;
		CheckValidVSurfaceIndex( uiIndex );
	#endif

	curr = gpVSurfaceHead;
	while( curr )
	{
		if( curr->uiIndex == uiIndex )
		{ //Found the node, so detach it and delete it.

			//Deallocate the memory for the video surface
			DeleteVideoSurface( curr->hVSurface );

			if( curr == gpVSurfaceHead )
			{ //Advance the head, because we are going to remove the head node.
				gpVSurfaceHead = gpVSurfaceHead->next;
			}
			if( curr == gpVSurfaceTail )
			{ //Back up the tail, because we are going to remove the tail node.
				gpVSurfaceTail = gpVSurfaceTail->prev;
			}
			//Detach the node from the vsurface list
			if( curr->next )
			{ //Make the prev node point to the next
				curr->next->prev = curr->prev;
			}
			if( curr->prev )
			{ //Make the next node point to the prev
				curr->prev->next = curr->next;
			}
			//The node is now detached.  Now deallocate it.

			#ifdef SGP_VIDEO_DEBUGGING
				if( curr->pName )
				{
					MemFree( curr->pName );
				}
				if( curr->pCode )
				{
					MemFree( curr->pCode );
				}
			#endif

			MemFree( curr );
			guiVSurfaceSize--;
			return TRUE;
		}
		curr = curr->next;
	}
	return FALSE;
}


// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoSurface( HVSURFACE hVSurface )
{
	LPDIRECTDRAWSURFACE2	lpDDSurface;

	// Assertions
	CHECKF( hVSurface != NULL );

	// Release palette
	if ( hVSurface->pPalette != NULL )
	{
		MemFree(hVSurface->pPalette);
		hVSurface->pPalette = NULL;
	}

	//if ( hVSurface->pClipper != NULL )
	//{
		// Release Clipper
	//	DDReleaseClipper( (LPDIRECTDRAWCLIPPER)hVSurface->pClipper );
	//}

	// Get surface pointer
	lpDDSurface = (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData;

	// Release surface
	if ( hVSurface->pSurfaceData1 != NULL )
	{
		DDReleaseSurface((LPDIRECTDRAWSURFACE*)&hVSurface->pSurfaceData1, &lpDDSurface );
	}

	// Release backup surface
	if ( hVSurface->pSavedSurfaceData != NULL )
	{
		DDReleaseSurface((LPDIRECTDRAWSURFACE*)&hVSurface->pSavedSurfaceData1,
								(LPDIRECTDRAWSURFACE2*)&hVSurface->pSavedSurfaceData );
	}

	// Release region data
	DeleteList( hVSurface->RegionList );

	//If there is a 16bpp palette, free it
	if( hVSurface->p16BPPPalette != NULL )
	{
		MemFree( hVSurface->p16BPPPalette );
		hVSurface->p16BPPPalette = NULL;
	}

	giMemUsedInSurfaces -= ( hVSurface->usHeight * hVSurface->usWidth * ( hVSurface->ubBitDepth / 8 ) );

	// Release object
	MemFree( hVSurface );

	return( TRUE );
}

// ********************************************************
//
// Clipper manipulation functions
//
// ********************************************************

BOOLEAN SetClipList( HVSURFACE hVSurface, SGPRect *RegionData, UINT16 usNumRegions )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	RGNDATA							*pRgnData;
	UINT16							cnt;
	RECT								aRect;
	LPDIRECTDRAW2				lpDD2Object;

	// Get Direct Draw Object
	lpDD2Object = GetDirectDraw2Object( );

	// Assertions
	Assert( hVSurface != NULL );
	Assert( RegionData != NULL );

	// Varifications
	CHECKF( usNumRegions > 0 );

	// If Clipper already created, release
	if ( hVSurface->pClipper != NULL )
	{
		// Release Clipper
		DDReleaseClipper( (LPDIRECTDRAWCLIPPER)hVSurface->pClipper );
	}

	// Create Clipper Object
	DDCreateClipper( lpDD2Object, 0, &((LPDIRECTDRAWCLIPPER)hVSurface->pClipper) );

	// Allocate region data
	pRgnData = ( LPRGNDATA )MemAlloc( sizeof( RGNDATAHEADER) + ( usNumRegions * sizeof( RECT ) ) );
	CHECKF( pRgnData );

	// Setup header
	pRgnData->rdh.dwSize = sizeof( RGNDATA );
	pRgnData->rdh.iType = RDH_RECTANGLES;
	pRgnData->rdh.nCount = usNumRegions;
	pRgnData->rdh.nRgnSize = usNumRegions * sizeof( RECT );
	pRgnData->rdh.rcBound.top  = 0;
	pRgnData->rdh.rcBound.left  = 0;
	pRgnData->rdh.rcBound.bottom  = (int)hVSurface->usHeight;
	pRgnData->rdh.rcBound.right = (int)hVSurface->usWidth;

	// Copy rectangles into region
	for ( cnt = 0; cnt < usNumRegions; cnt++ )
	{
		aRect.top = (UINT32)RegionData[ cnt ].iTop;
		aRect.left = (UINT32)RegionData[ cnt ].iLeft;
		aRect.bottom = (UINT32)RegionData[ cnt ].iBottom;
		aRect.right = (UINT32)RegionData[ cnt ].iRight;

		memcpy( pRgnData + sizeof( RGNDATAHEADER) + ( cnt * sizeof( RECT ) ), &aRect, sizeof( RECT ) );
	}

	// Set items into clipper
	DDSetClipperList( (LPDIRECTDRAWCLIPPER)hVSurface->pClipper, pRgnData, 0 );

	// Set Clipper into Surface
	DDSetClipper( (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData, (LPDIRECTDRAWCLIPPER)hVSurface->pClipper );

	// Delete region data
	MemFree( pRgnData );

	return( TRUE );
#endif
}


// ********************************************************
//
// Region manipulation functions
//
// ********************************************************

BOOLEAN GetNumRegions( HVSURFACE hVSurface , UINT32 *puiNumRegions )
{
	Assert( hVSurface );

	*puiNumRegions = ListSize( hVSurface->RegionList );

	return( TRUE );

}

BOOLEAN AddVSurfaceRegion( HVSURFACE hVSurface, VSURFACE_REGION *pNewRegion )
{
	Assert( hVSurface != NULL );
	Assert( pNewRegion != NULL );

	// Add new region to list
	hVSurface->RegionList = AddtoList( hVSurface->RegionList, pNewRegion, ListSize( hVSurface->RegionList ) );

	return( TRUE );
}

// Add a group of regions
BOOLEAN AddVSurfaceRegions( HVSURFACE hVSurface, VSURFACE_REGION **ppNewRegions, UINT16 uiNumRegions )
{
	UINT16 cnt;

	Assert( hVSurface != NULL );
	Assert( ppNewRegions != NULL );

	for ( cnt = 0; cnt < uiNumRegions; cnt++ )
	{
		AddVSurfaceRegion( hVSurface, ppNewRegions[ cnt ] );
	}

	return( TRUE );
}

BOOLEAN RemoveVSurfaceRegion( HVSURFACE hVSurface, UINT16 usIndex )
{
	VSURFACE_REGION	aRegion;

	Assert( hVSurface != NULL );

	return( RemfromList( hVSurface->RegionList, &aRegion, usIndex ) );

}

BOOLEAN ClearAllVSurfaceRegions( HVSURFACE hVSurface )
{
	UINT32 uiListSize, cnt;

	Assert( hVSurface != NULL );

	uiListSize = ListSize( hVSurface->RegionList );

	for ( cnt = uiListSize - 1; cnt >= 0; cnt-- )
	{
		RemoveVSurfaceRegion( hVSurface, (UINT16)cnt );
	}

	return( TRUE );
}

BOOLEAN GetVSurfaceRegion( HVSURFACE hVSurface, UINT16 usIndex, VSURFACE_REGION *aRegion )
{
	Assert( hVSurface != NULL );

	if ( !PeekList( hVSurface->RegionList, aRegion, usIndex ) )
	{
		return( FALSE );
	}

	return( TRUE );
}

BOOLEAN GetVSurfaceRect( HVSURFACE hVSurface, RECT *pRect)
{
	Assert( hVSurface != NULL );
	Assert( pRect != NULL );

	pRect->left=0;
	pRect->top=0;
	pRect->right=hVSurface->usWidth;
	pRect->bottom=hVSurface->usHeight;

	return( TRUE );
}

BOOLEAN ReplaceVSurfaceRegion( HVSURFACE hVSurface , UINT16 usIndex, VSURFACE_REGION *aRegion )
{
	VSURFACE_REGION OldRegion;

	Assert( hVSurface != NULL );

	// Validate index given
	if ( !PeekList( hVSurface->RegionList, &OldRegion, usIndex ) )
	{
		return( FALSE );
	}

	// Replace information
	hVSurface->RegionList = AddtoList( hVSurface->RegionList, aRegion, usIndex );

	return( TRUE );
}

BOOLEAN AddVSurfaceRegionAtIndex( HVSURFACE hVSurface, UINT16 usIndex, VSURFACE_REGION *pNewRegion )
{
	Assert( hVSurface != NULL );
	Assert( pNewRegion != NULL );

	// Add new region to list
	hVSurface->RegionList = AddtoList( hVSurface->RegionList, pNewRegion, usIndex );

	return( TRUE );

}

// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// Blt  will use DD Blt or BltFast depending on flags.
// Will drop down into user-defined blitter if 8->16 BPP blitting is being done

BOOLEAN BltVideoSurfaceToVideoSurface( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags, blt_vs_fx *pBltFx )
{
	VSURFACE_REGION aRegion;
	RECT					 SrcRect, DestRect;
	UINT8					*pSrcSurface8, *pDestSurface8;
	UINT16				*pDestSurface16, *pSrcSurface16;
	UINT32				uiSrcPitch, uiDestPitch, uiWidth, uiHeight;

	// Assertions
	Assert( hDestVSurface != NULL );

	// Check that both region and subrect are not given
	if ((fBltFlags&VS_BLT_SRCREGION) && (fBltFlags&VS_BLT_SRCSUBRECT))
	{
			DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Inconsistant blit flags given" ));
			return( FALSE );
	}

	// Check for dest src options
	if ( fBltFlags & VS_BLT_DESTREGION )
	{
			CHECKF( pBltFx != NULL );
			CHECKF( GetVSurfaceRegion( hDestVSurface, pBltFx->DestRegion, &aRegion ) );

			// Set starting coordinates from destination region
			iDestY = aRegion.RegionCoords.iTop;
			iDestX = aRegion.RegionCoords.iLeft;
	}

	// Check for fill, if true, fill entire region with color
	if ( fBltFlags & VS_BLT_COLORFILL )
	{
		return( FillSurface( hDestVSurface, pBltFx ) );
	}

	// Check for colorfill rectangle
	if ( fBltFlags & VS_BLT_COLORFILLRECT )
	{
		return( FillSurfaceRect( hDestVSurface, pBltFx ) );
	}

	// Check for source coordinate options - from region, specific rect or full src dimensions
	do
	{
		// Get Region from index, if specified
		if ( fBltFlags & VS_BLT_SRCREGION )
		{
			CHECKF( GetVSurfaceRegion( hSrcVSurface, usIndex, &aRegion ) )

			SrcRect.top = (int)aRegion.RegionCoords.iTop;
			SrcRect.left = (int)aRegion.RegionCoords.iLeft;
			SrcRect.bottom = (int)aRegion.RegionCoords.iBottom;
			SrcRect.right = (int)aRegion.RegionCoords.iRight;
			break;
		}

		// Use SUBRECT if specified
		if ( fBltFlags & VS_BLT_SRCSUBRECT )
		{
			SGPRect aSubRect;

			CHECKF( pBltFx != NULL );

			aSubRect = pBltFx->SrcRect;

			SrcRect.top = (int)aSubRect.iTop;
			SrcRect.left = (int)aSubRect.iLeft;
			SrcRect.bottom = (int)aSubRect.iBottom;
			SrcRect.right = (int)aSubRect.iRight;

			break;
		}

		// Here, use default, which is entire Video Surface
		// Check Sizes, SRC size MUST be <= DEST size
		if ( hDestVSurface->usHeight < hSrcVSurface->usHeight )
		{
				DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Incompatible height size given in Video Surface blit" ));
				return( FALSE );
		}
		if ( hDestVSurface->usWidth < hSrcVSurface->usWidth )
		{
				DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Incompatible height size given in Video Surface blit" ));
				return( FALSE );
		}

		SrcRect.top = (int)0;
		SrcRect.left = (int)0;
		SrcRect.bottom = (int)hSrcVSurface->usHeight;
		SrcRect.right = (int)hSrcVSurface->usWidth;

	} while( FALSE );

	// Once here, assert valid Src
	Assert( hSrcVSurface != NULL );

	// clipping -- added by DB
	GetVSurfaceRect( hDestVSurface, &DestRect);
	uiWidth=SrcRect.right-SrcRect.left;
	uiHeight=SrcRect.bottom-SrcRect.top;

	// check for position entirely off the screen
	if(iDestX >= DestRect.right)
		return(FALSE);
	if(iDestY >= DestRect.bottom)
		return(FALSE);
	if((iDestX+(INT32)uiWidth) < (INT32)DestRect.left)
		return(FALSE);
	if((iDestY+(INT32)uiHeight) < (INT32)DestRect.top)
		return(FALSE);

	// DB The mirroring stuff has to do it's own clipping because
	// it needs to invert some of the numbers
	if(!(fBltFlags & VS_BLT_MIRROR_Y))
	{
		if((iDestX+(INT32)uiWidth) >= (INT32)DestRect.right)
		{
			SrcRect.right-=((iDestX+uiWidth)-DestRect.right);
			uiWidth-=((iDestX+uiWidth)-DestRect.right);
		}
		if((iDestY+(INT32)uiHeight) >= (INT32)DestRect.bottom)
		{
			SrcRect.bottom-=((iDestY+uiHeight)-DestRect.bottom);
			uiHeight-=((iDestY+uiHeight)-DestRect.bottom);
		}
		if(iDestX < DestRect.left)
		{
			SrcRect.left+=(DestRect.left-iDestX);
			uiWidth-=(DestRect.left-iDestX);
			iDestX=DestRect.left;
		}
		if(iDestY < (INT32)DestRect.top)
		{
			SrcRect.top+=(DestRect.top-iDestY);
			uiHeight-=(DestRect.top-iDestY);
			iDestY=DestRect.top;
		}
	}

	// Send dest position, rectangle, etc to DD bltfast function
	// First check BPP values for compatibility
	if ( hDestVSurface->ubBitDepth == 16 && hSrcVSurface->ubBitDepth == 16 )
	{
		if(fBltFlags & VS_BLT_MIRROR_Y)
		{
			if((pSrcSurface16=(UINT16 *)LockVideoSurfaceBuffer(hSrcVSurface, &uiSrcPitch))==NULL)
			{
				DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Failed on lock of 16BPP surface for blitting" ));
				return(FALSE);
			}

			if((pDestSurface16=(UINT16 *)LockVideoSurfaceBuffer(hDestVSurface, &uiDestPitch))==NULL)
			{
				UnLockVideoSurfaceBuffer(hSrcVSurface);
				DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Failed on lock of 16BPP dest surface for blitting" ));
				return(FALSE);
			}

			Blt16BPPTo16BPPMirror(pDestSurface16, uiDestPitch, pSrcSurface16, uiSrcPitch, iDestX, iDestY, SrcRect.left, SrcRect.top, uiWidth, uiHeight);
			UnLockVideoSurfaceBuffer(hSrcVSurface);
			UnLockVideoSurfaceBuffer(hDestVSurface);
			return(TRUE);
		}
// For testing with non-DDraw blitting, uncomment to test -- DB
#ifndef JA2
		else
		{
			if((pSrcSurface16=(UINT16 *)LockVideoSurfaceBuffer(hSrcVSurface, &uiSrcPitch))==NULL)
			{
				DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Failed on lock of 16BPP surface for blitting" ));
				return(FALSE);
			}

			if((pDestSurface16=(UINT16 *)LockVideoSurfaceBuffer(hDestVSurface, &uiDestPitch))==NULL)
			{
				UnLockVideoSurfaceBuffer(hSrcVSurface);
				DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Failed on lock of 16BPP dest surface for blitting" ));
				return(FALSE);
			}

			Blt16BPPTo16BPP(pDestSurface16, uiDestPitch, pSrcSurface16, uiSrcPitch, iDestX, iDestY, SrcRect.left, SrcRect.top, uiWidth, uiHeight);
			UnLockVideoSurfaceBuffer(hSrcVSurface);
			UnLockVideoSurfaceBuffer(hDestVSurface);
			return(TRUE);
		}
#endif

		CHECKF( BltVSurfaceUsingDD( hDestVSurface, hSrcVSurface, fBltFlags, iDestX, iDestY, &SrcRect ) );

	}
	else if ( hDestVSurface->ubBitDepth == 8 && hSrcVSurface->ubBitDepth == 8 )
	{
		if((pSrcSurface8=(UINT8 *)LockVideoSurfaceBuffer(hSrcVSurface, &uiSrcPitch))==NULL)
		{
			DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Failed on lock of 8BPP surface for blitting" ));
			return(FALSE);
		}

		if((pDestSurface8=(UINT8 *)LockVideoSurfaceBuffer(hDestVSurface, &uiDestPitch))==NULL)
		{
			UnLockVideoSurfaceBuffer(hSrcVSurface);
			DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Failed on lock of 8BPP dest surface for blitting" ));
			return(FALSE);
		}

		//Blt8BPPDataTo8BPPBuffer( UINT8 *pBuffer, UINT32 uiDestPitchBYTES, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex );
		Blt8BPPTo8BPP(pDestSurface8, uiDestPitch, pSrcSurface8, uiSrcPitch, iDestX, iDestY, SrcRect.left, SrcRect.top, uiWidth, uiHeight);
		UnLockVideoSurfaceBuffer(hSrcVSurface);
		UnLockVideoSurfaceBuffer(hDestVSurface);
		return(TRUE);
	}
	else
	{
		DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, String( "Incompatible BPP values with src and dest Video Surfaces for blitting" ));
		return( FALSE );
	}

	return( TRUE );
}



// ******************************************************************************************
//
// UTILITY FUNCTIONS
//
// ******************************************************************************************

// Blt to backup buffer
BOOLEAN UpdateBackupSurface( HVSURFACE hVSurface )
{
	RECT		aRect;

	// Assertions
	Assert( hVSurface != NULL );

	// Validations
	CHECKF( hVSurface->pSavedSurfaceData != NULL );

	aRect.top = (int)0;
	aRect.left = (int)0;
	aRect.bottom = (int)hVSurface->usHeight;
	aRect.right = (int)hVSurface->usWidth;

	// Copy all contents into backup buffer
	DDBltFastSurface( (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData, 0, 0, (LPDIRECTDRAWSURFACE2)hVSurface->pSavedSurfaceData, &aRect, DDBLTFAST_NOCOLORKEY );

	return( TRUE );

}


// *****************************************************************************
//
// Private DirectDraw manipulation functions
//
// *****************************************************************************

LPDIRECTDRAWSURFACE2 GetVideoSurfaceDDSurface( HVSURFACE hVSurface )
{
	Assert( hVSurface != NULL );

	return( (LPDIRECTDRAWSURFACE2) hVSurface->pSurfaceData );
}


static HVSURFACE CreateVideoSurfaceFromDDSurface(SDL_Surface* surface)
{
	// Create Video Surface
	HVSURFACE						hVSurface;
	LPDIRECTDRAWPALETTE	pDDPalette;
	SGPPaletteEntry			SGPPalette[ 256 ];
	HRESULT							ReturnCode;

	hVSurface = MemAlloc(sizeof(*hVSurface));

	hVSurface->surface           = surface;
	hVSurface->usHeight          = surface->h;
	hVSurface->usWidth           = surface->h;
	hVSurface->ubBitDepth        = surface->format->BitsPerPixel;
	hVSurface->pSurfaceData      = NULL; // XXX remove
	hVSurface->pSurfaceData1     = NULL; // XXX remove
	hVSurface->pSavedSurfaceData = NULL; // XXX remove
	hVSurface->RegionList        = CreateList( DEFAULT_NUM_REGIONS, sizeof( VSURFACE_REGION ) );
	hVSurface->fFlags            = 0;

	if (surface->format->palette != NULL) // XXX necessary?
	{
		UINT32 i;

		hVSurface->pPalette = MemAlloc(sizeof(*hVSurface->pPalette) * 256);
		for (i = 0; i < 256; i++)
		{
			hVSurface->pPalette[i] = surface->format->palette->colors[i];
			SGPPalette[i].peRed   = surface->format->palette->colors[i].r;
			SGPPalette[i].peGreen = surface->format->palette->colors[i].g;
			SGPPalette[i].peBlue  = surface->format->palette->colors[i].b;
		}
		hVSurface->p16BPPPalette = Create16BPPPalette( SGPPalette );
	}
	else
	{
		hVSurface->pPalette = NULL;
		hVSurface->p16BPPPalette = NULL;
	}
	// Set meory flags
	if (surface->flags & SDL_SWSURFACE)
	{
		hVSurface->fFlags |= VSURFACE_SYSTEM_MEM_USAGE;
	}

	if (surface->flags & SDL_HWSURFACE)
	{
		hVSurface->fFlags |= VSURFACE_VIDEO_MEM_USAGE;
	}

	// All is well
  DbgMessage( TOPIC_VIDEOSURFACE, DBG_LEVEL_0, String("Success in Creating Video Surface from DD Surface" ) );

	return hVSurface;
}

HVSURFACE GetPrimaryVideoSurface( )
{
	return( ghPrimary );
}

HVSURFACE GetBackBufferVideoSurface( )
{
	return( ghBackBuffer );
}

HVSURFACE GetFrameBufferVideoSurface( )
{
	return( ghFrameBuffer );
}

HVSURFACE GetMouseBufferVideoSurface( )
{
	return( ghMouseBuffer );
}


// UTILITY FUNCTIONS FOR BLITTING

BOOLEAN ClipReleatedSrcAndDestRectangles( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, RECT *DestRect, RECT *SrcRect )
{

	Assert( hDestVSurface != NULL );
	Assert( hSrcVSurface != NULL );

	// Check for invalid start positions and clip by ignoring blit
	if ( DestRect->left >= hDestVSurface->usWidth || DestRect->top >= hDestVSurface->usHeight )
	{
		return( FALSE );
	}

	if ( SrcRect->left >= hSrcVSurface->usWidth || SrcRect->top >= hSrcVSurface->usHeight )
	{
		return( FALSE );
	}

	// For overruns
	// Clip destination rectangles
	if ( DestRect->right > hDestVSurface->usWidth )
	{
		// Both have to be modified or by default streching occurs
		DestRect->right = hDestVSurface->usWidth;
		SrcRect->right = SrcRect->left + ( DestRect->right - DestRect->left );
	}
	if ( DestRect->bottom > hDestVSurface->usHeight )
	{
		// Both have to be modified or by default streching occurs
		DestRect->bottom = hDestVSurface->usHeight;
		SrcRect->bottom = SrcRect->top + ( DestRect->bottom - DestRect->top );
	}

	// Clip src rectangles
	if ( SrcRect->right > hSrcVSurface->usWidth )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->right = hSrcVSurface->usWidth;
		DestRect->right = DestRect->left  + ( SrcRect->right - SrcRect->left );
	}
	if ( SrcRect->bottom > hSrcVSurface->usHeight )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->bottom = hSrcVSurface->usHeight;
		DestRect->bottom = DestRect->top + ( SrcRect->bottom - SrcRect->top );
	}

	// For underruns
	// Clip destination rectangles
	if ( DestRect->left < 0 )
	{
		// Both have to be modified or by default streching occurs
		DestRect->left = 0;
		SrcRect->left = SrcRect->right - ( DestRect->right - DestRect->left );
	}
	if ( DestRect->top < 0 )
	{
		// Both have to be modified or by default streching occurs
		DestRect->top = 0;
		SrcRect->top = SrcRect->bottom - ( DestRect->bottom - DestRect->top );
	}

	// Clip src rectangles
	if ( SrcRect->left < 0 )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->left = 0;
		DestRect->left = DestRect->right  - ( SrcRect->right - SrcRect->left );
	}
	if ( SrcRect->top < 0 )
	{
		// Both have to be modified or by default streching occurs
		SrcRect->top = 0;
		DestRect->top = DestRect->bottom - ( SrcRect->bottom - SrcRect->top );
	}

	return( TRUE );
}


BOOLEAN FillSurface( HVSURFACE hDestVSurface, blt_vs_fx *pBltFx )
{
	DDBLTFX				 BlitterFX;

	Assert( hDestVSurface != NULL );
	CHECKF( pBltFx != NULL );

	BlitterFX.dwSize = sizeof( DDBLTFX );
	BlitterFX.dwFillColor = pBltFx->ColorFill;

	DDBltSurface( (LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, NULL, NULL, NULL, DDBLT_COLORFILL, &BlitterFX );

	if ( hDestVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE && !hDestVSurface->fFlags & VSURFACE_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVSurface );
	}

	return( TRUE );
}

BOOLEAN FillSurfaceRect( HVSURFACE hDestVSurface, blt_vs_fx *pBltFx )
{
	SDL_Rect rect;

	Assert( hDestVSurface != NULL );
	CHECKF( pBltFx != NULL );

	rect.x = pBltFx->FillRect.iLeft;
	rect.y = pBltFx->FillRect.iTop;
	rect.w = pBltFx->FillRect.iRight - pBltFx->FillRect.iLeft;
	rect.h = pBltFx->FillRect.iBottom - pBltFx->FillRect.iTop;
	SDL_FillRect(hDestVSurface->surface, &rect, pBltFx->ColorFill);

	if ( hDestVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE && !hDestVSurface->fFlags & VSURFACE_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVSurface );
	}

	return( TRUE );
}


BOOLEAN BltVSurfaceUsingDD( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, RECT *SrcRect )
{
	UINT32		uiDDFlags;
	RECT			DestRect;

  // Blit using the correct blitter
	if ( fBltFlags & VS_BLT_FAST )
	{

		// Validations
		CHECKF( iDestX >= 0 );
		CHECKF( iDestY >= 0 );

		// Default flags
		uiDDFlags = 0;

		// Convert flags into DD flags, ( for transparency use, etc )
		if ( fBltFlags & VS_BLT_USECOLORKEY )
		{
			uiDDFlags |= DDBLTFAST_SRCCOLORKEY;
		}

		// Convert flags into DD flags, ( for transparency use, etc )
		if ( fBltFlags & VS_BLT_USEDESTCOLORKEY )
		{
			uiDDFlags |= DDBLTFAST_DESTCOLORKEY;
		}

		if ( uiDDFlags == 0 )
		{
			// Default here is no colorkey
			uiDDFlags = DDBLTFAST_NOCOLORKEY;
		}

		DDBltFastSurface( (LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, iDestX, iDestY, (LPDIRECTDRAWSURFACE2)hSrcVSurface->pSurfaceData, SrcRect, uiDDFlags );

	}
	else
	{
		// Normal, specialized blit for clipping, etc

		// Default flags
		uiDDFlags = DDBLT_WAIT;

		// Convert flags into DD flags, ( for transparency use, etc )
		if ( fBltFlags & VS_BLT_USECOLORKEY )
		{
			uiDDFlags |= DDBLT_KEYSRC;
		}

		// Setup dest rectangle
		DestRect.top =  (int)iDestY;
		DestRect.left = (int)iDestX;
		DestRect.bottom = (int)iDestY + ( SrcRect->bottom - SrcRect->top );
		DestRect.right = (int)iDestX + ( SrcRect->right - SrcRect->left );

		// Do Clipping of rectangles
		if ( !ClipReleatedSrcAndDestRectangles( hDestVSurface, hSrcVSurface, &DestRect, SrcRect ) )
		{
			// Returns false because dest start is > dest size
			return( TRUE );
		}

		// Check values for 0 size
		if ( DestRect.top == DestRect.bottom || DestRect.right == DestRect.left )
		{
			return( TRUE );
		}

		// Check for -ve values

#if 1 // XXX TODO
		SDL_Rect srcrect;
		SDL_Rect dstrect;

		srcrect.x = SrcRect->left;
		srcrect.y = SrcRect->top;
		srcrect.w = SrcRect->right  - SrcRect->left;
		srcrect.h = SrcRect->bottom - SrcRect->top;
		dstrect.x = DestRect.left;
		dstrect.y = DestRect.top;

		// XXX colour key?
		SDL_BlitSurface(hSrcVSurface->surface, &srcrect, hDestVSurface->surface, &dstrect);
#else
		DDBltSurface( (LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, &DestRect, (LPDIRECTDRAWSURFACE2)hSrcVSurface->pSurfaceData,
							SrcRect, uiDDFlags, NULL );

#endif
	}

	// Update backup surface with new data
	if ( hDestVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE && !hDestVSurface->fFlags & VSURFACE_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVSurface );
	}

	return( TRUE );
}

BOOLEAN Blt16BPPBufferShadowRectAlternateTable(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area);



BOOLEAN InternalShadowVideoSurfaceRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, BOOLEAN fLowPercentShadeTable )
{
	UINT16 *pBuffer;
	UINT32 uiPitch;
	SGPRect   area;
	HVSURFACE hVSurface;


	// CLIP IT!
	// FIRST GET SURFACE

  //
	// Get Video Surface
  //
	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_SHADOWVIDEOSURFACERECT;
	#endif
	CHECKF( GetVideoSurface( &hVSurface, uiDestVSurface ) );

	if ( X1 < 0 )
			X1 = 0;

	if ( X2 < 0 )
			return( FALSE );

	if ( Y2 < 0 )
			return( FALSE );

	if ( Y1 < 0 )
			Y1 = 0;

	if ( X2 >= hVSurface->usWidth )
			X2 = hVSurface->usWidth-1;

	if ( Y2 >= hVSurface->usHeight )
			Y2 = hVSurface->usHeight-1;

	if ( X1 >= hVSurface->usWidth )
			return( FALSE );

	if ( Y1 >= hVSurface->usHeight )
			return( FALSE );

	if (  ( X2 - X1 ) <= 0 )
			return( FALSE );

	if (  ( Y2 - Y1 ) <= 0 )
			return( FALSE );


	area.iTop=Y1;
	area.iBottom=Y2;
	area.iLeft=X1;
	area.iRight=X2;


	// Lock video surface
	pBuffer = (UINT16*)LockVideoSurface( uiDestVSurface, &uiPitch );
 	//UnLockVideoSurface( uiDestVSurface );

	if ( pBuffer == NULL )
	{
		return( FALSE );
	}

	if ( !fLowPercentShadeTable )
	{
		// Now we have the video object and surface, call the shadow function
		if(!Blt16BPPBufferShadowRect(pBuffer, uiPitch, &area))
		{
			// Blit has failed if false returned
			return( FALSE );
		}
	}
	else
	{
		// Now we have the video object and surface, call the shadow function
		if(!Blt16BPPBufferShadowRectAlternateTable(pBuffer, uiPitch, &area))
		{
			// Blit has failed if false returned
			return( FALSE );
		}
	}

	// Mark as dirty if it's the backbuffer
	//if ( uiDestVSurface == BACKBUFFER )
	//{
	//	InvalidateBackbuffer( );
	//}

	UnLockVideoSurface( uiDestVSurface );
	return( TRUE );
}


BOOLEAN ShadowVideoSurfaceRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2)
{
 return( InternalShadowVideoSurfaceRect( uiDestVSurface, X1, Y1, X2, Y2, FALSE ) );
}


BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2)
{
 return( InternalShadowVideoSurfaceRect( uiDestVSurface, X1, Y1, X2, Y2, TRUE ) );
}


//
// BltVSurfaceUsingDDBlt will always use Direct Draw Blt,NOT BltFast
BOOLEAN BltVSurfaceUsingDDBlt( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, RECT *SrcRect, RECT *DestRect )
{
	UINT32		uiDDFlags;

	// Default flags
	uiDDFlags = DDBLT_WAIT;

	// Convert flags into DD flags, ( for transparency use, etc )
	if ( fBltFlags & VS_BLT_USECOLORKEY )
	{
		uiDDFlags |= DDBLT_KEYSRC;
	}

	DDBltSurface( (LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, DestRect, (LPDIRECTDRAWSURFACE2)hSrcVSurface->pSurfaceData,
						SrcRect, uiDDFlags, NULL );

	// Update backup surface with new data
	if ( hDestVSurface->fFlags & VSURFACE_VIDEO_MEM_USAGE && !hDestVSurface->fFlags & VSURFACE_RESERVED_SURFACE )
	{
		UpdateBackupSurface( hDestVSurface );
	}

	return( TRUE );
}


//
// This function will stretch the source image to the size of the dest rect.
//
// If the 2 images are not 16 Bpp, it returns false.
//
BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, SGPRect *SrcRect, SGPRect *DestRect )
{
	HVSURFACE	hDestVSurface;
	HVSURFACE	hSrcVSurface;

	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST;
	#endif
	if( !GetVideoSurface( &hDestVSurface, uiDestVSurface ) )
	{
		return FALSE;
	}
	#ifdef _DEBUG
		gubVSDebugCode = DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC;
	#endif
	if( !GetVideoSurface( &hSrcVSurface, uiSrcVSurface ) )
	{
		return FALSE;
	}

	//if the 2 images are not both 16bpp, return FALSE
	if( (hDestVSurface->ubBitDepth != 16) && (hSrcVSurface->ubBitDepth != 16) )
		return(FALSE);

	if(!BltVSurfaceUsingDDBlt( hDestVSurface, hSrcVSurface, fBltFlags, iDestX, iDestY, (RECT*)SrcRect, (RECT*)DestRect ) )
	{
    //
		// VO Blitter will set debug messages for error conditions
    //

		return( FALSE );
	}

	return( TRUE );
}


BOOLEAN ShadowVideoSurfaceImage( UINT32	uiDestVSurface, HVOBJECT hImageHandle, INT32 iPosX, INT32 iPosY)
{
	//Horizontal shadow
	ShadowVideoSurfaceRect( uiDestVSurface, iPosX+3, iPosY+hImageHandle->pETRLEObject->usHeight, iPosX+hImageHandle->pETRLEObject->usWidth, iPosY+	hImageHandle->pETRLEObject->usHeight+3);

	//vertical shadow
	ShadowVideoSurfaceRect( uiDestVSurface, iPosX+hImageHandle->pETRLEObject->usWidth, iPosY+3, iPosX+hImageHandle->pETRLEObject->usWidth+3, iPosY+	hImageHandle->pETRLEObject->usHeight);
	return( TRUE );
}

BOOLEAN MakeVSurfaceFromVObject(UINT32 uiVObject, UINT16 usSubIndex, UINT32 *puiVSurface)
{
HVOBJECT hSrcVObject;
UINT32 uiVSurface;
VSURFACE_DESC hDesc;

	if(GetVideoObject(&hSrcVObject, uiVObject))
	{
		// ATE: Memset
		memset( &hDesc, 0, sizeof( VSURFACE_DESC ) );
		hDesc.fCreateFlags=VSURFACE_CREATE_DEFAULT;
		hDesc.usWidth=hSrcVObject->pETRLEObject[ usSubIndex ].usWidth;
		hDesc.usHeight=hSrcVObject->pETRLEObject[ usSubIndex ].usHeight;
		hDesc.ubBitDepth=PIXEL_DEPTH;

		if(AddVideoSurface(&hDesc, &uiVSurface))
		{
			if(BltVideoObjectFromIndex(uiVSurface, uiVObject, usSubIndex, 0, 0, VO_BLT_SRCTRANSPARENCY, NULL))
			{
				*puiVSurface=uiVSurface;
				return(TRUE);
			}
			else
				DeleteVideoSurfaceFromIndex(uiVSurface);
		}
	}

	return(FALSE);
}

#ifdef _DEBUG
void CheckValidVSurfaceIndex( UINT32 uiIndex )
{
	BOOLEAN fAssertError = FALSE;
	if( uiIndex == 0xffffffff )
	{ //-1 index -- deleted
		fAssertError = TRUE;
	}
	else if( uiIndex % 2 && uiIndex < 0xfffffff0  )
	{ //odd numbers are reserved for vobjects
		fAssertError = TRUE;
	}

	if( fAssertError )
	{
		UINT8 str[60];
		switch( gubVSDebugCode )
		{
			case DEBUGSTR_SETVIDEOSURFACETRANSPARENCY:
				sprintf( str, "SetVideoSurfaceTransparency" );
				break;
			case DEBUGSTR_ADDVIDEOSURFACEREGION:
				sprintf( str, "AddVideoSurfaceRegion" );
				break;
			case DEBUGSTR_GETVIDEOSURFACEDESCRIPTION:
				sprintf( str, "GetVideoSurfaceDescription" );
				break;
			case DEBUGSTR_BLTVIDEOSURFACE_DST:
				sprintf( str, "BltVideoSurface (dest)" );
				break;
			case DEBUGSTR_BLTVIDEOSURFACE_SRC:
				sprintf( str, "BltVideoSurface (src)" );
				break;
			case DEBUGSTR_COLORFILLVIDEOSURFACEAREA:
				sprintf( str, "ColorFillVideoSurfaceArea" );
				break;
			case DEBUGSTR_SHADOWVIDEOSURFACERECT:
				sprintf( str, "ShadowVideoSurfaceRect" );
				break;
			case DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST:
				sprintf( str, "BltStretchVideoSurface (dest)" );
				break;
			case DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC:
				sprintf( str, "BltStretchVideoSurface (src)" );
				break;
			case DEBUGSTR_DELETEVIDEOSURFACEFROMINDEX:
				sprintf( str, "DeleteVideoSurfaceFromIndex" );
				break;
			case DEBUGSTR_NONE:
			default:
				sprintf( str, "GetVideoSurface" );
				break;
		}
		if( uiIndex == 0xffffffff )
		{
			AssertMsg( 0, String( "Trying to %s with deleted index -1." , str ) );
		}
		else
		{
			AssertMsg( 0, String( "Trying to %s using a VOBJECT ID %d!", str, uiIndex ) );
		}
	}
}
#endif

#ifdef SGP_VIDEO_DEBUGGING
typedef struct DUMPFILENAME
{
	UINT8 str[256];
}DUMPFILENAME;


void DumpVSurfaceInfoIntoFile(const char *filename, BOOLEAN fAppend)
{
	VSURFACE_NODE *curr;
	FILE *fp;
	DUMPFILENAME *pName, *pCode;
	UINT32 *puiCounter;
	UINT8 tempName[ 256 ];
	UINT8 tempCode[ 256 ];
	UINT32 i, uiUniqueID;
	BOOLEAN fFound;
	if( !guiVSurfaceSize )
	{
		return;
	}

	if( fAppend )
	{
		fp = fopen( filename, "a" );
	}
	else
	{
		fp = fopen( filename, "w" );
	}
	Assert( fp );

	//Allocate enough strings and counters for each node.
	pName = (DUMPFILENAME*)MemAlloc( sizeof( DUMPFILENAME ) * guiVSurfaceSize );
	pCode = (DUMPFILENAME*)MemAlloc( sizeof( DUMPFILENAME ) * guiVSurfaceSize );
	memset( pName, 0, sizeof( DUMPFILENAME ) * guiVSurfaceSize );
	memset( pCode, 0, sizeof( DUMPFILENAME ) * guiVSurfaceSize );
	puiCounter = (UINT32*)MemAlloc( 4 * guiVSurfaceSize );
	memset( puiCounter, 0, 4 * guiVSurfaceSize );

	//Loop through the list and record every unique filename and count them
	uiUniqueID = 0;
	curr = gpVSurfaceHead;
	while( curr )
	{
		strcpy( tempName, curr->pName );
		strcpy( tempCode, curr->pCode );
		fFound = FALSE;
		for( i = 0; i < uiUniqueID; i++ )
		{
			if (strcasecmp(tempName, pName[i].str) == 0 && strcasecmp(tempCode, pCode[i].str) == 0)
			{ //same string
				fFound = TRUE;
				(puiCounter[ i ])++;
				break;
			}
		}
		if( !fFound )
		{
			strcpy( pName[i].str, tempName );
			strcpy( pCode[i].str, tempCode );
			(puiCounter[ i ])++;
			uiUniqueID++;
		}
		curr = curr->next;
	}

	//Now dump the info.
	fprintf( fp, "-----------------------------------------------\n" );
	fprintf( fp, "%d unique vSurface names exist in %d VSurfaces\n", uiUniqueID, guiVSurfaceSize );
	fprintf( fp, "-----------------------------------------------\n\n" );
	for( i = 0; i < uiUniqueID; i++ )
	{
		fprintf( fp, "%d occurrences of %s\n", puiCounter[i], pName[i].str );
		fprintf( fp, "%s\n\n", pCode[i].str );
	}
	fprintf( fp, "\n-----------------------------------------------\n\n" );

	//Free all memory associated with this operation.
	MemFree( pName );
	MemFree( pCode );
	MemFree( puiCounter );
	fclose( fp );
}

//Debug wrapper for adding vsurfaces
BOOLEAN _AddAndRecordVSurface(VSURFACE_DESC *VSurfaceDesc, UINT32 *uiIndex, UINT32 uiLineNum, const char *pSourceFile)
{
	UINT16 usLength;
	UINT8 str[256];
	if( !AddStandardVideoSurface( VSurfaceDesc, uiIndex ) )
	{
		return FALSE;
	}

	//record the filename of the vsurface (some are created via memory though)
	usLength = strlen( VSurfaceDesc->ImageFile ) + 1;
	gpVSurfaceTail->pName = (UINT8*)MemAlloc( usLength );
	memset( gpVSurfaceTail->pName, 0, usLength );
	strcpy( gpVSurfaceTail->pName, VSurfaceDesc->ImageFile );

	//record the code location of the calling creating function.
	sprintf( str, "%s -- line(%d)", pSourceFile, uiLineNum );
	usLength = strlen( str ) + 1;
	gpVSurfaceTail->pCode = (UINT8*)MemAlloc( usLength );
	memset( gpVSurfaceTail->pCode, 0, usLength );
	strcpy( gpVSurfaceTail->pCode, str );

	return TRUE;
}

#endif
