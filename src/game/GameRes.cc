#include "GameRes.h"
#include "Directories.h"
#include "EncodingCorrectors.h"
#include "FileMan.h"
#include "GameMode.h"
#include "Logger.h"
#include "Text.h"
#include "VObject.h"
#include <locale.h>
#include <stdexcept>


extern LanguageRes g_LanguageResDutch;
extern LanguageRes g_LanguageResEnglish;
extern LanguageRes g_LanguageResFrench;
extern LanguageRes g_LanguageResGerman;
extern LanguageRes g_LanguageResItalian;
extern LanguageRes g_LanguageResPolish;
extern LanguageRes g_LanguageResRussian;
extern LanguageRes g_LanguageResRussianGold;
extern LanguageRes g_LanguageResChinese;


/** Game version. */
static GameVersion s_gameVersion = GameVersion::ENGLISH;

/** Current language resources. */
const LanguageRes* g_langRes = &g_LanguageResEnglish;

/** Set language resources. */
static void setResources(const LanguageRes* langRes)
{
	g_langRes = langRes;
}


/** Choose game version. */
void setGameVersion(GameVersion ver)
{
	s_gameVersion = ver;
	switch(s_gameVersion)
	{
		case GameVersion::DUTCH:        setResources(&g_LanguageResDutch);              break;
		case GameVersion::ENGLISH:      setResources(&g_LanguageResEnglish);            break;
		case GameVersion::FRENCH:       setResources(&g_LanguageResFrench);             break;
		case GameVersion::GERMAN:       setResources(&g_LanguageResGerman);             break;
		case GameVersion::ITALIAN:      setResources(&g_LanguageResItalian);            break;
		case GameVersion::POLISH:       setResources(&g_LanguageResPolish);             break;
		case GameVersion::RUSSIAN:      setResources(&g_LanguageResRussian);            break;
		case GameVersion::RUSSIAN_GOLD: setResources(&g_LanguageResRussianGold);        break;
		case GameVersion::SIMPLIFIED_CHINESE:      setResources(&g_LanguageResChinese); break;
		default:
		{
			SLOGW("Unknown version. Using ENGLISH by defaul");
			s_gameVersion = GameVersion::ENGLISH;
			setResources(&g_LanguageResEnglish);
		}
	}

	void LanguageResToJson(LanguageRes const& resource, char const *filename);
	LanguageResToJson(g_LanguageResChinese, "/tmp/Translation_Chinese.json");
	LanguageResToJson(g_LanguageResDutch, "/tmp/Translation_Dutch.json");
	LanguageResToJson(g_LanguageResEnglish, "/tmp/Translation_English.json");
	LanguageResToJson(g_LanguageResFrench, "/tmp/Translation_French.json");
	LanguageResToJson(g_LanguageResGerman, "/tmp/Translation_German.json");
	LanguageResToJson(g_LanguageResItalian, "/tmp/Translation_Italian.json");
	LanguageResToJson(g_LanguageResPolish, "/tmp/Translation_Polish.json");
	LanguageResToJson(g_LanguageResRussian, "/tmp/Translation_Russian.json");
	LanguageResToJson(g_LanguageResRussianGold, "/tmp/Translation_RussianGold.json");
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

/** Check if this is Chinese version of the game. */
bool isChineseVersion()
{
	return s_gameVersion == GameVersion::SIMPLIFIED_CHINESE;
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

#define STI(LNG, x) LNG "/" x "_" LNG ".sti"
#define PCX(LNG, x) LNG "/" x "_" LNG ".pcx"


char const* GetMLGFilename(MultiLanguageGraphic const id)
{
	if((s_gameVersion == GameVersion::ENGLISH) || (s_gameVersion == GameVersion::FRENCH) || (s_gameVersion == GameVersion::RUSSIAN_GOLD) || (s_gameVersion == GameVersion::SIMPLIFIED_CHINESE))
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

	throw std::runtime_error(ST::format("Multilanguage resource {} is not found", id).to_std_string());
}


SGPVObject* AddVideoObjectFromFile(MultiLanguageGraphic const mlg)
{
	return AddVideoObjectFromFile(GetMLGFilename(mlg));
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
	else if(isEnglishVersion() || isChineseVersion())
	{
		return SE_ENGLISH;
	}
	return SE_NORMAL;
}

#include "Json.h"
#include <fstream>

JsonArray ToArray(ST::string const strings[], int count)
{
	JsonArray result;

	for (int i = 0; i != count; i++) {
		result.push(JsonValue{ strings[i] });
	}
	return result;
}

struct JsonObject2 : JsonObject
{
	JsonObject2 & s(char const * key, JsonValue && value) {
		set(key, std::move(value));
		return *this;
	}

	JsonObject2 & s(char const * key, JsonArray const& array) {
		set(key, array.toValue());
		return *this;
	}
};


#undef WeaponType                                          (g_langRes->WeaponType)
#undef TeamTurnString                                      (g_langRes->TeamTurnString)
#undef pAssignMenuStrings                                  (g_langRes->pAssignMenuStrings)
#undef pTrainingStrings                                    (g_langRes->pTrainingStrings)
#undef pTrainingMenuStrings                                (g_langRes->pTrainingMenuStrings)
#undef pAttributeMenuStrings                               (g_langRes->pAttributeMenuStrings)
#undef pVehicleStrings                                     (g_langRes->pVehicleStrings)
#undef pShortAttributeStrings                              (g_langRes->pShortAttributeStrings)
#undef pContractStrings                                    (g_langRes->pContractStrings)
#undef pAssignmentStrings                                  (g_langRes->pAssignmentStrings)
#undef pConditionStrings                                   (g_langRes->pConditionStrings)
#undef pPersonnelScreenStrings                             (g_langRes->pPersonnelScreenStrings)
#undef pUpperLeftMapScreenStrings                          (g_langRes->pUpperLeftMapScreenStrings)
#undef pTacticalPopupButtonStrings                         (g_langRes->pTacticalPopupButtonStrings)
#undef pSquadMenuStrings                                   (g_langRes->pSquadMenuStrings)
#undef pDoorTrapStrings                                    (g_langRes->pDoorTrapStrings)
#undef pLongAssignmentStrings                              (g_langRes->pLongAssignmentStrings)
#undef pMapScreenMouseRegionHelpText                       (g_langRes->pMapScreenMouseRegionHelpText)
#undef pNoiseVolStr                                        (g_langRes->pNoiseVolStr)
#undef pNoiseTypeStr                                       (g_langRes->pNoiseTypeStr)
#undef pDirectionStr                                       (g_langRes->pDirectionStr)
#undef pRemoveMercStrings                                  (g_langRes->pRemoveMercStrings)
#undef sTimeStrings                                        (g_langRes->sTimeStrings)
#undef pInvPanelTitleStrings                               (g_langRes->pInvPanelTitleStrings)
#undef pPOWStrings                                         (g_langRes->pPOWStrings)
#undef pMilitiaString                                      (g_langRes->pMilitiaString)
#undef pMilitiaButtonString                                (g_langRes->pMilitiaButtonString)
#undef pEpcMenuStrings                                     (g_langRes->pEpcMenuStrings)
#undef pRepairStrings                                      (g_langRes->pRepairStrings)
#undef sPreStatBuildString                                 (g_langRes->sPreStatBuildString)
#undef sStatGainStrings                                    (g_langRes->sStatGainStrings)
#undef pHelicopterEtaStrings                               (g_langRes->pHelicopterEtaStrings)
#undef sMapLevelString                                     (g_langRes->sMapLevelString)
#undef gsLoyalString                                       (g_langRes->gsLoyalString)
#undef gsUndergroundString                                 (g_langRes->gsUndergroundString)
#undef gsTimeStrings                                       (g_langRes->gsTimeStrings)
#undef sFacilitiesStrings                                  (g_langRes->sFacilitiesStrings)
#undef pMapPopUpInventoryText                              (g_langRes->pMapPopUpInventoryText)
#undef pwTownInfoStrings                                   (g_langRes->pwTownInfoStrings)
#undef pwMineStrings                                       (g_langRes->pwMineStrings)
#undef pwMiscSectorStrings                                 (g_langRes->pwMiscSectorStrings)
#undef pMapInventoryErrorString                            (g_langRes->pMapInventoryErrorString)
#undef pMapInventoryStrings                                (g_langRes->pMapInventoryStrings)
#undef pMovementMenuStrings                                (g_langRes->pMovementMenuStrings)
#undef pUpdateMercStrings                                  (g_langRes->pUpdateMercStrings)
#undef pMapScreenBorderButtonHelpText                      (g_langRes->pMapScreenBorderButtonHelpText)
#undef pMapScreenBottomFastHelp                            (g_langRes->pMapScreenBottomFastHelp)
#undef pMapScreenBottomText                                (g_langRes->pMapScreenBottomText)
#undef pMercDeadString                                     (g_langRes->pMercDeadString)
#undef pSenderNameList                                     (g_langRes->pSenderNameList)
#undef pNewMailStrings                                     (g_langRes->pNewMailStrings)
#undef pDeleteMailStrings                                  (g_langRes->pDeleteMailStrings)
#undef pEmailHeaders                                       (g_langRes->pEmailHeaders)
#undef pEmailTitleText                                     (g_langRes->pEmailTitleText)
#undef pFinanceTitle                                       (g_langRes->pFinanceTitle)
#undef pFinanceSummary                                     (g_langRes->pFinanceSummary)
#undef pFinanceHeaders                                     (g_langRes->pFinanceHeaders)
#undef pTransactionText                                    (g_langRes->pTransactionText)
#undef pMoralStrings                                       (g_langRes->pMoralStrings)
#undef pSkyriderText                                       (g_langRes->pSkyriderText)
#undef str_left_equipment                                  (g_langRes->str_left_equipment)
#undef pMapScreenStatusStrings                             (g_langRes->pMapScreenStatusStrings)
#undef pMapScreenPrevNextCharButtonHelpText                (g_langRes->pMapScreenPrevNextCharButtonHelpText)
#undef pEtaString                                          (g_langRes->pEtaString)
#undef pShortVehicleStrings                                (g_langRes->pShortVehicleStrings)
#undef pTrashItemText                                      (g_langRes->pTrashItemText)
#undef pMapErrorString                                     (g_langRes->pMapErrorString)
#undef pMapPlotStrings                                     (g_langRes->pMapPlotStrings)
#undef pBullseyeStrings                                    (g_langRes->pBullseyeStrings)
#undef pMiscMapScreenMouseRegionHelpText                   (g_langRes->pMiscMapScreenMouseRegionHelpText)
#undef str_he_leaves_where_drop_equipment                  (g_langRes->str_he_leaves_where_drop_equipment)
#undef str_she_leaves_where_drop_equipment                 (g_langRes->str_she_leaves_where_drop_equipment)
#undef str_he_leaves_drops_equipment                       (g_langRes->str_he_leaves_drops_equipment)
#undef str_she_leaves_drops_equipment                      (g_langRes->str_she_leaves_drops_equipment)
#undef pImpPopUpStrings                                    (g_langRes->pImpPopUpStrings)
#undef pImpButtonText                                      (g_langRes->pImpButtonText)
#undef pExtraIMPStrings                                    (g_langRes->pExtraIMPStrings)
#undef pFilesTitle                                         (g_langRes->pFilesTitle)
#undef pFilesSenderList                                    (g_langRes->pFilesSenderList)
#undef pHistoryLocations                                   (g_langRes->pHistoryLocations)
#undef pHistoryStrings                                     (g_langRes->pHistoryStrings)
#undef pHistoryHeaders                                     (g_langRes->pHistoryHeaders)
#undef pHistoryTitle                                       (g_langRes->pHistoryTitle)
#undef pShowBookmarkString                                 (g_langRes->pShowBookmarkString)
#undef pWebPagesTitles                                     (g_langRes->pWebPagesTitles)
#undef pWebTitle                                           (g_langRes->pWebTitle)
#undef pPersonnelString                                    (g_langRes->pPersonnelString)
#undef pErrorStrings                                       (g_langRes->pErrorStrings)
#undef pDownloadString                                     (g_langRes->pDownloadString)
#undef pBookMarkStrings                                    (g_langRes->pBookMarkStrings)
#undef pLaptopIcons                                        (g_langRes->pLaptopIcons)
#undef gsAtmStartButtonText                                (g_langRes->gsAtmStartButtonText)
#undef pPersonnelTeamStatsStrings                          (g_langRes->pPersonnelTeamStatsStrings)
#undef pPersonnelCurrentTeamStatsStrings                   (g_langRes->pPersonnelCurrentTeamStatsStrings)
#undef pPersonelTeamStrings                                (g_langRes->pPersonelTeamStrings)
#undef pPersonnelDepartedStateStrings                      (g_langRes->pPersonnelDepartedStateStrings)
#undef pMapHortIndex                                       (g_langRes->pMapHortIndex)
#undef pMapVertIndex                                       (g_langRes->pMapVertIndex)
#undef pMapDepthIndex                                      (g_langRes->pMapDepthIndex)
#undef pLaptopTitles                                       (g_langRes->pLaptopTitles)
#undef pDayStrings                                         (g_langRes->pDayStrings)
#undef pMilitiaConfirmStrings                              (g_langRes->pMilitiaConfirmStrings)
#undef pSkillAtZeroWarning                                 (g_langRes->pSkillAtZeroWarning)
#undef pIMPBeginScreenStrings                              (g_langRes->pIMPBeginScreenStrings)
#undef pIMPFinishButtonText                                (g_langRes->pIMPFinishButtonText)
#undef pIMPFinishStrings                                   (g_langRes->pIMPFinishStrings)
#undef pIMPVoicesStrings                                   (g_langRes->pIMPVoicesStrings)
#undef pPersTitleText                                      (g_langRes->pPersTitleText)
#undef pPausedGameText                                     (g_langRes->pPausedGameText)
#undef zOptionsToggleText                                  (g_langRes->zOptionsToggleText)
#undef zOptionsScreenHelpText                              (g_langRes->zOptionsScreenHelpText)
#undef pDoctorWarningString                                (g_langRes->pDoctorWarningString)
#undef pMilitiaButtonsHelpText                             (g_langRes->pMilitiaButtonsHelpText)
#undef pMapScreenJustStartedHelpText                       (g_langRes->pMapScreenJustStartedHelpText)
#undef pLandMarkInSectorString                             (g_langRes->pLandMarkInSectorString)
#undef gzMercSkillText                                     (g_langRes->gzMercSkillText)
#undef gzNonPersistantPBIText                              (g_langRes->gzNonPersistantPBIText)
#undef gzMiscString                                        (g_langRes->gzMiscString)
#undef wMapScreenSortButtonHelpText                        (g_langRes->wMapScreenSortButtonHelpText)
#undef pNewNoiseStr                                        (g_langRes->pNewNoiseStr)
#undef gzLateLocalizedString                               (g_langRes->gzLateLocalizedString)
#undef pAntiHackerString                                   (g_langRes->pAntiHackerString)
#undef pMessageStrings                                     (g_langRes->pMessageStrings)
#undef ItemPickupHelpPopup                                 (g_langRes->ItemPickupHelpPopup)
#undef TacticalStr                                         (g_langRes->TacticalStr)
#undef LargeTacticalStr                                    (g_langRes->LargeTacticalStr)
#undef zDialogActions                                      (g_langRes->zDialogActions)
#undef zDealerStrings                                      (g_langRes->zDealerStrings)
#undef zTalkMenuStrings                                    (g_langRes->zTalkMenuStrings)
#undef gzMoneyAmounts                                      (g_langRes->gzMoneyAmounts)
#undef gzProsLabel                                         (g_langRes->gzProsLabel)
#undef gzConsLabel                                         (g_langRes->gzConsLabel)
#undef gMoneyStatsDesc                                     (g_langRes->gMoneyStatsDesc)
#undef gWeaponStatsDesc                                    (g_langRes->gWeaponStatsDesc)
#undef sKeyDescriptionStrings                              (g_langRes->sKeyDescriptionStrings)
#undef zHealthStr                                          (g_langRes->zHealthStr)
#undef zVehicleName                                        (g_langRes->zVehicleName)
#undef pExitingSectorHelpText                              (g_langRes->pExitingSectorHelpText)
#undef InsContractText                                     (g_langRes->InsContractText)
#undef InsInfoText                                         (g_langRes->InsInfoText)
#undef MercAccountText                                     (g_langRes->MercAccountText)
#undef MercInfo                                            (g_langRes->MercInfo)
#undef MercNoAccountText                                   (g_langRes->MercNoAccountText)
#undef MercHomePageText                                    (g_langRes->MercHomePageText)
#undef sFuneralString                                      (g_langRes->sFuneralString)
#undef sFloristText                                        (g_langRes->sFloristText)
#undef sOrderFormText                                      (g_langRes->sOrderFormText)
#undef sFloristGalleryText                                 (g_langRes->sFloristGalleryText)
#undef sFloristCards                                       (g_langRes->sFloristCards)
#undef BobbyROrderFormText                                 (g_langRes->BobbyROrderFormText)
#undef BobbyRText                                          (g_langRes->BobbyRText)
#undef str_bobbyr_guns_num_guns_that_use_ammo              (g_langRes->str_bobbyr_guns_num_guns_that_use_ammo)
#undef BobbyRaysFrontText                                  (g_langRes->BobbyRaysFrontText)
#undef AimSortText                                         (g_langRes->AimSortText)
#undef str_aim_sort_price                                  (g_langRes->str_aim_sort_price)
#undef str_aim_sort_experience                             (g_langRes->str_aim_sort_experience)
#undef str_aim_sort_marksmanship                           (g_langRes->str_aim_sort_marksmanship)
#undef str_aim_sort_medical                                (g_langRes->str_aim_sort_medical)
#undef str_aim_sort_explosives                             (g_langRes->str_aim_sort_explosives)
#undef str_aim_sort_mechanical                             (g_langRes->str_aim_sort_mechanical)
#undef str_aim_sort_ascending                              (g_langRes->str_aim_sort_ascending)
#undef str_aim_sort_descending                             (g_langRes->str_aim_sort_descending)
#undef AimPolicyText                                       (g_langRes->AimPolicyText)
#undef AimMemberText                                       (g_langRes->AimMemberText)
#undef CharacterInfo                                       (g_langRes->CharacterInfo)
#undef VideoConfercingText                                 (g_langRes->VideoConfercingText)
#undef AimPopUpText                                        (g_langRes->AimPopUpText)
#undef AimLinkText                                         (g_langRes->AimLinkText)
#undef AimHistoryText                                      (g_langRes->AimHistoryText)
#undef AimFiText                                           (g_langRes->AimFiText)
#undef AimAlumniText                                       (g_langRes->AimAlumniText)
#undef AimScreenText                                       (g_langRes->AimScreenText)
#undef AimBottomMenuText                                   (g_langRes->AimBottomMenuText)
#undef zMarksMapScreenText                                 (g_langRes->zMarksMapScreenText)
#undef gpStrategicString                                   (g_langRes->gpStrategicString)
#undef gpGameClockString                                   (g_langRes->gpGameClockString)
#undef SKI_Text                                            (g_langRes->SKI_Text)
#undef SkiMessageBoxText                                   (g_langRes->SkiMessageBoxText)
#undef zSaveLoadText                                       (g_langRes->zSaveLoadText)
#undef zOptionsText                                        (g_langRes->zOptionsText)
#undef gzGIOScreenText                                     (g_langRes->gzGIOScreenText)
#undef gzHelpScreenText                                    (g_langRes->gzHelpScreenText)
#undef gzLaptopHelpText                                    (g_langRes->gzLaptopHelpText)
#undef gzMoneyWithdrawMessageText                          (g_langRes->gzMoneyWithdrawMessageText)
#undef gzCopyrightText                                     (g_langRes->gzCopyrightText)
#undef BrokenLinkText                                      (g_langRes->BrokenLinkText)
#undef gzBobbyRShipmentText                                (g_langRes->gzBobbyRShipmentText)
#undef zGioDifConfirmText                                  (g_langRes->zGioDifConfirmText)
#undef gzCreditNames                                       (g_langRes->gzCreditNames)
#undef gzCreditNameTitle                                   (g_langRes->gzCreditNameTitle)
#undef gzCreditNameFunny                                   (g_langRes->gzCreditNameFunny)
#undef pContractButtonString                               (g_langRes->pContractButtonString)
#undef gzIntroScreen                                       (g_langRes->gzIntroScreen)
#undef pUpdatePanelButtons                                 (g_langRes->pUpdatePanelButtons)
#undef sRepairsDoneString                                  (g_langRes->sRepairsDoneString)
#undef str_ceramic_plates_smashed                          (g_langRes->str_ceramic_plates_smashed)
#undef str_arrival_rerouted                                (g_langRes->str_arrival_rerouted)
#undef str_stat_health                                     (g_langRes->str_stat_health)
#undef str_stat_agility                                    (g_langRes->str_stat_agility)
#undef str_stat_dexterity                                  (g_langRes->str_stat_dexterity)
#undef str_stat_strength                                   (g_langRes->str_stat_strength)
#undef str_stat_leadership                                 (g_langRes->str_stat_leadership)
#undef str_stat_wisdom                                     (g_langRes->str_stat_wisdom)
#undef str_stat_exp_level                                  (g_langRes->str_stat_exp_level)
#undef str_stat_marksmanship                               (g_langRes->str_stat_marksmanship)
#undef str_stat_mechanical                                 (g_langRes->str_stat_mechanical)
#undef str_stat_explosive                                  (g_langRes->str_stat_explosive)
#undef str_stat_medical                                    (g_langRes->str_stat_medical)
#undef str_stat_list                                       (g_langRes->str_stat_list)
#undef str_aim_sort_list                                   (g_langRes->str_aim_sort_list)
#undef zNewTacticalMessages                                (g_langRes->zNewTacticalMessages)
#undef str_iron_man_mode_warning                           (g_langRes->str_iron_man_mode_warning)
#undef str_dead_is_dead_mode_warning                       (g_langRes->str_dead_is_dead_mode_warning)
#undef str_dead_is_dead_mode_enter_name                    (g_langRes->str_dead_is_dead_mode_enter_name)
#undef gs_dead_is_dead_mode_tab_name                       (g_langRes->gs_dead_is_dead_mode_tab_name)
#undef gzIMPSkillTraitsText (g_langRes->gzIMPSkillTraitsText)


void LanguageResToJson(LanguageRes const& resource, char const * filename)
{
	JsonObject2 result;

	#define AddArray(arrayName) .s(#arrayName , ToArray(   resource. arrayName , arrayName ## _SIZE))
	#define AddString(strName)  .s(#strName   , JsonValue{ resource. strName })

	result
AddArray(WeaponType)
AddArray(Message)
AddArray(TeamTurnString)
AddArray(pAssignMenuStrings)
AddArray(pTrainingStrings)
AddArray(pTrainingMenuStrings)
AddArray(pAttributeMenuStrings)
AddArray(pVehicleStrings)
AddArray(pShortAttributeStrings)
AddArray(pContractStrings)
AddArray(pAssignmentStrings)
AddArray(pConditionStrings)
AddArray(pPersonnelScreenStrings)
AddArray(pUpperLeftMapScreenStrings)
AddArray(pTacticalPopupButtonStrings)
AddArray(pSquadMenuStrings)
AddArray(pDoorTrapStrings)
AddArray(pLongAssignmentStrings)
AddArray(pMapScreenMouseRegionHelpText)
AddArray(pNoiseVolStr)
AddArray(pNoiseTypeStr)
AddArray(pDirectionStr)
AddArray(pRemoveMercStrings)
AddArray(sTimeStrings)
AddArray(pInvPanelTitleStrings)
AddArray(pPOWStrings)
AddArray(pMilitiaString)
AddArray(pMilitiaButtonString)
AddArray(pEpcMenuStrings)
AddArray(pRepairStrings)
AddArray(sPreStatBuildString)
AddArray(sStatGainStrings)
AddArray(pHelicopterEtaStrings)
AddString(sMapLevelString)
AddString(gsLoyalString)
AddString(gsUndergroundString)
AddArray(gsTimeStrings)
AddArray(sFacilitiesStrings)
AddArray(pMapPopUpInventoryText)
AddArray(pwTownInfoStrings)
AddArray(pwMineStrings)
AddArray(pwMiscSectorStrings)
AddArray(pMapInventoryErrorString)
AddArray(pMapInventoryStrings)
AddArray(pMovementMenuStrings)
AddArray(pUpdateMercStrings)
AddArray(pMapScreenBorderButtonHelpText)
AddArray(pMapScreenBottomFastHelp)
AddString(pMapScreenBottomText)
AddString(pMercDeadString)
AddArray(pSenderNameList)
AddArray(pDeleteMailStrings)
AddArray(pEmailHeaders)
AddString(pNewMailStrings)
AddString(pEmailTitleText)
AddString(pFinanceTitle)
AddArray(pFinanceSummary)
AddArray(pFinanceHeaders)
AddArray(pTransactionText)
AddArray(pMoralStrings)
AddArray(pSkyriderText)
AddString(str_left_equipment)
AddString(pEtaString)
AddString(str_he_leaves_where_drop_equipment)
AddString(str_she_leaves_where_drop_equipment)
AddString(str_he_leaves_drops_equipment)
AddString(str_she_leaves_drops_equipment)
AddString(pFilesTitle)
AddString(pHistoryTitle)
AddString(pHistoryLocations)
AddString(pWebTitle)
AddString(pPersonnelString)
AddString(pErrorStrings)
AddArray(pMapScreenStatusStrings)
AddArray(pMapScreenPrevNextCharButtonHelpText)
AddArray(pShortVehicleStrings)
AddArray(pTrashItemText)
AddArray(pMapErrorString)
AddArray(pMapPlotStrings)
AddArray(pBullseyeStrings)
AddArray(pMiscMapScreenMouseRegionHelpText)
AddArray(pImpPopUpStrings)
AddArray(pImpButtonText)
AddArray(pExtraIMPStrings)
AddArray(pFilesSenderList)
AddArray(pHistoryStrings)
AddArray(pHistoryHeaders)
AddArray(pShowBookmarkString)
AddArray(pWebPagesTitles)
AddArray(pDownloadString)
AddArray(pBookMarkStrings)
AddArray(pLaptopIcons)
AddArray(gsAtmStartButtonText)
AddArray(pPersonnelTeamStatsStrings)
AddArray(pPersonnelCurrentTeamStatsStrings)
AddArray(pPersonelTeamStrings)
AddArray(pPersonnelDepartedStateStrings)
AddArray(pMapHortIndex)
AddArray(pMapVertIndex)
AddArray(pMapDepthIndex)
AddArray(pLaptopTitles)
AddArray(pMilitiaConfirmStrings)
AddString(pDayStrings)
AddString(pSkillAtZeroWarning)
AddString(pIMPBeginScreenStrings)
AddString(pIMPFinishButtonText)
AddString(pIMPFinishStrings)
AddString(pIMPVoicesStrings)
AddString(pPersTitleText)
AddString(pMapScreenJustStartedHelpText)
AddString(pLandMarkInSectorString)
AddArray(pPausedGameText)
AddArray(zOptionsToggleText)
AddArray(zOptionsScreenHelpText)
AddArray(pDoctorWarningString)
AddArray(pMilitiaButtonsHelpText)
AddArray(gzMercSkillText)
AddArray(gzNonPersistantPBIText)
AddArray(gzMiscString)
AddArray(wMapScreenSortButtonHelpText)
AddArray(pNewNoiseStr)
AddArray(gzLateLocalizedString)
AddString(pAntiHackerString)
AddArray(pMessageStrings)
AddArray(ItemPickupHelpPopup)
AddArray(TacticalStr)
AddArray(LargeTacticalStr)
AddString(zDialogActions)
AddArray(zDealerStrings)
AddArray(zTalkMenuStrings)
AddArray(gzMoneyAmounts)
AddString(gzProsLabel)
AddString(gzConsLabel)
AddArray(gMoneyStatsDesc)
AddArray(gWeaponStatsDesc)
AddArray(sKeyDescriptionStrings)
AddArray(zHealthStr)
AddArray(zVehicleName)
AddArray(pExitingSectorHelpText)
AddArray(InsContractText)
AddArray(InsInfoText)
AddArray(MercAccountText)
AddArray(MercInfo)
AddArray(MercNoAccountText)
AddArray(MercHomePageText)
AddArray(sFuneralString)
AddArray(sFloristText)
AddArray(sOrderFormText)
AddArray(sFloristGalleryText)
AddArray(sFloristCards)
AddArray(BobbyROrderFormText)
AddArray(BobbyRText)
AddArray(BobbyRaysFrontText)
AddArray(AimSortText)
AddString(str_bobbyr_guns_num_guns_that_use_ammo)
AddString(str_aim_sort_price)
AddString(str_aim_sort_experience)
AddString(str_aim_sort_marksmanship)
AddString(str_aim_sort_medical)
AddString(str_aim_sort_explosives)
AddString(str_aim_sort_mechanical)
AddString(str_aim_sort_ascending)
AddString(str_aim_sort_descending)
AddArray(AimPolicyText)
AddArray(AimMemberText)
AddArray(CharacterInfo)
AddArray(VideoConfercingText)
AddArray(AimPopUpText)
AddString(AimLinkText)
AddArray(AimHistoryText)
AddArray(AimFiText)
AddArray(AimAlumniText)
AddArray(AimScreenText)
AddArray(AimBottomMenuText)
AddArray(zMarksMapScreenText)
AddArray(gpStrategicString)
AddString(gpGameClockString)
AddArray(SKI_Text)
AddArray(SkiMessageBoxText)
AddArray(zSaveLoadText)
AddArray(zOptionsText)
AddArray(gzGIOScreenText)
AddString(gzHelpScreenText)
AddArray(gzLaptopHelpText)
AddArray(gzMoneyWithdrawMessageText)
AddString(gzCopyrightText)
AddArray(BrokenLinkText)
AddArray(gzBobbyRShipmentText)
AddArray(zGioDifConfirmText)
AddArray(gzCreditNames)
AddArray(gzCreditNameTitle)
AddArray(gzCreditNameFunny)
AddString(pContractButtonString)
AddString(gzIntroScreen)
AddArray(pUpdatePanelButtons)
AddArray(sRepairsDoneString)
AddString(str_ceramic_plates_smashed)
AddString(str_arrival_rerouted)
AddString(str_stat_health)
AddString(str_stat_agility)
AddString(str_stat_dexterity)
AddString(str_stat_strength)
AddString(str_stat_leadership)
AddString(str_stat_wisdom)
AddString(str_stat_exp_level)
AddString(str_stat_marksmanship)
AddString(str_stat_mechanical)
AddString(str_stat_explosive)
AddString(str_stat_medical)
AddArray(str_stat_list)
AddArray(str_aim_sort_list)
AddArray(zNewTacticalMessages)
AddString(str_iron_man_mode_warning)
AddString(str_dead_is_dead_mode_warning)
AddString(str_dead_is_dead_mode_enter_name)
AddArray(gs_dead_is_dead_mode_tab_name)
AddArray(gzIMPSkillTraitsText)
	;

	std::ofstream ofs{filename};
	ofs << result.toValue().serialize().c_str();
}
