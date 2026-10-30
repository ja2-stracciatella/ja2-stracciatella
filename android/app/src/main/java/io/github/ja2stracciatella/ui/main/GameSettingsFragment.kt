package io.github.ja2stracciatella.ui.main

import android.app.AlertDialog
import android.os.Bundle
import android.text.Editable
import android.text.InputType
import android.text.TextWatcher
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import io.github.ja2stracciatella.GameSetting
import io.github.ja2stracciatella.GameSettingType
import io.github.ja2stracciatella.GameSettings
import io.github.ja2stracciatella.R
import io.github.ja2stracciatella.databinding.FragmentLauncherGameSettingsBinding
import io.github.ja2stracciatella.databinding.LauncherGameSettingItemBinding
import io.github.ja2stracciatella.databinding.LauncherGameSettingsSectionBinding
import java.io.IOException


class GameSettingsFragment : Fragment() {
    private var _binding: FragmentLauncherGameSettingsBinding? = null
    private val binding get() = _binding!!

    private val fragmentLogTag = "GameSettingsFragment"
    private var gameSettings: GameSettings? = null
    // Updates the controls of each setting after the values were reset
    private val showValueCallbacks = mutableListOf<() -> Unit>()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentLauncherGameSettingsBinding.inflate(inflater, container, false)
        showValueCallbacks.clear()

        val settings = try {
            GameSettings.load(requireContext())
        } catch (e: IOException) {
            showLoadError(e)
            return binding.root
        } catch (e: IllegalArgumentException) {
            showLoadError(e)
            return binding.root
        }
        gameSettings = settings

        for (section in settings.sections) {
            if (section.title != null) {
                val sectionBinding = LauncherGameSettingsSectionBinding.inflate(
                    inflater,
                    binding.gameSettingsList,
                    true
                )
                sectionBinding.gameSettingsSectionTitle.text = section.title
            }
            for (setting in section.settings) {
                addSetting(inflater, settings, setting)
            }
        }

        binding.gameSettingsResetAllButton.setOnClickListener {
            AlertDialog.Builder(requireContext())
                .setMessage(R.string.game_settings_reset_all_confirmation)
                .setPositiveButton(R.string.game_settings_reset_all_button) { _, _ ->
                    settings.resetAll()
                    showValueCallbacks.forEach { it() }
                }
                .setNegativeButton(android.R.string.cancel, null)
                .show()
        }

        return binding.root
    }

    override fun onPause() {
        super.onPause()

        // The game reads the settings when it starts, which always pauses the launcher first
        try {
            gameSettings?.save()
        } catch (e: IOException) {
            val message = "Could not save the game settings: ${e.message}"
            Log.e(fragmentLogTag, message)
            Toast.makeText(requireContext(), message, Toast.LENGTH_SHORT).show()
        }
    }

    private fun showLoadError(e: Exception) {
        Log.e(fragmentLogTag, "Could not load the game settings: ${e.message}")
        binding.gameSettingsErrorText.text = getString(R.string.game_settings_load_error, e.message)
        binding.gameSettingsErrorText.visibility = View.VISIBLE
        binding.gameSettingsResetAllButton.visibility = View.GONE
    }

    private fun addSetting(inflater: LayoutInflater, settings: GameSettings, setting: GameSetting) {
        val item = LauncherGameSettingItemBinding.inflate(inflater, binding.gameSettingsList, true)
        item.gameSettingTitle.text = setting.title
        item.gameSettingResetButton.text =
            getString(R.string.game_settings_reset_button, setting.defaultValue.content)

        val showModified = {
            item.gameSettingResetButton.visibility =
                if (settings.isModified(setting)) View.VISIBLE else View.GONE
        }
        val showValue = {
            val value = settings.valueOf(setting)
            if (setting.type == GameSettingType.BOOLEAN) {
                item.gameSettingCheckBox.isChecked = value.content == "true"
            } else if (item.gameSettingEdit.text.toString() != value.content) {
                item.gameSettingEdit.setText(value.content)
                item.gameSettingEdit.error = null
            }
            showModified()
        }

        if (setting.type == GameSettingType.BOOLEAN) {
            item.gameSettingEdit.visibility = View.GONE
            item.gameSettingCheckBox.setOnCheckedChangeListener { _, isChecked ->
                settings.set(setting, setting.parse(isChecked.toString())!!)
                showModified()
            }
        } else {
            item.gameSettingCheckBox.visibility = View.GONE
            item.gameSettingEdit.inputType = when (setting.type) {
                GameSettingType.INTEGER -> InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_SIGNED
                GameSettingType.DECIMAL -> InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_SIGNED or InputType.TYPE_NUMBER_FLAG_DECIMAL
                else -> InputType.TYPE_CLASS_TEXT
            }
            item.gameSettingEdit.addTextChangedListener(object : TextWatcher {
                override fun afterTextChanged(s: Editable) {}
                override fun beforeTextChanged(
                    s: CharSequence, start: Int,
                    count: Int, after: Int
                ) {
                }

                override fun onTextChanged(
                    s: CharSequence, start: Int,
                    before: Int, count: Int
                ) {
                    val value = setting.parse(s.toString())
                    if (value == null) {
                        item.gameSettingEdit.error = getString(R.string.game_settings_invalid_value)
                        return
                    }
                    item.gameSettingEdit.error = null
                    settings.set(setting, value)
                    showModified()
                }
            })
        }
        item.gameSettingResetButton.setOnClickListener {
            settings.reset(setting)
            showValue()
        }

        showValue()
        showValueCallbacks.add(showValue)
    }
}
