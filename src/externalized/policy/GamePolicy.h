#pragma once

#include <stdint.h>

enum UIMode
{
	UI_Tactical,
	UI_Map
};

enum HotkeyModifier
{
	HKMOD_None,
	HKMOD_CTRL,
	HKMOD_SHIFT,
	HKMOD_ALT,
	HKMOD_CTRL_SHIFT
};

#define gamepolicy(element) (GCM->getGamePolicy()->element)

class GamePolicy
{
public:
	/** Check if a hotkey is enabled. */
	virtual bool isHotkeyEnabled(UIMode mode, HotkeyModifier modifier, uint32_t key) const = 0;
	// this could be defaulted in C++11
	virtual ~GamePolicy() {}

	bool f_draw_item_shadow;              /**< Draw shadows from the inventory items. */

	int32_t ms_per_game_cycle;            /**< Milliseconds per game cycle. */

	int32_t starting_cash_easy;
	int32_t starting_cash_medium;
	int32_t starting_cash_hard;

	/* Battle */
	bool f_drop_everything;               /**< Enemy drop all equipment. */
	bool f_all_dropped_visible;           /**< All dropped equipment is visible right away. */

	bool multiple_interrupts;             // can interrupt more than once per turn

	int8_t enemy_weapon_minimal_status;   /**< Minimal status of the enemy weapon (0 - 100). */

	bool gui_extras;                      /* graphical user interface cosmetic mod */
	bool extra_attachments;               // allow more item attachments options

	bool middle_mouse_look;               // Look cursor with middle mouse button
	bool can_enter_turnbased;             // 'd' can start turnbased if in real-time

	bool ai_better_aiming_choice;         // decide where to shoot depending on to-hit probability if random choice is being made
	bool ai_go_prone_more_often;          // especially when already facing the right direction
	int8_t threshold_cth_head;            // threshold AI always take head shots, increase game difficulty
	int8_t threshold_cth_legs;            // threshold AI switch to leg shots from torso
	int8_t enemy_elite_minimum_level;     // increase challenge: minimum experience level for enemy elite soldier
	int8_t enemy_elite_maximum_level;     // maximum experience level for enemy elite soldier

	bool imp_load_saved_merc_by_nickname; // IMP merc is saved and can be loaded at IMP creation if has same nickname
	bool imp_load_keep_inventory;         // IMP merc gets inventory from last save game
	bool pablo_wont_steal;                // Packages not stolen

	float critical_damage_head_multiplier;//Head damage multiplier. Vanilla 1.5
	float critical_damage_legs_multiplier;//Legs damage multiplier. Vanilla 0.5
	int8_t chance_to_hit_minimum;         //Minimum chance to hit (0 - chance_to_hit_maximum) vanilla 1
	int8_t chance_to_hit_maximum;         //Maximum chance to hit (chance_to_hit_minimum - 100) vanilla 99

	/* IMP */
	int8_t imp_attribute_max;             // IMP character attribute maximum 0 to 100, vanilla 85
	int8_t imp_attribute_min;             // IMP character attribute minimum 0 to imp_attribute_max, vanilla 35
	int32_t imp_attribute_bonus;          // IMP character attribute unallocated bonus points, vanilla 40
	int32_t imp_attribute_zero_bonus;     // IMP character attribute points given instead of imp_attribute_min, vanilla 15
	bool imp_pick_skills_directly;        // Use the IMP_SkillTrait selection screen from JA2.5, skipping the personality quiz, vanilla falase

	// Difficulty / Campaign Progress
	float progress_weight_kills;         // Weight of kill count on campaign progress
	float progress_weight_control;       // Weight of area control on campaign progress
	float progress_weight_income;        // Weight of income on campaign progress
	int8_t kills_per_point_0;             // Kills per point for difficulty Easy
	int8_t kills_per_point_1;             // Kills per point for difficulty Medium
	int8_t kills_per_point_2;             // Kills per point for difficulty Hard
	int8_t progress_event_madlab_min;     // Minimum first progress to trigger event Quest Madlab
	int8_t progress_event_mike_min;       // Minimum first progress to trigger event Mike
	int8_t progress_event_iggy_min;       // Minimum first progress to trigger event Iggy

	int8_t unhired_merc_deaths_difficulty_0;       // Maximum unhired mercs KIA difficulty Easy
	int8_t unhired_merc_deaths_difficulty_1;       // Maximum unhired mercs KIA difficulty Medium
	int8_t unhired_merc_deaths_difficulty_2;       // Maximum unhired mercs KIA difficulty Hard

	////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////
};
