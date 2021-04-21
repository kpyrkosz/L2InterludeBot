#pragma once

#include <base_tab.hpp>
#include <current_character_data.hpp>
#include <wx/bookctrl.h>
#include <wx/checklst.h>
#include <wx/dataview.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

class spells_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxDataViewListCtrl* spells_listctrl;

  std::vector<bot_spell> spells;
  std::vector<bot_spell> most_recent_spells;
  const current_character_data& ccd_;

  void fill_spells_listview();
  void on_add_spell_button(wxCommandEvent& e);
  void on_remove_spell_button(wxCommandEvent& e);

public:
  spells_tab(wxBookCtrlBase* my_parent, const current_character_data& ccd);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
