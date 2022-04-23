set(MINGW_PREFIX "x86_64-w64-mingw32" CACHE STRING "MinGW Prefix")
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${MINGW_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${MINGW_PREFIX}-g++")
set(CMAKE_RC_COMPILER "${MINGW_PREFIX}-windres")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CARGO_BUILD_TARGET "x86_64-pc-windows-gnu" CACHE STRING "" FORCE)

set(LOCAL_SDL_LIB "dependencies/lib-SDL2-2.0.20-mingw/x86_64-w64-mingw32" CACHE STRING "" FORCE)
set(LOCAL_FLTK_LIB ON CACHE BOOL "" FORCE)
set(LOCAL_GTEST_LIB ON CACHE BOOL "" FORCE)
set(CFLAGS "${CFLAGS} -mwindows -mconsole" CACHE BOOL "" FORCE)
set(CMAKE_CXX_FLAGS_INIT "-Wa,-mbig-obj")
