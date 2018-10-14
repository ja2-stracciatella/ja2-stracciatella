include(ExternalProject)

externalproject_add(gtest-internal
    URL "https://github.com/google/googletest/archive/release-1.8.1.tar.gz"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/lib-gtest"
    CMAKE_ARGS
        ${EXTERNAL_PROJECT_CMAKE_ARGS}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib-gtest
        -Dgtest_force_shared_crt=TRUE
        -Dgtest_disable_pthreads=TRUE
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
