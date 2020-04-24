#include "GameRes.h"

#include <locale.h>
#include <stdexcept>

#include "sgp/FileMan.h"

#include "Directories.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Text.h"
#include "TranslationTable.h"
#include "GameState.h"
#include "EncodingCorrectors.h"
#include "StrUtils.h"

#include "Logger.h"

extern LanguageRes g_LanguageResDutch;
extern LanguageRes g_LanguageResEnglish;
extern LanguageRes g_LanguageResFrench;
extern LanguageRes g_LanguageResGerman;
extern LanguageRes g_LanguageResItalian;
extern LanguageRes g_LanguageResPolish;
extern LanguageRes g_LanguageResRussian;
extern LanguageRes g_LanguageResRussianGold;


/** Game version. */
static GameVersion s_gameVersion = GameVersion::ENGLISH;

/** Current language resources. */
const LanguageRes* g_langRes = &g_LanguageResEnglish;

/** Character->Glyph translation table for the current language. */
unsigned char const *TranslationTable = g_en_TranslationTable->m_table;


char32_t getZeroGlyphChar()
{
	if(s_gameVersion == GameVersion::RUSSIAN)
	{
		return U' ';
	}
	else
	{
		return U'A';
	}
}


/** Set language resources. */
static void setResources(const LanguageRes* langRes, const CTranslationTable *translationTable)
{
	g_langRes = langRes;
	TranslationTable = translationTable->m_table;
}


/** Choose game version. */
void setGameVersion(GameVersion ver)
{
	s_gameVersion = ver;
	switch(s_gameVersion)
	{
		case GameVersion::DUTCH:        setResources(&g_LanguageResDutch,             g_en_TranslationTable     ); break;
		case GameVersion::ENGLISH:      setResources(&g_LanguageResEnglish,           g_en_TranslationTable     ); break;
		case GameVersion::FRENCH:       setResources(&g_LanguageResFrench,            g_fr_TranslationTable     ); break;
		case GameVersion::GERMAN:       setResources(&g_LanguageResGerman,            g_en_TranslationTable     ); break;
		case GameVersion::ITALIAN:      setResources(&g_LanguageResItalian,           g_en_TranslationTable     ); break;
		case GameVersion::POLISH:       setResources(&g_LanguageResPolish,            g_en_TranslationTable     ); break;
		case GameVersion::RUSSIAN:      setResources(&g_LanguageResRussian,           g_rus_TranslationTable    ); break;
		case GameVersion::RUSSIAN_GOLD: setResources(&g_LanguageResRussianGold,       g_rusGold_TranslationTable); break;
		default:
		{
			SLOGW("Unknown version. Using ENGLISH by defaul");
			s_gameVersion = GameVersion::ENGLISH;
			setResources(&g_LanguageResEnglish, g_en_TranslationTable);
		}
	}
}


/** Check if this is English version of the game. */
bool isEnglishVersion()
{
	return s_gameVersion == GameVersion::ENGLISH;
}


/** Check if this is German version of the game. */
bool isGermanVersion()
{
	return s_gameVersion == GameVersion::GERMAN;
}


/** Check if this is Polish version of the game. */
bool isPolishVersion()
{
	return s_gameVersion == GameVersion::POLISH;
}


/** Check if this is Russian version of the game. */
bool isRussianVersion()
{
	return s_gameVersion == GameVersion::RUSSIAN;
}


/** Check if this is Russian GOLD version of the game. */
bool isRussianGoldVersion()
{
	return s_gameVersion == GameVersion::RUSSIAN_GOLD;
}


/**
 * Get encoding corrector for strings in data files.
 * @return NULL when no encoding corrector is required */
const IEncodingCorrector* getDataFilesEncodingCorrector()
{
	static RussianEncodingCorrector s_russianFixer;
	if(isRussianVersion() || isRussianGoldVersion())
	{
		return &s_russianFixer;
	}
	return NULL;
}

/** Get major map version. */
FLOAT getMajorMapVersion()
{
	// Don't mess with this value, unless you want to force update all maps in the game!
	return (s_gameVersion == GameVersion::RUSSIAN) ? 6.00 : 5.00;
}

/** Get list of resource libraries. */
std::vector<ST::string> GetResourceLibraries(const ST::string &dataDir)
{
	std::vector<ST::string> libraries = FindFilesInDir(dataDir, "slf", true, true);

	// for (int i = 0; i < libraries.size(); i++)
	// {
	//   SLOGW("%s", libraries[i].c_str());
	// }

	return libraries;
}


#define STI(LNG, x) LNG "/" x "_" LNG ".sti"
#define PCX(LNG, x) LNG "/" x "_" LNG ".pcx"


char const* GetMLGFilename(MultiLanguageGraphic const id)
{
	if((s_gameVersion == GameVersion::ENGLISH) || (s_gameVersion == GameVersion::FRENCH) || (s_gameVersion == GameVersion::RUSSIAN_GOLD))
	{
		switch (id)
		{
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
			case MLG_TITLETEXT:          return LOADSCREENSDIR "/titletext.sti";
			case MLG_TOALUMNI:           return LAPTOPDIR "/toalumni.sti";
			case MLG_TOMUGSHOTS:         return LAPTOPDIR "/tomugshots.sti";
			case MLG_TOSTATS:            return LAPTOPDIR "/tostats.sti";
			case MLG_WARNING:            return LAPTOPDIR "/warning.sti";
			case MLG_YOURAD13:           return LAPTOPDIR "/yourad_13.sti";
			default:
				break;
		}
	}
	else if(s_gameVersion == GameVersion::GERMAN)
	{
		switch (id)
		{
			case MLG_AIMSYMBOL:          return LAPTOPDIR "/aimsymbol.sti";     // Same graphic (no translation needed)
			case MLG_BOBBYNAME:          return LAPTOPDIR "/bobbyname.sti";     // Same graphic (no translation needed)
			case MLG_BOBBYRAYAD21:       return LAPTOPDIR "/bobbyrayad_21.sti"; // Same graphic (no translation needed)
			case MLG_BOBBYRAYLINK:       return "german"  "/bobbyraylink_german.sti";
			case MLG_CLOSED:             return "german"  "/closed_german.sti";
			case MLG_CONFIRMORDER:       return "german"  "/confirmorder_german.sti";
			case MLG_DESKTOP:            return "german"  "/desktop_german.pcx";
			case MLG_FUNERALAD9:         return "german"  "/funeralad_12_german.sti";
			case MLG_GOLDPIECEBUTTONS:   return "german"  "/goldpiecebuttons_german.sti";
			case MLG_HISTORY:            return "german"  "/history_german.sti";
			case MLG_IMPSYMBOL:          return "german"  "/impsymbol_german.sti";
			case MLG_INSURANCEAD10:      return "german"  "/insurancead_10_german.sti";
			case MLG_INSURANCELINK:      return "german"  "/insurancelink_german.sti";
			case MLG_INSURANCETITLE:     return "german"  "/largetitle_german.sti";
			case MLG_LARGEFLORISTSYMBOL: return "german"  "/largesymbol_german.sti";
			case MLG_LOADSAVEHEADER:     return "german"  "/loadscreenaddons_german.sti";
			case MLG_MCGILLICUTTYS:      return "german"  "/mcgillicuttys_german.sti";
			case MLG_MORTUARY:           return "german"  "/mortuary_german.sti";
			case MLG_MORTUARYLINK:       return "german"  "/mortuarylink_german.sti";
			case MLG_OPTIONHEADER:       return "german"  "/optionscreenaddons_german.sti";
			case MLG_ORDERGRID:          return LAPTOPDIR "/ordergrid.sti"; // Same file
			case MLG_PREBATTLEPANEL:     return "german"  "/prebattlepanel_german.sti";
			case MLG_SMALLFLORISTSYMBOL: return "german"  "/smallsymbol_german.sti";
			case MLG_SMALLTITLE:         return "german"  "/smalltitle_german.sti";
			case MLG_SPLASH:             return "german"  "/splash_german.sti";
			case MLG_STATSBOX:           return LAPTOPDIR "/statsbox.sti"; // Same file
			case MLG_STOREPLAQUE:        return "german"  "/storeplaque_german.sti";
			case MLG_TITLETEXT:          return "german"  "/titletext_german.sti";
			case MLG_TOALUMNI:           return "german"  "/toalumni_german.sti";
			case MLG_TOMUGSHOTS:         return "german"  "/tomugshots_german.sti";
			case MLG_TOSTATS:            return "german"  "/tostats_german.sti";
			case MLG_WARNING:            return "german"  "/warning_german.sti";
			case MLG_YOURAD13:           return "german"  "/yourad_13_german.sti";
			default:
				break;
	}
	}
	else if(s_gameVersion == GameVersion::DUTCH)
	{
		switch (id)
		{
			case MLG_AIMSYMBOL:          return STI("dutch",   "aimsymbol");
			case MLG_BOBBYNAME:          return STI("dutch",   "bobbyname");
			case MLG_BOBBYRAYAD21:       return STI("dutch",   "bobbyrayad_21");
			case MLG_BOBBYRAYLINK:       return STI("dutch",   "bobbyraylink");
			case MLG_CLOSED:             return STI("dutch",   "closed");
			case MLG_CONFIRMORDER:       return STI("dutch",   "confirmorder");
			case MLG_DESKTOP:            return PCX("dutch",   "desktop");
			case MLG_FUNERALAD9:         return STI("dutch",   "funeralad_9");
			case MLG_GOLDPIECEBUTTONS:   return STI("dutch",   "goldpiecebuttons");
			case MLG_HISTORY:            return STI("dutch",   "history");
			case MLG_IMPSYMBOL:          return STI("dutch",   "impsymbol");
			case MLG_INSURANCEAD10:      return STI("dutch",   "insurancead_10");
			case MLG_INSURANCELINK:      return STI("dutch",   "insurancelink");
			case MLG_INSURANCETITLE:     return STI("dutch",   "largetitle");
			case MLG_LARGEFLORISTSYMBOL: return STI("dutch",   "largesymbol");
			case MLG_LOADSAVEHEADER:     return STI("dutch",   "loadscreenaddons");
			case MLG_MCGILLICUTTYS:      return STI("dutch",   "mcgillicuttys");
			case MLG_MORTUARY:           return STI("dutch",   "mortuary");
			case MLG_MORTUARYLINK:       return STI("dutch",   "mortuarylink");
			case MLG_OPTIONHEADER:       return STI("dutch",   "optionscreenaddons");
			case MLG_ORDERGRID:          return STI("dutch",   "ordergrid");
			case MLG_PREBATTLEPANEL:     return STI("dutch",   "prebattlepanel");
			case MLG_SMALLFLORISTSYMBOL: return STI("dutch",   "smallsymbol");
			case MLG_SMALLTITLE:         return STI("dutch",   "smalltitle");
			case MLG_SPLASH:             return STI("dutch",   "splash");
			case MLG_STATSBOX:           return STI("dutch",   "statsbox");
			case MLG_STOREPLAQUE:        return STI("dutch",   "storeplaque");
			case MLG_TITLETEXT:          return STI("dutch",   "titletext");
			case MLG_TOALUMNI:           return STI("dutch",   "toalumni");
			case MLG_TOMUGSHOTS:         return STI("dutch",   "tomugshots");
			case MLG_TOSTATS:            return STI("dutch",   "tostats");
			case MLG_WARNING:            return STI("dutch",   "warning");
			case MLG_YOURAD13:           return STI("dutch",   "yourad_13");
			default:
				break;
		}
	}
	else if(s_gameVersion == GameVersion::ITALIAN)
	{
		switch (id)
		{
			case MLG_AIMSYMBOL:          return STI("italian", "aimsymbol");
			case MLG_BOBBYNAME:          return STI("italian", "bobbyname");
			case MLG_BOBBYRAYAD21:       return STI("italian", "bobbyrayad_21");
			case MLG_BOBBYRAYLINK:       return STI("italian", "bobbyraylink");
			case MLG_CLOSED:             return STI("italian", "closed");
			case MLG_CONFIRMORDER:       return STI("italian", "confirmorder");
			case MLG_DESKTOP:            return PCX("italian", "desktop");
			case MLG_FUNERALAD9:         return STI("italian", "funeralad_9");
			case MLG_GOLDPIECEBUTTONS:   return STI("italian", "goldpiecebuttons");
			case MLG_HISTORY:            return STI("italian", "history");
			case MLG_IMPSYMBOL:          return STI("italian", "impsymbol");
			case MLG_INSURANCEAD10:      return STI("italian", "insurancead_10");
			case MLG_INSURANCELINK:      return STI("italian", "insurancelink");
			case MLG_INSURANCETITLE:     return STI("italian", "largetitle");
			case MLG_LARGEFLORISTSYMBOL: return STI("italian", "largesymbol");
			case MLG_LOADSAVEHEADER:     return STI("italian", "loadscreenaddons");
			case MLG_MCGILLICUTTYS:      return STI("italian", "mcgillicuttys");
			case MLG_MORTUARY:           return STI("italian", "mortuary");
			case MLG_MORTUARYLINK:       return STI("italian", "mortuarylink");
			case MLG_OPTIONHEADER:       return STI("italian", "optionscreenaddons");
			case MLG_ORDERGRID:          return STI("italian", "ordergrid");
			case MLG_PREBATTLEPANEL:     return STI("italian", "prebattlepanel");
			case MLG_SMALLFLORISTSYMBOL: return STI("italian", "smallsymbol");
			case MLG_SMALLTITLE:         return STI("italian", "smalltitle");
			case MLG_SPLASH:             return STI("italian", "splash");
			case MLG_STATSBOX:           return STI("italian", "statsbox");
			case MLG_STOREPLAQUE:        return STI("italian", "storeplaque");
			case MLG_TITLETEXT:          return STI("italian", "titletext");
			case MLG_TOALUMNI:           return STI("italian", "toalumni");
			case MLG_TOMUGSHOTS:         return STI("italian", "tomugshots");
			case MLG_TOSTATS:            return STI("italian", "tostats");
			case MLG_WARNING:            return STI("italian", "warning");
			case MLG_YOURAD13:           return STI("italian", "yourad_13");
			default:
				break;
		}
	}
	else if(s_gameVersion == GameVersion::POLISH)
	{
		switch (id)
		{
			case MLG_AIMSYMBOL:          return STI("polish",  "aimsymbol");
			case MLG_BOBBYNAME:          return STI("polish",  "bobbyname");
			case MLG_BOBBYRAYAD21:       return STI("polish",  "bobbyrayad_21");
			case MLG_BOBBYRAYLINK:       return STI("polish",  "bobbyraylink");
			case MLG_CLOSED:             return STI("polish",  "closed");
			case MLG_CONFIRMORDER:       return STI("polish",  "confirmorder");
			case MLG_DESKTOP:            return PCX("polish",  "desktop");
			case MLG_FUNERALAD9:         return STI("polish",  "funeralad_9");
			case MLG_GOLDPIECEBUTTONS:   return STI("polish",  "goldpiecebuttons");
			case MLG_HISTORY:            return STI("polish",  "history");
			case MLG_IMPSYMBOL:          return STI("polish",  "impsymbol");
			case MLG_INSURANCEAD10:      return STI("polish",  "insurancead_10");
			case MLG_INSURANCELINK:      return STI("polish",  "insurancelink");
			case MLG_INSURANCETITLE:     return STI("polish",  "largetitle");
			case MLG_LARGEFLORISTSYMBOL: return STI("polish",  "largesymbol");
			case MLG_LOADSAVEHEADER:     return STI("polish",  "loadscreenaddons");
			case MLG_MCGILLICUTTYS:      return STI("polish",  "mcgillicuttys");
			case MLG_MORTUARY:           return STI("polish",  "mortuary");
			case MLG_MORTUARYLINK:       return STI("polish",  "mortuarylink");
			case MLG_OPTIONHEADER:       return STI("polish",  "optionscreenaddons");
			case MLG_ORDERGRID:          return STI("polish",  "ordergrid");
			case MLG_PREBATTLEPANEL:     return STI("polish",  "prebattlepanel");
			case MLG_SMALLFLORISTSYMBOL: return STI("polish",  "smallsymbol");
			case MLG_SMALLTITLE:         return STI("polish",  "smalltitle");
			case MLG_SPLASH:             return STI("polish",  "splash");
			case MLG_STATSBOX:           return STI("polish",  "statsbox");
			case MLG_STOREPLAQUE:        return STI("polish",  "storeplaque");
			case MLG_TITLETEXT:          return STI("polish",  "titletext");
			case MLG_TOALUMNI:           return STI("polish",  "toalumni");
			case MLG_TOMUGSHOTS:         return STI("polish",  "tomugshots");
			case MLG_TOSTATS:            return STI("polish",  "tostats");
			case MLG_WARNING:            return STI("polish",  "warning");
			case MLG_YOURAD13:           return STI("polish",  "yourad_13");
			default:
				break;
		}
	}
	else if(s_gameVersion == GameVersion::RUSSIAN)
	{
		switch (id)
		{
			case MLG_AIMSYMBOL:          return STI("russian", "aimsymbol");
			case MLG_BOBBYNAME:          return STI("russian", "bobbyname");
			case MLG_BOBBYRAYAD21:       return STI("russian", "bobbyrayad_21");
			case MLG_BOBBYRAYLINK:       return STI("russian", "bobbyraylink");
			case MLG_CLOSED:             return STI("russian", "closed");
			case MLG_CONFIRMORDER:       return STI("russian", "confirmorder");
			case MLG_DESKTOP:            return PCX("russian", "desktop");
			case MLG_FUNERALAD9:         return STI("russian", "funeralad_9");
			case MLG_GOLDPIECEBUTTONS:   return STI("russian", "goldpiecebuttons");
			case MLG_HISTORY:            return STI("russian", "history");
			case MLG_IMPSYMBOL:          return STI("russian", "impsymbol");
			case MLG_INSURANCEAD10:      return STI("russian", "insurancead_10");
			case MLG_INSURANCELINK:      return STI("russian", "insurancelink");
			case MLG_INSURANCETITLE:     return STI("russian", "largetitle");
			case MLG_LARGEFLORISTSYMBOL: return STI("russian", "largesymbol");
			case MLG_LOADSAVEHEADER:     return STI("russian", "loadscreenaddons");
			case MLG_MCGILLICUTTYS:      return STI("russian", "mcgillicuttys");
			case MLG_MORTUARY:           return STI("russian", "mortuary");
			case MLG_MORTUARYLINK:       return STI("russian", "mortuarylink");
			case MLG_OPTIONHEADER:       return STI("russian", "optionscreenaddons");
			case MLG_ORDERGRID:          return STI("russian", "ordergrid");
			case MLG_PREBATTLEPANEL:     return STI("russian", "prebattlepanel");
			case MLG_SMALLFLORISTSYMBOL: return STI("russian", "smallsymbol");
			case MLG_SMALLTITLE:         return STI("russian", "smalltitle");
			case MLG_SPLASH:             return STI("russian", "splash");
			case MLG_STATSBOX:           return STI("russian", "statsbox");
			case MLG_STOREPLAQUE:        return STI("russian", "bobbystoreplaque");
			case MLG_TITLETEXT:          return STI("russian", "titletext");
			case MLG_TOALUMNI:           return STI("russian", "toalumni");
			case MLG_TOMUGSHOTS:         return STI("russian", "tomugshots");
			case MLG_TOSTATS:            return STI("russian", "tostats");
			case MLG_WARNING:            return STI("russian", "warning");
			case MLG_YOURAD13:           return STI("russian", "yourad_13");
			default:
				break;
		}
	}

	throw std::runtime_error(FormattedString("Multilanguage resource %d is not found", id).to_std_string());
}

STRING_ENC_TYPE getStringEncType()
{
	if(isRussianVersion() || isRussianGoldVersion())
	{
		return SE_RUSSIAN;
	}
	else if(isPolishVersion())
	{
		return SE_POLISH;
	}
	else if(isEnglishVersion())
	{
		return SE_ENGLISH;
	}
	return SE_NORMAL;
}
