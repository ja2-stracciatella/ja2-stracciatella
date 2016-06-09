JA2-Stracciatella binary package for Mac OS X

Extract zip archive with the game.

JA2-Stracciatella can work with many versions of Jagged Alliance 2 data files.
You should start the game by running one of the command files, corresponding
your version of the game files.

For example, if you have English version installed, start the game by running
ja2-ENGLISH.command.  If you have French version installed, start the game by
running ja2-FRENCH.command, etc.

The first time you start the game, a hidden folder ".ja2" is created in your
user directory ("/Users/YouName/").  In there is a file "ja2.ini".  Edit this
file and insert the correct path to the Jagged Alliance 2 data files in the
line "data_dir = [...]".  If $DIR is the directory, then it shall contain:
- $DIR/Data/*.slf           - all .slf files from the installation AND game CD
- $DIR/Data/TILECACHE/*.sti - all files found in the TILECACHE directory from
- $DIR/Data/TILECACHE/*.jsd   the installed game

It is possible to change game resolution using command-line key '-res'.
For example, to start the game in 1024x768 mode, launch ja2 like this:

  ja2 -res 1024x768

Any reasonable combination of width and height should be possible.  You can
experiment and find one that suits you best.

JA2-Stracciatella is also available for Windows and Linux.

If you want to play the game on Linux, you will have to compile it from the
source codes.  It is not as difficult as it sounds:
 - download source codes from https://bitbucket.org/gennady/ja2-stracciatella
 - follow instructions from INSTALL.txt


WWW: https://github.com/ja2-stracciatella/ja2-stracciatella
