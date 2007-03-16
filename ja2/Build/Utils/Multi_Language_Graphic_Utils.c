#include "Language_Defines.h"
#include "Multi_Language_Graphic_Utils.h"
#include <stdio.h>
#include <string.h>

BOOLEAN GetMLGFilename( SGPFILENAME filename, UINT16 usMLGGraphicID )
{
	#if defined(ENGLISH) || defined(FRENCH)
		switch( usMLGGraphicID )
		{
			case MLG_AIMSYMBOL:
				strcpy(filename, "LAPTOP/AimSymbol.sti");
				return TRUE;
			case MLG_BOBBYNAME:
				strcpy(filename, "LAPTOP/BobbyName.sti");
				return TRUE;
			case MLG_BOBBYRAYAD21:
				strcpy(filename, "LAPTOP/BobbyRayAd_21.sti");
				return TRUE;
			case MLG_BOBBYRAYLINK:
				strcpy(filename, "LAPTOP/BobbyRayLink.sti");
				return TRUE;
			case MLG_CLOSED:
				strcpy(filename, "LAPTOP/Closed.sti");
				return TRUE;
			case MLG_CONFIRMORDER:
				strcpy(filename, "LAPTOP/ConfirmOrder.sti");
				return TRUE;
			case MLG_DESKTOP:
				strcpy(filename, "LAPTOP/desktop.pcx");
				return TRUE;
			case MLG_FUNERALAD9:
				strcpy(filename, "LAPTOP/FuneralAd_9.sti");
				return TRUE;
			case MLG_GOLDPIECEBUTTONS:
				strcpy(filename, "INTERFACE/goldpiecebuttons.sti");
				return TRUE;
			case MLG_HISTORY:
				strcpy(filename, "LAPTOP/history.sti");
				return TRUE;
			case MLG_INSURANCEAD10:
				strcpy(filename, "LAPTOP/insurancead_10.sti");
				return TRUE;
			case MLG_INSURANCELINK:
				strcpy(filename, "LAPTOP/insurancelink.sti");
				return TRUE;
			case MLG_INSURANCETITLE:
				strcpy(filename, "LAPTOP/largetitle.sti");
				return TRUE;
			case MLG_LARGEFLORISTSYMBOL:
				strcpy(filename, "LAPTOP/LargeSymbol.sti");
				return TRUE;
			case MLG_SMALLFLORISTSYMBOL:
				strcpy(filename, "LAPTOP/SmallSymbol.sti");
				return TRUE;
			case MLG_MCGILLICUTTYS:
				strcpy(filename, "LAPTOP/McGillicuttys.sti");
				return TRUE;
			case MLG_MORTUARY:
				strcpy(filename, "LAPTOP/Mortuary.sti");
				return TRUE;
			case MLG_MORTUARYLINK:
				strcpy(filename, "LAPTOP/MortuaryLink.sti");
				return TRUE;
			case MLG_ORDERGRID:
				strcpy(filename, "LAPTOP/OrderGrid.sti");
				return TRUE;
			case MLG_PREBATTLEPANEL:
				strcpy(filename, "INTERFACE/PreBattlePanel.sti");
				return TRUE;
			case MLG_SMALLTITLE:
				strcpy(filename, "LAPTOP/SmallTitle.sti");
				return TRUE;
			case MLG_STATSBOX:
				strcpy(filename, "LAPTOP/StatsBox.sti");
				return TRUE;
			case MLG_STOREPLAQUE:
				strcpy(filename, "LAPTOP/BobbyStorePlaque.sti");
				return TRUE;
			case MLG_TITLETEXT:
				strcpy(filename, "LOADSCREENS/titletext.sti");
				return TRUE;
			case MLG_TOALUMNI:
				strcpy(filename, "LAPTOP/ToAlumni.sti");
				return TRUE;
			case MLG_TOMUGSHOTS:
				strcpy(filename, "LAPTOP/ToMugShots.sti");
				return TRUE;
			case MLG_TOSTATS:
				strcpy(filename, "LAPTOP/ToStats.sti");
				return TRUE;
			case MLG_WARNING:
				strcpy(filename, "LAPTOP/Warning.sti");
				return TRUE;
			case MLG_YOURAD13:
				strcpy(filename, "LAPTOP/YourAd_13.sti");
				return TRUE;
			case MLG_OPTIONHEADER:
				strcpy(filename, "INTERFACE/optionscreenaddons.sti");
				return TRUE;
			case MLG_LOADSAVEHEADER:
				strcpy(filename, "INTERFACE/loadscreenaddons.sti");
				return TRUE;
			case MLG_SPLASH:
				strcpy(filename, "INTERFACE/splash.sti");
				return TRUE;
			case MLG_IMPSYMBOL:
				strcpy(filename, "LAPTOP/IMPSymbol.sti");
				return TRUE;
		}

	#elif defined( GERMAN )
		switch( usMLGGraphicID )
		{
			case MLG_AIMSYMBOL:
				//Same graphic (no translation needed)
				strcpy(filename, "LAPTOP/AimSymbol.sti");
				return TRUE;
			case MLG_BOBBYNAME:
				//Same graphic (no translation needed)
				strcpy(filename, "LAPTOP/BobbyName.sti");
				return TRUE;
			case MLG_BOBBYRAYAD21:
				//Same graphic (no translation needed)
				strcpy(filename, "LAPTOP/BobbyRayAd_21.sti");
				return TRUE;
			case MLG_BOBBYRAYLINK:
				strcpy(filename, "GERMAN/BobbyRayLink_german.sti");
				return TRUE;
			case MLG_CLOSED:
				strcpy(filename, "GERMAN/Closed_german.sti");
				return TRUE;
			case MLG_CONFIRMORDER:
				strcpy(filename, "GERMAN/ConfirmOrder_german.sti");
				return TRUE;
			case MLG_DESKTOP:
				strcpy(filename, "GERMAN/desktop_german.pcx");
				return TRUE;
			case MLG_FUNERALAD9:
				strcpy(filename, "GERMAN/FuneralAd_12_german.sti");
				return TRUE;
			case MLG_GOLDPIECEBUTTONS:
				strcpy(filename, "GERMAN/goldpiecebuttons_german.sti");
				return TRUE;
			case MLG_HISTORY:
				strcpy(filename, "GERMAN/history_german.sti");
				return TRUE;
			case MLG_IMPSYMBOL:
				strcpy(filename, "German/IMPSymbol_german.sti");
				return TRUE;
			case MLG_INSURANCEAD10:
				strcpy(filename, "GERMAN/insurancead_10_german.sti");
				return TRUE;
			case MLG_INSURANCELINK:
				strcpy(filename, "GERMAN/insurancelink_german.sti");
				return TRUE;
			case MLG_INSURANCETITLE:
				strcpy(filename, "GERMAN/largetitle_german.sti");
				return TRUE;
			case MLG_LARGEFLORISTSYMBOL:
				strcpy(filename, "GERMAN/LargeSymbol_german.sti");
				return TRUE;
			case MLG_SMALLFLORISTSYMBOL:
				strcpy(filename, "GERMAN/SmallSymbol_german.sti");
				return TRUE;
			case MLG_MCGILLICUTTYS:
				strcpy(filename, "GERMAN/McGillicuttys_german.sti");
				return TRUE;
			case MLG_MORTUARY:
				strcpy(filename, "GERMAN/Mortuary_german.sti");
				return TRUE;
			case MLG_MORTUARYLINK:
				strcpy(filename, "GERMAN/MortuaryLink_german.sti");
				return TRUE;
			case MLG_PREBATTLEPANEL:
				strcpy(filename, "GERMAN/PreBattlePanel_german.sti");
				return TRUE;
			case MLG_SMALLTITLE:
				strcpy(filename, "GERMAN/SmallTitle_german.sti");
				return TRUE;
			case MLG_STATSBOX:
				//Same file
				strcpy(filename, "LAPTOP/StatsBox.sti");
				return TRUE;
			case MLG_STOREPLAQUE:
				strcpy(filename, "GERMAN/StorePlaque_german.sti");
				return TRUE;
			case MLG_TITLETEXT:
				strcpy(filename, "GERMAN/titletext_german.sti");
				return TRUE;
			case MLG_TOALUMNI:
				strcpy(filename, "GERMAN/ToAlumni_german.sti");
				return TRUE;
			case MLG_TOMUGSHOTS:
				strcpy(filename, "GERMAN/ToMugShots_german.sti");
				return TRUE;
			case MLG_TOSTATS:
				strcpy(filename, "GERMAN/ToStats_german.sti");
				return TRUE;
			case MLG_WARNING:
				strcpy(filename, "GERMAN/Warning_german.sti");
				return TRUE;
			case MLG_YOURAD13:
				strcpy(filename, "GERMAN/YourAd_13_german.sti");
				return TRUE;
			case MLG_OPTIONHEADER:
				strcpy(filename, "GERMAN/optionscreenaddons_german.sti");
				return TRUE;
			case MLG_LOADSAVEHEADER:
				strcpy(filename, "GERMAN/loadscreenaddons_german.sti");
				return TRUE;
			case MLG_ORDERGRID:
				//Same file
				strcpy(filename, "LAPTOP/OrderGrid.sti");
				return TRUE;
			case MLG_SPLASH:
				strcpy(filename, "German/splash_german.sti");
				return TRUE;
		}

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
#if   defined( DUTCH )
	  zLanguage = "DUTCH";
#elif defined( FRENCH )
	  zLanguage = "FRENCH";
#elif defined( GERMAN )
	  zLanguage = "GERMAN";
#elif defined( ITALIAN )
	  zLanguage = "ITALIAN";
#elif defined( JAPANESE )
	  zLanguage = "JAPANESE";
#elif defined( KOREAN )
	  zLanguage = "KOREAN";
#elif defined( POLISH )
	  zLanguage = "POLISH";
#elif defined( RUSSIAN )
	  zLanguage = "RUSSIAN";
#elif defined( SPANISH )
	  zLanguage = "SPANISH";
#endif

		switch( usMLGGraphicID )
		{
			case MLG_AIMSYMBOL:
				sprintf( filename, "%s/AimSymbol_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_BOBBYNAME:
				sprintf( filename, "%s/BobbyName_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_BOBBYRAYAD21:
				sprintf( filename, "%s/BobbyRayAd_21_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_BOBBYRAYLINK:
				sprintf( filename, "%s/BobbyRayLink_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_CLOSED:
				sprintf( filename, "%s/Closed_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_CONFIRMORDER:
				sprintf( filename, "%s/ConfirmOrder_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_DESKTOP:
				sprintf( filename, "%s/desktop_%s.pcx", zLanguage, zLanguage );
				return TRUE;
			case MLG_FUNERALAD9:
				sprintf( filename, "%s/FuneralAd_9_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_GOLDPIECEBUTTONS:
				sprintf( filename, "%s/goldpiecebuttons_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_HISTORY:
				sprintf( filename, "%s/history_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_INSURANCEAD10:
				sprintf( filename, "%s/insurancead_10_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_INSURANCELINK:
				sprintf( filename, "%s/insurancelink_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_INSURANCETITLE:
				sprintf( filename, "%s/largetitle_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_LARGEFLORISTSYMBOL:
				sprintf( filename, "%s/LargeSymbol_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_ORDERGRID:
				sprintf( filename, "%s/OrderGrid_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_SMALLFLORISTSYMBOL:
				sprintf( filename, "%s/SmallSymbol_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_STATSBOX:
				sprintf( filename, "%s/StatsBox_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_MCGILLICUTTYS:
				sprintf( filename, "%s/McGillicuttys_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_MORTUARY:
				sprintf( filename, "%s/Mortuary_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_MORTUARYLINK:
				sprintf( filename, "%s/MortuaryLink_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_PREBATTLEPANEL:
				sprintf( filename, "%s/PreBattlePanel_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_SMALLTITLE:
				sprintf( filename, "%s/SmallTitle_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_STOREPLAQUE:
				sprintf( filename, "%s/StorePlaque_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_TITLETEXT:
				sprintf( filename, "%s/titletext_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_TOALUMNI:
				sprintf( filename, "%s/ToAlumni_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_TOMUGSHOTS:
				sprintf( filename, "%s/ToMugShots_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_TOSTATS:
				sprintf( filename, "%s/ToStats_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_WARNING:
				sprintf( filename, "%s/Warning_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_YOURAD13:
				sprintf( filename, "%s/YourAd_13_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_OPTIONHEADER:
				sprintf( filename, "%s/optionscreenaddons_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_LOADSAVEHEADER:
				sprintf( filename, "%s/loadscreenaddons_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_SPLASH:
				sprintf( filename, "%s/splash_%s.sti", zLanguage, zLanguage );
				return TRUE;
			case MLG_IMPSYMBOL:
				sprintf( filename, "%s/IMPSymbol_%s.sti", zLanguage, zLanguage );
				return TRUE;
		}
	#endif

	return FALSE;
}
