#ifndef	__DirectDraw_Calls_H__
#define	__DirectDraw_Calls_H__

#include "DirectX_Common.h"
#include <DDraw.h>

// Direct Draw Functions

#ifdef __cplusplus
extern "C" {
#endif

// Surface Functions

void DDLockSurface( LPDIRECTDRAWSURFACE2 pSurface, LPRECT pDestRect, LPDDSURFACEDESC pSurfaceDesc,
					    UINT32 uiFlags, HANDLE hEvent);

void DDUnlockSurface( LPDIRECTDRAWSURFACE2 pSurface, PTR pSurfaceData );


#ifdef __cplusplus
}
#endif


#endif	// __DirectDraw_Calls_H__
