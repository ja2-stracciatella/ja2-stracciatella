#pragma once

#include "GamePolicy.h"

#include "rapidjson/document.h"

class DefaultGamePolicy : public GamePolicy
{
public:
  DefaultGamePolicy(rapidjson::Document *json);

  /** Check if a hotkey is enabled. */
  virtual bool isHotkeyEnabled(UIMode mode, HotkeyModifier modifier, uint32_t key) const;

private:
  bool map_hotkey_ctrl_i;
  bool tactical_hotkey_j;
  bool tactical_hotkey_shift_j;
  bool tactical_hotkey_ctrl_n;
  bool tactical_hotkey_ctrl_q;
  bool tactical_hotkey_alt_r;
};
