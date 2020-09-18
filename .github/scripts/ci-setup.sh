#!/usr/bin/env bash
#
# Set up the CI environment
#
# Requires the following environment variables:64
#   CI_TARGET - target we are building for: linux/linux-mingw/mac

set -e
set -x

echo "CI_TARGET: $CI_TARGET"

export CLOUDSDK_CORE_DISABLE_PROMPTS=1
SCCACHE_VERSION="0.2.13"
SCCACHE_LINUX_PACKAGE="sccache-${SCCACHE_VERSION}-x86_64-unknown-linux-musl"

echo "## prepare environment ##"
export RUSTUP_INIT_ARGS="-y --no-modify-path --default-toolchain=$(cat ./rust-toolchain)"
if [[ "$CI_TARGET" == "linux" ]]; then
    sudo apt-get -yq update
    sudo apt-get -yq install build-essential libsdl2-dev libfltk1.3-dev
    
    # sccache
    curl -Ls "https://github.com/mozilla/sccache/releases/download/${SCCACHE_VERSION}/${SCCACHE_LINUX_PACKAGE}.tar.gz" | sudo tar zx --strip-component=1 -C /usr/bin/

    # Google Cloud SDK for Artifact Upload
    curl https://sdk.cloud.google.com | bash
    source $HOME/google-cloud-sdk/path.bash.inc

    # linuxdeploy for appimage build on linux
    mkdir $HOME/linuxdeploy
    curl https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -L -o $HOME/linuxdeploy/linuxdeploy
    chmod +x $HOME/linuxdeploy/linuxdeploy
    curl https://github.com/TheAssassin/appimagelint/releases/download/continuous/appimagelint-x86_64.AppImage -L -o $HOME/linuxdeploy/appimagelint
    chmod +x $HOME/linuxdeploy/appimagelint
    export PATH=$PATH:$HOME/linuxdeploy
    linuxdeploy --version
    appimagelint --version
elif [[ "$CI_TARGET" == "linux-mingw64" ]]; then
    # cross compiling
    sudo apt-get -yq update
    sudo apt-get -yq install build-essential mingw-w64

    # sccache
    curl -Ls "https://github.com/mozilla/sccache/releases/download/${SCCACHE_VERSION}/${SCCACHE_LINUX_PACKAGE}.tar.gz" | sudo tar zx --strip-component=1 -C /usr/bin/
    
    export RUSTUP_INIT_ARGS="${RUSTUP_INIT_ARGS} --target=x86_64-pc-windows-gnu"

    curl https://sdk.cloud.google.com | bash
    source $HOME/google-cloud-sdk/path.bash.inc
elif [[ "$CI_TARGET" == "mac" ]]; then
    brew install sccache
    brew cask install google-cloud-sdk
    source "$(brew --prefix)/Caskroom/google-cloud-sdk/latest/google-cloud-sdk/path.bash.inc"
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
gcloud version