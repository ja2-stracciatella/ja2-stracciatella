# \file dependencies/lib-miniaudio/CMakeLists.txt

option(LOCAL_MINIAUDIO_LIB "Download and build miniaudio instead of searching the system" ON)
if (NOT LOCAL_MINIAUDIO_LIB)
    message(STATUS "Using system miniaudio")

    if (NOT DEFINED MINIAUDIO_INCLUDE_DIR)
        message(FATAL_ERROR "MINIAUDIO_INCLUDE_DIR var not set")
    endif()

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

    include(CheckCXXSourceCompiles)

    # miniaudio doesn't do semver properly, check if any miniaudio-related build
    # failures have something to do with ABI changes that the semver doesn't
    # suggest.
    CHECK_CXX_SOURCE_COMPILES(
        "
            #include <miniaudio.h>
            int main() {
                static_assert(MA_VERSION_MAJOR == 0);
                static_assert(MA_VERSION_MINOR == 10);
                static_assert(MA_VERSION_REVISION >= 43);
            }
        "
        HAVE_MA_VERSION
    )
    if (NOT HAVE_MA_VERSION)
        message(FATAL_ERROR "miniaudio does not have correct version, should be
        >= 0.10.43")
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
