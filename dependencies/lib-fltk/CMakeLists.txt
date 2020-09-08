# \file dependencies/lib-fltk/CMakeLists.txt

message(STATUS "<fltk>")

# create builder
set(CACHE_FILE "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake")
set(INSTALL_PREFIX "${CMAKE_BINARY_DIR}/lib-fltk")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/external-project-cache.cmake.in"
    "${CACHE_FILE}"
    @ONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/builder/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/builder/CMakeLists.txt"
    @ONLY
)

# execute builder
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/builder")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/builder")

# interface library
find_file(FLTK_CONFIG "FLTKConfig.cmake"
    PATHS
        "${INSTALL_PREFIX}/CMake"
        "${INSTALL_PREFIX}/FLTK/.framework/Resources/CMake"
        "${INSTALL_PREFIX}/fltk"
    NO_DEFAULT_PATH
)
include("${FLTK_CONFIG}")
add_library(fltk-internal INTERFACE)
target_include_directories(fltk-internal INTERFACE "${FLTK_INCLUDE_DIRS}")
target_link_libraries(fltk-internal INTERFACE
    $<TARGET_PROPERTY:fltk_images,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_images,IMPORTED_LOCATION_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk,IMPORTED_LOCATION_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_forms,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_forms,IMPORTED_LOCATION_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_png,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_png,IMPORTED_LOCATION_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_z,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:fltk_z,IMPORTED_LOCATION_RELWITHDEBINFO>
)
set(FLTK_LIBRARIES fltk-internal PARENT_SCOPE)

message(STATUS "</fltk>")
