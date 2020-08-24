# \file CMakeLists.txt

cmake_minimum_required (VERSION 3.1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

find_program(CCACHE_PROGRAM "ccache")
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(ja2-stracciatella)
set(JA2_BINARY "ja2")
set(LAUNCHER_BINARY "ja2-launcher")
set(CMAKE_CXX_STANDARD 11)

## Semantic Versioning
# @see https://semver.org/
#
# The version must be set to:
#  * "0.<next minor>.0-dev" in the master branch
#  * "0.<minor>.<patch>" in release tags
#  * "0.<minor>.<next patch>-dev" in the "0.<minor>.x" branches
#
# TODO when releasing 1.0.0 we must decide what is a breaking change.
#      The master branch version will be "<major>.<next minor>.0-dev" before a
#      breaking change and "<next major>.0.0-dev" with a breaking change.

include(semver-functions)
semver_set(ja2-stracciatella_VERSION "0.17.1-git")
if (VERSION_TAG)
    semver_set_prerelease(ja2-stracciatella_VERSION_PRERELEASE "-${VERSION_TAG}")
endif()
find_package(Git)
if (GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE failed
        OUTPUT_VARIABLE hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT failed)
        list(APPEND ja2-stracciatella_VERSION_BUILDMETADATA "${hash}")
    endif()
endif()
semver_rebuild(ja2-stracciatella_VERSION)
message(STATUS "ja2-stracciatella version: ${ja2-stracciatella_VERSION}")

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
# @see CARGO_BUILD_TARGET in dependencies/lib-stracciatella/CMakeLists.txt
option(LOCAL_FLTK_LIB "Build with fltk lib" OFF)
option(LOCAL_RAPIDJSON_LIB "Build with local rapidjson lib" ON)
option(LOCAL_GTEST_LIB "Build with local gtest lib" ON)
# @see LOCAL_STRING_THEORY_LIB in dependencies/lib-string_theory/CMakeLists.txt
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)
option(BUILD_LAUNCHER "Build the ja2 launcher application" ON)
option(WITH_EDITOR_SLF "Include the latest free editor.slf" OFF)
set(WITH_CUSTOM_LOCALE "" CACHE STRING "Set a custom locale at the start, leave empty to disable")

if(MSVC)
	foreach(_var
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_MINSIZEREL
		CMAKE_C_FLAGS_RELWITHDEBINFO
	)
		# force /MD runtime (future note: cmake 3.15 has CMAKE_MSVC_RUNTIME_LIBRARY)
		string(REGEX REPLACE "/MDd|/MD|/MTd|/MT|/LDd|/LD" "/MD" _value "${${_var}}")
		if(NOT ${_var} STREQUAL _value)
			set_property(CACHE "${_var}" PROPERTY VALUE "${_value}")
		endif()
	endforeach()
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "ja2")
endif()

## Build

if(MSVC)
	# set the source and execution charset
	add_compile_options("/utf-8")
	# suppress the min and max macro definitions
	add_definitions("-DNOMINMAX")
endif()

if(NOT "${WITH_CUSTOM_LOCALE}" STREQUAL "")
    add_definitions("-DWITH_CUSTOM_LOCALE=\"${WITH_CUSTOM_LOCALE}\"")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

set(GAME_VERSION "v${ja2-stracciatella_VERSION}")

message(STATUS "Setting extra data dir to: ${EXTRA_DATA_DIR}")
add_definitions(-DEXTRA_DATA_DIR="${EXTRA_DATA_DIR}")

message(STATUS "Setting directory for libraries to: ${INSTALL_LIB_DIR}")
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

find_package(SDL2 REQUIRED)

if (NOT LOCAL_RAPIDJSON_LIB)
    find_package(RapidJSON REQUIRED)
else()
    message(STATUS "Compiling with local RapidJSON libraries from ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-rapidjson")
    set(RAPIDJSON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-rapidjson/rapidjson-1.1.0/include")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_ASSERTS)
endif()

if(BUILD_LAUNCHER)
    if (NOT LOCAL_FLTK_LIB)
        find_package(FLTK)
        if(NOT FLTK_FOUND)
            message(STATUS "FLTK not found. Disabling launcher.")
            set(BUILD_LAUNCHER OFF)
        endif()
    else()
        set(FLTK_LIBRARIES "")
        set(FLTK_INCLUDE_DIR "")
        add_subdirectory("dependencies/lib-fltk")
    endif()
endif()
message(STATUS "Fltk Libraries: ${FLTK_LIBRARIES}")

set(JA2_INCLUDES "")
set(JA2_SOURCES "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/externalized")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/game")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/sgp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-stracciatella")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-string_theory")

if(BUILD_LAUNCHER)
    set(LAUNCHER_INCLUDES ${FLTK_INCLUDE_DIR})
    set(LAUNCHER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/sgp/Logger.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/sgp/SGPStrings.cc"
    )
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/launcher")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${JA2_INCLUDES}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker/libsmacker
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
        add_subdirectory("dependencies/lib-gtest")
    endif()
    message(STATUS "GTest Libraries: ${GTEST_LIBRARIES}")

    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions("-DWITH_UNITTESTS")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wno-unused-function")
endif()

# This is used for performant log calls from C++ to Rust that use non-absolute file name
string(LENGTH "${CMAKE_SOURCE_DIR}/src/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

add_executable(${JA2_BINARY} ${JA2_SOURCES})
target_link_libraries(${JA2_BINARY} ${SDL2_LIBRARY} ${GTEST_LIBRARIES} smacker ${STRACCIATELLA_LIBRARIES} string_theory-internal)
add_dependencies(${JA2_BINARY} stracciatella)
set_property(SOURCE ${CMAKE_SOURCE_DIR}/src/game/GameVersion.cc APPEND PROPERTY COMPILE_DEFINITIONS "GAME_VERSION=v${ja2-stracciatella_VERSION}")

if(BUILD_LAUNCHER)
    add_executable(${LAUNCHER_BINARY} ${LAUNCHER_SOURCES})
    target_link_libraries(${LAUNCHER_BINARY} ${FLTK_LIBRARIES} ${STRACCIATELLA_LIBRARIES} string_theory-internal)
    add_dependencies(${LAUNCHER_BINARY} stracciatella)
endif()

macro(copy_assets_dir_to_ja2_binary_after_build DIR)
    add_custom_command(TARGET ${JA2_BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/${DIR} "$<TARGET_FILE_DIR:${JA2_BINARY}>/${DIR}")
endmacro()

copy_assets_dir_to_ja2_binary_after_build("externalized")
copy_assets_dir_to_ja2_binary_after_build("unittests")
copy_assets_dir_to_ja2_binary_after_build("mods")

if(WITH_EDITOR_SLF)
    set(EDITORSLF_FILE "${CMAKE_CURRENT_BINARY_DIR}/editor.slf")
    find_package(EditorSlf REQUIRED)
    add_custom_templated_target(download-editor-slf)
    add_dependencies(${JA2_BINARY} download-editor-slf)
endif()

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

## Auxiliary targets

add_custom_target(run-${JA2_BINARY}
    COMMAND "$<TARGET_FILE:${JA2_BINARY}>"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${JA2_BINARY}>"
    DEPENDS "${JA2_BINARY}"
)

add_custom_target(run-${JA2_BINARY}-editor
    COMMAND "$<TARGET_FILE:${JA2_BINARY}>" -editor
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${JA2_BINARY}>"
    DEPENDS "${JA2_BINARY}"
)

add_custom_target(run-${JA2_BINARY}-unittests
    COMMAND "$<TARGET_FILE:${JA2_BINARY}>" -unittests
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${JA2_BINARY}>"
    DEPENDS "${JA2_BINARY}"
)

if (TARGET "${LAUNCHER_BINARY}")
    add_custom_target(run-${LAUNCHER_BINARY}
        COMMAND "$<TARGET_FILE:${LAUNCHER_BINARY}>"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:${LAUNCHER_BINARY}>"
        DEPENDS "${LAUNCHER_BINARY}"
    )
endif()

## Installing and Packaging

set(CPACK_PACKAGE_VERSION_MAJOR ${ja2-stracciatella_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${ja2-stracciatella_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${ja2-stracciatella_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${ja2-stracciatella_VERSION})

set(CPACK_PACKAGE_CONTACT ${CONTACT})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION ${DESCRIPTION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "JA2 Stracciatella")

set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://ja2-stracciatella.github.io/")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libstdc++6, libgcc1, libc6, libfltk1.3-dev, libfltk-images1.3")

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
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PACKAGE_ARCH "${PACKAGE_ARCH}64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(PACKAGE_ARCH "${PACKAGE_ARCH}32")
    endif()
    # build environment:
    if(MSYS)
        set(PACKAGE_ARCH "${PACKAGE_ARCH}-msys2") # MSYS Makefiles, assuming msys2 shell
    elseif(CMAKE_HOST_UNIX)
        set(PACKAGE_ARCH "${PACKAGE_ARCH}-cross") # cross compiling
    endif()
elseif(MSVC)
    set(PACKAGE_ARCH "win-msvc")
    if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "(Win64|IA64|x64)")
        set(PACKAGE_ARCH "${PACKAGE_ARCH}64")
    else()
        set(PACKAGE_ARCH "${PACKAGE_ARCH}32")
    endif()
    set(PACKAGE_ARCH "${PACKAGE_ARCH}-${MSVC_TOOLSET_VERSION}")
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
    install(PROGRAMS "${CMAKE_BINARY_DIR}/lib-stracciatella/bin/ja2-resource-pack${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION bin)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION share/ja2)
    if(WITH_EDITOR_SLF)
        install(FILES "${EDITORSLF_FILE}" DESTINATION share/ja2)
    endif()
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
    install(PROGRAMS "${CMAKE_BINARY_DIR}/lib-stracciatella/bin/ja2-resource-pack${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION .)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION .)
    if(WITH_EDITOR_SLF)
        install(FILES "${EDITORSLF_FILE}" DESTINATION .)
    endif()
    install(FILES changes.md DESTINATION .)
endif()

if(MSVC OR MINGW)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    file(GLOB WIN_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-win/*.txt")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.8-mingw/README-SDL.txt DESTINATION .)
    install(FILES ${WIN_DIST_FILES} DESTINATION .)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.ico DESTINATION .)
endif()

if (MSVC)
    install(FILES ${SDL_DLL} DESTINATION .)
endif()

if (MINGW)
    configure_file("${CMAKE_SOURCE_DIR}/cmake/install-dlls-mingw.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/install-dlls-mingw.cmake" @ONLY)
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/install-dlls-mingw.cmake")
endif()

if(APPLE)
    file(GLOB APPLE_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/*.txt")
    install(FILES ${APPLE_DIST_FILES} DESTINATION .)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.8-macos/SDL2.framework DESTINATION .)
endif()

## Uninstall

add_custom_templated_target("uninstall")

## Rebuilding contributors.txt

add_custom_templated_target("rebuild-contributors-list")
