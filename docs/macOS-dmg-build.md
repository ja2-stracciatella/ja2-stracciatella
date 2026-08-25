# Building the macOS DMG

How to produce `ja2-stracciatella_<version>_macos.dmg` on an Apple Silicon Mac, and
why the build files contain the macOS-specific workarounds they do.

The commands below are the ones CI runs, taken from
[`.ci/ci-setup.sh`](../.ci/ci-setup.sh) and [`.ci/ci-build.sh`](../.ci/ci-build.sh).
Prefer changing those scripts over inventing a separate local process.

> Note: [`Release-checklist.md`](Release-checklist.md) still says
> `make build-release-on-mac`. That target is gone — the top-level Makefile was
> deleted in `d1442ad67`. Use this document instead.

## Prerequisites

CI installs these with Homebrew; a local machine needs the same:

```sh
brew install fltk@1.3 googletest
brew install sccache          # optional, compile cache, auto-detected
```

`fltk@1.3` is required, not `fltk`. The launcher statically links the 1.3 archives
by absolute path. `googletest` is required because `cmake/toolchain-macos.cmake`
forces `LOCAL_GTEST_LIB=OFF`, so the bundled gtest is not built.

Installing these changes the machine, so **ask the user before running brew**.
Everything else in this document is repo-local.

## Build

```sh
export PATH="/opt/homebrew/opt/fltk@1.3/bin:$PATH"
mkdir -p ci-build && cd ci-build

cmake -DCMAKE_BUILD_TYPE=ReleaseWithDebInfo -DVERSION_TAG= -DWITH_EDITOR_SLF=ON \
      -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DCPACK_GENERATOR=Bundle ..

cmake --build . --config ReleaseWithDebInfo -- -j 8

# CI's test steps — all three must pass
cmake --build . --target cargo-test
./ja2 -unittests
./ja2-launcher -help

cmake --build . --target package
```

No extra `-D` flags are needed. If you find yourself adding some to get a build
through, that is a bug in the build files — fix it there so CI and local agree.

`ja2 -unittests` must run from the build directory; it resolves game data relative
to the working directory and fails with a VFS error anywhere else.

## Result

```
ci-build/ja2-stracciatella_<version>_macos.dmg
```

Ignore `ci-build/_CPack_Packages/Darwin/Bundle/`. CPack leaves a byte-identical
copy of the DMG and an uncompressed `temp.dmg` there; neither is the deliverable.

Verify before handing it over:

```sh
hdiutil attach -nobrowse -readonly ci-build/ja2-stracciatella_<version>_macos.dmg
R="/Volumes/ja2-stracciatella_<version>_macos/JA2 Stracciatella.app/Contents/Resources"

# must list nothing outside the bundle: only @rpath/@executable_path entries
otool -L "$R/ja2" "$R/ja2-launcher" | grep -vE "/usr/lib/|/System/"

codesign --verify "$R/ja2" "$R/ja2-launcher"
"$R/ja2" -help && "$R/ja2-launcher" -help
hdiutil detach "/Volumes/ja2-stracciatella_<version>_macos"
```

The app is ad-hoc signed and not notarized, so first launch needs
right-click → Open. The DMG is **arm64 only**; it will not run on an Intel Mac.
A universal build would need universal SDL2, FLTK and Rust artifacts, which CI
does not do either.

## Why the build files look the way they do

Three macOS-specific problems are already solved in the repo. Do not undo these.

**Homebrew's Lua shadows the bundled one.**
[`dependencies/lib-lua/CMakeLists.txt`](../dependencies/lib-lua/CMakeLists.txt)
publishes the bundled Lua headers under a `lua/` subdirectory as well. sol2 reaches
for `<lua/lua.h>` before plain `<lua.h>`, and Homebrew's `lua` formula installs
`include/lua/lua.h`. Without this the build compiles against system Lua 5.5 while
linking the bundled 5.3.6, failing with `undefined symbol _lua_newuserdatauv`.

**FLTK headers must match the FLTK libraries.**
[`src/launcher/CMakeLists.txt`](../src/launcher/CMakeLists.txt) pins
`FLTK_INCLUDE_DIR` to `fltk@1.3` because the static archives next to it are 1.3.
`find_package(FLTK)` otherwise picks up a linked FLTK 1.4 from
`/opt/homebrew/include`, which compiles and then fails to link on
`Fl_Window::size_range`.

**The bundle must carry its own libraries.**
[`cmake/bundle-macos-dylibs.cmake`](../cmake/bundle-macos-dylibs.cmake) runs as an
install step from [`CMakeLists.txt`](../CMakeLists.txt). It copies every dependency
outside `/usr/lib` and `/System` into the app, rewrites the load commands to
`@executable_path`, recurses into the copied libraries, and re-signs them — editing
a Mach-O invalidates its signature and arm64 refuses to run the result. Without it
the DMG only runs on a machine with the same Homebrew packages (in practice
`libpng`, pulled in by the launcher).

The first two only bite on a machine that has extra Homebrew formulae installed.
CI never hits them because its runner image is clean, which is exactly why they
went unnoticed. The macOS CI job runs on a `macos-latest` VM, not a container, so
there is no image to pull and reproduce locally — matching the installed package
set is the closest equivalent.
