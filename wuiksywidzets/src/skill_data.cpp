#include <fstream>
#include <skill_data.hpp>

skill_data::skill_data()
{
  std::ifstream filein(R"(rsrc\skills_clean)");
  if (!filein)
    throw std::runtime_error("Skill data file is missing");
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

const skill_data&
skill_data::get()
{
  static skill_data potato;
  return potato;
}

const std::string&
skill_data::id_to_name(int id) const
{
  return id_to_name_.at(id);
}
