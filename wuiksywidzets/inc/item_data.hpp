#pragma once

#include <string>
#include <unordered_map>

// singleton exposing const only interface kinda works
class item_data
{
  std::unordered_map<int, std::string> id_to_name_;
  item_data();

public:
  static const item_data& get();
  const std::string& id_to_name(int id) const;
};
