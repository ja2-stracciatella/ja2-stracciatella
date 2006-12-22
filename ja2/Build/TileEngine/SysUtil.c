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
	VSURFACE_DESC		vs_desc;
	UINT16					usWidth;
	UINT16					usHeight;
	UINT8						ubBitDepth;

	// Create render buffer
	GetCurrentVideoSettings( &usWidth, &usHeight, &ubBitDepth );
	vs_desc.usWidth = usWidth;
	vs_desc.usHeight = usHeight;
	vs_desc.ubBitDepth = ubBitDepth;

	CHECKF( AddVideoSurface( &vs_desc, &guiSAVEBUFFER ) );

	CHECKF( AddVideoSurface( &vs_desc, &guiEXTRABUFFER ) );
	gfExtraBuffer = TRUE;

	guiRENDERBUFFER=FRAME_BUFFER;

	return( TRUE );

}
