include(ExternalProject)

externalproject_add(boost-internal
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib-boost"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies-root"
)
