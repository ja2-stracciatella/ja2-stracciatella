#!/usr/bin/env bash
#
# Set up the CI environment
#
# Requires the following environment variables:64
#   CI_TARGET - target we are building for: linux/linux-mingw/mac

set -e
set -x

echo "## prepare environment ##"
export RUSTUP_INIT_ARGS="-y --no-modify-path --default-toolchain=$(cat ./rust-toolchain) --profile=minimal"
if [[ "$CI_TARGET" == "linux" ]]; then
    sudo apt-get -yq update
    sudo apt-get -yq install build-essential libsdl2-dev libfltk1.3-dev ccache

elif [[ "$CI_TARGET" == "linux-mingw64" ]]; then
    # cross compiling
    sudo apt-get -yq update
    sudo apt-get -yq install build-essential mingw-w64
    sudo apt-get -yq install ccache

elif [[ "$CI_TARGET" == "msys2-mingw32" ]]; then
    # FIXME upgrades disabled until there is a fix for https://github.com/msys2/MSYS2-packages/issues/1141
    #pacman -Syu --noconfirm --needed # assumes the runtime has already been updated
    pacman -S --noconfirm --needed base-devel unzip
    pacman -S --noconfirm --needed mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-SDL2 mingw-w64-i686-fltk

elif [[ "$CI_TARGET" == "mac" ]]; then
    brew install ccache

else
    echo "unexpected target ${CI_TARGET}"
    exit 1
fi

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- $RUSTUP_INIT_ARGS
if [[ "$CI_TARGET" == "linux-mingw64" ]]; then
    # XXX currently rustup-init fails to add the target, so add it manually
    rustup target add x86_64-pc-windows-gnu
fi
export PATH=$PATH:$HOME/.cargo/bin

rustup component add clippy rustfmt
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
command -v ccache && ccache -V
