file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(LOCAL_JA2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Ambient_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Buildings.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Environment.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Exit_Grids.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Explosion_Control.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Fog_Of_War.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Interactive_Tiles.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Isometric_Utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LightEffects.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Lighting.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveExplosionType.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveLightEffect.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveLightSprite.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveRealObject.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveSmokeEffect.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Map_Edgepoints.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Overhead_Map.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Phys_Math.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Physics.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Pits.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Radar_Screen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderWorld.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Render_Dirty.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Render_Fun.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SaveLoadMap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Simple_Render_Utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Smell.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SmokeEffects.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Structure.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SysUtil.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Tactical_Placement_GUI.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/TileDat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/TileDef.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Tile_Animation.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Tile_Cache.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Tile_Surface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/WorldDat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/WorldDef.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/WorldMan.cc
)

set_property(
    SOURCE ${LOCAL_JA2_SOURCES}
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "TileEngine"
)

set_property(
    SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveExplosionType.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveLightEffect.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveLightSprite.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveRealObject.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/LoadSaveSmokeEffect.cc
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "LoadSaveDataTypes"
)

set(JA2_SOURCES
    ${JA2_SOURCES}
    ${LOCAL_JA2_SOURCES}
    ${LOCAL_JA2_HEADERS}
    PARENT_SCOPE
)

set(JA2_INCLUDES
    ${JA2_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
