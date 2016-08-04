cmake_minimum_required(VERSION 2.8)

project(smacker)

set(source_files
    libsmacker/smacker.c
    libsmacker/smk_hufftree.c
    libsmacker/smk_bitstream.c
)

add_library(smacker ${source_files})
