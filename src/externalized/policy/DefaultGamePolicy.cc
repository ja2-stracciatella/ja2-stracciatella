#include "DefaultGamePolicy.h"
#include "JsonObject.h"

#include "game/Tactical/Item_Types.h"

DefaultGamePolicy::DefaultGamePolicy(rapidjson::Document *json)
{
	JsonObjectReader gp = JsonObjectReader(json->GetObject());
	extra_hotkeys = gp.getOptionalBool("extra_hotkeys", true);
	can_enter_turnbased = gp.getOptionalBool("can_enter_turnbased");
	middle_mouse_look = gp.getOptionalBool("middle_mouse_look", true);

	f_draw_item_shadow = gp.getOptionalBool("draw_item_shadow", true);
	ms_per_game_cycle = gp.getOptionalInt("ms_per_game_cycle", 25);

	starting_cash_easy = gp.getOptionalInt("starting_cash_easy", 45000);
	starting_cash_medium = gp.getOptionalInt("starting_cash_medium", 35000);
	starting_cash_hard = gp.getOptionalInt("starting_cash_hard", 30000);

	f_drop_everything = gp.getOptionalBool("drop_everything");
	f_all_dropped_visible = gp.getOptionalBool("all_drops_visible");

	multiple_interrupts = gp.getOptionalBool("multiple_interrupts");

	enemy_weapon_minimal_status = gp.getOptionalInt("enemy_weapon_minimal_status", 0);

	JsonObjectReader ai = JsonObjectReader(gp.GetValue("ai"));
	ai_better_aiming_choice = ai.getOptionalBool("better_aiming_choice");
	ai_go_prone_more_often = ai.getOptionalBool("go_prone_more_often");
	threshold_cth_head = gp.getOptionalInt("threshold_cth_head", 67);
	threshold_cth_legs = gp.getOptionalInt("threshold_cth_legs", 67);
	enemy_elite_minimum_level = gp.getOptionalInt("enemy_elite_minimum_level", 6);
	enemy_elite_maximum_level = gp.getOptionalInt("enemy_elite_maximum_level", 10);

	gui_extras = gp.getOptionalBool("gui_extras", true);
	extra_attachments = gp.getOptionalBool("extra_attachments");

	pablo_wont_steal = gp.getOptionalBool("pablo_wont_steal");

	critical_damage_head_multiplier = gp.getOptionalDouble("tactical_head_damage_multiplier", 1.5);
	critical_damage_legs_multiplier = gp.getOptionalDouble("tactical_legs_damage_multiplier", 0.5);
	chance_to_hit_maximum = gp.getOptionalInt("chance_to_hit_maximum", 99);
	chance_to_hit_minimum = gp.getOptionalInt("chance_to_hit_minimum", 1);

	JsonObjectReader imp = JsonObjectReader(gp.GetValue("imp"));
	imp_load_saved_merc_by_nickname = imp.getOptionalBool("load_saved_merc_by_nickname");
	imp_load_keep_inventory = imp.getOptionalBool("load_keep_inventory");
	imp_attribute_max = imp.getOptionalInt("max_attribute_points", 85);
	imp_attribute_min = imp.getOptionalInt("min_attribute_points", 35);
	imp_attribute_zero_bonus = imp.getOptionalInt("zero_attribute_points_bonus", 15);
	imp_attribute_bonus = imp.getOptionalInt("bonus_attribute_points", 40);
	imp_pick_skills_directly = imp.getOptionalBool("pick_skills_directly");

	JsonObjectReader progress = JsonObjectReader(gp.GetValue("progress"));
	progress_event_madlab_min = progress.getOptionalInt("event_madlab_min", 35);
	progress_event_mike_min = progress.getOptionalInt("event_mike_min", 50);
	progress_event_iggy_min = progress.getOptionalInt("event_iggy_min", 70);

	kills_per_point_0 = progress.getOptionalInt("kills_per_point_0", 7);
	kills_per_point_1 = progress.getOptionalInt("kills_per_point_1", 10);
	kills_per_point_2 = progress.getOptionalInt("kills_per_point_2", 15);

	progress_weight_kills = progress.getOptionalDouble("weight_kills", 25.0);
	progress_weight_control = progress.getOptionalDouble("weight_control", 25.0);
	progress_weight_income = progress.getOptionalDouble("weight_income", 50.0);

	unhired_merc_deaths_difficulty_0 = gp.getOptionalInt("unhired_merc_deaths_difficulty_0", 1);
	unhired_merc_deaths_difficulty_1 = gp.getOptionalInt("unhired_merc_deaths_difficulty_1", 2);
	unhired_merc_deaths_difficulty_2 = gp.getOptionalInt("unhired_merc_deaths_difficulty_2", 3);
}

/** Check if a hotkey is enabled. */
bool DefaultGamePolicy::isHotkeyEnabled(UIMode mode, HotkeyModifier modifier, uint32_t key) const
{
	if(mode == UI_Tactical)
	{
		if(modifier == HKMOD_None)
		{
			switch(key)
			{
				case 'j':         return extra_hotkeys;
			}
		}
		else if(modifier == HKMOD_CTRL)
		{
			switch(key)
			{
				case 'n':
				case 'q':         return extra_hotkeys;
			}
		}
		else if(modifier == HKMOD_SHIFT)
		{
			switch(key)
			{
				case 'j':         return extra_hotkeys;
			}
		}
		else if(modifier == HKMOD_ALT)
		{
			switch(key)
			{
				case 'r':         return extra_hotkeys;
			}
		}
		else if(modifier == HKMOD_CTRL_SHIFT)
		{
			switch(key)
			{
				case 'r':         return extra_hotkeys;
			}
		}
	}

	if(mode == UI_Map)
	{
		if(modifier == HKMOD_CTRL)
		{
			switch(key)
			{
				case 'i':         return extra_hotkeys;
			}
		}
	}

	return false;
}
