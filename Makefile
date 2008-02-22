CONFIG ?= config.default
-include $(CONFIG)


ifeq ($(findstring $(LNG), DUTCH ENGLISH FRENCH GERMAN ITALIAN POLISH RUSSIAN RUSSIAN_GOLD),)
$(error LNG must be set to one of DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN or RUSSIAN_GOLD. Copy config.template to config.default and uncomment one of the languages)
endif

ifndef SGPDATADIR
$(warn No SGPDATADIR specified, make lowercase will not work)
endif


BINARY    ?= ja2
PREFIX    ?= /usr/local
MANPREFIX ?= $(PREFIX)

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 555 -s
INSTALL_MAN     ?= $(INSTALL) -m 444
INSTALL_DATA    ?= $(INSTALL) -m 444


SDL_CONFIG  ?= sdl-config
CFLAGS_SDL  ?= $(shell $(SDL_CONFIG) --cflags)
LDFLAGS_SDL ?= $(shell $(SDL_CONFIG) --libs)


CFLAGS += $(CFLAGS_SDL)
CFLAGS += -I Build
CFLAGS += -I Build/Tactical
CFLAGS += -I Build/Strategic
CFLAGS += -I Build/Editor
CFLAGS += -I Build/Res
CFLAGS += -I Build/Laptop
CFLAGS += -I Build/Utils
CFLAGS += -I Build/TileEngine
CFLAGS += -I Build/TacticalAI
CFLAGS += -I sgp

#CFLAGS += -Wall
#CFLAGS += -W
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings

CFLAGS += -DJA2
#CFLAGS += -D_DEBUG


ifdef WITH_DEMO
CFLAGS += -DJA2DEMO
endif

ifdef WITH_DEMO_ADS
ifndef WITH_DEMO
$(error WITH_DEMO_ADS needs WITH_DEMO)
endif
CFLAGS += -DJA2DEMOADS
endif

ifdef WITH_FIXMES
CFLAGS += -DWITH_FIXMES
endif

ifdef WITH_MAEMO
CFLAGS += -DWITH_MAEMO
endif

ifdef WITH_SOUND_DEBUG
CFLAGS += -DWITH_SOUND_DEBUG
endif

ifdef WITH_ZLIB
CFLAGS += -DWITH_ZLIB
endif

ifdef JA2BETAVERSION
CFLAGS += -DJA2BETAVERSION
endif

ifdef JA2TESTVERSION
CFLAGS += -DJA2TESTVERSION
endif

ifdef JA2EDITOR
CFLAGS += -DJA2EDITOR
endif

CFLAGS += -D$(LNG)

CFLAGS += -DSGPDATADIR=\"$(SGPDATADIR)\"

CCFLAGS += $(CFLAGS)
CCFLAGS += -std=gnu99
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wsequence-point

CXXFLAGS += $(CFLAGS)

LDFLAGS += $(LDFLAGS_SDL)
LDFLAGS += -lm

ifdef WITH_ZLIB
LDFLAGS += -lz
endif

SRCS :=
SRCS += Build/AniViewScreen.c
SRCS += Build/Credits.c

ifdef JA2EDITOR
SRCS += Build/Editor/Cursor_Modes.c
SRCS += Build/Editor/EditScreen.c
SRCS += Build/Editor/Edit_Sys.c
SRCS += Build/Editor/EditorBuildings.c
SRCS += Build/Editor/EditorItems.c
SRCS += Build/Editor/EditorMapInfo.c
SRCS += Build/Editor/EditorMercs.c
SRCS += Build/Editor/EditorTerrain.c
SRCS += Build/Editor/Editor_Callbacks.c
SRCS += Build/Editor/Editor_Modes.c
SRCS += Build/Editor/Editor_Taskbar_Creation.c
SRCS += Build/Editor/Editor_Taskbar_Utils.c
SRCS += Build/Editor/Editor_Undo.c
SRCS += Build/Editor/Item_Statistics.c
SRCS += Build/Editor/LoadScreen.c
SRCS += Build/Editor/MessageBox.c
SRCS += Build/Editor/NewSmooth.c
SRCS += Build/Editor/PopupMenu.c
SRCS += Build/Editor/Road_Smoothing.c
SRCS += Build/Editor/Sector_Summary.c
SRCS += Build/Editor/SelectWin.c
SRCS += Build/Editor/SmartMethod.c
SRCS += Build/Editor/Smooth.c
SRCS += Build/Editor/Smoothing_Utils.c
endif

SRCS += Build/Fade_Screen.c
SRCS += Build/GameInitOptionsScreen.c
SRCS += Build/GameLoop.c
SRCS += Build/GameScreen.c
SRCS += Build/GameSettings.c
SRCS += Build/GameVersion.c
SRCS += Build/HelpScreen.c
SRCS += Build/Init.c
SRCS += Build/Intro.c
SRCS += Build/JA2_Splash.c
SRCS += Build/JAScreens.c
SRCS += Build/Laptop/AIM.c
SRCS += Build/Laptop/AIMArchives.c
SRCS += Build/Laptop/AIMFacialIndex.c
SRCS += Build/Laptop/AIMHistory.c
SRCS += Build/Laptop/AIMPolicies.c
SRCS += Build/Laptop/AIMSort.c
SRCS += Build/Laptop/AimLinks.c
SRCS += Build/Laptop/AimMembers.c
SRCS += Build/Laptop/BobbyR.c
SRCS += Build/Laptop/BobbyRAmmo.c
SRCS += Build/Laptop/BobbyRArmour.c
SRCS += Build/Laptop/BobbyRGuns.c
SRCS += Build/Laptop/BobbyRMailOrder.c
SRCS += Build/Laptop/BobbyRMisc.c
SRCS += Build/Laptop/BobbyRShipments.c
SRCS += Build/Laptop/BobbyRUsed.c
SRCS += Build/Laptop/BrokenLink.c
SRCS += Build/Laptop/CharProfile.c
SRCS += Build/Laptop/EMail.c
SRCS += Build/Laptop/Files.c
SRCS += Build/Laptop/Finances.c
SRCS += Build/Laptop/Florist.c
SRCS += Build/Laptop/Florist_Cards.c
SRCS += Build/Laptop/Florist_Gallery.c
SRCS += Build/Laptop/Florist_Order_Form.c
SRCS += Build/Laptop/Funeral.c
SRCS += Build/Laptop/History.c
SRCS += Build/Laptop/IMPVideoObjects.c
SRCS += Build/Laptop/IMP_AboutUs.c
SRCS += Build/Laptop/IMP_Attribute_Entrance.c
SRCS += Build/Laptop/IMP_Attribute_Finish.c
SRCS += Build/Laptop/IMP_Attribute_Selection.c
SRCS += Build/Laptop/IMP_Begin_Screen.c
SRCS += Build/Laptop/IMP_Compile_Character.c
SRCS += Build/Laptop/IMP_Confirm.c
SRCS += Build/Laptop/IMP_Finish.c
SRCS += Build/Laptop/IMP_HomePage.c
SRCS += Build/Laptop/IMP_MainPage.c
SRCS += Build/Laptop/IMP_Personality_Entrance.c
SRCS += Build/Laptop/IMP_Personality_Finish.c
SRCS += Build/Laptop/IMP_Personality_Quiz.c
SRCS += Build/Laptop/IMP_Portraits.c
SRCS += Build/Laptop/IMP_Text_System.c
SRCS += Build/Laptop/IMP_Voices.c
SRCS += Build/Laptop/Insurance.c
SRCS += Build/Laptop/Insurance_Comments.c
SRCS += Build/Laptop/Insurance_Contract.c
SRCS += Build/Laptop/Insurance_Info.c
SRCS += Build/Laptop/Laptop.c
SRCS += Build/Laptop/Mercs.c
SRCS += Build/Laptop/Mercs_Account.c
SRCS += Build/Laptop/Mercs_Files.c
SRCS += Build/Laptop/Mercs_No_Account.c
SRCS += Build/Laptop/Personnel.c
SRCS += Build/Laptop/Store_Inventory.c
SRCS += Build/LoadSaveEMail.c
SRCS += Build/Loading_Screen.c
SRCS += Build/MainMenuScreen.c
SRCS += Build/MessageBoxScreen.c
SRCS += Build/Options_Screen.c
SRCS += Build/SaveLoadGame.c
SRCS += Build/SaveLoadScreen.c
SRCS += Build/Screens.c
SRCS += Build/Strategic/AI_Viewer.c
SRCS += Build/Strategic/Assignments.c
SRCS += Build/Strategic/Auto_Resolve.c
SRCS += Build/Strategic/Campaign_Init.c
SRCS += Build/Strategic/Creature_Spreading.c
SRCS += Build/Strategic/Game_Clock.c
SRCS += Build/Strategic/Game_Event_Hook.c
SRCS += Build/Strategic/Game_Events.c
SRCS += Build/Strategic/Game_Init.c
SRCS += Build/Strategic/Hourly_Update.c
SRCS += Build/Strategic/LoadSaveSectorInfo.c
SRCS += Build/Strategic/LoadSaveStrategicMapElement.c
SRCS += Build/Strategic/LoadSaveUndergroundSectorInfo.c
SRCS += Build/Strategic/MapScreen.c
SRCS += Build/Strategic/Map_Screen_Helicopter.c
SRCS += Build/Strategic/Map_Screen_Interface.c
SRCS += Build/Strategic/Map_Screen_Interface_Border.c
SRCS += Build/Strategic/Map_Screen_Interface_Bottom.c
SRCS += Build/Strategic/Map_Screen_Interface_Map.c
SRCS += Build/Strategic/Map_Screen_Interface_Map_Inventory.c
SRCS += Build/Strategic/Map_Screen_Interface_TownMine_Info.c
SRCS += Build/Strategic/Meanwhile.c
SRCS += Build/Strategic/Merc_Contract.c
SRCS += Build/Strategic/Player_Command.c
SRCS += Build/Strategic/PreBattle_Interface.c
SRCS += Build/Strategic/Queen_Command.c
SRCS += Build/Strategic/QuestText.c
SRCS += Build/Strategic/Quest_Debug_System.c
SRCS += Build/Strategic/Quests.c
SRCS += Build/Strategic/Scheduling.c
SRCS += Build/Strategic/Strategic.c
SRCS += Build/Strategic/StrategicMap.c
SRCS += Build/Strategic/Strategic_AI.c
SRCS += Build/Strategic/Strategic_Event_Handler.c
SRCS += Build/Strategic/Strategic_Merc_Handler.c
SRCS += Build/Strategic/Strategic_Mines.c
SRCS += Build/Strategic/Strategic_Movement.c
SRCS += Build/Strategic/Strategic_Movement_Costs.c
SRCS += Build/Strategic/Strategic_Pathing.c
SRCS += Build/Strategic/Strategic_Status.c
SRCS += Build/Strategic/Strategic_Town_Loyalty.c
SRCS += Build/Strategic/Strategic_Town_Reputation.c
SRCS += Build/Strategic/Strategic_Turns.c
SRCS += Build/Strategic/Town_Militia.c
SRCS += Build/Sys_Globals.c
SRCS += Build/Tactical/Air_Raid.c
SRCS += Build/Tactical/Animation_Cache.c
SRCS += Build/Tactical/Animation_Control.c
SRCS += Build/Tactical/Animation_Data.c
SRCS += Build/Tactical/ArmsDealerInvInit.c
SRCS += Build/Tactical/Arms_Dealer_Init.c
SRCS += Build/Tactical/Auto_Bandage.c
SRCS += Build/Tactical/Boxing.c
SRCS += Build/Tactical/Bullets.c
SRCS += Build/Tactical/Campaign.c
SRCS += Build/Tactical/Civ_Quotes.c
SRCS += Build/Tactical/Dialogue_Control.c
SRCS += Build/Tactical/DisplayCover.c
SRCS += Build/Tactical/Drugs_And_Alcohol.c
SRCS += Build/Tactical/End_Game.c
SRCS += Build/Tactical/Enemy_Soldier_Save.c
SRCS += Build/Tactical/FOV.c
SRCS += Build/Tactical/Faces.c
SRCS += Build/Tactical/Gap.c
SRCS += Build/Tactical/Handle_Doors.c
SRCS += Build/Tactical/Handle_Items.c
SRCS += Build/Tactical/Handle_UI.c
SRCS += Build/Tactical/Handle_UI_Plan.c
SRCS += Build/Tactical/Interface.c
SRCS += Build/Tactical/Interface_Control.c
SRCS += Build/Tactical/Interface_Cursors.c
SRCS += Build/Tactical/Interface_Dialogue.c
SRCS += Build/Tactical/Interface_Items.c
SRCS += Build/Tactical/Interface_Panels.c
SRCS += Build/Tactical/Interface_Utils.c
SRCS += Build/Tactical/Inventory_Choosing.c
SRCS += Build/Tactical/Items.c
SRCS += Build/Tactical/Keys.c
SRCS += Build/Tactical/LOS.c
SRCS += Build/Tactical/LoadSaveBullet.c
SRCS += Build/Tactical/LoadSaveMercProfile.c
SRCS += Build/Tactical/LoadSaveObjectType.c
SRCS += Build/Tactical/LoadSaveRottingCorpse.c
SRCS += Build/Tactical/LoadSaveSoldierCreate.c
SRCS += Build/Tactical/LoadSaveSoldierType.c
SRCS += Build/Tactical/LoadSaveVehicleType.c
SRCS += Build/Tactical/Map_Information.c
SRCS += Build/Tactical/Merc_Entering.c
SRCS += Build/Tactical/Merc_Hiring.c
SRCS += Build/Tactical/Militia_Control.c
SRCS += Build/Tactical/Morale.c
SRCS += Build/Tactical/OppList.c
SRCS += Build/Tactical/Overhead.c
SRCS += Build/Tactical/PathAI.c
SRCS += Build/Tactical/Points.c
SRCS += Build/Tactical/QArray.c
SRCS += Build/Tactical/Real_Time_Input.c
SRCS += Build/Tactical/Rotting_Corpses.c
SRCS += Build/Tactical/ShopKeeper_Interface.c
SRCS += Build/Tactical/SkillCheck.c
SRCS += Build/Tactical/Soldier_Add.c
SRCS += Build/Tactical/Soldier_Ani.c
SRCS += Build/Tactical/Soldier_Control.c
SRCS += Build/Tactical/Soldier_Create.c
SRCS += Build/Tactical/Soldier_Find.c
SRCS += Build/Tactical/Soldier_Init_List.c
SRCS += Build/Tactical/Soldier_Profile.c
SRCS += Build/Tactical/Soldier_Tile.c
SRCS += Build/Tactical/Spread_Burst.c
SRCS += Build/Tactical/Squads.c
SRCS += Build/Tactical/Strategic_Exit_GUI.c
SRCS += Build/Tactical/Structure_Wrap.c
SRCS += Build/Tactical/Tactical_Save.c
SRCS += Build/Tactical/Tactical_Turns.c
SRCS += Build/Tactical/TeamTurns.c
SRCS += Build/Tactical/Turn_Based_Input.c
SRCS += Build/Tactical/UI_Cursors.c
SRCS += Build/Tactical/Vehicles.c
SRCS += Build/Tactical/Weapons.c
SRCS += Build/Tactical/World_Items.c
SRCS += Build/TacticalAI/AIList.c
SRCS += Build/TacticalAI/AIMain.c
SRCS += Build/TacticalAI/AIUtils.c
SRCS += Build/TacticalAI/Attacks.c
SRCS += Build/TacticalAI/CreatureDecideAction.c
SRCS += Build/TacticalAI/DecideAction.c
SRCS += Build/TacticalAI/FindLocations.c
SRCS += Build/TacticalAI/Knowledge.c
SRCS += Build/TacticalAI/Medical.c
SRCS += Build/TacticalAI/Movement.c
SRCS += Build/TacticalAI/NPC.c
SRCS += Build/TacticalAI/PanicButtons.c
SRCS += Build/TacticalAI/Realtime.c
SRCS += Build/TileEngine/Ambient_Control.c
SRCS += Build/TileEngine/Buildings.c
SRCS += Build/TileEngine/Environment.c
SRCS += Build/TileEngine/Exit_Grids.c
SRCS += Build/TileEngine/Explosion_Control.c
SRCS += Build/TileEngine/Fog_Of_War.c
SRCS += Build/TileEngine/Interactive_Tiles.c
SRCS += Build/TileEngine/Isometric_Utils.c
SRCS += Build/TileEngine/LightEffects.c
SRCS += Build/TileEngine/Lighting.c
SRCS += Build/TileEngine/LoadSaveExplosionType.c
SRCS += Build/TileEngine/LoadSaveLightEffect.c
SRCS += Build/TileEngine/LoadSaveLightSprite.c
SRCS += Build/TileEngine/LoadSaveRealObject.c
SRCS += Build/TileEngine/LoadSaveSmokeEffect.c
SRCS += Build/TileEngine/Map_Edgepoints.c
SRCS += Build/TileEngine/Overhead_Map.c
SRCS += Build/TileEngine/Phys_Math.c
SRCS += Build/TileEngine/Physics.c
SRCS += Build/TileEngine/Pits.c
SRCS += Build/TileEngine/Radar_Screen.c
SRCS += Build/TileEngine/RenderWorld.c
SRCS += Build/TileEngine/Render_Dirty.c
SRCS += Build/TileEngine/Render_Fun.c
SRCS += Build/TileEngine/SaveLoadMap.c
SRCS += Build/TileEngine/Shade_Table_Util.c
SRCS += Build/TileEngine/Simple_Render_Utils.c
SRCS += Build/TileEngine/Smell.c
SRCS += Build/TileEngine/SmokeEffects.c
SRCS += Build/TileEngine/Structure.c
SRCS += Build/TileEngine/SysUtil.c
SRCS += Build/TileEngine/Tactical_Placement_GUI.c
SRCS += Build/TileEngine/TileDat.c
SRCS += Build/TileEngine/TileDef.c
SRCS += Build/TileEngine/Tile_Animation.c
SRCS += Build/TileEngine/Tile_Cache.c
SRCS += Build/TileEngine/Tile_Surface.c
SRCS += Build/TileEngine/WorldDat.c
SRCS += Build/TileEngine/WorldDef.c
SRCS += Build/TileEngine/WorldMan.c
SRCS += Build/Utils/Animated_ProgressBar.c
SRCS += Build/Utils/Cinematics.c
SRCS += Build/Utils/Cursors.c
SRCS += Build/Utils/Debug_Control.c
SRCS += Build/Utils/Encrypted_File.c
SRCS += Build/Utils/Event_Manager.c
SRCS += Build/Utils/Event_Pump.c
SRCS += Build/Utils/Font_Control.c
SRCS += Build/Utils/MapUtility.c
SRCS += Build/Utils/MercTextBox.c
SRCS += Build/Utils/Message.c
SRCS += Build/Utils/Multi_Language_Graphic_Utils.c
SRCS += Build/Utils/Music_Control.c
SRCS += Build/Utils/PopUpBox.c
SRCS += Build/Utils/Quantize.cc
SRCS += Build/Utils/Quantize_Wrap.cc
SRCS += Build/Utils/STIConvert.c
SRCS += Build/Utils/Slider.c
SRCS += Build/Utils/Sound_Control.c
SRCS += Build/Utils/Text.c
SRCS += Build/Utils/Text_Input.c
SRCS += Build/Utils/Text_Utils.c
SRCS += Build/Utils/Timer_Control.c
SRCS += Build/Utils/Utilities.c
SRCS += Build/Utils/WordWrap.c
SRCS += sgp/Button_Sound_Control.c
SRCS += sgp/Button_System.c
SRCS += sgp/Compression.c
SRCS += sgp/Config.c
SRCS += sgp/Container.c
SRCS += sgp/Cursor_Control.c
SRCS += sgp/Debug.c
SRCS += sgp/FileMan.c
SRCS += sgp/Font.c
SRCS += sgp/HImage.c
SRCS += sgp/ImpTGA.c
SRCS += sgp/Input.c
SRCS += sgp/LibraryDataBase.c
SRCS += sgp/Line.c
SRCS += sgp/MemMan.c
SRCS += sgp/MouseSystem.c
SRCS += sgp/PCX.c
SRCS += sgp/Random.c
SRCS += sgp/SGP.c
SRCS += sgp/SGPStrings.c
SRCS += sgp/STCI.c
SRCS += sgp/Shading.c
SRCS += sgp/Smack_Stub.c
SRCS += sgp/SoundMan.c
SRCS += sgp/Stubs.c
SRCS += sgp/VObject.c
SRCS += sgp/VObject_Blitters.c
SRCS += sgp/VSurface.c
SRCS += sgp/Video.c

LNGS :=
LNGS += Build/Utils/_DutchText.c
LNGS += Build/Utils/_EnglishText.c
LNGS += Build/Utils/_FrenchText.c
LNGS += Build/Utils/_GermanText.c
LNGS += Build/Utils/_ItalianText.c
LNGS += Build/Utils/_JA25EnglishText.c
LNGS += Build/Utils/_JA25GermanText.c
LNGS += Build/Utils/_JA25RussianText.c
LNGS += Build/Utils/_PolishText.c
LNGS += Build/Utils/_RussianText.c

SRCS += $(LNGS)

DEPS = $(filter %.d, $(SRCS:.c=.d) $(SRCS:.cc=.d))
OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cc=.o))

.SUFFIXES:
.SUFFIXES: .c .cc .d .o

Q ?= @

all: $(BINARY)

ifndef NO_DEPS
depend: $(DEPS)

ifeq ($(findstring $(MAKECMDGOALS), clean depend Data),)
-include $(DEPS)
endif
endif

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CCFLAGS) -c $< -o $@

.cc.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

.c.d:
	@echo '===> DEP $<'
	$(Q)$(CC) $(CCFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

.cc.d:
	@echo '===> DEP $<'
	$(Q)$(CXX) $(CXXFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(DEPS) $(OBJS) $(BINARY)

install: $(BINARY)
	@echo '===> INSTALL'
	$(Q)$(INSTALL) -d $(PREFIX)/bin $(MANPREFIX)/man/man6 $(PREFIX)/share/applications $(PREFIX)/share/pixmaps
	$(Q)$(INSTALL_PROGRAM) $(BINARY) $(PREFIX)/bin
	$(Q)$(INSTALL_MAN) ja2.6 $(MANPREFIX)/man/man6
	$(Q)$(INSTALL_DATA) ja2-stracciatella.desktop $(PREFIX)/share/applications
	$(Q)$(INSTALL_DATA) Build/Res/jagged3.ico $(PREFIX)/share/pixmaps/jagged2.ico


lowercase:
	$(Q)for i in \
		"$(SGPDATADIR)"/Data/*.[Ss][Ll][Ff] \
		"$(SGPDATADIR)"/Data/TILECACHE/*.[Jj][Ss][Dd] \
		"$(SGPDATADIR)"/Data/TILECACHE/*.[Ss][Tt][Ii]; \
	do \
		lower="`dirname "$$i"`/`basename "$$i" | LANG=C tr '[A-Z]' '[a-z]'`"; \
		[ "$$i" = "$$lower" ] || mv "$$i" "$$lower"; \
	done
