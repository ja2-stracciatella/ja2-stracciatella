# JA2-Stracciatella Continued

This is continuation of venerable JA2-Stracciatella project.

The goal of the project is to make Jagged Alliance 2 available on a wide
range of platforms, improve its stability, fix bugs and provide a
stable platform for mod development.

## Latest news

### 2014-xx-xx, version 0.12.xxx (not released yet)

- Fixed issue   #1 (Double amount of money on new game start)
- Fixed issue #121 (Misplaced text after IMP creation)
- Fixed issue #134 (On high res subtotal is not displayed correctly)
- Fixed issue #127 (Bobby Rays prices not getting updated)
- Fixed issue #131 (Unhandled exception with fall-from-roof animation)
- Fixed crash on leaving shop interface without closing attachment panel
  https://bitbucket.org/misanthropos/ja2-stracciatella/issue/1
- Fixed issue #129 (SegFault on loading a SavedGame)
- Fixed issue #128 (Segmentation fault on entering sector with enemies)
- Fixed issues #120, #124 (incorrect ammo type for CAWS and SPAS-15)
- Shift is no longer used as replacement for Alt (Option) on Linux and Mac OS X
  Now to move backward use:
    - Option + Left button mouse (LBM) on Mac OS X
    - Alt + LBM or Alt + Meta (WinKey) + LBM on Linux, depending on your desktop environment
  This fixes issue #123 and reverts changes made for issue #40.

### 2014-02-16, version 0.12.275

- Fixed issue  #66: Bug when distributing skill points during IMP creation
- Fixed issue #114: Crash when dismissing Dead Soldier
- Fixed issue #116: SIGSEGV when trying to scroll the screen while displaying line of sight
- Fixed issue #117: Glitchy rendering of moving mercs while displaying LOS
- Fixed issue #118: Mortar and Grenade Launcher not working

You can see the full list of changes in files changes.md and Changelog (for the original project).

## History of the Project

The original project was run by Tron since 2006.  He did an amazing job of
cleaning up JA2 sources and making them portable.  The work was massive too -
there are over *7000 commits* in the original svn repository
svn://tron.homeunix.org/ja2/trunk.  Unfortunately, the work on the project
ceased in 2010.  The [original project homepage](http://tron.homeunix.org/ja2)
is no longer available.  Some history can be found in [JA2-Stracciatella
Q&A](http://www.ja-galaxy-forum.com/board/ubbthreads.php/topics/186601/JA2_Stracciatella_Q_A.html)

## License

Unless specified explicitly in the commit message, all changes since `commit
8287b98` are released to the public domain.  All libraries in `_build/lib-*`
have their own licenses.

It is not known under which license Tron released his changes.  All we know,
the source codes were publicly available in his svn repository.

Original Jagged Alliance source codes were released by Strategy First Inc. in
2004 under Source Code License Agreement ("SFI-SCLA").  You can find the
license in file *SFI Source Code license agreement.txt*.

## Development

[![Build Status](https://travis-ci.org/gennady/ja2-stracciatella.svg?branch=master)](https://travis-ci.org/gennady/ja2-stracciatella)

### How to contribute

1. Fork it
2. Create a branch (git checkout -b my_feature_branch)
3. Commit your changes (git commit -am "description of your changes")
4. Push to the branch (git push origin my_feature_branch)
5. Create a pull requst from your branch into master

