#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/dataview.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

class items_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxDataViewListCtrl* items_listctrl;

  std::vector<bot_item> items;
  std::vector<bot_item> most_recent_items;

  void fill_items_listview();
  void on_add_item_button(wxCommandEvent& e);
  void on_remove_item_button(wxCommandEvent& e);

public:
  items_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
