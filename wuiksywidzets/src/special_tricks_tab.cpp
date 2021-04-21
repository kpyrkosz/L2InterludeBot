#include <event_identifiers.hpp>
#include <special_tricks_tab.hpp>

special_tricks_tab::special_tricks_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
  , recent_cfg{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  /*

  wxCheckBox* remove_trained_beastfarm_animals;
  wxCheckBox* turn_cancel_animation_visible;
  */
  remove_trained_beastfarm_animals =
    new wxCheckBox(content_panel, wxID_ANY, "Dont show trained animals on beast farm");
  turn_cancel_animation_visible =
    new wxCheckBox(content_panel, wxID_ANY, "Change cancel cast animation");
  dont_show_droped_arrows = new wxCheckBox(content_panel, wxID_ANY, "Dont show arrow drops");

  content_sizer->Add(remove_trained_beastfarm_animals);
  content_sizer->Add(turn_cancel_animation_visible);
  content_sizer->Add(dont_show_droped_arrows);

  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "SpEcIaL TrIcKs", false, 13);
}

void
special_tricks_tab::restore_most_recent_config()
{
  remove_trained_beastfarm_animals->SetValue(recent_cfg.remove_trained_beastfarm_animals);
  turn_cancel_animation_visible->SetValue(recent_cfg.turn_cancel_animation_visible);
  dont_show_droped_arrows->SetValue(recent_cfg.dont_show_droped_arrows);
}

void
special_tricks_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("remove_trained_beastfarm_animals",
                        remove_trained_beastfarm_animals->GetValue());
  config_to_write.Write("turn_cancel_animation_visible", turn_cancel_animation_visible->GetValue());
  config_to_write.Write("dont_show_droped_arrows", dont_show_droped_arrows->GetValue());
}

void
special_tricks_tab::load_configs(wxFileConfig& config_to_load)
{
  remove_trained_beastfarm_animals->SetValue(
    config_to_load.ReadBool("remove_trained_beastfarm_animals", false));
  turn_cancel_animation_visible->SetValue(
    config_to_load.ReadBool("turn_cancel_animation_visible", false));
  dont_show_droped_arrows->SetValue(config_to_load.ReadBool("dont_show_droped_arrows", false));
}

void
special_tricks_tab::update_and_serialize(binary_serializer& bs)
{
  recent_cfg.remove_trained_beastfarm_animals = remove_trained_beastfarm_animals->GetValue();
  recent_cfg.turn_cancel_animation_visible = turn_cancel_animation_visible->GetValue();
  recent_cfg.dont_show_droped_arrows = dont_show_droped_arrows->GetValue();

  bs << to_bot_protocol::tricks << recent_cfg.remove_trained_beastfarm_animals
     << recent_cfg.turn_cancel_animation_visible << recent_cfg.dont_show_droped_arrows;
}
