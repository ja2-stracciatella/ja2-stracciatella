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

	CHECKF(AddVideoSurface(usWidth, usHeight, ubBitDepth, &guiSAVEBUFFER));

	CHECKF(AddVideoSurface(usWidth, usHeight, ubBitDepth, &guiEXTRABUFFER));
	gfExtraBuffer = TRUE;

	guiRENDERBUFFER=FRAME_BUFFER;

	return( TRUE );

}
