set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(LOCAL_SDL_LIB "dependencies/lib-SDL2-2.0.20-macos" CACHE STRING "" FORCE)
set(LOCAL_GTEST_LIB OFF CACHE BOOL "" FORCE)
set(CMAKE_MACOSX_RPATH ON CACHE BOOL "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "-framework IOKit -framework Carbon -framework AudioUnit -framework AudioToolbox -framework OpenGL -framework CoreFoundation -framework AppKit" CACHE STRING "" FORCE)
