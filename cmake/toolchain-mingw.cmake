set(MINGW_PREFIX "i686-w64-mingw32" CACHE STRING "MinGW Prefix")
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${MINGW_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${MINGW_PREFIX}-g++")
set(CMAKE_RC_COMPILER "${MINGW_PREFIX}-windres")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(LOCAL_SDL_LIB "_build/lib-SDL-devel-1.2.15-mingw32" CACHE STRING "" FORCE)
set(LOCAL_BOOST_LIB ON CACHE BOOL "" FORCE)
set(CFLAGS "${CFLAGS} -mwindows -mconsole" CACHE BOOL "" FORCE)
