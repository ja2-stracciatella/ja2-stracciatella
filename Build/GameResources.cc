#include "LibraryDataBase.h"

#ifndef ENGLISH
# include "Multi_Language_Graphic_Utils.h"
#endif

static const char* gGameLibaries[] =
{
	"data.slf",
#ifndef JA2DEMO
	"ambient.slf",
#endif
	"anims.slf",
	"battlesnds.slf",
	"bigitems.slf",
	"binarydata.slf",
	"cursors.slf",
	"faces.slf",
	"fonts.slf",
	"interface.slf",
#ifndef JA2DEMO
	"laptop.slf",
#endif
	"maps.slf",
	"mercedt.slf",
	"music.slf",
	"npc_speech.slf",
	"npcdata.slf",
	"radarmaps.slf",
	"sounds.slf",
	"speech.slf",
	"tilesets.slf",
#ifndef JA2DEMO
	"loadscreens.slf",
#endif
#if 0 // XXX the intro videos are not needed right now, because there is no way to play them
	"intro.slf",
#endif

#if defined JA2DEMO
#	if defined JA2DEMOADS
	"demoads.slf",
#	endif
#elif defined DUTCH
	"dutch.slf",
#elif defined GERMAN
	"german.slf",
#elif defined ITALIAN
	"italian.slf",
#elif defined POLISH
	"polish.slf",
#elif defined RUSSIAN
	"russian.slf",
#endif

#ifdef JA2EDITOR
	"editor.slf"
#endif
};

void InitGameResources(void)
{
  InitializeFileDatabase(gGameLibaries, lengthof(gGameLibaries));
}
