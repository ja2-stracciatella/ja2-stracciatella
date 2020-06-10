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
	bool load_saved_npc;                  // NPC merc that was hired gets loaded from last save game if was saved while also enabled
	bool load_saved_npc_inventory;        // NPC merc gets inventory from last save game if was saved while also enabled

	bool ironman_can_save_realtime;       // Save during real-time even when enemies present "clayman"

	bool accurate_aim_circle;             // Aim circle size and colour corresponds to chance_to_hit

	bool all_skilltraits_mode;            // Allow soldier to get every perk available
	bool all_skilltraits_just_ours;       // limit to player soldiers
	int8_t all_skilltraits_wisdom;        // Wisdom required to qualify. Above 100 disables
	int8_t all_skilltraits_level;         // Level required to qualify. Above 10 disables
	bool enemy_elite_all_traits;          // Elites from queen's army get all perks

	bool inventory_management_extras;     // 1.13 inventory management hotkeys, shift-f, etc.
	bool ime_detach_ammo;                 // hotkey detaches magazines from sector weapons
	bool ime_detach_attachments;          // hotkey removes removable attachments from sector weapons
	bool ime_refill_canteens;             // hotkey refill sector canteens, drinking does not deplete canteens out of combat
	bool vehicle_inventory;               // Vehicles have accessable inventories


	bool pablo_wont_steal;                // Packages not stolen

	float critical_damage_head_multiplier;//Head damage multiplier. Vanilla 1.5
	float critical_damage_legs_multiplier;//Legs damage multiplier. Vanilla 0.5
	int8_t chance_to_hit_minimum;         //Minimum chance to hit (0 - chance_to_hit_maximum) vanilla 1
	int8_t chance_to_hit_maximum;         //Maximum chance to hit (chance_to_hit_minimum - 100) vanilla 99

	int8_t damage_minimum;                // Minimum damage taken i.e. hit by glowstick. Vanilla 1
	int8_t aim_bonus_per_std_ap;          // Aim bonus % for first 4 AP (aim clicks) spent. Vanilla 10.
	int8_t aim_bonus_sniperscope;         // Flat bonus after at suitable range. Vanilla 20
	int8_t range_bonus_silencer;          // Absolute bonus, negative for penalty, to range from silencer attachment. 10 per 1 tile. Vanilla 0.
	int8_t aim_bonus_per_extra_ap;        // Aim bonus % for extra AP (aim clicks) spent. Not vanilla.
	int8_t extra_ap_max_sniperscope;      // bonus to maximum AP that can be spent aiming to improve chance to hit. Vanilla 0
	int8_t extra_ap_max_bipod;            // bonus to maximum AP that can be spent aiming to improve chance to hit. Vanilla 0
	int8_t extra_ap_max_bipod_scope_combined; // bonus to maximum AP that can be spent aiming to improve chance to hit. Vanilla 0

	bool rocket_rifle_builtin_bipod;         // rocket launcher can only use built-in attachments. Vanilla false
	bool rocket_rifle_builtin_sniperscope;   // rocket launcher can only use built-in attachments. Vanilla false

	/* IMP */
	int8_t imp_attribute_max;             // IMP character attribute maximum 0 to 100, vanilla 85
	int8_t imp_attribute_min;             // IMP character attribute minimum 0 to imp_attribute_max, vanilla 35
	int32_t imp_attribute_bonus;          // IMP character attribute unallocated bonus points, vanilla 40
	int32_t imp_attribute_zero_bonus;     // IMP character attribute points given instead of imp_attribute_min, vanilla 15
	bool imp_pick_skills_directly;        // Use the IMP_SkillTrait selection screen from JA2.5, skipping the personality quiz, vanilla falase

	////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////
};
