# \file dependencies/lib-sdl2/CMakeLists.txt

message(STATUS "<sdl2>")

# create builder
set(CACHE_FILE "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake")
set(INSTALL_PREFIX "${CMAKE_BINARY_DIR}/lib-sdl2")
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
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/builder/Bug5100.patch"
    "${CMAKE_CURRENT_BINARY_DIR}/builder/Bug5100.patch"
    @ONLY
)

# execute builder
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/builder")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/builder")

# interface library
include("${INSTALL_PREFIX}/lib/cmake/SDL2/SDL2Config.cmake")

set(SDL2_INCLUDE_DIR "${INSTALL_PREFIX}/include/SDL2" PARENT_SCOPE)

add_library(sdl2-internal INTERFACE)
target_include_directories(sdl2-internal INTERFACE "${SDL2_INCLUDE_DIR}")
target_link_libraries(sdl2-internal INTERFACE
    $<TARGET_PROPERTY:SDL2::SDL2,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:SDL2::SDL2,IMPORTED_LOCATION_RELWITHDEBINFO>
    $<TARGET_PROPERTY:SDL2::hidapi,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:SDL2::hidapi,IMPORTED_LOCATION_RELWITHDEBINFO>
    $<TARGET_PROPERTY:SDL2::SDL2main,IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO>
    $<TARGET_PROPERTY:SDL2::SDL2main,IMPORTED_LOCATION_RELWITHDEBINFO>
)

set(SDL2_LIBRARY sdl2-internal PARENT_SCOPE)

message(STATUS "</sdl2>")
