#include <fstream>
#include <npc_data.hpp>

npc_data::npc_data()
{
  std::ifstream filein(R"(rsrc\npcname)");
  if (!filein)
    throw std::runtime_error("Npc data file is missing");
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

const npc_data&
npc_data::get()
{
  static npc_data potato;
  return potato;
}

const std::string&
npc_data::id_to_name(int id) const
{
  return id_to_name_.at(id);
}
