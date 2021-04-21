#include <player_character.hpp>

std::unordered_map<int, int>&
player_character::buffs()
{
  return buffid_to_expiration_time;
}

std::wstring&
player_character::name()
{
  return name_;
}
