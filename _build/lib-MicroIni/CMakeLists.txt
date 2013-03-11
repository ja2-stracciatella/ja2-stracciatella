cmake_minimum_required(VERSION 2.8)

# default build type
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# project name
project(MicroIni)

# header path
include_directories("${PROJECT_SOURCE_DIR}/include")

# option for choosing the build type (shared or static)
option(BUILD_SHARED_LIBS "TRUE to build MicroIni as a shared library, FALSE to build it as a static library" TRUE)

# option for building the example
option(BUILD_EXAMPLE "TRUE to build the example, FALSE to ignore it" FALSE)

# option for building the documentation
option(BUILD_DOC "TRUE to generate the documentation, FALSE to ignore it" FALSE)

# define MICROINI_STATIC if the build type is set to static
if(NOT BUILD_SHARED_LIBS)
    add_definitions(-DMICROINI_STATIC)
endif()

# add the subdirectories
add_subdirectory(src/MicroIni)
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
if(BUILD_DOC)
    add_subdirectory(doc)
endif()

# install rules
install(FILES license.txt DESTINATION .)
install(FILES readme.md DESTINATION . RENAME readme.txt)
