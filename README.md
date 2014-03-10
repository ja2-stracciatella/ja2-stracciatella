# JA2-Stracciatella Continued

This is continuation of venerable JA2-Stracciatella project.

The goal of the project is to make Jagged Alliance 2 available on a wide
range of platforms, improve its stability, fix bugs and provide a
stable platform for mod development.

## Latest news

### 2014-xx-xx, version 0.12.xxx (not released yet)

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

### 2014-01-18, version 0.12.265

- Fixed issue #111 Bug with externalized dealer inventory.
- Fixed issue #112 Another bug with externalized dealer inventory (Jake)
- Fixed issue #108 More documentation in externalized\readme.txt
- Fixed issue #113 JsonUtility_unittests.cc should not be compiled when WITH_UNITTESTS=0

### 2013-11-04, version 0.12.258 has been released

- some hardcoded data has been externalized, weapon characteristics for
  example.  See files in 'externalized' directory.  This make
  JA2-Stracciatella more suitable for mod development.
- some hotkeys have been added.  They are not enabled by default.  See
  externalized/game.json and externalized/readme.txt for details.
- Fixed issue #63 (Holding a two handed item, the second hand graphic is not remove in map inventory)
- Fixed issue #70 (Bug during imp creation)
- Fixed issue #77 (Graphical glitch when placing mercs before battle)
- Fixed issue #79 (Small issue in sector inventory)
- Fixed issue #82 (Hotkey requests)
- Fixed issue #85 (Another issue in sector inventory)
- Fixed issue #90 (Flower Service Buttons Not Selectable)
- Fixed issue #91 (Financial History Reset)
- Fixed issue #92 (NPC's speech text is in the wrong place)
- Fixed issue #103 (small typo in \externalized\readme.txt)
- Fixed issue #104 (Alt+o cheat is not working)
- Fixed issue #105 (Dissonance with externalized data)

### 2013-07-28, version 0.12.189 has been released

- Fixed issue #59 (Inventory won't open on strategic screen if using a non-default resolution)
- Fixed issue #58 (Game crashing while entering some sectors)
- Fixed issue #57 (Deadlock leads to player turn without reseting the AP)
- Fixed issue #56 (laptop mode has problems with high resolutions)
- Fixed issue #53 (crash when saving the game)
- Fixed issue #52 (High res glitch with merc move menu)
- Fixed issue #51 (Broken text log after loading a saved game)
- Fixed issue #47 (High res glitch on loading game)

### 2013-04-29, version 0.12.175 has been released

- On Linux and Mac OS X, SHIFT can be used instead of ALT to move backward

- Fixed crashes on Mac OS X

- Fixed issue #50 (Linking ja2 fails on Ubuntu 13.04)
- Fixed issue #48 (Unnamed civilians disappear)
- Fixed issue #44 (Ring fight bug prize money and invitation not triggered)
- Fixed issue #6  (Question mark instead of comma in Russian version)

You can see the full list of changes in files changes.md and Changelog (for the original project).

## History of the Project

The original project was run by Tron since 2006.  He did an amazing job of
cleaning up JA2 sources and making them portable.  The work was massive too -
there are over *7000 commits* in the [original svn
repository](svn://tron.homeunix.org/ja2/trunk).  Unfortunately, the work on
the project ceased in 2010.

See the following resources if you want to learn more:

 * [Original project homepage](http://tron.homeunix.org/ja2)

 * [JA2-Stracciatella Q&A](http://www.ja-galaxy-forum.com/board/ubbthreads.php/topics/186601/JA2_Stracciatella_Q_A.html)

## License

### New work

Unless specified explicitly in the commit message, all changes since `commit 8287b98` are released to the public domain.

### Libraries

All libraries in `_build/lib-*` have their own licenses.

### Tron's changes

It is not known under which license Tron released his changes.  All we know,
the source codes were publicly available in his svn repository.

### Original source codes

Jagged Alliance source codes were released by Strategy First Inc. in 2004
under Source Code License Agreement ("SFI-SCLA").  You can find the license in
file *SFI Source Code license agreement.txt*.
