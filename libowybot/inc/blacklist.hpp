#pragma once

#include <unordered_set>

class blacklist
{
  std::unordered_set<int> exclusive_;
  std::unordered_set<int> ignored_;

public:
  void add_as_ignored(int id);
  void add_as_exclusive(int id);
  void clear();
  bool is_allowed_target(int id) const;
};
