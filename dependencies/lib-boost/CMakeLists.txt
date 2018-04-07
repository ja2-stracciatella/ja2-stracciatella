cmake_minimum_required(VERSION 2.8)

project(boost)

include_directories(
    "${PROJECT_SOURCE_DIR}/boost"
)

set(source_files
    libs/system/src/error_code.cpp
    libs/filesystem/src/codecvt_error_category.cpp
    libs/filesystem/src/operations.cpp
    libs/filesystem/src/path.cpp
    libs/filesystem/src/path_traits.cpp
    libs/filesystem/src/portability.cpp
    libs/filesystem/src/unique_path.cpp
    libs/filesystem/src/utf8_codecvt_facet.cpp
    libs/filesystem/src/windows_file_codecvt.cpp
)

add_library(boost ${source_files})

install(TARGETS boost DESTINATION lib)
install(DIRECTORY boost/ DESTINATION include/boost FILES_MATCHING PATTERN "*.hpp")
