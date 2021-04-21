#pragma once

#include <string>
#include <unordered_map>
#include <game_data_facade.hpp>

class player_character
{
  std::wstring name_;
  std::unordered_map<int, int> buffid_to_expiration_time;

public:
  static player_character& get()
  {
    static player_character potato;
    return potato;
  }

  std::unordered_map<int, int>& buffs();
  std::wstring& name();
};
