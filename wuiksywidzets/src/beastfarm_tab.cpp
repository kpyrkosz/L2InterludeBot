#include <add_beast_dialog.hpp>
#include <beastfarm_data.hpp>
#include <beastfarm_tab.hpp>

void
beastfarm_tab::fill_listview()
{
  listctrl->DeleteAllItems();
  for (const auto beast : beasts)
  {
    auto beast_data = beastfarm_data::get().get_data(beast.id);
    if (!beast_data.is_valid)
      throw std::invalid_argument("Should never get there, most probably you've spoiled the saved "
                                  "beastfarm config file manually");

    wxVector<wxVariant> data;
    data.push_back(wxString::Format("%s (%d)", beast_data.name, beast.id));
    data.push_back(beast.high_priority ? "HIGH" : "LOW");
    if (beast_data.flags == -1)
    {
      data.push_back("x1/2");
      data.push_back("-");
      data.push_back("-");
      data.push_back("-");
    }
    else if (beast_data.flags >= 16)
    {
      data.push_back("x4");
      data.push_back("any");
      data.push_back(beast_data.flags == 17 ? "mage" : "fighter");
      data.push_back("-");
    }
    else
    {
      data.push_back(beast_data.flags & 8 ? "x2" : "x1");
      data.push_back(beast_data.flags & 4 ? "crystal" : "golden");
      data.push_back(beast_data.flags & 1 ? "mage" : "fighter");
      data.push_back(beast_data.flags & 2 ? "yes" : "no");
    }
    listctrl->AppendItem(data);
  }
}

void
beastfarm_tab::on_add_button(wxCommandEvent& e)
{
  add_beast_dialog qq(my_parent_,
                      [this](bot_beast&& to_add) { beasts.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_listview();
}

void
beastfarm_tab::on_remove_button(wxCommandEvent& e)
{
  auto selected = listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  beasts.erase(beasts.begin() + selected);
  fill_listview();
}

beastfarm_tab::beastfarm_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
  , most_recent_configs{}
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  listctrl->AppendTextColumn("Monster", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  listctrl->AppendTextColumn(
    "Priority", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT); // name + id
  listctrl->AppendTextColumn(
    "HP multipler", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  listctrl->AppendTextColumn("Spice", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  listctrl->AppendTextColumn("Type", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  listctrl->AppendTextColumn(
    "Alternative", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add_buff =
    new wxButton(content_panel, wxID_ANY, "Add beast", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove_buff =
    new wxButton(content_panel, wxID_ANY, "Remove beast", wxDefaultPosition, wxSize(120, 25));

  bottom_buttons_sizer->Add(add_buff, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove_buff, wxSizerFlags().DoubleBorder());
  add_buff->Bind(wxEVT_BUTTON, &beastfarm_tab::on_add_button, this);
  remove_buff->Bind(wxEVT_BUTTON, &beastfarm_tab::on_remove_button, this);

  prioritize_buffalo = new wxCheckBox(content_panel, wxID_ANY, "Prioritize buffalo");
  prioritize_kooka = new wxCheckBox(content_panel, wxID_ANY, "Prioritize kooka");
  prioritize_cougar = new wxCheckBox(content_panel, wxID_ANY, "Prioritize cougar");

  wxSizer* prioritization_sizer = new wxBoxSizer(wxHORIZONTAL);
  prioritization_sizer->Add(prioritize_buffalo, 1, wxEXPAND);
  prioritization_sizer->Add(prioritize_kooka, 1, wxEXPAND);
  prioritization_sizer->Add(prioritize_cougar, 1, wxEXPAND);

  content_sizer->Add(listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_sizer->Add(prioritization_sizer, 0, wxEXPAND);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Beastfarm", false, 15);
}

void
beastfarm_tab::restore_most_recent_config()
{
  prioritize_buffalo->SetValue(most_recent_configs.prefer_buffalo);
  prioritize_kooka->SetValue(most_recent_configs.prefer_kooka);
  prioritize_cougar->SetValue(most_recent_configs.prefer_cougar);
  beasts = most_recent_beasts;
  fill_listview();
}

void
beastfarm_tab::save_configs(wxFileConfig& config_to_write)
{
  /*config_to_write.Write("party_inviter", party_inviter_textctrl->GetValue());
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
  }*/
}

void
beastfarm_tab::load_configs(wxFileConfig& config_to_load)
{
  /*buffs.clear();

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
  fill_party_listview();*/
}

void
beastfarm_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_configs.prefer_buffalo = prioritize_buffalo->GetValue();
  most_recent_configs.prefer_kooka = prioritize_kooka->GetValue();
  most_recent_configs.prefer_cougar = prioritize_cougar->GetValue();
  most_recent_beasts = beasts;

  bs << to_bot_protocol::beastfarm << most_recent_configs.prefer_buffalo
     << most_recent_configs.prefer_kooka << most_recent_configs.prefer_cougar << beasts.size();
  for (const auto& elem : beasts)
    bs << elem.id << elem.high_priority;
}
