#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct pick_configs
{
  bool pick_on, non_greedy, out_of_fight, limit_to_box, manaherb_priority;
};

class pick_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;

  wxCheckBox* pick_on;
  wxCheckBox* non_greedy;
  wxCheckBox* out_of_fight;
  wxCheckBox* limit_to_box;
  wxCheckBox* manaherb_priority;

  pick_configs recent_cfg;

public:
  pick_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
