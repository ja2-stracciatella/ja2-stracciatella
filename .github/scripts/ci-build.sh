#!/usr/bin/env bash
#
# Build script for CI environments, assumes a posix-like host (linux or mac).
#
# Requires the following environment variables:
#   CI_REF - full reference of the current branch, tag, or pull request
#   CI_TARGET - target we are building for: linux/mingw/mac
# Required for publishing (nightly and releases):
#   SFTP_USER - ftp username for uploads (secret)
#   SFTP_PASSWORD - ftp password for uploads (secret)
# Unused environment variables:
#   CI_NAME - name of the job
#   CI_OS - host runner image: https://help.github.com/en/actions/automating-your-workflow-with-github-actions/virtual-environments-for-github-hosted-runners

set -e
set -x

echo "## prepare environment ##"
if [[ "${CI_REF}" == "refs/heads/nightly" ]]; then
  echo "-- NIGHTLY --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="nightlies"
  export BUILD_TYPE="ReleaseWithDebInfo"
  export VERSION_TAG="$(date +%Y%m%d)"
elif [[ "${CI_REF}" == "refs/tags/"* ]]; then
  echo "-- RELEASE --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="releases"
  export BUILD_TYPE="ReleaseWithDebInfo"
  # assumes that the version is already set up correctly
elif [[ "${CI_REF}" == "refs/pull/"* ]]; then
  export PULL_REQUEST=$(echo "${CI_REF}" | cut -d '/' -f 3)
  echo "-- PULL REQUEST ${PULL_REQUEST} --"
  export PUBLISH_BINARY="false" # secrets are not available in pull requests
  export PUBLISH_DIR="pull-requests/${PULL_REQUEST}"
  export VERSION_TAG="${PULL_REQUEST}pullrequest"
  export BUILD_TYPE="Debug"
else
  echo "-- QUICK BUILD --"
  export PUBLISH_BINARY="false"
  export BUILD_TYPE="Debug"
fi
export BUILD_CMD="cmake --build . --config ${BUILD_TYPE}"
export BUILD_TOOL_ARGS="-- -j 4"
export CONFIGURE_CMD="cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DVERSION_TAG=${VERSION_TAG}"
if [[ "${BUILD_TYPE}" != "Debug" ]]; then
   export CONFIGURE_CMD="${CONFIGURE_CMD} -DWITH_EDITOR_SLF=ON"
fi
if [[ "${PUBLISH_BINARY}" == "true" && "${SFTP_PASSWORD}" == "" ]]; then
  echo "Upload credentials are not set up"
  exit 1
fi

export RUN_TESTS=true
export RUN_INSTALL_TEST=true
export RUSTUP_INIT_ARGS="-y --no-modify-path --default-toolchain=$(cat ./rust-toolchain)"
if [[ "$CI_TARGET" == "linux" ]]; then
  sudo apt-get -yq update
  sudo apt-get -yq install build-essential libsdl2-dev libfltk1.3-dev ccache
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCMAKE_INSTALL_PREFIX=/usr -DEXTRA_DATA_DIR=/usr/share/ja2 -DCPACK_GENERATOR=DEB"
  export BUILD_TOOL_ARGS="-- -j 4"
elif [[ "$CI_TARGET" == "linux-mingw64" ]]; then
  # cross compiling
  sudo apt-get -yq update
  sudo apt-get -yq install build-essential mingw-w64 ccache
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake -DCPACK_GENERATOR=ZIP"
  export RUSTUP_INIT_ARGS="${RUSTUP_INIT_ARGS} --target=x86_64-pc-windows-gnu"
  export RUN_TESTS=false
  export BUILD_TOOL_ARGS="-- -j 4"
elif [[ "$CI_TARGET" == "msys2-mingw32" ]]; then
  # FIXME upgrades disabled until there is a fix for https://github.com/msys2/MSYS2-packages/issues/1141
  #pacman -Syu --noconfirm --needed # assumes the runtime has already been updated
  pacman -S --noconfirm --needed base-devel unzip
  pacman -S --noconfirm --needed mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-SDL2 mingw-w64-i686-fltk
  export CMAKE_GENERATOR="MSYS Makefiles"
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCPACK_GENERATOR=ZIP"
  export RUSTUP_HOME="$(cygpath -w ~/.rustup)"
  export CARGO_HOME="$(cygpath -w ~/.cargo)"
  export RUSTUP_INIT_ARGS="${RUSTUP_INIT_ARGS}-i686-pc-windows-gnu --default-host=i686-pc-windows-gnu"
  export RUN_INSTALL_TEST=false # no sudo
elif [[ "$CI_TARGET" == "mac" ]]; then
  brew install --HEAD ccache
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DCPACK_GENERATOR=Bundle"
  export BUILD_TOOL_ARGS="-- -j 4"
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

echo "## configure, build, package ##"
mkdir ci-build
cd ci-build
$CONFIGURE_CMD ..
cat ./CMakeCache.txt
$BUILD_CMD $BUILD_TOOL_ARGS
$BUILD_CMD --target package

command -v ccache && ccache -s

echo "## test ##"
if [[ "$RUN_TESTS" == "true" ]]; then
  if [[ "$RUN_INSTALL_TEST" == "true" ]]; then
    sudo $BUILD_CMD --target install
  fi
  $BUILD_CMD --target cargo-fmt-check
  $BUILD_CMD --target cargo-clippy
  $BUILD_CMD --target cargo-test
  ./ja2 -unittests
  ./ja2-launcher -help
  if [[ "$RUN_INSTALL_TEST" == "true" ]]; then
    sudo $BUILD_CMD --target uninstall
  fi
fi

echo "## publish ##"
for file in ja2-stracciatella_*; do
  echo "$file"
  if [[ "$file" == *".deb" ]]; then
    dpkg -c "$file"
  elif [[ "$file" == *".zip" ]]; then
    unzip -l "$file"
  elif [[ "$file" == *".dmg" ]]; then
    # based on https://ss64.com/osx/hdiutil.html
    hdiutil verify "$file"
    device=""
    while IFS=$'\n' read -r line; do
      echo "$line"
      if [[ "$line" != "/dev/"* ]]; then
        continue # expected <dev node><tab><content hint><tab><mount point>
      fi
      IFS=$'\t' read -ra arr <<< "$line"
      if [[ "$device" == "" ]]; then
        device="${arr[0]%"${arr[0]##*[![:space:]]}"}" # remove trailing whitespace
      fi
      if [[ "${arr[2]}" != "" ]]; then
        ls -laR "${arr[2]}"
      fi
    done <<< "$(hdiutil attach "$file")"
    hdiutil detach "$device"
  else
    echo "TODO list contents"
  fi
  if [[ "$PUBLISH_BINARY" == "true" ]]; then
    curl -v --retry 3 --connect-timeout 60 --max-time 150 --ftp-create-dirs -T "$file" -u $SFTP_USER:$SFTP_PASSWORD ftp://www61.your-server.de/$PUBLISH_DIR/
  fi
done

echo "## done ##"
