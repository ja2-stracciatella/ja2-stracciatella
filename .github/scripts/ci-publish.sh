#!/usr/bin/env bash
#
# Publish package artifacts to SFTP
#
# Requires the following environment variables:
#   CI_REF - full reference of the current branch, tag, or pull request
# Required for publishing (nightly and releases):
#   SFTP_USER - ftp username for uploads (secret)
#   SFTP_PASSWORD - ftp password for uploads (secret)

set -e
set -x

echo "## check environment ##"
if [[ "${CI_REF}" == "refs/heads/nightly" ]]; then
  echo "-- NIGHTLY --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="nightlies"
elif [[ "${CI_REF}" == "refs/tags/"* ]]; then
  echo "-- RELEASE --"
  export PUBLISH_BINARY="true"
  export PUBLISH_DIR="releases"
  # assumes that the version is already set up correctly
elif [[ "${CI_REF}" == "refs/pull/"* ]]; then
  export PULL_REQUEST=$(echo "${CI_REF}" | cut -d '/' -f 3)
  echo "-- PULL REQUEST ${PULL_REQUEST} --"
  export PUBLISH_BINARY="false" # secrets are not available in pull requests
else
  echo "-- QUICK BUILD --"
  export PUBLISH_BINARY="false"
fi

if [[ "${PUBLISH_BINARY}" == "true" && "${SFTP_PASSWORD}" == "" ]]; then
  echo "Upload credentials are not set up"
  exit 1
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
  if [[ "$PUBLISH_BINARY" == "true" ]]; then
    curl -v --retry 3 \
      --connect-timeout 60 --max-time 300 \
      --ftp-create-dirs -T "$file" -u $SFTP_USER:$SFTP_PASSWORD \
      ftp://www61.your-server.de/$PUBLISH_DIR/
  else
    echo "Skipped"
  fi
done

echo "## done ##"
