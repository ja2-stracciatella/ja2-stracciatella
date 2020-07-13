# Pre-release
* triage newer bugs and open PRs (check the milestone if one exists)
* fix, revert and delay the release or merges if necessary
* test a clean build (handled by travis)

## Runtime check
* IMP creation
* AIM and extending contracts
* taking Drassen airport or more
* buying from Bobby Ray's
* visual inspection of laptop screens

# Release
* update changes.md (run docs/draft-changelog.sh to get started)
  * update man page if necessary
* bump version
* github release + tags

## Build releases for distribution
To build all releases:
    $ make clean
    $ make build-releases

###  Windows release on Linux: 
 
    $ make clean 
    $ make build-win-release-on-linux 
 
### On Mac: 
    $ make clean 
    $ make build-release-on-mac 
 
### Debian packages: 
For the current architecture:
      $ make build-debian-package

## Announcements
* website
* README.md
* Bear's pit
* moddb
