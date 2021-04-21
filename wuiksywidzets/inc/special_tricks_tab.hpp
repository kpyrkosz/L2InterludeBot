#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checkbox.h>
#include <wx/wx.h>

struct tricks_configs
{
  bool remove_trained_beastfarm_animals;
  bool turn_cancel_animation_visible;
  bool dont_show_droped_arrows;
};

class special_tricks_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;

  wxCheckBox* remove_trained_beastfarm_animals;
  wxCheckBox* turn_cancel_animation_visible;
  wxCheckBox* dont_show_droped_arrows;

  tricks_configs recent_cfg;

public:
  special_tricks_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
