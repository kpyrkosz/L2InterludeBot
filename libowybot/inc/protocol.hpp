#pragma once

enum class to_bot_protocol : char
{
  log_string,
  pause_bot,
  target,
  farming,
  bounding_box,
  spells,
  items,
  buffs,
  request_your_position,
  manor,
  craft,
  dos,
  alarm,
  follow,
  pick,
  relax,
  out_of_party,
  tricks,
  broadcast,
  beastfarm
};

enum class to_controller_protocol : char
{
  log_string,
  game_entered,
  game_left,
  spell_list,
  item_list,
  char_position,
  broadcast
};

enum class broadcast_protocol : char
{
  log_string,
  request_buff
};
