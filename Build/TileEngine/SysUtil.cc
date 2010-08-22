#include "Local.h"
#include "SysUtil.h"
#include "VSurface.h"


SGPVSurface* guiSAVEBUFFER;
SGPVSurface* guiEXTRABUFFER;


void InitializeGameVideoObjects()
{
	guiSAVEBUFFER  = AddVideoSurface(g_screen_width, g_screen_height, PIXEL_DEPTH);
	guiEXTRABUFFER = AddVideoSurface(g_screen_width, g_screen_height, PIXEL_DEPTH);
}
