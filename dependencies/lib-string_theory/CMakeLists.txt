# \file dependencies/lib-string_theory/CMakeLists.txt

option(LOCAL_STRING_THEORY_LIB "Download and build string_theory instead of searching the system" ON)
if (NOT LOCAL_STRING_THEORY_LIB)
    message(STATUS "Using system string_theory")
    find_package(string_theory 3
        REQUIRED
        HINTS "${STRING_THEORY_ROOT}" "$ENV{STRING_THEORY_ROOT}"
        PATH_SUFFIXES lib/cmake/string_theory # path to string_theory-config.cmake
    )
    mark_as_advanced(string_theory_DIR)
    if (NOT string_theory_FOUND)
        message(FATAL_ERROR "string_theory 3+ not found")
    endif()

    # interface library
    add_library(string_theory-internal INTERFACE)
    target_include_directories(string_theory-internal INTERFACE ${STRING_THEORY_INCLUDE_DIRS})
    return()
endif()

message(STATUS "<string_theory>")

# create builder
set(CACHE_FILE "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake")
set(INSTALL_PREFIX "${CMAKE_BINARY_DIR}/lib-string_theory")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/external-project-cache.cmake.in"
    "${CACHE_FILE}"
    @ONLY
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/builder/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/builder/CMakeLists.txt"
    @ONLY
)

# execute builder
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}""-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/builder")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/builder")

# interface library
include("${INSTALL_PREFIX}/lib/cmake/string_theory/string_theory-config.cmake")
add_library(string_theory-internal INTERFACE)
target_include_directories(string_theory-internal INTERFACE "${STRING_THEORY_INCLUDE_DIRS}")

message(STATUS "</string_theory>")
