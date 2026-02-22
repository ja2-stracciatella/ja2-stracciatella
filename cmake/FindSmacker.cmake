# FindSmacker.cmake
# Find the libsmacker library
#
# This module defines:
#   SMACKER_FOUND        - True if libsmacker was found
#   SMACKER_INCLUDE_DIRS - The libsmacker include directories
#   SMACKER_LIBRARIES    - The libraries needed to use libsmacker

find_path(SMACKER_INCLUDE_DIR
    NAMES smacker.h
    PATH_SUFFIXES smacker
)

find_library(SMACKER_LIBRARY
    NAMES smacker
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Smacker
    REQUIRED_VARS SMACKER_LIBRARY SMACKER_INCLUDE_DIR
)

if(SMACKER_FOUND)
    set(SMACKER_LIBRARIES ${SMACKER_LIBRARY})
    set(SMACKER_INCLUDE_DIRS ${SMACKER_INCLUDE_DIR})

    if(NOT TARGET smacker)
        add_library(smacker UNKNOWN IMPORTED)
        set_target_properties(smacker PROPERTIES
            IMPORTED_LOCATION "${SMACKER_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SMACKER_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(SMACKER_INCLUDE_DIR SMACKER_LIBRARY)
