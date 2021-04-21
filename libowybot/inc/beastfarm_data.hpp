#pragma once

#include <unordered_map>

enum class beastfarm_feed
{
  crystal,
  golden,
  any,
  dont_feed
};

class beastfarm_data
{
  beastfarm_data();

  struct intermediate_monster
  {
    bool is_bigger;
    bool is_crystal;
    bool is_mage;
  };

  struct final_monster
  {
    bool is_mage;
  };

  struct monster_category
  {
    int base_monster_id;
    std::unordered_map<int, intermediate_monster> intermediate_forms;
    std::unordered_map<int, final_monster> final_forms;
  };

  monster_category buffalos;
  monster_category kookaburras;
  monster_category cougars;

  void initialize_category(monster_category& mc, int base_id);
  beastfarm_feed should_feed_among_category(const monster_category& mc,
                                            int current_monster_template_id,
                                            int wanted_id) const;

public:
  static const beastfarm_data& get()
  {
    static beastfarm_data potato;
    return potato;
  }
  beastfarm_feed should_feed(int current_monster_template_id, int wanted_id) const;
  bool is_feedable_template_id(int id) const;
  bool is_buffalo(int id) const;
  bool is_kooka(int id) const;
  bool is_cougar(int id) const;
};
