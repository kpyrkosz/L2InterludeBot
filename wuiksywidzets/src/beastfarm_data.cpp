#include <beastfarm_data.hpp>

const beastfarm_data&
beastfarm_data::get()
{
  static beastfarm_data potato;
  return potato;
}

beastfarm_monster
beastfarm_data::get_data(int id) const
{
  beastfarm_monster res{};
  res.is_valid = true;
  // first, find out if it's a valid ID
  if (id >= 21470 && id <= 21470 + 18)
  {
    res.name = "Buffalo";
    res.flags = id - 21470 - 1;
  }
  else if (id >= 21489 && id <= 21489 + 18)
  {
    res.name = "Cougar";
    res.flags = id - 21489 - 1;
  }
  else if (id >= 21451 && id <= 21451 + 18)
  {
    res.name = "Kooka";
    res.flags = id - 21451 - 1;
  }
  else
    res.is_valid = false;
  return res;
}
