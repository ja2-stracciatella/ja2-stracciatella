# Debugging

## In Game

To enable debugging output and tools start the game with the `-debug` parameter.

### Tactical Screen

- `F11` for Quest Debug Screen 
- `CTRL+f` for FPS Display 
- `CTRL+z` to toggle z-buffer 
- `ALT+m` for level node debug mode 
- `ALT+n` to play quotes of hovered merc 
- `q` for soldier and land debug mode 
- `y` for struct debug mode

To exit any screen press `q` again. To cycle through different pages use pageup and pagedown.

### Crash Debugging

Use your favourite tools (IDE, gdb, valgrind ...).

Example crash investigation start on Linux:
    # build the project with debugger information included
    $ make clean
    $ make all WITH_DEBUGINFO=1
    
    # start GDB and run the game until it crashes
    $ gdb ./ja2
    (gdb) run

    # inspect the callstack
    (gdb) backtrace

Define _DEBUG to get extra debug output.
