#!/usr/bin/env bash

set -x
set -e

echo "#\n# prepare environment\n#"
if [[ "${GITHUB_REF}" == "refs/heads/nightly" ]]; then
  echo "-- NIGHTLY --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="nightlies"
  export BUILD_TYPE="ReleaseWithDebInfo"
  export VERSION_TAG="$(date +%Y%m%d)"
elif [[ "${GITHUB_REF}" == "refs/tags/"* ]]; then
  echo "-- RELEASE --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="releases"
  export BUILD_TYPE="ReleaseWithDebInfo"
elif [[ "${GITHUB_REF}" == "refs/pull/"* ]]; then
  export PULL_REQUEST=$(echo "${GITHUB_REF}" | cut -d '/' -f 3)
  echo "-- PULL REQUEST ${PULL_REQUEST} --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="pull-requests/${PULL_REQUEST}"
  export VERSION_TAG="${PULL_REQUEST}pullrequest"
  export BUILD_TYPE="Debug"
else
  echo "-- QUICK BUILD --"
  export PUBLISH_BINARY="false"
  export BUILD_SWITCHES=""
  export BUILD_TYPE="Debug"
fi
export BUILD_SWITCHES="-DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DVERSION_TAG=${VERSION_TAG}"
if [[ "${BUILD_TYPE}" != "Debug" ]]; then
   export BUILD_SWITCHES="${BUILD_SWITCHES} -DWITH_EDITOR_SLF=ON"
fi
if [[ "${SFTP_PASSWORD}" == "" ]]; then
  export PUBLISH_BINARY="false"
fi
if [[ "$CI_target" == "linux" ]]; then
  sudo apt update
  sudo apt install cmake make g++ libsdl2-dev libboost-all-dev fluid libfltk1.3-dev fakeroot
  export BUILD_SWITCHES="${BUILD_SWITCHES} -DCMAKE_INSTALL_PREFIX=/usr -DEXTRA_DATA_DIR=/usr/share/ja2 -DLOCAL_BOOST_LIB=ON -DCPACK_GENERATOR=DEB"
elif [[ "$CI_TARGET" == "mingw" ]]; then
  sudo apt update
  sudo apt install cmake make g++ libsdl2-dev libboost-all-dev fluid libfltk1.3-dev fakeroot mingw-w64
  export BUILD_SWITCHES="${BUILD_SWITCHES} -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake -DCPACK_GENERATOR=ZIP"
elif [[ "$CI_TARGET" == "mac" ]]; then
  sudo brew update
  sudo brew install cmake make g++ libsdl2-dev libboost-all-dev fluid libfltk1.3-dev fakeroot
  export BUILD_SWITCHES="${BUILD_SWITCHES} -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DCPACK_GENERATOR=Bundle"
else
  echo "unexpected target ${CI_TARGET}"
  exit 1
fi
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain=$(cat ./rust-toolchain) -y --profile minimal
export PATH=$PATH:$HOME/.cargo/bin
if [[ "$CI_TARGET" == "mingw" ]]; then
  rustup target add x86_64-pc-windows-gnu
else
  rustup component add rustfmt
  rustup component add clippy
  cargo clippy -- -V
  cargo fmt -- -V
fi
export BUILD_CMD="cmake --build . --config $BUILD_TYPE"
echo "$PUBLISH_BINARY"
echo "$PUBLISH_DIR"
echo "$BUILD_SWITCHES"
rustc -V
cargo -V
cmake --version
fakeroot -v

echo "#\n# configure, build, test\n#"
mkdir ci-build
cd ci-build
cmake $BUILD_SWITCHES ..
cat ./CMakeCache.txt
$BUILD_CMD
if [[ "$CI_TARGET" != "mingw" ]]; then
  sudo $BUILD_CMD --target install
  $BUILD_CMD --target cargo-fmt-test
  $BUILD_CMD --target cargo-clippy-test
  $BUILD_CMD --target cargo-test
  ./ja2 -unittests
  ./ja2-launcher -help
  sudo $BUILD_CMD --target uninstall
fi

echo "#\n# package, publish\n#"
$BUILD_CMD --target package
for file in ja2-stracciatella_*; do
  echo "$file"
  if [[ "$file" == *".deb" ]]; then
    dpkg -c "$file"
  elif [[ "$file" == *".zip" ]]; then
    unzip -l "$file"
  else
    echo "TODO list contents"
  fi
  if [[ "$PUBLISH_BINARY" == "true" ]]; then
    curl -v --retry 3 --connect-timeout 60 --max-time 150 --ftp-create-dirs -T "$file" -u $SFTP_USER:$SFTP_PASSWORD ftp://www61.your-server.de/$PUBLISH_DIR/
  fi
done

echo "#\n# done\n#"
