#include "Local.h"
#include "SysUtil.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"


SGPVSurface* guiSAVEBUFFER;
SGPVSurface* guiEXTRABUFFER;


BOOLEAN	InitializeGameVideoObjects( )
{
	guiSAVEBUFFER = AddVideoSurface(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH);
	CHECKF(guiSAVEBUFFER != NO_VSURFACE);

	guiEXTRABUFFER = AddVideoSurface(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH);
	CHECKF(guiEXTRABUFFER != NO_VSURFACE);

	return( TRUE );
}
