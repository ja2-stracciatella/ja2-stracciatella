#include "game/Local.h"
#include "SysUtil.h"
#include "sgp/VSurface.h"
#include "game/UILayout.h"


SGPVSurface* guiSAVEBUFFER;
SGPVSurface* guiEXTRABUFFER;


void InitializeGameVideoObjects()
{
	guiSAVEBUFFER  = AddVideoSurface(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH);
	guiEXTRABUFFER = AddVideoSurface(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH);
}
