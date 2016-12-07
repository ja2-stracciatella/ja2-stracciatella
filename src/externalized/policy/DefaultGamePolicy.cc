#include "DefaultGamePolicy.h"

#include "src/game/Tactical/Item_Types.h"

DefaultGamePolicy::DefaultGamePolicy(rapidjson::Document *json)
{
  map_hotkey_ctrl_i     = (*json)["map_hotkey_ctrl_i"].GetBool();
  tactical_hotkey_j     = (*json)["tactical_hotkey_j"].GetBool();
  tactical_hotkey_shift_j   = (*json)["tactical_hotkey_shift_j"].GetBool();
  tactical_hotkey_ctrl_n    = (*json)["tactical_hotkey_ctrl_n"].GetBool();
  tactical_hotkey_ctrl_q    = (*json)["tactical_hotkey_ctrl_q"].GetBool();
  tactical_hotkey_alt_r     = (*json)["tactical_hotkey_alt_r"].GetBool();
  middle_mouse_look         = (*json)["middle_mouse_look"].GetBool();
  can_enter_turnbased       = (*json)["can_enter_turnbased"].GetBool();

  f_draw_item_shadow    = (*json)["draw_item_shadow"].GetBool();

  ms_per_game_cycle     = (*json)["ms_per_game_cycle"].GetInt();

  starting_cash_easy    = (*json)["starting_cash_easy"].GetInt();
  starting_cash_medium  = (*json)["starting_cash_medium"].GetInt();
  starting_cash_hard    = (*json)["starting_cash_hard"].GetInt();

  f_drop_everything     = (*json)["drop_everything"].GetBool();
  f_all_dropped_visible = (*json)["all_drops_visible"].GetBool();

  multiple_interrupts = (*json)["multiple_interrupts"].GetBool();

  gui_extras            = (*json)["gui_extras"].GetBool();

  enemy_weapon_minimal_status   = (*json)["enemy_weapon_minimal_status"].GetInt();

  ai_better_aiming_choice   = (*json)["ai"]["better_aiming_choice"].GetBool();
  ai_go_prone_more_often    = (*json)["ai"]["go_prone_more_often"].GetBool();
  threshold_cth_head        = (*json)["threshold_cth_head"].GetInt();
  threshold_cth_legs        = (*json)["threshold_cth_legs"].GetInt();
  enemy_elite_minimum_level = (*json)["enemy_elite_minimum_level"].GetInt();
  enemy_elite_maximum_level = (*json)["enemy_elite_maximum_level"].GetInt();

  pablo_wont_steal          = (*json)["pablo_wont_steal"].GetBool();

  critical_damage_head_multiplier = (*json)["tactical_head_damage_multiplier"].GetDouble();
  critical_damage_legs_multiplier = (*json)["tactical_legs_damage_multiplier"].GetDouble();
  chance_to_hit_maximum     = (*json)["chance_to_hit_maximum"].GetInt();
  chance_to_hit_minimum     = (*json)["chance_to_hit_minimum"].GetInt();

  imp_attribute_max         = (*json)["imp"]["max_attribute_points"].GetInt();
  imp_attribute_min         = (*json)["imp"]["min_attribute_points"].GetInt();
  imp_attribute_bonus       = (*json)["imp"]["bonus_attribute_points"].GetInt();
  imp_attribute_zero_bonus  = (*json)["imp"]["zero_attribute_points_bonus"].GetInt();
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
      case 'j':         return tactical_hotkey_j;
      }
    }
    else if(modifier == HKMOD_CTRL)
    {
      switch(key)
      {
      case 'n':         return tactical_hotkey_ctrl_n;
      case 'q':         return tactical_hotkey_ctrl_q;
      }
    }
    else if(modifier == HKMOD_SHIFT)
    {
      switch(key)
      {
      case 'j':         return tactical_hotkey_shift_j;
      }
    }
    else if(modifier == HKMOD_ALT)
    {
      switch(key)
      {
      case 'r':         return tactical_hotkey_alt_r;
      }
    }
  }

  if(mode == UI_Map)
  {
    if(modifier == HKMOD_CTRL)
    {
      switch(key)
      {
      case 'i':         return map_hotkey_ctrl_i;
      }
    }
  }

  return false;
}
