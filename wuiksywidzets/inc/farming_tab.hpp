#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checklst.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct farming_config
{
  std::string player_to_follow;
  bool autoattack, spoil, follow_player, movecloser, fakemov, pick, pick_far_away, pick_only_mine,
    pick_in_box, experimental_recharge, pick_close, prioritize_mana_herbs, rest, sweep;
};

class farming_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxCheckListBox* farming_listbox;
  wxTextCtrl* player_to_follow_text_ctrl;

  farming_config recent_cfg;

public:
  farming_tab(wxBookCtrlBase* my_parent);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
