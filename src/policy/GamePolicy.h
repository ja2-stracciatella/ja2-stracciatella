#pragma once

#include <stdint.h>

#include <string>
#include <vector>

class GamePolicy
{
public:
  bool f_draw_item_shadow;                      /**< Draw shadows from the inventory items. */

  int32_t starting_cash_easy;
  int32_t starting_cash_medium;
  int32_t starting_cash_hard;

  /* Battle */
  bool f_drop_everything;               /**< Enemy drop all equipment. */
  bool f_all_dropped_visible;           /**< All dropped equipment is visible right away. */

  int8_t enemy_weapon_minimal_status;   /**< Minimal status of the enemy weapon (0 - 100). */

  ////////////////////////////////////////////////////////////
  // IMP
  ////////////////////////////////////////////////////////////

  struct ExtraItem
  {
    uint16_t    item;
    uint16_t    count;

    ExtraItem(uint16_t _item)
      :item(_item), count(1)
    {
    }

    ExtraItem(uint16_t _item, uint16_t _count)
      :item(_item), count(_count)
    {
    }
  };

  /** Extra equipment received after IMP char generation. */
  std::vector<ExtraItem> imp_extra_equipment;

  ////////////////////////////////////////////////////////////
  //
  ////////////////////////////////////////////////////////////
};
