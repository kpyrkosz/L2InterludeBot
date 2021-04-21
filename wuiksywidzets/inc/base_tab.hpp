#pragma once

#include <array>
#include <binary_serializer.hpp>
#include <bot_config.hpp>
#include <event_identifiers.hpp>
#include <wx/fileconf.h>

class base_tab
{
public:
  virtual void restore_most_recent_config() = 0;
  virtual void save_configs(wxFileConfig& config_to_write) = 0;
  virtual void load_configs(wxFileConfig& config_to_load) = 0;
  virtual void update_and_serialize(binary_serializer& bs) = 0;
  virtual ~base_tab() = default;
};
