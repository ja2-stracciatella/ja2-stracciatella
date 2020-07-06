#!/usr/bin/env bash
#
# Check package contents and publish to storage
# This script needs to be executed in the workspace that contains the ci-build directory
#
# Requires the following environment variables:
#   CI_REF - full reference of the current branch, tag, or pull request
# Required for publishing (nightly, pull-requests and releases):
#   GCLOUD_CREDENTIALS_KEY, GCLOUD_CREDENTIALS_IV, GCLOUD_CREDENTIALS_SALT - values to decrypt gcloud credentials

set -e

echo "CI_REF: $CI_REF"

echo "## check environment ##"
if [[ "${CI_REF}" == "refs/heads/nightly" ]]; then
  echo "-- NIGHTLY --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="nightlies"
elif [[ "${CI_REF}" == "refs/tags/"* ]]; then
  export RELEASE=$(echo "${CI_REF}" | cut -d '/' -f 3)
  echo "-- RELEASE --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="releases/$RELEASE"
  # assumes that the version is already set up correctly
elif [[ "${CI_REF}" == "refs/pull/"* ]]; then
  export PULL_REQUEST=$(echo "${CI_REF}" | cut -d '/' -f 3)
  echo "-- PULL REQUEST ${PULL_REQUEST} --"
  if [[ "$GCLOUD_CREDENTIALS_KEY" == "" ]]; then
    # secrets are not available for pull requests from forks
    export PUBLISH_BINARY="false"
  else
    export PUBLISH_BINARY="true"
    export PUBLISH_DIR="pull-requests/$PULL_REQUEST/"
  fi
else
  echo "-- QUICK BUILD --"
  export PUBLISH_BINARY="false"
fi

cd ci-build

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
      IFS=$'\t' read -ra arr <<<"$line"
      if [[ "$device" == "" ]]; then
        device="${arr[0]%"${arr[0]##*[![:space:]]}"}" # remove trailing whitespace
      fi
      if [[ "${arr[2]}" != "" ]]; then
        ls -laR "${arr[2]}"
      fi
    done <<<"$(hdiutil attach "$file")"
    hdiutil detach "$device"
  else
    echo "TODO list contents"
  fi
  ../.ci/upload-artifact.sh $file
done


echo "## done ##"
