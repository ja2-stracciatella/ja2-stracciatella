set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(LOCAL_SDL_LIB "dependencies/lib-SDL2-2.0.8-VC" CACHE STRING "" FORCE)
set(LOCAL_FLTK_LIB ON CACHE BOOL "" FORCE)
set(LOCAL_GTEST_LIB ON CACHE BOOL "" FORCE)

set(_platform "${CMAKE_GENERATOR_PLATFORM}")
if("${_platform}" STREQUAL "" AND "${CMAKE_GENERATOR}" MATCHES "^Visual Studio ")
	set(_platform "${CMAKE_VS_PLATFORM_NAME_DEFAULT}")
endif()

if("${_platform}" MATCHES "(Win32|x86)")
    set(LIBSTRACCIATELLA_TARGET "i686-pc-windows-msvc" CACHE STRING "" FORCE)
elseif("${_platform}" MATCHES "(Win64|x64)")
    set(LIBSTRACCIATELLA_TARGET "x86_64-pc-windows-msvc" CACHE STRING "" FORCE)
elseif("${_platform}" STREQUAL "")
	message(FATAL_ERROR "target platform is unknown, try invoking cmake with the '-A <platform-name>' option or setting CMAKE_GENERATOR_PLATFORM")
else()
	message(FATAL_ERROR "target platform is not supported: '${_platform}'")
endif()

set(CMAKE_CXX_FLAGS_INIT "/bigobj")