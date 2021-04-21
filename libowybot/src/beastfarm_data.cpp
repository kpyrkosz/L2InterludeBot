#include <beastfarm_data.hpp>
#include <cassert>

beastfarm_data::beastfarm_data()
{
  initialize_category(buffalos, 21470);
  initialize_category(cougars, 21489);
  initialize_category(kookaburras, 21451);
}

void
beastfarm_data::initialize_category(monster_category& mc, int base_id)
{
  mc.base_monster_id = base_id;

  mc.final_forms[base_id + 18] = final_monster{true};
  mc.final_forms[base_id + 17] = final_monster{false};

  bool is_bigger;
  bool is_crystal;
  bool is_mage;

  // smaller
  is_bigger = false;
  // smaller crystal
  is_crystal = true;
  // smaller crystal mage
  is_mage = true;
  mc.intermediate_forms[base_id + 6] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 8] = intermediate_monster{is_bigger, is_crystal, is_mage};
  // smaller crystal fighter
  is_mage = false;
  mc.intermediate_forms[base_id + 5] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 7] = intermediate_monster{is_bigger, is_crystal, is_mage};
  // smaller golden
  is_crystal = false;
  // smaller golden mage
  is_mage = true;
  mc.intermediate_forms[base_id + 2] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 4] = intermediate_monster{is_bigger, is_crystal, is_mage};
  // smaller golden fighter
  is_mage = false;
  mc.intermediate_forms[base_id + 1] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 3] = intermediate_monster{is_bigger, is_crystal, is_mage};

  // bigger
  is_bigger = true;
  // bigger crystal
  is_crystal = true;
  // bigger crystal mage
  is_mage = true;
  mc.intermediate_forms[base_id + 14] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 16] = intermediate_monster{is_bigger, is_crystal, is_mage};
  // bigger crystal fighter
  is_mage = false;
  mc.intermediate_forms[base_id + 13] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 15] = intermediate_monster{is_bigger, is_crystal, is_mage};
  // bigger golden
  is_crystal = false;
  // bigger golden mage
  is_mage = true;
  mc.intermediate_forms[base_id + 10] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 12] = intermediate_monster{is_bigger, is_crystal, is_mage};
  // bigger golden fighter
  is_mage = false;
  mc.intermediate_forms[base_id + 9] = intermediate_monster{is_bigger, is_crystal, is_mage};
  mc.intermediate_forms[base_id + 11] = intermediate_monster{is_bigger, is_crystal, is_mage};
}

beastfarm_feed
beastfarm_data::should_feed_among_category(const monster_category& mc,
                                           int current_monster_template_id,
                                           int wanted_id) const
{
  // base monster can evolve into anything
  if (current_monster_template_id == mc.base_monster_id)
  {
    // if we want top form, we can start with any feed
    if (mc.final_forms.count(wanted_id))
      return beastfarm_feed::any;
    // for intermediate start with proper color
    return mc.intermediate_forms.at(wanted_id).is_crystal ? beastfarm_feed::crystal
                                                          : beastfarm_feed::golden;
  }
  // top monsters dont evolve
  if (mc.final_forms.count(current_monster_template_id))
    return beastfarm_feed::dont_feed;

  // if we got there it must be an intermediate form
  assert(mc.intermediate_forms.count(current_monster_template_id));

  // for intermediate forms check if the result lies on the path of the feeding graph
  // are we going for the top one?
  if (mc.final_forms.count(wanted_id))
  {
    if (mc.final_forms.at(wanted_id).is_mage ==
        mc.intermediate_forms.at(current_monster_template_id).is_mage)
      return mc.intermediate_forms.at(current_monster_template_id).is_crystal
               ? beastfarm_feed::crystal
               : beastfarm_feed::golden;
    // can't reach mage from fighter and vice-versa
    return beastfarm_feed::dont_feed;
  }
  //  dont feed if current stage is bigger, the check above would've returned true earlier
  if (mc.intermediate_forms.at(current_monster_template_id).is_bigger)
    return beastfarm_feed::dont_feed;
  // so current is an intermediate smaller
  assert(!mc.intermediate_forms.at(current_monster_template_id).is_bigger);
  // cannot achieve smaller from smaller
  if (mc.intermediate_forms.at(current_monster_template_id).is_bigger ==
      mc.intermediate_forms.at(wanted_id).is_bigger)
    return beastfarm_feed::dont_feed;
  // so the wanted is bigger, check color and type
  assert(mc.intermediate_forms.at(wanted_id).is_bigger);
  if (mc.intermediate_forms.at(current_monster_template_id).is_crystal ==
        mc.intermediate_forms.at(wanted_id).is_crystal &&
      mc.intermediate_forms.at(current_monster_template_id).is_mage ==
        mc.intermediate_forms.at(wanted_id).is_mage)
  {
    return mc.intermediate_forms.at(current_monster_template_id).is_crystal
             ? beastfarm_feed::crystal
             : beastfarm_feed::golden;
  }
  // no luck
  return beastfarm_feed::dont_feed;
}

beastfarm_feed
beastfarm_data::should_feed(int current_monster_template_id, int wanted_id) const
{
  if (current_monster_template_id == wanted_id)
    return beastfarm_feed::dont_feed;

  if (current_monster_template_id >= buffalos.base_monster_id &&
      current_monster_template_id <= buffalos.base_monster_id + 16 &&
      wanted_id >= buffalos.base_monster_id + 1 && wanted_id <= buffalos.base_monster_id + 18)
  {
    return should_feed_among_category(buffalos, current_monster_template_id, wanted_id);
  }
  if (current_monster_template_id >= kookaburras.base_monster_id &&
      current_monster_template_id <= kookaburras.base_monster_id + 16 &&
      wanted_id >= kookaburras.base_monster_id + 1 && wanted_id <= kookaburras.base_monster_id + 18)
  {
    return should_feed_among_category(kookaburras, current_monster_template_id, wanted_id);
  }
  if (current_monster_template_id >= cougars.base_monster_id &&
      current_monster_template_id <= cougars.base_monster_id + 16 &&
      wanted_id >= cougars.base_monster_id + 1 && wanted_id <= cougars.base_monster_id + 18)
  {
    return should_feed_among_category(cougars, current_monster_template_id, wanted_id);
  }
  return beastfarm_feed::dont_feed;
}

bool
beastfarm_data::is_feedable_template_id(int id) const
{
  return is_buffalo(id) || is_kooka(id) || is_cougar(id);
}

bool
beastfarm_data::is_buffalo(int id) const
{
  return id >= buffalos.base_monster_id && id <= buffalos.base_monster_id + 18;
}

bool
beastfarm_data::is_kooka(int id) const
{
  return id >= kookaburras.base_monster_id && id <= kookaburras.base_monster_id + 18;
}

bool
beastfarm_data::is_cougar(int id) const
{
  return id >= cougars.base_monster_id && id <= cougars.base_monster_id + 18;
}
