package io.github.ja2stracciatella.ui.main

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.adapter.FragmentStateAdapter
import io.github.ja2stracciatella.R

/**
 * A [FragmentStateAdapter] that returns a fragment corresponding to
 * one of the sections/tabs/pages.
 */
class SectionsPagerAdapter(fa: FragmentActivity) : FragmentStateAdapter(fa) {
    override fun createFragment(position: Int): Fragment {
        if (position == 0) {
            return DataTabFragment()
        }
        if (position == 1) {
            return SettingsFragment()
        }
        // getItem is called to instantiate the fragment for the given page.
        // Return a PlaceholderFragment (defined as a static inner class below).
        return LogsTabFragment()
    }

    override fun getItemCount(): Int {
        return TAB_TITLES.size
    }

    companion object {
        private val TAB_TITLES = arrayOf(
            R.string.tab_text_data,
            R.string.tab_text_settings,
            R.string.tab_text_logs
        )

        fun getTabTitle(position: Int): Int {
            return TAB_TITLES[position]
        }
    }
}
