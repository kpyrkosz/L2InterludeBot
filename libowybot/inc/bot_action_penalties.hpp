#pragma once

enum class intention
{
  idle,
  combat,
  picking,
  recharging,
  want_to_rest,
  resting,
  buffing,
  buffing_oop,
  casting,
};

// the values are integers meaning closest timestamp when the action can be executed
// for example, requesting assist will send the target packet and set the target penalty
// so that say party module will not try to change target and wait
// chaning it too quickly results in target jumping between two monsters, we want to avoid that
struct bot_action_penalties
{
  int target;
  int targetscan; // used exclusively by target scanning cause target up is for buffs/spells
  int items;
  int spells;
  int partybuffs;
  int movement;
  int autoattack;
  int seeding;
  int alarm_scan;
  int antistuck;
  int picking;
  int oop;
  int fishing;
  intention current_intention;
  int magic_cast_count; // count of magic cast requests since last succesful cast
  int pick_requests_count;
};

bot_action_penalties& get_penalties();
bool is_under_penalty(int penalty);
void update_penalty(int& penalty, int millisec_to_add);
