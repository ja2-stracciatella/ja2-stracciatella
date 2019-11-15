#!/usr/bin/env bash

set -x

echo "# set up environment"
if [[ "$GITHUB_REF" == "refs/pull/"* ]]; then
  export PULL_REQUEST=$(echo "$GITHUB_REF" | cut -d '/' -f 3)
  echo "-- PULL REQUEST ${PULL_REQUEST} --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="pull-requests/${PULL_REQUEST}"
  export VERSION_TAG="${PULL_REQUEST}pullrequest"
  export BUILD_TYPE="Debug"
elif [[ "$GITHUB_REF" == "refs/heads/nightly" ]]; then
  echo "-- NIGHTLY --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="nightlies"
  export BUILD_TYPE="ReleaseWithDebInfo"
  export VERSION_TAG="$(date +%Y%m%d)"
elif [[ "$GITHUB_REF" == "refs/tags/"* ]]; then
  echo "-- RELEASE --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="releases"
  export BUILD_TYPE="ReleaseWithDebInfo"
else
  echo "-- QUICK BUILD --"
  export PUBLISH_BINARY="false"
  export BUILD_SWITCHES=""
  export BUILD_TYPE="Debug"
fi
export BUILD_SWITCHES="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DVERSION_TAG=$VERSION_TAG $CI_BUILD_SWITCHES"
[[ "$BUILD_TYPE" != "Debug" ]] && export BUILD_SWITCHES="$BUILD_SWITCHES -DWITH_EDITOR_SLF=ON";
[[ "$SFTP_PASSWORD" == "" ]] && export PUBLISH_BINARY="false"
echo "$PUBLISH_BINARY"
echo "$PUBLISH_DIR"
echo "$BUILD_SWITCHES"

echo "# install dependencies"
which apt || alias apt="brew"
sudo apt update
sudo apt install cmake make g++ libsdl2-dev libboost-all-dev fluid libfltk1.3-dev fakeroot
if [[ "$CI_MINGW" == "true" ]]; then
  sudo apt install mingw-w64
  export DEFAULT_HOST_ARG=--default-host x86_64-pc-windows-gnu
fi
export DEFAULT_TOOLCHAIN_ARG=--default-toolchain=$(cat ./rust-toolchain)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- ${DEFAULT_HOST_ARG} ${DEFAULT_TOOLCHAIN_ARG} -y --profile minimal --component clippy rustfmt
export PATH=$PATH:$HOME/.cargo/bin
rustc -V
cargo -V
cargo clippy -- -V
cargo fmt -- -V
cmake --version
fakeroot -v

echo "# configure, build, test"
mkdir ci-build
cd ci-build
cmake $BUILD_SWITCHES ..
make
if [[ "$CI_MINGW" != "true" ]]; then
  sudo make install
  make cargo-fmt-test
  make cargo-clippy-test
  make cargo-test
  ./ja2 -unittests
  ./ja2-launcher -help
  sudo make uninstall;
fi

echo "# package, publish"
make package
for file in ja2-stracciatella_*; do
echo "$file"
[[ "$file" == *".deb" ]] && dpkg -c "$file"
[[ "$file" == *".zip" ]] && unzip -l "$file"
[[ "$file" == *".dmg" ]] && echo "TODO list contents"
[[ "$PUBLISH_BINARY" == "true" ]] && curl -v --retry 3 --connect-timeout 60 --max-time 150 --ftp-create-dirs -T "$file" -u $SFTP_USER:$SFTP_PASSWORD ftp://www61.your-server.de/$PUBLISH_DIR/
done
