---
name: upgrade-sdl-version
description: Use this skill to upgrade the included SDL versions. Use it when explicitly asked to upgrade.
disable-model-invocation: true
---

Upgrade the SDL3 versions included in this project to the latest version. You can find the latest release in the GitHub project libsdl-org/SDL.

Upgrade the following files or directories

- dependencies/lib-sdl3/builder/CMakeLists.txt.in: Update the download url and hash
- dependencies/lib-SDL3-{version}-macos: Download and extract (with 7zip) the .dmg release file. Remove any HFS and hidden directories afterwards. Also remove any non macOS files in the framework directory (e.g. ios, tvos).
- dependencies/lib-SDL3-{version}-mingw: Download and extract the -mingw release file
- dependencies/lib-SDL3-{version}-VC: Download and extract the -VC release file

After upgrading, check that the directory structure matches the previous one. Do not expect any git repositories inside the lib-SDL3-{version} directories, they are the extracted release packages. The directory names should follow the above pattern. Remove the old lib-SDL3-{version} directories when you are done. Also update the toolchain-{}.cmake files to point to the new versions.
