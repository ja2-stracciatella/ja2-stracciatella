#ifndef	__DirectDraw_Calls_H__
#define	__DirectDraw_Calls_H__

#include "DirectX_Common.h"
#include <DDraw.h>

// Direct Draw Functions

#ifdef __cplusplus
extern "C" {
#endif

// Surface Functions

void DDReleaseSurface (	LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2 );


void DDLockSurface( LPDIRECTDRAWSURFACE2 pSurface, LPRECT pDestRect, LPDDSURFACEDESC pSurfaceDesc,
					    UINT32 uiFlags, HANDLE hEvent);

void DDUnlockSurface( LPDIRECTDRAWSURFACE2 pSurface, PTR pSurfaceData );

void DDBltFastSurface( LPDIRECTDRAWSURFACE2 pDestSurface, UINT32 uiX, UINT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface,
							LPRECT pSrcRect, UINT32 uiTrans);

void DDBltSurface( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface,
					    LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx );


HRESULT BltFastDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, INT32 uiX, INT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiTrans );
HRESULT BltDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx );


#define IDirectDrawSurface2_SGPBltFast(p,a,b,c,d,e)         ( ( gfDontUseDDBlits == TRUE ) ?  BltFastDDSurfaceUsingSoftware( p, a, b, c, d, e ) : ( IDirectDrawSurface2_BltFast(p,a,b,c,d,e) ) )
#define IDirectDrawSurface2_SGPBlt(p,a,b,c,d,e)							( ( gfDontUseDDBlits == TRUE ) ?  BltDDSurfaceUsingSoftware( p, a, b, c, d, e ) : ( IDirectDrawSurface2_Blt(p,a,b,c,d,e) ) )


#ifdef __cplusplus
}
#endif


#endif	// __DirectDraw_Calls_H__
