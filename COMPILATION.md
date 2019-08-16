## Dependencies

- SDL2 >= `2.0.4` (version `2.0.4` is included in this repo for Windows and macOS).
  __WARNING__: There is an issue with SDL in version `2.0.6` that causes segfaults when playing sounds.
  Please ensure that you run the game with a different version of the SDL2 library otherwise sound will be
  disabled.
- boost-filesystem (included with `-DLOCAL_BOOST_LIB=ON`)
- cmake
- Rust and Cargo
- Your systems compiler

## General Notes

We use cmake as our build system, which is aimed at an out-of-source build. That means that you should call
cmake from a directory that is different from the source directory. You can create a directory inside the source
directory (`_bin` is ignored by git). Cmake only needs to be executed once unless you want to change options.

```
mkdir _bin && cd _bin
```

## Rust notes

We suggest to install Rust and Cargo using [rustup](http://rustup.rs/). This way you will get the most recent version
installed in your home directory. As rust is a rapidly developing language the binaries provided by your distribution
might be too old to build ja2-stracciatella and its dependencies. When using rustup the correct version of rust should
be automatically selected.

If you don't want to use rustup, you can always look up the currently required version in the
[rust-toolchain file](https://github.com/ja2-stracciatella/ja2-stracciatella/blob/master/rust-toolchain)

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

If you are using rustup, you might need to add the x86_64-pc-windows-gnu target to the rust toolchain before compiling.

```
rustup target add x86_64-pc-windows-gnu
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

__Note__: If you add, move or delete any files. Please make sure to reflect your changes in the `CMakeLists.txt` files,
rerun cmake and reload your Solution before making any additional changes. Otherwise other build systems might fail
 when trying to build your changes.

## Generate XCode Project

If you are most familiar using XCode for development you can generate a project from the sources.

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -G "XCode" path/to/source
```

__Note__: If you add, move or delete any files. Please make sure to reflect your changes in the `CMakeLists.txt` files,
rerun cmake and reload your XCode project before making any additional changes. Otherwise other build systems might fail
 when trying to build your changes.

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
