#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checklst.h>
#include <wx/dataview.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

class craft_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxDataViewListCtrl* craft_listctrl;

  std::vector<bot_craft> crafting;
  std::vector<bot_craft> most_recent_crafting;

  void fill_craft_listview();
  void on_add_craft_button(wxCommandEvent& e);
  void on_remove_craft_button(wxCommandEvent& e);

public:
  craft_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
