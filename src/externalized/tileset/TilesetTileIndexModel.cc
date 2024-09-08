#include "TilesetTileIndexModel.h"
#include "TileDat.h"
#include <array>
#include <stdexcept>
#include <string_theory/string>

namespace {
constexpr std::array<char const *, TileTypeDefines::NUMBEROFTILETYPES> namesArray
{
// These strings must be in the correct order, see TileDat.h.
	"FIRSTTEXTURE",
	"SECONDTEXTURE",
	"THIRDTEXTURE",
	"FOURTHTEXTURE",
	"FIFTHTEXTURE",
	"SIXTHTEXTURE",
	"SEVENTHTEXTURE",
	"REGWATERTEXTURE",
	"DEEPWATERTEXTURE",
	"FIRSTCLIFFHANG",
	"FIRSTCLIFF",
	"FIRSTCLIFFSHADOW",
	"FIRSTOSTRUCT",
	"SECONDOSTRUCT",
	"THIRDOSTRUCT",
	"FOURTHOSTRUCT",
	"FIFTHOSTRUCT",
	"SIXTHOSTRUCT",
	"SEVENTHOSTRUCT",
	"EIGHTOSTRUCT",
	"FIRSTFULLSTRUCT",
	"SECONDFULLSTRUCT",
	"THIRDFULLSTRUCT",
	"FOURTHFULLSTRUCT",
	"FIRSTSHADOW",
	"SECONDSHADOW",
	"THIRDSHADOW",
	"FOURTHSHADOW",
	"FIFTHSHADOW",
	"SIXTHSHADOW",
	"SEVENTHSHADOW",
	"EIGHTSHADOW",
	"FIRSTFULLSHADOW",
	"SECONDFULLSHADOW",
	"THIRDFULLSHADOW",
	"FOURTHFULLSHADOW",
	"FIRSTWALL",
	"SECONDWALL",
	"THIRDWALL",
	"FOURTHWALL",
	"FIRSTDOOR",
	"SECONDDOOR",
	"THIRDDOOR",
	"FOURTHDOOR",
	"FIRSTDOORSHADOW",
	"SECONDDOORSHADOW",
	"THIRDDOORSHADOW",
	"FOURTHDOORSHADOW",
	"SLANTROOFCEILING",
	"ANOTHERDEBRIS",
	"ROADPIECES",
	"FOURTHWINDOW",
	"FIRSTDECORATIONS",
	"SECONDDECORATIONS",
	"THIRDDECORATIONS",
	"FOURTHDECORATIONS",
	"FIRSTWALLDECAL",
	"SECONDWALLDECAL",
	"THIRDWALLDECAL",
	"FOURTHWALLDECAL",
	"FIRSTFLOOR",
	"SECONDFLOOR",
	"THIRDFLOOR",
	"FOURTHFLOOR",
	"FIRSTROOF",
	"SECONDROOF",
	"THIRDROOF",
	"FOURTHROOF",
	"FIRSTSLANTROOF",
	"SECONDSLANTROOF",
	"FIRSTONROOF",
	"SECONDONROOF",
	"MOCKFLOOR",
	"FIRSTISTRUCT",
	"SECONDISTRUCT",
	"THRIDISTRUCT",
	"FOURTHISTRUCT",
	"FIRSTCISTRUCT",
	"FIRSTROAD",
	"DEBRISROCKS",
	"DEBRISWOOD",
	"DEBRISWEEDS",
	"DEBRISGRASS",
	"DEBRISSAND",
	"DEBRISMISC",
	"ANIOSTRUCT",
	"FENCESTRUCT",
	"FENCESHADOW",
	"FIRSTVEHICLE",
	"SECONDVEHICLE",
	"FIRSTVEHICLESHADOW",
	"SECONDVEHICLESHADOW",
	"DEBRIS2MISC",
	"FIRSTDEBRISSTRUCT",
	"SECONDDEBRISSTRUCT",
	"FIRSTDEBRISSTRUCTSHADOW",
	"SECONDDEBRISSTRUCTSHADOW",
	"NINTHOSTRUCT",
	"TENTHOSTRUCT",
	"NINTHOSTRUCTSHADOW",
	"TENTHOSTRUCTSHADOW",
	"FIRSTEXPLDEBRIS",
	"SECONDEXPLDEBRIS",
	"FIRSTLARGEEXPDEBRIS",
	"SECONDLARGEEXPDEBRIS",
	"FIRSTLARGEEXPDEBRISSHADOW",
	"SECONDLARGEEXPDEBRISSHADOW",
	"FIFTHISTRUCT",
	"SIXTHISTRUCT",
	"SEVENTHISTRUCT",
	"EIGHTISTRUCT",
	"FIRSTHIGHROOF",
	"SECONDHIGHROOF",
	"FIFTHWALLDECAL",
	"SIXTHWALLDECAL",
	"SEVENTHWALLDECAL",
	"EIGTHWALLDECAL",
	"HUMANBLOOD",
	"CREATUREBLOOD",
	"FIRSTSWITCHES",
	"REVEALEDSLANTROOFS",
	"FIRSTREVEALEDHIGHROOFS",
	"SECONDREVEALEDHIGHROOFS",
	"GUNS",
	"P1ITEMS",
	"P2ITEMS",
	"WINDOWSHATTER",
	"P3ITEMS",
	"BODYEXPLOSION",
	"EXITTEXTURE",
	"FOOTPRINTS",
	"FIRSTPOINTERS",
	"SECONDPOINTERS",
	"THIRDPOINTERS",
	"GOODRUN",
	"GOODWALK",
	"GOODSWAT",
	"GOODPRONE",
	"CONFIRMMOVE",
	"VEHICLEMOVE",
	"ACTIONTWO",
	"XMARKER",
	"GOODRING",
	"ROTATINGKEY",
	"SELRING",
	"SPECIALTILES",
	"BULLETTILE",
	"FIRSTMISS",
	"SECONDMISS",
	"THIRDMISS",
	"WIREFRAMES"
};

TileTypeDefines deserializeTileTypeDefine(ST::string const& str)
{
	// This is a really simple, brute-force way to get a TileTypeDefines value
	// for a string, but since this function is only called once per item at
	// startup it's hardly worth it to "optimize" this.
	auto const posInArray{ std::find_if(
		namesArray.begin(), namesArray.end(),
		[&str](auto name) {
			return str.compare(name) == 0;
		})
	};

	if (posInArray == namesArray.end()) throw std::runtime_error("Unknown TileType");

	return static_cast<TileTypeDefines>(std::distance(namesArray.begin(), posInArray));
}

ST::string serializeTileTypeDefine(TileTypeDefines tt)
{
	return tt < TileTypeDefines::NUMBEROFTILETYPES
		? namesArray[tt]
		: "UNKNOWNTILETYPE";
}
}

TilesetTileIndexModel TilesetTileIndexModel::deserialize(const JsonValue &json) {
	auto obj = json.toObject();
	return TilesetTileIndexModel(
		deserializeTileTypeDefine(obj.GetString("type")),
		obj.GetUInt("subIndex")
	);
}

JsonValue TilesetTileIndexModel::serialize() const {
	JsonObject v;

	v.set("path", serializeTileTypeDefine(tileType));
	v.set("subIndex", subIndex);

	return v.toValue();
}

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(TilesetTileIndexModel, allArraySlotsUsed)
{
	EXPECT_EQ(std::count(namesArray.begin(), namesArray.end(), nullptr), 0);
}

TEST(TilesetTileIndexModel, deserialize)
{
	EXPECT_EQ(deserializeTileTypeDefine("XMARKER"), TileTypeDefines::XMARKER);
	EXPECT_THROW(deserializeTileTypeDefine("random nonsense"), std::runtime_error);
}

TEST(TilesetTileIndexModel, serialize)
{
	EXPECT_EQ(serializeTileTypeDefine(TileTypeDefines::DEBRIS2MISC).compare("DEBRIS2MISC"), 0);
	EXPECT_EQ(serializeTileTypeDefine(TileTypeDefines(8765)).compare("UNKNOWNTILETYPE"), 0);

	for (int i{ 0 }; i < TileTypeDefines::NUMBEROFTILETYPES; ++i)
	{  
		EXPECT_NE(serializeTileTypeDefine(TileTypeDefines(i)).compare("UNKNOWNTILETYPE"), 0);
	}
}
#endif
