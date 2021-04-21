#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checklst.h>
#include <wx/dataview.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

class dos_and_donts_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxDataViewListCtrl* dos_listctrl;

  std::vector<bot_rule> rules;
  std::vector<bot_rule> most_recent_rules;

  void fill_dos_and_donts_listview();
  void on_add_dos_button(wxCommandEvent& e);
  void on_remove_dos_button(wxCommandEvent& e);

public:
  dos_and_donts_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
