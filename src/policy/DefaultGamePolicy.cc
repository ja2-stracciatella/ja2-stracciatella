#include "DefaultGamePolicy.h"

#include "Build/Tactical/Item_Types.h"

DefaultGamePolicy::DefaultGamePolicy()
{
  f_draw_item_shadow = true;

  starting_cash_easy    = 45000;
  starting_cash_medium  = 35000;
  starting_cash_hard    = 30000;

  f_drop_everything     = false;
  f_all_dropped_visible = false;

  enemy_weapon_minimal_status   = 0;

  // imp_extra_equipment.push_back(ExtraItem(AKM));
  // imp_extra_equipment.push_back(ExtraItem(CLIP762W_30_AP, 4));
  // imp_extra_equipment.push_back(ExtraItem(CLIP762W_30_HP, 4));
}
