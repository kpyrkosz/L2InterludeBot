#include <algorithm>
#include <blacklist.hpp>

void
blacklist::add_as_ignored(int id)
{
  ignored_.insert(id);
}

void
blacklist::add_as_exclusive(int id)
{
  exclusive_.insert(id);
}

void
blacklist::clear()
{
  ignored_.clear();
  exclusive_.clear();
}

bool
blacklist::is_allowed_target(int id) const
{
  // if exclusive is not empty, we allow only those IDs
  if (!exclusive_.empty() && exclusive_.count(id) == 0)
    return false;
  // if ignored is not empty, filter out the bad guys
  if (!ignored_.empty() && ignored_.count(id))
    return false;
  return true;
}
