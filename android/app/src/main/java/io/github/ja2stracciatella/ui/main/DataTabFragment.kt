package io.github.ja2stracciatella.ui.main

import android.content.pm.PackageManager
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.codekidlabs.storagechooser.StorageChooser
import io.github.ja2stracciatella.ConfigurationModel
import io.github.ja2stracciatella.R
import kotlinx.android.synthetic.main.fragment_launcher_data_tab.view.*


/**
 * A placeholder fragment containing a simple view.
 */
class DataTabFragment : Fragment() {
    // Request permissions for game dir
    private val requestPermissionsCodeGameDir = 1001
    // Request permissions for save dir
    private val requestPermissionsCodeSaveGameDir = 1002

    private lateinit var configurationModel: ConfigurationModel

    override fun onCreate(savedInstanceState: Bundle?) {
        configurationModel = ViewModelProvider(requireActivity()).get(ConfigurationModel::class.java)
        super.onCreate(savedInstanceState)
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val tab = inflater.inflate(R.layout.fragment_launcher_data_tab, container, false)

        configurationModel.vanillaGameDir.observe(
            viewLifecycleOwner
        ) { vanillaGameDir ->
            if (vanillaGameDir.isNotEmpty()) {
                tab.gameDirValueText.text = vanillaGameDir
            }
        }
        configurationModel.saveGameDir.observe(
            viewLifecycleOwner
        ) { saveGameDir ->
            if (saveGameDir.isNotEmpty()) {
                tab.saveGameDirValueText.text = saveGameDir
            }
        }
        tab.gameDirChooseButton?.setOnClickListener { _ ->
            showGameDirChooser()
        }
        tab.saveGameDirChooseButton?.setOnClickListener { _ ->
            showSaveGameDirChooser()
        }
        return tab
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
                .build();
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
                .build();
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
