#include <event_identifiers.hpp>
#include <pick_tab.hpp>

pick_tab::pick_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
  , recent_cfg{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  pick_on = new wxCheckBox(content_panel, wxID_ANY, "Enable picking");
  non_greedy = new wxCheckBox(content_panel, wxID_ANY, "Non greedy pick");
  out_of_fight = new wxCheckBox(content_panel, wxID_ANY, "Pick only out of fight");
  limit_to_box = new wxCheckBox(content_panel, wxID_ANY, "Pick only in box");
  manaherb_priority = new wxCheckBox(content_panel, wxID_ANY, "Prioritize manaherbs below 80% mp");

  content_sizer->Add(pick_on);
  content_sizer->Add(non_greedy);
  content_sizer->Add(out_of_fight);
  content_sizer->Add(limit_to_box);
  content_sizer->Add(manaherb_priority);

  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Picking", false, 10);
}

void
pick_tab::restore_most_recent_config()
{
  pick_on->SetValue(recent_cfg.pick_on);
  non_greedy->SetValue(recent_cfg.non_greedy);
  out_of_fight->SetValue(recent_cfg.out_of_fight);
  limit_to_box->SetValue(recent_cfg.limit_to_box);
  manaherb_priority->SetValue(recent_cfg.manaherb_priority);
}

void
pick_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("pick_on", pick_on->GetValue());
  config_to_write.Write("non_greedy", non_greedy->GetValue());
  config_to_write.Write("out_of_fight", out_of_fight->GetValue());
  config_to_write.Write("limit_to_box", limit_to_box->GetValue());
  config_to_write.Write("manaherb_priority", manaherb_priority->GetValue());
}

void
pick_tab::load_configs(wxFileConfig& config_to_load)
{
  pick_on->SetValue(config_to_load.ReadBool("pick_on", false));
  non_greedy->SetValue(config_to_load.ReadBool("non_greedy", false));
  out_of_fight->SetValue(config_to_load.ReadBool("out_of_fight", false));
  limit_to_box->SetValue(config_to_load.ReadBool("limit_to_box", false));
  manaherb_priority->SetValue(config_to_load.ReadBool("manaherb_priority", false));
}

void
pick_tab::update_and_serialize(binary_serializer& bs)
{
  recent_cfg.pick_on = pick_on->GetValue();
  recent_cfg.non_greedy = non_greedy->GetValue();
  recent_cfg.out_of_fight = out_of_fight->GetValue();
  recent_cfg.limit_to_box = limit_to_box->GetValue();
  recent_cfg.manaherb_priority = manaherb_priority->GetValue();

  bs << to_bot_protocol::pick << recent_cfg.pick_on << recent_cfg.non_greedy
     << recent_cfg.out_of_fight << recent_cfg.limit_to_box << recent_cfg.manaherb_priority;
}
