#pragma once

#include <stdint.h>

class GamePolicy
{
public:
  bool f_draw_item_shadow;                      /**< Draw shadows from the inventory items. */

  int32_t starting_cash_easy;
  int32_t starting_cash_medium;
  int32_t starting_cash_hard;

  /* Battle */
  bool f_drop_everything;               /**< Enemy drop all equipment. */
};
