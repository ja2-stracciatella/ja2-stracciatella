#pragma once

#include <stdint.h>

#include <string>
#include <vector>

enum UIMode
{
  UI_Tactical,
  UI_Map
};

enum HotkeyModifier
{
  HKMOD_None,
  HKMOD_CTRL,
  HKMOD_SHIFT
};

#define gamepolicy(element) (GCM->getGamePolicy()->element)

class GamePolicy
{
public:
  /** Check if a hotkey is enabled. */
  virtual bool isHotkeyEnabled(UIMode mode, HotkeyModifier modifier, uint32_t key) const = 0;

  bool f_draw_item_shadow;              /**< Draw shadows from the inventory items. */

  int32_t ms_per_game_cycle;            /**< Milliseconds per game cycle. */

  int32_t starting_cash_easy;
  int32_t starting_cash_medium;
  int32_t starting_cash_hard;

  /* Battle */
  bool f_drop_everything;               /**< Enemy drop all equipment. */
  bool f_all_dropped_visible;           /**< All dropped equipment is visible right away. */

  int8_t enemy_weapon_minimal_status;   /**< Minimal status of the enemy weapon (0 - 100). */

  bool gui_extras;                      /* graphical user interface cosmetic mod */
 
  ////////////////////////////////////////////////////////////
  //
  ////////////////////////////////////////////////////////////
};
