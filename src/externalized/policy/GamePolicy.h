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

	int32_t target_fps;
	double game_durations_multiplier;

	int32_t starting_cash_easy;
	int32_t starting_cash_medium;
	int32_t starting_cash_hard;

	uint16_t squad_size;

	/* Battle */
	bool f_drop_everything;               /**< Enemy drop all equipment. */
	bool f_all_dropped_visible;           /**< All dropped equipment is visible right away. */

	bool multiple_interrupts;             // can interrupt more than once per turn

	int8_t enemy_weapon_minimal_status;   /**< Minimal status of the enemy weapon (0 - 100). */

	bool gui_extras;                      /* graphical user interface cosmetic mod */
	bool informative_popups;              /* Reveal modifiers in hover popup boxes */
	bool extra_attachments;               // allow more item attachments options
	bool skip_sleep_explanation;          // skip annoying popups

	bool middle_mouse_look;               // Look cursor with middle mouse button
	bool can_enter_turnbased;             // 'd' can start turnbased if in real-time

	bool ai_better_aiming_choice;         // decide where to shoot depending on to-hit probability if random choice is being made
	bool ai_go_prone_more_often;          // especially when already facing the right direction
	int8_t threshold_cth_head;            // threshold AI always take head shots, increase game difficulty
	int8_t threshold_cth_legs;            // threshold AI switch to leg shots from torso

	bool avoid_ambushes;                  // AI able to recognize and avoid ambushes on seeing friendlies' corpses
	bool stay_on_rooftop;                 // AI on guard on rooftop are disallowed to go down

	int8_t enemy_elite_minimum_level;     // increase challenge: minimum experience level for enemy elite soldier
	int8_t enemy_elite_maximum_level;     // maximum experience level for enemy elite soldier

	bool imp_load_saved_merc_by_nickname; // IMP merc is saved and can be loaded at IMP creation if has same nickname
	bool imp_load_keep_inventory;         // IMP merc gets inventory from last save game
	bool pablo_wont_steal;                // Packages not stolen

	float critical_damage_head_multiplier;//Head damage multiplier. Vanilla 1.5
	float critical_damage_legs_multiplier;//Legs damage multiplier. Vanilla 0.5
	int8_t chance_to_hit_minimum;         //Minimum chance to hit (0 - chance_to_hit_maximum) vanilla 1
	int8_t chance_to_hit_maximum;         //Maximum chance to hit (chance_to_hit_minimum - 100) vanilla 99

	int8_t aim_bonus_per_std_ap;          // Aim bonus % for first 4 AP (aim clicks) spent
	int8_t aim_bonus_sniperscope;         // Flat bonus after at suitable range
	int8_t aim_bonus_laserscope;          // Aim bonus in the dark
	int16_t range_penalty_silencer;        // Absolute penalty to range from silencer
	int16_t range_bonus_barrel_extender;   // Aim bonus from extender

	bool always_show_cursor_in_tactical;  // Always show mouse cursor during tactical view (if false, no mourse cursor is shown when moving in real-time mode, selecting a merc, etc)
	bool show_hit_chance;                 // Show chance-to-hit when pressing 'F' and next to mouse cursor when preparing an attack

	float website_loading_time_scale;     // Scales the loading time of websites on the laptop. Lower value means faster loading. Setting this to 0.0 removes the loading entirely.

	bool diagonally_interactable_doors;   // Open doors without exposing your mercs too much. Also affects switches.

	/* IMP */
	int8_t imp_attribute_max;             // IMP character attribute maximum 0 to 100, vanilla 85
	int8_t imp_attribute_min;             // IMP character attribute minimum 0 to imp_attribute_max, vanilla 35
	int32_t imp_attribute_bonus;          // IMP character attribute unallocated bonus points, vanilla 40
	int32_t imp_attribute_zero_bonus;     // IMP character attribute points given instead of imp_attribute_min, vanilla 15
	bool imp_pick_skills_directly;        // Use the IMP_SkillTrait selection screen from JA2.5, skipping the personality quiz, vanilla falase

	/* M.E.R.C. */
	uint8_t merc_online_min_days;         // The earliest day on or after which M.E.R.C. goes online
	uint8_t merc_online_max_days;         // The latest day on or before which M.E.R.C. goes online

	// Difficulty / Campaign Progress
	float progress_weight_kills;         // Weight of kill count on campaign progress
	float progress_weight_control;       // Weight of area control on campaign progress
	float progress_weight_income;        // Weight of income on campaign progress
	int8_t kills_per_point_easy;             // Kills per point for difficulty Easy
	int8_t kills_per_point_medium;             // Kills per point for difficulty Medium
	int8_t kills_per_point_hard;             // Kills per point for difficulty Hard
	int8_t progress_event_madlab_min;     // Minimum first progress to trigger event Quest Madlab
	int8_t progress_event_mike_min;       // Minimum first progress to trigger event Mike
	int8_t progress_event_iggy_min;       // Minimum first progress to trigger event Iggy

	int8_t unhired_merc_deaths_easy;       // Maximum unhired mercs KIA difficulty Easy
	int8_t unhired_merc_deaths_medium;       // Maximum unhired mercs KIA difficulty Medium
	int8_t unhired_merc_deaths_hard;       // Maximum unhired mercs KIA difficulty Hard

	uint16_t start_sector;        // Starting sector
	bool reveal_start_sector;     // Should the start sector radar map be shown at start

	////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////
};
