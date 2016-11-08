include(ExternalProject)

set(BOOST_BOOTSTRAP_SUFFIX ".sh")
if (MSVC)
    set(BOOST_BOOTSTRAP_SUFFIX ".bat")
endif()
set(BOOST_B2_SUFFIX "")
if (MSVC)
    set(BOOST_B2_SUFFIX ".exe")
endif()

externalproject_add(boost-internal
    URL "${CMAKE_CURRENT_SOURCE_DIR}/boost_1_61_0.tar.bz2"
    INSTALL_DIR "${DEPENDENCIES_INSTALL_DIR}"
    CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap${BOOST_BOOTSTRAP_SUFFIX}
    BUILD_COMMAND ""
    INSTALL_COMMAND <SOURCE_DIR>/b2${BOOST_B2_SUFFIX} install -d0 --prefix=<INSTALL_DIR> --layout=system --without-context --without-coroutine --with-filesystem link=static
    BUILD_IN_SOURCE 1
)

externalproject_add(gtest-internal
    URL "${CMAKE_CURRENT_SOURCE_DIR}/googletest-release-1.8.0.tar.gz"
    CONFIGURE_COMMAND cmake -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> <SOURCE_DIR>
    BUILD_COMMAND ""
    INSTALL_COMMAND cmake --build . --target install
    INSTALL_DIR "${DEPENDENCIES_INSTALL_DIR}"
)

externalproject_add(microini-internal
    URL "${CMAKE_CURRENT_SOURCE_DIR}/MicroIni-master.zip"
    CONFIGURE_COMMAND cmake -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_SHARED_LIBS=OFF <SOURCE_DIR>
    BUILD_COMMAND ""
    INSTALL_COMMAND cmake --build . --target install
    INSTALL_DIR "${DEPENDENCIES_INSTALL_DIR}"
)

externalproject_add(rapidjson-internal
    URL "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson-1.1.0.tar.gz"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include/ <INSTALL_DIR>
    INSTALL_DIR "${DEPENDENCIES_INSTALL_DIR}/include"
)

externalproject_add(utf8cpp-internal
    URL "${CMAKE_CURRENT_SOURCE_DIR}/utf8_v2_3_4.zip"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/source/ <INSTALL_DIR>
    INSTALL_DIR "${DEPENDENCIES_INSTALL_DIR}/include"
)
