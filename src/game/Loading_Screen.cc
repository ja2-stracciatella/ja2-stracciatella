#include "Loading_Screen.h"

#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Debug.h"
#include "Directories.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "GameInstance.h"
#include "LoadingScreenModel.h"
#include "Local.h"
#include "Random.h"
#include "StrategicMap.h"
#include "Strategic_Movement.h"
#include "VSurface.h"
#include "Video.h"
#include "UILayout.h"

#include <string_theory/format>


UINT8 gubLastLoadingScreenID = LOADINGSCREEN_NOTHING;


UINT8 GetLoadScreenID(const SGPSector& sector)
{
	bool  const night     = NightTime();
	UINT8 const sector_id = sector.AsByte();
	
	if (DidGameJustStart()) return LOADINGSCREEN_HELI;

	const LoadingScreen* screen = GCM->getLoadingScreenForSector(sector_id, sector.z, night);
	if (screen != NULL)
	{
		return screen->index;
	}

	switch (sector.z)
	{
		case 0:
			if (IsThisSectorASAMSector(sector))
			{
				return night ? LOADINGSCREEN_NIGHTSAM : LOADINGSCREEN_DAYSAM;
			}

			switch (SectorInfo[sector_id].ubTraversability[THROUGH_STRATEGIC_MOVE])
			{
				case TOWN:
					return Random(2) == 0 ?
						(night ? LOADINGSCREEN_NIGHTTOWN1 : LOADINGSCREEN_DAYTOWN1) :
						(night ? LOADINGSCREEN_NIGHTTOWN2 : LOADINGSCREEN_DAYTOWN2);
				case SAND:
				case SAND_ROAD:
					return night ? LOADINGSCREEN_NIGHTDESERT : LOADINGSCREEN_DAYDESERT;
				case FARMLAND:
				case FARMLAND_ROAD:
				case ROAD:
					return night ? LOADINGSCREEN_NIGHTGENERIC : LOADINGSCREEN_DAYGENERIC;
				case PLAINS:
				case SPARSE:
				case HILLS:
				case PLAINS_ROAD:
				case SPARSE_ROAD:
				case HILLS_ROAD:
					return night ? LOADINGSCREEN_NIGHTWILD : LOADINGSCREEN_DAYWILD;
				case DENSE:
				case SWAMP:
				case SWAMP_ROAD:
				case DENSE_ROAD:
					return night ? LOADINGSCREEN_NIGHTFOREST : LOADINGSCREEN_DAYFOREST;
				case TROPICS:
				case TROPICS_ROAD:
				case WATER:
				case NS_RIVER:
				case EW_RIVER:
				case COASTAL:
				case COASTAL_ROAD:
					return night ? LOADINGSCREEN_NIGHTTROPICAL : LOADINGSCREEN_DAYTROPICAL;
				default:
					Assert(false);
					return night ? LOADINGSCREEN_NIGHTGENERIC : LOADINGSCREEN_DAYGENERIC;
			}

		case 1:
			// All sectors at this level, except mine sectors, are mapped to specific loading screens
			return LOADINGSCREEN_MINE;

		case 2:
		case 3:
			// All level 2 and 3 maps are caves.
			return LOADINGSCREEN_CAVE;

		default:
			Assert(false); // Shouldn't ever happen
			return night ? LOADINGSCREEN_NIGHTGENERIC : LOADINGSCREEN_DAYGENERIC;
	}
}


void DisplayLoadScreenWithID(UINT8 const id)
{
	const LoadingScreen* screen = GCM->getLoadingScreen(id);
	ST::string filename = LOADSCREENSDIR + screen->filename;

	try
	{ // Blit the background image.
		BltVideoSurfaceOnce(FRAME_BUFFER, filename.c_str(), STD_SCREEN_X, STD_SCREEN_Y);
	}
	catch (...)
	{ // Failed to load the file, so use a black screen and print out message.
		SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
		FRAME_BUFFER->Fill(0);
		MPrint(5, 5, ST::format("{} loadscreen data file not found", filename));
	}

	gubLastLoadingScreenID = id;
	InvalidateScreen();
	RefreshScreen();
}
