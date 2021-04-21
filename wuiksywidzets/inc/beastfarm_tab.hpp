#pragma once

#include <base_tab.hpp>
#include <string>
#include <wx/bookctrl.h>
#include <wx/dataview.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct beastfarm_config
{
  bool prefer_buffalo;
  bool prefer_kooka;
  bool prefer_cougar;
};

class beastfarm_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;

  wxDataViewListCtrl* listctrl;
  wxCheckBox* prioritize_buffalo;
  wxCheckBox* prioritize_kooka;
  wxCheckBox* prioritize_cougar;

  std::vector<bot_beast> beasts;
  std::vector<bot_beast> most_recent_beasts;

  beastfarm_config most_recent_configs;

  void fill_listview();
  void on_add_button(wxCommandEvent& e);
  void on_remove_button(wxCommandEvent& e);

public:
  beastfarm_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
