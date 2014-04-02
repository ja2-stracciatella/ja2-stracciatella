# History of changes

## v0.12.275

- Fixed issue  #66: Bug when distributing skill points during IMP creation
- Fixed issue #114: Crash when dismissing Dead Soldier
- Fixed issue #116: SIGSEGV when trying to scroll the screen while displaying line of sight
- Fixed issue #117: Glitchy rendering of moving mercs while displaying LOS
- Fixed issue #118: Mortar and Grenade Launcher not working

## v0.12.265

- Fixed issue #111 Bug with externalized dealer inventory.
- Fixed issue #112 Another bug with externalized dealer inventory (Jake)
- Fixed issue #108 More documentation in externalized\readme.txt
- Fixed issue #113 JsonUtility_unittests.cc should not be compiled when WITH_UNITTESTS=0

## v0.12.258

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

## v0.12.189

- Fixed issue #59 (Inventory won't open on strategic screen if using a non-default resolution)
- Fixed issue #58 (Game crashing while entering some sectors)
- Fixed issue #57 (Deadlock leads to player turn without reseting the AP)
- Fixed issue #56 (laptop mode has problems with high resolutions)
- Fixed issue #53 (crash when saving the game)
- Fixed issue #52 (High res glitch with merc move menu)
- Fixed issue #51 (Broken text log after loading a saved game)
- Fixed issue #47 (High res glitch on loading game)

## v0.12.175

- On Linux and Mac OS X, SHIFT can be used instead of ALT to move backward

- Fixed crashes on Mac OS X

- Fixed issue #50 (Linking ja2 fails on Ubuntu 13.04)
- Fixed issue #48 (Unnamed civilians disappear)
- Fixed issue #44 (Ring fight bug prize money and invitation not triggered)
- Fixed issue #6  (Question mark instead of comma in Russian version)

## v0.12.159

- Saved games are now compatible between Windows and unix platforms (Linux,
  Mac OS X).  Saves made on Windows can be loaded on Linux and vise versa.
  All new saves are done in Windows format.  Old unix saves can still be
  loaded.

- Fixed issue #31 (Only fences in banks and cliffs section)
- Fixed issue #28 (Map editor doesn't work in Russian (not gold) version)
- Fixed issue #5  (Russian language is broken in VS build)

## v0.12.138

- Unit tests added
- Refined compilation process on Mac OS
- Fixed issue #26 (Dead bodies are not visible again)

## v0.12.99

- Fixed issue #23 (Dead bodies are not visible)
- Fixed issue #22 (Incorrect body position on non-standard resolutions)
- Fixed issue #21 (Game is really slow on Mac OS X)

## v0.12.93

- Map editor is now included into the game exe.
  Start it with command-like key '-editor' or '-editorauto'.
  Map editor requires Editor.slf to work.

- Fixed issue #18 (Crash on exiting full screen mode on Mac OS)

## v0.12.47

- Added support for high video resolutions.  For example, game can be
  started in 1024x768 mode like this: ja2.exe -res 1024x768
  Any reasonable resolution should work.

- Added compilation instructions for building the project on Linux,
  Windows, Mac OS 10.6 - 10.8, cross building on Linux for Windows.

- Added solution files for Visual Studio 10 Express, Xcode 4.2, 4.6

## v0.12.31

- Single exe can now work with many versions (localizations) of the game.
  There is no need anymore to compile binary for every version of the game.

- There is no need anymore to lower-case game resources on Linux.

# JA2-Stracciatella History of Changes

See in file Changelog
