#include <alarm_tab.hpp>
#include <binary_serializer.hpp>
#include <event_identifiers.hpp>

alarm_tab::alarm_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
  , recent_cfg{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  check_for_spawn = new wxCheckBox(content_panel, wxID_ANY, "Alarm on monster/rb spawn");
  spawn_name = new wxTextCtrl(content_panel, wxID_ANY);
  on_enter_bounding_box =
    new wxCheckBox(content_panel, wxID_ANY, "Alarm on nonparty in bounding box");

  content_sizer->Add(check_for_spawn);
  content_sizer->Add(spawn_name);
  content_sizer->Add(on_enter_bounding_box);

  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Alarm", false, 8);
}

void
alarm_tab::restore_most_recent_config()
{
  check_for_spawn->SetValue(recent_cfg.check_for_spawn);
  spawn_name->SetValue(recent_cfg.spawn_name);
  on_enter_bounding_box->SetValue(recent_cfg.on_enter_bounding_box);
}

void
alarm_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("check_for_spawn", check_for_spawn->GetValue());
  config_to_write.Write("spawn_name", spawn_name->GetValue());
  config_to_write.Write("on_enter_bounding_box", on_enter_bounding_box->GetValue());
}

void
alarm_tab::load_configs(wxFileConfig& config_to_load)
{
  check_for_spawn->SetValue(config_to_load.ReadBool("check_for_spawn", false));
  spawn_name->SetValue(config_to_load.ReadObject("spawn_name", wxString()));
  on_enter_bounding_box->SetValue(config_to_load.ReadBool("on_enter_bounding_box", false));
}

void
alarm_tab::update_and_serialize(binary_serializer& bs)
{
  recent_cfg.check_for_spawn = check_for_spawn->GetValue();
  recent_cfg.spawn_name = spawn_name->GetValue();
  recent_cfg.on_enter_bounding_box = on_enter_bounding_box->GetValue();

  bs << to_bot_protocol::alarm << recent_cfg.check_for_spawn << recent_cfg.spawn_name
     << recent_cfg.on_enter_bounding_box;
}
