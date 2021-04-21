#include <add_oop_dialog.hpp>
#include <out_of_party_tab.hpp>
#include <skill_data.hpp>

void
out_of_party_tab::fill_listview()
{
  oop_listctrl->DeleteAllItems();
  for (const auto oop : oops)
  {
    wxVector<wxVariant> data;
    data.push_back(wxVariant(oop.active));
    data.push_back(
      wxString::Format("%s (%d)", skill_data::get().id_to_name(oop.spellid), oop.spellid));
    data.push_back(oop.playername);
    data.push_back(wxString::Format("%d", oop.buff_before));
    data.push_back(oop.is_targetless ? "YES" : "NO");
    data.push_back(oop.requires_invite ? "YES" : "NO");
    oop_listctrl->AppendItem(data);
  }
}

void
out_of_party_tab::on_add_button(wxCommandEvent& e)
{
  add_oop_dialog qq(my_parent_,
                    [this](bot_oop_buff&& to_add) { oops.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_listview();
}

void
out_of_party_tab::on_remove_button(wxCommandEvent& e)
{
  auto selected = oop_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  oops.erase(oops.begin() + selected);
  fill_listview();
}

out_of_party_tab::out_of_party_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  oop_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  oop_listctrl->AppendToggleColumn(
    "Active", wxDATAVIEW_CELL_ACTIVATABLE, wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER);
  oop_listctrl->AppendTextColumn(
    "Spell", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  oop_listctrl->AppendTextColumn(
    "Buffer name", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  oop_listctrl->AppendTextColumn(
    "Buff before (sec)", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  oop_listctrl->AppendTextColumn(
    "Targetless", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  oop_listctrl->AppendTextColumn(
    "Requires invite", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add_spell =
    new wxButton(content_panel, wxID_ANY, "Add spell", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove_spell =
    new wxButton(content_panel, wxID_ANY, "Remove selected", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add_spell, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove_spell, wxSizerFlags().DoubleBorder());
  add_spell->Bind(wxEVT_BUTTON, &out_of_party_tab::on_add_button, this);
  remove_spell->Bind(wxEVT_BUTTON, &out_of_party_tab::on_remove_button, this);

  content_sizer->Add(oop_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_panel->SetSizer(content_sizer);
  my_parent->AddPage(content_panel, "Out of party", false, 12);
}

void
out_of_party_tab::restore_most_recent_config()
{
  oops = most_recent_oops;
  fill_listview();
}

void
out_of_party_tab::save_configs(wxFileConfig& config_to_write)
{
  int qqcounter = 0;
  for (const auto& oop : oops)
  {
    config_to_write.Write(wxString::Format("/oop/%d/active", qqcounter), oop.active);
    config_to_write.Write(wxString::Format("/oop/%d/spellid", qqcounter), oop.spellid);
    config_to_write.Write(wxString::Format("/oop/%d/playername", qqcounter),
                          oop.playername.c_str());
    config_to_write.Write(wxString::Format("/oop/%d/buff_before", qqcounter), oop.buff_before);
    config_to_write.Write(wxString::Format("/oop/%d/is_targetless", qqcounter), oop.is_targetless);
    config_to_write.Write(wxString::Format("/oop/%d/requires_invite", qqcounter),
                          oop.requires_invite);
    ++qqcounter;
  }
}

void
out_of_party_tab::load_configs(wxFileConfig& config_to_load)
{
  oops.clear();

  long index;
  wxString name;
  config_to_load.SetPath("/oop");
  bool go_forward = config_to_load.GetFirstGroup(name, index);
  while (go_forward)
  {
    config_to_load.SetPath(wxString::Format("/oop/%s", name));

    bot_oop_buff entry;
    entry.active = config_to_load.ReadBool("active", false);
    entry.spellid = config_to_load.ReadLong("spellid", 0);
    entry.playername = config_to_load.ReadObject("playername", wxString());
    entry.buff_before = config_to_load.ReadLong("buff_before", 0);
    entry.is_targetless = config_to_load.ReadBool("is_targetless", false);
    entry.requires_invite = config_to_load.ReadBool("requires_invite", false);

    config_to_load.SetPath("/oop");
    go_forward = config_to_load.GetNextGroup(name, index);
  }
  fill_listview();
}

void
out_of_party_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_oops = oops;

  // todo accumulate..
  int total = 0;
  for (const auto& elem : oops)
    if (elem.active)
      ++total;

  bs << to_bot_protocol::out_of_party << total;
  for (const auto& elem : oops)
  {
    if (!elem.active)
      continue;
    bs << elem.spellid << elem.playername << elem.buff_before << elem.is_targetless
       << elem.requires_invite;
  }
}
