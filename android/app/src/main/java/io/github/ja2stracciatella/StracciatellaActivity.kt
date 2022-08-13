package io.github.ja2stracciatella

import android.view.View
import org.libsdl.app.SDLActivity

open class StracciatellaActivity : SDLActivity() {
    override fun getLibraries(): Array<String?>? {
        return arrayOf(
            "SDL2",
            "ja2"
        )
    }

    // We suppress deprecation warnings here as our Android SDK minimum version
    // does not have replacements for those APIs yet.
    @Suppress("DEPRECATION")
    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)

        // Set app to fullscreen mode
        if (hasFocus) {
            window.decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION)
        }
    }
}
