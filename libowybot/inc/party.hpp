#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class party
{
public:
  struct active_buff
  {
    int id;
    int expiration_time;
  };

  struct party_member
  {
    int objid, x, y, z;
    int hp, maxhp, mp, maxmp, cp, maxcp;
    std::unordered_map<int, int> buffid_to_expiration_time;
    std::wstring name;
  };

private:
  std::unordered_map<int, party_member> members_;

public:
  static party& get()
  {
    static party potato;
    return potato;
  }

  std::unordered_map<int, party::party_member>& party::members();
  const party_member* member_by_nick(const std::wstring& name);
  const party_member* member_by_nick(const std::string& name);
  void print();
};
