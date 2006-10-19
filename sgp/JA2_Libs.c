LibraryInitHeader gGameLibaries[ ] =
{
		//Library Name					Can be	Init at start
//													on cd
	{ "Data.slf",							FALSE, TRUE },
	{ "Ambient.slf",					FALSE, TRUE },
	{ "Anims.slf",						FALSE, TRUE },
	{ "BattleSnds.slf",				FALSE, TRUE },
	{ "BigItems.slf",					FALSE, TRUE },
	{ "BinaryData.slf",				FALSE, TRUE },
	{ "Cursors.slf",					FALSE, TRUE },
	{ "Faces.slf",						FALSE, TRUE },
	{ "Fonts.slf",						FALSE, TRUE },
	{ "InterFace.slf",				FALSE, TRUE },
	{ "Laptop.slf",						FALSE, TRUE },
	{ "Maps.slf",							TRUE,  TRUE },
	{ "MercEdt.slf",					FALSE, TRUE },
	{ "Music.slf",						TRUE,  TRUE },
	{ "Npc_Speech.slf",				TRUE,	 TRUE },
	{ "NpcData.slf",					FALSE, TRUE },
	{ "RadarMaps.slf",				FALSE, TRUE },
	{ "Sounds.slf",						FALSE, TRUE },
	{ "Speech.slf",						TRUE,  TRUE },
//	{ "TileCache.slf",				FALSE, TRUE },
	{ "TileSets.slf",					TRUE,  TRUE },
	{ "LoadScreens.slf",			TRUE,  TRUE },
	{ "Intro.slf",						TRUE,  TRUE },

#ifdef JA2DEMO
	{ "DemoAds.slf",					FALSE, TRUE },
#endif

#ifdef GERMAN
	{ "German.slf",						FALSE, TRUE },
#endif

#ifdef POLISH
	{ "Polish.slf",						FALSE, TRUE },
#endif

#ifdef DUTCH
	{ "Dutch.slf",						FALSE, TRUE },
#endif

#ifdef ITALIAN
	{ "Italian.slf",					FALSE, TRUE },
#endif

#ifdef RUSSIAN
	{ "Russian.slf",					FALSE, TRUE },
#endif

#ifdef JA2EDITOR
	{ "Editor.slf",           FALSE, TRUE }
#endif

};
