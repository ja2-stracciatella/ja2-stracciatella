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
