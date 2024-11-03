message(STATUS "Entering src directory")

option(ENABLE_ASSERTS "Assertion failures terminate the program (always enabled in Debug)" OFF)
option(ENABLE_PCH "Use precompiled standard library headers during compilation" OFF)
option(WITH_ASAN "Build with address sanitizer" OFF)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)
option(WITH_MAGICENUM "Build with magic_enum library" ON)

if(MSVC)
	# set the source and execution charset
	add_compile_options("/utf-8")
	# suppress the min and max macro definitions
	target_compile_definitions(ja2 PRIVATE NOMINMAX)
endif()

function(add_warnings target)
    if(NOT MSVC)
        target_compile_options(${target} PRIVATE -Wall -Wtype-limits -Wignored-qualifiers)
    else()
        target_compile_options(${target} PRIVATE /W3 /wd4018 /wd4101 /wd4244 /wd4996)
        # 4018 : example: signed/unsigned mismatch
        # 4101 : example: unreferenced local variable
        # 4200 : example: nonstandard extension used : zero-sized array in struct/union
        # 4244 : example: conversion from 'int' to 'real', possible loss of data
        # 4996 : example: This function or variable may be unsafe (sscanf)
    endif()
endfunction()

target_include_directories(${JA2_BINARY} SYSTEM PRIVATE
    ${SDL2_INCLUDE_DIR}
    ${SOL_INCLUDE_DIR}
    ${LUA_INCLUDE_DIRS}
    ${MINIAUDIO_INCLUDE_DIRS}
    ${MAGICENUM_INCLUDE_DIR}
)

add_subdirectory(launcher)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR})
set(JA2_INCLUDES "")
set(JA2_SOURCES "")
add_subdirectory(externalized)
add_subdirectory(game)
add_subdirectory(sgp)

target_include_directories(${JA2_BINARY} PRIVATE ${JA2_INCLUDES})
target_sources(${JA2_BINARY} PRIVATE ${JA2_SOURCES})

set_target_properties(${JA2_BINARY} PROPERTIES
    UNITY_BUILD_MODE "GROUP")

if (WITH_ASAN)
    target_compile_options(${JA2_BINARY} PRIVATE -fsanitize=address)
    target_link_options(${JA2_BINARY} PRIVATE -fsanitize=address)
endif()

if(ENABLE_PCH)
    target_precompile_headers(${JA2_BINARY} PRIVATE
        <algorithm>
        <array>
        <chrono>
        <cmath>
        <functional>
        <map>
        <memory>
        <numeric>
        <optional>
        <random>
        <set>
        <stdexcept>
        <stdint.h>
        <stdlib.h>
        <type_traits>
        <utility>
        <vector>
    )
endif()

if(WITH_FIXMES)
    message(STATUS "Building with fixme messages")
    target_compile_definitions(${JA2_BINARY} PRIVATE WITH_FIXMES)
endif()

if(WITH_MAEMO)
    message(STATUS "Building with right click mapped to F4 (menu button)")
    target_compile_definitions(${JA2_BINARY} PRIVATE WITH_MAEMO)
endif()

if(NOT WITH_MAGICENUM)
    message(STATUS "Compiling without magic_enum. This build will not have enum-gen functionalities")
    target_compile_definitions(${JA2_BINARY} PRIVATE NO_MAGICENUM_LIB)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR ENABLE_ASSERTS)
    target_compile_definitions(${JA2_BINARY} PRIVATE ENABLE_ASSERTS)
endif()

add_warnings(${JA2_BINARY})
