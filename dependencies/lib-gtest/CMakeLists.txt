include(ExternalProject)

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/external-project-cache.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake"
    @ONLY
)

externalproject_add(gtest-internal
    URL "https://github.com/google/googletest/archive/release-1.8.1.tar.gz"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/lib-gtest"
    CMAKE_ARGS
        "-C${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib-gtest"
        -Dgtest_force_shared_crt=TRUE
        -Dgtest_disable_pthreads=TRUE
    # NOTE this will always use the initial config in multi-config projects
    BUILD_COMMAND "${CMAKE_COMMAND}" --build . --config "${CMAKE_BUILD_TYPE}"
    INSTALL_COMMAND "${CMAKE_COMMAND}" --build . --target install --config "${CMAKE_BUILD_TYPE}"
)


set(LIB_SUFFIX "")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIB_SUFFIX "d")
endif()

set(GTEST_INCLUDE_DIRS
    "${CMAKE_BINARY_DIR}/lib-gtest/include"
    PARENT_SCOPE
)
set(GTEST_LIBRARIES
    "${CMAKE_BINARY_DIR}/lib-gtest/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${LIB_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    PARENT_SCOPE
)
