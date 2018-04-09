# History of changes

## 0.16.1

- Bugfix: Fix crash during AI turn in Meduna (#495)
- Bugfix: Prevent saving during stopwatch (#661)
- Bugfix: Fix crash during load screen with RUSSIAN data files (#695)
- Launcher: Change label "JA2 Data Directory" to "JA2 Game Directory" (#693)


## 0.16.0

- Bugfix (vanilla): Do not increase shots fired statistic without a live target (#622)
- Bugfix (vanilla): Fix Bobby Ray's popup 'Out of Stock' appearing again when trying to leave the website (#534)
- Bugfix (vanilla): Fix gas on a roof can damage the merc underneath the roof (#477)
- Bugfix (vanilla): Fix open doors can block grenades (#94)
- Bugfix (vanilla): Fix smoke/gas spreads over roof edge (#600)
- Bugfix (vanilla): Fix climbing makes mercs visible at night (#402)
- Bugfix (vanilla): Fix throwing knife flying animation messed up (#395)
- Bugfix (vanilla): Fix helicopter is ignored by pathfinding (#317)
- Bugfix (vanilla): Fix area select rectangle not removed if interrupted (#550)
- Bugfix (vanilla): Fix replace non-existing corpse animation (#526)
- Bugfix (vanilla): Fix replace reference to non-existing flame animation (#525)
- Bugfix (vanilla): Fix doorcr_b.wav sound file exception (#520)
- Bugfix (vanilla): Fix game crash when inserting ceramic plates (#516)
- Bugfix (vanilla): Fix unconscious merc gains exp for bullet avoidance (#163)
- Bugfix (vanilla): Fix free disk space is calculated wrong (#413)
- Bugfix: Fix game speed is generally slow (#99)
- Bugfix: Don't crash if we don't have a target for punching (#627)
- Bugfix: Don't crash in SoundLoadDisk() on Arch Linux with SDL2 2.0.6 (#608)
- Bugfix: Fix sound memory limit hit after upgrading to SDL 2.0.7 (#609)
- Bugfix: Fix the rust cli parser does not accept relative paths (#559)
- Bugfix: Fix getting "Reading from file failed" when fast forwarding time after savegame load (#528)
- Bugfix: Fix crash in laptop AND huge memory footprint (#68)
- Bugfix: Fix adding money with '+' key cheat not working (#426)
- Bugfix: Fix hit by grenade issues (#247)
- Bugfix: Fix MERCS can punch through open doors (#595)
- Bugfix: Fix attack for 0 AP (#192)
- Bugfix: Alt+o cheat damages bloodcats now (#583)
- Bugfix: Passive bloodcat ambush fix (#584)
- Bugfix: Fix when enemy is noticed, no AP for action may be taken (#188)
- Bugfix: Fix attacking teammate(dialogue) on roof puts him through the roof (#567)
- Bugfix: Fix mysteriously disappearing APs when stealing (#191)
- Bugfix: Fix position of Mercenary task orders in Tactical screen (#537)
- Bugfix: Fix and enhance cli switch parsing (#515)
- Bugfix: Fix maximum militia Message at the wrong position (#540)
- Bugfix: Fix build fails if there is a space in the directory name (#529)
- Bugfix: Shrink gfKeyState (alternative) (#523)
- Bugfix: Teach (Estimate)ActionPoints about KID_SKIPPING (#502)
- Bugfix: Fix segfault in Blt8BPPDataTo16BPPBufferTransZTranslucent when rendering smoke (#468)
- Bugfix: Remove AI deadlocks (#466)
- Bugfix: Fix experience gain not acknowledged properly (#398)
- Bugfix: Fix minimap flickering on selected merc (#202)
- Bugfix: Fix current merc in minimap flickering (#452)
- Bugfix: Check for time accelleration properly when checking for end of turn (#451)
- Bugfix: Fix accelerated bleeding when returning to realtime (#160)
- Bugfix: Fix graphical glitch in main menu after quitting a game (#106)
- Bugfix: Fix menu bug when ending a game (#72)
- Bugfix: Fix can't type IMP code after saving via Save screen (#419)
- Bugfix: Fix issues with text inputs (#429)
- Bugfix: Fix game crashes during enemy turn (#415)
- Bugfix: Fix description box missing when opening "Deposit/Withdraw money" widget in Tactical screen (#379)
- Bugfix: Fix shipping cost BR not redrawn properly (#394)
- Bugfix: Fix bad Performance in OS X (#319)
- Bugfix: Fix autoresolve crash (#381)
- Bugfix: Only check retreat conditions for valid sectors (#374)
- Bugfix: Enable assertive asserts only on debug builds (#373)
- Bugfix: Show version number in main menu (#359)
- Bugfix: Fix minor debug page fault (#337)
- Bugfix: Fix undefined behaviour in LOS.cc (#342)
- Bugfix: Leave running stance if not moving (#194)
- Enhancement (vanilla): Fix merc forgets his stance after jumping on a roof (#84)
- Enhancement (vanilla): Fix selling attachments to Tony - strange prices (#423)
- Enhancement (vanilla): Add yellow or green star for attached Talon (#424)
- Enhancement: Add extra item combinations (#623)
- Enhancement: Unify hotkeys (#626)
- Enhancement: Integrate vxx's work (#307)
- Enhancement: Initial Launcher Implementation (#548)
- Enhancement: Allow resizing of game window (#555)
- Enhancement: Move ja2.ini to json (#291)
- Enhancement: Set options in ja2.ini (#298)
- Enhancement: Change name of the user folder (#164)
- Enhancement: Implement automated nightly builds (#442)
- Enhancement: Add Appveyor CI (#440)
- Enhancement: Add comments explaining game.json variables (#401)
- Enhancement: Add new Game Mode: Dead is Dead (#308)
- Enhancement: Make music modable to increase music variety (#386)
- Enhancement: Update to SDL2 (#216)
- Enhancement: Support Tribsoft's Linux port (#365)
- Enhancement: Introduce cmake build system (#354)
- Enhancement: Integrate gui_extras (#339)
- Enhancement: Implement uninplemented stuff (#264)
- Enhancement: Add “IMP Quiz Honest Answers” mini mod (#303)
- Enhancement: Simplify moving to the next sector in tactical view (windowed mode) (#314)
- Enhancement: Add “Generous Rebels” mini mod (#305)
- Maintenance: Explicitly initialize MovementMode (#635)
- Maintenance: Fix “Release mode” linking and deb file permissions (#634)
- Maintenance: Make CIs automatically build release packages (#632)
- Maintenance: Misc cleanup (#625)
- Maintenance: Normalize whitespace (#556)
- Maintenance: CMake cleanup (minimal version) (#482)
- Maintenance: Remove #ifdef JA2 code (#471)
- Maintenance: Remove #ifdef JA2TESTVERSION code (#472)
- Maintenance: Remove #ifdef JA2BETAVERSION code (#470)
- Maintenance: Reorganize source tree (#455)
- Maintenance: Update rapidjson to 1.1.0 and add comments to game.json (#411)
- Maintenance: Externalize lib boost (#229)
- Maintenance: Update boost to 1.61 (#326)
- Maintenance: Rework debug logging (#60)
- Maintenance: Update smacker.c (#310)
- Maintenance: Remove WITH_MODS ifdef (#322)
- Maintenance: Rework logging (#250)

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
(this is the old changelog of the original project)

(r7072) Fix a vanilla glitch: When scrolling back in an email, the email delete window was not redrawn properly.
(r7071) Fix a glitch in r4977: The yes/no buttons in the email delete window could not be clicked, when the email had more than one page.
(r7070) Fix a vanilla bug: Darren used the wrong quote for the first fight on a day, if there were fights on previous days.
(r7069) Fix a vanilla bug: The sum of fractions for troops and elites was more than 100% for one army composition.
(r7066) Fix a bug in r3615, which caused an assertion failure/null-pointer access when removing a dead merc via the contract menu.
(r7062) Fix a vanilla bug: When a player group arrives in a sector, in some cases it was checked on the wrong (out of bounds) side, whether enemies noticed them.
(r7056) Fix a vanilla bug, which prevented enemies from climbing roofs.
(r7055) Fix a vanilla bug: When a soldier changed team (e.g. getting hostile), he lost his camouflage.
(r6933) Plug a vanilla one-time memory leak.
(r6926) Fix a vanilla bug: Due to a logic bug multiple terrorists could end up in the same sector.
(r6924) Fix a vanilla glitch in the editor: Pressing page up/down while an item is selected and the options menu is active caused some widgets of the items menu to be shown.
(r6864) Fix a vanilla glitch: Too few items could be drawn, if some items on the ground were not visible.
(r6777) Fix a vanilla glitch: When the SAM control in Meduna was rebuilt, a graphic with wrong orientation was used.
(r6758) Fix a vanilla out of bounds array access, when starting the creature quest.
(r6650) Fix a vanilla bug in the editor: When selecting a miner, he was turned into an elite soldier.
(r6622) Fix a vanilla glitch: While a merc is on a roof and planning to perform an action (usually giving first aid), the highlighted tile was incorrectly drawn on the floor level.
(r6606) All directory names (especially data/) are lowercase now.
(r6601) Fix a vanilla out of bounds array access, which happened when entering Bobby Ray's shipment page.
(r6418) Fix a vanilla bug: Mercs should train health a bit by travelling by foot, but due to an error in the calculation this did not work.
(r6404) Fix a vanilla bug: All soldiers had an additional need for sleep as if seriously wounded, even when at perfect health.
(r6298) Improve the vanilla behaviour of the scroll bar in the help screen: Now moving the mouse slowly does not cause it to hang anymore and the relative position of the mouse cursor to the bar is fixed.
(r6293) Enable scrolling in the help screen with the mouse wheel.
(r6270) Fix a vanilla glitch: Disable the prev/next buttons in the personnel screen not only when there is exactly one merc in the list, but also if there is none.
(r6253) Fix a vanilla glitch in the editor: The text for the brothel siren trap was missing.
(r6152) Instead of displaying nothing (vanilla) or asserting (till now) show "Unknown" if a weapon has an imprint by a nameless enemy soldier.
(r6147) Fix a vanilla out of bounds array access (now a null-pointer access) when not all three boxers are there.
(r6130) Fix a vanilla glitch: When switching between current and past mercs in the personnel screen, properly disable/enable the next/previous merc buttons.
(r6127, r6129) Fix a vanilla glitch: Prevent selecting an empty slot when looking at past mercs.
(r6126) Fix a vanilla glitch: Only enable the next page button in the personnel screen when there are more than 20 past mercs.
(r6125) Fix vanilla bugs (wrong portraits, out of bounds access) for displaying the portraits of past merc in the personnel screen when there are more than 20 past mercs.
(r6071) Fix a vanilla glitch: A merge recipe for the guardian vest was missing.
(r6043) Fix a vanilla display glitch in debug mode: When turning off the Z-buffer display, the screen was not redrawn.

v0.12.1 - 2009-06-15
(r6011) Fix a bug in r5121 which prevented hiring mercs from AIM.
(r6005) Fix a vanilla buffer overflow when loading/saving savegames.

v0.12 - 2009-06-12 - Walnut
(r5970) Slightly improve the cursor key handling in the load/save screen.
(r5852) Fix a vanilla out of bounds array access when a wall gets damaged.
(r5787) Fix a vanilla bug: Throwing a knife costed one point less than it should.
(r5732) Fix a vanilla display glitch: The item tooltip for the auto rocket rifle should not show "(rocket)", just like for the rocket rifle.
(r5730) Fix a vanilla display glitch: Do not print "0" in the item description box, when a gun uses no ammunition (e.g. LAW).
(r5721) Fix a vanilla bug: When an enemy soldier is looking for items in one case not all weapons, which have a fingerprint ID, were considered correctly.
(r5719) Fix a vanilla bug: When an enemy soldier is looking for items and finds a non-helmet/vest/leggings piece of armour it was incorrectly considered for pickup.
(r5673) Adjust the char-to-glyph translation table for the French version, so "i" with circumflex is displayed correctly.
(r5651) Fix a vanilla bug: Closing the laptop, while a page in the browser is loading, caused artifacts (not deleted GUI elements, accessing not loaded graphics).
(r5621, r5623) Always make the tooltips of the contract button and the inventory toggle region available, even if they are deactivated.
(r5553) Fix a bug in r1791 which caused a null-pointer access when loading a savegame which starts in tactical mode and all mercs are busy.
(r5494) Now the game runs on (little-endian) 64bit architectures (AMD64), too.
(r5482) Fix a bug in r3969, which prevented reentering a sector after fleeing from a battle there in the Windows version.
(r5463) Fix a vanilla glitch: The width of tooltip boxes with hotkey highlights in the text did not properly fit the width of the text.
(r5429) Fix a bug in r3369, which caused that the first merc leaving the helicopter was not placed in the center of the map.
(r5422) Improve the overhead map a bit: Print the name of the soldier the mouse is over last, so it is not covered by markers or the name of the selected soldier.
(r5421) Fix a vanilla glitch: In the overhead map the name of soldiers on a roof was not properly printed over their markers.
(r5418) Fix a vanilla glitch: In the overhead map for sectors with elevated land (e.g. D13) the name of soldiers was not properly printed over their markers.
(r5416) Fix a vanilla glitch: In the overhead map for sectors with elevated land (e.g. D13) the mouse cursors did not align with the displayed soldier and item markers.
(r5277) Fix a vanilla bug: When trying to hire a merc while the team is full, the option to buy his equipment is lost.
(r5275) Fix a vanilla bug: When closing the laptop via the ESC key while visiting the IMP site the "X" button for closing was not removed.
(r5258) Allow all printable chars in text input fields. E.g. umlauts and cyrillic letters are allowed in savegame descriptions now.
(r5250) Fix a vanilla buffer overrun, which occurs when entering text into a text input field.
(r5223) Fix incorrectly encoded texts in the English version when loading them from data files. A few sentences by Malice contain accented letters, which were not displayed correctly.
(r5213) Allow all printable chars in the name and nickname of IMPs. E.g. umlauts and cyrillic letters are allowed now.
(r5207) Fix a vanilla glitch: The highlighted text of a text input field was deleted, when a non-character key (like cursor up) was pressed.
(r5161) Fix a vanilla bug: Only consume one regeneration booster at a time, even when multiple are selected (only the effects of one are applied).
(r5149) Plug a vanilla memory leak.
(r5143) Fix a vanilla graphics glitch: The battle sector locator on the strategic map was not fully drawn.
(r5142, r5460, r5464, r5748) Improve German translation.
(r5138) Fix a vanilla bug: In turnbased mode it was not possible for a merc to place an explosive on the tile he is currently at.
(r5082) Fix a vanilla glitch: In the Russian (Agoniya Vlasti) version the plaque on Bobby Ray's page was missing, because the image file is named different than for other languages.
(r5081) Fix a vanilla bug, which allowed recruiting AIM mercs for two weeks at the price for one day.
(r5072, r5073, r5074) Plug a vanilla memory leak (now a null-pointer access) when loading a new sector and entering the tactical screen from the map screen.
(r5051) Make the whole keypad useable by mapping all keypad keys to normal keys, e.g. keypad enter behaves the same as the return key.
(r5027) Fix a glitch in r5021 which caused wrong display in the file and email viewers.

v0.11 - 2009-01-08 - Liquorice
(r5023) Make the info box left of the item selection in the editor a bit more useful: Remove the static "Status Info Line [12345]" text and do not overflow the box with the item name.
(r5005) Fix a vanilla display glitch in the editor: Now always draw a shadow for the name, health, slot and waypoints for mercs instead of doing it randomly.
(r4977) Enable scrolling through emails with the mouse wheel.
(r4949) Enable scrolling of the email list with the mouse wheel.
(r4974) Minor improvement of the Dutch translation.
(r4959) Fix vanilla inconsistencies between the displayed file list and the clickable regions in the file viewer.
(r4956) Fix a vanilla display glitch in the file viewer (at least in the German version): MOM's portrait was displayed at a different height compared to all other portraits.
(r4954) Enable scrolling with the mouse wheel in the file viewer.
(r4953) Fix a vanilla graphics glitch in the laptop file viewer: The background of the content area was not completely filled.
(r4949) Enable scrolling of the finance log with the mouse wheel.
(r4942) Enable scrolling of the history log with the mouse wheel.
(r4939) Enable scrolling of Bobby Ray's items list with the mouse wheel.
(r4936) Remove the duplicate weight display for weapons and armour from Bobby Ray's.
(r4929) Enable scrolling the dealer inventory with the mouse wheel.
(r4925) Fix a vanilla graphics glitch in the editor: The right and lower border of popup menus was missing.
(r4923) Allow word separation at hyphens when formatting text.
(r4864) Fix a vanilla bug: In an ambush all mercs were moved to the centre of their sectors, even if they were in different sectors than the ambush.
(r4857) Fix a glitch in r4280: The wrong dying sound was played for animals and monsters.
(r4856, r4931, r4934, r4935, r4974) Correct vanilla mistranslations and improve German translation.
(r4855) Fix a glitch in r3732, which caused that the last page of the history was missing, if there is a single entry on that page.
(r4849) Fix a vanilla out of bounds array access, when a soldier puts an item in his primary hand, which is not a weapon.  This caused burst mode to be reset, when reloading a gun.
(r4837) Fix a bug in r894, which caused the list of available skills in the IMP character generator to be filtered improperly.
(r4833) Fix a vanilla bug: The helicopter was missing a piece when landed in Estoni.
(r4832) Fix error in r1643, which lead to a null-pointer access in the personnel screen, when there are mercs with some stat values of 0.
(r4831) Fix a vanilla bug in the editor: The Z coordinate of the map was not set for surface maps.
(r4830) Fix a vanilla glitch: Some text in the editor taskbar sometimes got a shadow.
(r4829) Fix a vanilla bug, which made it possible to hire a merc, if there is a hated merc on the team, though he should refuse for other reasons.
(r4768) Enable scrolling the item pickup list with the mouse wheel.
(r4765) Fix a vanilla bug, when trying to remove more than $20.000 from the player's account by clicking on $100 or $10 while trading with a shopkeeper (broken display).
(r4722) Restore path cancellation when right clicking on the destination column in the map screen, which was broken in r3592.
(r4671) Fix a vanilla glitch: Some dying sounds were not played because of inconsistent file names.
(r4663) Fix a vanilla glitch: Do not add two corpses for mercs died in auto resolve.
(r4600, r4817) Plug vanilla memory leak when loading savegames.
(r4597) Plug vanilla memory leak in the laptop file viewer.
(r4487) Fix a vanilla endless loop when an enemy tries to throw a grenade straight up.
(r4426) Fix a vanilla user after free when leaving a message on a merc's answering machine and the greeting message has ended.
(r4366) Fix a vanilla crash in the editor when looking at the items summary and there are no items in the current sector.
(r4361) Enable scrolling the items lists in the editor with the mouse wheel.
(r4360) Enable scrolling the terrain and building graphics lists in the editor with the mouse wheel.
(r4359) Fix a vanilla buffer underrun, which occurs when displaying a multiline text containing a word longer than the maximum line width.  The most prominent example is the item view in the map editor with the German data files ("Leichte Panzerabwehrwaffe").
(r4357) Make the map editor available via the command line parameter -editor.
(r4303) Fix a vanilla bug, which restored a wrong amount of breath points at the start of the turn.

v0.10 - 2008-06-02 - Caramel
(r4277) Fix a vanilla glitch regarding uneven distribution when playing some merc battle sounds, in one case even playing the wrong sound.
(r4275) Fix a bug in r668, which caused wrong title bars to be displayed for some web pages.
(r4269) Fix a vanilla out of bounds array access when somebody gets hit.
(r4200) Fix a vanilla bug, which caused stacked items to be too heavy.
(r4169) Enable scrolling the sector inventory with the mouse wheel.
(r4040) Fix several vanilla inconsistencies when placing the assignment dropdown menus.
(r3952, r3955, r3957, r3965, r3966, r3967, r3968, r3969, r3972) Plug vanilla memory and file leaks (mostly in error handling).
(r3920) Fix a vanilla string buffer overrun, occuring in the French and Italian translations, when displaying the turn message at the top in combat.
(r3919, r4161) Fix vanilla bugs, which accessed uninitialised data.
(r3918) Fix a bug in r3226, which broke saving/loading savegames in the Windows version.
(r3915) Fix a vanilla bug, which made items invisible, end up on the wrong level (floor/roof) or hover above the ground, when shuffling them in the sector inventory.
(r3914) Fix a bug in r2162, which could cause less militia to appear in a sector than there should.
(r3908) Fix a vanilla bug, which prevented the army from attacking San Mona, when the player has defense in Grumm.

v0.9 - 2008-04-22 - Blueberry
(r3852) Fix a vanilla off-by-one bug: The first .jsd found, was skipped, which results in a randomly missing corpse graphic.
(r3819) Plug vanilla memory leak, which occurs when receiving stuff from Bobby Ray.
(r3806) Fix a vanilla glitch: The tooltip area for pros/cons of a weapon was wider than the item description box in the map screen.
(r3757) For OSX get the path to the game data from the bundle.
(r3755) Remove caching of shade tables on disk.
(r3721) Fix a vanilla bug: When starting a new game, the date of the first history entry was set to the date of the last loaded game.
(r3708) Fix a vanilla bug: Items on top and in structures (tables, crates, etc.) could not be selected in the overhead map.
(r3704) Enable toggling between fullscreen and window mode at runtime by pressing ALT+RETURN.
(r3675) Fix a vanilla display glitch: Set the name of crows to "CROW" instead of "CIVILIAN", so the proper name is displayed in the overhead map.
(r3673) Fix vanilla display glitches when drawing the list of names of items on the ground: Properly clamp the list to the view area and prevent incorrect display, when invisible items are present.
(r3668) Fix vanilla displacement of people and item markers in the overhead map for smaller maps (like the rebel basement).
(r3638) Fix a vanilla out of bounds array access.
(r3598) Fix a vanilla memory leak when loading savegames.
(r3592) Fix vanilla bugs (different destinations within one squad, combats with no participants) when right clicking the the name, location or destination columns to abort movement planning mode.
(r3546) Ensure that taking screenshots does not overwrite older screenshots.
(r3529) Fix a bug caused by change of behaviour in r2719, which lead to truncation of the finance records.
(r3489) Fix a vanilla out of bounds access (which is a NULL-pointer access since r2625) when looking at the sector inventory and no merc is on the team.
(r3444, r3445, r3600, r3605) Fix several vanilla graphical deviations when drawing items.
(r3433) Fix a vanilla display bug of the remaining time on Slay's contract.  It was always 2982616d 4h.
(r3400) Fix a vanilla bug, which under rare circumstances caused items to disappear when changing sectors and looking at the sector inventory.
(r3340) Fix a bug, which caused corrupted savegames when saving pseudo objects.  Most common symptom is corrupted savegames after receiving the "donation" from Armand.
(r3325, r3327) Plug vanilla memory leaks which occur during strategic movement planning.
(r3274) Fix a vanilla bug: Only consume one adrenaline booster at a time, even when multiple are selected (only the effects of one are applied).
(r3214) Fix an error in r2460 which caused that only one merc at a time could be put into a vehicle.
(r3192) Fix a vanilla bug, which allowed erroneously freeing POWs by giving multiple mercs an assignment at once along with the POW.
(r3104) Fix a crash caused by r1766 when using alternate give mode (right-click) to hand an item to another merc in tactical mode.
(r3086) Fix a vanilla bug when playing a non-gun nut game: When replacing weapons and ammunition for this mode, sometimes ammunition was erroneously removed.  This caused that Vicky had no extra ammunition at start.

v0.8 - 2008-02-20 - Lemon
(r3064) Add support for Maemo, so the menu button acts as modifier key to perform right clicks.
(r3059) (Partially) fix a vanilla bug: Mercs could perform actions while looking into the opposite direction, when they were moving backwards just before.  In most cases this was harmless, but in some instances (climbing down a roof, jumping over a fence) the merc would end up in the wrong spot.
(r3052) Minor improvement of the English translation.
(r3028) Fix a vanilla glitch, which prevented showing the assignment menu by right-clicking in tactical mode, when a soldier was targeted, but no soldier was selected.
(r3027) Fix a vanilla bug, which bogusly allowed giving vehicles other assignments (like "train") by right-clicking them in tactical mode.
(r3023) Fix a vanilla glitch: When a random soldier in a vehicle should say a battle sound maybe none was said though there were soldiers in the vehicle.
(r2984) Fix a vanilla memory leak when handling delayed/stolen items from Bobby Ray's.
(r2931) Fix a vanilla out of bounds array access (which now is a NULL-pointer access) when clicking on a vehicle in the tactical screen.
(r2896) Fix a vanilla out of bounds array access (which now is a NULL-pointer access) when loading a savegame, which starts in tacical, and opening the assignment menu of a merc.
(r2832) Fix a bug in the sound system: Some sounds are sampled at 44kHz, but the sound system uses 22kHz as playback rate, so resample these sounds before playing.
(r2678) Fix a crash, which is caused by a vanilla mixup of SoldierID and ProfileID, in the insurance contract screen when there are no insurable mercs.
(r2609, r2840, r2886, r2974, r3047) Improve the German translation, especially fix gross vanilla mistranslations for the terms "shooting range" and "hired".
(r2580) Fix a vanilla memory leak of the radar image when a meanwhile sequence is playing.
(r2576) Fix a vanilla glitch: After a battle against bloodcats probably a wrong soldier (e.g. a soldier not in the battle sector) tried to say his "cool"-line.
(r2569, r2575) Fix a vanilla glitch: The clickable area of the vehicle selection box did not have the same width as the graphics of the box.
(r2565) Fix a vanilla graphical glitch: The gap between the left and right column of popup boxes was displayed wider than specified.
(r2531) Fix a vanilla graphical glitch: The level indicator was misplaced by one pixel.
(r2493) Fix a vanilla bug (mismatch of format specifier and argument of swscanf()), which caused wrong colours (or other undefined behaviour) in the credits screen.
(r2370) Fix vanilla inconsistencies in placing the item graphics and the "unload ammo" button between tactical and map screen.
(r2368) Fix vanilla inconsistencies in placing the item name/calibre/amount of money texts in the item description box.
(r2366) Fix a vanilla display glitch: When adding/removing an attachment in the map screen, the tooltips did not get updated.
(r2365) Fix a vanilla graphical glitch: A graphic was misplaced by one pixel in the item description box for money in the map screen.
(r2359) Fix a vanilla glitch: When looking at the description of an item in tactical mode, the status of the item was erroneously highlighted, too, when the item has low weight (only the weight should be highlighted).
(r2227) Fix two vanilla bugs: The test, whether there are two free slots to recruit John and Mary, was never triggered, so when only one slot was left, it resulted in a crash.  Further the test was too restrictive and checked for three free slots instead of two.
(r2217) Fix a vanilla bug which could lead to missing light effects in tactical when loading savegames.
(r2022) Fix a vanilla out of bounds array access in CalcChanceToHitGun().
(r2021) Fix a vanilla bug: The Russian versions did not play NPC speeches, because they incorrectly used different filenames than all other languages, but the filenames in the data files are the same.
(r2006, r2414) Improve Russian translation, especially fix one gross vanilla mistranslation, which mixed up Carmen and Slay.
(r1944) Fix a vanilla glitch: Do not show a balance in the personnel screen when looking at past mercs.
(r1798) Fix vanilla display bug: No path was plotted and no APs for walking were calculated for refueling when the cursor was on a vehicle.
(r1650) Fix vanilla rounding errors: The scrollbar in the inventory view of the personnel screen was not positioned correctly and could be moved one beyond the last item.

v0.7 - 2007-10-21 - Cinnamon
(r1581) Enable scrolling with the mouse wheel for sliders (volume sliders in the options menu), the message log box in the map screen and the inventory list in the personnel screen.
(r1580) Add support for the mouse wheel.
(r1545) Fix a vanilla glitch: The colour used for the APs in the single merc panel can differ from the colour used in the team panel.
(r1535) Fix a vanilla bug: After losing the central SAM, the queen sends troops to recapture the SAM to the wrong sector.
(r1523) Fix a mixup in r1487: The end turn button in the single merc panel was linked to the mute button callback.
(r1522) Fix a crash caused by r1400 in battles with bloodcats.

v0.6 - 2007-09-09 - Strawberry
(r1472) Fix graphically stuck buttons.
(r1461, r1475) Fix wide string/string format specifiers.
(r1458) Fix a vanilla infinite loop when throwing objects.
(r1449) Fix vanilla graphical glitches: When the team panel is visible, the dialogue box, civilian quote box and the list of items on ground still get clamped vertically as if the invetory panel was visible, i.e. 20px to high.
(r1442) Change the scroll speeds, so diagonal scrolling is always axis aligned.
(r1427) Fix a vanilla bug: Do not count vehicles for calculating the average stats in the personnel screen.
(r1393) Fix a vanilla bug: In the personnel screen the salary of non-AIM/MERC mercenaries was assumed to be 0, so it did not count twoards the min/max/daily salary stats.
(r1387) Fix vanilla slight misplacement of name/location and assignment/state in the personnel screen.
(r1350) Add a desktop file.
(r1325) Fix vanilla misplacement of the key ring click area in tactical mode.
(r1315) Improve the Russian translation.
(r1271) Implement SoundSetPan(). This caused an unexpected termination of the game in certain places.
(r1267) Fix a bug in r1135 which caused some texts not being displayed because of uninitialised drawing coordinates.

v0.5 - 2007-07-24 - Hazelnut
(r1259) Implement Blt16BPPTo16BPPTrans(). It is used in the editor.
(r1257) Fix a transcription error in IsTileRedundent(): The source data only uses one byte per pixel, not two.
(r1253) Fix incorrectly encoded cyrillic texts in the non-Russion versions when loading them from the data files.
(r1252) Add a workaround for the broken swprintf() implementation of FreeBSD for printing characters which cannot be represented by the current LC_CTYPE.
(r1251) Improve the Russian translation.
(r1238) Remove the black box around the text in the help window by marking the text surface as transparent.
(r1234) Fix loading of tactical maps for the Russian Gold version.
(r1233) Fix wide string/string format specifiers.
(r1219) Fix two transcription errors in Blt8BPPDataTo16BPPBufferMonoShadowClip(), which is used for blitting text:
        - Do not draw a background pixel, if the background colour is transparent
        - Draw background pixels in transparent areas (if the background colour is not transparent)
(r1215) Fix a vanilla bug: When doctoring and the character only has first aid kits available, a big item (from the second hand position) could end up in a small pocket and/or multiple first aid kits could be placed into the first hand position.
(r1210, r1214) Fixes for porting to 64bit platforms
(r1201) Adjust the char/glyph mapping table for two Russian releases (BUKA Agonia Vlasti and Russian Gold) and fix the incorrectly encoded Russian texts when loading them from the data files.
(r1198) Fix the incorrectly encoded Polish texts when loading them from data files. Playing with Polish data files should work flawlessly now.
(r1196) Correct the char to glyph translation table. This should fix the Polish translation.
(r1193) Simplify the interface of GetTileIndexFromTypeSubIndex(). This also fixes one vanilla incorrect use of this function in PasteSingleBrokenWall().

v0.4 - 2007-07-08 - Cherry
(r1183) Add configfile handling and change the code to use the configfile to determine the path to the binary data.
(r1139) Fix a vanilla bug: In SwapLarrysProfiles() ->bInvStatus[] gets copied twice, but ->bInvNumber[] not at all. Rectify this.
(r1121) Fix a vanilla glitch: If text is highlighted and delete gets pressed do not just beep but actually delete the text.
(r1120) Slightly alter the behaviour when pressing left and right without shift and a part of the text was highlighted, i.e. do not consider the fact there was a highlight when calculating the new cursor position. It feels better this way imo.
(r1103, r1104, r1105) Fix hotkey highlights in texts
(r1081) Load the item names from the correct position in the data file. The bug was introduced in r396.
(r1025) Fix a vanilla resource leak: Unload all fonts on exit. Five fonts were not unloaded.
(r1004) Check whether gusSelectedSoldier is not NO_SOLDIER before using it as an index into MercPtrs[] to prevent an out of bounds array access. This fixes a crash when entering tactical mode in a sector where all mercs are busy.
(r970) Fix a vanilla memory leak in ClearEventQueue(): Free the removed events.
(r928) #include <stdint.h> to get the declaration for uintptr_t and do not rely on namespace pollution of other headers.
(r854, r903) Fix wide string/string format specifiers.
(r829) Fix two vanilla bugs in the IMP result mail generator:
       - No text was generated if agility was below average, but above the extremly low limit, because the latter test was reversed
       - If leadership was the only extremly low attribute no introduction text was generated
(r805) Do not store the email subjects in the savegames, but always fetch them from the data files. This way f.e. you do not end up with english subjects when loading a savegame made with the english version in the german version.
(r777) Just use *.jsd as pattern instead of *.[Jj][Ss][Dd]. The former works on Windows' FindFirstFile() and should work on case sensitive filesystems too, because all filenames are required to be lowercase. This fixes the problem of missing corpses on Windows.
(r711) Use given parameter Length instead of lengthof(sString). This fixes the sector a merc is in not being displayed in the strategic screen.

v0.3 - 2007-04-08 - Chocolate
(r688, r690, r696, r698, r699, r700, r703, r704, r706) Windows support
(r676) The yes/no buttons for deleting emails work again now.
(r674) Simplify the code to swap emails. This fixes the vanilla bug that sorting emails corrupts emails with variable content (i.e. mercenary names and monetary amounts), because not all parts of emails got swapped.
(r658) Plug some vanilla file leaks on error.
(r652) Slightly improve command line parameter parsing and provide the -nosound switch.
(r634) Fix a vanilla bug: If FileOpen() fails 0 is returned, not -1.
(r629) Fix incorrect vanilla resource handling: In CreateVideoObject() the passed HIMAGE gets destroyed on only one error path, not on others and neither if the function returns successfully. Its callers always destroy the HIMAGE, therefore remove the only DestroyImage() in CreateVideoObject(), so the HIMAGE does not get freed twice on this error path.
(r627) Fix a vanilla glitch: In BltStretchVideoSurface() return with an error, if either VSurface has a colour depth != 16, not only if both have a colour depth != 16.
(r623) Fix a vanilla resource leak: Do not leak a lock on a VSurface on error.
(r597) Fix incorrect vanilla file handling: Do not leak the file handle when playing a Smack file fails.
(r563) Fix incorrect vanilla file handling: Do not close the file in STCILoadIndexed(). The caller does that.
(r560) Fix incorrect vanilla file handling: When reading a savegame fails do no close the file multiple times.
(r559) Fix incorrect vanilla file handling: When writing a savegame fails do no close the not yet opened file resp. do not close it multiple times.
(r549) Remove a vanilla restriction: There is no technical reason to forbid % and \ when entering text. % caused problems because printf()-like functions were used incorrectly. \ never was a problem.
(r548) Use the unicode char provided by SDL for text input. This is a lot simpler and avoids the internal translation table, which cannot respect different keyboard layouts.
(r528) Use scroll lock to toggle input grabbing. This is useful for scrolling with the mouse in tactical mode when playing in windowed mode.
(r505, r583, r584, r602, r606, r705) Fix wide string/string format specifiers.

v0.2 - 2007-03-15 - Pistachio
(r497) Fix scrolling of the tactical view: SDL_UpdateRect() was missing.
(r491) Implement sound support.
(r484) Fix some vanilla format string vulnerabilities which lead to crashes when a text field (for example the savegame title) contains a percent sign.
(r480) Fix vanilla odditities in string handling: Properly print percent ("%") signs.  Some strings get parsed by printf()-like functions multiple times. Of course only the first time (if at all) they should be interpreted as format strings. Therefore Do The Right Thing(tm) and use "%S" as format string and the already parsed string as argument in the subsequent runs. The directly visible effect is dissapearing percent signs. There are some gross (and partially incorrect, because they had a dangling % in the format string) workarounds for this in the code. Now that these strings are correctly handled remove those workarounds.
(r479, r486) Fix wide string/string format specifiers.
(r468) Do not try to close a file when it was not opened.
(r467) Use %C instead of %c to print wchar_ts.
(r464) Set the title bar text to something meaningful.
(r458) Fix a vanilla incorrect translation, which also displayed some garbage.
(r451) Implement Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip().
(r450) Implement Blt8BPPDataTo16BPPBufferTransShadowZNBClip().
(r446) Implement Blt8BPPDataTo16BPPBufferOutlineZClip().
(r431) INTERFACE/mapcursr.sti gets loaded thrice in direct succession and leaked twice.
(r429) Remove video objects which only get loaded and deleted (if at all, memory leak, anyone?), but never used.
(r400) Implement Blt8BPPDataTo16BPPBufferOutlineClip() and Blt8BPPDataTo16BPPBufferShadowZNBClip(). They are used when things get thrown + clipping.
(r399) Implement a simple zoom blitter.
(r397) Implement Blt8BPPDataTo16BPPBufferOutlineZNB() and Blt8BPPDataTo16BPPBufferShadowZNB(). They are used when things get thrown.
(r395) Fix a vanilla out of bounds access: The money button image has 5 subimages, not 7, which is assumed for creating this button in the tactical screen. Use the same subimages like the map screen.
(r393) Implement Blt8BPPDataTo16BPPBufferTransZTranslucent() and Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(). They are used for explosions and probably other semi-transparent stuff.
(r392) Fix minor graphical glitch in Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured(): The pixelation alignment was dependent on the number of skipped lines, which it should not.
(r391) Fix Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(): The obscured parts of the sprites were not displayed.
(r390) Implement Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured().
(r382) Fix a memory leak: The SDL_Surface got leaked when deleting a video surface.
(r381) Fix a vanilla memory leak which happens during loading savegames.
(r378) Fix Blt8BPPDataTo16BPPBufferMonoShadowClip(), clipping was done incorrectly which lead to graphical artifacts when drawing text.

v0.1 - 2006-11-16 - Vanilla - First release
(r369) Fixing german translation - there is no longer a time limit, only an option to set whether saving in battles is allowed. Changing text accordingly.
(r363) Add a crude manpage.
(r359) Also check the DATADIR when opening a file/checking if a file exists.
(r358) Detect the users home directory (using $HOME and getpwuid()) and use the subdirectory .ja2 there for local data.
(r357) Allow all data files to be in the data directory.
(r356) Make the directory of the data files configurable.
(r354) Fix compilation of the dutch, french, italian and polish translations. Make the language selectable via the Makefile.
(r341) Add -FULLSCREEN as a commandline-option to start ja2 in fullscreen.
(r340) Add handling of special keys to hackish keyboard support (r71)
(r327) Implement Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip().
(r319) Reduce the timer code to the only functionality it is used for: Getting a millisecond counter. This fixes a vanilla race condition caused by wrinting to the global counter multiple times in the callback, which could cause the counter jump backwards.
(r256) Further SDLify the video module and throw away unused DirectX stuff. The in-game mouse cursor works now.
(r246) Implement a hack to display /something/ when a zooming blit is requested.
(r243) Make the editor compile and start. You need the file "Editor.slf".
(r238) Fix crash, caused by a vanilla out of bounds array access, when entering a sector and you are immediately in combat mode.
(r194) Implement CheckVideoObjectScreenCoordinateInData().
(r167) Implement Blt8BPPDataTo16BPPBufferHalfRect().
(r155) Implement Blt8BPPDataTo16BPPBufferTransShadowZNB().
(r154) Implement Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip().
(r153) Implement IsTileRedundent().
(r152) Implement Blt8BPPDataTo16BPPBufferTransZIncObscureClip().
(r151) Implement Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough().
(r150) Implement Blt8BPPDataTo16BPPBufferTransZIncClip().
(r149) Implement Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured().
(r148) Implement Blt8BPPDataTo16BPPBufferOutlineZ().
(r147) Implement Blt8BPPDataTo16BPPBufferShadowClip().
(r145) Implement Blt8BPPDataTo16BPPBufferShadow().
(r144) Implement Blt8BPPDataTo16BPPBufferTransZNBClip().
(r143) Implement Blt8BPPDataTo16BPPBufferTransZClip().
(r142) Implement Blt8BPPDataTo16BPPBufferShadowZClip().
(r141) Implement Blt8BPPDataTo16BPPBufferShadowZ().
(r140) Implement Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip().
(r139) Implement Blt8BPPDataTo16BPPBufferTransShadowZNBObscured().
(r138) Implement Blt8BPPDataTo16BPPBufferTransZNB().
(r137) Implement Blt8BPPDataTo16BPPBufferTransZ().
(r136) Implement Blt8BPPDataTo16BPPBufferTransZPixelateObscured().
(r131) Place a SDL_Delay(1) in the main loop so the process doesn't eat that much CPU
(r118) Implement Blt8BPPDataTo16BPPBufferOutline() and Blt8BPPDataTo16BPPBufferOutlineShadow().
(r83) Repaired Random() (original Implementation broke with RAND_MAX > 32767).
(r75, r78, r94, r112, r127, r128, r129, r130, r134, r170, r171, r172, r173, r174, r175, r178, r179, r180, r182, r183, r190, r191, r208, r212, r214, r236, r242, r336, r337, r338, r351, r364, r365, r366, r367, r368) Fix wide string/string format specifiers.
(r74) Implement Blt8BPPDataTo16BPPBufferHalf() and Blt8BPPDataTo16BPPBufferTransparentClip().
(r71) Implement crude keyboard input.
(r70) Implement Blt8BPPDataSubTo16BPPBuffer()
(r69) Implement another code path in BltVSurfaceUsingDD().
(r68) Implement/FIXME some mouse movement related functions.
(r67) Implement Blt16BPPBufferShadowRect().
(r66) SDLify BltVSurfaceUsingDD().
(r65, r113) Fix a vanilla buffer overflow: uiSeekAmount is the number of bytes, not the number of 16bit characters. So properly adjust the loop delimiter.
(r64) Implement Blt16BPPBufferPixelateRectWithColor().
(r57) Implement Blt8BPPDataTo16BPPBufferMonoShadowClip().
(r54) SDLify some parts of the code. The main menu shows up now.
(r53) Implement three blitters in C.
(r47) Some more FIXMEs to make it run.
(r46) Implement/FIXME more bits. Now it spins in the main loop.
(r45) Implement some more bits and sprinkle some more FIXMEs. Runs a bit further, but still immediatly terminates.
(r44) Make it work a little bit by replacing some UNIMPLEMENTED by FIXME and actually implemeting some stuff.
(r42) Put UNIMPLENTED() everywhere. At least it can be linked now.
