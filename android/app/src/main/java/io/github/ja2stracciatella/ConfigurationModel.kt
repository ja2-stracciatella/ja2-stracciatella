package io.github.ja2stracciatella

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel

enum class VanillaVersion(val value: String) {
    DUTCH("DUTCH"),
    ENGLISH("ENGLISH"),
    FRENCH("FRENCH"),
    GERMAN("GERMAN"),
    ITALIAN("ITALIAN"),
    POLISH("POLISH"),
    RUSSIAN("RUSSIAN"),
    RUSSIAN_GOLD("RUSSIAN_GOLD"),
    SIMPLIFIED_CHINESE("SIMPLIFIED_CHINESE");

    fun getLabel(): String {
        return when (this) {
            DUTCH -> "Dutch"
            ENGLISH -> "English"
            FRENCH -> "French"
            GERMAN -> "German"
            ITALIAN -> "Italian"
            POLISH -> "Polish"
            RUSSIAN -> "Russian"
            RUSSIAN_GOLD -> "Russian (Gold)"
            SIMPLIFIED_CHINESE -> "Simplified Chinese"
        }
    }
}

class ConfigurationModel : ViewModel() {

    val vanillaGameDir = MutableLiveData<String?>()
    val vanillaGameVersion = MutableLiveData<VanillaVersion?>()
    val saveGameDir = MutableLiveData<String?>()

    fun setVanillaGameDir(vanillaGameDirSet: String?) {
        vanillaGameDir.value = vanillaGameDirSet
    }

    fun setVanillaGameVersion(version: VanillaVersion?) {
        vanillaGameVersion.value = version
    }

    fun setSaveGameDir(saveGameDirSet: String?) {
        saveGameDir.value = saveGameDirSet
    }
}
