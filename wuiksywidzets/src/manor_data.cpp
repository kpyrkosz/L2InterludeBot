#include <fstream>
#include <manor_data.hpp>

manor_data::manor_data()
  : castle_names_{"Aden",
                  "Dion",
                  "Giran",
                  "Gludio",
                  "Goddard",
                  "Innadril",
                  "Oren",
                  "Rune",
                  "Schuttgart"}
{
  std::ifstream crop_data(R"(rsrc\manor\Manor_crops)");
  if (!crop_data)
    throw std::runtime_error("Crop data file is missing");
  while (!crop_data.eof())
  {
    int id;
    std::string name;
    crop_data >> id;
    crop_data >> std::ws;
    std::getline(crop_data, name);
    crop_to_id[name] = id;
    crop_data >> std::ws;
  }

  for (const auto& castle : castle_names_)
  {
    std::ifstream seed_data(R"(rsrc\manor\Seeds_)" + castle);
    if (!seed_data)
      throw std::runtime_error("Seed data is missing for: " + castle);
    territory_manor_seeds to_add;
    while (!seed_data.eof())
    {
      int id;
      std::string name;
      seed_data >> id;
      seed_data >> std::ws;
      std::getline(seed_data, name);
      to_add.seed_to_id[name] = id;
      seed_data >> std::ws;
    }
    seed_datas_[castle] = std::move(to_add);
  }
}

const manor_data&
manor_data::get()
{
  static manor_data potato;
  return potato;
}

const std::vector<std::string>&
manor_data::castle_names() const
{
  return castle_names_;
}

int
manor_data::crop_name_to_id(const std::string& name) const
{
  return crop_to_id.at(name);
}

const manor_data::territory_manor_seeds&
manor_data::get_by_castle(const std::string& name) const
{
  return seed_datas_.at(name);
}
