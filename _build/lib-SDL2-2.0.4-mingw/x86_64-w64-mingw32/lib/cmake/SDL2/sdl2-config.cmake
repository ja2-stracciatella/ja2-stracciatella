# sdl2 cmake project-config input for ./configure scripts

set(prefix "/usr/local/cross-tools/x86_64-w64-mingw32") 
set(exec_prefix "${prefix}")
set(libdir "${exec_prefix}/lib")
set(SDL2_PREFIX "/usr/local/cross-tools/x86_64-w64-mingw32")
set(SDL2_EXEC_PREFIX "/usr/local/cross-tools/x86_64-w64-mingw32")
set(SDL2_LIBDIR "${exec_prefix}/lib")
set(SDL2_INCLUDE_DIRS "${prefix}/include/SDL2")
set(SDL2_LIBRARIES "-L${SDL2_LIBDIR}  -lmingw32 -lSDL2main -lSDL2  -mwindows")
