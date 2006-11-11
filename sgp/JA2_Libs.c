LibraryInitHeader gGameLibaries[ ] =
{
		//Library Name					Can be	Init at start
//													on cd
	{ "Data.slf",							TRUE, TRUE },
	{ "Ambient.slf",					TRUE, TRUE },
	{ "Anims.slf",						TRUE, TRUE },
	{ "BattleSnds.slf",				TRUE, TRUE },
	{ "BigItems.slf",					TRUE, TRUE },
	{ "BinaryData.slf",				TRUE, TRUE },
	{ "Cursors.slf",					TRUE, TRUE },
	{ "Faces.slf",						TRUE, TRUE },
	{ "Fonts.slf",						TRUE, TRUE },
	{ "InterFace.slf",				TRUE, TRUE },
	{ "Laptop.slf",						TRUE, TRUE },
	{ "Maps.slf",							TRUE, TRUE },
	{ "MercEdt.slf",					TRUE, TRUE },
	{ "Music.slf",						TRUE, TRUE },
	{ "Npc_Speech.slf",				TRUE,	TRUE },
	{ "NpcData.slf",					TRUE, TRUE },
	{ "RadarMaps.slf",				TRUE, TRUE },
	{ "Sounds.slf",						TRUE, TRUE },
	{ "Speech.slf",						TRUE, TRUE },
//	{ "TileCache.slf",				TRUE, TRUE },
	{ "TileSets.slf",					TRUE, TRUE },
	{ "LoadScreens.slf",			TRUE, TRUE },
	{ "Intro.slf",						TRUE, TRUE },

#ifdef JA2DEMO
	{ "DemoAds.slf",					TRUE, TRUE },
#endif

#ifdef GERMAN
	{ "German.slf",						TRUE, TRUE },
#endif

#ifdef POLISH
	{ "Polish.slf",						TRUE, TRUE },
#endif

#ifdef DUTCH
	{ "Dutch.slf",						TRUE, TRUE },
#endif

#ifdef ITALIAN
	{ "Italian.slf",					TRUE, TRUE },
#endif

#ifdef RUSSIAN
	{ "Russian.slf",					TRUE, TRUE },
#endif

#ifdef JA2EDITOR
	{ "Editor.slf",           TRUE, TRUE }
#endif

};
