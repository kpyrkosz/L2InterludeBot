#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class manor_data
{
public:
  struct territory_manor_seeds
  {
    std::unordered_map<std::string, int> seed_to_id;
  };

private:
  std::unordered_map<std::string, territory_manor_seeds> seed_datas_;
  std::unordered_map<std::string, int> crop_to_id;
  std::vector<std::string> castle_names_;

  manor_data();

public:
  static const manor_data& get();
  const std::vector<std::string>& castle_names() const;
  int crop_name_to_id(const std::string& name) const;
  const territory_manor_seeds& get_by_castle(const std::string& name) const;
};
