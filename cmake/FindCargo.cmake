# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# The module defines the following variables:
#   CARGO_FOUND - true if cargo was found
#   CARGO_EXECUTABLE - path to the executable
#   CARGO_VERSION - cargo version number
# Example usage:
#   find_package(Cargo 0.10.0 REQUIRED)

find_program(CARGO_EXECUTABLE cargo HINTS ENV PATH PATHS PATH_SUFFIXES bin)

if (CARGO_EXECUTABLE)
    set(COMMAND ${CARGO_EXECUTABLE} --version)
    execute_process(COMMAND ${COMMAND} OUTPUT_VARIABLE CARGO_VERSION_OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(CARGO_VERSION_OUTPUT MATCHES "cargo ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(CARGO_VERSION ${CMAKE_MATCH_1} CACHE INTERNAL "Cargo version")
    endif()
endif()
mark_as_advanced(CARGO_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cargo REQUIRED_VARS CARGO_EXECUTABLE CARGO_VERSION VERSION_VAR CARGO_VERSION)
