#include <bot_confiuration.hpp>

settings&
get_bot_settings()
{
  static settings potato{};
  return potato;
}
