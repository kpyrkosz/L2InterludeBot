#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checklst.h>
#include <wx/dataview.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

class manor_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxDataViewListCtrl* manor_listctrl;

  std::vector<bot_manor> manor_seeds;
  std::vector<bot_manor> most_recent_manor_seeds;

  void fill_manor_listview();
  void on_add_manor_button(wxCommandEvent& e);
  void on_remove_manor_button(wxCommandEvent& e);

public:
  manor_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
