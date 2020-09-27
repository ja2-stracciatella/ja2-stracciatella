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
    # SDL2 and FLTK to link against
    linux-install-via-apt-get libsdl2-dev libfltk1.3-dev
    
    # sccache for compilation caching
    linux-install-sccache

    # Google Cloud SDK for Artifact Upload
    linux-install-google-cloud-sdk

    # Rust via Rustup
    unix-install-rustup

    # Appimage build tools (linuxdeploy and appimagelint)
    linux-install-appimage-build-tools
elif [[ "$CI_TARGET" == "linux-mingw64" ]]; then
    # MinGW compiler for cross-compiling
    linux-install-via-apt-get build-essential mingw-w64

    # sccache for compilation caching
    linux-install-sccache

    # Google Cloud SDK for Artifact Upload
    linux-install-google-cloud-sdk
    
    # Rust via Rustup
    unix-install-rustup x86_64-pc-windows-gnu
elif [[ "$CI_TARGET" == "mac" ]]; then
    # sccache for compilation caching
    macOS-install-via-brew sccache
    
    # Google Cloud SDK for Artifact Upload
    macOS-install-via-brew-cask google-cloud-sdk
    source "$(brew --prefix)/Caskroom/google-cloud-sdk/latest/google-cloud-sdk/path.bash.inc"

    # Rust via Rustup
    unix-install-rustup
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
command -v sccache && sccache -V || echo "sccache not available"
which gcloud
gcloud version
if [[ "$CI_TARGET" == "linux" ]]; then
    linuxdeploy --version
    appimagelint --version
fi