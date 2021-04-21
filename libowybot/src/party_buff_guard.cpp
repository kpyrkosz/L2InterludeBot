#include <Windows.h>
#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <game_data_facade.hpp>
#include <party_buff_guard.hpp>
#include <player_character.hpp>

bool
party_buff_guard::perform_action()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.partybuffs))
    return false;
  if (game_data_facade::get_my_user().is_dead())
  {
    update_penalty(penalties.partybuffs, 777);
    return true;
  }

  if (penalties.current_intention != intention::idle &&
      penalties.current_intention != intention::buffing)
    return false;

  User& my_user = game_data_facade::get_my_user();
  for (const auto& nick_buff_pair : get_bot_settings().party_buffs)
  {
    const auto& target_nick = nick_buff_pair.first;
    const auto& buff_vector = nick_buff_pair.second;
    int current_timestamp = GetTickCount() / 1000;

    // check if the target is me
    if (target_nick == player_character::get().name())
    {
      // hmm copypasta below
      for (const auto& bot_buff : buff_vector)
      {
        // check if we have this spell
        if (game_data_facade::get_skills().count(bot_buff.buffid) == 0)
          continue;
        // check, if we have mana to cast it
        if (game_data_facade::get_skills().at(bot_buff.buffid)->manacost >
            game_data_facade::get_my_user().mp)
          continue;
        // there is no buff? maybe it's about to expire?
        if ((player_character::get().buffs().count(bot_buff.buffid) == 0 ||
             player_character::get().buffs().at(bot_buff.buffid) - bot_buff.buff_before <
               current_timestamp) &&
            game_data_facade::get_cooldowns()[bot_buff.buffid] < GetTickCount() - 1111) // penaltyqq
        {
          // CAST THE BUFF!
          if (!bot_buff.is_targetless && my_user.get_target_id() != game_data_facade::get_my_id())
          {
            action_executor::action(game_data_facade::get_my_id());
            penalties.current_intention = intention::buffing;
            return true;
          }
          update_penalty(penalties.target, 444);
          update_penalty(penalties.partybuffs, 333);
          action_executor::use_spell(bot_buff.buffid);
          printf("Casting %d SELF\n", bot_buff.buffid);
          penalties.current_intention = intention::buffing;
          return true;
        }
        // nope, try next one
      }
    }
    player_character::get().buffs();
    // check if the guy is in party
    auto* pt_member = party::get().member_by_nick(target_nick);
    if (pt_member == nullptr)
      continue;

    // check if he's near
    User* he = game_data_facade::get_by_id(pt_member->objid);
    if (!he)
      continue;

    // is he dead?
    if (he->is_dead())
      continue;
    auto& his_xyz = he->get_xyz();
    auto& my_xyx = my_user.get_xyz();
    float dist_sq = (his_xyz.x - my_xyx.x) * (his_xyz.x - my_xyx.x) +
                    (his_xyz.y - my_xyx.y) * (his_xyz.y - my_xyx.y) +
                    (his_xyz.z - my_xyx.z) * (his_xyz.z - my_xyx.z);

    // 500 units away?
    if (dist_sq > 500.0f * 500.0f)
      continue;

    // he is in party, check the buffs
    // loop through the buff vector
    for (const auto& bot_buff : buff_vector)
    {
      // check if we have this spell
      if (game_data_facade::get_skills().count(bot_buff.buffid) == 0)
        continue;
      // check, if we have mana to cast it
      if (game_data_facade::get_skills().at(bot_buff.buffid)->manacost >
          game_data_facade::get_my_user().mp)
        continue;
      // there is no buff? maybe it's about to expire?
      if ((pt_member->buffid_to_expiration_time.count(bot_buff.buffid) == 0 ||
           pt_member->buffid_to_expiration_time.at(bot_buff.buffid) - bot_buff.buff_before <
             current_timestamp) &&
          game_data_facade::get_cooldowns()[bot_buff.buffid] < GetTickCount() - 1111) // penaltyqq
      {
        // CAST THE BUFF!
        if (!bot_buff.is_targetless && my_user.get_target_id() != pt_member->objid)
        {
          update_penalty(penalties.target, 1111);
          action_executor::action(pt_member->objid);
          penalties.current_intention = intention::buffing;
          return true;
        }
        update_penalty(penalties.target, 444);
        update_penalty(penalties.partybuffs, 777);
        action_executor::use_spell(bot_buff.buffid);
        printf("Casting %d on %ws\n", bot_buff.buffid, pt_member->name.c_str());
        penalties.current_intention = intention::buffing;
        return true;
      }
      // nope, try next one
    }
  }
  penalties.current_intention = intention::idle;
  update_penalty(penalties.partybuffs, 1234);
  return false;
}
