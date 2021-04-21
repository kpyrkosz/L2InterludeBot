#pragma once

#include <manor_crops.hpp>
#include <native_lineage_classes.hpp>
#include <vector>

namespace action_executor
{
void init_anti_antibots();
void whisper(const wchar_t* msg, const wchar_t* receiver);
void action(int objid, bool shift = false);
void to_village();
void ctrl_attack(int objid, bool shift = false);
void use_spell(int spellid, bool shift = false, bool ctrl = false);
void move_to_xyz(const FVector& xyz);
void use_item(int skillid, bool ctrl_pressed = false);
void sell_crops(const std::vector<manor_crop>& crops);
void buy_seeds(int manor_id, const std::vector<manor_buy_seed>& seeds);
void craft_click(int recipe_id);
} // namespace action_executor
