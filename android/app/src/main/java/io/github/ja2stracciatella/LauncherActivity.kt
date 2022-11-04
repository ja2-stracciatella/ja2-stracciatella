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
    private lateinit var configurationModel: ConfigurationModel

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        configurationModel = ViewModelProvider(this)[ConfigurationModel::class.java]
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

    fun getRecommendedResolution(): Resolution {
        val screenWidth =
            Integer.max(resources.displayMetrics.widthPixels, resources.displayMetrics.heightPixels)
        val screenHeight =
            Integer.min(resources.displayMetrics.widthPixels, resources.displayMetrics.heightPixels)
        val scalingX = screenWidth.toDouble() / Resolution.DEFAULT.width.toDouble()
        val scalingY = screenHeight.toDouble() / Resolution.DEFAULT.height.toDouble()
        val scaling = java.lang.Double.min(scalingX, scalingY)

        if (configurationModel.scalingQuality.value == ScalingQuality.PERFECT) {
            val scalingInt = scaling.toInt()
            val width = Resolution.DEFAULT.width + ((screenWidth - Resolution.DEFAULT.width.toInt() * scalingInt) / scalingInt).toUInt()
            val height = Resolution.DEFAULT.height + ((screenHeight - Resolution.DEFAULT.height.toInt() * scalingInt) / scalingInt).toUInt()
            return Resolution(width - (width % 2u), height - (height % 2u))
        }
        val width = Resolution.DEFAULT.width + ((screenWidth - Resolution.DEFAULT.width.toInt() * scaling) / scaling).toUInt()
        return Resolution(width - (width % 2u), Resolution.DEFAULT.height)
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
        try {
            val text = File(ja2JsonPath).readText()
            val json: Ja2Json = jsonFormat.decodeFromString(text)

            configurationModel.setVanillaGameDir(json.vanillaGameDir)
            configurationModel.setSaveGameDir(json.saveGameDir)

            if (json.vanillaGameVersion != null) {
                configurationModel.setVanillaGameVersion(json.vanillaGameVersion)
            } else {
                configurationModel.setVanillaGameVersion(VanillaVersion.DEFAULT)
            }
            if (json.scalingQuality != null) {
                configurationModel.setScalingQuality(json.scalingQuality)
            } else {
                configurationModel.setScalingQuality(ScalingQuality.DEFAULT)
            }
            if (json.resolution != null) {
                configurationModel.setResolution(json.resolution)
            } else {
                configurationModel.setResolution(getRecommendedResolution())
            }
        } catch (e: SerializationException) {
            Log.w(activityLogTag, "Could not decode ja2.json: ${e.message}")
            configurationModel.setVanillaGameVersion(VanillaVersion.ENGLISH)
            configurationModel.setScalingQuality(ScalingQuality.DEFAULT)
            configurationModel.setResolution(getRecommendedResolution())
        } catch (e: IOException) {
            Log.w(activityLogTag, "Could not read $ja2JsonPath: ${e.message}")
            configurationModel.setVanillaGameVersion(VanillaVersion.ENGLISH)
            configurationModel.setScalingQuality(ScalingQuality.DEFAULT)
            configurationModel.setResolution(getRecommendedResolution())
        }
    }

    private fun saveJA2Json() {
        val json = Ja2Json(
            configurationModel.vanillaGameDir.value,
            configurationModel.vanillaGameVersion.value,
            configurationModel.saveGameDir.value,
            configurationModel.resolution.value,
            configurationModel.scalingQuality.value
        )
        val parentDir = File(ja2JsonPath).parentFile
        if (parentDir?.exists() != true) {
            parentDir?.mkdirs()
        }
        File(ja2JsonPath).writeText(jsonFormat.encodeToString(json))
    }
}
