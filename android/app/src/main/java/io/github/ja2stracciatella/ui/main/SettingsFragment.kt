package io.github.ja2stracciatella.ui.main

import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import io.github.ja2stracciatella.*
import io.github.ja2stracciatella.databinding.FragmentLauncherSettingsBinding


/**
 * A placeholder fragment containing a simple view.
 */
class SettingsFragment : Fragment() {
    private var _binding: FragmentLauncherSettingsBinding? = null
    private val binding get() = _binding!!

    private lateinit var configurationModel: ConfigurationModel
    private lateinit var scalingQualities: Array<ScalingQuality>

    override fun onCreate(savedInstanceState: Bundle?) {
        configurationModel = ViewModelProvider(requireActivity())[ConfigurationModel::class.java]
        scalingQualities = (ScalingQuality::values)()

        super.onCreate(savedInstanceState)
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentLauncherSettingsBinding.inflate(inflater, container, false)

        val spinnerLabels = scalingQualities.map { it.getLabel() }
        val adapter: ArrayAdapter<String> =
            ArrayAdapter(this.requireContext(), R.layout.launcher_spinner_item, spinnerLabels)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        binding.scalingQualitySpinner.adapter = adapter

        configurationModel.resolution.observe(viewLifecycleOwner) { resolution ->
            if (binding.resolutionWidthEdit.text.toString() != resolution.width.toString()) {
                binding.resolutionWidthEdit.setText(resolution.width.toString())
            }
            if (binding.resolutionHeightEdit.text.toString() != resolution.height.toString()) {
                binding.resolutionHeightEdit.setText(resolution.height.toString())
            }
        }
        binding.resolutionWidthEdit.addTextChangedListener(object : TextWatcher {
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
                if (s.isNotEmpty()) {
                    try {
                        val width = s.toString().toUInt()
                        val current = configurationModel.resolution.value ?: Resolution.DEFAULT
                        if (width != current.width) {
                            configurationModel.setResolution(
                                Resolution(
                                    width,
                                    current.height
                                )
                            )
                        }

                    } catch (_: java.lang.NumberFormatException) {
                    } catch (_: java.lang.NullPointerException) {
                    }
                }
            }
        })
        binding.resolutionHeightEdit.addTextChangedListener(object : TextWatcher {
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
                if (s.isNotEmpty()) {
                    try {
                        val height = s.toString().toUInt()
                        val current = configurationModel.resolution.value ?: Resolution.DEFAULT
                        if (height != current.height) {
                            configurationModel.setResolution(
                                Resolution(
                                    current.width,
                                    height
                                )
                            )
                        }

                    } catch (_: java.lang.NumberFormatException) {
                    } catch (_: java.lang.NullPointerException) {
                    }
                }
            }
        })

        configurationModel.scalingQuality.observe(viewLifecycleOwner) { scalingQuality ->
            val index = scalingQualities.indexOf(scalingQuality)
            binding.scalingQualitySpinner.setSelection(index)
        }
        binding.scalingQualitySpinner.onItemSelectedListener =
            object : AdapterView.OnItemSelectedListener {
                override fun onItemSelected(
                    parent: AdapterView<*>?,
                    view: View?,
                    position: Int,
                    id: Long
                ) {
                    if (position >= 0 && position < scalingQualities.size) {
                        configurationModel.setScalingQuality(scalingQualities[position])
                    }
                }

                override fun onNothingSelected(parent: AdapterView<*>?) {
                }
            }

        binding.resolutionAutoButton.setOnClickListener {
            val launcherActivity = requireActivity()
            if (launcherActivity is LauncherActivity) {
                configurationModel.resolution.value = launcherActivity.getRecommendedResolution()
            }
        }

        return binding.root
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
        fun newInstance(sectionNumber: Int): SettingsFragment {
            return SettingsFragment().apply {
                arguments = Bundle().apply {
                    putInt(ARG_SECTION_NUMBER, sectionNumber)
                }
            }
        }
    }
}
