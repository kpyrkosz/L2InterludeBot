#include <add_buff_dialog.hpp>
#include <party_tab.hpp>
#include <skill_data.hpp>

void
party_tab::fill_party_listview()
{
  party_listctrl->DeleteAllItems();
  for (const auto buff : buffs)
  {
    wxVector<wxVariant> data;
    data.push_back(wxVariant(buff.active));
    data.push_back(skill_data::get().id_to_name(buff.spellid));
    data.push_back(wxString(buff.playername.c_str()));
    data.push_back(wxString::Format("%d", buff.buff_before_second));
    data.push_back(buff.is_targetless ? "YES" : "NO");
    party_listctrl->AppendItem(data);
  }
}

void
party_tab::on_add_buff_button(wxCommandEvent& e)
{
  add_buff_dialog qq(my_parent_,
                     [this](bot_buff&& to_add) { buffs.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_party_listview();
}

void
party_tab::on_remove_buff_button(wxCommandEvent& e)
{
  auto selected = party_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  buffs.erase(buffs.begin() + selected);
  fill_party_listview();
}

party_tab::party_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
  , most_recent_party_configs{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  accept_pt = new wxCheckBox(content_panel, wxID_ANY, "Auto accept party");
  party_inviter_textctrl = new wxTextCtrl(content_panel, wxID_ANY);

  party_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  party_listctrl->AppendToggleColumn(
    "Active", wxDATAVIEW_CELL_ACTIVATABLE, wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER);
  party_listctrl->AppendTextColumn(
    "Buff", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT); // name + id
  party_listctrl->AppendTextColumn(
    "Player name", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  party_listctrl->AppendTextColumn(
    "Rebuff before end (sec)", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  party_listctrl->AppendTextColumn(
    "Is targetless", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add_buff =
    new wxButton(content_panel, wxID_ANY, "Add buff", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove_buff =
    new wxButton(content_panel, wxID_ANY, "Remove buff", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add_buff, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove_buff, wxSizerFlags().DoubleBorder());
  add_buff->Bind(wxEVT_BUTTON, &party_tab::on_add_buff_button, this);
  remove_buff->Bind(wxEVT_BUTTON, &party_tab::on_remove_buff_button, this);

  content_sizer->Add(party_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_sizer->Add(accept_pt);
  content_sizer->Add(new wxStaticText(content_panel, wxID_ANY, "Player name"));
  content_sizer->Add(party_inviter_textctrl);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Party", false, 4);
}

void
party_tab::restore_most_recent_config()
{
  party_inviter_textctrl->SetValue(most_recent_party_configs.party_inviter);
  accept_pt->SetValue(most_recent_party_configs.accept_party);
  buffs = most_recent_buffs;
  fill_party_listview();
}

void
party_tab::save_configs(wxFileConfig& config_to_write)
{
  config_to_write.Write("party_inviter", party_inviter_textctrl->GetValue());
  config_to_write.Write("acceptparty", accept_pt->IsChecked());
  int qqcounter = 0;
  for (const auto& buff : buffs)
  {
    config_to_write.Write(wxString::Format("/buffs/%d/spellid", qqcounter), buff.spellid);
    config_to_write.Write(wxString::Format("/buffs/%d/active", qqcounter), buff.active);
    config_to_write.Write(wxString::Format("/buffs/%d/buff_before_ms", qqcounter),
                          buff.buff_before_second);
    config_to_write.Write(wxString::Format("/buffs/%d/playername", qqcounter),
                          buff.playername.c_str());
    config_to_write.Write(wxString::Format("/buffs/%d/is_targetless", qqcounter),
                          buff.is_targetless);
    ++qqcounter;
  }
}

void
party_tab::load_configs(wxFileConfig& config_to_load)
{
  buffs.clear();

  accept_pt->SetValue(config_to_load.ReadBool("acceptparty", false));
  party_inviter_textctrl->SetValue(config_to_load.ReadObject("party_inviter", wxString()));

  long index;
  wxString name;
  config_to_load.SetPath("/buffs");
  bool go_forward = config_to_load.GetFirstGroup(name, index);
  while (go_forward)
  {
    config_to_load.SetPath(wxString::Format("/buffs/%s", name));

    bot_buff entry;
    entry.spellid = config_to_load.ReadLong("spellid", 0);
    entry.active = config_to_load.ReadBool("active", false);
    entry.buff_before_second = config_to_load.ReadLong("buff_before_ms", 0);
    entry.playername = config_to_load.ReadObject("playername", wxString());
    entry.is_targetless = config_to_load.ReadBool("is_targetless", false);
    buffs.emplace_back(entry);

    config_to_load.SetPath("/buffs");
    go_forward = config_to_load.GetNextGroup(name, index);
  }
  fill_party_listview();
}

void
party_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_party_configs.party_inviter = party_inviter_textctrl->GetValue();
  most_recent_party_configs.accept_party = accept_pt->GetValue();
  most_recent_buffs = buffs;

  // todo accumulate..
  int total = 0;
  for (const auto& elem : buffs)
    if (elem.active)
      ++total;

  bs << to_bot_protocol::buffs << most_recent_party_configs.party_inviter
     << most_recent_party_configs.accept_party << total;
  for (const auto& elem : buffs)
  {
    if (!elem.active)
      continue;
    bs << elem.spellid << elem.playername << elem.buff_before_second << elem.is_targetless;
  }
}
