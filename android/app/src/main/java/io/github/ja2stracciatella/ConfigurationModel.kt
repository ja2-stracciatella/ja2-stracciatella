package io.github.ja2stracciatella

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import kotlinx.serialization.SerializationException

class ConfigurationModel : ViewModel() {

    val vanillaGameDir = MutableLiveData<String>()
    val saveGameDir = MutableLiveData<String>()

    fun setVanillaGameDir(vanillaGameDirSet: String) {
        vanillaGameDir.value = vanillaGameDirSet
    }

    fun setSaveGameDir(saveGameDirSet: String) {
        saveGameDir.value = saveGameDirSet
    }
}
