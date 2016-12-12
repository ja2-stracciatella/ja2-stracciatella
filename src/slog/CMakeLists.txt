cmake_minimum_required(VERSION 2.8)

project(slog)

set(source_files
    slog.c
    )

add_library(slog ${source_files})
