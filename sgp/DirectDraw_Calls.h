#ifndef	__DirectDraw_Calls_H__
#define	__DirectDraw_Calls_H__

#include "DirectX_Common.h"
#include <ddraw.h>

// Direct Draw Functions

#ifdef __cplusplus
extern "C" {
#endif

// Surface Functions

void DDCreateSurface ( LPDIRECTDRAW2 pExistingDirectDraw,
			DDSURFACEDESC *pNewSurfaceDesc, LPDIRECTDRAWSURFACE *ppNewSurface1, LPDIRECTDRAWSURFACE2 *ppNewSurface2 );
void DDCreateSurfaceInMemory ( LPDIRECTDRAW2 pExistingDirectDraw,
								DDSURFACEDESC *pNewSurfaceDesc,
								BOOLEAN fVideoMemory, LPDIRECTDRAWSURFACE *ppNewSurface1,
								LPDIRECTDRAWSURFACE2 *ppNewSurface2 );
void DDCreateZBufferSurface ( LPDIRECTDRAW2 pDirectDraw, INT32 iWidth, INT32 iHeight,
									  BOOLEAN fVideoMemory, LPDIRECTDRAWSURFACE *ppZBufferSurface1,
									  LPDIRECTDRAWSURFACE2 *ppZBufferSurface2 );
void DDCreateRasterSurface ( LPDIRECTDRAW2 pDirectDraw, INT32 iWidth, INT32 iHeight,
									  BOOLEAN fVideoMemory, LPDIRECTDRAWSURFACE *ppRasterSurface1,
									  LPDIRECTDRAWSURFACE2 *ppRasterSurface2 );
void DDGetSurfaceDescription ( LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc );
void DDGetSurfaceCaps ( LPDIRECTDRAWSURFACE2 pSurface, DDSCAPS *pSurfaceCaps );
void DDAddAttachedSurface ( LPDIRECTDRAWSURFACE2 pParentSurface,
							LPDIRECTDRAWSURFACE2 pAddChildSurface );
void DDDeleteAttachedSurface ( LPDIRECTDRAWSURFACE2 pParentSurface,
							LPDIRECTDRAWSURFACE2 pDeleteChildSurface );
void DDReleaseSurface (	LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2 );

void DDGetDDInterface( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAW *ppDirectDraw );


void DDLockSurface( LPDIRECTDRAWSURFACE2 pSurface, LPRECT pDestRect, LPDDSURFACEDESC pSurfaceDesc,
					    UINT32 uiFlags, HANDLE hEvent);

void DDUnlockSurface( LPDIRECTDRAWSURFACE2 pSurface, PTR pSurfaceData );

void DDRestoreSurface( LPDIRECTDRAWSURFACE2 pSurface );

void DDBltFastSurface( LPDIRECTDRAWSURFACE2 pDestSurface, UINT32 uiX, UINT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface,
							LPRECT pSrcRect, UINT32 uiTrans);

void DDBltSurface( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface,
					    LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx );

void DDSetSurfacePalette( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWPALETTE pDDPalette );

void DDGetSurfacePalette( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWPALETTE *ppDDPalette );

void DDGetDC( LPDIRECTDRAWSURFACE2 pSurface, HDC *phDC );

void DDReleaseDC( LPDIRECTDRAWSURFACE2 pSurface, HDC hDC );

void DDSetSurfaceColorKey( LPDIRECTDRAWSURFACE2 pSurface, UINT32 uiFlags, LPDDCOLORKEY pDDColorKey );


// Palette Functions

void DDCreatePalette( LPDIRECTDRAW2 pDirectDraw, UINT32 uiFlags, LPPALETTEENTRY pColorTable, LPDIRECTDRAWPALETTE FAR *ppDDPalette,
								IUnknown FAR * pUnkOuter);


void DDSetPaletteEntries( LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiStartingEntry,
								UINT32 uiCount, LPPALETTEENTRY pEntries );

void DDReleasePalette( LPDIRECTDRAWPALETTE pPalette );


void DDGetPaletteEntries( LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiBase,
								UINT32 uiNumEntries, LPPALETTEENTRY pEntries );


// Clipper functions
void DDCreateClipper( LPDIRECTDRAW2 pDirectDraw, UINT32 fFlags, LPDIRECTDRAWCLIPPER *pDDClipper );
void DDSetClipper( LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWCLIPPER pDDClipper );
void DDReleaseClipper( LPDIRECTDRAWCLIPPER pDDClipper );
void DDSetClipperList( LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, UINT32 uiFlags);

HRESULT BltFastDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, INT32 uiX, INT32 uiY, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiTrans );
HRESULT BltDDSurfaceUsingSoftware( LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx );


#define IDirectDrawSurface2_SGPBltFast(p,a,b,c,d,e)         ( ( gfDontUseDDBlits == TRUE ) ?  BltFastDDSurfaceUsingSoftware( p, a, b, c, d, e ) : ( IDirectDrawSurface2_BltFast(p,a,b,c,d,e) ) )
#define IDirectDrawSurface2_SGPBlt(p,a,b,c,d,e)							( ( gfDontUseDDBlits == TRUE ) ?  BltDDSurfaceUsingSoftware( p, a, b, c, d, e ) : ( IDirectDrawSurface2_Blt(p,a,b,c,d,e) ) )


#ifdef __cplusplus
}
#endif


#endif	// __DirectDraw_Calls_H__
