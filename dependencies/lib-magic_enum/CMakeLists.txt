# \file dependencies/lib-magic_enum/CMakeLists.txt

option(WITH_MAGICENUM "Build with magic_enum library" ON)
option(LOCAL_MAGICENUM_LIB "Download and build magic_enum instead of searching the system" ON)

if(NOT WITH_MAGICENUM)
    target_compile_definitions(${JA2_BINARY} PRIVATE NO_MAGICENUM_LIB)
    return()
endif()

if (NOT LOCAL_MAGICENUM_LIB)
    message(STATUS "Using system magic_enum")

    # Try CMake config first (standard install via CMake)
    find_package(magic_enum 0.9.7 CONFIG QUIET)

    if(NOT magic_enum_FOUND)
        # Fallback to pkg-config (e.g., Meson installs)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(magic_enum REQUIRED IMPORTED_TARGET magic_enum>=0.9.7)
        target_link_libraries(${JA2_BINARY} PRIVATE PkgConfig::magic_enum)
        return()
    endif()

    target_link_libraries(${JA2_BINARY} PRIVATE magic_enum::magic_enum)
    return()
endif()

message(STATUS "<magic_enum>")

# create getter
set(GETTER_DIR "${CMAKE_BINARY_DIR}/dependencies/lib-magic_enum/getter")
set(SRC_DIR "${GETTER_DIR}/get-magic_enum-prefix/src/get-magic_enum")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${GETTER_DIR}/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${GETTER_DIR}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to configure magic_enum getter: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build "${GETTER_DIR}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to build magic_enum getter: ${result}")
endif()

if(NOT EXISTS "${SRC_DIR}/include/magic_enum/magic_enum.hpp")
    message(FATAL_ERROR "magic_enum headers not found at ${SRC_DIR}/include/magic_enum/magic_enum.hpp after extraction")
endif()

target_include_directories(${JA2_BINARY} SYSTEM PRIVATE "${SRC_DIR}/include")

message(STATUS "</magic_enum>")
