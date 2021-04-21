#pragma once

#include <unordered_map>
#include <vector>

struct manor_crop
{
  int objid;
  int itemid;
  int castle;
  int count;
};

struct manor_buy_seed
{
  int itemid;
  int count;
};

struct crop_info
{
  int reward_item_id;
  int reward_total_price;
  int crop_id;
  int cropcount;
  int castleid;
};

namespace manor_crops
{
std::vector<manor_crop>& expectatios();
std::unordered_map<int, std::vector<crop_info>>& crop_buy_data();
std::unordered_map<int, std::wstring>& castle_names();
} // namespace manor_crops
