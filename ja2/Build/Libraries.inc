static const char* gGameLibaries[] =
{
	"data.slf",
	"ambient.slf",
	"anims.slf",
	"battlesnds.slf",
	"bigitems.slf",
	"binarydata.slf",
	"cursors.slf",
	"faces.slf",
	"fonts.slf",
	"interface.slf",
	"laptop.slf",
	"maps.slf",
	"mercedt.slf",
	"music.slf",
	"npc_speech.slf",
	"npcdata.slf",
	"radarmaps.slf",
	"sounds.slf",
	"speech.slf",
	"tilesets.slf",
	"loadscreens.slf",
#if 0 // XXX the intro videos are not needed right now, because there is no way to play them
	"intro.slf",
#endif

#ifdef JA2DEMO
	"demoads.slf",
#endif

#ifdef GERMAN
	"german.slf",
#endif

#ifdef POLISH
	"polish.slf",
#endif

#ifdef DUTCH
	"dutch.slf",
#endif

#ifdef ITALIAN
	"italian.slf",
#endif

#if defined RUSSIAN && !defined RUSSIAN_GOLD
	"russian.slf",
#endif

#ifdef JA2EDITOR
	"editor.slf"
#endif
};
