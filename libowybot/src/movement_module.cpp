#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>
#include <movement_module.hpp>
#include <windows.h>

bool
movement_module::perform_action()
{
  // following
  const auto& bot_config = get_bot_settings();
  if (!bot_config.follow_player)
    return false;

  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.movement))
    return false;

  if (penalties.current_intention != intention::idle)
    return false;

  // find the player
  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  // ja jebie xD
  std::wstring wstrname(bot_config.player_to_follow.begin(), bot_config.player_to_follow.end());
  User* the_guy = ziemniak->UNetworkHandler::GetUser((unsigned short*)wstrname.c_str());
  if (!the_guy)
  {
    update_penalty(penalties.movement, 2222);
    return true;
  }

  // hmm
  // TODO1 - occasionally move around when you're close and the target is also close for long period
  // TODO2 - dont move directly on him but somewhere between you and him
  // TODO3 - change behavior when you are close, not so close and "where the fuck are you rushing?!"
  auto& his_xyz = the_guy->get_xyz();
  auto& my_xyx = game_data_facade::get_my_user().get_xyz();
  float dist_sq = (his_xyz.x - my_xyx.x) * (his_xyz.x - my_xyx.x) +
                  (his_xyz.y - my_xyx.y) * (his_xyz.y - my_xyx.y) +
                  (his_xyz.z - my_xyx.z) * (his_xyz.z - my_xyx.z);

  // we are close
  if (dist_sq < 300.0f * 300.0f)
  {
    update_penalty(penalties.movement, 2222);
    return true;
  }
  // we are not close, so first check if we are far far away, then go directly to his xyz
  if (dist_sq > 1500.0f * 1500.0f)
  {
    puts("Following player FAR FAR AWAY");
    update_penalty(penalties.movement, 3333);
    action_executor::move_to_xyz(his_xyz);
    return true;
  }
  // we are somewhere between, move to random place near the direct line between you

  puts("Following player CLOSE");
  update_penalty(penalties.movement, 1470);
  FVector dest(my_xyx);
  dest -= his_xyz;
  dest.Normalize();
  dest *= 200.0f;
  dest += his_xyz;
  action_executor::move_to_xyz(dest);
  return true;
}
