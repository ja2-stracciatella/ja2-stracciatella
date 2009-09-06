#include <stdlib.h>

#include "Directories.h"
#include "Multi_Language_Graphic_Utils.h"


char const* GetMLGFilename(MultiLanguageGraphic const id)
{
	switch (id)
	{
#if defined ENGLISH || defined FRENCH || defined RUSSIAN_GOLD
		case MLG_AIMSYMBOL:          return LAPTOPDIR "/aimsymbol.sti";
		case MLG_BOBBYNAME:          return LAPTOPDIR "/bobbyname.sti";
		case MLG_BOBBYRAYAD21:       return LAPTOPDIR "/bobbyrayad_21.sti";
		case MLG_BOBBYRAYLINK:       return LAPTOPDIR "/bobbyraylink.sti";
		case MLG_CLOSED:             return LAPTOPDIR "/closed.sti";
		case MLG_CONFIRMORDER:       return LAPTOPDIR "/confirmorder.sti";
		case MLG_DESKTOP:            return LAPTOPDIR "/desktop.pcx";
		case MLG_FUNERALAD9:         return LAPTOPDIR "/funeralad_9.sti";
		case MLG_GOLDPIECEBUTTONS:   return INTERFACEDIR "/goldpiecebuttons.sti";
		case MLG_HISTORY:            return LAPTOPDIR "/history.sti";
		case MLG_IMPSYMBOL:          return LAPTOPDIR "/impsymbol.sti";
		case MLG_INSURANCEAD10:      return LAPTOPDIR "/insurancead_10.sti";
		case MLG_INSURANCELINK:      return LAPTOPDIR "/insurancelink.sti";
		case MLG_INSURANCETITLE:     return LAPTOPDIR "/largetitle.sti";
		case MLG_LARGEFLORISTSYMBOL: return LAPTOPDIR "/largesymbol.sti";
		case MLG_LOADSAVEHEADER:     return INTERFACEDIR "/loadscreenaddons.sti";
		case MLG_MCGILLICUTTYS:      return LAPTOPDIR "/mcgillicuttys.sti";
		case MLG_MORTUARY:           return LAPTOPDIR "/mortuary.sti";
		case MLG_MORTUARYLINK:       return LAPTOPDIR "/mortuarylink.sti";
		case MLG_OPTIONHEADER:       return INTERFACEDIR "/optionscreenaddons.sti";
		case MLG_ORDERGRID:          return LAPTOPDIR "/ordergrid.sti";
		case MLG_PREBATTLEPANEL:     return INTERFACEDIR "/prebattlepanel.sti";
		case MLG_SMALLFLORISTSYMBOL: return LAPTOPDIR "/smallsymbol.sti";
		case MLG_SMALLTITLE:         return LAPTOPDIR "/smalltitle.sti";
		case MLG_SPLASH:             return INTERFACEDIR "/splash.sti";
		case MLG_STATSBOX:           return LAPTOPDIR "/statsbox.sti";
		case MLG_STOREPLAQUE:        return LAPTOPDIR "/bobbystoreplaque.sti";
#	if defined JA2DEMO
		case MLG_TITLETEXT:          return INTERFACEDIR "/titletext.sti";
#	else
		case MLG_TITLETEXT:          return LOADSCREENSDIR "/titletext.sti";
#	endif
		case MLG_TOALUMNI:           return LAPTOPDIR "/toalumni.sti";
		case MLG_TOMUGSHOTS:         return LAPTOPDIR "/tomugshots.sti";
		case MLG_TOSTATS:            return LAPTOPDIR "/tostats.sti";
		case MLG_WARNING:            return LAPTOPDIR "/warning.sti";
		case MLG_YOURAD13:           return LAPTOPDIR "/yourad_13.sti";
#elif defined GERMAN
#	define GERMANDIR "german"
		case MLG_AIMSYMBOL:          return LAPTOPDIR "/aimsymbol.sti";     // Same graphic (no translation needed)
		case MLG_BOBBYNAME:          return LAPTOPDIR "/bobbyname.sti";     // Same graphic (no translation needed)
		case MLG_BOBBYRAYAD21:       return LAPTOPDIR "/bobbyrayad_21.sti"; // Same graphic (no translation needed)
		case MLG_BOBBYRAYLINK:       return GERMANDIR "/bobbyraylink_german.sti";
		case MLG_CLOSED:             return GERMANDIR "/closed_german.sti";
		case MLG_CONFIRMORDER:       return GERMANDIR "/confirmorder_german.sti";
		case MLG_DESKTOP:            return GERMANDIR "/desktop_german.pcx";
		case MLG_FUNERALAD9:         return GERMANDIR "/funeralad_12_german.sti";
		case MLG_GOLDPIECEBUTTONS:   return GERMANDIR "/goldpiecebuttons_german.sti";
		case MLG_HISTORY:            return GERMANDIR "/history_german.sti";
		case MLG_IMPSYMBOL:          return GERMANDIR "/impsymbol_german.sti";
		case MLG_INSURANCEAD10:      return GERMANDIR "/insurancead_10_german.sti";
		case MLG_INSURANCELINK:      return GERMANDIR "/insurancelink_german.sti";
		case MLG_INSURANCETITLE:     return GERMANDIR "/largetitle_german.sti";
		case MLG_LARGEFLORISTSYMBOL: return GERMANDIR "/largesymbol_german.sti";
		case MLG_LOADSAVEHEADER:     return GERMANDIR "/loadscreenaddons_german.sti";
		case MLG_MCGILLICUTTYS:      return GERMANDIR "/mcgillicuttys_german.sti";
		case MLG_MORTUARY:           return GERMANDIR "/mortuary_german.sti";
		case MLG_MORTUARYLINK:       return GERMANDIR "/mortuarylink_german.sti";
		case MLG_OPTIONHEADER:       return GERMANDIR "/optionscreenaddons_german.sti";
		case MLG_ORDERGRID:          return LAPTOPDIR "/ordergrid.sti"; // Same file
		case MLG_PREBATTLEPANEL:     return GERMANDIR "/prebattlepanel_german.sti";
		case MLG_SMALLFLORISTSYMBOL: return GERMANDIR "/smallsymbol_german.sti";
		case MLG_SMALLTITLE:         return GERMANDIR "/smalltitle_german.sti";
		case MLG_SPLASH:             return GERMANDIR "/splash_german.sti";
		case MLG_STATSBOX:           return LAPTOPDIR "/statsbox.sti"; // Same file
		case MLG_STOREPLAQUE:        return GERMANDIR "/storeplaque_german.sti";
		case MLG_TITLETEXT:          return GERMANDIR "/titletext_german.sti";
		case MLG_TOALUMNI:           return GERMANDIR "/toalumni_german.sti";
		case MLG_TOMUGSHOTS:         return GERMANDIR "/tomugshots_german.sti";
		case MLG_TOSTATS:            return GERMANDIR "/tostats_german.sti";
		case MLG_WARNING:            return GERMANDIR "/warning_german.sti";
		case MLG_YOURAD13:           return GERMANDIR "/yourad_13_german.sti";
#else
	//The foreign language defined determines the name of the directory and filename.
	//For example, the German version of:
	//
	//		LAPTOPDIR "/impsymbol.sti"
	//
	//	would become:
	//
	//		LNG "/impsymbol_" LNG ".sti"

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

		case MLG_AIMSYMBOL:          return STI("aimsymbol");
		case MLG_BOBBYNAME:          return STI("bobbyname");
		case MLG_BOBBYRAYAD21:       return STI("bobbyrayad_21");
		case MLG_BOBBYRAYLINK:       return STI("bobbyraylink");
		case MLG_CLOSED:             return STI("closed");
		case MLG_CONFIRMORDER:       return STI("confirmorder");
		case MLG_DESKTOP:            return PCX("desktop");
		case MLG_FUNERALAD9:         return STI("funeralad_9");
		case MLG_GOLDPIECEBUTTONS:   return STI("goldpiecebuttons");
		case MLG_HISTORY:            return STI("history");
		case MLG_IMPSYMBOL:          return STI("impsymbol");
		case MLG_INSURANCEAD10:      return STI("insurancead_10");
		case MLG_INSURANCELINK:      return STI("insurancelink");
		case MLG_INSURANCETITLE:     return STI("largetitle");
		case MLG_LARGEFLORISTSYMBOL: return STI("largesymbol");
		case MLG_LOADSAVEHEADER:     return STI("loadscreenaddons");
		case MLG_MCGILLICUTTYS:      return STI("mcgillicuttys");
		case MLG_MORTUARY:           return STI("mortuary");
		case MLG_MORTUARYLINK:       return STI("mortuarylink");
		case MLG_OPTIONHEADER:       return STI("optionscreenaddons");
		case MLG_ORDERGRID:          return STI("ordergrid");
		case MLG_PREBATTLEPANEL:     return STI("prebattlepanel");
		case MLG_SMALLFLORISTSYMBOL: return STI("smallsymbol");
		case MLG_SMALLTITLE:         return STI("smalltitle");
		case MLG_SPLASH:             return STI("splash");
		case MLG_STATSBOX:           return STI("statsbox");
#if defined RUSSIAN
		case MLG_STOREPLAQUE:        return STI("bobbystoreplaque");
#else
		case MLG_STOREPLAQUE:        return STI("storeplaque");
#endif
		case MLG_TITLETEXT:          return STI("titletext");
		case MLG_TOALUMNI:           return STI("toalumni");
		case MLG_TOMUGSHOTS:         return STI("tomugshots");
		case MLG_TOSTATS:            return STI("tostats");
		case MLG_WARNING:            return STI("warning");
		case MLG_YOURAD13:           return STI("yourad_13");
#endif
		default:                     abort();
	}
}
