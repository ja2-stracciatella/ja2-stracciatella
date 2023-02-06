package io.github.ja2stracciatella.ui.main

import android.content.ClipData
import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import io.github.ja2stracciatella.R
import io.github.ja2stracciatella.databinding.FragmentLauncherLogsTabBinding
import java.io.File


class LogsTabFragment : Fragment() {
    private var _binding: FragmentLauncherLogsTabBinding? = null
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentLauncherLogsTabBinding.inflate(inflater, container, false)

        binding.logsCopyToClipboardButton.setOnClickListener {
            val text = binding.logsText.text
            val clipboard =
                requireContext().getSystemService(Context.CLIPBOARD_SERVICE) as android.content.ClipboardManager
            val clip = ClipData.newPlainText(resources.getText(R.string.logs_copied_to_clipboard_name), text)
            clipboard.setPrimaryClip(clip)
            Toast.makeText(requireContext(), resources.getText(R.string.logs_copied_to_clipboard_toast), Toast.LENGTH_SHORT).show()
        }

        return binding.root
    }

    override fun onResume() {
        super.onResume()

        val logFile = File(activity?.cacheDir, "ja2.log")
        if (logFile.exists()) {
            val content = logFile.readText()
            binding.logsText.text = content
        }
    }
}
