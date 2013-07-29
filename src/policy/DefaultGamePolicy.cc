#include "DefaultGamePolicy.h"


DefaultGamePolicy::DefaultGamePolicy()
{
  f_draw_item_shadow = true;

  starting_cash_easy    = 45000;
  starting_cash_medium  = 35000;
  starting_cash_hard    = 30000;

  f_drop_everything     = false;

  enemy_weapon_minimal_status   = 0;
}
