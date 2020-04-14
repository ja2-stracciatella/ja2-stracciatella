# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# The module defines the following variables:
#   RUSTC_FOUND - true if rust was found
#   RUSTC_EXECUTABLE - path to the executable
#   RUSTC_VERSION - rust stable version number
#   RUSTC_HOST = rust host triplet
# Example usage:
#   find_package(Rustc 0.10.0 REQUIRED)

find_program(RUSTC_EXECUTABLE rustc HINTS ENV PATH PATHS PATH_SUFFIXES bin)

if (RUSTC_EXECUTABLE)
    execute_process(
        COMMAND ${RUSTC_EXECUTABLE} -Vv
        OUTPUT_VARIABLE RUSTC_OUTPUT
    )
    if (RUSTC_OUTPUT MATCHES "release: ([^\n]+)\n")
        set(RUSTC_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "Rustc version")
    endif()
    if (RUSTC_OUTPUT MATCHES "host: ([^\n]+)\n")
        set(RUSTC_HOST ${CMAKE_MATCH_1} CACHE INTERNAL "Rustc host")
    endif()
endif()
mark_as_advanced(RUSTC_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rustc
    REQUIRED_VARS RUSTC_EXECUTABLE RUSTC_VERSION RUSTC_HOST
    VERSION_VAR RUSTC_VERSION
)
