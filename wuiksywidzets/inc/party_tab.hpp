#pragma once

#include <base_tab.hpp>
#include <string>
#include <wx/bookctrl.h>
#include <wx/dataview.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct party_configs
{
  std::string party_inviter;
  bool accept_party;
};

class party_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxTextCtrl* party_inviter_textctrl;
  wxDataViewListCtrl* party_listctrl;
  wxCheckBox* accept_pt;

  std::vector<bot_buff> buffs;
  std::vector<bot_buff> most_recent_buffs;

  party_configs most_recent_party_configs;

  void fill_party_listview();
  void on_add_buff_button(wxCommandEvent& e);
  void on_remove_buff_button(wxCommandEvent& e);

public:
  party_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
