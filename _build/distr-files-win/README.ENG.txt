JA2-Stracciatella 0.11 binary package for Windows


The first time you start the game, a directory "JA2" is created in your user
directory ("My documents").  In there is a file "ja2.ini".  Edit this file and
insert the correct path to the Jagged Alliance 2 data files in the line
"data_dir = [...]".  If $DIR is the directory, then it shall contain:
- $DIR/Data/*.slf           - all .slf files from the installation AND game CD
- $DIR/Data/TILECACHE/*.sti - all files found in the TILECACHE directory from
- $DIR/Data/TILECACHE/*.jsd   the installed game

This amounts to:
-  22 .slf (or 23, if there is a specific .slf for the translation)
-  28 .sti
- 109 .jsd


WWW: http://ja2.dragonriders.de/
