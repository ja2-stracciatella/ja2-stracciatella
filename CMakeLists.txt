cmake_minimum_required (VERSION 2.6)
project (ja2-stracciatella)
set(BINARY "ja2")
set(VERSION "0.15.x")
set(GAME_VERSION "v${VERSION}")

set(EXTRA_DATA_DIR "" CACHE STRING "Directory for externalized data")
set(LOCAL_SDL_LIB "" CACHE STRING "Use local SDL library from this directory")
option(LOCAL_BOOST_LIB "Build with local boost lib" OFF)
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)
option(INSTALLABLE "Should this build be installable" OFF)

set(CFLAGS "${CFLAGS} -DJA2 -DGAME_VERSION=\\\"${GAME_VERSION}\\\" -DMICROINI_STATIC")

if (EXTRA_DATA_DIR STREQUAL "")
    set(CFLAGS "${CFLAGS} -DEXTRA_DATA_DIR=\\\"\\\"")
else()
    MESSAGE( STATUS "Setting extra data dir to" ${EXTRA_DATA_DIR})
    set(CFLAGS "${CFLAGS} -DEXTRA_DATA_DIR=\\\"${EXTRA_DATA_DIR}\\\"")
endif()

if (WITH_FIXMES)
    MESSAGE( STATUS "Building with fixme messages" )
    set(CFLAGS "${CFLAGS} -DWITH_FIXMES")
endif()

if (WITH_MAEMO)
    MESSAGE( STATUS "Building with right click mapped to F4 (menu button)" )
    set(CFLAGS "${CFLAGS} -DWITH_MAEMO")
endif()

if (WITH_SOUND_DEBUG)
    MESSAGE( STATUS "Building with sound debug" )
    set(CFLAGS "${CFLAGS} -DWITH_SOUND_DEBUG")
endif()

if (NOT (LOCAL_SDL_LIB STREQUAL ""))
    MESSAGE( STATUS "Using local SDL from " "${ja2-stracciatella_SOURCE_DIR}/${LOCAL_SDL_LIB}")
    set(CMAKE_PREFIX_PATH "${ja2-stracciatella_SOURCE_DIR}/${LOCAL_SDL_LIB}")
endif()
find_package(SDL REQUIRED)

if (NOT LOCAL_BOOST_LIB)
    find_package(Boost REQUIRED COMPONENTS filesystem system)
else()
    MESSAGE(STATUS "Compiling with local Boost libraries from ${ja2-stracciatella_SOURCE_DIR}/_build/lib-boost" )
    set(Boost_INCLUDE_DIRS "${ja2-stracciatella_SOURCE_DIR}/_build/lib-boost")
    set(Boost_LIBRARIES "")
endif()

include_directories(
    ${ja2-stracciatella_SOURCE_DIR}
    ${ja2-stracciatella_SOURCE_DIR}/Build
    ${ja2-stracciatella_SOURCE_DIR}/Build/Editor
    ${ja2-stracciatella_SOURCE_DIR}/Build/Laptop
    ${ja2-stracciatella_SOURCE_DIR}/Build/Res
    ${ja2-stracciatella_SOURCE_DIR}/Build/Strategic
    ${ja2-stracciatella_SOURCE_DIR}/Build/Tactical
    ${ja2-stracciatella_SOURCE_DIR}/Build/TacticalAI
    ${ja2-stracciatella_SOURCE_DIR}/Build/TileEngine
    ${ja2-stracciatella_SOURCE_DIR}/Build/Utils
    ${ja2-stracciatella_SOURCE_DIR}/sgp
    ${ja2-stracciatella_SOURCE_DIR}/src
    ${ja2-stracciatella_SOURCE_DIR}/src/content
    ${ja2-stracciatella_SOURCE_DIR}/src/internals
    ${ja2-stracciatella_SOURCE_DIR}/src/policy
    ${ja2-stracciatella_SOURCE_DIR}/_build/lib-MicroIni/include
    ${ja2-stracciatella_SOURCE_DIR}/_build/lib-rapidjson
    ${ja2-stracciatella_SOURCE_DIR}/_build/lib-slog
    ${ja2-stracciatella_SOURCE_DIR}/_build/lib-smacker/libsmacker
    ${ja2-stracciatella_SOURCE_DIR}/_build/lib-utf8cpp/source
    ${SDL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

set(SOURCES

    Build/AniViewScreen.cc
    Build/Credits.cc
    Build/UILayout.cc

    Build/Editor/Cursor_Modes.cc
    Build/Editor/EditScreen.cc
    Build/Editor/Edit_Sys.cc
    Build/Editor/EditorBuildings.cc
    Build/Editor/EditorItems.cc
    Build/Editor/EditorMapInfo.cc
    Build/Editor/EditorMercs.cc
    Build/Editor/EditorTerrain.cc
    Build/Editor/Editor_Callbacks.cc
    Build/Editor/Editor_Modes.cc
    Build/Editor/Editor_Taskbar_Creation.cc
    Build/Editor/Editor_Taskbar_Utils.cc
    Build/Editor/Editor_Undo.cc
    Build/Editor/Item_Statistics.cc
    Build/Editor/LoadScreen.cc
    Build/Editor/MessageBox.cc
    Build/Editor/NewSmooth.cc
    Build/Editor/PopupMenu.cc
    Build/Editor/Road_Smoothing.cc
    Build/Editor/Sector_Summary.cc
    Build/Editor/SelectWin.cc
    Build/Editor/SmartMethod.cc
    Build/Editor/Smooth.cc
    Build/Editor/Smoothing_Utils.cc

    Build/Cheats.cc
    Build/Fade_Screen.cc
    Build/GameInitOptionsScreen.cc
    Build/GameLoop.cc
    Build/GameRes.cc
    Build/GameState.cc
    Build/GameScreen.cc
    Build/GameSettings.cc
    Build/GameVersion.cc

    Build/HelpScreen.cc
    Build/Init.cc
    Build/Intro.cc
    Build/JA2_Splash.cc
    Build/JAScreens.cc
    Build/Laptop/AIM.cc
    Build/Laptop/AIMArchives.cc
    Build/Laptop/AIMFacialIndex.cc
    Build/Laptop/AIMHistory.cc
    Build/Laptop/AIMLinks.cc
    Build/Laptop/AIMMembers.cc
    Build/Laptop/AIMPolicies.cc
    Build/Laptop/AIMSort.cc
    Build/Laptop/BobbyR.cc
    Build/Laptop/BobbyRAmmo.cc
    Build/Laptop/BobbyRArmour.cc
    Build/Laptop/BobbyRGuns.cc
    Build/Laptop/BobbyRMailOrder.cc
    Build/Laptop/BobbyRMisc.cc
    Build/Laptop/BobbyRShipments.cc
    Build/Laptop/BobbyRUsed.cc
    Build/Laptop/BrokenLink.cc
    Build/Laptop/CharProfile.cc
    Build/Laptop/EMail.cc
    Build/Laptop/Files.cc
    Build/Laptop/Finances.cc
    Build/Laptop/Florist.cc
    Build/Laptop/Florist_Cards.cc
    Build/Laptop/Florist_Gallery.cc
    Build/Laptop/Florist_Order_Form.cc
    Build/Laptop/Funeral.cc
    Build/Laptop/History.cc
    Build/Laptop/IMPVideoObjects.cc
    Build/Laptop/IMP_AboutUs.cc
    Build/Laptop/IMP_Attribute_Entrance.cc
    Build/Laptop/IMP_Attribute_Finish.cc
    Build/Laptop/IMP_Attribute_Selection.cc
    Build/Laptop/IMP_Begin_Screen.cc
    Build/Laptop/IMP_Compile_Character.cc
    Build/Laptop/IMP_Confirm.cc
    Build/Laptop/IMP_Finish.cc
    Build/Laptop/IMP_HomePage.cc
    Build/Laptop/IMP_MainPage.cc
    Build/Laptop/IMP_Personality_Entrance.cc
    Build/Laptop/IMP_Personality_Finish.cc
    Build/Laptop/IMP_Personality_Quiz.cc
    Build/Laptop/IMP_Portraits.cc
    Build/Laptop/IMP_Text_System.cc
    Build/Laptop/IMP_Voices.cc
    Build/Laptop/Insurance.cc
    Build/Laptop/Insurance_Comments.cc
    Build/Laptop/Insurance_Contract.cc
    Build/Laptop/Insurance_Info.cc
    Build/Laptop/Laptop.cc
    Build/Laptop/Mercs.cc
    Build/Laptop/Mercs_Account.cc
    Build/Laptop/Mercs_Files.cc
    Build/Laptop/Mercs_No_Account.cc
    Build/Laptop/Personnel.cc
    Build/Laptop/Store_Inventory.cc
    Build/LoadSaveEMail.cc
    Build/LoadSaveTacticalStatusType.cc
    Build/Loading_Screen.cc
    Build/MainMenuScreen.cc
    Build/MercPortrait.cc
    Build/MessageBoxScreen.cc
    Build/Options_Screen.cc
    Build/SaveLoadGame.cc
    Build/SaveLoadScreen.cc
    Build/Screens.cc
    Build/Strategic/AI_Viewer.cc
    Build/Strategic/Assignments.cc
    Build/Strategic/Auto_Resolve.cc
    Build/Strategic/Campaign_Init.cc
    Build/Strategic/Creature_Spreading.cc
    Build/Strategic/Game_Clock.cc
    Build/Strategic/Game_Event_Hook.cc
    Build/Strategic/Game_Events.cc
    Build/Strategic/Game_Init.cc
    Build/Strategic/Hourly_Update.cc
    Build/Strategic/LoadSaveSectorInfo.cc
    Build/Strategic/LoadSaveStrategicMapElement.cc
    Build/Strategic/LoadSaveUndergroundSectorInfo.cc
    Build/Strategic/MapScreen.cc
    Build/Strategic/Map_Screen_Helicopter.cc
    Build/Strategic/Map_Screen_Interface.cc
    Build/Strategic/Map_Screen_Interface_Border.cc
    Build/Strategic/Map_Screen_Interface_Bottom.cc
    Build/Strategic/Map_Screen_Interface_Map.cc
    Build/Strategic/Map_Screen_Interface_Map_Inventory.cc
    Build/Strategic/Map_Screen_Interface_TownMine_Info.cc
    Build/Strategic/Meanwhile.cc
    Build/Strategic/Merc_Contract.cc
    Build/Strategic/Player_Command.cc
    Build/Strategic/PreBattle_Interface.cc
    Build/Strategic/Queen_Command.cc
    Build/Strategic/QuestText.cc
    Build/Strategic/Quest_Debug_System.cc
    Build/Strategic/Quests.cc
    Build/Strategic/Scheduling.cc
    Build/Strategic/Strategic.cc
    Build/Strategic/StrategicMap.cc
    Build/Strategic/Strategic_AI.cc
    Build/Strategic/Strategic_Event_Handler.cc
    Build/Strategic/Strategic_Merc_Handler.cc
    Build/Strategic/Strategic_Mines.cc
    Build/Strategic/Strategic_Movement.cc
    Build/Strategic/Strategic_Movement_Costs.cc
    Build/Strategic/Strategic_Pathing.cc
    Build/Strategic/Strategic_Status.cc
    Build/Strategic/Strategic_Town_Loyalty.cc
    Build/Strategic/Strategic_Turns.cc
    Build/Strategic/Town_Militia.cc
    Build/Sys_Globals.cc
    Build/Tactical/Air_Raid.cc
    Build/Tactical/Animation_Cache.cc
    Build/Tactical/Animation_Control.cc
    Build/Tactical/Animation_Data.cc
    Build/Tactical/ArmsDealerInvInit.cc
    Build/Tactical/Arms_Dealer_Init.cc
    Build/Tactical/Auto_Bandage.cc
    Build/Tactical/Boxing.cc
    Build/Tactical/Bullets.cc
    Build/Tactical/Campaign.cc
    Build/Tactical/Civ_Quotes.cc
    Build/Tactical/Dialogue_Control.cc
    Build/Tactical/DisplayCover.cc
    Build/Tactical/Drugs_And_Alcohol.cc
    Build/Tactical/End_Game.cc
    Build/Tactical/Enemy_Soldier_Save.cc
    Build/Tactical/FOV.cc
    Build/Tactical/Faces.cc
    Build/Tactical/Gap.cc
    Build/Tactical/Handle_Doors.cc
    Build/Tactical/Handle_Items.cc
    Build/Tactical/Handle_UI.cc
    Build/Tactical/Interface.cc
    Build/Tactical/Interface_Control.cc
    Build/Tactical/Interface_Cursors.cc
    Build/Tactical/Interface_Dialogue.cc
    Build/Tactical/Interface_Items.cc
    Build/Tactical/Interface_Panels.cc
    Build/Tactical/Interface_Utils.cc
    Build/Tactical/Inventory_Choosing.cc
    Build/Tactical/Items.cc
    Build/Tactical/Keys.cc
    Build/Tactical/LOS.cc
    Build/Tactical/LoadSaveBasicSoldierCreateStruct.cc
    Build/Tactical/LoadSaveBullet.cc
    Build/Tactical/LoadSaveMercProfile.cc
    Build/Tactical/LoadSaveObjectType.cc
    Build/Tactical/LoadSaveRottingCorpse.cc
    Build/Tactical/LoadSaveSoldierCreate.cc
    Build/Tactical/LoadSaveSoldierType.cc
    Build/Tactical/LoadSaveVehicleType.cc
    Build/Tactical/Map_Information.cc
    Build/Tactical/Merc_Entering.cc
    Build/Tactical/Merc_Hiring.cc
    Build/Tactical/Militia_Control.cc
    Build/Tactical/Morale.cc
    Build/Tactical/OppList.cc
    Build/Tactical/Overhead.cc
    Build/Tactical/PathAI.cc
    Build/Tactical/Points.cc
    Build/Tactical/QArray.cc
    Build/Tactical/Real_Time_Input.cc
    Build/Tactical/Rotting_Corpses.cc
    Build/Tactical/ShopKeeper_Interface.cc
    Build/Tactical/SkillCheck.cc
    Build/Tactical/Soldier_Add.cc
    Build/Tactical/Soldier_Ani.cc
    Build/Tactical/Soldier_Control.cc
    Build/Tactical/Soldier_Create.cc
    Build/Tactical/Soldier_Find.cc
    Build/Tactical/Soldier_Init_List.cc
    Build/Tactical/Soldier_Profile.cc
    Build/Tactical/Soldier_Tile.cc
    Build/Tactical/Spread_Burst.cc
    Build/Tactical/Squads.cc
    Build/Tactical/Strategic_Exit_GUI.cc
    Build/Tactical/Structure_Wrap.cc
    Build/Tactical/Tactical_Save.cc
    Build/Tactical/Tactical_Turns.cc
    Build/Tactical/TeamTurns.cc
    Build/Tactical/Turn_Based_Input.cc
    Build/Tactical/UI_Cursors.cc
    Build/Tactical/Vehicles.cc
    Build/Tactical/Weapons.cc
    Build/Tactical/World_Items.cc
    Build/TacticalAI/AIList.cc
    Build/TacticalAI/AIMain.cc
    Build/TacticalAI/AIUtils.cc
    Build/TacticalAI/Attacks.cc
    Build/TacticalAI/CreatureDecideAction.cc
    Build/TacticalAI/DecideAction.cc
    Build/TacticalAI/FindLocations.cc
    Build/TacticalAI/Knowledge.cc
    Build/TacticalAI/Medical.cc
    Build/TacticalAI/Movement.cc
    Build/TacticalAI/NPC.cc
    Build/TacticalAI/PanicButtons.cc
    Build/TacticalAI/Realtime.cc
    Build/TileEngine/Ambient_Control.cc
    Build/TileEngine/Buildings.cc
    Build/TileEngine/Environment.cc
    Build/TileEngine/Exit_Grids.cc
    Build/TileEngine/Explosion_Control.cc
    Build/TileEngine/Fog_Of_War.cc
    Build/TileEngine/Interactive_Tiles.cc
    Build/TileEngine/Isometric_Utils.cc
    Build/TileEngine/LightEffects.cc
    Build/TileEngine/Lighting.cc
    Build/TileEngine/LoadSaveExplosionType.cc
    Build/TileEngine/LoadSaveLightEffect.cc
    Build/TileEngine/LoadSaveLightSprite.cc
    Build/TileEngine/LoadSaveRealObject.cc
    Build/TileEngine/LoadSaveSmokeEffect.cc
    Build/TileEngine/Map_Edgepoints.cc
    Build/TileEngine/Overhead_Map.cc
    Build/TileEngine/Phys_Math.cc
    Build/TileEngine/Physics.cc
    Build/TileEngine/Pits.cc
    Build/TileEngine/Radar_Screen.cc
    Build/TileEngine/RenderWorld.cc
    Build/TileEngine/Render_Dirty.cc
    Build/TileEngine/Render_Fun.cc
    Build/TileEngine/SaveLoadMap.cc
    Build/TileEngine/Simple_Render_Utils.cc
    Build/TileEngine/Smell.cc
    Build/TileEngine/SmokeEffects.cc
    Build/TileEngine/Structure.cc
    Build/TileEngine/SysUtil.cc
    Build/TileEngine/Tactical_Placement_GUI.cc
    Build/TileEngine/TileDat.cc
    Build/TileEngine/TileDef.cc
    Build/TileEngine/Tile_Animation.cc
    Build/TileEngine/Tile_Cache.cc
    Build/TileEngine/Tile_Surface.cc
    Build/TileEngine/WorldDat.cc
    Build/TileEngine/WorldDef.cc
    Build/TileEngine/WorldMan.cc
    Build/Utils/Animated_ProgressBar.cc
    Build/Utils/Cinematics.cc
    Build/Utils/Cursors.cc
    Build/Utils/Debug_Pages.cc
    Build/Utils/Event_Manager.cc
    Build/Utils/Event_Pump.cc
    Build/Utils/Font_Control.cc
    Build/Utils/MapUtility.cc
    Build/Utils/MercTextBox.cc
    Build/Utils/Message.cc
    Build/Utils/Music_Control.cc
    Build/Utils/PopUpBox.cc
    Build/Utils/Quantize.cc
    Build/Utils/STIConvert.cc
    Build/Utils/Slider.cc
    Build/Utils/Sound_Control.cc
    Build/Utils/Text.cc
    Build/Utils/Text_Input.cc
    Build/Utils/Text_Utils.cc
    Build/Utils/Timer_Control.cc
    Build/Utils/Utilities.cc
    Build/Utils/WordWrap.cc
    sgp/Button_Sound_Control.cc
    sgp/Button_System.cc
    sgp/Cursor_Control.cc
    sgp/Debug.cc
    sgp/EncodingCorrectors.cc
    sgp/FileMan.cc
    sgp/Font.cc
    sgp/HImage.cc
    sgp/ImpTGA.cc
    sgp/Input.cc
    sgp/LibraryDataBase.cc
    sgp/Line.cc
    sgp/LoadSaveData.cc
    sgp/MemMan.cc
    sgp/MouseSystem.cc
    sgp/PCX.cc
    sgp/Random.cc
    sgp/SGP.cc
    sgp/SGPStrings.cc
    sgp/STCI.cc
    sgp/Shading.cc
    sgp/Smack_Stub.cc
    sgp/SoundMan.cc
    sgp/StrUtils.cc
    sgp/TranslationTable.cc
    sgp/UTF8String.cc
    sgp/VObject.cc
    sgp/VObject_Blitters.cc
    sgp/VSurface.cc
    sgp/Video.cc

    src/AmmoTypeModel.cc
    src/CalibreModel.cc
    src/DealerInventory.cc
    src/DefaultContentManager.cc
    src/ItemModel.cc
    src/JsonUtility.cc
    src/MagazineModel.cc
    src/MercProfile.cc
    src/ModPackContentManager.cc
    src/Soldier.cc
    src/WeaponModels.cc
    src/content/ContentMercs.cc
    src/content/Dialogs.cc
    src/content/npcs.cc
    src/internals/enums.cc
    src/policy/DefaultGamePolicy.cc
    src/policy/DefaultIMPPolicy.cc

    _build/lib-MicroIni/src/MicroIni/File.cpp
    _build/lib-MicroIni/src/MicroIni/Line.cpp
    _build/lib-MicroIni/src/MicroIni/Section.cpp
    _build/lib-MicroIni/src/MicroIni/Value.cpp

    _build/lib-slog/slog/slog.c

    _build/lib-smacker/libsmacker/smacker.c
    _build/lib-smacker/libsmacker/smk_hufftree.c
    _build/lib-smacker/libsmacker/smk_bitstream.c

    Build/Utils/_DutchText.cc
    Build/Utils/_EnglishText.cc
    Build/Utils/_FrenchText.cc
    Build/Utils/_GermanText.cc
    Build/Utils/_ItalianText.cc
    Build/Utils/_PolishText.cc
    Build/Utils/_RussianText.cc
)

if (LOCAL_BOOST_LIB)
    LIST(APPEND SOURCES
        _build/lib-boost/libs/system/src/error_code.cpp
        _build/lib-boost/libs/filesystem/src/codecvt_error_category.cpp
        _build/lib-boost/libs/filesystem/src/operations.cpp
        _build/lib-boost/libs/filesystem/src/path.cpp
        _build/lib-boost/libs/filesystem/src/path_traits.cpp
        _build/lib-boost/libs/filesystem/src/portability.cpp
        _build/lib-boost/libs/filesystem/src/unique_path.cpp
        _build/lib-boost/libs/filesystem/src/utf8_codecvt_facet.cpp
        _build/lib-boost/libs/filesystem/src/windows_file_codecvt.cpp
    )
endif()

if (WITH_UNITTESTS)
    MESSAGE(STATUS "Compiling with unittests" )

    set(CFLAGS "${CFLAGS} -DWITH_UNITTESTS")
    include_directories(
        ${ja2-stracciatella_SOURCE_DIR}/_build/lib-gtest/include
        ${ja2-stracciatella_SOURCE_DIR}/_build/lib-gtest
    )
    LIST(APPEND SOURCES
        _build/lib-gtest/src/gtest.cc
        _build/lib-gtest/src/gtest-death-test.cc
        _build/lib-gtest/src/gtest-filepath.cc
        _build/lib-gtest/src/gtest-port.cc
        _build/lib-gtest/src/gtest-printers.cc
        _build/lib-gtest/src/gtest-test-part.cc
        _build/lib-gtest/src/gtest-typed-test.cc
        Build/SaveLoadGame_unittest.cc
        Build/Tactical/LoadSaveMercProfile_unittest.cc
        Build/VanillaDataStructures_unittest.cc
        sgp/FileMan_unittest.cc
        sgp/LoadSaveData_unittest.cc
        sgp/UTF8String_unittest.cc
        sgp/wchar_unittest.cc
        src/DefaultContentManagerUT.cc
        src/DefaultContentManager_unittests.cc
        src/JsonUtility_unittests.cc
        src/VanillaWeapons_unittests.cc
        src/TestUtils.cc
    )
endif()

set(CMAKE_CXX_FLAGS ${CFLAGS})
add_executable(${BINARY} ${SOURCES})
target_link_libraries(${BINARY} ${SDL_LIBRARY} ${Boost_LIBRARIES})
add_custom_command(TARGET ${BINARY} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/externalized $<TARGET_FILE_DIR:${BINARY}>)

if (INSTALLABLE)
    INSTALL(TARGETS ${BINARY} RUNTIME DESTINATION bin)
    INSTALL(DIRECTORY externalized mods _unittests DESTINATION share/ja2)
    INSTALL(FILES _build/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
    INSTALL(FILES _build/icons/logo.svg DESTINATION share/icons/$(ICON_THEME)/scalable/apps/ja2-stracciatella.svg)
    INSTALL(FILES ja2_manpage DESTINATION share/man/man6/ja2.6)
endif()
