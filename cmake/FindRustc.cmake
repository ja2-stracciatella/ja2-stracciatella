# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# The module defines the following variables:
#   RUSTC_FOUND - true if cargo was found
#   RUSTC_EXECUTABLE - path to the executable
#   RUSTC_VERSION - cargo version number
# Example usage:
#   find_package(Rustc 0.10.0 REQUIRED)

find_program(RUSTC_EXECUTABLE rustc HINTS ENV PATH PATHS PATH_SUFFIXES bin)

if (RUSTC_EXECUTABLE)
    set(COMMAND ${RUSTC_EXECUTABLE} --version)
    execute_process(COMMAND ${COMMAND} OUTPUT_VARIABLE RUSTC_VERSION_OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(RUSTC_VERSION_OUTPUT MATCHES "rustc ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(RUSTC_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "Rustc version")
    endif()
endif()
mark_as_advanced(RUSTC_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rustc REQUIRED_VARS RUSTC_EXECUTABLE RUSTC_VERSION VERSION_VAR RUSTC_VERSION)
