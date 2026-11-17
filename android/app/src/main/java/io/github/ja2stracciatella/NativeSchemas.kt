package io.github.ja2stracciatella

/**
 * Access to the json schemas of the externalized files, which are compiled into the game library.
 */
object NativeSchemas {
    init {
        // The same libraries StracciatellaActivity loads, loading them again when the game starts does nothing
        System.loadLibrary("SDL3")
        System.loadLibrary("ja2")
    }

    // Path relative to the externalized dir, e.g. game.json. Returns null if there is no schema for the path.
    fun getSchemaForPath(path: String): String? {
        return getSchemaBytesForPath(path)?.toString(Charsets.UTF_8)
    }

    @JvmStatic
    private external fun getSchemaBytesForPath(path: String): ByteArray?
}
