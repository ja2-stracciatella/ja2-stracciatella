#include "DefaultGamePolicy.h"

#include "Build/Tactical/Item_Types.h"

DefaultGamePolicy::DefaultGamePolicy(rapidjson::Document *json)
{
  map_hotkey_ctrl_i     = (*json)["map_hotkey_ctrl_i"].GetBool();
  tactical_hotkey_j     = (*json)["tactical_hotkey_j"].GetBool();
  tactical_hotkey_shift_n       = (*json)["tactical_hotkey_shift_n"].GetBool();
  tactical_hotkey_ctrl_n        = (*json)["tactical_hotkey_ctrl_n"].GetBool();

  f_draw_item_shadow    = (*json)["draw_item_shadow"].GetBool();

  ms_per_game_cycle     = (*json)["ms_per_game_cycle"].GetInt();

  starting_cash_easy    = (*json)["starting_cash_easy"].GetInt();
  starting_cash_medium  = (*json)["starting_cash_medium"].GetInt();
  starting_cash_hard    = (*json)["starting_cash_hard"].GetInt();

  f_drop_everything     = (*json)["drop_everything"].GetBool();
  f_all_dropped_visible = (*json)["all_drops_visible"].GetBool();

  enemy_weapon_minimal_status   = (*json)["enemy_weapon_minimal_status"].GetInt();
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
      }
    }
    else if(modifier == HKMOD_SHIFT)
    {
      switch(key)
      {
      case 'n':         return tactical_hotkey_shift_n;
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
