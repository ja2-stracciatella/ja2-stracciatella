# \file dependencies/lib-miniaudio/CMakeLists.txt

option(LOCAL_MINIAUDIO_LIB "Download and build miniaudio instead of searching the system" ON)
if (NOT LOCAL_MINIAUDIO_LIB)
    message(STATUS "Using system miniaudio")

    include(CheckIncludeFileCXX)
    set (CMAKE_REQUIRED_INCLUDES "${MINIAUDIO_INCLUDE_DIR}")

    CHECK_INCLUDE_FILE_CXX("miniaudio.h" MINIAUDIO_FOUND)
    if (NOT MINIAUDIO_FOUND)
        message(FATAL_ERROR "miniaudio.h not found")
    endif()

    CHECK_INCLUDE_FILE_CXX("extras/stb_vorbis.c" STB_VORBIS_FOUND)
    if (NOT STB_VORBIS_FOUND)
        message(FATAL_ERROR "miniaudio extras/stb_vorbis.c not found")
    endif()

    set(MINIAUDIO_INCLUDE_DIRS "${MINIAUDIO_INCLUDE_DIR}" PARENT_SCOPE)

    return()
endif()

message(STATUS "<miniaudio>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-miniaudio/src")
set(BUILD_DIR "${CMAKE_BINARY_DIR}/lib-miniaudio/build")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

set(MINIAUDIO_INCLUDE_DIRS ${SRC_DIR} PARENT_SCOPE)

message(STATUS "</miniaudio>")
