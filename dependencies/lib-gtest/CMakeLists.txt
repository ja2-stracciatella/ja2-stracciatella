include(ExternalProject)

externalproject_add(gtest-internal
    URL "https://github.com/google/googletest/archive/release-1.8.1.tar.gz"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/lib-gtest"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib-gtest
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_RC_COMPILER=${CMAKE_RC_COMPILER}
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
        -Dgtest_force_shared_crt=TRUE
        -Dgtest_disable_pthreads=TRUE
)
