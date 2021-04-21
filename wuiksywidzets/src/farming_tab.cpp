#include <farming_tab.hpp>

farming_tab::farming_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  wxString details[] = {"Autoattack",
                        "Spoil CHECK ALSO SWEEP bottom lol",
                        "Follow player",
                        "Move closer before attacking",
                        "Fake movement before moving to monster",
                        "Pick items",
                        "[[deprecated]]Pick only mine/party",
                        "[[deprecated]]Pick far away",
                        "Pick only in box",
                        "Start recharging BLAU 40%-80%",
                        "Pick close to blau",
                        "Prioritize manaherbs if mana is < 80%",
                        "Rest when mana is 15% or lower",
                        "Sweep"};
  farming_listbox = new wxCheckListBox(content_panel,
                                       wxID_ANY,
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       sizeof(details) / sizeof(details[0]),
                                       details);
  player_to_follow_text_ctrl = new wxTextCtrl(content_panel, wxID_ANY);
  content_sizer->Add(farming_listbox);
  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Enter player name to follow"));
  content_sizer->Add(player_to_follow_text_ctrl);
  content_panel->SetSizer(content_sizer);
  my_parent->AddPage(content_panel, "Farming", false, 1);
}

void
farming_tab::restore_most_recent_config()
{
  player_to_follow_text_ctrl->SetValue(recent_cfg.player_to_follow);

  farming_listbox->Check(0, recent_cfg.autoattack);
  farming_listbox->Check(1, recent_cfg.spoil);
  farming_listbox->Check(2, recent_cfg.follow_player);
  farming_listbox->Check(3, recent_cfg.movecloser);
  farming_listbox->Check(4, recent_cfg.fakemov);
  farming_listbox->Check(5, recent_cfg.pick);
  farming_listbox->Check(6, recent_cfg.pick_only_mine);
  farming_listbox->Check(7, recent_cfg.pick_far_away);
  farming_listbox->Check(8, recent_cfg.pick_in_box);
  farming_listbox->Check(9, recent_cfg.experimental_recharge);
  farming_listbox->Check(10, recent_cfg.pick_close);
  farming_listbox->Check(11, recent_cfg.prioritize_mana_herbs);
  farming_listbox->Check(12, recent_cfg.rest);
  farming_listbox->Check(13, recent_cfg.sweep);
}

void
farming_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("player_to_follow", player_to_follow_text_ctrl->GetValue());
  config_to_write.Write("autoattack", farming_listbox->IsChecked(0));
  config_to_write.Write("spoil", farming_listbox->IsChecked(1));
  config_to_write.Write("follow_player", farming_listbox->IsChecked(2));
  config_to_write.Write("movecloser", farming_listbox->IsChecked(3));
  config_to_write.Write("fakemov", farming_listbox->IsChecked(4));
  config_to_write.Write("pick", farming_listbox->IsChecked(5));
  config_to_write.Write("pick_only_mine", farming_listbox->IsChecked(6));
  config_to_write.Write("pick_far_away", farming_listbox->IsChecked(7));
  config_to_write.Write("pick_in_box", farming_listbox->IsChecked(8));
  config_to_write.Write("experimental_recharge", farming_listbox->IsChecked(9));
  config_to_write.Write("pick_close", farming_listbox->IsChecked(10));
  config_to_write.Write("prioritize_mana_herbs", farming_listbox->IsChecked(11));
  config_to_write.Write("rest", farming_listbox->IsChecked(12));
  config_to_write.Write("sweep", farming_listbox->IsChecked(13));
}

void
farming_tab::load_configs(wxFileConfig& config_to_load)
{
  player_to_follow_text_ctrl->SetValue(config_to_load.ReadObject("player_to_follow", wxString()));

  farming_listbox->Check(0, config_to_load.ReadBool("autoattack", false));
  farming_listbox->Check(1, config_to_load.ReadBool("spoil", false));
  farming_listbox->Check(2, config_to_load.ReadBool("follow_player", false));
  farming_listbox->Check(3, config_to_load.ReadBool("movecloser", false));
  farming_listbox->Check(4, config_to_load.ReadBool("fakemov", false));
  farming_listbox->Check(5, config_to_load.ReadBool("pick", false));
  farming_listbox->Check(6, config_to_load.ReadBool("pick_only_mine", false));
  farming_listbox->Check(7, config_to_load.ReadBool("pick_far_away", false));
  farming_listbox->Check(8, config_to_load.ReadBool("pick_in_box", false));
  farming_listbox->Check(9, config_to_load.ReadBool("experimental_recharge", false));
  farming_listbox->Check(10, config_to_load.ReadBool("pick_close", false));
  farming_listbox->Check(11, config_to_load.ReadBool("prioritize_mana_herbs", false));
  farming_listbox->Check(12, config_to_load.ReadBool("rest", false));
  farming_listbox->Check(13, config_to_load.ReadBool("sweep", false));
}

void
farming_tab::update_and_serialize(binary_serializer& bs)
{
  recent_cfg.player_to_follow = player_to_follow_text_ctrl->GetValue().c_str();
  recent_cfg.autoattack = farming_listbox->IsChecked(0);
  recent_cfg.spoil = farming_listbox->IsChecked(1);
  recent_cfg.follow_player = farming_listbox->IsChecked(2);
  recent_cfg.movecloser = farming_listbox->IsChecked(3);
  recent_cfg.fakemov = farming_listbox->IsChecked(4);
  recent_cfg.pick = farming_listbox->IsChecked(5);
  recent_cfg.pick_only_mine = farming_listbox->IsChecked(6);
  recent_cfg.pick_far_away = farming_listbox->IsChecked(7);
  recent_cfg.pick_in_box = farming_listbox->IsChecked(8);
  recent_cfg.experimental_recharge = farming_listbox->IsChecked(9);
  recent_cfg.pick_close = farming_listbox->IsChecked(10);
  recent_cfg.prioritize_mana_herbs = farming_listbox->IsChecked(11);
  recent_cfg.rest = farming_listbox->IsChecked(12);
  recent_cfg.sweep = farming_listbox->IsChecked(13);

  bs << to_bot_protocol::farming << recent_cfg.player_to_follow << recent_cfg.autoattack
     << recent_cfg.spoil << recent_cfg.follow_player << recent_cfg.movecloser << recent_cfg.fakemov
     << recent_cfg.pick << recent_cfg.pick_far_away << recent_cfg.pick_only_mine
     << recent_cfg.pick_in_box << recent_cfg.experimental_recharge << recent_cfg.pick_close
     << recent_cfg.prioritize_mana_herbs << recent_cfg.rest << recent_cfg.sweep;
}
