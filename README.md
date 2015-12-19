# JA2-Stracciatella Continued

This is continuation of venerable JA2-Stracciatella project.

The goal of the project was to make Jagged Alliance 2 available on a wide range of platforms, improve its stability, fix bugs and provide a stable platform for mod development.  At the moment the goal mostly to fix bugs.

## How to start the game

1. Install original Jagged Alliance 2 game on your computer.  Data files from the original game will be used by JA2-Stracciatella

2. [Download JA2-Stracciatella](https://bitbucket.org/gennady/ja2-stracciatella/downloads) or [compile](COMPILATION.md) it from the source codes.

3. Start the game the first time.  It will create the configuration file %USERPROFILE%\Documents\JA2\ja2.ini on Windows or ~/.ja2/ja2.ini on Unix-like systems.

4. Edit the configuration file and set parameter data_dir to point on the directory where the original game was installed on step 1.  For example, D:\games\ja2\ (on Windows) or /home/user/games/ja2-installed (on Linux).

5. If you installed not English version of the original game, but one of the localized varieties (e.g. French or Russian), you need to start ja2.exe with parameter telling which version of the game you are using.  For example: ```ja2.exe -resversion FRENCH```

Supported localizations are DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD.  RUSSIAN is for BUKA Agonia Vlasty release, RUSSIAN_GOLD is for Gold release.

If you downloaded precompiled version of JA2-Stracciatella, the archive may already contain set of bat files for each supported localizations.

Run ```ja2.exe -help``` for list of available options.

## Latest news

- Fall 2015, the project is still alive.  Some pull requests were merged.
- 2014-05-01, version 0.12.352 has been released
- 2014-02-16, version 0.12.275 has been released

## Development

The project is developed on [bitbucket](https://bitbucket.org/gennady/ja2-stracciatella).  Pull requests with bug fixes are very welcome.

[![Build Status](https://travis-ci.org/gennady/ja2-stracciatella.svg?branch=master)](https://travis-ci.org/gennady/ja2-stracciatella)

### How to contribute

The best way to contribute is to make a pull request with a bug fix.  Please see list of open issues [here](https://bitbucket.org/gennady/ja2-stracciatella/issues?status=new&status=open).

The second best way is to file a bug report if you encounter a bug.

### How to make a pull request

1. Fork the project
2. Create a branch (git checkout -b my_feature_branch)
3. Commit your changes (git commit -am "description of your changes")
4. Push to the branch (git push origin my_feature_branch)
5. Create a pull requst from your branch into master

Please don't reformat the code for the sake of it, because it will make the merge process harder.  Instead use the following settings in your editor:
- display tab as 2 spaces
- indent with spaces

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
