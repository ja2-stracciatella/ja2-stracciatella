#include "Debug.h"
#include "DirectDraw_Calls.h"
#include "DirectX_Common.h"
#include "SGP.h"
#include "VObject_Blitters.h"
#include <string.h>


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
