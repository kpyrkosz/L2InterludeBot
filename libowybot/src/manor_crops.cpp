#include <manor_crops.hpp>

std::vector<manor_crop>&
manor_crops::expectatios()
{
  static std::vector<manor_crop> potato;
  return potato;
}

std::unordered_map<int, std::vector<crop_info>>&
manor_crops::crop_buy_data()
{
  static std::unordered_map<int, std::vector<crop_info>> potato;
  return potato;
}

std::unordered_map<int, std::wstring>&
manor_crops::castle_names()
{
  static std::unordered_map<int, std::wstring> potato;
  return potato;
}
