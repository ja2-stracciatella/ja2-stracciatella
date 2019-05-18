cmake_minimum_required(VERSION 2.8)

project(slog)

set(CMAKE_CXX_STANDARD 11)

set(source_files
    slog.cc
)

add_library(slog ${source_files})
