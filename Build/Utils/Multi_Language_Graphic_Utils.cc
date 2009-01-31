#include <stdlib.h>
#include "Multi_Language_Graphic_Utils.h"


char const* GetMLGFilename(MultiLanguageGraphic const id)
{
	switch (id)
	{
#if defined ENGLISH || defined FRENCH || defined RUSSIAN_GOLD
		case MLG_AIMSYMBOL:          return "LAPTOP/AimSymbol.sti";
		case MLG_BOBBYNAME:          return "LAPTOP/BobbyName.sti";
		case MLG_BOBBYRAYAD21:       return "LAPTOP/BobbyRayAd_21.sti";
		case MLG_BOBBYRAYLINK:       return "LAPTOP/BobbyRayLink.sti";
		case MLG_CLOSED:             return "LAPTOP/Closed.sti";
		case MLG_CONFIRMORDER:       return "LAPTOP/ConfirmOrder.sti";
		case MLG_DESKTOP:            return "LAPTOP/desktop.pcx";
		case MLG_FUNERALAD9:         return "LAPTOP/FuneralAd_9.sti";
		case MLG_GOLDPIECEBUTTONS:   return "INTERFACE/goldpiecebuttons.sti";
		case MLG_HISTORY:            return "LAPTOP/history.sti";
		case MLG_INSURANCEAD10:      return "LAPTOP/insurancead_10.sti";
		case MLG_INSURANCELINK:      return "LAPTOP/insurancelink.sti";
		case MLG_INSURANCETITLE:     return "LAPTOP/largetitle.sti";
		case MLG_LARGEFLORISTSYMBOL: return "LAPTOP/LargeSymbol.sti";
		case MLG_SMALLFLORISTSYMBOL: return "LAPTOP/SmallSymbol.sti";
		case MLG_MCGILLICUTTYS:      return "LAPTOP/McGillicuttys.sti";
		case MLG_MORTUARY:           return "LAPTOP/Mortuary.sti";
		case MLG_MORTUARYLINK:       return "LAPTOP/MortuaryLink.sti";
		case MLG_ORDERGRID:          return "LAPTOP/OrderGrid.sti";
		case MLG_PREBATTLEPANEL:     return "INTERFACE/PreBattlePanel.sti";
		case MLG_SMALLTITLE:         return "LAPTOP/SmallTitle.sti";
		case MLG_STATSBOX:           return "LAPTOP/StatsBox.sti";
		case MLG_STOREPLAQUE:        return "LAPTOP/BobbyStorePlaque.sti";
#	if defined JA2DEMO
		case MLG_TITLETEXT:          return "INTERFACE/titletext.sti";
#	else
		case MLG_TITLETEXT:          return "LOADSCREENS/titletext.sti";
#	endif
		case MLG_TOALUMNI:           return "LAPTOP/ToAlumni.sti";
		case MLG_TOMUGSHOTS:         return "LAPTOP/ToMugShots.sti";
		case MLG_TOSTATS:            return "LAPTOP/ToStats.sti";
		case MLG_WARNING:            return "LAPTOP/Warning.sti";
		case MLG_YOURAD13:           return "LAPTOP/YourAd_13.sti";
		case MLG_OPTIONHEADER:       return "INTERFACE/optionscreenaddons.sti";
		case MLG_LOADSAVEHEADER:     return "INTERFACE/loadscreenaddons.sti";
		case MLG_SPLASH:             return "INTERFACE/splash.sti";
		case MLG_IMPSYMBOL:          return "LAPTOP/IMPSymbol.sti";
#elif defined GERMAN
		case MLG_AIMSYMBOL:          return "LAPTOP/AimSymbol.sti";     // Same graphic (no translation needed)
		case MLG_BOBBYNAME:          return "LAPTOP/BobbyName.sti";     // Same graphic (no translation needed)
		case MLG_BOBBYRAYAD21:       return "LAPTOP/BobbyRayAd_21.sti"; // Same graphic (no translation needed)
		case MLG_BOBBYRAYLINK:       return "GERMAN/BobbyRayLink_german.sti";
		case MLG_CLOSED:             return "GERMAN/Closed_german.sti";
		case MLG_CONFIRMORDER:       return "GERMAN/ConfirmOrder_german.sti";
		case MLG_DESKTOP:            return "GERMAN/desktop_german.pcx";
		case MLG_FUNERALAD9:         return "GERMAN/FuneralAd_12_german.sti";
		case MLG_GOLDPIECEBUTTONS:   return "GERMAN/goldpiecebuttons_german.sti";
		case MLG_HISTORY:            return "GERMAN/history_german.sti";
		case MLG_IMPSYMBOL:          return "GERMAN/IMPSymbol_german.sti";
		case MLG_INSURANCEAD10:      return "GERMAN/insurancead_10_german.sti";
		case MLG_INSURANCELINK:      return "GERMAN/insurancelink_german.sti";
		case MLG_INSURANCETITLE:     return "GERMAN/largetitle_german.sti";
		case MLG_LARGEFLORISTSYMBOL: return "GERMAN/LargeSymbol_german.sti";
		case MLG_SMALLFLORISTSYMBOL: return "GERMAN/SmallSymbol_german.sti";
		case MLG_MCGILLICUTTYS:      return "GERMAN/McGillicuttys_german.sti";
		case MLG_MORTUARY:           return "GERMAN/Mortuary_german.sti";
		case MLG_MORTUARYLINK:       return "GERMAN/MortuaryLink_german.sti";
		case MLG_PREBATTLEPANEL:     return "GERMAN/PreBattlePanel_german.sti";
		case MLG_SMALLTITLE:         return "GERMAN/SmallTitle_german.sti";
		case MLG_STATSBOX:           return "LAPTOP/StatsBox.sti"; // Same file
		case MLG_STOREPLAQUE:        return "GERMAN/StorePlaque_german.sti";
		case MLG_TITLETEXT:          return "GERMAN/titletext_german.sti";
		case MLG_TOALUMNI:           return "GERMAN/ToAlumni_german.sti";
		case MLG_TOMUGSHOTS:         return "GERMAN/ToMugShots_german.sti";
		case MLG_TOSTATS:            return "GERMAN/ToStats_german.sti";
		case MLG_WARNING:            return "GERMAN/Warning_german.sti";
		case MLG_YOURAD13:           return "GERMAN/YourAd_13_german.sti";
		case MLG_OPTIONHEADER:       return "GERMAN/optionscreenaddons_german.sti";
		case MLG_LOADSAVEHEADER:     return "GERMAN/loadscreenaddons_german.sti";
		case MLG_ORDERGRID:          return "LAPTOP/OrderGrid.sti"; // Same file
		case MLG_SPLASH:             return "GERMAN/splash_german.sti";
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
		case MLG_INSURANCEAD10:      return STI("insurancead_10");
		case MLG_INSURANCELINK:      return STI("insurancelink");
		case MLG_INSURANCETITLE:     return STI("largetitle");
		case MLG_LARGEFLORISTSYMBOL: return STI("LargeSymbol");
		case MLG_ORDERGRID:          return STI("OrderGrid");
		case MLG_SMALLFLORISTSYMBOL: return STI("SmallSymbol");
		case MLG_STATSBOX:           return STI("StatsBox");
		case MLG_MCGILLICUTTYS:      return STI("McGillicuttys");
		case MLG_MORTUARY:           return STI("Mortuary");
		case MLG_MORTUARYLINK:       return STI("MortuaryLink");
		case MLG_PREBATTLEPANEL:     return STI("PreBattlePanel");
		case MLG_SMALLTITLE:         return STI("SmallTitle");
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
		case MLG_OPTIONHEADER:       return STI("optionscreenaddons");
		case MLG_LOADSAVEHEADER:     return STI("loadscreenaddons");
		case MLG_SPLASH:             return STI("splash");
		case MLG_IMPSYMBOL:          return STI("IMPSymbol");
#endif
		default:                     abort();
	}
}
