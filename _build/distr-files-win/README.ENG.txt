JA2-Stracciatella binary package for Windows


The first time you start the game, a directory "JA2" is created in your user
directory ("My documents").  In there is a file "ja2.ini".  Edit this file and
insert the correct path to the Jagged Alliance 2 data files in the line
"data_dir = [...]".  If $DIR is the directory, then it shall contain:
- $DIR/Data/*.slf           - all .slf files from the installation AND game CD
- $DIR/Data/TILECACHE/*.sti - all files found in the TILECACHE directory from
- $DIR/Data/TILECACHE/*.jsd   the installed game

ja2.exe can work with many versions of Jagged Alliance 2 data files.  You
should start the game by running one of the bat file, corresponding your
version of the game files.

For example, if you have English version installed, start the game by running
ja2-ENGLISH.bat.  If you have French version installed, start the game by
running ja2-FRENCH.bat, etc.


If you want to play the game on Linux, you will have to compile it from the
source codes.  It is not as difficult as it sounds:
 - download source codes from https://bitbucket.org/gennady/ja2-stracciatella
 - follow instructions from INSTALL.txt


WWW: https://bitbucket.org/gennady/ja2-stracciatella
