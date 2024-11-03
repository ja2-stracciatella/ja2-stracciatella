# \file dependencies/lib-magic_enum/CMakeLists.txt

option(LOCAL_MAGICENUM_LIB "Download and build magic_enum instead of searching the system" ON)

if(NOT WITH_MAGICENUM)
    target_compile_definitions(${JA2_BINARY} PRIVATE NO_MAGICENUM_LIB)
    return()
endif()

function(verify_header test_dir)
    include(CheckIncludeFileCXX)
    set(CMAKE_REQUIRED_INCLUDES "${test_dir}")
    unset(MAGICENUM_FOUND CACHE)
    CHECK_INCLUDE_FILE_CXX("magic_enum.hpp" MAGICENUM_FOUND)
    if (NOT MAGICENUM_FOUND)
        message(FATAL_ERROR "magic_enum.hpp not found")
    endif()
    target_include_directories(${JA2_BINARY} PRIVATE "${test_dir}" SYSTEM)
endfunction()

if (NOT LOCAL_MAGICENUM_LIB)
    message(STATUS "Using system magic_enum")

    if (NOT DEFINED MAGICENUM_INCLUDE_DIR)
        message(FATAL_ERROR "MAGICENUM_INCLUDE_DIR var not set")
    endif()

    verify_header("${MAGICENUM_INCLUDE_DIR}")
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

verify_header("${SRC_DIR}/include")

message(STATUS "</magic_enum>")
