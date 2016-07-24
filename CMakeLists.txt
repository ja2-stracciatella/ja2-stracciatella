cmake_minimum_required (VERSION 2.8)

## Some macros

macro(add_custom_templated_target NAME)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/target-${NAME}.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake/target-${NAME}.cmake"
            IMMEDIATE @ONLY)

    add_custom_target(${NAME}
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/target-${NAME}.cmake)
endmacro()

## Project Setup

project (ja2-stracciatella)
set(BINARY "ja2")

## Versioning

SET(ja2-stracciatella_VERSION_MAJOR "0")
SET(ja2-stracciatella_VERSION_MINOR "15")
SET(ja2-stracciatella_VERSION_PATCH "x")
set(ja2-stracciatella_VERSION "${ja2-stracciatella_VERSION_MAJOR}.${ja2-stracciatella_VERSION_MINOR}.${ja2-stracciatella_VERSION_PATCH}")
set(GAME_VERSION "v${ja2-stracciatella_VERSION}")

## Meta Information
set(CONTACT "JA2 Stracciatella Team <no-email@ja2-stracciatella.github.io>")
set(DESCRIPTION "An improved, cross-platform, stable Jagged Alliance 2 runtime.")

## Options

set(EXTRA_DATA_DIR "" CACHE STRING "Directory for externalized data")
set(LOCAL_SDL_LIB "" CACHE STRING "Use local SDL library from this directory")
option(LOCAL_BOOST_LIB "Build with local boost lib" OFF)
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)

## Build

SET(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(CFLAGS "${CFLAGS} -DJA2 -DGAME_VERSION=\\\"${GAME_VERSION}\\\" -DMICROINI_STATIC")

if (EXTRA_DATA_DIR STREQUAL "")
    set(CFLAGS "${CFLAGS} -DEXTRA_DATA_DIR=\\\"\\\"")
else()
    MESSAGE( STATUS "Setting extra data dir to" "${EXTRA_DATA_DIR}")
    set(CFLAGS "${CFLAGS} -DEXTRA_DATA_DIR=\\\"${EXTRA_DATA_DIR}\\\"")
endif()

if (WITH_FIXMES)
    MESSAGE( STATUS "Building with fixme messages" )
    set(CFLAGS "${CFLAGS} -DWITH_FIXMES")
endif()

if (WITH_MAEMO)
    MESSAGE( STATUS "Building with right click mapped to F4 (menu button)" )
    set(CFLAGS "${CFLAGS} -DWITH_MAEMO")
endif()

if (WITH_SOUND_DEBUG)
    MESSAGE( STATUS "Building with sound debug" )
    set(CFLAGS "${CFLAGS} -DWITH_SOUND_DEBUG")
endif()

if (NOT (LOCAL_SDL_LIB STREQUAL ""))
    MESSAGE( STATUS "Using local SDL from " "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SDL_LIB}")
    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SDL_LIB}")
endif()
find_package(SDL REQUIRED)

if (NOT LOCAL_BOOST_LIB)
    find_package(Boost REQUIRED COMPONENTS filesystem system)
else()
    MESSAGE(STATUS "Compiling with local Boost libraries from ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-boost" )
    add_subdirectory("_build/lib-boost")
    set(Boost_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-boost")
    set(Boost_LIBRARIES "boost")
endif()

set(JA2_INCLUDES "")
set(JA2_SOURCES "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Build")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/sgp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")
add_subdirectory("_build/lib-MicroIni")
add_subdirectory("_build/lib-slog")
add_subdirectory("_build/lib-smacker")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JA2_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-MicroIni/include
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-rapidjson
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-slog
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-smacker/libsmacker
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-utf8cpp/source
    ${SDL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

foreach(FILE ${JA2_SOURCES})
  get_filename_component(PARENT_DIR "${FILE}" PATH)
  file(RELATIVE_PATH GROUP "${CMAKE_CURRENT_SOURCE_DIR}" "${PARENT_DIR}")
  string(REPLACE "/" "\\" GROUP "${GROUP}")
  source_group("${GROUP}" FILES "${FILE}")
endforeach()

if (WITH_UNITTESTS)
    MESSAGE(STATUS "Compiling with unittests" )

    add_subdirectory("_build/lib-gtest")
    set(lib-gtest "gtest")
    set(CFLAGS "${CFLAGS} -DWITH_UNITTESTS")

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-gtest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-gtest
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
add_executable(${BINARY} ${JA2_SOURCES})
target_link_libraries(${BINARY} ${SDL_LIBRARY} ${Boost_LIBRARIES} ${lib-gtest} MicroIni slog smacker)

macro(copy_dir_to_ja2_binary_after_build DIR)
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR} "$<TARGET_FILE_DIR:${BINARY}>/${DIR}")
endmacro()

copy_dir_to_ja2_binary_after_build("externalized")
copy_dir_to_ja2_binary_after_build("_unittests")
copy_dir_to_ja2_binary_after_build("mods")

if (MSVC)
    STRING(REPLACE "SDLmain.lib" "SDL.dll" SDL_DLL ${SDLMAIN_LIBRARY})
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SDL_DLL} "$<TARGET_FILE_DIR:${BINARY}>")
endif()

## Installing and Packaging

SET(CPACK_PACKAGE_VERSION_MAJOR ${ja2-stracciatella_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${ja2-stracciatella_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${ja2-stracciatella_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${ja2-stracciatella_VERSION})
SET(CPACK_PACKAGE_CONTACT ${CONTACT})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DESCRIPTION})
SET(CPACK_PACKAGE_DESCRIPTION ${DESCRIPTION})

SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://ja2-stracciatella.github.io/")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl1.2debian, libstdc++6, libgcc1, libc6")

if(UNIX AND NOT MINGW AND NOT APPLE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PACKAGE_ARCH "amd64")
    else()
        set(PACKAGE_ARCH "i386")
    endif()
elseif(MINGW)
    SET(PACKAGE_ARCH "win-mingw")
elseif(APPLE)
    SET(PACKAGE_ARCH "macos")
else()
    SET(PACKAGE_ARCH "unknown")
endif()
SET(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_ARCH}")

INCLUDE(CPack)

if (UNIX AND NOT MINGW AND NOT APPLE)
    INSTALL(TARGETS ${BINARY} RUNTIME DESTINATION bin)
    INSTALL(DIRECTORY externalized mods _unittests DESTINATION share/ja2)
    INSTALL(FILES _build/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
    INSTALL(FILES _build/icons/logo.svg DESTINATION share/icons/hicolor/scalable/apps/ja2-stracciatella.svg)
    INSTALL(FILES ja2_manpage DESTINATION share/man/man6/ja2.6)
else()
    INSTALL(TARGETS ${BINARY} RUNTIME DESTINATION .)
    INSTALL(DIRECTORY externalized mods _unittests DESTINATION .)
    INSTALL(FILES changes.md DESTINATION .)
endif()

if (MINGW)
    file(GLOB WIN_MINGW_DIST_DLLS "_build/distr-files-win-mingw/*.dll")
    INSTALL(FILES ${WIN_MINGW_DIST_DLLS} DESTINATION .)
endif()

if(WIN32 OR MINGW)
    file(GLOB WIN_DIST_FILES "_build/distr-files-win/*")
    INSTALL(FILES ${WIN_DIST_FILES} DESTINATION .)
    INSTALL(FILES _build/icons/logo.ico DESTINATION .)
    INSTALL(FILES _build/lib-SDL-devel-1.2.15-mingw32/bin/SDL.dll DESTINATION .)
endif()

if(APPLE)
    file(GLOB APPLE_DIST_FILES "_build/distr-files-mac/*")
    INSTALL(FILES ${APPLE_DIST_FILES} DESTINATION .)
    INSTALL(FILES _build/icons/logo.icns DESTINATION .)
endif()

add_custom_templated_target("uninstall")

## Release building

macro(add_vagrant_build_target NAME BOX VM TARGET ARGS)
    add_custom_target(${NAME})
    add_custom_command(TARGET ${NAME}
            COMMAND vagrant up ${VM} && vagrant ssh -c "sh /tmp/ja2-stracciatella/_build/buildboxes/common/build.sh '${TARGET}' '${ARGS}'" ${VM} && vagrant halt ${VM}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/_build/buildboxes/${BOX})
endmacro(add_vagrant_build_target)

add_vagrant_build_target(
        "build-deb-package-on-u1404_amd64"
        "u1404_amd64"
        "amd64"
        "package"
        "-DCMAKE_INSTALL_PREFIX=/usr -DEXTRA_DATA_DIR=/usr/share/ja2 -DLOCAL_BOOST_LIB=ON -DCPACK_GENERATOR=DEB"
)
add_vagrant_build_target(
        "build-deb-package-on-u1404_i386"
        "u1404_i386"
        "i386"
        "package"
        "-DCMAKE_INSTALL_PREFIX=/usr -DEXTRA_DATA_DIR=/usr/share/ja2 -DLOCAL_BOOST_LIB=ON -DCPACK_GENERATOR=DEB"
)
add_vagrant_build_target(
        "build-win-release-on-u1404_amd64"
        "u1404_amd64"
        "win"
        "package"
        "-DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake -DCPACK_GENERATOR=\"ZIP;NSIS\""
)
if(APPLE)
    # This can only be executed on macOS since it is not legally allowed to run the VM on any other OS
    add_vagrant_build_target(
            "build-macos-release"
            "macos"
            "macos"
            "package"
            "-DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DCPACK_GENERATOR=\"ZIP\""
    )
endif()
add_vagrant_build_target(
        "check-compilation-on-freebsd10"
        "freebsd-10.2"
        "freebsd"
        "all"
        ""
)
add_vagrant_build_target(
        "check-compilation-on-openbsd59"
        "openbsd-5.9"
        "openbsd"
        "all"
        ""
)


add_custom_target(build-releases)
add_custom_command(
    TARGET build-releases
    COMMAND make build-deb-package-on-u1404_amd64
    COMMAND make build-deb-package-on-u1404_i386
    COMMAND make build-win-release-on-u1404_amd64
    COMMAND make check-compilation-on-freebsd10
    COMMAND make check-compilation-on-openbsd59
)

## Rebuilding contributors.txt

add_custom_templated_target("rebuild-contributors-list")
