file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(LOCAL_JA2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Animation_Cache.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Animation_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Animation_Data.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ArmsDealerInvInit.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Arms_Dealer_Init.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Auto_Bandage.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Boxing.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Bullets.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Campaign.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Civ_Quotes.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialogue_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DisplayCover.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drugs_And_Alcohol.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/End_Game.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Enemy_Soldier_Save.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/FOV.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Faces.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Gap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Handle_Doors.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Handle_Items.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Handle_UI.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Cursors.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Dialogue.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Items.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Panels.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inventory_Choosing.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Items.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Keys.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LOS.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveBasicSoldierCreateStruct.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveBullet.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveMercProfile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveObjectType.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveRottingCorpse.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveSoldierCreate.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveSoldierType.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveVehicleType.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Map_Information.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Merc_Entering.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Merc_Hiring.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Militia_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Morale.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/OppList.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Overhead.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/PathAI.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Points.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/QArray.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Real_Time_Input.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Rotting_Corpses.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ShopKeeper_Interface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SkillCheck.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Add.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Ani.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Create.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Find.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Init_List.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Profile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier_Tile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Spread_Burst.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Squads.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Strategic_Exit_GUI.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Structure_Wrap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Tactical_Save.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Tactical_Turns.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/TeamTurns.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Touch_UI.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Turn_Based_Input.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/UI_Cursors.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Vehicles.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Weapons.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/World_Items.cc
)

if (WITH_UNITTESTS)
    set(LOCAL_JA2_SOURCES
        ${LOCAL_JA2_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveMercProfile_unittest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Weapons_unittest.cc
    )
endif()

set_property(
    SOURCE ${LOCAL_JA2_SOURCES}
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "Tactical"
)

# Interface_Panels has enum values that clash with Interface.cc
# PathAI defines some macros that cause problems in other source files.
set_property(
    SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/FOV.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Interface_Panels.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/PathAI.cc
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "Tactical_Others"
)

set_property(
    SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveBasicSoldierCreateStruct.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveBullet.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveMercProfile.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveObjectType.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveRottingCorpse.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveSoldierCreate.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveSoldierType.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveVehicleType.cc
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "LoadSaveDataTypes"
)

set(JA2_SOURCES
    ${JA2_SOURCES}
    ${LOCAL_JA2_SOURCES}
    ${LOCAL_JA2_HEADERS}
    PARENT_SCOPE
)
