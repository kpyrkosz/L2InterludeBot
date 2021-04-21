#include <party.hpp>

std::unordered_map<int, party::party_member>&
party::members()
{
  return members_;
}

const party::party_member*
party::member_by_nick(const std::wstring& name)
{
  for (const auto& p : members_)
    if (p.second.name == name)
      return &p.second;
  return nullptr;
}

const party::party_member*
party::member_by_nick(const std::string& name)
{
  return member_by_nick(std::wstring{name.begin(), name.end()});
}

void
party::print()
{
  if (members_.empty())
  {
    puts("no party");
    return;
  }
  printf("Party size %u\n", party::get().members().size());
  for (const auto elem : party::get().members())
    printf("ID %X name %ws\n", elem.first, elem.second.name.c_str());
}
