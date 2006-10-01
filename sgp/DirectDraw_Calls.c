#include "Debug.h"
#include "DirectDraw_Calls.h"
#include "DirectX_Common.h"
#include "SGP.h"
#include "VObject_Blitters.h"
#include <string.h>

// DirectDrawSurface2 Calls
void
DDCreateSurface (	LPDIRECTDRAW2 pExistingDirectDraw,
								DDSURFACEDESC *pNewSurfaceDesc,
								LPDIRECTDRAWSURFACE *ppNewSurface1,
								LPDIRECTDRAWSURFACE2 *ppNewSurface2 )
{
#if 1 // XXX TODO
	FIXME
#else
	Assert ( pExistingDirectDraw != NULL );
	Assert ( pNewSurfaceDesc != NULL );
	Assert ( ppNewSurface1 != NULL );
	Assert ( ppNewSurface2 != NULL );

	// create the directdraw surface
	ATTEMPT ( IDirectDraw2_CreateSurface ( pExistingDirectDraw,
						pNewSurfaceDesc, ppNewSurface1, NULL ) );

	//get the direct draw surface 2 interface
	ATTEMPT ( IDirectDrawSurface_QueryInterface ( *ppNewSurface1,	&IID_IDirectDrawSurface2, (LPVOID*) ppNewSurface2 ) );
#endif
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
#if 1 // XXX TODO
	FIXME
#else
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
#endif
}

void DDUnlockSurface( LPDIRECTDRAWSURFACE2 pSurface, PTR pSurfaceData )
{
#if 1 // XXX TODO
	FIXME
#else
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_Unlock( pSurface, pSurfaceData ) );
#endif
}


void DDGetSurfaceDescription ( LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc )
{
#if 1 // XXX TODO
	FIXME
#else
	Assert ( pSurface != NULL );
	Assert ( pSurfaceDesc != NULL );

	ZEROMEM ( *pSurfaceDesc );
	pSurfaceDesc->dwSize = sizeof ( DDSURFACEDESC );

	ATTEMPT ( IDirectDrawSurface2_GetSurfaceDesc ( pSurface, pSurfaceDesc ) );
#endif
}

void DDGetSurfaceCaps ( LPDIRECTDRAWSURFACE2 pSurface, DDSCAPS *pSurfaceCaps )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pSurface != NULL  );
	Assert( pSurfaceCaps != NULL );

	ATTEMPT( IDirectDrawSurface2_GetCaps( pSurface, pSurfaceCaps ) );
#endif
}

void DDCreateRasterSurface ( LPDIRECTDRAW2 pDirectDraw, INT32 iWidth, INT32 iHeight,
									  BOOLEAN fVideoMemory,
									  LPDIRECTDRAWSURFACE *ppRasterSurface1,
									  LPDIRECTDRAWSURFACE2 *ppRasterSurface2 )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
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
#endif
}

void DDCreateZBufferSurface ( LPDIRECTDRAW2 pDirectDraw, INT32 iWidth, INT32 iHeight,
									  BOOLEAN fVideoMemory,
									  LPDIRECTDRAWSURFACE *ppZBufferSurface1,
									  LPDIRECTDRAWSURFACE2 *ppZBufferSurface2 )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
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
#endif
}

void
DDAddAttachedSurface (	LPDIRECTDRAWSURFACE2 pParentSurface,
							LPDIRECTDRAWSURFACE2 pAddChildSurface )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert ( pParentSurface != NULL  );
	Assert ( pAddChildSurface != NULL );

	// attach the child to the parent surface
	ATTEMPT ( IDirectDrawSurface2_AddAttachedSurface ( pParentSurface,
								pAddChildSurface ) );
#endif
}

void
DDDeleteAttachedSurface (	LPDIRECTDRAWSURFACE2 pParentSurface,
							LPDIRECTDRAWSURFACE2 pDeleteChildSurface )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert ( pParentSurface != NULL  );
	Assert ( pDeleteChildSurface != NULL );

	// seperate the z buffer surface from the raster surface
	ATTEMPT ( IDirectDrawSurface2_DeleteAttachedSurface ( pParentSurface,
									0, pDeleteChildSurface ) );
#endif
}

void
DDReleaseSurface ( LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2 )
{
#if 1 // XXX TODO
	FIXME
#else
	Assert ( ppOldSurface1 != NULL );
	Assert ( ppOldSurface2 != NULL );
	Assert ( *ppOldSurface1 != NULL );
	Assert ( *ppOldSurface2 != NULL );

	ATTEMPT ( IDirectDrawSurface2_Release ( *ppOldSurface2 ) );
	ATTEMPT ( IDirectDrawSurface_Release ( *ppOldSurface1 ) );

	*ppOldSurface1 = NULL;
	*ppOldSurface2 = NULL;
#endif
}

void DDRestoreSurface( LPDIRECTDRAWSURFACE2 pSurface )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_Restore( pSurface ) );
#endif
}


void DDBltFastSurface( LPDIRECTDRAWSURFACE2 pDestSurface, UINT32 uiX, UINT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface,
							LPRECT pSrcRect, UINT32 uiTrans)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	HRESULT ReturnCode;

	Assert( pDestSurface != NULL  );
	Assert( pSrcSurface != NULL  );

	do
	{
		ReturnCode = IDirectDrawSurface2_SGPBltFast( pDestSurface, uiX, uiY, pSrcSurface, pSrcRect, uiTrans );

	} while( ReturnCode == DDERR_WASSTILLDRAWING );
#endif
}


void DDBltSurface( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface,
					    LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx )
{
#if 1 // XXX TODO
	FIXME // XXX TODO0001
#else
	HRESULT ReturnCode;

	Assert( pDestSurface != NULL  );

	do
	{
		ReturnCode = IDirectDrawSurface2_SGPBlt( pDestSurface, pDestRect, pSrcSurface, pSrcRect, uiFlags, pDDBltFx );

	} while( ReturnCode == DDERR_WASSTILLDRAWING );

	ATTEMPT( ReturnCode );
#endif
}


void DDCreatePalette( LPDIRECTDRAW2 pDirectDraw, UINT32 uiFlags, LPPALETTEENTRY pColorTable, LPDIRECTDRAWPALETTE FAR *ppDDPalette,
								IUnknown FAR * pUnkOuter)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pDirectDraw != NULL );

	ATTEMPT( IDirectDraw2_CreatePalette( pDirectDraw, uiFlags, pColorTable, ppDDPalette, pUnkOuter ) );
#endif
}

void DDSetSurfacePalette( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWPALETTE pDDPalette )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pDDPalette != NULL  );
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_SetPalette( pSurface, pDDPalette ) );
#endif
}

void DDGetSurfacePalette( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWPALETTE *ppDDPalette )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( ppDDPalette != NULL );
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_GetPalette( pSurface, ppDDPalette ) );
#endif
}

void DDSetPaletteEntries( LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiStartingEntry,
								UINT32 uiCount, LPPALETTEENTRY pEntries )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pPalette != NULL );
	Assert( pEntries != NULL );

	ATTEMPT( IDirectDrawPalette_SetEntries( pPalette, uiFlags, uiStartingEntry, uiCount, pEntries ) );
#endif
}

void DDGetPaletteEntries( LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiBase,
								UINT32 uiNumEntries, LPPALETTEENTRY pEntries )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pPalette != NULL );
	Assert( pEntries != NULL );

	ATTEMPT( IDirectDrawPalette_GetEntries( pPalette, uiFlags, uiBase, uiNumEntries, pEntries ) );
#endif
}

void DDReleasePalette( LPDIRECTDRAWPALETTE pPalette )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pPalette != NULL );

	ATTEMPT( IDirectDrawPalette_Release( pPalette ) );
#endif
}

void DDGetDC( LPDIRECTDRAWSURFACE2 pSurface, HDC *phDC )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pSurface != NULL );
	Assert( phDC != NULL );

	ATTEMPT( IDirectDrawSurface2_GetDC( pSurface, phDC ) );
#endif
}

void DDReleaseDC( LPDIRECTDRAWSURFACE2 pSurface, HDC hDC )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pSurface != NULL );

	ATTEMPT( IDirectDrawSurface2_ReleaseDC( pSurface, hDC ) );
#endif
}

void DDSetSurfaceColorKey( LPDIRECTDRAWSURFACE2 pSurface, UINT32 uiFlags, LPDDCOLORKEY pDDColorKey )
{
#if 1 // XXX TODO
	FIXME
#else
	Assert( pSurface != NULL );
	Assert( pDDColorKey != NULL );

	ATTEMPT( IDirectDrawSurface2_SetColorKey( pSurface, uiFlags, pDDColorKey ) );
#endif
}

void DDGetDDInterface( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAW *ppDirectDraw )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pSurface != NULL );
	Assert( ppDirectDraw != NULL );

	ATTEMPT( IDirectDrawSurface2_GetDDInterface( pSurface, ppDirectDraw ) );
#endif
}

// Clipper FUnctions
void DDCreateClipper( LPDIRECTDRAW2 pDirectDraw, UINT32 fFlags, LPDIRECTDRAWCLIPPER *pDDClipper )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pDirectDraw != NULL );
	Assert( pDDClipper != NULL );

	ATTEMPT( IDirectDraw2_CreateClipper( pDirectDraw, 0, pDDClipper, NULL ) );
#endif
}

void DDSetClipper( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWCLIPPER pDDClipper )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pSurface != NULL );
	Assert( pDDClipper != NULL );

	ATTEMPT( IDirectDrawSurface2_SetClipper( pSurface, pDDClipper ) );
#endif
}

void DDReleaseClipper( LPDIRECTDRAWCLIPPER pDDClipper )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pDDClipper != NULL );

	ATTEMPT( IDirectDrawClipper_Release( pDDClipper ) );
#endif
}

void DDSetClipperList( LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, UINT32 uiFlags)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	Assert( pDDClipper != NULL );
	Assert( pClipList != NULL );

	ATTEMPT( IDirectDrawClipper_SetClipList( pDDClipper, pClipList, uiFlags ) );
#endif
}



HRESULT BltFastDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, INT32 uiX, INT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiTrans )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	DDSURFACEDESC SurfaceDescription;
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;
  HRESULT       ReturnCode;
  DDCOLORKEY    ColorKey;
	UINT16				us16BPPColorKey;


	// Lock surfaces
	DDLockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL, &SurfaceDescription, 0, 0);
	uiDestPitchBYTES = SurfaceDescription.lPitch;
	pDestBuf				 = SurfaceDescription.lpSurface;


	// Lock surfaces
	DDLockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL, &SurfaceDescription, 0, 0);
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
#endif
}


HRESULT BltDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	DDSURFACEDESC SurfaceDescription;
	UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	UINT8	 *pDestBuf, *pSrcBuf;
  HRESULT       ReturnCode;
  DDCOLORKEY    ColorKey;
	UINT16				us16BPPColorKey;


	// Lock surfaces
	DDLockSurface( (LPDIRECTDRAWSURFACE2)pDestSurface, NULL, &SurfaceDescription, 0, 0);
	uiDestPitchBYTES = SurfaceDescription.lPitch;
	pDestBuf				 = SurfaceDescription.lpSurface;


	if ( pSrcSurface != NULL )
	{
		// Lock surfaces
		DDLockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL, &SurfaceDescription, 0, 0);
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
		DDLockSurface( (LPDIRECTDRAWSURFACE2)pSrcSurface, NULL, &SurfaceDescription, 0, 0);
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
#endif
}
