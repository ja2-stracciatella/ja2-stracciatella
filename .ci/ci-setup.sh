#!/usr/bin/env bash
#
# Set up the CI environment
#
# Requires the following environment variables:64
#   CI_TARGET - target we are building for: linux/linux-mingw/mac

set -e
set -x

echo "CI_TARGET: $CI_TARGET"

source "$(dirname "${BASH_SOURCE[0]}")/ci-functions.sh"

echo "## prepare environment ##"
if [[ "$CI_TARGET" == "linux" ]]; then
    GCC_VER="${TARGET_GCC_MAJOR_VERSION:-8}"

    # SDL2 and FLTK to link against
    linux-install-via-apt-get libsdl2-dev libfltk1.3-dev "gcc-$GCC_VER" "g++-$GCC_VER"

    # choose a new-enough gcc version
    linux-set-gcc-version "$GCC_VER"

    # sccache for compilation caching
    linux-install-sccache

    # Google Cloud SDK for Artifact Upload
    linux-install-google-cloud-sdk

    # Rust via Rustup
    unix-install-rustup

    # Appimage build tools (linuxdeploy and appimagelint)
    linux-install-appimage-build-tools
elif [[ "$CI_TARGET" == "linux-mingw64" ]]; then
    GCC_VER="${TARGET_GCC_MAJOR_VERSION:-8}"

    # MinGW compiler for cross-compiling
    linux-install-via-apt-get build-essential mingw-w64 "gcc-$GCC_VER" "g++-$GCC_VER"

    # choose a new-enough version of gcc
    linux-set-gcc-version "$GCC_VER"

    # sccache for compilation caching
    linux-install-sccache

    # Google Cloud SDK for Artifact Upload
    linux-install-google-cloud-sdk
    
    # Rust via Rustup
    unix-install-rustup x86_64-pc-windows-gnu

    # Use CMake 3.21 instead of the default installed.
    # This is a workaround for the packaging target failing since CMake v3.22 (#1451)
    curl -sL https://cmake.org/files/v3.21/cmake-3.21.4-linux-x86_64.sh -o cmakeinstall.sh \
        && chmod +x cmakeinstall.sh \
        && sudo ./cmakeinstall.sh --prefix=/usr/local --exclude-subdir \
        && rm cmakeinstall.sh

elif [[ "$CI_TARGET" == "mac" ]]; then
    # sccache for compilation caching
    macOS-install-via-brew sccache
    
    # Google Cloud SDK for Artifact Upload
    macOS-install-via-brew-cask google-cloud-sdk
    source "$(brew --prefix)/Caskroom/google-cloud-sdk/latest/google-cloud-sdk/path.bash.inc"

    # Rust via Rustup
    unix-install-rustup
elif [[ "$CI_TARGET" == windows-* ]]; then
    # sccache for compilation caching
    windows-install-via-chocolatey sccache

    # Google Cloud SDK for Artifact Upload
    windows-install-google-cloud-sdk

    # Rust via Rustup
    if [[ "$CI_TARGET" == "windows-msvc-x86" ]]; then
        windows-install-rustup i686-pc-windows-msvc
    elif [[ "$CI_TARGET" == "windows-msvc-amd64" ]]; then
        windows-install-rustup x86_64-pc-windows-msvc
    else
        echo "unexpected target ${CI_TARGET}"
        exit 1
    fi
elif [[ "$CI_TARGET" == "android" ]]; then
    # Ninja build system
    linux-install-via-apt-get ninja-build

    # sccache for compilation caching
    linux-install-sccache

    # Google Cloud SDK for Artifact Upload
    linux-install-google-cloud-sdk

    # Rust via Rustup
    unix-install-rustup armv7-linux-androideabi aarch64-linux-android i686-linux-android x86_64-linux-android

    # Specific version of Android NDK
    linux-install-via-android-sdkmanager "ndk;21.0.6113669"
else
    echo "unexpected target ${CI_TARGET}"
    exit 1
fi

# print build environment info
rustup show
env
which rustc
rustc -V
which cargo
cargo -V
cargo clippy -- -V
cargo fmt -- -V
which cmake
cmake --version
sccache -V
which gcloud
gcloud version
if [[ "$CI_TARGET" == "linux" ]]; then
    linuxdeploy --version
    appimagelint --version
fi