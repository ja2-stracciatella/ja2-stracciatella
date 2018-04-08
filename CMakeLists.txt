cmake_minimum_required (VERSION 2.8)

include(ExternalProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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
set(LIBSTRACCIATELLA_TARGET "" CACHE STRING "Rust target architecture for libstracciatella")
option(LOCAL_LIBS_ONLY "Build only local libs" OFF)
option(LOCAL_SDL_LIB "Build with local sdl2 lib" ${LOCAL_LIBS_ONLY})
option(LOCAL_FLTK_LIB "Build with local fltk lib" ${LOCAL_LIBS_ONLY})
option(LOCAL_BOOST_LIB "Build with local boost lib" ${LOCAL_LIBS_ONLY})
option(LOCAL_RAPIDJSON_LIB "Build with local rapidjson lib" ON)
option(LOCAL_GTEST_LIB "Build with local gtest lib" ON)
option(WITH_UNITTESTS "Build with unittests" ON)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)
option(BUILD_LAUNCHER "Build the ja2 launcher application" ON)

# Install options
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")

set(STRACCIATELLA_CACHE_VARIABLES
    -DVERSION_MAJOR=${ja2-stracciatella_VERSION_MAJOR}
    -DVERSION_MINOR=${ja2-stracciatella_VERSION_MINOR}
    -DVERSION_PATCH=${ja2-stracciatella_VERSION_PATCH}
    -DVERSION_TAG=
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DEXTRA_DATA_DIR=${EXTRA_DATA_DIR}
    -DLIBSTRACCIATELLA_TARGET=${LIBSTRACCIATELLA_TARGET}
    -DLOCAL_SDL_LIB=${LOCAL_SDL_LIB}
    -DLOCAL_FLTK_LIB=${LOCAL_FLTK_LIB}
    -DLOCAL_BOOST_LIB=${LOCAL_BOOST_LIB}
    -DLOCAL_RAPIDJSON_LIB=${LOCAL_RAPIDJSON_LIB}
    -DLOCAL_GTEST_LIB=${LOCAL_GTEST_LIB}
    -DWITH_UNITTESTS=${WITH_UNITTESTS}
    -DWITH_FIXMES=${WITH_FIXMES}
    -DWITH_MAEMO=${WITH_MAEMO}
    -DBUILD_LAUNCHER=${BUILD_LAUNCHER}
)

## Build

# Setup external projects for internal dependencies
set(ALL_DEPENDS libstracciatella stracciatella-assets)
set(DEPENDENCIES_INSTALL_ROOT "${CMAKE_BINARY_DIR}/dependencies-root")
add_subdirectory("dependencies")

externalproject_add(libstracciatella
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rust"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
        ${STRACCIATELLA_CACHE_VARIABLES}
    BUILD_ALWAYS ON
)

# Target to run Rust unit tests
find_package(Cargo 0.9.0 REQUIRED)
find_package(Rustc 1.15.0 REQUIRED)
add_custom_target(rust-unit-tests
    COMMAND ${CMAKE_COMMAND} -E env ${CARGO_EXECUTABLE} test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust
)

# External project to build assets
externalproject_add(stracciatella-assets
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
    BUILD_ALWAYS ON
)

# External Project for game and launcher binaries
externalproject_add(ja2
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
        -DDEPENDENCIES_INSTALL_ROOT=${DEPENDENCIES_INSTALL_ROOT}
        ${STRACCIATELLA_CACHE_VARIABLES}
    BUILD_ALWAYS ON
    DEPENDS ${ALL_DEPENDS}
)

# ## Installing and Packaging

# set(CPACK_PACKAGE_VERSION_MAJOR ${ja2-stracciatella_VERSION_MAJOR})
# set(CPACK_PACKAGE_VERSION_MINOR ${ja2-stracciatella_VERSION_MINOR})
# set(CPACK_PACKAGE_VERSION_PATCH ${ja2-stracciatella_VERSION_PATCH})
# set(CPACK_PACKAGE_VERSION ${ja2-stracciatella_VERSION})
# if (NOT (VERSION_TAG STREQUAL ""))
#     set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${VERSION_TAG}")
# endif()

# set(CPACK_PACKAGE_CONTACT ${CONTACT})
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DESCRIPTION})
# set(CPACK_PACKAGE_DESCRIPTION ${DESCRIPTION})
# set(CPACK_PACKAGE_INSTALL_DIRECTORY "JA2 Stracciatella")

# set(CPACK_DEBIAN_PACKAGE_SECTION "games")
# set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://ja2-stracciatella.github.io/")
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0, libstdc++6, libgcc1, libc6")

# set(CPACK_BUNDLE_NAME "JA2 Stracciatella")
# set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.icns")
# set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/BundleInfo.plist")
# set(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/ja2-startup.sh")

# set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortcut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\JA2 Stracciatella.lnk' '$INSTDIR\\\\ja2-launcher.exe' '' '$INSTDIR\\\\logo.ico'")
# set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
# set(CPACK_NSIS_MUI_FINISHPAGE_RUN "ja2-launcher.exe")
# set(CPACK_NSIS_DISPLAY_NAME "JA2 Stracciatella")
# set(CPACK_NSIS_PACKAGE_NAME "JA2 Stracciatella")
# set(CPACK_NSIS_URL_INFO_ABOUT "https://ja2-stracciatella.github.io/")

# if(UNIX AND NOT MINGW AND NOT APPLE)
#     if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#         set(PACKAGE_ARCH "amd64")
#     else()
#         set(PACKAGE_ARCH "i386")
#     endif()
# elseif(MINGW)
#     set(PACKAGE_ARCH "win-mingw")
# elseif(MSVC)
#     set(PACKAGE_ARCH "win-msvc")
#     if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
#         set(PACKAGE_ARCH "${PACKAGE_ARCH}64")
#     else()
#         set(PACKAGE_ARCH "${PACKAGE_ARCH}32")
#     endif()
# elseif(APPLE)
#     set(PACKAGE_ARCH "macos")
# else()
#     set(PACKAGE_ARCH "unknown")
# endif()
# set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_ARCH}")

# include(CPack)

# if (UNIX AND NOT MINGW AND NOT APPLE)
#     install(TARGETS ${JA2_BINARY} RUNTIME DESTINATION bin)
#     if(BUILD_LAUNCHER)
#         install(TARGETS ${LAUNCHER_BINARY} RUNTIME DESTINATION bin)
#     endif()
#     install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION share/ja2)
#     install(FILES ${STRACCIATELLA_SHARED_LIB} DESTINATION ${INSTALL_LIB_DIR})
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2-stracciatella.desktop DESTINATION share/applications)
#     install(
#         FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.svg
#         RENAME ja2-stracciatella.svg
#         DESTINATION share/icons/hicolor/scalable/apps)
#     if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD|DragonFly|OpenBSD")
#         install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2_manpage DESTINATION man/man6 RENAME ja2.6)
#     else()
#         install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-linux/ja2_manpage DESTINATION share/man/man6 RENAME ja2.6)
#     endif()
# else()
#     install(TARGETS ${JA2_BINARY} RUNTIME DESTINATION .)
#     if(BUILD_LAUNCHER)
#         install(TARGETS ${LAUNCHER_BINARY} RUNTIME DESTINATION .)
#     endif()
#     install(FILES ${STRACCIATELLA_SHARED_LIB} DESTINATION .)
#     install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/externalized assets/mods assets/unittests DESTINATION .)
#     install(FILES changes.md DESTINATION .)
# endif()

# if(MSVC OR MINGW)
#     set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
#     set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
#     set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
#     include(InstallRequiredSystemLibraries)

#     file(GLOB WIN_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-win/*.txt")
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.4-mingw/README-SDL.txt DESTINATION .)
#     install(FILES ${WIN_DIST_FILES} DESTINATION .)
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/logo.ico DESTINATION .)
# endif()

# if (MSVC)
#     install(FILES ${SDL_DLL} DESTINATION .)
# endif()

# if (MINGW)
#     file(GLOB WIN_MINGW_DIST_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-win-mingw/*.dll")
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.4-mingw/i686-w64-mingw32/bin/SDL2.dll DESTINATION .)
#     install(FILES ${WIN_MINGW_DIST_DLLS} DESTINATION .)
# endif()

# if(APPLE)
#     file(GLOB APPLE_DIST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/distr-files-mac/*.txt")
#     install(FILES ${APPLE_DIST_FILES} DESTINATION .)
#     install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib-SDL2-2.0.4-macos/SDL2.framework DESTINATION .)
# endif()

# ## Uninstalling

# add_custom_templated_target("uninstall")

# ## Rebuilding contributors.txt

# add_custom_templated_target("rebuild-contributors-list")
