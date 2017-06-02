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

project(ja2-stracciatella)
set(BINARY "ja2")
set(CMAKE_CXX_STANDARD 98)

## Versioning

set(ja2-stracciatella_VERSION_MAJOR "0")
set(ja2-stracciatella_VERSION_MINOR "15")
set(ja2-stracciatella_VERSION_PATCH "x")
set(ja2-stracciatella_VERSION "${ja2-stracciatella_VERSION_MAJOR}.${ja2-stracciatella_VERSION_MINOR}.${ja2-stracciatella_VERSION_PATCH}")

## Meta Information
set(CONTACT "JA2 Stracciatella Team <no-email@ja2-stracciatella.github.io>")
set(DESCRIPTION "An improved, cross-platform, stable Jagged Alliance 2 runtime.")

## Options
# Default CMAKE_BUILD_TYPE to RelWithDebInfo
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug RelWithDebInfo." FORCE)
endif()
set(VERSION_TAG "" CACHE STRING "Build date for nightly versioning")
set(EXTRA_DATA_DIR "" CACHE STRING "Directory for externalized data")
set(LOCAL_SDL_LIB "" CACHE STRING "Use local SDL library from this directory")
set(LIBSTRACCIATELLA_TARGET "" CACHE STRING "Rust target architecture for libstracciatella")
option(LOCAL_BOOST_LIB "Build with local boost lib" OFF)
option(LOCAL_RAPIDJSON_LIB "Build with local rapidjson lib" ON)
option(LOCAL_GTEST_LIB "Build with local gtest lib" ON)
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)

## Build

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

set(GAME_VERSION "v${ja2-stracciatella_VERSION}")
if (NOT (VERSION_TAG STREQUAL ""))
    set(GAME_VERSION "${GAME_VERSION}-${VERSION_TAG}")
endif()

add_definitions(-DGAME_VERSION="\\"${GAME_VERSION}\\"")

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
    message(STATUS "Compiling with local Boost libraries from ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-boost")
    add_subdirectory("dependencies/lib-boost")
    set(Boost_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-boost")
    set(Boost_LIBRARIES "boost")
endif()

if (NOT LOCAL_RAPIDJSON_LIB)
    find_package(RapidJSON REQUIRED)
else()
    message(STATUS "Compiling with local RapidJSON libraries from ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-rapidjson")
    set(RAPIDJSON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-rapidjson/rapidjson-1.1.0/include")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_ASSERTS)
endif()

set(JA2_INCLUDES "")
set(JA2_SOURCES "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/externalized")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/game")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/sgp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/slog")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/rust")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${JA2_INCLUDES}
    ${Boost_INCLUDE_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker/libsmacker
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-utf8cpp/source
)

foreach(FILE ${JA2_SOURCES})
  get_filename_component(PARENT_DIR "${FILE}" PATH)
  file(RELATIVE_PATH GROUP "${CMAKE_CURRENT_SOURCE_DIR}" "${PARENT_DIR}")
  string(REPLACE "/" "\\" GROUP "${GROUP}")
  source_group("${GROUP}" FILES "${FILE}")
endforeach()

if (WITH_UNITTESTS)
    message(STATUS "Compiling with unittests" )

    if (NOT LOCAL_GTEST_LIB)
        find_package(GTest REQUIRED)
    else()
        message(STATUS "Compiling with local GTest libraries from ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-gtest")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-gtest")
        set(GTEST_LIBRARIES "gtest")
        set(GTEST_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-gtest/include")
    endif()

    set(lib-gtest "${GTEST_LIBRARIES}")
    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions("-DWITH_UNITTESTS")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
add_executable(${BINARY} ${JA2_SOURCES})
target_link_libraries(${BINARY} ${SDL2_LIBRARY} ${Boost_LIBRARIES} ${lib-gtest} slog smacker rust-stracciatella)

macro(copy_assets_dir_to_ja2_binary_after_build DIR)
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/${DIR} "$<TARGET_FILE_DIR:${BINARY}>/${DIR}")
endmacro()

copy_assets_dir_to_ja2_binary_after_build("externalized")
copy_assets_dir_to_ja2_binary_after_build("unittests")
copy_assets_dir_to_ja2_binary_after_build("mods")

get_property(STRACCIATELLA_SHARED_LIB TARGET rust-stracciatella PROPERTY IMPORTED_LOCATION)

if (MSVC OR APPLE)
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${STRACCIATELLA_SHARED_LIB} "$<TARGET_FILE_DIR:${BINARY}>")
endif()
if (MSVC)
    string(REPLACE "SDL2main.lib" "SDL2.dll" SDL_DLL ${SDL2MAIN_LIBRARY})
    add_custom_command(TARGET ${BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SDL_DLL} "$<TARGET_FILE_DIR:${BINARY}>")
endif()
if (APPLE)
    add_custom_command(TARGET ${BINARY}
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path"
            "$<TARGET_FILE:${BINARY}>")
endif()


## Installing and Packaging

set(CPACK_PACKAGE_VERSION_MAJOR ${ja2-stracciatella_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${ja2-stracciatella_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${ja2-stracciatella_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${ja2-stracciatella_VERSION})
if (NOT (VERSION_TAG STREQUAL ""))
    set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${VERSION_TAG}")
endif()

set(CPACK_PACKAGE_CONTACT ${CONTACT})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION ${DESCRIPTION})

set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://ja2-stracciatella.github.io/")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libstdc++6, libgcc1, libc6")

if(UNIX AND NOT MINGW AND NOT APPLE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PACKAGE_ARCH "amd64")
    else()
        set(PACKAGE_ARCH "i386")
    endif()
elseif(MINGW)
    set(PACKAGE_ARCH "win-mingw")
elseif(MSVC)
    set(PACKAGE_ARCH "win-msvc")
    if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
        set(PACKAGE_ARCH "${PACKAGE_ARCH}64")
    else()
        set(PACKAGE_ARCH "${PACKAGE_ARCH}32")
    endif()
elseif(APPLE)
    set(PACKAGE_ARCH "macos")
else()
    set(PACKAGE_ARCH "unknown")
endif()
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_ARCH}")

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif()
include(InstallRequiredSystemLibraries)
include(CPack)

if (UNIX AND NOT MINGW AND NOT APPLE)
    install(TARGETS ${BINARY} RUNTIME DESTINATION bin)
    install(DIRECTORY assets/externalized assets/mods assets/unittests DESTINATION share/ja2)
    install(FILES ${STRACCIATELLA_SHARED_LIB} DESTINATION lib)
    install(FILES assets/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
    install(
        FILES assets/icons/logo.svg
        RENAME ja2-stracciatella.svg
        DESTINATION share/icons/hicolor/scalable/apps)
    if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD|DragonFly|OpenBSD")
        install(FILES ja2_manpage DESTINATION man/man6 RENAME ja2.6)
    else()
        install(FILES ja2_manpage DESTINATION share/man/man6 RENAME ja2.6)
    endif()
else()
    install(TARGETS ${BINARY} RUNTIME DESTINATION .)
    install(FILES ${STRACCIATELLA_SHARED_LIB} DESTINATION .)
    install(DIRECTORY assets/externalized assets/mods assets/unittests DESTINATION .)
    install(FILES changes.md DESTINATION .)
endif()

if (MINGW)
    file(GLOB WIN_MINGW_DIST_DLLS "assets/distr-files-win-mingw/*.dll")
    install(FILES dependencies/lib-SDL2-2.0.4-mingw/i686-w64-mingw32/bin/SDL2.dll DESTINATION .)
    install(FILES ${WIN_MINGW_DIST_DLLS} DESTINATION .)
endif()

if (MSVC)
    install(FILES ${SDL_DLL} DESTINATION .)
    install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION . COMPONENT Libraries)
endif()

if(MSVC OR MINGW)
    file(GLOB WIN_DIST_FILES "assets/distr-files-win/*")
    install(FILES ${WIN_DIST_FILES} DESTINATION .)
    install(FILES assets/icons/logo.ico DESTINATION .)
endif()

if(APPLE)
    file(GLOB APPLE_DIST_FILES "assets/distr-files-mac/*")
    install(FILES ${APPLE_DIST_FILES} DESTINATION .)
    install(DIRECTORY dependencies/lib-SDL2-2.0.4-macos/SDL2.framework DESTINATION .)
    install(FILES assets/icons/logo.icns DESTINATION .)
endif()

add_custom_templated_target("uninstall")

## Release building

macro(add_vagrant_build_target NAME BOX TARGET ARGS)
    add_custom_target(${NAME})
    add_custom_command(TARGET ${NAME}
            COMMAND vagrant up && vagrant ssh -c "sh /tmp/ja2-stracciatella/release/common/build.sh '${TARGET}' '${ARGS}'" && vagrant halt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/release/${BOX})
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
        "-DCMAKE_C_COMPILER=egcc -DCMAKE_CXX_COMPILER=eg++"
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
