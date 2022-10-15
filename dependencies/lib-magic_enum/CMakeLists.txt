# \file dependencies/lib-magic_enum/CMakeLists.txt

option(LOCAL_MAGICENUM_LIB "Download and build magic_enum instead of searching the system" ON)

if (NOT WITH_MAGICENUM)
    return()
endif()

if (NOT LOCAL_MAGICENUM_LIB)
    message(STATUS "Using system magic_enum")

    if (NOT DEFINED MAGICENUM_INCLUDE_DIR)
        message(FATAL_ERROR "MAGICENUM_INCLUDE_DIR var not set")
    endif()

    include(CheckIncludeFileCXX)
    set (CMAKE_REQUIRED_INCLUDES "${MAGICENUM_INCLUDE_DIR}")

    CHECK_INCLUDE_FILE_CXX("magic_enum.hpp" MAGICENUM_FOUND)
    if (NOT MAGICENUM_FOUND)
        message(FATAL_ERROR "magic_enum.hpp not found")
    endif()

    set(MAGICENUM_INCLUDE_DIR "${MAGICENUM_INCLUDE_DIR}" PARENT_SCOPE)

    return()
endif()

message(STATUS "<magic_enum>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-magic_enum/src")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

set(MAGICENUM_INCLUDE_DIR ${SRC_DIR}/include PARENT_SCOPE)

message(STATUS "</magic_enum>")
