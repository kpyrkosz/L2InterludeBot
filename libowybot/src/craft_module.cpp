#include <action_executor.hpp>
#include <bot_confiuration.hpp>
#include <craft_module.hpp>
#include <game_data_facade.hpp>
#include <windows.h>

bool
do_i_have_enough_mats(FL2RecipeData* recipe_ptr)
{
  auto& items = game_data_facade::get_items();
  for (int i = 0; i < recipe_ptr->req.mat_count; ++i)
  {
    single_material_requirement* req = recipe_ptr->req.material_array[i];
    if (items.count(req->item_id) == 0 || items.at(req->item_id).count < req->count)
      return false;
  }
  return true;
}

// return success upon succesful clicking
bool
attempt_to_click(int recipe_id, bool recursive, int expected_count)
{
  // find the recipe in game data
  auto* recipe_ptr = GL2GameData.GetRecipeDataByIndex(recipe_id);
  if (!recipe_ptr)
    return false;

  auto& items = game_data_facade::get_items();
  // check if we already have enough of the resulting thing
  if (items.count(recipe_ptr->resulting_item_id) != 0 &&
      items.at(recipe_ptr->resulting_item_id).count >= expected_count)
    return false;

  // check if we can click it
  if (game_data_facade::get_my_user().mp >= recipe_ptr->manacost &&
      do_i_have_enough_mats(recipe_ptr))
  {
    action_executor::craft_click(recipe_id);
    return true;
  }

  // if the craft is nonrecursive, we are leaving right now
  if (!recursive)
    return false;

  // recursive craft, fasten your seatbelts boiiiis
  for (int i = 0; i < recipe_ptr->req.mat_count; ++i)
  {
    // check every required material
    single_material_requirement* req = recipe_ptr->req.material_array[i];

    // if we have it, just go to the next one
    if (items.count(req->item_id) != 0 && items.at(req->item_id).count >= req->count)
      continue;

    // we dont have it. check if it's craftable
    auto* recursive_recipe = GL2GameData.GetRecipeDataByProductID(req->item_id);

    // nope, it's a base material
    if (!recursive_recipe)
      continue;

    // yeah, it's craftable, recurse!
    bool click_res = attempt_to_click(recursive_recipe->recipe_id, true, req->count);

    // we clicked succesfully
    if (click_res)
      return true;

    // no luck, try next mat
    // continue;
  }
  return false;
}

bool
craft_module::perform_action()
{
  // TODO CHECK IF THE RCP IS REGISTERED IN BOOK!!!
  // crafting does not have penalty i think?
  // let's set 2 sec rescan when you are short of mats
  static auto penalty = GetTickCount();
  if (penalty > GetTickCount())
    return false;
  for (auto& craft : get_bot_settings().craft)
  {
    if (attempt_to_click(craft.recipe_id, craft.recursive, craft.count))
      return true;
  }
  penalty = GetTickCount() + 2222;
  return false;
}
