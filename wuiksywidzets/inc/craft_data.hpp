#pragma once

#include <string>
#include <unordered_map>

// singleton exposing const only interface kinda works
class craft_data
{
  std::unordered_map<std::string, int> recipe_to_id_;
  std::unordered_map<int, std::string> id_to_recipe_;

  craft_data();

public:
  static const craft_data& get();
  const std::unordered_map<std::string, int>& get_data() const;
  const std::string& name_from_id(int id) const;
};
