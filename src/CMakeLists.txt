cmake_minimum_required(VERSION 3.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(JA2_BINARY "ja2")
set(LAUNCHER_BINARY "ja2-launcher")

## Build Options

set(CMAKE_CXX_STANDARD 11)

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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_ASSERTS)
endif()

## Dependencies

set(CMAKE_FIND_ROOT_PATH ${DEPENDENCIES_INSTALL_ROOT})

set(ADDITIONAL_LIBS "")
if (UNIX AND NOT MINGW)
    set(ADDITIONAL_LIBS dl)
endif()

find_package(SDL2 REQUIRED)
message(STATUS "SDL2 Libraries: ${SDL2_LIBRARY} SDL2 Includes: ${SDL2_INCLUDE_DIR}")

if (LOCAL_BOOST_LIB)
    find_package(
        LocalBoost
        REQUIRED
    )
else()
    find_package(
        Boost
        REQUIRED
        COMPONENTS filesystem system
    )
endif()
message(STATUS "Boost Libraries: ${Boost_LIBRARIES} Boost Includes: ${Boost_INCLUDE_DIRS}")

find_package(RapidJSON REQUIRED)
message(STATUS "RapidJSON Includes: ${RAPIDJSON_INCLUDE_DIRS}")

find_package(Utf8Cpp REQUIRED)
message(STATUS "Utf8Cpp Includes: ${UTF8CPP_INCLUDE_DIRS}")

find_package(LocalSmacker REQUIRED)
message(STATUS "Smacker Libraries: ${SMACKER_LIBRARIES} Smacker Includes: ${SMACKER_INCLUDE_DIRS}")

if (WITH_UNITTESTS)
    find_package(GTest REQUIRED)
    message(STATUS "GTest Libraries: ${GTEST_LIBRARIES} GTest Includes: ${GTEST_INCLUDE_DIRS}")
endif()

if(BUILD_LAUNCHER)
    if (LOCAL_FLTK_LIB)
        include(${DEPENDENCIES_INSTALL_ROOT}/share/fltk/FLTKConfig.cmake)
        set(FLTK_LIBRARIES fltk fltk_images fltk_png)
        message(STATUS "Using local fltk dependency")
    else()
        find_package(FLTK REQUIRED)
        if (UNIX AND NOT MINGW)
            set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} pthread)
        endif()
        message(STATUS "FLTK Libraries: ${FLTK_LIBRARIES} FLTK Includes: ${FLTK_INCLUDE_DIR}")
    endif()
endif()

find_package(Stracciatella REQUIRED)
message(STATUS "Libstracciatella: ${STRACCIATELLA_LIBRARY}")

set(JA2_INCLUDES "")
set(JA2_SOURCES "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/externalized")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/game")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/sgp")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/slog")

if(BUILD_LAUNCHER)
    set(LAUNCHER_INCLUDES ${FLTK_INCLUDE_DIR})
    set(LAUNCHER_SOURCES "")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/launcher")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JA2_INCLUDES}
    ${Boost_INCLUDE_DIRS}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${SMACKER_INCLUDE_DIRS}
    ${UTF8CPP_INCLUDE_DIRS}
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
    add_definitions("-DWITH_UNITTESTS")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
add_executable(${JA2_BINARY} ${JA2_SOURCES})
target_link_libraries(${JA2_BINARY}
    ${SDL2_LIBRARY}
    ${Boost_LIBRARIES}
    ${lib-gtest}
    slog
    ${SMACKER_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${STRACCIATELLA_LIBRARY}
    ${ADDITIONAL_LIBS}
)

if(BUILD_LAUNCHER)
    add_executable(${LAUNCHER_BINARY} ${LAUNCHER_SOURCES})

    target_link_libraries(${LAUNCHER_BINARY}
        slog
        ${STRACCIATELLA_LIBRARY}
        ${ADDITIONAL_LIBS}
        ${FLTK_LIBRARIES}
    )
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

install(TARGETS ja2 ja2-launcher DESTINATION .)
