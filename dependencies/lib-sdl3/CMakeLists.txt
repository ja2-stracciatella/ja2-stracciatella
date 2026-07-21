# \file dependencies/lib-sdl3/CMakeLists.txt

message(STATUS "<sdl3>")

# create builder
set(CACHE_FILE "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake")
set(INSTALL_PREFIX "${CMAKE_BINARY_DIR}/lib-sdl3")
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
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/builder")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/builder")

# interface library
include("${INSTALL_PREFIX}/lib/cmake/SDL3/SDL3Config.cmake")

foreach(_sdl3_target SDL3::SDL3-shared SDL3::Headers)
    if(TARGET ${_sdl3_target})
        set_target_properties(${_sdl3_target} PROPERTIES IMPORTED_GLOBAL TRUE)
    endif()
endforeach()

message(STATUS "</sdl3>")
