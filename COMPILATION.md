## Dependencies

- SDL2 >= `2.0.4` (version `2.0.8` is included in this repo for Windows and macOS).
  __WARNING__: There is an issue with SDL in version `2.0.6` that causes segfaults when playing sounds.
  Please ensure that you run the game with a different version of the SDL2 library otherwise sound will be
  disabled.
- cmake
- Rust and Cargo
- Your systems compiler

## Optional dependencies

FLTK is required to build the GUI launcher. If it is not installed, a bundled copy will be used.
If you do already have it, make sure the package also provides the libfltk_images library or in
case of Debian and derivatives, install it manually (libfltk-images1.3).

Stracciatella bundles a few other projects for development purposes. If you have them installed already,
the system version will be used. This holds for: gtest, rapidjson and string theory.

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

## Build on Linux or freeBSD

```
cmake path/to/source
make
```

If you want to be able to install the resulting binary on your system, please ensure that `CMAKE_INSTALL_PREFIX` matches
with `EXTRA_DATA_DIR`. Example: `cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DEXTRA_DATA_DIR=/usr/local/share/ja2 path/to/source`

## Build on OpenBSD (tested on -current as of mid-August 2021)

```
# The bundled/downloaded GTest sources fail to build.
doas pkg_add gtest

# miniaudio requires some feature test macros to build properly on OpenBSD
FEATURE_TEST_MACROS='-D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE'

cmake path/to/source \
	-DCMAKE_CXX_FLAGS="$FEATURE_TEST_MACROS" \
	-DCMAKE_C_FLAGS="$FEATURE_TEST_MACROS" \
	-DLOCAL_GTEST_LIB=0
make
```

## Build for Windows on Linux using MinGW (cross build)

Additional requirements: MinGW compiler

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake path/to/source
make
```

If you are using rustup, you might need to add the MinGW target to the rust toolchain before compiling.

When building for 64-bit:

```
rustup target add x86_64-pc-windows-gnu
```

When building for 32-bit:

```
rustup target add i686-pc-windows-gnu
```

## Build on macOS

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake path/to/source
make
```

## Build on Windows using MSYS2

Install [msys2](https://www.msys2.org/).

Open the msys2 shell.
Use "MSYS MinGW 64-bit" to build 64-bit and "MSYS MinGW 32-bit" to build 32-bit.

Update msys2, you might have to restart the msys2 shell and run the command again:
```
pacman -Syu
```

Install the build environment and dependencies:
```
pacman -S base-devel
```
to build 64-bit:
```
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-rust mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-fltk
```
to build 32-bit:
```
pacman -S mingw-w64-i686-toolchain mingw-w64-i686-rust mingw-w64-i686-cmake mingw-w64-i686-SDL2 mingw-w64-i686-fltk
```

Get ja2-stracciatella, cd into it, and build the package:
```
mkdir _bin && cd _bin
cmake .. "-GMSYS Makefiles" -DCPACK_GENERATOR=ZIP
make package
```

You now have a zip file with the game, including the dll dependencies.

## Generate Visual Studio Solution

If you are most familiar using Visual Studio for development you can generate a solution from the sources.

Install Visual C++, CMake tools, MSBuild and Windows SDK with Visual Studio Installer.

Then in Visual Studio's Developer Command Prompt, change to the ja2-stracciatella project directory, and generate the solution with CMake:

```
mkdir _bin
cd _bin
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-msvc.cmake ..
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
| `EXTRA_DATA_DIR` | Directory to read externalized data from relative to binary location. Useful for creating installable packages that have a fixed data path. | `` |
| `LOCAL_SDL_LIB` | Use SDL library from this directory. | `` |
| `WITH_UNITTESTS` | Build with unittests | `ON` |
| `WITH_FIXMES` | Build with fixme messages | `OFF` |
| `WITH_MAEMO` | Build with right click mapped to F4 (menu button) | `OFF` |
| `WITH_EDITOR_SLF` | Download the latest free editor.slf during build | `OFF` |

Example:

```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DWITH_FIXMES=ON path/to/source
make
```
