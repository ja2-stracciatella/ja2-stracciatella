package io.github.ja2stracciatella.ui.main

import android.content.pm.PackageManager
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.codekidlabs.storagechooser.StorageChooser
import io.github.ja2stracciatella.ConfigurationModel
import io.github.ja2stracciatella.R
import io.github.ja2stracciatella.VanillaVersion
import io.github.ja2stracciatella.databinding.FragmentLauncherDataTabBinding


/**
 * A placeholder fragment containing a simple view.
 */
class DataTabFragment : Fragment() {
    private var _binding: FragmentLauncherDataTabBinding? = null
    private val binding get() = _binding!!

    // Request permissions for game dir
    private val requestPermissionsCodeGameDir = 1001
    // Request permissions for save dir
    private val requestPermissionsCodeSaveGameDir = 1002

    private lateinit var configurationModel: ConfigurationModel
    private lateinit var versions: Array<VanillaVersion>

    override fun onCreate(savedInstanceState: Bundle?) {
        configurationModel = ViewModelProvider(requireActivity())[ConfigurationModel::class.java]
        versions = (VanillaVersion::values)()

        super.onCreate(savedInstanceState)
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentLauncherDataTabBinding.inflate(inflater, container, false)

        val versionLabels = versions.map { v: VanillaVersion -> v.getLabel() }
        val spinnerLabels = listOf(getString(R.string.game_version_empty_text)) + versionLabels
        val adapter: ArrayAdapter<String> = ArrayAdapter(this.requireContext(), R.layout.launcher_spinner_item, spinnerLabels)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.gameVersionSpinner.adapter = adapter

        configurationModel.vanillaGameDir.observe(
            viewLifecycleOwner
        ) { vanillaGameDir ->
            if (vanillaGameDir != null) {
                binding.gameDirValueText.text = vanillaGameDir
            }
        }
        configurationModel.vanillaGameVersion.observe(
            viewLifecycleOwner
        ) { vanillaGameVersion ->
            if (vanillaGameVersion == null) {
                binding.gameVersionSpinner.setSelection(0)
            } else {
                val index = versions.indexOf(vanillaGameVersion)
                binding.gameVersionSpinner.setSelection(index + 1)
            }
        }
        configurationModel.saveGameDir.observe(
            viewLifecycleOwner
        ) { saveGameDir ->
            if (saveGameDir != null) {
                binding.saveGameDirValueText.text = saveGameDir
            }
        }
        binding.gameDirChooseButton.setOnClickListener {
            showGameDirChooser()
        }
        binding.gameVersionSpinner.onItemSelectedListener =
            object : AdapterView.OnItemSelectedListener {
                override fun onItemSelected(
                    parent: AdapterView<*>?,
                    view: View?,
                    position: Int,
                    id: Long
                ) {
                    if (position == 0) {
                        configurationModel.setVanillaGameVersion(null)
                    } else {
                        configurationModel.setVanillaGameVersion(versions[position-1])
                    }
                }

                override fun onNothingSelected(parent: AdapterView<*>?) {
                }
            }
        binding.saveGameDirChooseButton.setOnClickListener {
            showSaveGameDirChooser()
        }
        return binding.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    private fun showGameDirChooser() {
        getPermissionsIfNecessaryForAction(requestPermissionsCodeGameDir) {
            // We suppress deprecation for .fragmentManager here as we need the android.app.FragmentManager
            // and not androidx.fragment.app.FragmentManager for the Storage Chooser
            @Suppress("DEPRECATION")
            val directoryChooser = StorageChooser.Builder()
                .withActivity(activity)
                .withFragmentManager(activity?.fragmentManager)
                .allowCustomPath(true)
                .setType(StorageChooser.DIRECTORY_CHOOSER)
                .build()
            directoryChooser.setOnSelectListener { path ->
                configurationModel.apply {
                    setVanillaGameDir(path)
                }
            }
            directoryChooser.show()
        }
    }

    private fun showSaveGameDirChooser() {
        getPermissionsIfNecessaryForAction(requestPermissionsCodeSaveGameDir) {
            // We suppress deprecation for .fragmentManager here as we need the android.app.FragmentManager
            // and not androidx.fragment.app.FragmentManager for the Storage Chooser
            @Suppress("DEPRECATION")
            val directoryChooser = StorageChooser.Builder()
                .withActivity(activity)
                .withFragmentManager(activity?.fragmentManager)
                .allowCustomPath(true)
                .setType(StorageChooser.DIRECTORY_CHOOSER)
                .build()
            directoryChooser.setOnSelectListener { path ->
                configurationModel.apply {
                    setSaveGameDir(path)
                }
            }
            directoryChooser.show()
        }
    }

    private fun getPermissionsIfNecessaryForAction(permissionsCode: Int, action: () -> Unit) {
        val permissions = arrayOf(android.Manifest.permission.READ_EXTERNAL_STORAGE, android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
        val hasAllPermissions = permissions.all { ContextCompat.checkSelfPermission(requireContext(), android.Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED }
        if (hasAllPermissions)  {
            action()
        } else {
            requestPermissions(permissions, permissionsCode)
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        if (requestCode == requestPermissionsCodeGameDir) {
            if (grantResults.all { r -> r == PackageManager.PERMISSION_GRANTED }) {
                showGameDirChooser()
            } else {
                Toast.makeText(requireContext(), "Cannot select game directory without proper permissions", Toast.LENGTH_SHORT).show()
            }
        }
        if (requestCode == requestPermissionsCodeSaveGameDir) {
            if (grantResults.all { r -> r == PackageManager.PERMISSION_GRANTED }) {
                showSaveGameDirChooser()
            } else {
                Toast.makeText(requireContext(), "Cannot select save game directory without proper permissions", Toast.LENGTH_SHORT).show()
            }
        }
    }

    companion object {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        private const val ARG_SECTION_NUMBER = "section_number"

        /**
         * Returns a new instance of this fragment for the given section
         * number.
         */
        @JvmStatic
        fun newInstance(sectionNumber: Int): DataTabFragment {
            return DataTabFragment().apply {
                arguments = Bundle().apply {
                    putInt(ARG_SECTION_NUMBER, sectionNumber)
                }
            }
        }
    }
}
