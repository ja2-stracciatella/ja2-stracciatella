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
#include "Strategic_Movement.h"
#include "VSurface.h"
#include "Video.h"


LoadingScreenID gubLastLoadingScreenID = LOADINGSCREEN_NOTHING;


LoadingScreenID GetLoadScreenID(INT16 const x, INT16 const y, INT8 const z)
{
	bool  const night     = NightTime();
	UINT8 const sector_id = SECTOR(x, y);
	switch (z)
	{
		case 0:
			switch (sector_id)
			{
				case SEC_A2:
				case SEC_B2:
					return night ? LOADINGSCREEN_NIGHTCHITZENA : LOADINGSCREEN_DAYCHITZENA;
				case SEC_A9:
					if (DidGameJustStart()) return LOADINGSCREEN_HELI;
					/* FALLTHROUGH */
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
				case SEC_D2:
				case SEC_D15:
				case SEC_I8:
				case SEC_N4:
					return night ? LOADINGSCREEN_NIGHTSAM : LOADINGSCREEN_DAYSAM;
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
		case LOADINGSCREEN_NOTHING:       filename = LOADSCREENSDIR "/LS_Heli.sti";          break;
		case LOADINGSCREEN_DAYGENERIC:    filename = LOADSCREENSDIR "/LS_DayGeneric.sti";    break;
		case LOADINGSCREEN_DAYTOWN1:      filename = LOADSCREENSDIR "/LS_DayTown1.sti";      break;
		case LOADINGSCREEN_DAYTOWN2:      filename = LOADSCREENSDIR "/LS_DayTown2.sti";      break;
		case LOADINGSCREEN_DAYWILD:       filename = LOADSCREENSDIR "/LS_DayWild.sti";       break;
		case LOADINGSCREEN_DAYTROPICAL:   filename = LOADSCREENSDIR "/LS_DayTropical.sti";   break;
		case LOADINGSCREEN_DAYFOREST:     filename = LOADSCREENSDIR "/LS_DayForest.sti";     break;
		case LOADINGSCREEN_DAYDESERT:     filename = LOADSCREENSDIR "/LS_DayDesert.sti";     break;
		case LOADINGSCREEN_DAYPALACE:     filename = LOADSCREENSDIR "/LS_DayPalace.sti";     break;
		case LOADINGSCREEN_NIGHTGENERIC:  filename = LOADSCREENSDIR "/LS_NightGeneric.sti";  break;
		case LOADINGSCREEN_NIGHTWILD:     filename = LOADSCREENSDIR "/LS_NightWild.sti";     break;
		case LOADINGSCREEN_NIGHTTOWN1:    filename = LOADSCREENSDIR "/LS_NightTown1.sti";    break;
		case LOADINGSCREEN_NIGHTTOWN2:    filename = LOADSCREENSDIR "/LS_NightTown2.sti";    break;
		case LOADINGSCREEN_NIGHTFOREST:   filename = LOADSCREENSDIR "/LS_NightForest.sti";   break;
		case LOADINGSCREEN_NIGHTTROPICAL: filename = LOADSCREENSDIR "/LS_NightTropical.sti"; break;
		case LOADINGSCREEN_NIGHTDESERT:   filename = LOADSCREENSDIR "/LS_NightDesert.sti";   break;
		case LOADINGSCREEN_NIGHTPALACE:   filename = LOADSCREENSDIR "/LS_NightPalace.sti";   break;
		case LOADINGSCREEN_HELI:          filename = LOADSCREENSDIR "/LS_Heli.sti";          break;
		case LOADINGSCREEN_BASEMENT:      filename = LOADSCREENSDIR "/LS_Basement.sti";      break;
		case LOADINGSCREEN_MINE:          filename = LOADSCREENSDIR "/LS_Mine.sti";          break;
		case LOADINGSCREEN_CAVE:          filename = LOADSCREENSDIR "/LS_Cave.sti";          break;
		case LOADINGSCREEN_DAYPINE:       filename = LOADSCREENSDIR "/LS_DayPine.sti";       break;
		case LOADINGSCREEN_NIGHTPINE:     filename = LOADSCREENSDIR "/LS_NightPine.sti";     break;
		case LOADINGSCREEN_DAYMILITARY:   filename = LOADSCREENSDIR "/LS_DayMilitary.sti";   break;
		case LOADINGSCREEN_NIGHTMILITARY: filename = LOADSCREENSDIR "/LS_NightMilitary.sti"; break;
		case LOADINGSCREEN_DAYSAM:        filename = LOADSCREENSDIR "/LS_DaySAM.sti";        break;
		case LOADINGSCREEN_NIGHTSAM:      filename = LOADSCREENSDIR "/LS_NightSAM.sti";      break;
		case LOADINGSCREEN_DAYPRISON:     filename = LOADSCREENSDIR "/LS_DayPrison.sti";     break;
		case LOADINGSCREEN_NIGHTPRISON:   filename = LOADSCREENSDIR "/LS_NightPrison.sti";   break;
		case LOADINGSCREEN_DAYHOSPITAL:   filename = LOADSCREENSDIR "/LS_DayHospital.sti";   break;
		case LOADINGSCREEN_NIGHTHOSPITAL: filename = LOADSCREENSDIR "/LS_NightHospital.sti"; break;
		case LOADINGSCREEN_DAYAIRPORT:    filename = LOADSCREENSDIR "/LS_DayAirport.sti";    break;
		case LOADINGSCREEN_NIGHTAIRPORT:  filename = LOADSCREENSDIR "/LS_NightAirport.sti";  break;
		case LOADINGSCREEN_DAYLAB:        filename = LOADSCREENSDIR "/LS_DayLab.sti";        break;
		case LOADINGSCREEN_NIGHTLAB:      filename = LOADSCREENSDIR "/LS_NightLab.sti";      break;
		case LOADINGSCREEN_DAYOMERTA:     filename = LOADSCREENSDIR "/LS_DayOmerta.sti";     break;
		case LOADINGSCREEN_NIGHTOMERTA:   filename = LOADSCREENSDIR "/LS_NightOmerta.sti";   break;
		case LOADINGSCREEN_DAYCHITZENA:   filename = LOADSCREENSDIR "/LS_DayChitzena.sti";   break;
		case LOADINGSCREEN_NIGHTCHITZENA: filename = LOADSCREENSDIR "/LS_NightChitzena.sti"; break;
		case LOADINGSCREEN_DAYMINE:       filename = LOADSCREENSDIR "/LS_DayMine.sti" ;      break;
		case LOADINGSCREEN_NIGHTMINE:     filename = LOADSCREENSDIR "/LS_NightMine.sti" ;    break;
		case LOADINGSCREEN_DAYBALIME:     filename = LOADSCREENSDIR "/LS_DayBalime.sti" ;    break;
		case LOADINGSCREEN_NIGHTBALIME:   filename = LOADSCREENSDIR "/LS_NightBalime.sti";   break;
		default:                          filename = LOADSCREENSDIR "/LS_Heli.sti";          break;
#endif
	}

	try
	{ // Blit the background image.
		BltVideoSurfaceOnce(FRAME_BUFFER, filename, 0, 0);
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
