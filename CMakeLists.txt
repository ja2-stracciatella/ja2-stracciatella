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
set(JA2_BINARY "ja2")
set(LAUNCHER_BINARY "ja2-launcher")
set(CMAKE_CXX_STANDARD 11)

## Versioning

set(ja2-stracciatella_VERSION_MAJOR "0")
set(ja2-stracciatella_VERSION_MINOR "16")
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
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(LOCAL_SDL_LIB "" CACHE STRING "Use local SDL library from this directory")
set(LIBSTRACCIATELLA_TARGET "" CACHE STRING "Rust target architecture for libstracciatella")
option(LOCAL_FLTK_LIB "Build with fltk lib" OFF)
option(LOCAL_BOOST_LIB "Build with local boost lib" OFF)
option(LOCAL_RAPIDJSON_LIB "Build with local rapidjson lib" ON)
option(LOCAL_GTEST_LIB "Build with local gtest lib" ON)
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)
option(BUILD_LAUNCHER "Build the ja2 launcher application" ON)

## Build

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

set(GAME_VERSION "v${ja2-stracciatella_VERSION}")
if (NOT (VERSION_TAG STREQUAL ""))
    set(GAME_VERSION "${GAME_VERSION}-${VERSION_TAG}")
endif()

add_definitions(-DGAME_VERSION="\\"${GAME_VERSION}\\"")

message(STATUS "Setting extra data dir to" "${EXTRA_DATA_DIR}")
add_definitions(-DEXTRA_DATA_DIR="${EXTRA_DATA_DIR}")

message(STATUS "Setting directory for libraries to" "${INSTALL_LIB_DIR}")
add_definitions(-DINSTALL_LIB_DIR="${INSTALL_LIB_DIR}")

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

set(FLTK_LIBRARIES "")
set(FLTK_INCLUDE_DIR "")
if(BUILD_LAUNCHER)
    if (NOT LOCAL_FLTK_LIB)
        find_package(FLTK)
        if(NOT FLTK_FOUND)
            message(STATUS "FLTK not found. Disabling launcher.")
            set(BUILD_LAUNCHER OFF)
        endif()
    else()
        add_subdirectory("dependencies/lib-fltk")
        set(FLTK_INCLUDE_DIR "${CMAKE_BINARY_DIR}/lib-fltk/include")

        if (UNIX AND NOT MINGW AND NOT APPLE)
            set(FLTK_LIBRARIES
                "${CMAKE_BINARY_DIR}/lib-fltk/lib64/libfltk.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib64/libfltk_images.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib64/libfltk_forms.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib64/libfltk_png.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib64/libfltk_z.a"
                X11)
        elseif(APPLE)
            set(FLTK_LIBRARIES
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/libfltk.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/libfltk_images.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/libfltk_forms.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/libfltk_png.a"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/libfltk_z.a")
        elseif(MSVC)
            set(MSVC_LIB_SUFFIX "")
            if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(MSVC_LIB_SUFFIX "d")
            endif()
            set(FLTK_LIBRARIES
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/fltk${MSVC_LIB_SUFFIX}.lib"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/fltk_images${MSVC_LIB_SUFFIX}.lib"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/fltk_forms${MSVC_LIB_SUFFIX}.lib"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/fltk_png${MSVC_LIB_SUFFIX}.lib"
                "${CMAKE_BINARY_DIR}/lib-fltk/lib/fltk_z${MSVC_LIB_SUFFIX}.lib")
        endif()
    endif()
endif()
message(STATUS "Fltk Libraries: ${FLTK_LIBRARIES}")

set(JA2_INCLUDES "")
set(JA2_SOURCES "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/externalized")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/game")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/sgp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/slog")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/rust")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker")

if(BUILD_LAUNCHER)
    set(LAUNCHER_INCLUDES ${FLTK_INCLUDE_DIR})
    set(LAUNCHER_SOURCES "")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/launcher")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${JA2_INCLUDES}
    ${Boost_INCLUDE_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker/libsmacker
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-utf8cpp/source
)

if(BUILD_LAUNCHER)
    include_directories(
        ${LAUNCHER_INCLUDES}
    )
endif()

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

set(ADDITIONAL_LIBS "")
if (UNIX AND NOT MINGW)
    set(ADDITIONAL_LIBS dl pthread)
elseif(MSVC)
    set(ADDITIONAL_LIBS ws2_32 shell32 advapi32 userenv)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
add_executable(${JA2_BINARY} ${JA2_SOURCES})
target_link_libraries(${JA2_BINARY} ${SDL2_LIBRARY} ${Boost_LIBRARIES} ${lib-gtest} slog smacker rust-stracciatella ${ADDITIONAL_LIBS})

if(BUILD_LAUNCHER)
    add_executable(${LAUNCHER_BINARY} ${LAUNCHER_SOURCES})
    target_link_libraries(${LAUNCHER_BINARY} ${FLTK_LIBRARIES} slog rust-stracciatella ${ADDITIONAL_LIBS})
    if (LOCAL_FLTK_LIB)
        add_dependencies(${LAUNCHER_BINARY} fltk-internal)
    endif()
endif()

macro(copy_assets_dir_to_ja2_binary_after_build DIR)
    add_custom_command(TARGET ${JA2_BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/${DIR} "$<TARGET_FILE_DIR:${JA2_BINARY}>/${DIR}")
endmacro()

copy_assets_dir_to_ja2_binary_after_build("externalized")
copy_assets_dir_to_ja2_binary_after_build("unittests")
copy_assets_dir_to_ja2_binary_after_build("mods")


if (MSVC)
    string(REPLACE "SDL2main.lib" "SDL2.dll" SDL_DLL ${SDL2MAIN_LIBRARY})
    add_custom_command(TARGET ${JA2_BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SDL_DLL} "$<TARGET_FILE_DIR:${JA2_BINARY}>")
endif()
if (APPLE)
    add_custom_command(TARGET ${JA2_BINARY}
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path"
            "$<TARGET_FILE:${JA2_BINARY}>")
    add_custom_command(TARGET ${LAUNCHER_BINARY}
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path"
            "$<TARGET_FILE:${LAUNCHER_BINARY}>")
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
set(CPACK_PACKAGE_INSTALL_DIRECTORY "JA2 Stracciatella")

set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://ja2-stracciatella.github.io/")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libstdc++6, libgcc1, libc6")

set(CPACK_BUNDLE_NAME "JA2 Stracciatella")
set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.icns")
set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/BundleInfo.plist")
set(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/ja2-startup.sh")

set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortcut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\JA2 Stracciatella.lnk' '$INSTDIR\\\\ja2-launcher.exe' '' '$INSTDIR\\\\logo.ico'")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "ja2-launcher.exe")
set(CPACK_NSIS_DISPLAY_NAME "JA2 Stracciatella")
set(CPACK_NSIS_PACKAGE_NAME "JA2 Stracciatella")
set(CPACK_NSIS_URL_INFO_ABOUT "https://ja2-stracciatella.github.io/")

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

include(CPack)

if (UNIX AND NOT MINGW AND NOT APPLE)
    install(TARGETS ${JA2_BINARY} RUNTIME DESTINATION bin)
    if(BUILD_LAUNCHER)
        install(TARGETS ${LAUNCHER_BINARY} RUNTIME DESTINATION bin)
    endif()
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION share/ja2)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.svg
        RENAME ja2-stracciatella.svg
        DESTINATION share/icons/hicolor/scalable/apps)
    if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD|DragonFly|OpenBSD")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2_manpage DESTINATION man/man6 RENAME ja2.6)
    else()
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2_manpage DESTINATION share/man/man6 RENAME ja2.6)
    endif()
else()
    install(TARGETS ${JA2_BINARY} RUNTIME DESTINATION .)
    if(BUILD_LAUNCHER)
        install(TARGETS ${LAUNCHER_BINARY} RUNTIME DESTINATION .)
    endif()
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION .)
    install(FILES changes.md DESTINATION .)
endif()

if(MSVC OR MINGW)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    file(GLOB WIN_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-win/*.txt")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.4-mingw/README-SDL.txt DESTINATION .)
    install(FILES ${WIN_DIST_FILES} DESTINATION .)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.ico DESTINATION .)
endif()

if (MSVC)
    install(FILES ${SDL_DLL} DESTINATION .)
endif()

if (MINGW)
    file(GLOB WIN_MINGW_DIST_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-win-mingw/*.dll")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.4-mingw/i686-w64-mingw32/bin/SDL2.dll DESTINATION .)
    install(FILES ${WIN_MINGW_DIST_DLLS} DESTINATION .)
endif()

if(APPLE)
    file(GLOB APPLE_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/*.txt")
    install(FILES ${APPLE_DIST_FILES} DESTINATION .)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.4-macos/SDL2.framework DESTINATION .)
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
