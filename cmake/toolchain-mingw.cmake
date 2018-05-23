set(MINGW_PREFIX "x86_64-w64-mingw32" CACHE STRING "MinGW Prefix")
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${MINGW_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${MINGW_PREFIX}-g++")
set(CMAKE_RC_COMPILER "${MINGW_PREFIX}-windres")

set(LOCAL_LIBS_ONLY  ON CACHE BOOL "" FORCE)
set(LIBSTRACCIATELLA_TARGET "x86_64-pc-windows-gnu" CACHE STRING "" FORCE)
