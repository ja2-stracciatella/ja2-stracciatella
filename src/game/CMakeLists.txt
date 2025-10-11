add_and_include_subdirectory("Editor")
add_and_include_subdirectory("Laptop")
add_and_include_subdirectory("Strategic")
add_and_include_subdirectory("Tactical")
add_and_include_subdirectory("TacticalAI")
add_and_include_subdirectory("TileEngine")
add_and_include_subdirectory("Utils")

file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(LOCAL_JA2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Credits.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/UILayout.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Fade_Screen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameInitOptionsScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameLoop.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameRes.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameMode.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameSettings.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GameVersion.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/HelpScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Init.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Intro.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/JA2_Splash.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/JAScreens.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveEMail.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveTacticalStatusType.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Loading_Screen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MainMenuScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MercPortrait.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageBoxScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Options_Screen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SaveLoadGame.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SaveLoadGameStates.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SaveLoadScreen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Screens.cc
)

if (WITH_UNITTESTS)
    set(LOCAL_JA2_SOURCES
        ${LOCAL_JA2_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/SaveLoadGame_unittest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/SaveLoadGameStates_unittest.cc
    )
endif()

set_property(
    SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveEMail.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveTacticalStatusType.cc
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "LoadSaveDataTypes"
)

set(JA2_SOURCES
    ${JA2_SOURCES}
    ${LOCAL_JA2_SOURCES}
    ${LOCAL_JA2_HEADERS}
    PARENT_SCOPE
)
