# JA2-Stracciatella

[![GitHub CI](https://img.shields.io/github/actions/workflow/status/ja2-stracciatella/ja2-stracciatella/github-ci.yml?branch=master&label=GitHub%20CI&logo=github)](https://github.com/ja2-stracciatella/ja2-stracciatella/actions?query=workflow%3A%22GitHub+CI%22)
[![AppVeyor](https://img.shields.io/appveyor/ci/ja2-stracciatella/ja2-stracciatella/master.svg?style=flat-square&logo=appveyor&label=AppVeyor)](https://ci.appveyor.com/project/ja2-stracciatella/ja2-stracciatella)
[![Coverity Scan](https://img.shields.io/coverity/scan/8431.svg?style=flat-square&label=Coverity%20Scan)](https://scan.coverity.com/projects/ja2-stracciatella-ja2-stracciatella)
[![Current Release](https://img.shields.io/github/downloads/ja2-stracciatella/ja2-stracciatella/v0.22.1/total)](https://github.com/ja2-stracciatella/ja2-stracciatella/releases/tag/v0.22.1)

JA2-Stracciatella ports the classic 1999 PC tactical game *Jagged Alliance 2* to a wider range of platforms, improves game stability, fixes bugs and enhances moddability. At the moment the goal is mostly to fix bugs.

We are a community-driven open source project that welcomes all. We have a few maintainers who keep the big picture in mind, and a long list of contributors. The code can be forked, poked, twisted and hacked. The project is developed on [GitHub](https://github.com/ja2-stracciatella/ja2-stracciatella) where it can be customized, extended, and collaboratively developed.

You are invited to join us and [contribute](CONTRIBUTING.md)!

[Official Homepage](https://ja2-stracciatella.github.io)

[Discord - #ja2-stracciatella](https://discord.com/invite/GqrVZUM)

## How to start the game

1. Install original Jagged Alliance 2 game on your computer.  Data files from the original game will be used by JA2-Stracciatella

2. [Download JA2-Stracciatella](http://ja2-stracciatella.github.io/download/) or [compile](COMPILATION.md) it from the cloned git repository.

### With the optional launcher

3. Start the launcher and use it to configure the game. It will automatically create the configuration file.

4. Set “JA2 Data Directory” to point to the directory where the original game was installed during step 1. You can manually enter the directory or use the “...” button to browse your computer.

5. If you haven't installed the English version of the original game, you have to select the correct “Game Version” i.e. localization. Note that the game supports two different Russian localizations: RUSSIAN for the “BUKA Agonia Vlasty” release and RUSSIAN_GOLD for the “Gold” release.

### Without the optional launcher

3. Start the game the first time.  It will create the configuration file `%USERPROFILE%\Documents\JA2\ja2.json` on Windows or `~/.ja2/ja2.json` on Unix-like systems.

4. Edit the configuration file and set parameter game_dir to point to the directory where the original game was installed during step 1.  For example, `D:\games\ja2\` (on Windows) or `/home/user/games/ja2-installed` (on Linux).

5. If you installed not the English version of the original game, but one of the localized varieties (e.g. French or Russian), you need to start `ja2.exe` with parameter telling which version of the game you are using.  For example: `ja2.exe -resversion FRENCH`

Supported localizations are DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD. Use RUSSIAN for the “BUKA Agonia Vlasty” release and RUSSIAN_GOLD for the “Gold” release.

If you downloaded a precompiled version of JA2-Stracciatella, the archive may contain a set of bat files for all supported localizations.

Run `ja2.exe -help` for list of available options.

## History of the Project

We are continuing the venerable JA2-Stracciatella project, originally started by Tron in 2006.

He did an amazing job of cleaning up the JA2 sources and making them portable. The work was massive - there are over *7000 commits* in the original [svn repository](svn://tron.homeunix.org/ja2/trunk).

Though the [original project homepage](http://tron.homeunix.org/ja2)
is no longer available, some history can be found in the [JA2-Stracciatella
Q&A](https://thepit.ja-galaxy-forum.com/index.php?t=msg&th=13222) thread on The Bear's Pit forums, or on the
[Wayback Machine](https://web.archive.org/web/20140204204243/http://tron.homeunix.org/ja2).

More history available on the [Stracciatella project homepage](https://ja2-stracciatella.github.io/history/).

## License

Unless specified explicitly in the commit message, all changes since `commit 8287b98`
are released to the public domain.  All libraries in `dependencies/lib-*`
have their own licenses.

It is not known under which license Tron released his changes.  All we know,
the source codes were publicly available in his svn repository.

The original Jagged Alliance source code was released by Strategy First Inc. in
2004 under the Source Code License Agreement ("SFI-SCLA").  You can find the
license in file *SFI Source Code license agreement.txt*.
