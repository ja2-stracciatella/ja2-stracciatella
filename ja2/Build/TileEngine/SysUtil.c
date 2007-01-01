#include "SysUtil.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"

UINT32					guiRENDERBUFFER = 0;
UINT32					guiSAVEBUFFER = 0;
UINT32					guiEXTRABUFFER = 0;

BOOLEAN gfExtraBuffer = FALSE;


BOOLEAN	InitializeGameVideoObjects( )
{
	UINT16					usWidth;
	UINT16					usHeight;
	UINT8						ubBitDepth;

	// Create render buffer
	GetCurrentVideoSettings( &usWidth, &usHeight, &ubBitDepth );

	guiSAVEBUFFER = AddVideoSurface(usWidth, usHeight, ubBitDepth);
	CHECKF(guiSAVEBUFFER != NO_VSURFACE);

	guiEXTRABUFFER = AddVideoSurface(usWidth, usHeight, ubBitDepth);
	CHECKF(guiEXTRABUFFER != NO_VSURFACE);
	gfExtraBuffer = TRUE;

	guiRENDERBUFFER=FRAME_BUFFER;

	return( TRUE );

}
