#include "DefaultGamePolicy.h"

#include "Build/Tactical/Item_Types.h"

DefaultGamePolicy::DefaultGamePolicy(rapidjson::Document *json)
{
  f_draw_item_shadow    = (*json)["draw_item_shadow"].GetBool();

  starting_cash_easy    = (*json)["starting_cash_easy"].GetInt();
  starting_cash_medium  = (*json)["starting_cash_medium"].GetInt();
  starting_cash_hard    = (*json)["starting_cash_hard"].GetInt();

  f_drop_everything     = (*json)["drop_everything"].GetBool();
  f_all_dropped_visible = (*json)["all_drops_visible"].GetBool();

  enemy_weapon_minimal_status   = (*json)["enemy_weapon_minimal_status"].GetInt();
}
