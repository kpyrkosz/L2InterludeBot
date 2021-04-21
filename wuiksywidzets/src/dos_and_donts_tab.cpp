#include <add_dos_dialog.hpp>
#include <dos_and_donts_tab.hpp>

void
dos_and_donts_tab::fill_dos_and_donts_listview()
{
  dos_listctrl->DeleteAllItems();
  for (const auto r : rules)
  {
    wxVector<wxVariant> data;
    switch (r.brw)
    {
      case bot_rule_what::target:
        data.push_back("TARGET");
        break;
      case bot_rule_what::attack:
        data.push_back("ATTACK");
        break;
      case bot_rule_what::manor:
        data.push_back("MANOR");
        break;
      case bot_rule_what::spoil:
        data.push_back("SPOIL");
        break;
    }
    data.push_back(r.is_include ? "INCLUDE" : "IGNORE");
    data.push_back(wxString::Format("%d", r.mob_id));
    dos_listctrl->AppendItem(data);
  }
}

void
dos_and_donts_tab::on_add_dos_button(wxCommandEvent& e)
{
  add_dos_dialog qq(my_parent_,
                    [this](bot_rule&& to_add) { rules.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_dos_and_donts_listview();
}

void
dos_and_donts_tab::on_remove_dos_button(wxCommandEvent& e)
{
  auto selected = dos_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  rules.erase(rules.begin() + selected);
  fill_dos_and_donts_listview();
}

dos_and_donts_tab::dos_and_donts_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  dos_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  dos_listctrl->AppendTextColumn("What", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  dos_listctrl->AppendTextColumn("Mode", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  dos_listctrl->AppendTextColumn("Who", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add =
    new wxButton(content_panel, wxID_ANY, "Add rule", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove =
    new wxButton(content_panel, wxID_ANY, "Remove rule", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove, wxSizerFlags().DoubleBorder());
  add->Bind(wxEVT_BUTTON, &dos_and_donts_tab::on_add_dos_button, this);
  remove->Bind(wxEVT_BUTTON, &dos_and_donts_tab::on_remove_dos_button, this);

  content_sizer->Add(dos_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Dos and donts", false, 7);
}

void
dos_and_donts_tab::restore_most_recent_config()
{
  rules = most_recent_rules;
  fill_dos_and_donts_listview();
}

void
dos_and_donts_tab::save_configs(wxFileConfig& config_to_write)
{
}

void
dos_and_donts_tab::load_configs(wxFileConfig& config_to_load)
{
}

void
dos_and_donts_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_rules = rules;

  bs << to_bot_protocol::dos << rules.size();
  for (const auto& r : rules)
    bs << r.brw << r.is_include << r.mob_id;
}
