#include <craft_data.hpp>
#include <fstream>

craft_data::craft_data()
{
  std::ifstream rcp_data(R"(rsrc\recipes_clean)");
  if (!rcp_data)
    throw std::runtime_error("Recipe data file is missing");
  while (!rcp_data.eof())
  {
    int id;
    std::string name;
    rcp_data >> id;
    rcp_data >> std::ws;
    std::getline(rcp_data, name);
    recipe_to_id_[name] = id;
    id_to_recipe_[id] = name;
    rcp_data >> std::ws;
  }
}

const craft_data&
craft_data::get()
{
  static craft_data potato;
  return potato;
}

const std::unordered_map<std::string, int>&
craft_data::get_data() const
{
  return recipe_to_id_;
}

const std::string&
craft_data::name_from_id(int id) const
{
  return id_to_recipe_.at(id);
}
