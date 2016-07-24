## Dependencies

- SDL 1.2 (included for Windows and OS X)
- boost-filesystem (included with `-DLOCAL_BOOST_LIB=ON`)
- cmake
- Your systems compiler

## General Notes

We use cmake as our build system, which is aimed at an out-of-source build. That means that you should call
cmake from a directory that is different from the source directory. You can create a directory inside the source
directory (`_bin` is ignored by git). Cmake only needs to be executed once unless you want to change options.

```
mkdir _bin && cd _bin
```

## Build on Linux or freeBSD/openBSD

```
cmake path/to/source
make
```

## Build for Windows on Linux using MinGW (cross build)

Additional requirements: MinGW compiler

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake path/to/source
make
```

## Build on macOS

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake path/to/source
make
```

## Generate Visual Studio Solution

If you are most familiar using Visual Studio for development you can generate a solution from the sources.

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-msvc.cmake -G "Visual Studio 10" path/to/source
```

## Generate XCode Project

If you are most familiar using XCode for development you can generate a project from the sources.

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -G "XCode" path/to/source
```

## Additional Options

If you want to configure the build differently, you can pass additional options to
cmake. The supported options are:

| Switch        | Description           | Default  |
| ------------- |-------------| -----|
| `EXTRA_DATA_DIR` | Directory to read externalized data from. Useful for creating installable packages that have a fixed data path. | `` |
| `LOCAL_SDL_LIB` | Use SDL library from this directory. | `` |
| `LOCAL_BOOST_LIB` | Build with local boost lib from `_build` directory. No global boost installation required. | `OFF` |
| `WITH_UNITTESTS` | Build with unittests | `ON` |
| `WITH_FIXMES` | Build with fixme messages | `OFF` |
| `WITH_MAEMO` | Build with right click mapped to F4 (menu button) | `OFF` |

Example:

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DWITH_FIXMES=ON path/to/source
make
```
