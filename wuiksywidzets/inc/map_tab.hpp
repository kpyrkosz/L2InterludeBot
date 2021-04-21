#pragma once

#include <base_tab.hpp>
#include <bounding_box_selection_panel.hpp> //temp
#include <wx/bookctrl.h>
#include <wx/wx.h>

class map_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;

public:
  map_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
