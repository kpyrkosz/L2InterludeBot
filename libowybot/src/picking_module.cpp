#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>
#include <party.hpp>
#include <picking_module.hpp>

bool
picking_module::perform_action()
{
  // let's mash up both picking and recharging in there for now
  // week later: YEAH, FUCK, I KNEW I WOULD REGRET IT
  auto& penalties = get_penalties();

  if (is_under_penalty(penalties.picking))
    return false;

  if (game_data_facade::get_my_user().is_dead())
  {
    if (penalties.current_intention == intention::picking)
      penalties.current_intention = intention::idle;
    update_penalty(penalties.picking, 777);
    return true;
  }

  const auto& bot_config = get_bot_settings();
  if (!bot_config.pick_cfg.picking_on)
  {
    penalties.current_intention = intention::idle;
    update_penalty(penalties.picking, 777);
    return true;
  }

  // first check the manaherbs, even at the cost of interrupting other actions, it's a priority
  if (bot_config.pick_cfg.manaherb_priority && game_data_facade::get_my_user().mp_percentage() < 80)
  {
    // is there any nearby?
    auto herbs = helpers_filters::get_manaherbs();
    if (!herbs.empty())
    {
      // hmm the closest search should be a function cause it repeats in many places
      auto closest_item_it = herbs.begin();
      const FVector& my_xyz = game_data_facade::get_my_user().get_xyz();
      float smallest_size_squared = (my_xyz - (*closest_item_it)->get_xyz()).SizeSquared();
      for (auto it = std::next(closest_item_it); it != herbs.end(); ++it)
      {
        float size_sq = (my_xyz - (*it)->get_xyz()).SizeSquared();
        if (size_sq < smallest_size_squared)
        {
          smallest_size_squared = size_sq;
          closest_item_it = it;
        }
      }
      if (smallest_size_squared < 1300.f * 1300.f)
      {
        puts("KUHWAAAAAAAAAAA MANAHERB");
        // also this should be guarded by action executor because it is too easy to forget
        ++penalties.pick_requests_count;
        penalties.current_intention = intention::picking;
        update_penalty(penalties.picking, 333);
        action_executor::action((*closest_item_it)->objid);
        return true;
      }
      else
        puts("MANAHERB ALE DALEEEEEEKO :(");
    }
  }

  if (penalties.current_intention != intention::idle &&
      penalties.current_intention != intention::picking)
    return false;

  /*
  bool limit_to_box;
*/

  // scan for nearby items, honoring the configs
  // so first check if we are attacked by someone if the out of fight flag is set
  if (bot_config.pick_cfg.out_of_fight)
  {
    // if we are under attack set state to idle so that combat can catch it
    auto attackers = helpers_filters::get_monsters_attacking_me_or_party(false);
    // we are under attack! abort!
    if (!attackers.empty())
    {
      penalties.current_intention = intention::idle;
      update_penalty(penalties.picking, 555);
      return true;
    }
    // fall through
  }

  auto drops = game_data_facade::get_items([&bot_config](const Item& item) {
    return !bot_config.pick_cfg.limit_to_box ||
           game_data_facade::is_in_bounding_box(item.get_xyz());
  });

  if (drops.empty())
  {
    penalties.current_intention = intention::idle;
    penalties.pick_requests_count = 0;
    return true;
  }

  puts("no probuje piknac super trybem ; O");

  penalties.current_intention = intention::picking;
  // hmm the closest search should be a function cause it repeats in many places
  auto closest_item_it = drops.begin();
  const FVector& my_xyz = game_data_facade::get_my_user().get_xyz();
  float smallest_size_squared = (my_xyz - (*closest_item_it)->get_xyz()).SizeSquared();
  for (auto it = std::next(closest_item_it); it != drops.end(); ++it)
  {
    float size_sq = (my_xyz - (*it)->get_xyz()).SizeSquared();
    if (size_sq < smallest_size_squared)
    {
      smallest_size_squared = size_sq;
      closest_item_it = it;
    }
  }
  // also this should be guarded by action executor because it is too easy to forget
  ++penalties.pick_requests_count;
  if (smallest_size_squared > 10000.0f)
  {
    puts("MOVING CLOSE TO PICK!!");
    action_executor::move_to_xyz((*closest_item_it)->get_xyz());
  }
  else
  {
    puts("PICKING!!");
    ++penalties.pick_requests_count;
    action_executor::action((*closest_item_it)->objid);
  }
  update_penalty(penalties.picking, 444);
  return true;
}
