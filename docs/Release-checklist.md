# Release Checklist

## Pre-release development

- [ ] Fix or postpone issues in the [milestone](https://github.com/ja2-stracciatella/ja2-stracciatella/milestone/6)
- [ ] Triage open pull requests and recent bugs
- [ ] Finalize choice of version number
  - [ ] Decide if this version should be 1.0 #443
- [ ] Add new contributors to contributors.txt
- [ ] Update changes.md (run docs/draft-changelog.sh to get started)
- [ ] Update man page if necessary

## Release candidate

- [ ] Update Version number to `-rc` and tag
- [ ] Mark the tag as a Github prerelease
- [ ] Runtime tests of prerelease packages
  - [ ] Linux
    - [ ] Ubuntu 20.04 x64
  - [ ] Windows
  - [ ] OS X

Runtime tests:

- IMP creation
- AIM and extending contracts
- Taking Drassen airport or more
- Buying from Bobby Ray's
- Visual inspection of laptop screens

If you want to build the packages manually:

    $ make clean
    $ make build-releases # to build all releases
    $ make build-win-release-on-linux # crosscompile Windows release on Linux
    $ make build-release-on-mac # build the mac bundle
    $ make build-debian-package # build the .DEB package


## Release

- [ ] Attach built versions to GitHub release
  - [ ] Linux, Windows
  - [ ] OS X
- [ ] Document new features on the website
- [ ] Make GitHub prerelease the release, update texts
- [ ] Announce
  - [ ] Write main announcement
  - [ ] Website download page (update frontmatter variables as needed)
  - [ ] Website news
  - [ ] Bear's pit
  - [ ] Moddb


## Post-release

- [ ] Set version to +1 and update README.md link
- [ ] Close this bug and GitHub milestone
- [ ] Create new milestone if none exists yet
