#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct alarm_config
{
  bool check_for_spawn;
  std::string spawn_name;
  bool on_enter_bounding_box;
};

class alarm_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;

  wxCheckBox* check_for_spawn;
  wxTextCtrl* spawn_name;
  wxCheckBox* on_enter_bounding_box;
  alarm_config recent_cfg;

public:
  alarm_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
