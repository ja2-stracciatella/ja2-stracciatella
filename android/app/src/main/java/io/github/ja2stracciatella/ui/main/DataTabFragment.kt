package io.github.ja2stracciatella.ui.main

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import com.codekidlabs.storagechooser.StorageChooser
import io.github.ja2stracciatella.ConfigurationModel
import io.github.ja2stracciatella.R
import kotlinx.android.synthetic.main.fragment_launcher_data_tab.view.*


/**
 * A placeholder fragment containing a simple view.
 */
class DataTabFragment : Fragment() {
    private lateinit var configurationModel: ConfigurationModel
    private lateinit var chooser: StorageChooser

    override fun onCreate(savedInstanceState: Bundle?) {
        configurationModel = ViewModelProvider(requireActivity()).get(ConfigurationModel::class.java)
        chooser = StorageChooser.Builder()
            .withActivity(activity)
            .withFragmentManager(activity?.fragmentManager)
            .allowCustomPath(true)
            .setType(StorageChooser.DIRECTORY_CHOOSER)
            .build();
        chooser.setOnSelectListener { path ->
            Log.e("TAG", "SELECTED_PATH $path")
            configurationModel.apply {
                setVanillaGameDir(path)
            }
        }
        super.onCreate(savedInstanceState)
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val tab = inflater.inflate(R.layout.fragment_launcher_data_tab, container, false)

        configurationModel.vanillaGameDir.observe(
            viewLifecycleOwner,
            Observer<String> { vanillaGameDir ->
                tab.gameDirValueText.text = vanillaGameDir
            })
        tab.gameDirChooseButton?.setOnClickListener { _ ->
            chooser.show()
        }
        return tab
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