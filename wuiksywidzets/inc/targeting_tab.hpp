#pragma once

#include <base_tab.hpp>
#include <wx/bookctrl.h>
#include <wx/checklst.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

struct targeting_configs
{
  target_option target_opt;
  bool prioritize, assist_wait, retarget;
  std::string target_name;
};

class targeting_tab : public base_tab
{
  wxBookCtrlBase* my_parent_;
  wxSocketBase* associated_socket_;

  wxRadioBox* target_radiobox;
  wxCheckListBox* targeting_listbox;
  wxTextCtrl* player_name_text_ctrl;

  targeting_configs recent_cfg;

  void on_bounding_box_button(wxCommandEvent& e);

public:
  targeting_tab(wxBookCtrlBase* my_parent, wxSocketBase* associated_socket);

  virtual void restore_most_recent_config() override;
  virtual void save_configs(wxFileConfig& config_to_write) override;
  virtual void load_configs(wxFileConfig& config_to_load) override;
  virtual void update_and_serialize(binary_serializer& bs) override;
};
