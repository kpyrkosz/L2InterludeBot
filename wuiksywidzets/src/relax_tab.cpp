#include <event_identifiers.hpp>
#include <relax_tab.hpp>

relax_tab::relax_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Relax, nothing here yet ;)"));

  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Relax", false, 11);
}

void
relax_tab::restore_most_recent_config()
{
}

void
relax_tab::save_configs(wxFileConfig& config_to_write)
{
}

void
relax_tab::load_configs(wxFileConfig& config_to_load)
{
}

void
relax_tab::update_and_serialize(binary_serializer& bs)
{
  bs << to_bot_protocol::relax;
}
