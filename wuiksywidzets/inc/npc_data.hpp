#pragma once

#include <string>
#include <unordered_map>

// singleton exposing const only interface kinda works
class npc_data
{
  std::unordered_map<int, std::string> id_to_name_;
  npc_data();

public:
  static const npc_data& get();
  const std::string& id_to_name(int id) const;
};
