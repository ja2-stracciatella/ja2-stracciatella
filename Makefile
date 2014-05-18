#
# See COMPILATION.txt for instructions.
# Please update COMPILATION.txt if necessary after changing this file.
#

-include Makefile.config
# Makefile.config can be empty.  In this case application is build
# to be executed from the current directory.
#
# Or Makefile.config may contain the following values:
#   BINARY_DIR
#   MANPAGE_DIR
#   FULL_PATH_EXTRA_DATA_DIR
#   INSTALLABLE                := "yes"
# In this case application can be installed.


# By default build the project with unit tests.
# If you want to build without them, use make WITH_UNITTESTS=0
WITH_UNITTESTS ?= 1

WITH_LPTHREAD ?= 1

BINARY    ?= ja2

VERSION := 0.14.xx
GAME_VERSION := v$(VERSION)
CFLAGS += -DGAME_VERSION=\"$(GAME_VERSION)\"


############################################################
# SDL Library settings.
# Project can be built with local SDL library (from _build/lib-SDL-devel-*)
# or system installed SDL library.
############################################################


ifdef LOCAL_SDL_LIB
CFLAGS_SDL= -I./$(LOCAL_SDL_LIB)/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
LDFLAGS_SDL=-L./$(LOCAL_SDL_LIB)/lib -lmingw32 -lSDLmain -lSDL -mwindows
endif

ifndef LOCAL_SDL_LIB

# using system SDL library

SDL_CONFIG  ?= sdl-config
ifndef CFLAGS_SDL
CFLAGS_SDL  := $(shell $(SDL_CONFIG) --cflags)
endif
ifndef LDFLAGS_SDL
LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)
endif

endif

CFLAGS += $(CFLAGS_SDL)
LDFLAGS += $(LDFLAGS_SDL)

############################################################
# MinGW settings for building on Windows and for
# cross-building on Linux
############################################################

ifdef USE_MINGW

ifndef MINGW_PREFIX
$(error MINGW_PREFIX is not specified.  Examples: MINGW_PREFIX=i586-mingw32msvc (on Linux), MINGW_PREFIX=/cygdrive/c/MinGW/bin/mingw32 (on Windows))
endif

CC=$(MINGW_PREFIX)-gcc
CXX=$(MINGW_PREFIX)-g++
CPP=$(MINGW_PREFIX)-cpp
RANLIB=$(MINGW_PREFIX)-ranlib

CFLAGS += -mwindows -mconsole

endif

############################################################
#
############################################################

ifdef WITH_DEBUGINFO
CFLAGS += -g
endif

CFLAGS += -I .
CFLAGS += -I Build
CFLAGS += -I Build/Editor
CFLAGS += -I Build/Laptop
CFLAGS += -I Build/Res
CFLAGS += -I Build/Strategic
CFLAGS += -I Build/Tactical
CFLAGS += -I Build/TacticalAI
CFLAGS += -I Build/TileEngine
CFLAGS += -I Build/Utils
CFLAGS += -I sgp
CFLAGS += -I src
CFLAGS += -I _build/lib-MicroIni/include
CFLAGS += -I _build/lib-boost
CFLAGS += -I _build/lib-rapidjson
CFLAGS += -I _build/lib-slog
CFLAGS += -I _build/lib-smacker/libsmacker
CFLAGS += -I _build/lib-utf8cpp/source

#CFLAGS += -Wall
#CFLAGS += -W
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wunused-label
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings

CFLAGS += -DJA2
CFLAGS += -DMICROINI_STATIC


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

CCFLAGS += $(CFLAGS)
CCFLAGS += -std=gnu99
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wmissing-prototypes

CXXFLAGS += $(CFLAGS)

LDFLAGS += -lm

ifeq "$(WITH_LPTHREAD)" "1"
LDFLAGS += -lpthread
endif

ifdef WITH_ZLIB
LDFLAGS += -lz
endif

SRCS :=
SRCS += Build/AniViewScreen.cc
SRCS += Build/Credits.cc
SRCS += Build/UILayout.cc

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

SRCS += Build/Cheats.cc
SRCS += Build/Fade_Screen.cc
SRCS += Build/GameInitOptionsScreen.cc
SRCS += Build/GameLoop.cc
SRCS += Build/GameRes.cc
SRCS += Build/GameState.cc
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
SRCS += Build/Utils/Event_Manager.cc
SRCS += Build/Utils/Event_Pump.cc
SRCS += Build/Utils/Font_Control.cc
SRCS += Build/Utils/MapUtility.cc
SRCS += Build/Utils/MercTextBox.cc
SRCS += Build/Utils/Message.cc
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
SRCS += sgp/Container.cc
SRCS += sgp/Cursor_Control.cc
SRCS += sgp/Debug.cc
SRCS += sgp/EncodingCorrectors.cc
SRCS += sgp/FileMan.cc
SRCS += sgp/Font.cc
SRCS += sgp/HImage.cc
SRCS += sgp/ImpTGA.cc
SRCS += sgp/Input.cc
SRCS += sgp/LibraryDataBase.cc
SRCS += sgp/Line.cc
SRCS += sgp/LoadSaveData.cc
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
SRCS += sgp/StrUtils.cc
SRCS += sgp/TranslationTable.cc
SRCS += sgp/UTF8String.cc
SRCS += sgp/VObject.cc
SRCS += sgp/VObject_Blitters.cc
SRCS += sgp/VSurface.cc
SRCS += sgp/Video.cc

SRCS += src/AmmoTypeModel.cc
SRCS += src/CalibreModel.cc
SRCS += src/DealerInventory.cc
SRCS += src/DefaultContentManager.cc
SRCS += src/ItemModel.cc
SRCS += src/JsonUtility.cc
SRCS += src/MagazineModel.cc
SRCS += src/MercProfile.cc
SRCS += src/ModPackContentManager.cc
SRCS += src/Soldier.cc
SRCS += src/WeaponModels.cc
SRCS += src/content/ContentMercs.cc
SRCS += src/content/Dialogs.cc
SRCS += src/content/npcs.cc
SRCS += src/internals/enums.cc
SRCS += src/policy/DefaultGamePolicy.cc
SRCS += src/policy/DefaultIMPPolicy.cc

SRCS += _build/lib-MicroIni/src/MicroIni/File.cpp
SRCS += _build/lib-MicroIni/src/MicroIni/Line.cpp
SRCS += _build/lib-MicroIni/src/MicroIni/Section.cpp
SRCS += _build/lib-MicroIni/src/MicroIni/Value.cpp

SRCS += _build/lib-boost/libs/system/src/error_code.cpp
SRCS += _build/lib-boost/libs/filesystem/src/codecvt_error_category.cpp
SRCS += _build/lib-boost/libs/filesystem/src/operations.cpp
SRCS += _build/lib-boost/libs/filesystem/src/path.cpp
SRCS += _build/lib-boost/libs/filesystem/src/path_traits.cpp
SRCS += _build/lib-boost/libs/filesystem/src/portability.cpp
SRCS += _build/lib-boost/libs/filesystem/src/unique_path.cpp
SRCS += _build/lib-boost/libs/filesystem/src/utf8_codecvt_facet.cpp
SRCS += _build/lib-boost/libs/filesystem/src/windows_file_codecvt.cpp

SRCS += _build/lib-slog/slog/slog.c

SRCS += _build/lib-smacker/libsmacker/smacker.c
SRCS += _build/lib-smacker/libsmacker/smk_hufftree.c
SRCS += _build/lib-smacker/libsmacker/smk_bitstream.c

LNGS :=
LNGS += Build/Utils/_DutchText.cc
LNGS += Build/Utils/_EnglishText.cc
LNGS += Build/Utils/_FrenchText.cc
LNGS += Build/Utils/_GermanText.cc
LNGS += Build/Utils/_ItalianText.cc
LNGS += Build/Utils/_PolishText.cc
LNGS += Build/Utils/_RussianText.cc

SRCS += $(LNGS)

ifeq "$(WITH_UNITTESTS)" "1"
CFLAGS += -D WITH_UNITTESTS
CFLAGS += -I _build/lib-gtest/include
CFLAGS += -I _build/lib-gtest
SRCS += _build/lib-gtest/src/gtest.cc
SRCS += _build/lib-gtest/src/gtest-death-test.cc
SRCS += _build/lib-gtest/src/gtest-filepath.cc
SRCS += _build/lib-gtest/src/gtest-port.cc
SRCS += _build/lib-gtest/src/gtest-printers.cc
SRCS += _build/lib-gtest/src/gtest-test-part.cc
SRCS += _build/lib-gtest/src/gtest-typed-test.cc
SRCS += Build/SaveLoadGame_unittest.cc
SRCS += Build/Tactical/LoadSaveMercProfile_unittest.cc
SRCS += Build/VanillaDataStructures_unittest.cc
SRCS += sgp/FileMan_unittest.cc
SRCS += sgp/LoadSaveData_unittest.cc
SRCS += sgp/UTF8String_unittest.cc
SRCS += sgp/wchar_unittest.cc
SRCS += src/DefaultContentManagerUT.cc
SRCS += src/DefaultContentManager_unittests.cc
SRCS += src/JsonUtility_unittests.cc
SRCS += src/VanillaWeapons_unittests.cc
SRCS += src/TestUtils.cc
endif

OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cc=.o) $(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .d .o

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

.cpp.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(DEPS) $(OBJS) $(BINARY)

install: $(BINARY)
	test -z "$(INSTALLABLE)" || install -d $(BINARY_DIR)
	test -z "$(INSTALLABLE)" || install -d $(MANPAGE_DIR)
	test -z "$(INSTALLABLE)" || install -d $(FULL_PATH_EXTRA_DATA_DIR)
	test -z "$(INSTALLABLE)" || install -m 555 $(BINARY) $(BINARY_DIR)
	test -z "$(INSTALLABLE)" || cp -r externalized $(FULL_PATH_EXTRA_DATA_DIR)
	test -z "$(INSTALLABLE)" || cp -r mods         $(FULL_PATH_EXTRA_DATA_DIR)
	test -z "$(INSTALLABLE)" || cp -r _unittests   $(FULL_PATH_EXTRA_DATA_DIR)
	test -z "$(INSTALLABLE)" || install -m 444 ja2_manpage $(MANPAGE_DIR)/ja2.6

	@test -n "$(INSTALLABLE)" || echo "------------------------------------------------------------------------------"
	@test -n "$(INSTALLABLE)" || echo "This build doesn't support installation."
	@test -n "$(INSTALLABLE)" || echo "You can run the game right from this directory, like this: ./$(BINARY)"
	@test -n "$(INSTALLABLE)" || echo "If you want a local installation, do:"
	@test -n "$(INSTALLABLE)" || echo "  ./configure"
	@test -n "$(INSTALLABLE)" || echo "  make"
	@test -n "$(INSTALLABLE)" || echo "  sudo make install"
	@test -n "$(INSTALLABLE)" || echo "------------------------------------------------------------------------------"

deinstall:
	test -z "$(INSTALLABLE)" || rm $(MANPAGE_DIR)/ja2.6
	test -z "$(INSTALLABLE)" || rm $(BINARY_DIR)/$(BINARY)
	test -z "$(INSTALLABLE)" || test -d $(FULL_PATH_EXTRA_DATA_DIR)
	test -z "$(INSTALLABLE)" || rm -rf $(FULL_PATH_EXTRA_DATA_DIR)/externalized
	test -z "$(INSTALLABLE)" || rm -rf $(FULL_PATH_EXTRA_DATA_DIR)/mods
	test -z "$(INSTALLABLE)" || rm -rf $(FULL_PATH_EXTRA_DATA_DIR)/_unittests
	test -z "$(INSTALLABLE)" || rmdir $(FULL_PATH_EXTRA_DATA_DIR)

	@test -n "$(INSTALLABLE)" || echo "------------------------------------------------------------------------------"
	@test -n "$(INSTALLABLE)" || echo "This build doesn't support deinstallation"
	@test -n "$(INSTALLABLE)" || echo "------------------------------------------------------------------------------"

rebuild-tags:
	-rm TAGS
	find . -type f \( -name "*.c" -o -iname "*.cc" -o -name "*.h" \) | xargs etags --append

rebuild-tags-win:
	-rm TAGS
	find . -type f \( -name "*.c" -o -iname "*.cc" -o -name "*.h" \) | xargs /cygdrive/c/Programs/emacs-23.3/bin/etags.exe --append

fix-permissions:
	chmod +x _build/solution-vs10e/Debug/exe/SDL.dll
	chmod +x _build/solution-vs10e/Release/exe/SDL.dll

WIN_RELEASE_BASE_DIR := "release-win-mingw-cross"
ifdef BETA
WIN_RELEASE_NAME := "ja2-$(GAME_VERSION)-win-beta"
else
WIN_RELEASE_NAME := "ja2-$(GAME_VERSION)-win"
endif
WIN_RELEASE := $(WIN_RELEASE_BASE_DIR)/$(WIN_RELEASE_NAME)
WIN_RELEASE_ZIP := $(WIN_RELEASE_BASE_DIR)/$(WIN_RELEASE_NAME).zip

MAC_RELEASE_BASE_DIR := "release-mac"
MAC_RELEASE_NAME := "ja2-$(GAME_VERSION)-macos"
MAC_RELEASE := $(MAC_RELEASE_BASE_DIR)/$(MAC_RELEASE_NAME)
MAC_RELEASE_ZIP := $(MAC_RELEASE_BASE_DIR)/$(MAC_RELEASE_NAME).zip

SRC_RELEASE_BASE_DIR := "release-src"

build-beta-win-release-on-linux:
	make BETA=1 build-win-release-on-linux

# sudo apt-get install gcc-mingw-w64 g++-mingw-w64
build-win-release-on-linux:
	-rm -rf $(WIN_RELEASE) $(WIN_RELEASE_ZIP)
	mkdir -p $(WIN_RELEASE)
	make USE_MINGW=1 MINGW_PREFIX=i686-w64-mingw32 LOCAL_SDL_LIB=_build/lib-SDL-devel-1.2.15-mingw32 WITH_LPTHREAD=0
	mv ./ja2 $(WIN_RELEASE)/ja2.exe
	cp _build/lib-SDL-devel-1.2.15-mingw32/bin/SDL.dll $(WIN_RELEASE)
	cp _build/distr-files-win/*.bat $(WIN_RELEASE)
	cp _build/distr-files-win/*.txt $(WIN_RELEASE)
	cp _build/distr-files-win-mingw/*.dll $(WIN_RELEASE)
	cp -r _unittests $(WIN_RELEASE)
	cp -r externalized $(WIN_RELEASE)
	cp Changelog $(WIN_RELEASE)/Changelog.txt
	cp changes.md $(WIN_RELEASE)/changes.md
	cd $(WIN_RELEASE_BASE_DIR) && zip -r $(WIN_RELEASE_NAME).zip $(WIN_RELEASE_NAME)

# Building i386 release on MAC
#
# Using precompiled static SDL library since it is quite difficult for
# end users to setup the library.  LGPL allows static linking if we
# provide source codes for our application.
MACOS_SDL_STATIC=./_build/lib-SDL-devel-1.2.15-macos-i386
MACOS_STATIC_CFLAGS_SDL=-arch i386 -I$(MACOS_SDL_STATIC)/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE
MACOS_STATIC_LDFLAGS_SDL=$(MACOS_SDL_STATIC)/lib/libSDLmain.a $(MACOS_SDL_STATIC)/lib/libSDL.a  -Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit
build-release-on-mac:
	-rm -rf $(MAC_RELEASE) $(MAC_RELEASE_ZIP)
	mkdir -p $(MAC_RELEASE)
	make "CFLAGS_SDL=$(MACOS_STATIC_CFLAGS_SDL)" "LDFLAGS_SDL=$(MACOS_STATIC_LDFLAGS_SDL)"
	mv ./ja2 $(MAC_RELEASE)/ja2
	cp _build/distr-files-mac/*.command $(MAC_RELEASE)
	cp _build/distr-files-mac/*.txt $(MAC_RELEASE)
	cp -r _unittests $(MAC_RELEASE)
	cp -r externalized $(MAC_RELEASE)
	cp Changelog $(MAC_RELEASE)/Changelog.txt
	cp changes.md $(MAC_RELEASE)/changes.md
	cd $(MAC_RELEASE_BASE_DIR) && zip -r $(MAC_RELEASE_NAME).zip $(MAC_RELEASE_NAME)

build-on-mac:
	make "CFLAGS_SDL=$(MACOS_STATIC_CFLAGS_SDL)" "LDFLAGS_SDL=$(MACOS_STATIC_LDFLAGS_SDL)"

build-on-win:
	PATH=/cygdrive/c/MinGW/bin:$$PATH make all USE_MINGW=1 MINGW_PREFIX=/cygdrive/c/MinGW/bin/mingw32 LOCAL_SDL_LIB=_build/lib-SDL-devel-1.2.15-mingw32
	cp /cygdrive/c/MinGW/bin/libstdc++-6.dll .
	cp /cygdrive/c/MinGW/bin/libgcc_s_dw2-1.dll .
	cp _build/lib-SDL-devel-1.2.15-mingw32/bin/SDL.dll .

SOURCE_DIR_NAME := ja2-stracciatella_$(VERSION)
build-source-archive:
	mkdir -p $(SRC_RELEASE_BASE_DIR)
	git archive HEAD --prefix=$(SOURCE_DIR_NAME)/ | gzip >$(SRC_RELEASE_BASE_DIR)/$(SOURCE_DIR_NAME).tar.gz

DEB_PKG_BUILD_FOLDER ?= _deb

# sudo apt-get install pbuilder
build-debian-package: build-source-archive
	-rm -rf $(DEB_PKG_BUILD_FOLDER)
	mkdir $(DEB_PKG_BUILD_FOLDER)
	cp $(SRC_RELEASE_BASE_DIR)/$(SOURCE_DIR_NAME).tar.gz $(DEB_PKG_BUILD_FOLDER)/$(SOURCE_DIR_NAME).orig.tar.gz
	cd $(DEB_PKG_BUILD_FOLDER) && tar -xzf $(SOURCE_DIR_NAME).orig.tar.gz
	cp -r _build/deb-package/debian $(DEB_PKG_BUILD_FOLDER)/$(SOURCE_DIR_NAME)
	cd $(DEB_PKG_BUILD_FOLDER)/$(SOURCE_DIR_NAME) && debuild -us -uc
	mkdir -p release-deb-packages
	cp $(DEB_PKG_BUILD_FOLDER)/$(SOURCE_DIR_NAME)-*.deb release-deb-packages
# To debug build issues, go to $(DEB_PKG_BUILD_FOLDER)/$(SOURCE_DIR_NAME) directory and try:
# $ fakeroot debian/rules clean
# $ fakeroot debian/rules binary

# Build Debian packages and the Windows release in
# a totally controlled environment using Vagrant (http://www.vagrantup.com)
build-releases:
	$(MAKE) build-deb-package-on-u1204_i386
	$(MAKE) build-deb-package-on-u1204_amd64
	$(MAKE) build-win-release-on-u1204_amd64_win

build-deb-package-on-u1204_i386:
	$(MAKE) clean
	cd _build/buildboxes/u1204_i386 && vagrant up
	cd _build/buildboxes/u1204_i386 && vagrant ssh -c "make -C /home/vagrant/strac build-debian-package DEB_PKG_BUILD_FOLDER=/home/vagrant/_deb_build_folder"
	cd _build/buildboxes/u1204_i386 && vagrant ssh -c "sudo shutdown -h now"

build-deb-package-on-u1204_amd64:
	$(MAKE) clean
	cd _build/buildboxes/u1204_amd64 && vagrant up
	cd _build/buildboxes/u1204_amd64 && vagrant ssh -c "make -C /home/vagrant/strac build-debian-package DEB_PKG_BUILD_FOLDER=/home/vagrant/_deb_build_folder"
	cd _build/buildboxes/u1204_amd64 && vagrant ssh -c "sudo shutdown -h now"

build-win-release-on-u1204_amd64_win:
	$(MAKE) clean
	cd _build/buildboxes/u1204_amd64_win && vagrant up
	cd _build/buildboxes/u1204_amd64_win && vagrant ssh -c "make -C /home/vagrant/strac build-win-release-on-linux"
	cd _build/buildboxes/u1204_amd64_win && vagrant ssh -c "sudo shutdown -h now"

# Check compilation on different operation systems
check-compilation:
	$(MAKE) check-compilation-on-u1404
	$(MAKE) check-compilation-on-freebsd10
	$(MAKE) check-compilation-on-openbsd55

check-compilation-on-u1404:
	$(MAKE) clean
	cd _build/buildboxes/u1404_amd64 && vagrant up
	cd _build/buildboxes/u1404_amd64 && vagrant ssh -c "make -C /home/vagrant/strac -j2"
	cd _build/buildboxes/u1404_amd64 && vagrant ssh -c "sudo shutdown -h now"

# VirtualBox shared folder cannot be mounted on FreeBSD guest system,
# so we need to copy sources to the box over ssh before compiling them
check-compilation-on-freebsd10:
	$(MAKE) clean
	cd _build/buildboxes/freebsd-10.0 && vagrant up
	cd _build/buildboxes/freebsd-10.0 && vagrant ssh-config >/tmp/strac-freebsd10-ssh-config
	scp -F /tmp/strac-freebsd10-ssh-config _build/buildboxes/freebsd-10.0/bootstrap.sh default:/usr/home/vagrant
	ssh -F /tmp/strac-freebsd10-ssh-config default "/usr/home/vagrant/bootstrap.sh"
	ssh -F /tmp/strac-freebsd10-ssh-config default "rm -rf /usr/home/vagrant/strac"
	ssh -F /tmp/strac-freebsd10-ssh-config default "mkdir /usr/home/vagrant/strac"
	scp -F /tmp/strac-freebsd10-ssh-config -r * default:/usr/home/vagrant/strac
	ssh -F /tmp/strac-freebsd10-ssh-config default "gmake CXX=c++ -C /usr/home/vagrant/strac -j2"
	ssh -F /tmp/strac-freebsd10-ssh-config default "sudo shutdown -p now"

check-compilation-on-openbsd55:
	$(MAKE) clean
	cd _build/buildboxes/openbsd-5.5 && vagrant up
	cd _build/buildboxes/openbsd-5.5 && vagrant ssh-config >/tmp/strac-openbsd55-ssh-config
	scp -F /tmp/strac-openbsd55-ssh-config _build/buildboxes/openbsd-5.5/bootstrap.sh default:/home/vagrant
	ssh -F /tmp/strac-openbsd55-ssh-config default "/home/vagrant/bootstrap.sh"
	ssh -F /tmp/strac-openbsd55-ssh-config default "rm -rf /home/vagrant/strac"
	ssh -F /tmp/strac-openbsd55-ssh-config default "mkdir /home/vagrant/strac"
	scp -F /tmp/strac-openbsd55-ssh-config -r * default:/home/vagrant/strac
	ssh -F /tmp/strac-openbsd55-ssh-config default "gmake CC=egcc CXX=eg++ -C /home/vagrant/strac -j2"
	ssh -F /tmp/strac-openbsd55-ssh-config default "sudo shutdown -hp now"


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
# Check man page
# --------------
#  man ./ja2_manpage
#
#
# Build releases for distribution
#--------------------------------
#
#  Windows release on Linux:
#
#    $ make clean
#    $ make build-win-release-on-linux
#
#  On Mac:
#    $ make clean
#    $ make build-release-on-mac
#
#  Debian packages:
#
#    For the current architecture:
#      $ make build-debian-package
#
