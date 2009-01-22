#include <stdlib.h>
#include "Multi_Language_Graphic_Utils.h"


const char* GetMLGFilename(const MultiLanguageGraphic id)
{
	const char* s;
	switch (id)
	{
#if defined ENGLISH || defined FRENCH || defined RUSSIAN_GOLD
		case MLG_AIMSYMBOL:          s = "LAPTOP/AimSymbol.sti";             break;
		case MLG_BOBBYNAME:          s = "LAPTOP/BobbyName.sti";             break;
		case MLG_BOBBYRAYAD21:       s = "LAPTOP/BobbyRayAd_21.sti";         break;
		case MLG_BOBBYRAYLINK:       s = "LAPTOP/BobbyRayLink.sti";          break;
		case MLG_CLOSED:             s = "LAPTOP/Closed.sti";                break;
		case MLG_CONFIRMORDER:       s = "LAPTOP/ConfirmOrder.sti";          break;
		case MLG_DESKTOP:            s = "LAPTOP/desktop.pcx";               break;
		case MLG_FUNERALAD9:         s = "LAPTOP/FuneralAd_9.sti";           break;
		case MLG_GOLDPIECEBUTTONS:   s = "INTERFACE/goldpiecebuttons.sti";   break;
		case MLG_HISTORY:            s = "LAPTOP/history.sti";               break;
		case MLG_INSURANCEAD10:      s = "LAPTOP/insurancead_10.sti";        break;
		case MLG_INSURANCELINK:      s = "LAPTOP/insurancelink.sti";         break;
		case MLG_INSURANCETITLE:     s = "LAPTOP/largetitle.sti";            break;
		case MLG_LARGEFLORISTSYMBOL: s = "LAPTOP/LargeSymbol.sti";           break;
		case MLG_SMALLFLORISTSYMBOL: s = "LAPTOP/SmallSymbol.sti";           break;
		case MLG_MCGILLICUTTYS:      s = "LAPTOP/McGillicuttys.sti";         break;
		case MLG_MORTUARY:           s = "LAPTOP/Mortuary.sti";              break;
		case MLG_MORTUARYLINK:       s = "LAPTOP/MortuaryLink.sti";          break;
		case MLG_ORDERGRID:          s = "LAPTOP/OrderGrid.sti";             break;
		case MLG_PREBATTLEPANEL:     s = "INTERFACE/PreBattlePanel.sti";     break;
		case MLG_SMALLTITLE:         s = "LAPTOP/SmallTitle.sti";            break;
		case MLG_STATSBOX:           s = "LAPTOP/StatsBox.sti";              break;
		case MLG_STOREPLAQUE:        s = "LAPTOP/BobbyStorePlaque.sti";      break;
#	if defined JA2DEMO
		case MLG_TITLETEXT:          s = "INTERFACE/titletext.sti";          break;
#	else
		case MLG_TITLETEXT:          s = "LOADSCREENS/titletext.sti";        break;
#	endif
		case MLG_TOALUMNI:           s = "LAPTOP/ToAlumni.sti";              break;
		case MLG_TOMUGSHOTS:         s = "LAPTOP/ToMugShots.sti";            break;
		case MLG_TOSTATS:            s = "LAPTOP/ToStats.sti";               break;
		case MLG_WARNING:            s = "LAPTOP/Warning.sti";               break;
		case MLG_YOURAD13:           s = "LAPTOP/YourAd_13.sti";             break;
		case MLG_OPTIONHEADER:       s = "INTERFACE/optionscreenaddons.sti"; break;
		case MLG_LOADSAVEHEADER:     s = "INTERFACE/loadscreenaddons.sti";   break;
		case MLG_SPLASH:             s = "INTERFACE/splash.sti";             break;
		case MLG_IMPSYMBOL:          s = "LAPTOP/IMPSymbol.sti";             break;
#elif defined GERMAN
		case MLG_AIMSYMBOL:          s = "LAPTOP/AimSymbol.sti";                 break; //Same graphic (no translation needed)
		case MLG_BOBBYNAME:          s = "LAPTOP/BobbyName.sti";                 break; //Same graphic (no translation needed)
		case MLG_BOBBYRAYAD21:       s = "LAPTOP/BobbyRayAd_21.sti";             break; //Same graphic (no translation needed)
		case MLG_BOBBYRAYLINK:       s = "GERMAN/BobbyRayLink_german.sti";       break;
		case MLG_CLOSED:             s = "GERMAN/Closed_german.sti";             break;
		case MLG_CONFIRMORDER:       s = "GERMAN/ConfirmOrder_german.sti";       break;
		case MLG_DESKTOP:            s = "GERMAN/desktop_german.pcx";            break;
		case MLG_FUNERALAD9:         s = "GERMAN/FuneralAd_12_german.sti";       break;
		case MLG_GOLDPIECEBUTTONS:   s = "GERMAN/goldpiecebuttons_german.sti";   break;
		case MLG_HISTORY:            s = "GERMAN/history_german.sti";            break;
		case MLG_IMPSYMBOL:          s = "German/IMPSymbol_german.sti";          break;
		case MLG_INSURANCEAD10:      s = "GERMAN/insurancead_10_german.sti";     break;
		case MLG_INSURANCELINK:      s = "GERMAN/insurancelink_german.sti";      break;
		case MLG_INSURANCETITLE:     s = "GERMAN/largetitle_german.sti";         break;
		case MLG_LARGEFLORISTSYMBOL: s = "GERMAN/LargeSymbol_german.sti";        break;
		case MLG_SMALLFLORISTSYMBOL: s = "GERMAN/SmallSymbol_german.sti";        break;
		case MLG_MCGILLICUTTYS:      s = "GERMAN/McGillicuttys_german.sti";      break;
		case MLG_MORTUARY:           s = "GERMAN/Mortuary_german.sti";           break;
		case MLG_MORTUARYLINK:       s = "GERMAN/MortuaryLink_german.sti";       break;
		case MLG_PREBATTLEPANEL:     s = "GERMAN/PreBattlePanel_german.sti";     break;
		case MLG_SMALLTITLE:         s = "GERMAN/SmallTitle_german.sti";         break;
		case MLG_STATSBOX:           s = "LAPTOP/StatsBox.sti";                  break; //Same file
		case MLG_STOREPLAQUE:        s = "GERMAN/StorePlaque_german.sti";        break;
		case MLG_TITLETEXT:          s = "GERMAN/titletext_german.sti";          break;
		case MLG_TOALUMNI:           s = "GERMAN/ToAlumni_german.sti";           break;
		case MLG_TOMUGSHOTS:         s = "GERMAN/ToMugShots_german.sti";         break;
		case MLG_TOSTATS:            s = "GERMAN/ToStats_german.sti";            break;
		case MLG_WARNING:            s = "GERMAN/Warning_german.sti";            break;
		case MLG_YOURAD13:           s = "GERMAN/YourAd_13_german.sti";          break;
		case MLG_OPTIONHEADER:       s = "GERMAN/optionscreenaddons_german.sti"; break;
		case MLG_LOADSAVEHEADER:     s = "GERMAN/loadscreenaddons_german.sti";   break;
		case MLG_ORDERGRID:          s = "LAPTOP/OrderGrid.sti";                 break; //Same file
		case MLG_SPLASH:             s = "German/splash_german.sti";             break;
#else
	//The foreign language defined determines the name of the directory and filename.
	//For example, the German version of:
	//
	//		"LAPTOP/IMPSymbol.sti"
	//
	//	would become:
	//
	//		"GERMAN/IMPSymbol_German.sti"

#if   defined DUTCH
#	define LNG "DUTCH"
#elif defined ITALIAN
#	define LNG "ITALIAN"
#elif defined POLISH
# define LNG "POLISH"
#elif defined RUSSIAN
# define LNG "RUSSIAN"
#else
#	error No supported language chosen
#endif

#define GEN(x, suffix) LNG "/" x "_" LNG suffix
#define STI(x) GEN(x, ".sti")
#define PCX(x) GEN(x, ".pcx")

		case MLG_AIMSYMBOL:          s = STI("AimSymbol");          break;
		case MLG_BOBBYNAME:          s = STI("BobbyName");          break;
		case MLG_BOBBYRAYAD21:       s = STI("BobbyRayAd_21");      break;
		case MLG_BOBBYRAYLINK:       s = STI("BobbyRayLink");       break;
		case MLG_CLOSED:             s = STI("Closed");             break;
		case MLG_CONFIRMORDER:       s = STI("ConfirmOrder");       break;
		case MLG_DESKTOP:            s = PCX("desktop");            break;
		case MLG_FUNERALAD9:         s = STI("FuneralAd_9");        break;
		case MLG_GOLDPIECEBUTTONS:   s = STI("goldpiecebuttons");   break;
		case MLG_HISTORY:            s = STI("history");            break;
		case MLG_INSURANCEAD10:      s = STI("insurancead_10");     break;
		case MLG_INSURANCELINK:      s = STI("insurancelink");      break;
		case MLG_INSURANCETITLE:     s = STI("largetitle");         break;
		case MLG_LARGEFLORISTSYMBOL: s = STI("LargeSymbol");        break;
		case MLG_ORDERGRID:          s = STI("OrderGrid");          break;
		case MLG_SMALLFLORISTSYMBOL: s = STI("SmallSymbol");        break;
		case MLG_STATSBOX:           s = STI("StatsBox");           break;
		case MLG_MCGILLICUTTYS:      s = STI("McGillicuttys");      break;
		case MLG_MORTUARY:           s = STI("Mortuary");           break;
		case MLG_MORTUARYLINK:       s = STI("MortuaryLink");       break;
		case MLG_PREBATTLEPANEL:     s = STI("PreBattlePanel");     break;
		case MLG_SMALLTITLE:         s = STI("SmallTitle");         break;
#if defined RUSSIAN
		case MLG_STOREPLAQUE:        s = STI("BobbyStorePlaque");   break;
#else
		case MLG_STOREPLAQUE:        s = STI("StorePlaque");        break;
#endif
		case MLG_TITLETEXT:          s = STI("titletext");          break;
		case MLG_TOALUMNI:           s = STI("ToAlumni");           break;
		case MLG_TOMUGSHOTS:         s = STI("ToMugShots");         break;
		case MLG_TOSTATS:            s = STI("ToStats");            break;
		case MLG_WARNING:            s = STI("Warning");            break;
		case MLG_YOURAD13:           s = STI("YourAd_13");          break;
		case MLG_OPTIONHEADER:       s = STI("optionscreenaddons"); break;
		case MLG_LOADSAVEHEADER:     s = STI("loadscreenaddons");   break;
		case MLG_SPLASH:             s = STI("splash");             break;
		case MLG_IMPSYMBOL:          s = STI("IMPSymbol");          break;
#endif
		default:                     abort();
	}
	return s;
}
