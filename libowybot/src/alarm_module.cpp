#include <action_executor.hpp>
#include <alarm_module.hpp>
#include <algorithm>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>

bool
alarm_module::perform_action()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.alarm_scan))
    return false;

  const auto& bot_config = get_bot_settings();
  if (bot_config.alarm_cfg.on_enter_bounding_box)
  {
    auto players = game_data_facade::get_nearby_players(true);
    for (const auto& p : players)
    {
      if (!p->IsMyPartyMember())
      {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        printf("ALARM! PLAYER %ws is in the box at h/m/s %d:%d:%d\n",
               p->GetName(),
               lt.wHour,
               lt.wMinute,
               lt.wSecond);
        PlaySound("c:/windows/media/notify.wav", 0, SND_ALIAS | SND_ASYNC);
        update_penalty(penalties.alarm_scan, 3333);
        return true;
      }
    }
  }
  if (bot_config.alarm_cfg.check_for_spawn)
  {
    auto mobs = game_data_facade::get_users([](const User& u) { return u.is_monster(); });
    for (const auto& m : mobs)
    {
      std::wstring name = (wchar_t*)m->GetName();
      if (name.find(bot_config.alarm_cfg.spawn_name) != std::wstring::npos)
      {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        printf("ALARM! MONSTER %ws ALIVE at h/m/s %d:%d:%d\n",
               name.c_str(),
               lt.wHour,
               lt.wMinute,
               lt.wSecond);
        PlaySound("c:/windows/media/tada.wav", 0, SND_ALIAS | SND_ASYNC);
        update_penalty(penalties.alarm_scan, 3333);
        return true;
      }
    }
  }
  update_penalty(penalties.alarm_scan, 3333);
  return false;
}
