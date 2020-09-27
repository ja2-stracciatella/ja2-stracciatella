package io.github.ja2stracciatella

import org.libsdl.app.SDLActivity

open class StracciatellaActivity : SDLActivity() {
    open override fun getLibraries(): Array<String?>? {
        return arrayOf(
            "hidapi",
            "SDL2",
            "ja2"
        );
    }
}