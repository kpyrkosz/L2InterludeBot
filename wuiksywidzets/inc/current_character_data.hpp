#pragma once

#include <string>
#include <unordered_map>

struct current_character_data
{
  std::unordered_map<int, std::string> spells;
  std::unordered_map<int, std::string> items;
};
