add_and_include_subdirectory("army")
add_and_include_subdirectory("content")
add_and_include_subdirectory("internals")
add_and_include_subdirectory("json")
add_and_include_subdirectory("mercs")
add_and_include_subdirectory("policy")
add_and_include_subdirectory("scripting")
add_and_include_subdirectory("strategic")
add_and_include_subdirectory("strings")
add_and_include_subdirectory("tileset")
add_and_include_subdirectory("tactical")
add_and_include_subdirectory("ui")

file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(LOCAL_JA2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/AmmoTypeModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ArmourModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/CalibreModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DealerModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DealerInventory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DefaultContentManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ExplosionAnimationModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ExplosiveModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/InventoryGraphicsModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ItemModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonUtility.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ExplosiveCalibreModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadingScreenModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MagazineModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ModPackContentManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/RustInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ShippingDestinationModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/VehicleModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/WeaponModels.cc
)

if (WITH_UNITTESTS)
    set(LOCAL_JA2_SOURCES
        ${LOCAL_JA2_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/DefaultContentManagerUT.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/DefaultContentManager_unittests.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/JsonUtility_unittests.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/VanillaWeapons_unittests.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/TestUtils.cc
    )
endif()

set_property(
    SOURCE ${LOCAL_JA2_SOURCES}
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "externalized"
)

set(JA2_SOURCES
    ${JA2_SOURCES}
    ${LOCAL_JA2_HEADERS}
    ${LOCAL_JA2_SOURCES}
    PARENT_SCOPE
)
