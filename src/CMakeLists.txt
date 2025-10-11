message(STATUS "Entering src directory")

option(ENABLE_ASSERTS "Assertion failures terminate the program (always enabled in Debug)" OFF)
option(ENABLE_PCH "Use precompiled standard library headers during compilation" OFF)
option(WITH_ASAN "Build with address sanitizer" OFF)
option(WITH_FIXMES "Build with fixme messages" OFF)
option(WITH_MAEMO "Build with right click mapped to F4 (menu button)" OFF)

function(add_warnings target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wstring-conversion>
        $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall -Wtype-limits -Wignored-qualifiers>
        $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4018 /wd4101 /wd4244 /wd4996>
            # 4018 : example: signed/unsigned mismatch
            # 4101 : example: unreferenced local variable
            # 4244 : example: conversion from 'int' to 'real', possible loss of data
            # 4996 : example: This function or variable may be unsafe (sscanf)
    )
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

macro(add_and_include_subdirectory dir)
    add_subdirectory(${dir})
    target_include_directories(${JA2_BINARY} PRIVATE ${dir})
endmacro()

set(JA2_SOURCES "")
add_and_include_subdirectory(externalized)
add_and_include_subdirectory(game)
add_and_include_subdirectory(sgp)

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
