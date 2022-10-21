package io.github.ja2stracciatella

import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.lifecycle.ViewModelProvider
import com.google.android.material.tabs.TabLayoutMediator
import io.github.ja2stracciatella.databinding.ActivityLauncherBinding
import io.github.ja2stracciatella.ui.main.SectionsPagerAdapter
import kotlinx.serialization.SerializationException
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonPrimitive
import java.io.File
import java.io.IOException


class LauncherActivity : AppCompatActivity() {
    private lateinit var binding: ActivityLauncherBinding

    private val activityLogTag = "LauncherActivity"
    private val requestPermissionsCode = 1000
    private val jsonFormat = Json {
        prettyPrint = true
    }
    private val ja2JsonFilename = ".ja2/ja2.json"
    private val gameDirKey = "game_dir"
    private val saveGameDirKey = "save_game_dir"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityLauncherBinding.inflate(layoutInflater)
        val view = binding.root

        loadJA2Json()

        setContentView(view)
        val sectionsPagerAdapter = SectionsPagerAdapter(this)
        binding.viewPager.adapter = sectionsPagerAdapter

        TabLayoutMediator(binding.tabs, binding.viewPager) { tab, position ->
            tab.text =
                applicationContext.resources.getString(SectionsPagerAdapter.getTabTitle(position))
        }.attach()

        binding.fab.setOnClickListener {
            startGame()
        }
    }

    override fun onResume() {
        super.onResume()

        val exception = NativeExceptionContainer.getException()
        Log.i(activityLogTag, "Resuming LauncherActivity, previous exception: $exception")
        if (exception != null) {
            Toast.makeText(
                this,
                "A exception occurred when running the game: $exception",
                Toast.LENGTH_LONG
            ).show()
            NativeExceptionContainer.resetException()
        }
    }

    private fun getPermissionsIfNecessaryForAction(action: () -> Unit) {
        val permissions = arrayOf(
            android.Manifest.permission.READ_EXTERNAL_STORAGE,
            android.Manifest.permission.WRITE_EXTERNAL_STORAGE
        )
        val hasAllPermissions = permissions.all {
            ContextCompat.checkSelfPermission(
                applicationContext,
                android.Manifest.permission.READ_EXTERNAL_STORAGE
            ) == PackageManager.PERMISSION_GRANTED
        }
        if (hasAllPermissions) {
            action()
        } else {
            requestPermissions(permissions, requestPermissionsCode)
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        if (requestCode == requestPermissionsCode) {
            if (grantResults.all { r -> r == PackageManager.PERMISSION_GRANTED }) {
                startGame()
            } else {
                Toast.makeText(
                    this,
                    "Cannot start the game without proper permissions",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    private fun startGame() {
        try {
            getPermissionsIfNecessaryForAction {
                saveJA2Json()
                NativeExceptionContainer.resetException()
                val intent = Intent(this@LauncherActivity, StracciatellaActivity::class.java)
                startActivity(intent)
            }
        } catch (e: IOException) {
            val message = "Could not write ${ja2JsonPath}: ${e.message}"
            Log.e(activityLogTag, message)
            Toast.makeText(this, message, Toast.LENGTH_SHORT).show()
        }
    }

    private val ja2JsonPath: String
        get() {
            return "${applicationContext.filesDir.absolutePath}/$ja2JsonFilename"
        }

    private fun loadJA2Json() {
        val configurationModel = ViewModelProvider(this)[ConfigurationModel::class.java]
        try {
            val text = File(ja2JsonPath).readText()
            val json: Ja2Json = jsonFormat.decodeFromString(text)

            configurationModel.setVanillaGameDir(json.vanillaGameDir)
            configurationModel.setVanillaGameVersion(json.vanillaGameVersion)
            configurationModel.setSaveGameDir(json.saveGameDir)
        } catch (e: SerializationException) {
            Log.w(activityLogTag, "Could not decode ja2.json: ${e.message}")
        } catch (e: IOException) {
            Log.w(activityLogTag, "Could not read $ja2JsonPath: ${e.message}")
        }
    }

    private fun saveJA2Json() {
        val configurationModel = ViewModelProvider(this)[ConfigurationModel::class.java]
        val json = Ja2Json(
            configurationModel.vanillaGameDir.value,
            configurationModel.vanillaGameVersion.value,
            configurationModel.saveGameDir.value
        )
        val parentDir = File(ja2JsonPath).parentFile
        if (parentDir?.exists() != true) {
            parentDir?.mkdirs()
        }
        File(ja2JsonPath).writeText(jsonFormat.encodeToString(json))
    }
}
