#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct follow_config
{
  std::string player_name;
  bool follow_attack_like;
  bool keep_distance;
  int dist_min;
  int dist_max;
};

class follow_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;

  wxTextCtrl* player_name;
  wxCheckBox* follow_attack_like;
  wxCheckBox* keep_distance;
  wxTextCtrl* dist_min;
  wxTextCtrl* dist_max;

  follow_config recent_cfg;

public:
  follow_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
