#include "Campaign_Types.h"
#include "Debug.h"
#include "Directories.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Loading_Screen.h"
#include "Local.h"
#include "Random.h"
#include "Render_Dirty.h"
#include "StrategicMap.h"
#include "Strategic_Movement.h"
#include "VSurface.h"
#include "Video.h"
#include "UILayout.h"


LoadingScreenID gubLastLoadingScreenID = LOADINGSCREEN_NOTHING;


LoadingScreenID GetLoadScreenID(INT16 const x, INT16 const y, INT8 const z)
{
	bool  const night     = NightTime();
	UINT8 const sector_id = SECTOR(x, y);
	switch (z)
	{
		case 0:
			if (DidGameJustStart()) return LOADINGSCREEN_HELI;
			switch (sector_id)
			{
				case SEC_A2:
				case SEC_B2:
					return night ? LOADINGSCREEN_NIGHTCHITZENA : LOADINGSCREEN_DAYCHITZENA;
				case SEC_A9:
				case SEC_A10:
					return night ? LOADINGSCREEN_NIGHTOMERTA : LOADINGSCREEN_DAYOMERTA;
				case SEC_P3:
					return night ? LOADINGSCREEN_NIGHTPALACE : LOADINGSCREEN_DAYPALACE;
				case SEC_H13: // Military installations
				case SEC_H14:
				case SEC_I13:
				case SEC_N7:
					return night ? LOADINGSCREEN_NIGHTMILITARY : LOADINGSCREEN_DAYMILITARY;
				case SEC_K4:
					return night ? LOADINGSCREEN_NIGHTLAB : LOADINGSCREEN_DAYLAB;
				case SEC_J9:
					return night ? LOADINGSCREEN_NIGHTPRISON : LOADINGSCREEN_DAYPRISON;
				case SEC_F8:
					return night ? LOADINGSCREEN_NIGHTHOSPITAL : LOADINGSCREEN_DAYHOSPITAL;
				case SEC_B13:
				case SEC_N3:
					return night ? LOADINGSCREEN_NIGHTAIRPORT : LOADINGSCREEN_DAYAIRPORT;
				case SEC_L11:
				case SEC_L12:
					return night ? LOADINGSCREEN_NIGHTBALIME : LOADINGSCREEN_DAYBALIME;
				case SEC_H3:
				case SEC_H8:
				case SEC_D4:
					return night ? LOADINGSCREEN_NIGHTMINE : LOADINGSCREEN_DAYMINE;
			}

			if (IsThisSectorASAMSector(x, y, z))
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
#ifndef JA2DEMO
					Assert(false);
#endif
					return night ? LOADINGSCREEN_NIGHTGENERIC : LOADINGSCREEN_DAYGENERIC;
			}

		case 1:
			switch (sector_id)
			{
				case SEC_A10: // Miguel's basement
				case SEC_I13:	// Alma prison dungeon
				case SEC_J9:	// Tixa prison dungeon
				case SEC_K4:	// Orta weapons plant
				case SEC_O3:  // Meduna
#ifdef JA2DEMO
				case SEC_P1:  // Demoville dungeon
#endif
				case SEC_P3:  // Meduna
					return LOADINGSCREEN_BASEMENT;
				default:			// Rest are mines
					return LOADINGSCREEN_MINE;
			}

		case 2:
		case 3:
			// All level 2 and 3 maps are caves.
			return LOADINGSCREEN_CAVE;

		default:
			Assert(false); // Shouldn't ever happen
			return night ? LOADINGSCREEN_NIGHTGENERIC : LOADINGSCREEN_DAYGENERIC;
	}
}


void DisplayLoadScreenWithID(LoadingScreenID const id)
{
	char const* filename;
	switch (id)
	{
#ifdef JA2DEMO
		case LOADINGSCREEN_NOTHING:       filename = INTERFACEDIR "/loadscreen1.sti";        break;
		case LOADINGSCREEN_NIGHTGENERIC:  filename = INTERFACEDIR "/loadscreen2.sti";        break;
		case LOADINGSCREEN_HELI:          filename = INTERFACEDIR "/loadscreen1.sti";        break;
		case LOADINGSCREEN_BASEMENT:      filename = INTERFACEDIR "/loadscreen4.sti";        break;
		case LOADINGSCREEN_CAVE:          filename = INTERFACEDIR "/loadscreen5.sti";        break;
		default:                          filename = INTERFACEDIR "/loadscreen1.sti";        break;
#else
		case LOADINGSCREEN_NOTHING:       filename = LOADSCREENSDIR "/ls_heli.sti";          break;
		case LOADINGSCREEN_DAYGENERIC:    filename = LOADSCREENSDIR "/ls_daygeneric.sti";    break;
		case LOADINGSCREEN_DAYTOWN1:      filename = LOADSCREENSDIR "/ls_daytown1.sti";      break;
		case LOADINGSCREEN_DAYTOWN2:      filename = LOADSCREENSDIR "/ls_daytown2.sti";      break;
		case LOADINGSCREEN_DAYWILD:       filename = LOADSCREENSDIR "/ls_daywild.sti";       break;
		case LOADINGSCREEN_DAYTROPICAL:   filename = LOADSCREENSDIR "/ls_daytropical.sti";   break;
		case LOADINGSCREEN_DAYFOREST:     filename = LOADSCREENSDIR "/ls_dayforest.sti";     break;
		case LOADINGSCREEN_DAYDESERT:     filename = LOADSCREENSDIR "/ls_daydesert.sti";     break;
		case LOADINGSCREEN_DAYPALACE:     filename = LOADSCREENSDIR "/ls_daypalace.sti";     break;
		case LOADINGSCREEN_NIGHTGENERIC:  filename = LOADSCREENSDIR "/ls_nightgeneric.sti";  break;
		case LOADINGSCREEN_NIGHTWILD:     filename = LOADSCREENSDIR "/ls_nightwild.sti";     break;
		case LOADINGSCREEN_NIGHTTOWN1:    filename = LOADSCREENSDIR "/ls_nighttown1.sti";    break;
		case LOADINGSCREEN_NIGHTTOWN2:    filename = LOADSCREENSDIR "/ls_nighttown2.sti";    break;
		case LOADINGSCREEN_NIGHTFOREST:   filename = LOADSCREENSDIR "/ls_nightforest.sti";   break;
		case LOADINGSCREEN_NIGHTTROPICAL: filename = LOADSCREENSDIR "/ls_nighttropical.sti"; break;
		case LOADINGSCREEN_NIGHTDESERT:   filename = LOADSCREENSDIR "/ls_nightdesert.sti";   break;
		case LOADINGSCREEN_NIGHTPALACE:   filename = LOADSCREENSDIR "/ls_nightpalace.sti";   break;
		case LOADINGSCREEN_HELI:          filename = LOADSCREENSDIR "/ls_heli.sti";          break;
		case LOADINGSCREEN_BASEMENT:      filename = LOADSCREENSDIR "/ls_basement.sti";      break;
		case LOADINGSCREEN_MINE:          filename = LOADSCREENSDIR "/ls_mine.sti";          break;
		case LOADINGSCREEN_CAVE:          filename = LOADSCREENSDIR "/ls_cave.sti";          break;
		case LOADINGSCREEN_DAYPINE:       filename = LOADSCREENSDIR "/ls_daypine.sti";       break;
		case LOADINGSCREEN_NIGHTPINE:     filename = LOADSCREENSDIR "/ls_nightpine.sti";     break;
		case LOADINGSCREEN_DAYMILITARY:   filename = LOADSCREENSDIR "/ls_daymilitary.sti";   break;
		case LOADINGSCREEN_NIGHTMILITARY: filename = LOADSCREENSDIR "/ls_nightmilitary.sti"; break;
		case LOADINGSCREEN_DAYSAM:        filename = LOADSCREENSDIR "/ls_daysam.sti";        break;
		case LOADINGSCREEN_NIGHTSAM:      filename = LOADSCREENSDIR "/ls_nightsam.sti";      break;
		case LOADINGSCREEN_DAYPRISON:     filename = LOADSCREENSDIR "/ls_dayprison.sti";     break;
		case LOADINGSCREEN_NIGHTPRISON:   filename = LOADSCREENSDIR "/ls_nightprison.sti";   break;
		case LOADINGSCREEN_DAYHOSPITAL:   filename = LOADSCREENSDIR "/ls_dayhospital.sti";   break;
		case LOADINGSCREEN_NIGHTHOSPITAL: filename = LOADSCREENSDIR "/ls_nighthospital.sti"; break;
		case LOADINGSCREEN_DAYAIRPORT:    filename = LOADSCREENSDIR "/ls_dayairport.sti";    break;
		case LOADINGSCREEN_NIGHTAIRPORT:  filename = LOADSCREENSDIR "/ls_nightairport.sti";  break;
		case LOADINGSCREEN_DAYLAB:        filename = LOADSCREENSDIR "/ls_daylab.sti";        break;
		case LOADINGSCREEN_NIGHTLAB:      filename = LOADSCREENSDIR "/ls_nightlab.sti";      break;
		case LOADINGSCREEN_DAYOMERTA:     filename = LOADSCREENSDIR "/ls_dayomerta.sti";     break;
		case LOADINGSCREEN_NIGHTOMERTA:   filename = LOADSCREENSDIR "/ls_nightomerta.sti";   break;
		case LOADINGSCREEN_DAYCHITZENA:   filename = LOADSCREENSDIR "/ls_daychitzena.sti";   break;
		case LOADINGSCREEN_NIGHTCHITZENA: filename = LOADSCREENSDIR "/ls_nightchitzena.sti"; break;
		case LOADINGSCREEN_DAYMINE:       filename = LOADSCREENSDIR "/ls_daymine.sti" ;      break;
		case LOADINGSCREEN_NIGHTMINE:     filename = LOADSCREENSDIR "/ls_nightmine.sti" ;    break;
		case LOADINGSCREEN_DAYBALIME:     filename = LOADSCREENSDIR "/ls_daybalime.sti" ;    break;
		case LOADINGSCREEN_NIGHTBALIME:   filename = LOADSCREENSDIR "/ls_nightbalime.sti";   break;
		default:                          filename = LOADSCREENSDIR "/ls_heli.sti";          break;
#endif
	}

	try
	{ // Blit the background image.
		BltVideoSurfaceOnce(FRAME_BUFFER, filename, STD_SCREEN_X, STD_SCREEN_Y);
	}
	catch (...)
	{ // Failed to load the file, so use a black screen and print out message.
		SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
		FRAME_BUFFER->Fill(0);
		mprintf(5, 5, L"%hs loadscreen data file not found", filename);
	}

	gubLastLoadingScreenID = id;
	InvalidateScreen();
	ExecuteBaseDirtyRectQueue();
	RefreshScreen();
}
