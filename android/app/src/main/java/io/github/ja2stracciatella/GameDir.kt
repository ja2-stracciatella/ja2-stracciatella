package io.github.ja2stracciatella

import android.app.AlertDialog
import android.content.Context
import java.io.File

class GameDir {
    companion object {
        fun checkGameDirectoryForCommonMistakes(
            context: Context,
            dir: String?,
            callback: () -> Unit
        ) {
            if (dir != null) {
                val gameDir = File(dir)
                checkForDataDir(context, gameDir) { dataDirSegment ->
                    val dataDir = File(dir, dataDirSegment)
                    checkFor1Dot13(context, dataDir, callback)
                }
            } else {
                AlertDialog.Builder(context)
                    .setTitle(R.string.common_mistakes_no_game_dir_selected_title)
                    .setMessage(R.string.common_mistakes_no_game_dir_selected_description)
                    .setPositiveButton(android.R.string.cancel) { _, _ -> }
                    .show()
            }
        }

        private fun checkForDataDir(context: Context, gameDir: File, callback: (String) -> Unit) {
            // Check for missing data dir
            val searchFor = "Data"
            val found = checkForExistenceCaseInsensitive(gameDir, searchFor)
            if (found == null) {
                AlertDialog.Builder(context)
                    .setTitle(R.string.common_mistakes_no_data_dir_found_title)
                    .setMessage(R.string.common_mistakes_no_data_dir_found_description)
                    .setPositiveButton(android.R.string.cancel) { _, _ -> }
                    .setNegativeButton(R.string.continue_action) { _, _ -> callback(searchFor) }
                    .show()
            } else {
                callback(found)
            }
        }

        private fun checkFor1Dot13(context: Context, dataDir: File, callback: () -> Unit) {
            // Check for 1.13 installation
            val file = checkForExistenceCaseInsensitive(dataDir, "Ja2Set.dat.xml")
            if (file != null) {
                AlertDialog.Builder(context)
                    .setTitle(R.string.common_mistakes_one_dot_thirteen_detected_title)
                    .setMessage(R.string.common_mistakes_one_dot_thirteen_detected_description)
                    .setPositiveButton(android.R.string.cancel) { _, _ -> }
                    .setNegativeButton(R.string.continue_action) { _, _ -> callback() }
                    .show()
            } else {
                callback()
            }
        }

        private fun checkForExistenceCaseInsensitive(
            parent: File,
            fileOrDirectory: String
        ): String? {
            val contents = parent.listFiles()
            if (contents != null) {
                return contents.firstOrNull { c ->
                    c.name.equals(fileOrDirectory, true)
                }?.name
            }
            return null
        }
    }
}
