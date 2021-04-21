#include <event_identifiers.hpp>
#include <follow_tab.hpp>

follow_tab::follow_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
  , recent_cfg{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  player_name = new wxTextCtrl(content_panel, wxID_ANY);
  follow_attack_like = new wxCheckBox(content_panel, wxID_ANY, "Follow by attack");
  keep_distance = new wxCheckBox(content_panel, wxID_ANY, "Keep distance between");
  dist_min = new wxTextCtrl(content_panel, wxID_ANY);
  dist_max = new wxTextCtrl(content_panel, wxID_ANY);

  dist_min->SetValue("200");
  dist_max->SetValue("500");

  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Player to follow"));
  content_sizer->Add(follow_attack_like);
  content_sizer->Add(keep_distance);
  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Min distance"));
  content_sizer->Add(dist_min);
  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Max distance"));
  content_sizer->Add(dist_max);

  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Follow", false, 9);
}

void
follow_tab::restore_most_recent_config()
{
  player_name->SetValue(recent_cfg.player_name);
  follow_attack_like->SetValue(recent_cfg.follow_attack_like);
  keep_distance->SetValue(recent_cfg.keep_distance);
  dist_min->SetValue(wxString::Format("%d", recent_cfg.dist_min));
  dist_max->SetValue(wxString::Format("%d", recent_cfg.dist_max));
}

void
follow_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("player_name", player_name->GetValue());
  config_to_write.Write("follow_attack_like", follow_attack_like->GetValue());
  config_to_write.Write("keep_distance", keep_distance->GetValue());
  config_to_write.Write("dist_min", dist_min->GetValue());
  config_to_write.Write("dist_max", dist_max->GetValue());
}

void
follow_tab::load_configs(wxFileConfig& config_to_load)
{
  player_name->SetValue(config_to_load.ReadObject("spawn_name", wxString()));
  follow_attack_like->SetValue(config_to_load.ReadBool("follow_attack_like", false));
  keep_distance->SetValue(config_to_load.ReadBool("keep_distance", false));
  dist_min->SetValue(config_to_load.ReadObject("dist_min", wxString("200")));
  dist_max->SetValue(config_to_load.ReadObject("dist_max", wxString("500")));
}

void
follow_tab::update_and_serialize(binary_serializer& bs)
{
  recent_cfg.player_name = player_name->GetValue();
  recent_cfg.follow_attack_like = follow_attack_like->GetValue();
  recent_cfg.keep_distance = keep_distance->GetValue();
  recent_cfg.dist_min = wxAtoi(dist_min->GetValue());
  recent_cfg.dist_max = wxAtoi(dist_max->GetValue());

  bs << to_bot_protocol::follow << recent_cfg.player_name << recent_cfg.follow_attack_like
     << recent_cfg.keep_distance << recent_cfg.dist_min << recent_cfg.dist_max;
}
