package io.github.ja2stracciatella

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel

class ConfigurationModel : ViewModel() {

    val vanillaGameDir = MutableLiveData<String>()

    fun setVanillaGameDir(vanillaGameDirSet: String) {
        vanillaGameDir.value = vanillaGameDirSet
    }
}