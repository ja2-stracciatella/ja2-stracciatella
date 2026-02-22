project(smacker)

add_library(smacker STATIC
    libsmacker/smacker.c
)

target_include_directories(smacker PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libsmacker")

if(NOT MSVC)
    target_compile_options(smacker PRIVATE -pipe -Wall -Wextra -fsigned-char)
endif()
