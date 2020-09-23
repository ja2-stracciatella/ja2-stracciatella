#pragma once

/*! \mainpage Scripting in JA2 Stracciatella
 *
 * # Extending mods by Lua scripting
 *
 * If you want to change game behaviours that cannot be controlled by externalized JSON, you can provide Lua script to extend the vanilla game logic. C++ codebase has a set of integration points, so that at certain point in game, C++ will make a call to Lua functions provided by the mods, if such function exists. Lua scripts can then read and modify game state or trigger activities.
 *
 * # Example
 * 
 * Lua scripting engine is enabled if your mod must provide scripts/main.lua
 * 
 * ```lua
 * -- Imports the enums.lua provided by the base game
 * -- This gives you access to predefined enum values such as Items or MercsProfiles
 * JA2Require("enums.lua")
 *
 * -- This functions gets called after a sector is loaded, but before we actually put enemies and soldiers in
 * function BeforePrepareSector()
 *     log.info("Do something before preparing sector")
 * end
 * ```
 * 
 * ## Logging
 * 
 * Logger is provided as a global object `log` in the Lua environment. Use it as follows:
 * 
 * ```lua
 * log.debug("Log at DEBUG level")  --start ja2 with -debug switch to enable DEBUG log
 * log.info("Log at INFO level")
 * log.warn("Log at WARN level")
 * log.error("Log at ERROR level")
 * ```
 */

/**
 * Re-initializes the state of the scripting engine. This should be run whenever a game is 
 * started or loaded, because there might be states in the lua space.
 */
void InitScriptingEngine();
