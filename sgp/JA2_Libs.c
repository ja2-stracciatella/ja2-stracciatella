LibraryInitHeader gGameLibaries[ ] =
{
		//Library Name					Can be	Init at start
//													on cd
	{ "data.slf",							TRUE, TRUE },
	{ "ambient.slf",					TRUE, TRUE },
	{ "anims.slf",						TRUE, TRUE },
	{ "battlesnds.slf",				TRUE, TRUE },
	{ "bigitems.slf",					TRUE, TRUE },
	{ "binarydata.slf",				TRUE, TRUE },
	{ "cursors.slf",					TRUE, TRUE },
	{ "faces.slf",						TRUE, TRUE },
	{ "fonts.slf",						TRUE, TRUE },
	{ "interface.slf",				TRUE, TRUE },
	{ "laptop.slf",						TRUE, TRUE },
	{ "maps.slf",							TRUE, TRUE },
	{ "mercedt.slf",					TRUE, TRUE },
	{ "music.slf",						TRUE, TRUE },
	{ "npc_speech.slf",				TRUE,	TRUE },
	{ "npcdata.slf",					TRUE, TRUE },
	{ "radarmaps.slf",				TRUE, TRUE },
	{ "sounds.slf",						TRUE, TRUE },
	{ "speech.slf",						TRUE, TRUE },
//	{ "tilecache.slf",				TRUE, TRUE },
	{ "tilesets.slf",					TRUE, TRUE },
	{ "loadscreens.slf",			TRUE, TRUE },
	{ "intro.slf",						TRUE, TRUE },

#ifdef JA2DEMO
	{ "demoads.slf",					TRUE, TRUE },
#endif

#ifdef GERMAN
	{ "german.slf",						TRUE, TRUE },
#endif

#ifdef POLISH
	{ "polish.slf",						TRUE, TRUE },
#endif

#ifdef DUTCH
	{ "dutch.slf",						TRUE, TRUE },
#endif

#ifdef ITALIAN
	{ "italian.slf",					TRUE, TRUE },
#endif

#ifdef RUSSIAN
	{ "russian.slf",					TRUE, TRUE },
#endif

#ifdef JA2EDITOR
	{ "editor.slf",           TRUE, TRUE }
#endif

};
