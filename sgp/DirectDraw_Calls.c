#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "DirectX_Common.h"
	#include "DirectDraw_Calls.h"

	#include <DDraw.h>
	#include "Debug.h"
	#include "video_private.h"
#endif

// DirectDrawSurface2 Calls
void
DDCreateSurface (	LPDIRECTDRAW2 pExistingDirectDraw,
								DDSURFACEDESC *pNewSurfaceDesc,
								LPDIRECTDRAWSURFACE *ppNewSurface1,
								LPDIRECTDRAWSURFACE2 *ppNewSurface2 )
{
	Assert ( pExistingDirectDraw != NULL );
	Assert ( pNewSurfaceDesc != NULL );
	Assert ( ppNewSurface1 != NULL );
	Assert ( ppNewSurface2 != NULL );

	// create the directdraw surface
	ATTEMPT ( IDirectDraw2_CreateSurface ( pExistingDirectDraw,
						pNewSurfaceDesc, ppNewSurface1, NULL ) );

	//get the direct draw surface 2 interface
	ATTEMPT ( IDirectDrawSurface_QueryInterface ( *ppNewSurface1,	&IID_IDirectDrawSurface2, (LPVOID*) ppNewSurface2 ) );
}


// DirectDrawSurface2 Calls
void
DDCreateSurfaceInMemory ( LPDIRECTDRAW2 pExistingDirectDraw,
								DDSURFACEDESC *pNewSurfaceDesc,
								BOOLEAN fVideoMemory, LPDIRECTDRAWSURFACE *ppNewSurface1,
								LPDIRECTDRAWSURFACE2 *ppNewSurface2 )
{
	DDSURFACEDESC	DDSurfaceDesc;
	BOOLEAN			fDestination;

	Assert ( pExistingDirectDraw != NULL );
	Assert ( pNewSurfaceDesc != NULL );
	Assert ( ppNewSurface1 != NULL );
	Assert ( ppNewSurface2 != NULL );

	// copy to a local so we don't change the input parameter
	memcpy ( &DDSurfaceDesc, pNewSurfaceDesc, sizeof ( DDSURFACEDESC ) );

	// must have caps set since we are adding to the ddsCaps element
	DDSurfaceDesc.dwFlags |= DDSD_CAPS;

	// If this is a hardware D3D driver, the Z-Buffer MUST end up in video
	// memory.  Otherwise, it MUST end up in system memory.
	if ( fVideoMemory )
		DDSurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	else
		DDSurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	// create the surface
	DDCreateSurface ( pExistingDirectDraw, &DDSurfaceDesc, ppNewSurface1, ppNewSurface2 );

   // get surface information
	DDGetSurfaceDescription ( *ppNewSurface2, &DDSurfaceDesc );

	// was the surface created in video memory?
	fDestination = ( DDSurfaceDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY ) ?
													TRUE : FALSE;

	// did we create the surface in the right memory area?
	if ( fDestination != fVideoMemory )
	{
		// nope we couldn't do it right so release the newly created surface
		// and pass back a NULL pointer to the user
		DDReleaseSurface ( ppNewSurface1, ppNewSurface2 );
	}
}

// Lock, unlock calls
void DDLockSurface ( LPDIRECTDRAWSURFACE2 pSurface, LPRECT pDestRect, LPDDSURFACEDESC pSurfaceDesc, UINT32 uiFlags, HANDLE hEvent )
{
	HRESULT ReturnCode;

	Assert( pSurface != NULL );
	Assert( pSurfaceDesc != NULL );

	ZEROMEM ( *pSurfaceDesc );
	pSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);

	do
	{
		ReturnCode = IDirectDrawSurface2_Lock( pSurface, pDestRect, pSurfaceDesc, uiFlags, hEvent);

	} while( ReturnCode == DDERR_WASSTILLDRAWING );

	ATTEMPT( ReturnCode );

}

void DDUnlockSurface( LPDIRECTDRAWSURFACE2 pSurface, PTR pSurfaceData )
{
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_Unlock( pSurface, pSurfaceData ) );

}


void DDGetSurfaceDescription ( LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc )
{
	Assert ( pSurface != NULL );
	Assert ( pSurfaceDesc != NULL );

	ZEROMEM ( *pSurfaceDesc );
	pSurfaceDesc->dwSize = sizeof ( DDSURFACEDESC );

	ATTEMPT ( IDirectDrawSurface2_GetSurfaceDesc ( pSurface, pSurfaceDesc ) );
}

void DDGetSurfaceCaps ( LPDIRECTDRAWSURFACE2 pSurface, DDSCAPS *pSurfaceCaps )
{
	Assert( pSurface != NULL  );
	Assert( pSurfaceCaps != NULL );

	ATTEMPT( IDirectDrawSurface2_GetCaps( pSurface, pSurfaceCaps ) );

}

void DDCreateRasterSurface ( LPDIRECTDRAW2 pDirectDraw, INT32 iWidth, INT32 iHeight,
									  BOOLEAN fVideoMemory,
									  LPDIRECTDRAWSURFACE *ppRasterSurface1,
									  LPDIRECTDRAWSURFACE2 *ppRasterSurface2 )
{
	DDSURFACEDESC	DDSurfaceDesc;

	// validate used portions of the structure
	Assert ( pDirectDraw != NULL );
	Assert ( iWidth != 0 );
	Assert ( iHeight != 0 );
	Assert ( ppRasterSurface1 != NULL );
	Assert ( ppRasterSurface2 != NULL );

	// create the raster surface
	ZEROMEM ( DDSurfaceDesc );
	DDSurfaceDesc.dwSize		= sizeof ( DDSurfaceDesc );
	DDSurfaceDesc.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	DDSurfaceDesc.dwWidth	= iWidth;
	DDSurfaceDesc.dwHeight	= iHeight;
	DDSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN;
	DDCreateSurfaceInMemory ( pDirectDraw, &DDSurfaceDesc, fVideoMemory, ppRasterSurface1, ppRasterSurface2 );
}

void DDCreateZBufferSurface ( LPDIRECTDRAW2 pDirectDraw, INT32 iWidth, INT32 iHeight,
									  BOOLEAN fVideoMemory,
									  LPDIRECTDRAWSURFACE *ppZBufferSurface1,
									  LPDIRECTDRAWSURFACE2 *ppZBufferSurface2 )
{
	DDSURFACEDESC	DDSurfaceDesc;

	// validate used portions of the structure
	Assert ( pDirectDraw != NULL );
	Assert ( iWidth != 0 );
	Assert ( iHeight != 0 );
	Assert ( ppZBufferSurface1 != NULL );
	Assert ( ppZBufferSurface2 != NULL );

	// create the z buffer
	ZEROMEM ( DDSurfaceDesc );
	DDSurfaceDesc.dwSize		= sizeof ( DDSurfaceDesc );
	DDSurfaceDesc.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;
	DDSurfaceDesc.dwWidth	= iWidth;
	DDSurfaceDesc.dwHeight	= iHeight;
	DDSurfaceDesc.dwZBufferBitDepth	= 16;
	DDSurfaceDesc.ddsCaps.dwCaps		= DDSCAPS_ZBUFFER;
	DDCreateSurfaceInMemory ( pDirectDraw, &DDSurfaceDesc, fVideoMemory, ppZBufferSurface1, ppZBufferSurface2 );
}

void
DDAddAttachedSurface (	LPDIRECTDRAWSURFACE2 pParentSurface,
							LPDIRECTDRAWSURFACE2 pAddChildSurface )
{
	Assert ( pParentSurface != NULL  );
	Assert ( pAddChildSurface != NULL );

	// attach the child to the parent surface
	ATTEMPT ( IDirectDrawSurface2_AddAttachedSurface ( pParentSurface,
								pAddChildSurface ) );
}

void
DDDeleteAttachedSurface (	LPDIRECTDRAWSURFACE2 pParentSurface,
							LPDIRECTDRAWSURFACE2 pDeleteChildSurface )
{
	Assert ( pParentSurface != NULL  );
	Assert ( pDeleteChildSurface != NULL );

	// seperate the z buffer surface from the raster surface
	ATTEMPT ( IDirectDrawSurface2_DeleteAttachedSurface ( pParentSurface,
									0, pDeleteChildSurface ) );
}

void
DDReleaseSurface ( LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2 )
{
	Assert ( ppOldSurface1 != NULL );
	Assert ( ppOldSurface2 != NULL );
	Assert ( *ppOldSurface1 != NULL );
	Assert ( *ppOldSurface2 != NULL );

	ATTEMPT ( IDirectDrawSurface2_Release ( *ppOldSurface2 ) );
	ATTEMPT ( IDirectDrawSurface_Release ( *ppOldSurface1 ) );

	*ppOldSurface1 = NULL;
	*ppOldSurface2 = NULL;
}

void DDRestoreSurface( LPDIRECTDRAWSURFACE2 pSurface )
{
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_Restore( pSurface ) );
}


void DDBltFastSurface( LPDIRECTDRAWSURFACE2 pDestSurface, UINT32 uiX, UINT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface,
							LPRECT pSrcRect, UINT32 uiTrans)
{
	HRESULT ReturnCode;

	Assert( pDestSurface != NULL  );
	Assert( pSrcSurface != NULL  );

	do
	{
		ReturnCode = IDirectDrawSurface2_SGPBltFast( pDestSurface, uiX, uiY, pSrcSurface, pSrcRect, uiTrans );

	} while( ReturnCode == DDERR_WASSTILLDRAWING );
}


void DDBltSurface( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface,
					    LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx )
{

	HRESULT ReturnCode;

	Assert( pDestSurface != NULL  );

	do
	{
		ReturnCode = IDirectDrawSurface2_SGPBlt( pDestSurface, pDestRect, pSrcSurface, pSrcRect, uiFlags, pDDBltFx );

	} while( ReturnCode == DDERR_WASSTILLDRAWING );

	ATTEMPT( ReturnCode );
}


void DDCreatePalette( LPDIRECTDRAW2 pDirectDraw, UINT32 uiFlags, LPPALETTEENTRY pColorTable, LPDIRECTDRAWPALETTE FAR *ppDDPalette,
								IUnknown FAR * pUnkOuter)
{

	Assert( pDirectDraw != NULL );

	ATTEMPT( IDirectDraw2_CreatePalette( pDirectDraw, uiFlags, pColorTable, ppDDPalette, pUnkOuter ) );

}

void DDSetSurfacePalette( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWPALETTE pDDPalette )
{
	Assert( pDDPalette != NULL  );
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_SetPalette( pSurface, pDDPalette ) );

}

void DDGetSurfacePalette( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWPALETTE *ppDDPalette )
{
	Assert( ppDDPalette != NULL );
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_GetPalette( pSurface, ppDDPalette ) );

}

void DDSetPaletteEntries( LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiStartingEntry,
								UINT32 uiCount, LPPALETTEENTRY pEntries )
{
	Assert( pPalette != NULL );
	Assert( pEntries != NULL );

	ATTEMPT( IDirectDrawPalette_SetEntries( pPalette, uiFlags, uiStartingEntry, uiCount, pEntries ) );

}

void DDGetPaletteEntries( LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiBase,
								UINT32 uiNumEntries, LPPALETTEENTRY pEntries )
{
	Assert( pPalette != NULL );
	Assert( pEntries != NULL );

	ATTEMPT( IDirectDrawPalette_GetEntries( pPalette, uiFlags, uiBase, uiNumEntries, pEntries ) );

}

void DDReleasePalette( LPDIRECTDRAWPALETTE pPalette )
{
	Assert( pPalette != NULL );

	ATTEMPT( IDirectDrawPalette_Release( pPalette ) );
}

void DDGetDC( LPDIRECTDRAWSURFACE2 pSurface, HDC *phDC )
{

	Assert( pSurface != NULL );
	Assert( phDC != NULL );

	ATTEMPT( IDirectDrawSurface2_GetDC( pSurface, phDC ) );

}

void DDReleaseDC( LPDIRECTDRAWSURFACE2 pSurface, HDC hDC )
{

	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_ReleaseDC( pSurface, hDC ) );

}

void DDSetSurfaceColorKey( LPDIRECTDRAWSURFACE2 pSurface, UINT32 uiFlags, LPDDCOLORKEY pDDColorKey )
{
	Assert( pSurface != NULL );
	Assert( pDDColorKey != NULL );

	ATTEMPT( IDirectDrawSurface2_SetColorKey( pSurface, uiFlags, pDDColorKey ) );

}

void DDGetDDInterface( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAW *ppDirectDraw )
{
	Assert( pSurface != NULL );
	Assert( ppDirectDraw != NULL );

	ATTEMPT( IDirectDrawSurface2_GetDDInterface( pSurface, ppDirectDraw ) );
}

// Clipper FUnctions
void DDCreateClipper( LPDIRECTDRAW2 pDirectDraw, UINT32 fFlags, LPDIRECTDRAWCLIPPER *pDDClipper )
{
	Assert( pDirectDraw != NULL );
	Assert( pDDClipper != NULL );

	ATTEMPT( IDirectDraw2_CreateClipper( pDirectDraw, 0, pDDClipper, NULL ) );

}

void DDSetClipper( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWCLIPPER pDDClipper )
{
	Assert( pSurface != NULL );
	Assert( pDDClipper != NULL );

	ATTEMPT( IDirectDrawSurface2_SetClipper( pSurface, pDDClipper ) );

}

void DDReleaseClipper( LPDIRECTDRAWCLIPPER pDDClipper )
{
	Assert( pDDClipper != NULL );

	ATTEMPT( IDirectDrawClipper_Release( pDDClipper ) );
}

void DDSetClipperList( LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, UINT32 uiFlags)
{
	Assert( pDDClipper != NULL );
	Assert( pClipList != NULL );

	ATTEMPT( IDirectDrawClipper_SetClipList( pDDClipper, pClipList, uiFlags ) );
}



HRESULT BltFastDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, INT32 uiX, INT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiTrans )
{
	DDSURFACEDESC SurfaceDescription;
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;
  HRESULT       ReturnCode;
  DDCOLORKEY    ColorKey;
	UINT16				us16BPPColorKey;


	// Lock surfaces
	DDLockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL, &SurfaceDescription, 0, NULL);
	uiDestPitchBYTES = SurfaceDescription.lPitch;
	pDestBuf				 = SurfaceDescription.lpSurface;


	// Lock surfaces
	DDLockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL, &SurfaceDescription, 0, NULL);
	uiSrcPitchBYTES = SurfaceDescription.lPitch;
	pSrcBuf				 = SurfaceDescription.lpSurface;

	if ( uiTrans == DDBLTFAST_NOCOLORKEY )
	{
		Blt16BPPTo16BPP( (UINT16 *)pDestBuf, uiDestPitchBYTES,
					(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
					uiX , uiY,
					pSrcRect->left , pSrcRect->top,
					( pSrcRect->right - pSrcRect->left ),
					( pSrcRect->bottom - pSrcRect->top ) );
	}
	else if ( uiTrans == DDBLTFAST_SRCCOLORKEY )
	{
		// Get 16 bpp color key.....
		ReturnCode = IDirectDrawSurface2_GetColorKey( pSrcSurface, DDCKEY_SRCBLT, &ColorKey);

	  if (ReturnCode == DD_OK)
		{
			us16BPPColorKey = (UINT16)ColorKey.dwColorSpaceLowValue;

			Blt16BPPTo16BPPTrans( (UINT16 *)pDestBuf, uiDestPitchBYTES,
						(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
						uiX , uiY,
						pSrcRect->left , pSrcRect->top,
						( pSrcRect->right - pSrcRect->left ),
						( pSrcRect->bottom - pSrcRect->top ), us16BPPColorKey );

		}
	}
	else
	{
		// Not supported.....
	}


	DDUnlockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL );
	DDUnlockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL );

	return( DD_OK );
}


HRESULT BltDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx )
{
	DDSURFACEDESC SurfaceDescription;
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;
  HRESULT       ReturnCode;
  DDCOLORKEY    ColorKey;
	UINT16				us16BPPColorKey;


	// Lock surfaces
	DDLockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL, &SurfaceDescription, 0, NULL);
	uiDestPitchBYTES = SurfaceDescription.lPitch;
	pDestBuf				 = SurfaceDescription.lpSurface;


	if ( pSrcSurface != NULL )
	{
		// Lock surfaces
		DDLockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL, &SurfaceDescription, 0, NULL);
		uiSrcPitchBYTES = SurfaceDescription.lPitch;
		pSrcBuf				 = SurfaceDescription.lpSurface;
	}

	if ( pSrcRect != NULL &&
			 ( ( pSrcRect->right - pSrcRect->left ) != ( pDestRect->right - pDestRect->left ) ||
			 ( pSrcRect->bottom - pSrcRect->top ) != ( pDestRect->bottom - pDestRect->top ) ) )
	{
		DDUnlockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL );

		if ( pSrcSurface != NULL )
		{
			DDUnlockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL );
		}

		// Fall back to DD
		IDirectDrawSurface2_Blt( pDestSurface, pDestRect, pSrcSurface, pSrcRect, uiFlags, pDDBltFx );

		return( DD_OK );
	}
	else if ( uiFlags == DDBLT_WAIT )
	{
		// Lock surfaces
		DDLockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL, &SurfaceDescription, 0, NULL);
		uiSrcPitchBYTES = SurfaceDescription.lPitch;
		pSrcBuf				 = SurfaceDescription.lpSurface;

		Blt16BPPTo16BPP( (UINT16 *)pDestBuf, uiDestPitchBYTES,
					(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
					pDestRect->left , pDestRect->top,
					pSrcRect->left , pSrcRect->top,
					( pSrcRect->right - pSrcRect->left ),
					( pSrcRect->bottom - pSrcRect->top ) );
	}
	else if ( uiFlags & DDBLT_KEYSRC )
	{
		// Get 16 bpp color key.....
		ReturnCode = IDirectDrawSurface2_GetColorKey( pSrcSurface, DDCKEY_SRCBLT, &ColorKey);

	  if (ReturnCode == DD_OK)
		{
			us16BPPColorKey = (UINT16)ColorKey.dwColorSpaceLowValue;

			Blt16BPPTo16BPPTrans( (UINT16 *)pDestBuf, uiDestPitchBYTES,
						(UINT16 *)pSrcBuf, uiSrcPitchBYTES,
						pDestRect->left , pDestRect->top,
						pSrcRect->left , pSrcRect->top,
						( pSrcRect->right - pSrcRect->left ),
						( pSrcRect->bottom - pSrcRect->top ), us16BPPColorKey );

		}
	}
	else if ( uiFlags & DDBLT_COLORFILL )
	{
		// do color fill here...
		FillRect16BPP( (UINT16 *)pDestBuf, uiDestPitchBYTES, pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom, (UINT16)pDDBltFx->dwFillColor );
	}
	else
	{
		// Not supported.....
	}


	DDUnlockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL );

	if ( pSrcSurface != NULL )
	{
		DDUnlockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL );
	}

	return( DD_OK );
}
