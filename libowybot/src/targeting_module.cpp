#include <action_executor.hpp>
#include <algorithm>
#include <beastfarm_data.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>
#include <party.hpp>
#include <targeting_module.hpp>
#include <windows.h>

// lambda for the monster iterator to filter out potential targets
bool
should_target_this_monster(const User& the_guy)
{
  // check if it's a monster or a guard in delevel mode
  if ((!the_guy.is_monster() && !get_bot_settings().target_guards_for_delevel_test) ||
      (!the_guy.is_guard() && get_bot_settings().target_guards_for_delevel_test))
    return false;

  // dead?
  if (the_guy.is_dead())
    return false;

  // is he in acceptable Z range
  if (auto& my_xyz = game_data_facade::get_my_user().get_xyz();
      the_guy.get_xyz().z < my_xyz.z - 500.0f || the_guy.get_xyz().z > my_xyz.z + 500.0f)
    return false;

  // in bounding box
  if (!game_data_facade::is_in_bounding_box(the_guy.get_xyz()))
    return false;

  // ignore trained beastfarm animals
  if (the_guy.template_id() >= 16013 && the_guy.template_id() <= 16018)
    return false;

  // prioritize monsters that have aggroed on me or party member, even when the monster is banned
  if (int his_target = the_guy.get_target_id();
      his_target == game_data_facade::get_my_id() || party::get().members().count(his_target))
    return true;

  // in beastfarm mode prefer the feedable beasts
  bool was_lookin_for_a_beast = false;
  if (get_bot_settings().beastfarm_cfg.prefer_buffalo)
    if (beastfarm_data::get().is_buffalo(the_guy.template_id()))
      return true;
    else
      was_lookin_for_a_beast = true;

  if (get_bot_settings().beastfarm_cfg.prefer_kooka)
    if (beastfarm_data::get().is_kooka(the_guy.template_id()))
      return true;
    else
      was_lookin_for_a_beast = true;

  if (get_bot_settings().beastfarm_cfg.prefer_cougar)
    if (beastfarm_data::get().is_cougar(the_guy.template_id()))
      return true;
    else
      was_lookin_for_a_beast = true;

  // means we specified to prefer some beast but this target does not qualify
  if (was_lookin_for_a_beast)
    return false;

  // is the monster allowed by the blacklist system
  if (!get_bot_settings().target_blacklist.is_allowed_target(the_guy.template_id()))
    return false;

  // that's it, i guess ;)
  return true;
}

void
target_monster()
{
  auto& penalties = get_penalties();
  // kolosalny blad! spell casting i buff zajmuja to dla siebie kulturalnie a ten
  // modul im w tym przeszkadza!! TODO!!! cos z tym bo to ostatni puzel!!!
  // tu powinien byc jakis czek, ze ktorys modul zazyczyl sobie wylacznego prawa do trgetu
  if (is_under_penalty(penalties.target) || is_under_penalty(penalties.targetscan))
    return;
  const auto& bot_config = get_bot_settings();
  User* my_target = game_data_facade::get_my_target();
  if (my_target && my_target->is_dead() && bot_config.sweep &&
      my_target->get_apawn()->get_acontroller()->is_custom_sweepable())
  {
    update_penalty(penalties.target, 777);
    // sweepable, wait for sweep in autoattack module, mby restructurize?
    return;
  }

  if (my_target && my_target->is_dead() && !bot_config.manor_seeds.empty() &&
      my_target->get_apawn()->get_acontroller()->is_custom_manorable())
  {
    update_penalty(penalties.target, 777);
    // manorablesowableblablable, wait for sweep in autoattack module, mby restructurize?
    return;
  }
  // retarget if fight didnt start and me or party member is under attack
  if (bot_config.prioritize)
  {
    // gather potential attackers
    auto attackers = helpers_filters::get_monsters_attacking_me_or_party();
    // if there are none, continue with standard procedure
    if (!attackers.empty())
    {
      // if my current target is one of them, then i'm hitting the right guy
      if (std::find_if(begin(attackers), end(attackers), [&my_target](const User* the_guy) {
            return the_guy->objid() == game_data_facade::get_my_user().get_target_id();
          }) != attackers.end())
      {
        update_penalty(penalties.targetscan, 888);
        return;
      }
      // well then, time to target one of these cunts, stop slapping me and my mates!!
      auto the_fucker =
        helpers_filters::closest_among_subset(attackers, game_data_facade::get_my_user().get_xyz());
      action_executor::action(the_fucker->objid());
      update_penalty(penalties.target, 444);
      update_penalty(penalties.targetscan, 888);
      return;
    }
  }

  // retarget if monster spawns closer than my current target
  bool should_retarget_closer = false;
  if (bot_config.retarget && game_data_facade::get_my_user().get_target_id() != 0)
  {
    // warning - i call it twice and check distance twice, todo fix later
    auto mobs = game_data_facade::get_users(should_target_this_monster);
    if (!mobs.empty())
    {
      auto closest =
        helpers_filters::closest_among_subset(mobs, game_data_facade::get_my_user().get_xyz());
      if (closest->objid() != game_data_facade::get_my_user().get_target_id())
        should_retarget_closer = true;
    }
  }

  // the old targeting code
  // check if he's attackable
  if (!my_target || should_retarget_closer ||
      game_data_facade::get_my_user().get_target_id() == 0 || my_target->is_dead() ||
      (!my_target->is_monster() && !bot_config.target_guards_for_delevel_test) ||
      (!my_target->is_guard() && bot_config.target_guards_for_delevel_test))
  {
    puts("searching for monster");
    const auto& my_xyz = game_data_facade::get_my_user().get_xyz();
    auto mobs = game_data_facade::get_users(should_target_this_monster);

    auto iter =
      std::min_element(begin(mobs), end(mobs), [&my_xyz](const User* lhs, const User* rhs) {
        return (my_xyz - lhs->get_xyz()).SizeSquared() < (my_xyz - rhs->get_xyz()).SizeSquared();
      });
    if (iter != end(mobs))
      action_executor::action((*iter)->objid());
    update_penalty(penalties.target, 444);
    update_penalty(penalties.targetscan, 888);
    return;
  }
  update_penalty(penalties.target, 444);
  update_penalty(penalties.targetscan, 888);
}

void
assist_player()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.target))
    return;

  const auto& bot_config = get_bot_settings();
  User* my_target = game_data_facade::get_my_target();

  // special case - if the target is sweepable do not assist yet, sweep first
  if (my_target && my_target->is_dead() && bot_config.sweep &&
      my_target->get_apawn()->get_acontroller()->is_custom_sweepable())
  {
    update_penalty(penalties.target, 777);
    // sweepable, wait for sweep in autoattack module, mby restructurize?
    return;
  }

  // find the player
  UNetworkHandler& ziemniak = UNetworkHandler::get();
  User* the_guy =
    ziemniak.UNetworkHandler::GetUser((unsigned short*)bot_config.target_name.c_str());
  if (!the_guy)
  {
    update_penalty(penalties.target, 777);
    return;
  }
  auto his_target = the_guy->get_target_id();

  if (his_target != 0 && game_data_facade::get_my_user().get_target_id() != his_target)
  {
    // check if he's attackable
    FObjectMap tempf{1, game_data_facade::get_by_id(his_target)};
    if (!ziemniak.UNetworkHandler::IsGNOMatch(2, &tempf))
    {
      // target is not attackable
      update_penalty(penalties.target, 777);
      return;
    }
    puts("assisting!");
    update_penalty(penalties.target, 777);
    action_executor::action(his_target);
    return;
  }
  update_penalty(penalties.target, 333);
}

void
keep_target()
{
  // todo kurwa, jak to ID wyciagnac z usera???
}

void
dead_sweepable()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.target))
    return;
  const auto& bot_config = get_bot_settings();
  User* my_target = game_data_facade::get_my_target();

  if (!my_target || !my_target->is_dead() ||
      !my_target->get_apawn()->get_acontroller()->is_custom_sweepable())
  {
    puts("searching for deadsweepable monster");
    const auto& my_xyz = game_data_facade::get_my_user().get_xyz();
    auto mobs = game_data_facade::get_users([](const User& the_guy) { // check if it's a monster
      auto& my_xyz = game_data_facade::get_my_user().get_xyz();
      return the_guy.is_monster() && the_guy.is_dead() &&
             the_guy.get_apawn()->get_acontroller()->is_custom_sweepable() &&
             the_guy.get_xyz().z > my_xyz.z - 500.0f && the_guy.get_xyz().z < my_xyz.z + 500.0f &&
             game_data_facade::is_in_bounding_box(the_guy.get_xyz());
    });

    auto iter =
      std::min_element(begin(mobs), end(mobs), [&my_xyz](const User* lhs, const User* rhs) {
        return (my_xyz - lhs->get_xyz()).SizeSquared() < (my_xyz - rhs->get_xyz()).SizeSquared();
      });
    if (iter != end(mobs))
      action_executor::action((*iter)->objid());
    update_penalty(penalties.target, 444);
    return;
  }
  update_penalty(penalties.target, 444);
}

bool
targeting_module::perform_action()
{
  const auto& bot_config = get_bot_settings();
  auto& penalties = get_penalties();
  if (game_data_facade::get_my_user().is_dead())
  {
    update_penalty(penalties.target, 777);
    return true;
  }

  switch (bot_config.target)
  {
    case target_option::none:
      return true;
    case target_option::monsters:
      target_monster();
      return true;
    case target_option::assist:
      assist_player();
      return true;
    case target_option::keep_on_player:
      keep_target();
      return true;
    case target_option::dead_sweepable:
      dead_sweepable();
      return true;
  }
  return false;
}
