include(ExternalProject)

set(STRACCIATELLA_INCLUDE_DIR
    "${CMAKE_BINARY_DIR}/lib-stracciatella/include"
)
set(STRACCIATELLA_LIBRARIES
    "${CMAKE_BINARY_DIR}/lib-stracciatella/lib/${CMAKE_STATIC_LIBRARY_PREFIX}stracciatella${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/external-project-cache.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake"
    @ONLY
)

externalproject_add(libstracciatella
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rust"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/lib-stracciatella"
    BUILD_ALWAYS ON
    CMAKE_ARGS
        "-C${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib-stracciatella"
        "-DLIBSTRACCIATELLA_TARGET=${LIBSTRACCIATELLA_TARGET}"
        "-DEXTRA_DATA_DIR=${EXTRA_DATA_DIR}"
    BUILD_BYPRODUCTS "${STRACCIATELLA_LIBRARIES}"
    USES_TERMINAL_BUILD TRUE
)
