include(ExternalProject)

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/external-project-cache.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake"
    @ONLY
)

externalproject_add(stracciatella
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/rust"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/lib-stracciatella"
    CMAKE_ARGS
        "-C${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib-stracciatella"
        "-DLIBSTRACCIATELLA_TARGET=${LIBSTRACCIATELLA_TARGET}"
        "-DEXTRA_DATA_DIR=${EXTRA_DATA_DIR}"
    # NOTE this will always use the initial config in multi-config projects
    BUILD_COMMAND ""
    INSTALL_COMMAND "${CMAKE_COMMAND}" --build . --target install --config "${CMAKE_BUILD_TYPE}"
)

# Trigger a rebuild of stracciatela when the sources change.
# The md5 checksums and paths of the source files are computed every time cmake runs.
# When the computed data is different from the data in the md5 file,
# the md5 file is updated and the update file is touched if it exists.
# The update file is the stamp for the update step of the external project.
set(STAMP_GLOB_RECURSE_SOURCES
    "${CMAKE_SOURCE_DIR}/rust/CMakeLists.txt"
    "${CMAKE_SOURCE_DIR}/rust/*.toml"
    "${CMAKE_SOURCE_DIR}/rust/*.rs"
)
set(STAMP_MD5_FILE "${CMAKE_CURRENT_BINARY_DIR}/stracciatella-stamp.md5")
set(STAMP_UPDATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/stracciatella-prefix/src/stracciatella-stamp/stracciatella-update")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/external-project-stamp.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/external-project-stamp.cmake"
    @ONLY
)
add_custom_target(stracciatella-stamp COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/external-project-stamp.cmake")
add_dependencies(stracciatella stracciatella-stamp)


set(STRACCIATELLA_INCLUDE_DIR
    "${CMAKE_BINARY_DIR}/lib-stracciatella/include"
    PARENT_SCOPE
)
set(STRACCIATELLA_LIBRARIES
    "${CMAKE_BINARY_DIR}/lib-stracciatella/lib/${CMAKE_STATIC_LIBRARY_PREFIX}stracciatella${CMAKE_STATIC_LIBRARY_SUFFIX}"
    PARENT_SCOPE
)
set(STRACCIATELLA_EXECUTABLES
    "${CMAKE_BINARY_DIR}/lib-stracciatella/bin/ja2-resource-pack${CMAKE_EXECUTABLE_SUFFIX}"
    PARENT_SCOPE
)

