#pragma once

#include <deque>
#include <functional>
#include <native_lineage_classes.hpp>
#include <unordered_map>
#include <vector>

struct dropped_item
{
  int objid;
  int itemid; // not yet
  float x, y, z;
  float distance;
};

struct monster
{
  int objid;
  float x, y, z;
  float distance;
};

struct item_data
{
  int objid;
  int count;
};

// this records timestamp + xyz of monster that died out of one of my party mates
// used primarly to filter out drops to pick that belong to me/my party
// make sure you clean it from time to time
struct my_drops
{
  unsigned timestamp;
  int objid;
  FVector loc;
};

struct fishing
{
  enum class fishing_state
  {
    inactive,
    bait_set,
    fight,
  } fs;
  bool should_reel;
};

namespace game_data_facade
{
int get_my_id();
bool is_in_bounding_box(const FVector& loc); // MEIN GOTT, USUN TO STAD
fishing& get_fishing_state();
User& get_my_user();
User* get_my_target();
User* get_by_id(int objid);
bool& is_in_world(); // todo leaveworld
[[deprecated]] std::vector<dropped_item> get_dropped_items(bool only_mine, bool use_blacklist);
[[deprecated]] bool temp_does_item_exist(int objid);
[[deprecated]] std::vector<User*> get_nearby_players(bool use_boundingbox = false);
std::vector<User*> get_users(std::function<bool(const User& user)> acceptance_predicate);
std::vector<Item*> get_items(std::function<bool(const Item& item)> acceptance_predicate);
std::unordered_map<int, int>& get_cooldowns();             // key is spellid
std::unordered_map<int, item_data>& get_items();           // key is templateid, not uid
std::unordered_map<int, FL2MagicSkillData*>& get_skills(); // key is spellid
std::unordered_map<int, my_drops>& get_my_drops();
[[deprecated]] bool is_monster_on_me();
} // namespace game_data_facade

namespace helpers_filters
{
std::vector<User*> get_monsters_attacking_me_or_party(bool include_party_members = true);
std::vector<Item*> get_manaherbs();
std::vector<Item*> get_items_near_point(const FVector& xyz, float radius);
User* closest_among_subset(const std::vector<User*>& users, const FVector& midpoint);
} // namespace helpers_filters
