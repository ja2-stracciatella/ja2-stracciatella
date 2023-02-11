#include "TilesetTileIndexModel.h"

TilesetTileIndexModel::TilesetTileIndexModel(TileTypeDefines tileType_, uint16_t subIndex_) : tileType(tileType_), subIndex(subIndex_) {
}

TileTypeDefines deserializeTileTypeDefine(const ST::string& str);
ST::string serializeTileTypeDefine(TileTypeDefines t);

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

TileTypeDefines deserializeTileTypeDefine(const ST::string& str) {
	  if (str == "FIRSTTEXTURE") return TileTypeDefines::FIRSTTEXTURE;
	  if (str == "SECONDTEXTURE") return TileTypeDefines::SECONDTEXTURE;
	  if (str == "THIRDTEXTURE") return TileTypeDefines::THIRDTEXTURE;
	  if (str == "FOURTHTEXTURE") return TileTypeDefines::FOURTHTEXTURE;
	  if (str == "FIFTHTEXTURE") return TileTypeDefines::FIFTHTEXTURE;
	  if (str == "SIXTHTEXTURE") return TileTypeDefines::SIXTHTEXTURE;
	  if (str == "SEVENTHTEXTURE") return TileTypeDefines::SEVENTHTEXTURE;
	  if (str == "REGWATERTEXTURE") return TileTypeDefines::REGWATERTEXTURE;
	  if (str == "DEEPWATERTEXTURE") return TileTypeDefines::DEEPWATERTEXTURE;
	  if (str == "FIRSTCLIFFHANG") return TileTypeDefines::FIRSTCLIFFHANG;
	  if (str == "FIRSTCLIFF") return TileTypeDefines::FIRSTCLIFF;
	  if (str == "FIRSTCLIFFSHADOW") return TileTypeDefines::FIRSTCLIFFSHADOW;
	  if (str == "FIRSTOSTRUCT") return TileTypeDefines::FIRSTOSTRUCT;
	  if (str == "SECONDOSTRUCT") return TileTypeDefines::SECONDOSTRUCT;
	  if (str == "THIRDOSTRUCT") return TileTypeDefines::THIRDOSTRUCT;
	  if (str == "FOURTHOSTRUCT") return TileTypeDefines::FOURTHOSTRUCT;
	  if (str == "FIFTHOSTRUCT") return TileTypeDefines::FIFTHOSTRUCT;
	  if (str == "SIXTHOSTRUCT") return TileTypeDefines::SIXTHOSTRUCT;
	  if (str == "SEVENTHOSTRUCT") return TileTypeDefines::SEVENTHOSTRUCT;
	  if (str == "EIGHTOSTRUCT") return TileTypeDefines::EIGHTOSTRUCT;
	  if (str == "FIRSTFULLSTRUCT") return TileTypeDefines::FIRSTFULLSTRUCT;
	  if (str == "SECONDFULLSTRUCT") return TileTypeDefines::SECONDFULLSTRUCT;
	  if (str == "THIRDFULLSTRUCT") return TileTypeDefines::THIRDFULLSTRUCT;
	  if (str == "FOURTHFULLSTRUCT") return TileTypeDefines::FOURTHFULLSTRUCT;
	  if (str == "FIRSTSHADOW") return TileTypeDefines::FIRSTSHADOW;
	  if (str == "SECONDSHADOW") return TileTypeDefines::SECONDSHADOW;
	  if (str == "THIRDSHADOW") return TileTypeDefines::THIRDSHADOW;
	  if (str == "FOURTHSHADOW") return TileTypeDefines::FOURTHSHADOW;
	  if (str == "FIFTHSHADOW") return TileTypeDefines::FIFTHSHADOW;
	  if (str == "SIXTHSHADOW") return TileTypeDefines::SIXTHSHADOW;
	  if (str == "SEVENTHSHADOW") return TileTypeDefines::SEVENTHSHADOW;
	  if (str == "EIGHTSHADOW") return TileTypeDefines::EIGHTSHADOW;
	  if (str == "FIRSTFULLSHADOW") return TileTypeDefines::FIRSTFULLSHADOW;
	  if (str == "SECONDFULLSHADOW") return TileTypeDefines::SECONDFULLSHADOW;
	  if (str == "THIRDFULLSHADOW") return TileTypeDefines::THIRDFULLSHADOW;
	  if (str == "FOURTHFULLSHADOW") return TileTypeDefines::FOURTHFULLSHADOW;
	  if (str == "FIRSTWALL") return TileTypeDefines::FIRSTWALL;
	  if (str == "SECONDWALL") return TileTypeDefines::SECONDWALL;
	  if (str == "THIRDWALL") return TileTypeDefines::THIRDWALL;
	  if (str == "FOURTHWALL") return TileTypeDefines::FOURTHWALL;
	  if (str == "FIRSTDOOR") return TileTypeDefines::FIRSTDOOR;
	  if (str == "SECONDDOOR") return TileTypeDefines::SECONDDOOR;
	  if (str == "THIRDDOOR") return TileTypeDefines::THIRDDOOR;
	  if (str == "FOURTHDOOR") return TileTypeDefines::FOURTHDOOR;
	  if (str == "FIRSTDOORSHADOW") return TileTypeDefines::FIRSTDOORSHADOW;
	  if (str == "SECONDDOORSHADOW") return TileTypeDefines::SECONDDOORSHADOW;
	  if (str == "THIRDDOORSHADOW") return TileTypeDefines::THIRDDOORSHADOW;
	  if (str == "FOURTHDOORSHADOW") return TileTypeDefines::FOURTHDOORSHADOW;
	  if (str == "SLANTROOFCEILING") return TileTypeDefines::SLANTROOFCEILING;
	  if (str == "ANOTHERDEBRIS") return TileTypeDefines::ANOTHERDEBRIS;
	  if (str == "ROADPIECES") return TileTypeDefines::ROADPIECES;
	  if (str == "FOURTHWINDOW") return TileTypeDefines::FOURTHWINDOW;
	  if (str == "FIRSTDECORATIONS") return TileTypeDefines::FIRSTDECORATIONS;
	  if (str == "SECONDDECORATIONS") return TileTypeDefines::SECONDDECORATIONS;
	  if (str == "THIRDDECORATIONS") return TileTypeDefines::THIRDDECORATIONS;
	  if (str == "FOURTHDECORATIONS") return TileTypeDefines::FOURTHDECORATIONS;
	  if (str == "FIRSTWALLDECAL") return TileTypeDefines::FIRSTWALLDECAL;
	  if (str == "SECONDWALLDECAL") return TileTypeDefines::SECONDWALLDECAL;
	  if (str == "THIRDWALLDECAL") return TileTypeDefines::THIRDWALLDECAL;
	  if (str == "FOURTHWALLDECAL") return TileTypeDefines::FOURTHWALLDECAL;
	  if (str == "FIRSTFLOOR") return TileTypeDefines::FIRSTFLOOR;
	  if (str == "SECONDFLOOR") return TileTypeDefines::SECONDFLOOR;
	  if (str == "THIRDFLOOR") return TileTypeDefines::THIRDFLOOR;
	  if (str == "FOURTHFLOOR") return TileTypeDefines::FOURTHFLOOR;
	  if (str == "FIRSTROOF") return TileTypeDefines::FIRSTROOF;
	  if (str == "SECONDROOF") return TileTypeDefines::SECONDROOF;
	  if (str == "THIRDROOF") return TileTypeDefines::THIRDROOF;
	  if (str == "FOURTHROOF") return TileTypeDefines::FOURTHROOF;
	  if (str == "FIRSTSLANTROOF") return TileTypeDefines::FIRSTSLANTROOF;
	  if (str == "SECONDSLANTROOF") return TileTypeDefines::SECONDSLANTROOF;
	  if (str == "FIRSTONROOF") return TileTypeDefines::FIRSTONROOF;
	  if (str == "SECONDONROOF") return TileTypeDefines::SECONDONROOF;
	  if (str == "MOCKFLOOR") return TileTypeDefines::MOCKFLOOR;
	  if (str == "FIRSTISTRUCT") return TileTypeDefines::FIRSTISTRUCT;
	  if (str == "SECONDISTRUCT") return TileTypeDefines::SECONDISTRUCT;
	  if (str == "THRIDISTRUCT") return TileTypeDefines::THRIDISTRUCT;
	  if (str == "FOURTHISTRUCT") return TileTypeDefines::FOURTHISTRUCT;
	  if (str == "FIRSTCISTRUCT") return TileTypeDefines::FIRSTCISTRUCT;
	  if (str == "FIRSTROAD") return TileTypeDefines::FIRSTROAD;
	  if (str == "DEBRISROCKS") return TileTypeDefines::DEBRISROCKS;
	  if (str == "DEBRISWOOD") return TileTypeDefines::DEBRISWOOD;
	  if (str == "DEBRISWEEDS") return TileTypeDefines::DEBRISWEEDS;
	  if (str == "DEBRISGRASS") return TileTypeDefines::DEBRISGRASS;
	  if (str == "DEBRISSAND") return TileTypeDefines::DEBRISSAND;
	  if (str == "DEBRISMISC") return TileTypeDefines::DEBRISMISC;
	  if (str == "ANIOSTRUCT") return TileTypeDefines::ANIOSTRUCT;
	  if (str == "FENCESTRUCT") return TileTypeDefines::FENCESTRUCT;
	  if (str == "FENCESHADOW") return TileTypeDefines::FENCESHADOW;
	  if (str == "FIRSTVEHICLE") return TileTypeDefines::FIRSTVEHICLE;
	  if (str == "SECONDVEHICLE") return TileTypeDefines::SECONDVEHICLE;
	  if (str == "FIRSTVEHICLESHADOW") return TileTypeDefines::FIRSTVEHICLESHADOW;
	  if (str == "SECONDVEHICLESHADOW") return TileTypeDefines::SECONDVEHICLESHADOW;
	  if (str == "DEBRIS2MISC") return TileTypeDefines::DEBRIS2MISC;
	  if (str == "FIRSTDEBRISSTRUCT") return TileTypeDefines::FIRSTDEBRISSTRUCT;
	  if (str == "SECONDDEBRISSTRUCT") return TileTypeDefines::SECONDDEBRISSTRUCT;
	  if (str == "FIRSTDEBRISSTRUCTSHADOW") return TileTypeDefines::FIRSTDEBRISSTRUCTSHADOW;
	  if (str == "SECONDDEBRISSTRUCTSHADOW") return TileTypeDefines::SECONDDEBRISSTRUCTSHADOW;
	  if (str == "NINTHOSTRUCT") return TileTypeDefines::NINTHOSTRUCT;
	  if (str == "TENTHOSTRUCT") return TileTypeDefines::TENTHOSTRUCT;
	  if (str == "NINTHOSTRUCTSHADOW") return TileTypeDefines::NINTHOSTRUCTSHADOW;
	  if (str == "TENTHOSTRUCTSHADOW") return TileTypeDefines::TENTHOSTRUCTSHADOW;
	  if (str == "FIRSTEXPLDEBRIS") return TileTypeDefines::FIRSTEXPLDEBRIS;
	  if (str == "SECONDEXPLDEBRIS") return TileTypeDefines::SECONDEXPLDEBRIS;
	  if (str == "FIRSTLARGEEXPDEBRIS") return TileTypeDefines::FIRSTLARGEEXPDEBRIS;
	  if (str == "SECONDLARGEEXPDEBRIS") return TileTypeDefines::SECONDLARGEEXPDEBRIS;
	  if (str == "FIRSTLARGEEXPDEBRISSHADOW") return TileTypeDefines::FIRSTLARGEEXPDEBRISSHADOW;
	  if (str == "SECONDLARGEEXPDEBRISSHADOW") return TileTypeDefines::SECONDLARGEEXPDEBRISSHADOW;
	  if (str == "FIFTHISTRUCT") return TileTypeDefines::FIFTHISTRUCT;
	  if (str == "SIXTHISTRUCT") return TileTypeDefines::SIXTHISTRUCT;
	  if (str == "SEVENTHISTRUCT") return TileTypeDefines::SEVENTHISTRUCT;
	  if (str == "EIGHTISTRUCT") return TileTypeDefines::EIGHTISTRUCT;
	  if (str == "FIRSTHIGHROOF") return TileTypeDefines::FIRSTHIGHROOF;
	  if (str == "SECONDHIGHROOF") return TileTypeDefines::SECONDHIGHROOF;
	  if (str == "FIFTHWALLDECAL") return TileTypeDefines::FIFTHWALLDECAL;
	  if (str == "SIXTHWALLDECAL") return TileTypeDefines::SIXTHWALLDECAL;
	  if (str == "SEVENTHWALLDECAL") return TileTypeDefines::SEVENTHWALLDECAL;
	  if (str == "EIGTHWALLDECAL") return TileTypeDefines::EIGTHWALLDECAL;
	  if (str == "HUMANBLOOD") return TileTypeDefines::HUMANBLOOD;
	  if (str == "CREATUREBLOOD") return TileTypeDefines::CREATUREBLOOD;
	  if (str == "FIRSTSWITCHES") return TileTypeDefines::FIRSTSWITCHES;
	  if (str == "REVEALEDSLANTROOFS") return TileTypeDefines::REVEALEDSLANTROOFS;
	  if (str == "FIRSTREVEALEDHIGHROOFS") return TileTypeDefines::FIRSTREVEALEDHIGHROOFS;
	  if (str == "SECONDREVEALEDHIGHROOFS") return TileTypeDefines::SECONDREVEALEDHIGHROOFS;
	  if (str == "GUNS") return TileTypeDefines::GUNS;
	  if (str == "P1ITEMS") return TileTypeDefines::P1ITEMS;
	  if (str == "P2ITEMS") return TileTypeDefines::P2ITEMS;
	  if (str == "WINDOWSHATTER") return TileTypeDefines::WINDOWSHATTER;
	  if (str == "P3ITEMS") return TileTypeDefines::P3ITEMS;
	  if (str == "BODYEXPLOSION") return TileTypeDefines::BODYEXPLOSION;
	  if (str == "EXITTEXTURE") return TileTypeDefines::EXITTEXTURE;
	  if (str == "FOOTPRINTS") return TileTypeDefines::FOOTPRINTS;
	  if (str == "FIRSTPOINTERS") return TileTypeDefines::FIRSTPOINTERS;
	  if (str == "SECONDPOINTERS") return TileTypeDefines::SECONDPOINTERS;
	  if (str == "THIRDPOINTERS") return TileTypeDefines::THIRDPOINTERS;
	  if (str == "GOODRUN") return TileTypeDefines::GOODRUN;
	  if (str == "GOODWALK") return TileTypeDefines::GOODWALK;
	  if (str == "GOODSWAT") return TileTypeDefines::GOODSWAT;
	  if (str == "GOODPRONE") return TileTypeDefines::GOODPRONE;
	  if (str == "CONFIRMMOVE") return TileTypeDefines::CONFIRMMOVE;
	  if (str == "VEHICLEMOVE") return TileTypeDefines::VEHICLEMOVE;
	  if (str == "ACTIONTWO") return TileTypeDefines::ACTIONTWO;
	  if (str == "XMARKER") return TileTypeDefines::XMARKER;
	  if (str == "GOODRING") return TileTypeDefines::GOODRING;
	  if (str == "ROTATINGKEY") return TileTypeDefines::ROTATINGKEY;
	  if (str == "SELRING") return TileTypeDefines::SELRING;
	  if (str == "SPECIALTILES") return TileTypeDefines::SPECIALTILES;
	  if (str == "BULLETTILE") return TileTypeDefines::BULLETTILE;
	  if (str == "FIRSTMISS") return TileTypeDefines::FIRSTMISS;
	  if (str == "SECONDMISS") return TileTypeDefines::SECONDMISS;
	  if (str == "THIRDMISS") return TileTypeDefines::THIRDMISS;
	  if (str == "WIREFRAMES") return TileTypeDefines::WIREFRAMES;

	auto errorMessage = ST::format("unknown tile type: `{}`", str);
	throw std::runtime_error(errorMessage.c_str());
}

ST::string serializeTileTypeDefine(TileTypeDefines t) {
	return "";
}
