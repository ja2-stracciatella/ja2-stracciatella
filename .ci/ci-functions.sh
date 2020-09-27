## Setup related functions

unix-install-rustup () {
    curl --proto '=https' --tlsv1.2 -sSfL https://sh.rustup.rs | sh -s -- -y --default-toolchain=$(cat ./rust-toolchain)
    # Additional toolchains can be passed which are then installed
    for var in "$@"
    do
        rustup target add $var
    done
}

linux-install-sccache () {
    local SCCACHE_VERSION="0.2.13"
    local SCCACHE_LINUX_PACKAGE="sccache-${SCCACHE_VERSION}-x86_64-unknown-linux-musl"

    curl -sSfL "https://github.com/mozilla/sccache/releases/download/${SCCACHE_VERSION}/${SCCACHE_LINUX_PACKAGE}.tar.gz" | sudo tar zx --strip-component=1 -C /usr/bin/
}

linux-install-via-apt-get () {
    sudo apt-get -yq update
    sudo apt-get -yq install $@
}

linux-install-google-cloud-sdk () {
    export CLOUDSDK_CORE_DISABLE_PROMPTS=1

    curl https://sdk.cloud.google.com | bash
    source $HOME/google-cloud-sdk/path.bash.inc
}

linux-install-appimage-build-tools () {
    sudo curl -sSfL -o /usr/bin/linuxdeploy https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    sudo chmod +x /usr/bin/linuxdeploy
    sudo curl -sSfL -o /usr/bin/appimagelint https://github.com/TheAssassin/appimagelint/releases/download/continuous/appimagelint-x86_64.AppImage
    sudo chmod +x /usr/bin/appimagelint
}

macOS-install-via-brew () {
    brew install $@
}

macOS-install-via-brew-cask () {
    brew cask install google-cloud-sdk
}

windows-install-via-chocolatey () {
    choco install $@
}

windows-install-rustup () {
    # Difference to the Unix version: This only installs the one toolchain we pass in
    curl -sSf -o rustup-init.exe https://win.rustup.rs/
    ./rustup-init.exe -y --default-toolchain $(cat ./rust-toolchain)-$1
    export PATH="$PATH;$USERPROFILE\.cargo\bin"
}

windows-install-google-cloud-sdk () {
    export CLOUDSDK_CORE_DISABLE_PROMPTS=1
    local ARCHIVE_PATH="$HOMEPATH\google-cloud.zip"
    local UNZIP_PATH="$USERPROFILE"
    curl -sSf -o "$ARCHIVE_PATH" https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-sdk-299.0.0-windows-x86.zip
    unzip "$ARCHIVE_PATH" -d $UNZIP_PATH -qq
    export PATH="$PATH;$UNZIP_PATH\google-cloud-sdk\bin"
}

