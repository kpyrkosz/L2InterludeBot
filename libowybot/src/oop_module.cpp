#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>
#include <oop_module.hpp>
#include <party.hpp>

bool
oop_module::perform_action()
{
  // this module is responsible for checking own buffs and requesting rebuff
  // also checks the queue maybe someone wants me to buff him
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.oop))
    return false;

  if (game_data_facade::get_my_user().is_dead())
  {
    update_penalty(penalties.oop, 1111);
    return true;
  }

  const auto& bot_config = get_bot_settings();

  if (!bot_config.oops.empty())
  {
    // stuff i want from others
  }

  return false;
}
