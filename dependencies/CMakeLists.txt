include(ExternalProject)

set(DEPENDENCIES_CMAKE_FLAGS -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_ROOT} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

if (LOCAL_BOOST_LIB)
    externalproject_add(boost-internal
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib-boost"
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_ROOT}"
    )
endif()

if (LOCAL_FLTK_LIB)
	externalproject_add(fltk-internal
		URL "http://fltk.org/pub/fltk/1.3.4/fltk-1.3.4-2-source.tar.gz"
		CMAKE_ARGS ${DEPENDENCIES_CMAKE_FLAGS}
			-DOPTION_USE_GL=OFF -DOPTION_USE_SYSTEM_ZLIB=OFF -DOPTION_USE_SYSTEM_LIBJPEG=OFF
			-DOPTION_USE_SYSTEM_LIBPNG=OFF -DOPTION_USE_XINERAMA=OFF -DOPTION_USE_XFIXES=OFF
			-DOPTION_USE_XCURSOR=OFF -DOPTION_USE_XFT=OFF -DOPTION_USE_XRENDER=OFF
			-DOPTION_USE_XDBE=OFF -DOPTION_BUILD_EXAMPLES=OFF
	)
endif()

if (LOCAL_GTEST_LIB)
	externalproject_add(gtest-internal
		URL "https://github.com/google/googletest/archive/release-1.8.0.tar.gz"
		CMAKE_ARGS ${DEPENDENCIES_CMAKE_FLAGS}
			-DBUILD_GTEST=ON -DBUILD_GMOCK=OFF -DINSTALL_GTEST=ON
	)
endif()

if (LOCAL_SDL_LIB)
	externalproject_add(sdl2-internal
		URL "https://www.libsdl.org/release/SDL2-2.0.4.tar.gz"
		CMAKE_ARGS ${DEPENDENCIES_CMAKE_FLAGS}
			-DVIDEO_WAYLAND=OFF
	)
endif()

externalproject_add(rapidjson-internal
    URL "https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz"
    CMAKE_ARGS ${DEPENDENCIES_CMAKE_FLAGS}
		-DRAPIDJSON_BUILD_DOC=OFF
		-DRAPIDJSON_BUILD_EXAMPLES=OFF
		-DRAPIDJSON_BUILD_TESTS=OFF
)

externalproject_add(smacker-internal
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib-smacker"
    CMAKE_ARGS ${DEPENDENCIES_CMAKE_FLAGS}
)

externalproject_add(utf8cpp-internal
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib-utf8cpp"
    CMAKE_ARGS ${DEPENDENCIES_CMAKE_FLAGS}
)
