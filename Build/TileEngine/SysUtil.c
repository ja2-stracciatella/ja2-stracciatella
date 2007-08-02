#include "Local.h"
#include "SysUtil.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"

UINT32					guiSAVEBUFFER = 0;
UINT32					guiEXTRABUFFER = 0;

BOOLEAN gfExtraBuffer = FALSE;


BOOLEAN	InitializeGameVideoObjects( )
{
	guiSAVEBUFFER = AddVideoSurface(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH);
	CHECKF(guiSAVEBUFFER != NO_VSURFACE);

	guiEXTRABUFFER = AddVideoSurface(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH);
	CHECKF(guiEXTRABUFFER != NO_VSURFACE);
	gfExtraBuffer = TRUE;

	return( TRUE );
}
