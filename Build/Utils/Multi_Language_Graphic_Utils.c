#include "Multi_Language_Graphic_Utils.h"


BOOLEAN GetMLGFilename(SGPFILENAME filename, UINT16 usMLGGraphicID)
{
#if defined ENGLISH || defined FRENCH || defined RUSSIAN_GOLD
	const char* s;
	switch (usMLGGraphicID)
	{
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
		case MLG_TITLETEXT:          s = "LOADSCREENS/titletext.sti";        break;
		case MLG_TOALUMNI:           s = "LAPTOP/ToAlumni.sti";              break;
		case MLG_TOMUGSHOTS:         s = "LAPTOP/ToMugShots.sti";            break;
		case MLG_TOSTATS:            s = "LAPTOP/ToStats.sti";               break;
		case MLG_WARNING:            s = "LAPTOP/Warning.sti";               break;
		case MLG_YOURAD13:           s = "LAPTOP/YourAd_13.sti";             break;
		case MLG_OPTIONHEADER:       s = "INTERFACE/optionscreenaddons.sti"; break;
		case MLG_LOADSAVEHEADER:     s = "INTERFACE/loadscreenaddons.sti";   break;
		case MLG_SPLASH:             s = "INTERFACE/splash.sti";             break;
		case MLG_IMPSYMBOL:          s = "LAPTOP/IMPSymbol.sti";             break;
		default:                     return FALSE;
	}
	strcpy(filename, s);
	return TRUE;

#elif defined(GERMAN)
	const char* s;
	switch (usMLGGraphicID)
	{
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
		default:                     return FALSE;
	}
	strcpy(filename, s);
	return TRUE;

#else
	//The foreign language defined determines the name of the directory and filename.
	//For example, the German version of:
	//
	//		"LAPTOP/IMPSymbol.sti"
	//
	//	would become:
	//
	//		"GERMAN/IMPSymbol_German.sti"

	const char* zLanguage;
#if   defined(DUTCH)
  zLanguage = "DUTCH";
#elif defined(FRENCH)
  zLanguage = "FRENCH";
#elif defined(GERMAN)
  zLanguage = "GERMAN";
#elif defined(ITALIAN)
  zLanguage = "ITALIAN";
#elif defined(JAPANESE)
  zLanguage = "JAPANESE";
#elif defined(KOREAN)
  zLanguage = "KOREAN";
#elif defined(POLISH)
  zLanguage = "POLISH";
#elif defined RUSSIAN || defined RUSSIAN_GOLD
  zLanguage = "RUSSIAN";
#elif defined(SPANISH)
  zLanguage = "SPANISH";
#endif

	const char* s;
	switch (usMLGGraphicID)
	{
		case MLG_AIMSYMBOL:          s = "%s/AimSymbol_%s.sti";          break;
		case MLG_BOBBYNAME:          s = "%s/BobbyName_%s.sti";          break;
		case MLG_BOBBYRAYAD21:       s = "%s/BobbyRayAd_21_%s.sti";      break;
		case MLG_BOBBYRAYLINK:       s = "%s/BobbyRayLink_%s.sti";       break;
		case MLG_CLOSED:             s = "%s/Closed_%s.sti";             break;
		case MLG_CONFIRMORDER:       s = "%s/ConfirmOrder_%s.sti";       break;
		case MLG_DESKTOP:            s = "%s/desktop_%s.pcx";            break;
		case MLG_FUNERALAD9:         s = "%s/FuneralAd_9_%s.sti";        break;
		case MLG_GOLDPIECEBUTTONS:   s = "%s/goldpiecebuttons_%s.sti";   break;
		case MLG_HISTORY:            s = "%s/history_%s.sti";            break;
		case MLG_INSURANCEAD10:      s = "%s/insurancead_10_%s.sti";     break;
		case MLG_INSURANCELINK:      s = "%s/insurancelink_%s.sti";      break;
		case MLG_INSURANCETITLE:     s = "%s/largetitle_%s.sti";         break;
		case MLG_LARGEFLORISTSYMBOL: s = "%s/LargeSymbol_%s.sti";        break;
		case MLG_ORDERGRID:          s = "%s/OrderGrid_%s.sti";          break;
		case MLG_SMALLFLORISTSYMBOL: s = "%s/SmallSymbol_%s.sti";        break;
		case MLG_STATSBOX:           s = "%s/StatsBox_%s.sti";           break;
		case MLG_MCGILLICUTTYS:      s = "%s/McGillicuttys_%s.sti";      break;
		case MLG_MORTUARY:           s = "%s/Mortuary_%s.sti";           break;
		case MLG_MORTUARYLINK:       s = "%s/MortuaryLink_%s.sti";       break;
		case MLG_PREBATTLEPANEL:     s = "%s/PreBattlePanel_%s.sti";     break;
		case MLG_SMALLTITLE:         s = "%s/SmallTitle_%s.sti";         break;
		case MLG_STOREPLAQUE:        s = "%s/StorePlaque_%s.sti";        break;
		case MLG_TITLETEXT:          s = "%s/titletext_%s.sti";          break;
		case MLG_TOALUMNI:           s = "%s/ToAlumni_%s.sti";           break;
		case MLG_TOMUGSHOTS:         s = "%s/ToMugShots_%s.sti";         break;
		case MLG_TOSTATS:            s = "%s/ToStats_%s.sti";            break;
		case MLG_WARNING:            s = "%s/Warning_%s.sti";            break;
		case MLG_YOURAD13:           s = "%s/YourAd_13_%s.sti";          break;
		case MLG_OPTIONHEADER:       s = "%s/optionscreenaddons_%s.sti"; break;
		case MLG_LOADSAVEHEADER:     s = "%s/loadscreenaddons_%s.sti";   break;
		case MLG_SPLASH:             s = "%s/splash_%s.sti";             break;
		case MLG_IMPSYMBOL:          s = "%s/IMPSymbol_%s.sti";          break;
		default:                     return FALSE;
	}
	sprintf(filename, s, zLanguage, zLanguage);
	return TRUE;
#endif
}
