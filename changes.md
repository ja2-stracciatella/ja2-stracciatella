# History of changes

## 0.15.0
- Feature: New logo is used as application icon
- Feature: Using the brothel in san mona is now possible (#76)
- Feature: Allow stacked item popup to have multiple lines.
- Feature: FreeBSD and OpenBSD build system improvements
- Bugfix: Fix water not being taken into account for movement noise
- Bugfix: Fix random crashes after AI-vs-AI autoresolve battles (#253)
- Bugfix: Talon cannot be loaded with two grenades anymore (#130)
- Bugfix: Mercs that end a turn without moving while running now leave running stance (#194)
- Bugfix: Helicopter shadow is now rendered all the way to the screen border on high resolutions (#261)
- Bugfix: Some interface elements were not rendered correctly in editor on high resolutions (#152)
- Bugfix: Repair can now be used in underground sectors (#152)
- Bugfix: Credit screen rendering was fixed for higher resolutions (#222)
- Bugfix: Some sounds sounded different from vanilla (#78)
- Bugfix: Broken contract button in strategic view (#80)
- Bugfix: Clipping issue with militia map (#88)
- Bugfix: Vertically center time compress text in its box
- Bugfix: Armor label position in strategy merc inventory
- Bugfix: Ammo label position in weapon detail view
- Bugfix: Only heal up Elliot if he is not already dead (#153)
- Bugfix: Patch from Thomas Moore to fix contract renewal interface freeze
- Bugfix: Position cash splitting interface correctly in strategic mode (#178)
- Bugfix: Cow is not a bloodcat
- Bugfix: Number of bloodcats on medium difficulty was the same as on hard
- Bugfix: Dont end up in a locked ui state when somebody leaves (#200)
- Bugfix: Fix corpses and items appearing in water (#179)
- Bugfix: Rare boxing crash when money is handed over (#199)
- Bugfix: crash when loading map (#143)
- Bugfix: End of meanwhile cutscene crash (#198)

## v0.14.1

- various build system changes

## v0.12.352

- In-game video is playable now. Thanks to Misanthropos and Greg Kennedy (author of libsmacker library)
- Fixed issue #1   (Double amount of money on new game start)
- Fixed issues #120, #124 (incorrect ammo type for CAWS and SPAS-15)
- Fixed issue #121 (Misplaced text after IMP creation)
- Fixed issue #128 (Segmentation fault on entering sector with enemies)
- Fixed issue #127 (Bobby Rays prices not getting updated)
- Fixed issue #129 (SegFault on loading a SavedGame)
- Fixed issue #131 (Unhandled exception with fall-from-roof animation)
- Fixed issue #134 (On high res subtotal is not displayed correctly)
- Fixed crash on leaving shop interface without closing attachment panel
- Shift is no longer used as replacement for Alt (Option) on Linux and Mac OS X. Now to move backward use: Option + Left button mouse (LBM) on Mac OS X
- Alt + LBM or Alt + Meta (WinKey) + LBM on Linux, depending on your desktop environment. This fixes issue #123 and reverts changes made for issue #40.

## v0.12.275

- Fixed issue #66  (Bug when distributing skill points during IMP creation)
- Fixed issue #114 (Crash when dismissing Dead Soldier)
- Fixed issue #116 (SIGSEGV when trying to scroll the screen while displaying line of sight)
- Fixed issue #117 (Glitchy rendering of moving mercs while displaying LOS)
- Fixed issue #118 (Mortar and Grenade Launcher not working)

## v0.12.265

- Fixed issue #108 (More documentation in externalized\readme.txt)
- Fixed issue #111 (Bug with externalized dealer inventory.)
- Fixed issue #112 (Another bug with externalized dealer inventory (Jake))
- Fixed issue #113 (JsonUtility_unittests.cc should not be compiled when WITH_UNITTESTS=0)

## v0.12.258

- some hardcoded data has been externalized, weapon characteristics for example. See files in 'externalized' directory.  This make JA2-Stracciatella more suitable for mod development.
- some hotkeys have been added.  They are not enabled by default. See externalized/game.json and externalized/readme.txt for details.
- Fixed issue #63  (Holding a two handed item, the second hand graphic is not remove in map inventory)
- Fixed issue #70  (Bug during imp creation)
- Fixed issue #77  (Graphical glitch when placing mercs before battle)
- Fixed issue #79  (Small issue in sector inventory)
- Fixed issue #82  (Hotkey requests)
- Fixed issue #85  (Another issue in sector inventory)
- Fixed issue #90  (Flower Service Buttons Not Selectable)
- Fixed issue #91  (Financial History Reset)
- Fixed issue #92  (NPC's speech text is in the wrong place)
- Fixed issue #103 (small typo in \externalized\readme.txt)
- Fixed issue #104 (Alt+o cheat is not working)
- Fixed issue #105 (Dissonance with externalized data)

## v0.12.189

- Fixed issue #47 (High res glitch on loading game)
- Fixed issue #51 (Broken text log after loading a saved game)
- Fixed issue #52 (High res glitch with merc move menu)
- Fixed issue #53 (crash when saving the game)
- Fixed issue #56 (laptop mode has problems with high resolutions)
- Fixed issue #57 (Deadlock leads to player turn without resetting the AP)
- Fixed issue #58 (Game crashing while entering some sectors)
- Fixed issue #59 (Inventory won't open on strategic screen if using a non-default resolution)

## v0.12.175

- On Linux and Mac OS X, SHIFT can be used instead of ALT to move backward
- Fixed issue #6  (Question mark instead of comma in Russian version)
- Fixed issue #44 (Ring fight bug prize money and invitation not triggered)
- Fixed issue #48 (Unnamed civilians disappear)
- Fixed issue #50 (Linking ja2 fails on Ubuntu 13.04)
- Fixed crashes on Mac OS X

## v0.12.159

- Saved games are now compatible between Windows and unix platforms (Linux, Mac OS X). Saves made on Windows can be loaded on Linux and vice versa. All new saves are done in Windows format. Old unix saves can still be loaded.
- Fixed issue #5  (Russian language is broken in VS build)
- Fixed issue #28 (Map editor doesn't work in Russian (not gold) version)
- Fixed issue #31 (Only fences in banks and cliffs section)

## v0.12.138

- Unit tests added
- Refined compilation process on Mac OS
- Fixed issue #26 (Dead bodies are not visible again)

## v0.12.99

- Fixed issue #21 (Game is really slow on Mac OS X)
- Fixed issue #22 (Incorrect body position on non-standard resolutions)
- Fixed issue #23 (Dead bodies are not visible)

## v0.12.93

- Map editor is now included into the game exe. Start it with command-like key '-editor' or '-editorauto'. Map editor requires Editor.slf to work.
- Fixed issue #18 (Crash on exiting full screen mode on Mac OS)

## v0.12.47

- Added support for high video resolutions. For example, game can be started in 1024x768 mode like this: ja2.exe -res 1024x768 - Any reasonable resolution should work.
- Added compilation instructions for building the project on Linux, Windows, Mac OS 10.6 - 10.8, cross building on Linux for Windows.
- Added solution files for Visual Studio 10 Express, Xcode 4.2, 4.6

## v0.12.31

- Single exe can now work with many versions (localizations) of the game. There is no need anymore to compile binary for every version of the game.
- There is no need anymore to lower-case game resources on Linux.

# JA2-Stracciatella History of Changes

See in file Changelog
