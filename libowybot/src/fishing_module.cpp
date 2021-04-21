#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <fishing_module.hpp>
#include <game_data_facade.hpp>

bool
fishing_module::perform_action()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.fishing))
    return false;

  const auto& bot_config = get_bot_settings();
  if (!bot_config.fishing_test)
    return false;

  puts("FISHING TICK\n");
  /*
  1312    Fishing
  1313   Pumping
  1314   Reeling
  */
  // add soulshot switch, for now temporarly hardcoded A grade TODO
  // 6539 Fishing Shot: A-grade [A]

  switch (game_data_facade::get_fishing_state().fs)
  {
    case fishing::fishing_state::inactive:
      action_executor::use_spell(1312);
      if (game_data_facade::get_items().count(6539))
        action_executor::use_item(game_data_facade::get_items().at(6539).objid);
      break;
    case fishing::fishing_state::bait_set:
      break;
    case fishing::fishing_state::fight:
      action_executor::use_spell(game_data_facade::get_fishing_state().should_reel ? 1314 : 1313);
      if (game_data_facade::get_items().count(6539))
        action_executor::use_item(game_data_facade::get_items().at(6539).objid);
      break;
  }

  update_penalty(penalties.fishing, 555);
  return false;
}
