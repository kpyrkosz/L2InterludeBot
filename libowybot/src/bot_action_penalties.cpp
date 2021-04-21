#include <Windows.h>
#include <bot_action_penalties.hpp>

bot_action_penalties&
get_penalties()
{
  static bot_action_penalties potato{};
  return potato;
}

bool
is_under_penalty(int penalty)
{
  return penalty > GetTickCount();
}

void
update_penalty(int& penalty, int millisec_to_add)
{
  penalty = GetTickCount() + millisec_to_add;
}
