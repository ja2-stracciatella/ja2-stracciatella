#!/usr/bin/env bash
#
# Publish package artifacts to Google Cloud Storage
# This script needs to be executed from a subdirectory within the repo that contains a finished build
#
# Required env variables (nightly and releases):
#   CI_REF - full reference of the current branch, tag, or pull request
#   GCLOUD_CREDENTIALS_KEY, GCLOUD_CREDENTIALS_IV, GCLOUD_CREDENTIALS_SALT - values to decrypt gcloud credentials

set -e

echo "CI_REF: $CI_REF"

if [[ "$PUBLISH_BINARY" != "true" ]]; then
  echo "Skipping upload of artifacts"
  exit 0
fi

if [[ "$PUBLISH_DIR" == "" ]]; then
  echo "PUBLISH_DIR env variable not set"
  exit 1
fi


# Bucket Name to upload to
BUCKET_NAME=ja2-builds

# Decrypt google cloud key using secret env variables
openssl aes-256-cbc -K $GCLOUD_CREDENTIALS_KEY -iv $GCLOUD_CREDENTIALS_IV -S $GCLOUD_CREDENTIALS_SALT -in ../.ci/gcloud-key.json.enc -out ./gcloud-key.json -d

# Select file to upload (currently only supports a single file)
export PUBLISH_FILENAME="$(echo ja2-stracciatella_*)"
if [[ "$CI_REF" = "refs/heads/nightly" ]]; then
  export PUBLISH_DIR="$PUBLISH_DIR/${PUBLISH_FILENAME%_*}/"
fi

# Login to the gcloud cli (need to redirect stderr to stdout, otherwise powershell might treat the command as failed)
gcloud auth activate-service-account --key-file=./gcloud-key.json 2>&1

# Upload files (need to redirect stderr to stdout, otherwise powershell might treat the command as failed)
echo "Uploading $PUBLISH_FILENAME to $PUBLISH_DIR";
gsutil cp $(pwd)/$PUBLISH_FILENAME gs://$BUCKET_NAME/$PUBLISH_DIR 2>&1
