package io.github.ja2stracciatella

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.ViewModelProvider
import androidx.viewpager.widget.ViewPager
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.tabs.TabLayout
import io.github.ja2stracciatella.ui.main.SectionsPagerAdapter
import java.io.File


class LauncherActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_launcher)
        val sectionsPagerAdapter = SectionsPagerAdapter(this, supportFragmentManager)
        val viewPager: ViewPager = findViewById(R.id.view_pager)
        viewPager.adapter = sectionsPagerAdapter
        val tabs: TabLayout = findViewById(R.id.tabs)
        tabs.setupWithViewPager(viewPager)
        val fab: FloatingActionButton = findViewById(R.id.fab)

        fab.setOnClickListener { _ ->
            saveJA2Json()
            val intent = Intent(this@LauncherActivity, StracciatellaActivity::class.java) // (1) (2)
            startActivity(intent)
        }
    }

    private fun getJa2JsonPath(): String {
        return "${applicationContext.filesDir.absolutePath}/ja2.json"
    }

    private fun saveJA2Json() {
        val configurationModel = ViewModelProvider(this).get(ConfigurationModel::class.java)
        File(getJa2JsonPath()).writeText("""
            { "game_dir": "${configurationModel.vanillaGameDir.value}" }
        """.trimIndent())
    }
}