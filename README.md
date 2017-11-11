# JA2-Stracciatella Continued

[![Build Status Travis](https://img.shields.io/travis/ja2-stracciatella/ja2-stracciatella/master.svg?style=flat-square)](https://travis-ci.org/ja2-stracciatella/ja2-stracciatella)
[![Build Status Appveyor](https://img.shields.io/appveyor/ci/ja2-stracciatella/ja2-stracciatella/master.svg?style=flat-square)](https://ci.appveyor.com/project/ja2-stracciatella/ja2-stracciatella)

This is continuation of venerable JA2-Stracciatella project.

The goal of the project was to make Jagged Alliance 2 available on a wide range of platforms, improve its stability, fix bugs and provide a stable platform for mod development.  At the moment the goal mostly to fix bugs.

Official Homepage: http://ja2-stracciatella.github.io/

## How to start the game

1. Install original Jagged Alliance 2 game on your computer.  Data files from the original game will be used by JA2-Stracciatella

2. [Download JA2-Stracciatella](http://ja2-stracciatella.github.io/download/) or [compile](COMPILATION.md) it from the cloned git repository.

3. Start the game the first time.  It will create the configuration file %USERPROFILE%\Documents\JA2\ja2.json on Windows or ~/.ja2/ja2.json on Unix-like systems.

4. Edit the configuration file and set parameter data_dir to point on the directory where the original game was installed on step 1.  For example, D:\games\ja2\ (on Windows) or /home/user/games/ja2-installed (on Linux).

5. If you installed not English version of the original game, but one of the localized varieties (e.g. French or Russian), you need to start ja2.exe with parameter telling which version of the game you are using.  For example: ```ja2.exe -resversion FRENCH```

Supported localizations are DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD.  RUSSIAN is for BUKA Agonia Vlasty release, RUSSIAN_GOLD is for Gold release.

If you downloaded precompiled version of JA2-Stracciatella, the archive may already contain set of bat files for each supported localizations.

Run ```ja2.exe -help``` for list of available options.

## Development

The project is developed on [github](https://github.com/ja2-stracciatella/ja2-stracciatella).  Pull requests with bug fixes are very welcome.

### How to contribute

The best way to contribute is to make a pull request with a bug fix.  Please see list of open issues [here](https://github.com/ja2-stracciatella/ja2-stracciatella/issues).

The second best way is to file a bug report if you encounter a bug.

### How to make a pull request

1. Fork the project
2. Create a branch (git checkout -b my_feature_branch)
3. Commit your changes (git commit -am "description of your changes")
4. Push to the branch (git push origin my_feature_branch)
5. Create a pull requst from your branch into master

Please don't reformat the code for the sake of it, because it will make the merge process harder.  Instead use the following settings in your editor:
- display tab as 8 spaces
- indent with tabs
If you add new code, please don't add spaces after opening or before closing round brackets.

## History of the Project

The original project was run by Tron since 2006.  He did an amazing job of
cleaning up JA2 sources and making them portable.  The work was massive too -
there are over *7000 commits* in the original svn repository
svn://tron.homeunix.org/ja2/trunk.  Unfortunately, the work on the project
ceased in 2010.  The [original project homepage](http://tron.homeunix.org/ja2)
is no longer available.  Some history can be found in [JA2-Stracciatella
Q&A](http://thepit.ja-galaxy-forum.com/index.php?t=msg&th=13222)

## License

Unless specified explicitly in the commit message, all changes since `commit
8287b98` are released to the public domain.  All libraries in `dependencies/lib-*`
have their own licenses.

It is not known under which license Tron released his changes.  All we know,
the source codes were publicly available in his svn repository.

Original Jagged Alliance source codes were released by Strategy First Inc. in
2004 under Source Code License Agreement ("SFI-SCLA").  You can find the
license in file *SFI Source Code license agreement.txt*.
