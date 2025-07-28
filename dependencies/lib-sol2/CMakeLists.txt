# \file dependencies/lib-sol2/CMakeLists.txt

option(LOCAL_SOL_LIB "Download and build Sol2 instead of searching the system" ON)
if (NOT LOCAL_SOL_LIB)
    message(STATUS "Using system Sol2")
    find_package(sol2 "3.2.2"
        REQUIRED
        PATH_SUFFIXES lib/cmake/sol2 # path to sol2-config.cmake
    )
    if (NOT sol2_FOUND)
        message(FATAL_ERROR "Sol2 not found")
    endif()

    set(SOL_INCLUDE_DIR "${SOL2_INCLUDE_DIRS}" PARENT_SCOPE)
    return()
endif()

message(STATUS "<sol2>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-sol2/src")
set(BUILD_DIR "${CMAKE_BINARY_DIR}/lib-sol2/build")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# copy patch to build dir
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/gcc-15.patch"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/gcc-15.patch"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

set(SOL_INCLUDE_DIR ${SRC_DIR}/single/include PARENT_SCOPE)

message(STATUS "</sol2>")
