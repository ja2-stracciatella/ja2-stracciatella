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
set (CMAKE_CXX_STANDARD 98)

## Versioning

set(ja2-stracciatella_VERSION_MAJOR "0")
set(ja2-stracciatella_VERSION_MINOR "15")
set(ja2-stracciatella_VERSION_PATCH "x")
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

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_definitions(-DJA2)
add_definitions(-DGAME_VERSION="\\"${GAME_VERSION}\\"")
add_definitions(-DMICROINI_STATIC)

message(STATUS "Setting extra data dir to" "${EXTRA_DATA_DIR}")
add_definitions(-DEXTRA_DATA_DIR="${EXTRA_DATA_DIR}")

if (WITH_FIXMES)
    message(STATUS "Building with fixme messages" )
    add_definitions(-DWITH_FIXMES)
endif()

if (WITH_MAEMO)
    message(STATUS "Building with right click mapped to F4 (menu button)" )
    add_definitions(-DWITH_MAEMO)
endif()

if (WITH_SOUND_DEBUG)
    message(STATUS "Building with sound debug" )
    add_definitions(-DWITH_SOUND_DEBUG)
endif()

if (NOT (LOCAL_SDL_LIB STREQUAL ""))
    message(STATUS "Using local SDL from " "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SDL_LIB}")
    set(ENV{SDL2DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SDL_LIB}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(SDL2 REQUIRED)

if (NOT LOCAL_BOOST_LIB)
    find_package(Boost REQUIRED COMPONENTS filesystem system)
else()
    message(STATUS "Compiling with local Boost libraries from ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-boost" )
    add_subdirectory("_build/lib-boost")
    set(Boost_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-boost")
    set(Boost_LIBRARIES "boost")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_ASSERTS)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-rapidjson/rapidjson-1.1.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-slog
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-smacker/libsmacker
    ${CMAKE_CURRENT_SOURCE_DIR}/_build/lib-utf8cpp/source
    ${SDL2_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

foreach(FILE ${JA2_SOURCES})
  get_filename_component(PARENT_DIR "${FILE}" PATH)
  file(RELATIVE_PATH GROUP "${CMAKE_CURRENT_SOURCE_DIR}" "${PARENT_DIR}")
  string(REPLACE "/" "\\" GROUP "${GROUP}")
  source_group("${GROUP}" FILES "${FILE}")
endforeach()

if (WITH_UNITTESTS)
    message(STATUS "Compiling with unittests" )

    find_package(GTest REQUIRED)
    set(lib-gtest ${GTEST_LIBRARIES})
    add_definitions(-DWITH_UNITTESTS)

    include_directories(${GEST_INCLUDE_DIRS})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
add_executable(${BINARY} ${JA2_SOURCES})
target_link_libraries(${BINARY} ${SDL2_LIBRARY} ${Boost_LIBRARIES} ${lib-gtest} MicroIni slog smacker)

macro(copy_dir_to_ja2_binary_after_build DIR)
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/${DIR} "$<TARGET_FILE_DIR:${BINARY}>/${DIR}")
endmacro()

copy_dir_to_ja2_binary_after_build("externalized")
copy_dir_to_ja2_binary_after_build("_unittests")
copy_dir_to_ja2_binary_after_build("mods")

if (MSVC)
    string(REPLACE "SDL2main.lib" "SDL2.dll" SDL_DLL ${SDL2MAIN_LIBRARY})
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SDL_DLL} "$<TARGET_FILE_DIR:${BINARY}>")
endif()

## Installing and Packaging

set(CPACK_PACKAGE_VERSION_MAJOR ${ja2-stracciatella_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${ja2-stracciatella_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${ja2-stracciatella_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${ja2-stracciatella_VERSION})
set(CPACK_PACKAGE_CONTACT ${CONTACT})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION ${DESCRIPTION})

set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://ja2-stracciatella.github.io/")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2, libstdc++6, libgcc1, libc6")

if(UNIX AND NOT MINGW AND NOT APPLE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PACKAGE_ARCH "amd64")
    else()
        set(PACKAGE_ARCH "i386")
    endif()
elseif(MINGW)
    set(PACKAGE_ARCH "win-mingw")
elseif(APPLE)
    set(PACKAGE_ARCH "macos")
else()
    set(PACKAGE_ARCH "unknown")
endif()
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_ARCH}")

include(CPack)

if (UNIX AND NOT MINGW AND NOT APPLE)
    install(TARGETS ${BINARY} RUNTIME DESTINATION bin)
    install(DIRECTORY externalized mods _unittests DESTINATION share/ja2)
    install(FILES _build/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
    install(
        FILES _build/icons/logo.svg
        RENAME ja2-stracciatella.svg
        DESTINATION share/icons/hicolor/scalable/apps)
    if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD|DragonFly|OpenBSD")
        install(FILES ja2_manpage DESTINATION man/man6 RENAME ja2.6)
    else()
        install(FILES ja2_manpage DESTINATION share/man/man6 RENAME ja2.6)
    endif()
else()
    install(TARGETS ${BINARY} RUNTIME DESTINATION .)
    install(DIRECTORY externalized mods _unittests DESTINATION .)
    install(FILES changes.md DESTINATION .)
endif()

if (MINGW)
    file(GLOB WIN_MINGW_DIST_DLLS "_build/distr-files-win-mingw/*.dll")
    install(FILES ${WIN_MINGW_DIST_DLLS} DESTINATION .)
endif()

if(WIN32 OR MINGW)
    file(GLOB WIN_DIST_FILES "_build/distr-files-win/*")
    install(FILES ${WIN_DIST_FILES} DESTINATION .)
    install(FILES _build/icons/logo.ico DESTINATION .)
    install(FILES _build/lib-SDL2-2.0.4-mingw/i686-w64-mingw32/bin/SDL2.dll DESTINATION .)
endif()

if(APPLE)
    file(GLOB APPLE_DIST_FILES "_build/distr-files-mac/*")
    install(FILES ${APPLE_DIST_FILES} DESTINATION .)
    install(DIRECTORY _build/lib-SDL2-2.0.4-macos/SDL2.framework DESTINATION .)
    install(FILES _build/icons/logo.icns DESTINATION .)
endif()

add_custom_templated_target("uninstall")

## Release building

macro(add_vagrant_build_target NAME BOX TARGET ARGS)
    add_custom_target(${NAME})
    add_custom_command(TARGET ${NAME}
            COMMAND vagrant up && vagrant ssh -c "sh /tmp/ja2-stracciatella/_build/buildboxes/common/build.sh '${TARGET}' '${ARGS}'" && vagrant halt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/_build/buildboxes/${BOX})
endmacro(add_vagrant_build_target)

add_vagrant_build_target(
        "build-deb-package-on-u1404_amd64"
        "u1404_amd64"
        "package"
        "-DCMAKE_INSTALL_PREFIX=/usr -DEXTRA_DATA_DIR=/usr/share/ja2 -DLOCAL_BOOST_LIB=ON -DCPACK_GENERATOR=DEB"
)
add_vagrant_build_target(
        "build-deb-package-on-u1404_i386"
        "u1404_i386"
        "package"
        "-DCMAKE_INSTALL_PREFIX=/usr -DEXTRA_DATA_DIR=/usr/share/ja2 -DLOCAL_BOOST_LIB=ON -DCPACK_GENERATOR=DEB"
)
add_vagrant_build_target(
        "build-win-release-on-u1404_amd64"
        "u1404_amd64"
        "package"
        "-DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake -DCPACK_GENERATOR=\"ZIP;NSIS\""
)
if(APPLE)
    # This can only be executed on macOS since it is not legally allowed to run the VM on any other OS
    add_vagrant_build_target(
            "build-macos-release"
            "macos"
            "package"
            "-DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DCPACK_GENERATOR=\"ZIP\""
    )
endif()
add_vagrant_build_target(
        "check-compilation-on-freebsd10"
        "freebsd-10.2"
        "all"
        ""
)
add_vagrant_build_target(
        "check-compilation-on-openbsd59"
        "openbsd-5.9"
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
