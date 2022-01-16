add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/army")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/content")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/internals")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mercs")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/policy")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/scripting")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/strategic")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tileset")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tactical")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ui")

file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(LOCAL_JA2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/AmmoTypeModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/CalibreModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DealerModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DealerInventory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/DefaultContentManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/InventoryGraphicsModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ItemModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonUtility.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/LoadingScreenModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MagazineModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ModPackContentManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ShippingDestinationModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Soldier.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/SubImageModel.cc
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

set(JA2_SOURCES
    ${JA2_SOURCES}
    ${LOCAL_JA2_HEADERS}
    ${LOCAL_JA2_SOURCES}
    PARENT_SCOPE
)
set(JA2_INCLUDES
    ${JA2_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
