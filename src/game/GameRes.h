#pragma once

#include <string>
#include <vector>

/* Game resources */

#include "Types.h"
#include "IEncodingCorrector.h"

#include "StringEncodingTypes.h"

/** List of supported game versions (localizations). */
enum GameVersion {
	GV_DUTCH,
	GV_ENGLISH,
	GV_FRENCH,
	GV_GERMAN,
	GV_ITALIAN,
	GV_POLISH,
	GV_RUSSIAN,
	GV_RUSSIAN_GOLD,
};

enum MultiLanguageGraphic
{
	MLG_AIMSYMBOL,
	MLG_BOBBYNAME,
	MLG_BOBBYRAYAD21,
	MLG_BOBBYRAYLINK,
	MLG_CLOSED,
	MLG_CONFIRMORDER,
	MLG_DESKTOP,
	MLG_FUNERALAD9,
	MLG_GOLDPIECEBUTTONS,
	MLG_HISTORY,
	MLG_IMPSYMBOL,
	MLG_INSURANCEAD10,
	MLG_INSURANCELINK,
	MLG_INSURANCETITLE, //LargeTitle
	MLG_LARGEFLORISTSYMBOL, //LargeSymbol
	MLG_LOADSAVEHEADER, //LoadScreenAddOns
	MLG_MCGILLICUTTYS,
	MLG_MORTUARY,
	MLG_MORTUARYLINK,
	MLG_OPTIONHEADER, //OptionScreenAddOns
	MLG_ORDERGRID,
	MLG_PREBATTLEPANEL,
	MLG_SMALLFLORISTSYMBOL, //SmallSymbol
	MLG_SMALLTITLE,
	MLG_SPLASH,
	MLG_STATSBOX,
	MLG_STOREPLAQUE,
	MLG_TITLETEXT,
	MLG_TOALUMNI,
	MLG_TOMUGSHOTS,
	MLG_TOSTATS,
	MLG_WARNING,
	MLG_YOURAD13,
	MSG__LAST
};

char const* GetMLGFilename(MultiLanguageGraphic);

/** Choose game version. */
void setGameVersion(GameVersion ver);

/** Get list of resource libraries. */
std::vector<std::string> GetResourceLibraries(const std::string &dataDir);

/**
 * Get encoding corrector for strings in data files.
 * @return NULL when no encoding corrector is required */
const IEncodingCorrector* getDataFilesEncodingCorrector();

/** Check if this is English version of the game. */
bool isEnglishVersion();

/** Check if this is German version of the game. */
bool isGermanVersion();

/** Check if this is Polish version of the game. */
bool isPolishVersion();

/** Check if this is Russian version of the game. */
bool isRussianVersion();

/** Check if this is Russian GOLD version of the game. */
bool isRussianGoldVersion();

/** Get major map version. */
FLOAT getMajorMapVersion();

wchar_t getZeroGlyphChar();

/*************************************************************
 *
 ************************************************************/

/** Get current string encoding type. */
STRING_ENC_TYPE getStringEncType();
