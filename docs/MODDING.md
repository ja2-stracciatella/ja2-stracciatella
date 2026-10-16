# Modding

## What can be modded

### JA2 data resources

You can substitute JA2 data files (e.g. STI or EDT files) by placing your own in the mod directory.

The resource files can be in extracted form (e.g. `maps/A9.dat`, letter casing does not matter), or in SLF archives. When the game looks for a particular resource file, it will first look through the enabled mod directories, and then the base game data directory.

Check out also the [ja2-open-toolset](https://github.com/ja2-stracciatella/ja2-open-toolset) project which provide tools to work with the data files.

### Externalized JSON table data

For details, look at the [externalized](https://github.com/ja2-stracciatella/ja2-stracciatella/tree/master/assets/externalized) directory. You might want a good JSON editor that can highlight syntax and errors.

- Items, weapons and magazine (you can re-use and re-purpose the item slots, but not adding new slots)
- Strategic map (movement cost, towns)
- Strategic AI (garrison and patrol)
- I.M.P. policy (levels, items, etc)
- M.E.R.C. listing
- Dealers and their inventories
- Music
- Loading screens

### Lua scripting

You can add scripting by adding `data/scripts/main.lua` in your mod directory. In your Lua scripts, you would define listeners, which will be invoked by the game. A basic example can be found in the [generous-rebels](https://github.com/ja2-stracciatella/ja2-stracciatella/blob/master/assets/mods/generous-rebels/data/scripts/main.lua) mod.

Look at [FunctionLibrary.h](https://github.com/ja2-stracciatella/ja2-stracciatella/blob/master/src/externalized/scripting/FunctionsLibrary.h) to get a sense of what's available.
