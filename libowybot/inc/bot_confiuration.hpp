#pragma once

#include <blacklist.hpp>
#include <party.hpp>
#include <string>
#include <unordered_map>
#include <vector>

enum class target_option : char
{
  none,
  monsters,
  assist,
  keep_on_player,
  dead_sweepable
};

enum class spell_target : char
{
  monster,
  self,
  nick
};

enum class class_condition_type : char
{
  always,
  hpgt,
  hplt,
  managt,
  manalt
};

struct buff
{
  int buffid;
  int buff_before;
  bool is_targetless;
};

struct bot_item
{
  int template_id;
  int cooldown_ms;
  class_condition_type cond_type;
  int cond_val;
  int last_use_timestamp;
};

struct bot_spell
{
  int spellid;
  int cooldown;
  class_condition_type condtype;
  int condval;
  spell_target target;
  std::string nick_optional;
  int last_use_timestamp;
};

struct bot_manor
{
  int seed_itemid;
  int crop_itemid;
  int expected_count;
};

struct bot_craft
{
  int recipe_id;
  int count;
  bool recursive;
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

struct dos_and_dont
{
  std::vector<int> include;
  std::vector<int> ignore;
};

struct alarm_config
{
  bool check_for_spawn;
  std::wstring spawn_name; // w bo gra ma w qq
  bool on_enter_bounding_box;
};

struct follow_config
{
  std::wstring player_name;
  bool follow_attack_like;
  bool keep_distance;
  int dist_min;
  int dist_max;
};

struct pick_configs
{
  bool picking_on, non_greedy, out_of_fight, limit_to_box, manaherb_priority;
};

struct bot_oop_buff
{
  int spellid;
  std::wstring playername;
  int buff_before;
  bool is_targetless;
  bool requires_invite;
};

struct tricks_configs
{
  bool remove_trained_beastfarm_animals;
  bool turn_cancel_animation_visible;
  bool dont_show_droped_arrows;
};

struct beastfarm_configs
{
  std::vector<int> high_priority;
  std::vector<int> low_priority;
  bool prefer_buffalo;
  bool prefer_kooka;
  bool prefer_cougar;
};

struct settings
{
  bool is_active;
  target_option target;
  std::wstring target_name;
  std::string player_to_follow;
  bool prioritize, assist_wait, retarget, autoattack, spoil, follow_player, movecloser, acceptparty,
    sweep;
  [[deprecated]] bool DEPRpick_close, DEPRprioritize_mana_herbs, DEPR, DEPRfakemov, DEPRpick,
    DEPRpick_far_away, DEPRpick_only_mine, DEPRpick_in_box, DEPRexperimental_recharge;
  std::string party_inviter;
  std::unordered_map<std::wstring, std::vector<buff>> party_buffs;
  std::vector<bot_item> bot_items;
  std::vector<bot_spell> bot_spells;
  std::vector<std::pair<int, int>> bounding_box;
  std::vector<std::pair<int, int>> antistuck_points;
  std::vector<bot_manor> manor_seeds;
  std::vector<bot_craft> craft;
  std::vector<bot_oop_buff> oops;
  blacklist target_blacklist;
  blacklist attack_blacklist;
  blacklist manor_blacklist;
  blacklist spoil_blacklist;
  bool manor_test;     // to bylo do wymiany cropow
  bool manor_buy_test; // to do kupowania
  alarm_config alarm_cfg;
  follow_config follow_cfg;
  pick_configs pick_cfg;
  tricks_configs tricks_cfg;
  beastfarm_configs beastfarm_cfg;
  bool target_guards_for_delevel_test;
  bool fishing_test;
  bool whisper_test;
};

settings& get_bot_settings();
