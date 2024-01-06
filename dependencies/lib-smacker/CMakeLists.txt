option(LOCAL_SMACKER_LIB "Use bundled libsmacker instead of searching the system" ON)
if (NOT LOCAL_SMACKER_LIB)
    message(STATUS "Using system libsmacker")
    find_library(LIBSMACKER_found libsmacker.so)
    if (NOT LIBSMACKER_found)
        message(FATAL_ERROR "libsmacker not found")
    endif()
    return()
endif()

project(smacker)

add_library(smacker STATIC
    libsmacker/smacker.c
    libsmacker/smk_hufftree.c
    libsmacker/smk_bitstream.c
)

target_include_directories(smacker PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libsmacker")

if(NOT MSVC)
    # Compilation flags copied from the Makefile
    target_compile_options(smacker PRIVATE -pipe -Wall -Wextra -ansi -pedantic)
endif()
