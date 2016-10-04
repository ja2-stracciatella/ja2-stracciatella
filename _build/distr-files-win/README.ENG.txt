JA2-Stracciatella binary package for Windows

Extract zip archive with the game.

JA2-Stracciatella can work with many versions of Jagged Alliance 2 data files.
You should start the game by running one of the bat files, corresponding your
version of the game files.

For example, if you have English version installed, start the game by running
ja2-ENGLISH.bat.  If you have French version installed, start the game by
running ja2-FRENCH.bat, etc.

The first time you start the game, a directory "JA2" is created in your user
directory ("My documents").  In there is a file "ja2.ini".  Edit this file and
insert the correct path to the Jagged Alliance 2 data files in the line
"data_dir = [...]".  If $DIR is the directory, then it shall contain:
- $DIR/Data/*.slf           - all .slf files from the installation AND game CD
- $DIR/Data/TILECACHE/*.sti - all files found in the TILECACHE directory from
- $DIR/Data/TILECACHE/*.jsd   the installed game

It is possible to change game resolution using command-line key '-res'.
For example, to start the game in 1024x768 mode, launch ja2.exe like this:

  ja2.exe -res 1024x768

Any reasonable combination of width and height should be possible.  You can
experiment and find one that suits you best.

JA2-Stracciatella is also available for macOS and Linux.

WWW: https://ja2-stracciatella.github.io
