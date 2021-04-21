#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/dataview.h>
#include <wx/wx.h>

class out_of_party_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxDataViewListCtrl* oop_listctrl;

  std::vector<bot_oop_buff> oops;
  std::vector<bot_oop_buff> most_recent_oops;

  void fill_listview();
  void on_add_button(wxCommandEvent& e);
  void on_remove_button(wxCommandEvent& e);

public:
  out_of_party_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
