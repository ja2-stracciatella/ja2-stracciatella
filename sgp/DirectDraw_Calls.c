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
