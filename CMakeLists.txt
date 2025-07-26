# \file CMakeLists.txt

cmake_minimum_required (VERSION 3.18.1)
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

# compiler caching
find_program(SCCACHE_PROGRAM "sccache")
set(USE_SCCACHE ON CACHE BOOL "Set to OFF to not cache compiler outputs")
if(SCCACHE_PROGRAM AND USE_SCCACHE)
    message(STATUS "Using sccache found at ${SCCACHE_PROGRAM} for caching build results")
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}" CACHE STRING "" FORCE)
endif()

project(ja2-stracciatella)
set(JA2_BINARY "ja2")
set(LAUNCHER_BINARY "ja2-launcher")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

file(READ "version" BASE_SEMVER_VERSION)
# Trim leading and trailing whitespace / newlines
string(STRIP "${BASE_SEMVER_VERSION}" BASE_SEMVER_VERSION)

include(semver-functions)
semver_set(ja2-stracciatella_VERSION "${BASE_SEMVER_VERSION}-git")
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
option(BUILD_SDL_LIB "Build SDL from source" OFF)
option(LOCAL_GTEST_LIB "Build with local gtest lib" ON)
# @see LOCAL_STRING_THEORY_LIB in dependencies/lib-string_theory/CMakeLists.txt
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_EDITOR_SLF "Include the latest free editor.slf" OFF)
option(WITH_RUST_BINARIES "Include rust binaries in build" ON)
# @see LOCAL_LUA_LIB in dependencies/lib-lua/CMakeLists.txt
# @see LOCAL_SOL_LIB in dependencies/lib-sol2/CMakeLists.txt
# @see LOCAL_MINIAUDIO_LIB in dependencies/lib-miniaudio/CMakeLists.txt
set(WITH_CUSTOM_LOCALE "" CACHE STRING "Set a custom locale at the start, leave empty to disable")
# Options specific to building C++ files can be found in the list file for
# the src directory. Options specific to building the launcher application
# are located in the list file for the launcher directory.

if(MSVC)
    # set the source and execution charset
    add_compile_options("/utf-8")
    # suppress the min and max macro definitions
    add_compile_definitions(NOMINMAX)

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
        # force /Z7 instead of /ZI to include debug information in obj file
        string(REGEX REPLACE "/Zi|/ZI" "/Z7" _value ${${_var}})
		if(NOT ${${_var}} STREQUAL ${_value})
			set_property(CACHE "${_var}" PROPERTY VALUE "${_value}")
        endif()
	endforeach()
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "ja2")
endif()

## Build

# The main target
if(ANDROID)
    add_library(${JA2_BINARY} SHARED)
else()
    add_executable(${JA2_BINARY})
endif()

if(NOT "${WITH_CUSTOM_LOCALE}" STREQUAL "")
    add_definitions("-DWITH_CUSTOM_LOCALE=\"${WITH_CUSTOM_LOCALE}\"")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

set(GAME_VERSION "v${ja2-stracciatella_VERSION}")

message(STATUS "Setting directory for libraries to: ${INSTALL_LIB_DIR}")
add_definitions(-DINSTALL_LIB_DIR="${INSTALL_LIB_DIR}")

if(BUILD_SDL_LIB)
    set(SDL2_LIBRARY "")
    set(SDL2_INCLUDE_DIR "")
    add_subdirectory("dependencies/lib-sdl2")
else()
    if (NOT (LOCAL_SDL_LIB STREQUAL ""))
        message(STATUS "Using local SDL from " "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SDL_LIB}")
        set(ENV{SDL2DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SDL_LIB}")
    endif()
    find_package(SDL2 2.0.5 REQUIRED)
endif()
message(STATUS "SDL2 Libraries: ${SDL2_LIBRARY}; SDL2 Include Dir: ${SDL2_INCLUDE_DIR}")


# Dependencies must be processed first so the src directory can reliably use them.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-magic_enum")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-lua")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-sol2")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-smacker")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-stracciatella")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-string_theory")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-miniaudio")
if (MINGW)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-mingw-std-threads")
endif()

# Prepare compilation of our own C++ source files.
add_subdirectory(src)

if (WITH_UNITTESTS)
    message(STATUS "Compiling with unittests" )

    if (NOT LOCAL_GTEST_LIB)
        find_package(GTest REQUIRED)
    else()
        message(STATUS "Compiling with local GTest libraries from ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-gtest")
        add_subdirectory("dependencies/lib-gtest")
    endif()
    message(STATUS "GTest Libraries: ${GTEST_LIBRARIES}")

    target_include_directories(ja2 PRIVATE ${GTEST_INCLUDE_DIRS})
    target_compile_definitions(ja2 PRIVATE WITH_UNITTESTS)
endif()

target_link_libraries(${JA2_BINARY} PRIVATE
    ${SDL2_LIBRARY}
    ${GTEST_LIBRARIES}
    smacker
    ${STRACCIATELLA_LIBRARIES}
    string_theory-internal
    lua
)
if (MINGW)
    target_link_libraries(${JA2_BINARY} PRIVATE mingw_stdthreads ntdll)
endif()
if (ANDROID)
    target_link_libraries(${JA2_BINARY} PRIVATE android log)
endif()

add_dependencies(${JA2_BINARY} stracciatella)
set_property(SOURCE ${CMAKE_SOURCE_DIR}/src/game/GameVersion.cc APPEND PROPERTY COMPILE_DEFINITIONS "GAME_VERSION=v${ja2-stracciatella_VERSION}")


macro(copy_assets_dir_to_ja2_binary_after_build DIR)
    add_custom_command(TARGET ${JA2_BINARY} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/${DIR} "$<TARGET_FILE_DIR:${JA2_BINARY}>/${DIR}")
endmacro()

copy_assets_dir_to_ja2_binary_after_build("externalized")
copy_assets_dir_to_ja2_binary_after_build("unittests")
copy_assets_dir_to_ja2_binary_after_build("mods")

if(WITH_EDITOR_SLF)
    set(EDITORSLF_FILE "${CMAKE_CURRENT_BINARY_DIR}/externalized/editor.slf")
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
        set(PACKAGE_ARCH "x86-64")
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
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PACKAGE_ARCH "${PACKAGE_ARCH}64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
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

if (UNIX AND NOT MINGW AND NOT APPLE AND NOT ANDROID)
    install(TARGETS ${JA2_BINARY} RUNTIME DESTINATION bin)
    if(BUILD_LAUNCHER)
        install(TARGETS ${LAUNCHER_BINARY} RUNTIME DESTINATION bin)
    endif()
    if(WITH_RUST_BINARIES)
        install(PROGRAMS "${STRACCIATELLA_EXECUTABLES}" DESTINATION bin)
    endif()
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION share/ja2)
    if(WITH_EDITOR_SLF)
        install(FILES "${EDITORSLF_FILE}" DESTINATION share/ja2/externalized)
    endif()
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.svg
        RENAME ja2-stracciatella.svg
        DESTINATION share/icons/hicolor/scalable/apps)
    install(DIRECTORY "assets/distr-files-linux/man6" TYPE MAN)
elseif(ANDROID)
    install(TARGETS ${JA2_BINARY} LIBRARY DESTINATION .)
else()
    install(TARGETS ${JA2_BINARY} RUNTIME DESTINATION .)
    if(BUILD_LAUNCHER)
        install(TARGETS ${LAUNCHER_BINARY} RUNTIME DESTINATION .)
    endif()
    if(WITH_RUST_BINARIES)
        install(PROGRAMS "${STRACCIATELLA_EXECUTABLES}" DESTINATION .)
    endif()
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION .)
    if(WITH_EDITOR_SLF)
        install(FILES "${EDITORSLF_FILE}" DESTINATION ./externalized)
    endif()
    install(FILES changes.md DESTINATION .)
endif()

if(MSVC OR MINGW)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    file(GLOB WIN_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-win/*.txt")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.20-mingw/README-SDL.txt DESTINATION .)
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
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.20-macos/SDL2.framework DESTINATION .)
endif()

## Build AppImage

add_custom_target(package-appimage
    COMMAND rm -rf *.AppImage
    COMMAND linuxdeploy --appdir AppDir --output appimage
    COMMAND mv *.AppImage ${PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_ARCH}.AppImage
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS "${JA2_BINARY}"
)

## Uninstall

add_custom_templated_target("uninstall")

## Rebuilding contributors.txt

add_custom_templated_target("rebuild-contributors-list")
