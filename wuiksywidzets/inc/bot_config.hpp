#pragma once

#include <string>
#include <vector>

enum class target_option : char
{
  none,
  monsters,
  assist,
  keep_on_player,
  dead_sweepable
};

struct bounding_box
{
  std::vector<std::pair<int, int>> box_points;
  std::vector<std::pair<int, int>> antistuck_points;
};

enum class class_condition_type : char
{
  always,
  hpgt,
  hplt,
  managt,
  manalt
};

enum class spell_target : char
{
  monster,
  self,
  nick
};

enum class bot_rule_what
{
  target,
  attack,
  manor,
  spoil
};

struct bot_rule
{
  bot_rule_what brw;
  bool is_include;
  int mob_id;
};

struct bot_spell
{
  bool active;
  int spellid;
  int cooldown;
  class_condition_type condtype;
  int condval;
  spell_target target;
  std::string nick_optional;
};

struct bot_oop_buff
{
  bool active;
  int spellid;
  std::string playername;
  int buff_before;
  bool is_targetless;
  bool requires_invite;
};

struct bot_item
{
  bool active;
  int itemid;
  int cooldown;
  class_condition_type condtype;
  int condval;
};

struct bot_buff
{
  bool active;
  int spellid;
  std::string playername;
  int buff_before_second;
  bool is_targetless;
};

struct bot_beast
{
  int id;
  bool high_priority;
};

struct bot_manor
{
  std::string castle;
  std::string seed;
  int expected_count;
};

struct bot_craft
{
  int recipe_id;
  int count;
  bool recursive;
};
