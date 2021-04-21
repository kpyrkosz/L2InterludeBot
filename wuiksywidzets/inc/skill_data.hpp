#pragma once

#include <string>
#include <unordered_map>

// singleton exposing const only interface kinda works
class skill_data
{
  std::unordered_map<int, std::string> id_to_name_;
  skill_data();

public:
  static const skill_data& get();
  const std::string& id_to_name(int id) const;
};
