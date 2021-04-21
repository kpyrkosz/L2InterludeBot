#include <aes_boy.hpp>
#include <binary_serializer.hpp>
#include <event_identifiers.hpp>
#include <targeting_tab.hpp>

void
targeting_tab::on_bounding_box_button(wxCommandEvent& e)
{
  // todo, this can be outsourced
  wxLogMessage("Requesting current XYZ");
  std::array<char, 2048> to_send;
  binary_serializer bs(to_send);
  bs << to_bot_protocol::request_your_position;
  auto total_size = bs.get_current_size();
  associated_socket_->Write((char*)&total_size, sizeof(total_size));
  aes_boy::inplace_encrypt_to_dll(to_send.data(), total_size);
  associated_socket_->Write(to_send.data(), total_size);
}

targeting_tab::targeting_tab(wxBookCtrlBase* my_parent, wxSocketBase* associated_socket)
  : my_parent_(my_parent)
  , associated_socket_(associated_socket)
  , recent_cfg{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  wxString target_opts[] = {"Don't target",
                            "Target nearest monster",
                            "Assist player",
                            "Keep target on player",
                            "Dead sweepable"};
  target_radiobox = new wxRadioBox(content_panel,
                                   wxID_ANY,
                                   "Targeting option",
                                   wxDefaultPosition,
                                   wxDefaultSize,
                                   sizeof(target_opts) / sizeof(target_opts[0]),
                                   target_opts,
                                   1,
                                   wxRA_SPECIFY_COLS);
  // TO MA SIE WLACZAC WYLACZNIE GDY KTOS WYBIERZE TARGET MONSTER
  wxString monster_target_details[] = {"Prioritize monsters attacking me or party",
                                       "In assist mode, wait for targeter to start attack",
                                       "If monsters spawns while running to target, retarget?"};
  targeting_listbox =
    new wxCheckListBox(content_panel,
                       wxID_ANY,
                       wxDefaultPosition,
                       wxDefaultSize,
                       sizeof(monster_target_details) / sizeof(monster_target_details[0]),
                       monster_target_details);
  // TEXTBOX TYLKO ASIST/KEEP
  player_name_text_ctrl = new wxTextCtrl(content_panel, wxID_ANY);

  content_sizer->Add(target_radiobox);
  content_sizer->Add(targeting_listbox);
  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Enter player name"));
  content_sizer->Add(player_name_text_ctrl);
  wxButton* setup_box =
    new wxButton(content_panel, wxID_ANY, "Setup bounding box", wxDefaultPosition, wxSize(150, 25));
  setup_box->Bind(wxEVT_BUTTON, &targeting_tab::on_bounding_box_button, this);

  content_sizer->Add(setup_box);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Targeting", false, 0);
}

void
targeting_tab::restore_most_recent_config()
{
  switch (recent_cfg.target_opt)
  {
    case target_option::none:
      target_radiobox->Select(0);
      break;
    case target_option::monsters:
      target_radiobox->Select(1);
      break;
    case target_option::assist:
      target_radiobox->Select(2);
      break;
    case target_option::keep_on_player:
      target_radiobox->Select(3);
      break;
    case target_option::dead_sweepable:
      target_radiobox->Select(4);
      break;
  }

  player_name_text_ctrl->SetValue(recent_cfg.target_name);
  targeting_listbox->Check(0, recent_cfg.prioritize);
  targeting_listbox->Check(1, recent_cfg.assist_wait);
  targeting_listbox->Check(2, recent_cfg.retarget);
}

void
targeting_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("target_name", player_name_text_ctrl->GetValue());
  config_to_write.Write("prioritize", targeting_listbox->IsChecked(0));
  config_to_write.Write("assist_wait", targeting_listbox->IsChecked(1));
  config_to_write.Write("retarget", targeting_listbox->IsChecked(2));
  config_to_write.Write("target", target_radiobox->GetSelection());
}

void
targeting_tab::load_configs(wxFileConfig& config_to_load)
{
  player_name_text_ctrl->SetValue(config_to_load.ReadObject("target_name", wxString()));
  targeting_listbox->Check(0, config_to_load.ReadBool("prioritize", false));
  targeting_listbox->Check(1, config_to_load.ReadBool("assist_wait", false));
  targeting_listbox->Check(2, config_to_load.ReadBool("retarget", false));
  target_radiobox->Select(config_to_load.ReadLong("target", 0));
}

void
targeting_tab::update_and_serialize(binary_serializer& bs)
{
  recent_cfg.target_name = player_name_text_ctrl->GetValue().c_str();
  recent_cfg.prioritize = targeting_listbox->IsChecked(0);
  recent_cfg.assist_wait = targeting_listbox->IsChecked(1);
  recent_cfg.retarget = targeting_listbox->IsChecked(2);

  switch (target_radiobox->GetSelection())
  {
    case 0:
      recent_cfg.target_opt = target_option::none;
      break;
    case 1:
      recent_cfg.target_opt = target_option::monsters;
      break;
    case 2:
      recent_cfg.target_opt = target_option::assist;
      break;
    case 3:
      recent_cfg.target_opt = target_option::keep_on_player;
      break;
    case 4:
      recent_cfg.target_opt = target_option::dead_sweepable;
      break;
  }
  bs << to_bot_protocol::target << (char)recent_cfg.target_opt << recent_cfg.target_name
     << recent_cfg.prioritize << recent_cfg.assist_wait << recent_cfg.retarget;
}
