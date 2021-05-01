#!/usr/bin/env bash
#
# Build script for CI environments, assumes a posix-like host (linux or mac).
#
# Requires the following environment variables:
#   CI_REF - full reference of the current branch, tag, or pull request
#   CI_TARGET - target we are building for: linux/mingw/mac
# Unused environment variables:
#   CI_NAME - name of the job
#   CI_OS - host runner image: https://help.github.com/en/actions/automating-your-workflow-with-github-actions/virtual-environments-for-github-hosted-runners

set -e
set -x

echo "CI_REF: $CI_REF"
echo "CI_TARGET: $CI_TARGET"

echo "## check environment ##"
if [[ "${CI_REF}" == "refs/heads/nightly" ]]; then
  echo "-- NIGHTLY --"
  export BUILD_TYPE="ReleaseWithDebInfo"
  export VERSION_TAG="$(date +%Y%m%d)"
elif [[ "${CI_REF}" == "refs/tags/"* ]]; then
  echo "-- RELEASE --"
  export BUILD_TYPE="ReleaseWithDebInfo"
  # assumes that the version is already set up correctly
elif [[ "${CI_REF}" == "refs/pull/"* ]]; then
  export PULL_REQUEST=$(echo "${CI_REF}" | cut -d '/' -f 3)
  echo "-- PULL REQUEST ${PULL_REQUEST} --"
  export VERSION_TAG="${PULL_REQUEST}pullrequest"
  export BUILD_TYPE="Debug"
else
  echo "-- QUICK BUILD --"
  export BUILD_TYPE="Debug"
fi
export BUILD_CMD="cmake --build . --config ${BUILD_TYPE}"
export BUILD_TOOL_ARGS="-- -j 4"
export CONFIGURE_CMD="cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DVERSION_TAG=${VERSION_TAG}"
if [[ "${BUILD_TYPE}" != "Debug" ]]; then
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DWITH_EDITOR_SLF=ON"
fi

export RUN_TESTS=true
export RUN_RUST_CHECKS=false
export RUSTUP_INIT_ARGS="-y --no-modify-path --default-toolchain=$(cat ./rust-toolchain) --profile=minimal"
if [[ "$CI_TARGET" == "linux" ]]; then
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCMAKE_INSTALL_PREFIX=AppDir/usr -DEXTRA_DATA_DIR=../share/ja2"
  export RUN_RUST_CHECKS=true

elif [[ "$CI_TARGET" == "linux-mingw64" ]]; then
  # cross compiling
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-mingw.cmake -DCPACK_GENERATOR=ZIP"
  export RUN_TESTS=false

elif [[ "$CI_TARGET" == "mac" ]]; then
  export CONFIGURE_CMD="${CONFIGURE_CMD} -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain-macos.cmake -DCPACK_GENERATOR=Bundle"
  export BUILD_TOOL_ARGS="-- -j 4"

elif [[ "$CI_TARGET" == "android" ]]; then
  export CONFIGURE_CMD="true"
  export BUILD_CMD="../android/gradlew"
  export BUILD_TOOL_ARGS="-p ../android assembleDebug -PbuildDir=$(pwd)/ci-build"
  export RUN_TESTS=false
  export PACKAGE_NAME="ja2-stracciatella_$(./android/gradlew -q -p ./android projectVersion)-$VERSION_TAG+$(git rev-parse --short HEAD)_android.apk"
else
  echo "unexpected target ${CI_TARGET}"
  exit 1
fi

echo "## configure, build, package ##"
mkdir -p ci-build
cd ci-build
if [[ "$CI_TARGET" != "android" ]]; then
  $CONFIGURE_CMD ..
  cat ./CMakeCache.txt
fi
$BUILD_CMD $BUILD_TOOL_ARGS

echo "## test ##"
if [[ "$RUN_TESTS" == "true" ]]; then
  if [[ "$CI_TARGET" == "linux" ]]; then
    $BUILD_CMD --target install
  fi
  if [[ "$RUN_RUST_CHECKS" == "true" ]]; then
    $BUILD_CMD --target cargo-fmt-check
    $BUILD_CMD --target cargo-clippy
  fi
  $BUILD_CMD --target cargo-test
  if [[ "$CI_TARGET" == "linux" ]]; then
    ./AppDir/usr/bin/ja2 -unittests
    ./AppDir/usr/bin/ja2-launcher -help

    # Smoke test to check if the binary can run on older distros
    DISTRO_IMAGE="debian:10"
    PACKAGES="libfontconfig1 libx11-6 libsdl2-2.0.0 libfltk1.3 libfltk-images1.3"
    SETUP_COMMAND="apt-get update && apt-get -yq install $PACKAGES"
    docker run -v "$(pwd)/AppDir/usr/bin:/ja2" "$DISTRO_IMAGE" bash -c "$SETUP_COMMAND && /ja2/ja2 -help"

  else
    ./ja2 -unittests
    ./ja2-launcher -help
  fi
fi

if [[ "$CI_TARGET" == "linux" ]]; then
  $BUILD_CMD --target package-appimage
elif [[ "$CI_TARGET" == "android" ]]; then
  cp ./outputs/apk/debug/app-debug.apk "./$PACKAGE_NAME"
else
  $BUILD_CMD --target package
fi

if [[ "$CI_TARGET" == "linux" ]]; then
  $BUILD_CMD --target uninstall
fi

# print sccache cache statistics
echo "## sccache statistics"
command -v sccache &&
  sccache -s ||
  echo "sccache not installed"

echo "## done ##"
