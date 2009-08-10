#include <stdlib.h>

#include "Directories.h"
#include "Multi_Language_Graphic_Utils.h"


char const* GetMLGFilename(MultiLanguageGraphic const id)
{
	switch (id)
	{
#if defined ENGLISH || defined FRENCH || defined RUSSIAN_GOLD
		case MLG_AIMSYMBOL:          return LAPTOPDIR "/AimSymbol.sti";
		case MLG_BOBBYNAME:          return LAPTOPDIR "/BobbyName.sti";
		case MLG_BOBBYRAYAD21:       return LAPTOPDIR "/BobbyRayAd_21.sti";
		case MLG_BOBBYRAYLINK:       return LAPTOPDIR "/BobbyRayLink.sti";
		case MLG_CLOSED:             return LAPTOPDIR "/Closed.sti";
		case MLG_CONFIRMORDER:       return LAPTOPDIR "/ConfirmOrder.sti";
		case MLG_DESKTOP:            return LAPTOPDIR "/desktop.pcx";
		case MLG_FUNERALAD9:         return LAPTOPDIR "/FuneralAd_9.sti";
		case MLG_GOLDPIECEBUTTONS:   return INTERFACEDIR "/goldpiecebuttons.sti";
		case MLG_HISTORY:            return LAPTOPDIR "/history.sti";
		case MLG_IMPSYMBOL:          return LAPTOPDIR "/IMPSymbol.sti";
		case MLG_INSURANCEAD10:      return LAPTOPDIR "/insurancead_10.sti";
		case MLG_INSURANCELINK:      return LAPTOPDIR "/insurancelink.sti";
		case MLG_INSURANCETITLE:     return LAPTOPDIR "/largetitle.sti";
		case MLG_LARGEFLORISTSYMBOL: return LAPTOPDIR "/LargeSymbol.sti";
		case MLG_LOADSAVEHEADER:     return INTERFACEDIR "/loadscreenaddons.sti";
		case MLG_MCGILLICUTTYS:      return LAPTOPDIR "/McGillicuttys.sti";
		case MLG_MORTUARY:           return LAPTOPDIR "/Mortuary.sti";
		case MLG_MORTUARYLINK:       return LAPTOPDIR "/MortuaryLink.sti";
		case MLG_OPTIONHEADER:       return INTERFACEDIR "/optionscreenaddons.sti";
		case MLG_ORDERGRID:          return LAPTOPDIR "/OrderGrid.sti";
		case MLG_PREBATTLEPANEL:     return INTERFACEDIR "/PreBattlePanel.sti";
		case MLG_SMALLFLORISTSYMBOL: return LAPTOPDIR "/SmallSymbol.sti";
		case MLG_SMALLTITLE:         return LAPTOPDIR "/SmallTitle.sti";
		case MLG_SPLASH:             return INTERFACEDIR "/splash.sti";
		case MLG_STATSBOX:           return LAPTOPDIR "/StatsBox.sti";
		case MLG_STOREPLAQUE:        return LAPTOPDIR "/BobbyStorePlaque.sti";
#	if defined JA2DEMO
		case MLG_TITLETEXT:          return INTERFACEDIR "/titletext.sti";
#	else
		case MLG_TITLETEXT:          return LOADSCREENSDIR "/titletext.sti";
#	endif
		case MLG_TOALUMNI:           return LAPTOPDIR "/ToAlumni.sti";
		case MLG_TOMUGSHOTS:         return LAPTOPDIR "/ToMugShots.sti";
		case MLG_TOSTATS:            return LAPTOPDIR "/ToStats.sti";
		case MLG_WARNING:            return LAPTOPDIR "/Warning.sti";
		case MLG_YOURAD13:           return LAPTOPDIR "/YourAd_13.sti";
#elif defined GERMAN
#	define GERMANDIR "german"
		case MLG_AIMSYMBOL:          return LAPTOPDIR "/AimSymbol.sti";     // Same graphic (no translation needed)
		case MLG_BOBBYNAME:          return LAPTOPDIR "/BobbyName.sti";     // Same graphic (no translation needed)
		case MLG_BOBBYRAYAD21:       return LAPTOPDIR "/BobbyRayAd_21.sti"; // Same graphic (no translation needed)
		case MLG_BOBBYRAYLINK:       return GERMANDIR "/BobbyRayLink_german.sti";
		case MLG_CLOSED:             return GERMANDIR "/Closed_german.sti";
		case MLG_CONFIRMORDER:       return GERMANDIR "/ConfirmOrder_german.sti";
		case MLG_DESKTOP:            return GERMANDIR "/desktop_german.pcx";
		case MLG_FUNERALAD9:         return GERMANDIR "/FuneralAd_12_german.sti";
		case MLG_GOLDPIECEBUTTONS:   return GERMANDIR "/goldpiecebuttons_german.sti";
		case MLG_HISTORY:            return GERMANDIR "/history_german.sti";
		case MLG_IMPSYMBOL:          return GERMANDIR "/IMPSymbol_german.sti";
		case MLG_INSURANCEAD10:      return GERMANDIR "/insurancead_10_german.sti";
		case MLG_INSURANCELINK:      return GERMANDIR "/insurancelink_german.sti";
		case MLG_INSURANCETITLE:     return GERMANDIR "/largetitle_german.sti";
		case MLG_LARGEFLORISTSYMBOL: return GERMANDIR "/LargeSymbol_german.sti";
		case MLG_LOADSAVEHEADER:     return GERMANDIR "/loadscreenaddons_german.sti";
		case MLG_MCGILLICUTTYS:      return GERMANDIR "/McGillicuttys_german.sti";
		case MLG_MORTUARY:           return GERMANDIR "/Mortuary_german.sti";
		case MLG_MORTUARYLINK:       return GERMANDIR "/MortuaryLink_german.sti";
		case MLG_OPTIONHEADER:       return GERMANDIR "/optionscreenaddons_german.sti";
		case MLG_ORDERGRID:          return LAPTOPDIR "/OrderGrid.sti"; // Same file
		case MLG_PREBATTLEPANEL:     return GERMANDIR "/PreBattlePanel_german.sti";
		case MLG_SMALLFLORISTSYMBOL: return GERMANDIR "/SmallSymbol_german.sti";
		case MLG_SMALLTITLE:         return GERMANDIR "/SmallTitle_german.sti";
		case MLG_SPLASH:             return GERMANDIR "/splash_german.sti";
		case MLG_STATSBOX:           return LAPTOPDIR "/StatsBox.sti"; // Same file
		case MLG_STOREPLAQUE:        return GERMANDIR "/StorePlaque_german.sti";
		case MLG_TITLETEXT:          return GERMANDIR "/titletext_german.sti";
		case MLG_TOALUMNI:           return GERMANDIR "/ToAlumni_german.sti";
		case MLG_TOMUGSHOTS:         return GERMANDIR "/ToMugShots_german.sti";
		case MLG_TOSTATS:            return GERMANDIR "/ToStats_german.sti";
		case MLG_WARNING:            return GERMANDIR "/Warning_german.sti";
		case MLG_YOURAD13:           return GERMANDIR "/YourAd_13_german.sti";
#else
	//The foreign language defined determines the name of the directory and filename.
	//For example, the German version of:
	//
	//		LAPTOPDIR "/IMPSymbol.sti"
	//
	//	would become:
	//
	//		LNG "/IMPSymbol_" LNG ".sti"

#if   defined DUTCH
#	define LNG "dutch"
#elif defined ITALIAN
#	define LNG "italian"
#elif defined POLISH
# define LNG "polish"
#elif defined RUSSIAN
# define LNG "russian"
#else
#	error No supported language chosen
#endif

#define GEN(x, suffix) LNG "/" x "_" LNG suffix
#define STI(x) GEN(x, ".sti")
#define PCX(x) GEN(x, ".pcx")

		case MLG_AIMSYMBOL:          return STI("AimSymbol");
		case MLG_BOBBYNAME:          return STI("BobbyName");
		case MLG_BOBBYRAYAD21:       return STI("BobbyRayAd_21");
		case MLG_BOBBYRAYLINK:       return STI("BobbyRayLink");
		case MLG_CLOSED:             return STI("Closed");
		case MLG_CONFIRMORDER:       return STI("ConfirmOrder");
		case MLG_DESKTOP:            return PCX("desktop");
		case MLG_FUNERALAD9:         return STI("FuneralAd_9");
		case MLG_GOLDPIECEBUTTONS:   return STI("goldpiecebuttons");
		case MLG_HISTORY:            return STI("history");
		case MLG_IMPSYMBOL:          return STI("IMPSymbol");
		case MLG_INSURANCEAD10:      return STI("insurancead_10");
		case MLG_INSURANCELINK:      return STI("insurancelink");
		case MLG_INSURANCETITLE:     return STI("largetitle");
		case MLG_LARGEFLORISTSYMBOL: return STI("LargeSymbol");
		case MLG_LOADSAVEHEADER:     return STI("loadscreenaddons");
		case MLG_MCGILLICUTTYS:      return STI("McGillicuttys");
		case MLG_MORTUARY:           return STI("Mortuary");
		case MLG_MORTUARYLINK:       return STI("MortuaryLink");
		case MLG_OPTIONHEADER:       return STI("optionscreenaddons");
		case MLG_ORDERGRID:          return STI("OrderGrid");
		case MLG_PREBATTLEPANEL:     return STI("PreBattlePanel");
		case MLG_SMALLFLORISTSYMBOL: return STI("SmallSymbol");
		case MLG_SMALLTITLE:         return STI("SmallTitle");
		case MLG_SPLASH:             return STI("splash");
		case MLG_STATSBOX:           return STI("StatsBox");
#if defined RUSSIAN
		case MLG_STOREPLAQUE:        return STI("BobbyStorePlaque");
#else
		case MLG_STOREPLAQUE:        return STI("StorePlaque");
#endif
		case MLG_TITLETEXT:          return STI("titletext");
		case MLG_TOALUMNI:           return STI("ToAlumni");
		case MLG_TOMUGSHOTS:         return STI("ToMugShots");
		case MLG_TOSTATS:            return STI("ToStats");
		case MLG_WARNING:            return STI("Warning");
		case MLG_YOURAD13:           return STI("YourAd_13");
#endif
		default:                     abort();
	}
}
