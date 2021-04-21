#include <fstream>
#include <item_data.hpp>

item_data::item_data()
{
  std::ifstream filein(R"(rsrc\items_clean)");
  if (!filein)
    throw std::runtime_error("Item data file is missing");
  while (!filein.eof())
  {
    int id;
    std::string name;
    filein >> id;
    filein >> std::ws;
    std::getline(filein, name);
    id_to_name_[id] = name;
    filein >> std::ws;
  }
}

const item_data&
item_data::get()
{
  static item_data potato;
  return potato;
}

const std::string&
item_data::id_to_name(int id) const
{
  return id_to_name_.at(id);
}
