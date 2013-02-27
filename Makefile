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
ifndef CFLAGS_SDL
CFLAGS_SDL  := $(shell $(SDL_CONFIG) --cflags)
endif
ifndef LDFLAGS_SDL
LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)
endif

ifdef WITH_DEBUGINFO
CFLAGS += -g
endif

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
CFLAGS += -Wunused-label
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings

CFLAGS += -DJA2


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

ifdef _DEBUG
  CFLAGS += -D_DEBUG
  ifndef JA2TESTVERSION
    JA2TESTVERSION := yes
  endif
endif

ifdef JA2TESTVERSION
  CFLAGS += -DJA2TESTVERSION
  ifndef JA2BETAVERSION
	JA2BETAVERSION := yes
  endif
endif

ifdef JA2BETAVERSION
CFLAGS += -DJA2BETAVERSION -DSGP_DEBUG -DFORCE_ASSERTS_ON -DSGP_VIDEO_DEBUGGING
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
CCFLAGS += -Wmissing-prototypes

CXXFLAGS += $(CFLAGS)

LDFLAGS += $(LDFLAGS_SDL)
LDFLAGS += -lm

ifdef WITH_ZLIB
LDFLAGS += -lz
endif

SRCS :=
SRCS += Build/AniViewScreen.cc
SRCS += Build/Credits.cc

ifdef JA2EDITOR
SRCS += Build/Editor/Cursor_Modes.cc
SRCS += Build/Editor/EditScreen.cc
SRCS += Build/Editor/Edit_Sys.cc
SRCS += Build/Editor/EditorBuildings.cc
SRCS += Build/Editor/EditorItems.cc
SRCS += Build/Editor/EditorMapInfo.cc
SRCS += Build/Editor/EditorMercs.cc
SRCS += Build/Editor/EditorTerrain.cc
SRCS += Build/Editor/Editor_Callbacks.cc
SRCS += Build/Editor/Editor_Modes.cc
SRCS += Build/Editor/Editor_Taskbar_Creation.cc
SRCS += Build/Editor/Editor_Taskbar_Utils.cc
SRCS += Build/Editor/Editor_Undo.cc
SRCS += Build/Editor/Item_Statistics.cc
SRCS += Build/Editor/LoadScreen.cc
SRCS += Build/Editor/MessageBox.cc
SRCS += Build/Editor/NewSmooth.cc
SRCS += Build/Editor/PopupMenu.cc
SRCS += Build/Editor/Road_Smoothing.cc
SRCS += Build/Editor/Sector_Summary.cc
SRCS += Build/Editor/SelectWin.cc
SRCS += Build/Editor/SmartMethod.cc
SRCS += Build/Editor/Smooth.cc
SRCS += Build/Editor/Smoothing_Utils.cc
endif

SRCS += Build/Fade_Screen.cc
SRCS += Build/GameInitOptionsScreen.cc
SRCS += Build/GameLoop.cc
SRCS += Build/GameResources.cc
SRCS += Build/GameScreen.cc
SRCS += Build/GameSettings.cc
SRCS += Build/GameVersion.cc
SRCS += Build/HelpScreen.cc
SRCS += Build/Init.cc
SRCS += Build/Intro.cc
SRCS += Build/JA2_Splash.cc
SRCS += Build/JAScreens.cc
SRCS += Build/Laptop/AIM.cc
SRCS += Build/Laptop/AIMArchives.cc
SRCS += Build/Laptop/AIMFacialIndex.cc
SRCS += Build/Laptop/AIMHistory.cc
SRCS += Build/Laptop/AIMLinks.cc
SRCS += Build/Laptop/AIMMembers.cc
SRCS += Build/Laptop/AIMPolicies.cc
SRCS += Build/Laptop/AIMSort.cc
SRCS += Build/Laptop/BobbyR.cc
SRCS += Build/Laptop/BobbyRAmmo.cc
SRCS += Build/Laptop/BobbyRArmour.cc
SRCS += Build/Laptop/BobbyRGuns.cc
SRCS += Build/Laptop/BobbyRMailOrder.cc
SRCS += Build/Laptop/BobbyRMisc.cc
SRCS += Build/Laptop/BobbyRShipments.cc
SRCS += Build/Laptop/BobbyRUsed.cc
SRCS += Build/Laptop/BrokenLink.cc
SRCS += Build/Laptop/CharProfile.cc
SRCS += Build/Laptop/EMail.cc
SRCS += Build/Laptop/Files.cc
SRCS += Build/Laptop/Finances.cc
SRCS += Build/Laptop/Florist.cc
SRCS += Build/Laptop/Florist_Cards.cc
SRCS += Build/Laptop/Florist_Gallery.cc
SRCS += Build/Laptop/Florist_Order_Form.cc
SRCS += Build/Laptop/Funeral.cc
SRCS += Build/Laptop/History.cc
SRCS += Build/Laptop/IMPVideoObjects.cc
SRCS += Build/Laptop/IMP_AboutUs.cc
SRCS += Build/Laptop/IMP_Attribute_Entrance.cc
SRCS += Build/Laptop/IMP_Attribute_Finish.cc
SRCS += Build/Laptop/IMP_Attribute_Selection.cc
SRCS += Build/Laptop/IMP_Begin_Screen.cc
SRCS += Build/Laptop/IMP_Compile_Character.cc
SRCS += Build/Laptop/IMP_Confirm.cc
SRCS += Build/Laptop/IMP_Finish.cc
SRCS += Build/Laptop/IMP_HomePage.cc
SRCS += Build/Laptop/IMP_MainPage.cc
SRCS += Build/Laptop/IMP_Personality_Entrance.cc
SRCS += Build/Laptop/IMP_Personality_Finish.cc
SRCS += Build/Laptop/IMP_Personality_Quiz.cc
SRCS += Build/Laptop/IMP_Portraits.cc
SRCS += Build/Laptop/IMP_Text_System.cc
SRCS += Build/Laptop/IMP_Voices.cc
SRCS += Build/Laptop/Insurance.cc
SRCS += Build/Laptop/Insurance_Comments.cc
SRCS += Build/Laptop/Insurance_Contract.cc
SRCS += Build/Laptop/Insurance_Info.cc
SRCS += Build/Laptop/Laptop.cc
SRCS += Build/Laptop/Mercs.cc
SRCS += Build/Laptop/Mercs_Account.cc
SRCS += Build/Laptop/Mercs_Files.cc
SRCS += Build/Laptop/Mercs_No_Account.cc
SRCS += Build/Laptop/Personnel.cc
SRCS += Build/Laptop/Store_Inventory.cc
SRCS += Build/LoadSaveEMail.cc
SRCS += Build/LoadSaveTacticalStatusType.cc
SRCS += Build/Loading_Screen.cc
SRCS += Build/MainMenuScreen.cc
SRCS += Build/MercPortrait.cc
SRCS += Build/MessageBoxScreen.cc
SRCS += Build/Options_Screen.cc
SRCS += Build/SaveLoadGame.cc
SRCS += Build/SaveLoadScreen.cc
SRCS += Build/Screens.cc
SRCS += Build/Strategic/AI_Viewer.cc
SRCS += Build/Strategic/Assignments.cc
SRCS += Build/Strategic/Auto_Resolve.cc
SRCS += Build/Strategic/Campaign_Init.cc
SRCS += Build/Strategic/Creature_Spreading.cc
SRCS += Build/Strategic/Game_Clock.cc
SRCS += Build/Strategic/Game_Event_Hook.cc
SRCS += Build/Strategic/Game_Events.cc
SRCS += Build/Strategic/Game_Init.cc
SRCS += Build/Strategic/Hourly_Update.cc
SRCS += Build/Strategic/LoadSaveSectorInfo.cc
SRCS += Build/Strategic/LoadSaveStrategicMapElement.cc
SRCS += Build/Strategic/LoadSaveUndergroundSectorInfo.cc
SRCS += Build/Strategic/MapScreen.cc
SRCS += Build/Strategic/Map_Screen_Helicopter.cc
SRCS += Build/Strategic/Map_Screen_Interface.cc
SRCS += Build/Strategic/Map_Screen_Interface_Border.cc
SRCS += Build/Strategic/Map_Screen_Interface_Bottom.cc
SRCS += Build/Strategic/Map_Screen_Interface_Map.cc
SRCS += Build/Strategic/Map_Screen_Interface_Map_Inventory.cc
SRCS += Build/Strategic/Map_Screen_Interface_TownMine_Info.cc
SRCS += Build/Strategic/Meanwhile.cc
SRCS += Build/Strategic/Merc_Contract.cc
SRCS += Build/Strategic/Player_Command.cc
SRCS += Build/Strategic/PreBattle_Interface.cc
SRCS += Build/Strategic/Queen_Command.cc
SRCS += Build/Strategic/QuestText.cc
SRCS += Build/Strategic/Quest_Debug_System.cc
SRCS += Build/Strategic/Quests.cc
SRCS += Build/Strategic/Scheduling.cc
SRCS += Build/Strategic/Strategic.cc
SRCS += Build/Strategic/StrategicMap.cc
SRCS += Build/Strategic/Strategic_AI.cc
SRCS += Build/Strategic/Strategic_Event_Handler.cc
SRCS += Build/Strategic/Strategic_Merc_Handler.cc
SRCS += Build/Strategic/Strategic_Mines.cc
SRCS += Build/Strategic/Strategic_Movement.cc
SRCS += Build/Strategic/Strategic_Movement_Costs.cc
SRCS += Build/Strategic/Strategic_Pathing.cc
SRCS += Build/Strategic/Strategic_Status.cc
SRCS += Build/Strategic/Strategic_Town_Loyalty.cc
SRCS += Build/Strategic/Strategic_Turns.cc
SRCS += Build/Strategic/Town_Militia.cc
SRCS += Build/Sys_Globals.cc
SRCS += Build/Tactical/Air_Raid.cc
SRCS += Build/Tactical/Animation_Cache.cc
SRCS += Build/Tactical/Animation_Control.cc
SRCS += Build/Tactical/Animation_Data.cc
SRCS += Build/Tactical/ArmsDealerInvInit.cc
SRCS += Build/Tactical/Arms_Dealer_Init.cc
SRCS += Build/Tactical/Auto_Bandage.cc
SRCS += Build/Tactical/Boxing.cc
SRCS += Build/Tactical/Bullets.cc
SRCS += Build/Tactical/Campaign.cc
SRCS += Build/Tactical/Civ_Quotes.cc
SRCS += Build/Tactical/Dialogue_Control.cc
SRCS += Build/Tactical/DisplayCover.cc
SRCS += Build/Tactical/Drugs_And_Alcohol.cc
SRCS += Build/Tactical/End_Game.cc
SRCS += Build/Tactical/Enemy_Soldier_Save.cc
SRCS += Build/Tactical/FOV.cc
SRCS += Build/Tactical/Faces.cc
SRCS += Build/Tactical/Gap.cc
SRCS += Build/Tactical/Handle_Doors.cc
SRCS += Build/Tactical/Handle_Items.cc
SRCS += Build/Tactical/Handle_UI.cc
SRCS += Build/Tactical/Interface.cc
SRCS += Build/Tactical/Interface_Control.cc
SRCS += Build/Tactical/Interface_Cursors.cc
SRCS += Build/Tactical/Interface_Dialogue.cc
SRCS += Build/Tactical/Interface_Items.cc
SRCS += Build/Tactical/Interface_Panels.cc
SRCS += Build/Tactical/Interface_Utils.cc
SRCS += Build/Tactical/Inventory_Choosing.cc
SRCS += Build/Tactical/Items.cc
SRCS += Build/Tactical/Keys.cc
SRCS += Build/Tactical/LOS.cc
SRCS += Build/Tactical/LoadSaveBasicSoldierCreateStruct.cc
SRCS += Build/Tactical/LoadSaveBullet.cc
SRCS += Build/Tactical/LoadSaveMercProfile.cc
SRCS += Build/Tactical/LoadSaveObjectType.cc
SRCS += Build/Tactical/LoadSaveRottingCorpse.cc
SRCS += Build/Tactical/LoadSaveSoldierCreate.cc
SRCS += Build/Tactical/LoadSaveSoldierType.cc
SRCS += Build/Tactical/LoadSaveVehicleType.cc
SRCS += Build/Tactical/Map_Information.cc
SRCS += Build/Tactical/Merc_Entering.cc
SRCS += Build/Tactical/Merc_Hiring.cc
SRCS += Build/Tactical/Militia_Control.cc
SRCS += Build/Tactical/Morale.cc
SRCS += Build/Tactical/OppList.cc
SRCS += Build/Tactical/Overhead.cc
SRCS += Build/Tactical/PathAI.cc
SRCS += Build/Tactical/Points.cc
SRCS += Build/Tactical/QArray.cc
SRCS += Build/Tactical/Real_Time_Input.cc
SRCS += Build/Tactical/Rotting_Corpses.cc
SRCS += Build/Tactical/ShopKeeper_Interface.cc
SRCS += Build/Tactical/SkillCheck.cc
SRCS += Build/Tactical/Soldier_Add.cc
SRCS += Build/Tactical/Soldier_Ani.cc
SRCS += Build/Tactical/Soldier_Control.cc
SRCS += Build/Tactical/Soldier_Create.cc
SRCS += Build/Tactical/Soldier_Find.cc
SRCS += Build/Tactical/Soldier_Init_List.cc
SRCS += Build/Tactical/Soldier_Profile.cc
SRCS += Build/Tactical/Soldier_Tile.cc
SRCS += Build/Tactical/Spread_Burst.cc
SRCS += Build/Tactical/Squads.cc
SRCS += Build/Tactical/Strategic_Exit_GUI.cc
SRCS += Build/Tactical/Structure_Wrap.cc
SRCS += Build/Tactical/Tactical_Save.cc
SRCS += Build/Tactical/Tactical_Turns.cc
SRCS += Build/Tactical/TeamTurns.cc
SRCS += Build/Tactical/Turn_Based_Input.cc
SRCS += Build/Tactical/UI_Cursors.cc
SRCS += Build/Tactical/Vehicles.cc
SRCS += Build/Tactical/Weapons.cc
SRCS += Build/Tactical/World_Items.cc
SRCS += Build/TacticalAI/AIList.cc
SRCS += Build/TacticalAI/AIMain.cc
SRCS += Build/TacticalAI/AIUtils.cc
SRCS += Build/TacticalAI/Attacks.cc
SRCS += Build/TacticalAI/CreatureDecideAction.cc
SRCS += Build/TacticalAI/DecideAction.cc
SRCS += Build/TacticalAI/FindLocations.cc
SRCS += Build/TacticalAI/Knowledge.cc
SRCS += Build/TacticalAI/Medical.cc
SRCS += Build/TacticalAI/Movement.cc
SRCS += Build/TacticalAI/NPC.cc
SRCS += Build/TacticalAI/PanicButtons.cc
SRCS += Build/TacticalAI/Realtime.cc
SRCS += Build/TileEngine/Ambient_Control.cc
SRCS += Build/TileEngine/Buildings.cc
SRCS += Build/TileEngine/Environment.cc
SRCS += Build/TileEngine/Exit_Grids.cc
SRCS += Build/TileEngine/Explosion_Control.cc
SRCS += Build/TileEngine/Fog_Of_War.cc
SRCS += Build/TileEngine/Interactive_Tiles.cc
SRCS += Build/TileEngine/Isometric_Utils.cc
SRCS += Build/TileEngine/LightEffects.cc
SRCS += Build/TileEngine/Lighting.cc
SRCS += Build/TileEngine/LoadSaveExplosionType.cc
SRCS += Build/TileEngine/LoadSaveLightEffect.cc
SRCS += Build/TileEngine/LoadSaveLightSprite.cc
SRCS += Build/TileEngine/LoadSaveRealObject.cc
SRCS += Build/TileEngine/LoadSaveSmokeEffect.cc
SRCS += Build/TileEngine/Map_Edgepoints.cc
SRCS += Build/TileEngine/Overhead_Map.cc
SRCS += Build/TileEngine/Phys_Math.cc
SRCS += Build/TileEngine/Physics.cc
SRCS += Build/TileEngine/Pits.cc
SRCS += Build/TileEngine/Radar_Screen.cc
SRCS += Build/TileEngine/RenderWorld.cc
SRCS += Build/TileEngine/Render_Dirty.cc
SRCS += Build/TileEngine/Render_Fun.cc
SRCS += Build/TileEngine/SaveLoadMap.cc
SRCS += Build/TileEngine/Simple_Render_Utils.cc
SRCS += Build/TileEngine/Smell.cc
SRCS += Build/TileEngine/SmokeEffects.cc
SRCS += Build/TileEngine/Structure.cc
SRCS += Build/TileEngine/SysUtil.cc
SRCS += Build/TileEngine/Tactical_Placement_GUI.cc
SRCS += Build/TileEngine/TileDat.cc
SRCS += Build/TileEngine/TileDef.cc
SRCS += Build/TileEngine/Tile_Animation.cc
SRCS += Build/TileEngine/Tile_Cache.cc
SRCS += Build/TileEngine/Tile_Surface.cc
SRCS += Build/TileEngine/WorldDat.cc
SRCS += Build/TileEngine/WorldDef.cc
SRCS += Build/TileEngine/WorldMan.cc
SRCS += Build/Utils/Animated_ProgressBar.cc
SRCS += Build/Utils/Cinematics.cc
SRCS += Build/Utils/Cursors.cc
SRCS += Build/Utils/Debug_Control.cc
SRCS += Build/Utils/Encrypted_File.cc
SRCS += Build/Utils/Event_Manager.cc
SRCS += Build/Utils/Event_Pump.cc
SRCS += Build/Utils/Font_Control.cc
SRCS += Build/Utils/MapUtility.cc
SRCS += Build/Utils/MercTextBox.cc
SRCS += Build/Utils/Message.cc
SRCS += Build/Utils/Multi_Language_Graphic_Utils.cc
SRCS += Build/Utils/Music_Control.cc
SRCS += Build/Utils/PopUpBox.cc
SRCS += Build/Utils/Quantize.cc
SRCS += Build/Utils/STIConvert.cc
SRCS += Build/Utils/Slider.cc
SRCS += Build/Utils/Sound_Control.cc
SRCS += Build/Utils/Text.cc
SRCS += Build/Utils/Text_Input.cc
SRCS += Build/Utils/Text_Utils.cc
SRCS += Build/Utils/Timer_Control.cc
SRCS += Build/Utils/Utilities.cc
SRCS += Build/Utils/WordWrap.cc
SRCS += sgp/Button_Sound_Control.cc
SRCS += sgp/Button_System.cc
SRCS += sgp/Config.cc
SRCS += sgp/Container.cc
SRCS += sgp/Cursor_Control.cc
SRCS += sgp/Debug.cc
SRCS += sgp/FileMan.cc
SRCS += sgp/Font.cc
SRCS += sgp/HImage.cc
SRCS += sgp/ImpTGA.cc
SRCS += sgp/Input.cc
SRCS += sgp/LibraryDataBase.cc
SRCS += sgp/Line.cc
SRCS += sgp/MemMan.cc
SRCS += sgp/MouseSystem.cc
SRCS += sgp/PCX.cc
SRCS += sgp/Random.cc
SRCS += sgp/SGP.cc
SRCS += sgp/SGPStrings.cc
SRCS += sgp/STCI.cc
SRCS += sgp/Shading.cc
SRCS += sgp/Smack_Stub.cc
SRCS += sgp/SoundMan.cc
SRCS += sgp/TranslationTable.c
SRCS += sgp/VObject.cc
SRCS += sgp/VObject_Blitters.cc
SRCS += sgp/VSurface.cc
SRCS += sgp/Video.cc

LNGS :=
LNGS += Build/Utils/_DutchText.cc
LNGS += Build/Utils/_EnglishText.cc
LNGS += Build/Utils/_FrenchText.cc
LNGS += Build/Utils/_GermanText.cc
LNGS += Build/Utils/_ItalianText.cc
LNGS += Build/Utils/_JA25EnglishText.cc
LNGS += Build/Utils/_JA25GermanText.cc
LNGS += Build/Utils/_JA25RussianText.cc
LNGS += Build/Utils/_PolishText.cc
LNGS += Build/Utils/_RussianText.cc

SRCS += $(LNGS)

OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cc=.o))
DEPS = $(OBJS:.o=.d)

.SUFFIXES:
.SUFFIXES: .c .cc .d .o

Q ?= @

all: $(BINARY)

-include $(DEPS)

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CCFLAGS) -c -MMD -o $@ $<

.cc.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

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

deinstall:
	@echo '===> DEINSTALL'
	$(Q)rm $(PREFIX)/bin/$(BINARY)
	$(Q)rm $(MANPREFIX)/man/man6/ja2.6
	$(Q)rm $(PREFIX)/share/applications/ja2-stracciatella.desktop
	$(Q)rm $(PREFIX)/share/pixmaps/jagged2.ico


lowercase:
	$(Q)for i in \
		"$(SGPDATADIR)"/Data/*.[Ss][Ll][Ff] \
		"$(SGPDATADIR)"/Data/TILECACHE/*.[Jj][Ss][Dd] \
		"$(SGPDATADIR)"/Data/TILECACHE/*.[Ss][Tt][Ii]; \
	do \
		lower="`dirname "$$i"`/`basename "$$i" | LANG=C tr '[A-Z]' '[a-z]'`"; \
		[ "$$i" = "$$lower" ] || mv "$$i" "$$lower"; \
	done

rebuild-tags:
	-rm TAGS
	find . -type f \( -name "*.c" -o -iname "*.cc" -o -name "*.h" \) | xargs etags --append


# How to
# ========================================
#
# Debug a segfault
# ----------------
#
#  make clean
#  make all WITH_DEBUGINFO=1
#
#  gdb ./ja2
#
#  (gdb) run
#  (gdb) backtrace
#
#
