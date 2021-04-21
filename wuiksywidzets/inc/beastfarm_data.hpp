#pragma once

#include <string>

struct beastfarm_monster
{
  bool is_valid; // in case non-beastfarm ID is passed, mby use optional?
  std::string name;
  int flags;
};

class beastfarm_data
{
public:
  static const beastfarm_data& get();
  beastfarm_monster get_data(int id) const;
};
