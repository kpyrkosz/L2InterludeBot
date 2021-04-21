#include <add_spell_dialog.hpp>
#include <skill_data.hpp>
#include <spells_tab.hpp>

void
spells_tab::fill_spells_listview()
{
  spells_listctrl->DeleteAllItems();
  for (const auto spell : spells)
  {
    wxVector<wxVariant> data;
    data.push_back(wxVariant(spell.active));

    data.push_back(
      wxString::Format("%s (%d)", skill_data::get().id_to_name(spell.spellid), spell.spellid));

    data.push_back(wxString::Format("%d", spell.cooldown));
    switch (spell.condtype)
    {
      case class_condition_type::always:
        data.push_back(wxVariant("ALWAYS"));
        break;
      case class_condition_type::hpgt:
        data.push_back(wxString::Format("HP > %d", spell.condval));
        break;
      case class_condition_type::hplt:
        data.push_back(wxString::Format("HP < %d", spell.condval));
        break;
      case class_condition_type::managt:
        data.push_back(wxString::Format("MP > %d", spell.condval));
        break;
      case class_condition_type::manalt:
        data.push_back(wxString::Format("MP < %d", spell.condval));
        break;
      default:
        data.push_back(wxVariant("BAD_ENUM"));
        break;
    }
    switch (spell.target)
    {
      case spell_target::monster:
        data.push_back(wxVariant("MONSTER"));
        break;
      case spell_target::self:
        data.push_back(wxVariant("SELF"));
        break;
      case spell_target::nick:
        data.push_back(wxString::Format("Nick: '%s'", spell.nick_optional.c_str()));
        break;
      default:
        data.push_back(wxVariant("BAD_ENUM"));
        break;
    }
    spells_listctrl->AppendItem(data);
  }
}

void
spells_tab::on_add_spell_button(wxCommandEvent& e)
{
  add_spell_dialog qq(
    my_parent_, [this](bot_spell&& to_add) { spells.emplace_back(std::move(to_add)); }, ccd_);
  qq.ShowModal();
  fill_spells_listview();
}

void
spells_tab::on_remove_spell_button(wxCommandEvent& e)
{
  auto selected = spells_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  spells.erase(spells.begin() + selected);
  fill_spells_listview();
}

spells_tab::spells_tab(wxBookCtrlBase* my_parent, const current_character_data& ccd)
  : my_parent_(my_parent)
  , ccd_(ccd)
{
  auto* content_panel = new wxPanel(my_parent, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  spells_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);
  // wxCOL_WIDTH_AUTOSIZE

  spells_listctrl->AppendToggleColumn(
    "Active", wxDATAVIEW_CELL_ACTIVATABLE, wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER);
  spells_listctrl->AppendTextColumn(
    "Spell", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT); // name + id
  spells_listctrl->AppendTextColumn(
    "Cooldown (ms)", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  spells_listctrl->AppendTextColumn(
    "Condition", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  spells_listctrl->AppendTextColumn(
    "Target", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add_spell =
    new wxButton(content_panel, wxID_ANY, "Add spell", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove_spell =
    new wxButton(content_panel, wxID_ANY, "Remove selected", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add_spell, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove_spell, wxSizerFlags().DoubleBorder());
  add_spell->Bind(wxEVT_BUTTON, &spells_tab::on_add_spell_button, this);
  remove_spell->Bind(wxEVT_BUTTON, &spells_tab::on_remove_spell_button, this);
  // add will spawn subdialog
  content_sizer->Add(spells_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_panel->SetSizer(content_sizer);
  my_parent->AddPage(content_panel, "Spells", false, 2);
}

void
spells_tab::restore_most_recent_config()
{
  spells = most_recent_spells;
  fill_spells_listview();
}

void
spells_tab::save_configs(wxFileConfig& config_to_write)
{
  int qqcounter = 0;
  for (const auto& spell : spells)
  {
    config_to_write.Write(wxString::Format("/spells/%d/active", qqcounter), spell.active);
    config_to_write.Write(wxString::Format("/spells/%d/spellid", qqcounter), spell.spellid);
    config_to_write.Write(wxString::Format("/spells/%d/condtype", qqcounter), (int)spell.condtype);
    config_to_write.Write(wxString::Format("/spells/%d/condval", qqcounter), spell.condval);
    config_to_write.Write(wxString::Format("/spells/%d/cooldown", qqcounter), spell.cooldown);
    config_to_write.Write(wxString::Format("/spells/%d/nick_optional", qqcounter),
                          spell.nick_optional.c_str());
    config_to_write.Write(wxString::Format("/spells/%d/target", qqcounter), (int)spell.target);
    ++qqcounter;
  }
}

void
spells_tab::load_configs(wxFileConfig& config_to_load)
{
  spells.clear();

  long index;
  wxString name;
  config_to_load.SetPath("/spells");
  bool go_forward = config_to_load.GetFirstGroup(name, index);
  while (go_forward)
  {
    config_to_load.SetPath(wxString::Format("/spells/%s", name));

    bot_spell entry;
    entry.spellid = config_to_load.ReadLong("spellid", 0);
    entry.active = config_to_load.ReadBool("active", false);
    entry.condtype = static_cast<class_condition_type>(config_to_load.ReadLong("condtype", 0));
    entry.condval = config_to_load.ReadLong("condval", 0);
    entry.cooldown = config_to_load.ReadLong("cooldown", 0);
    entry.target = static_cast<spell_target>(config_to_load.ReadLong("target", 0));
    entry.nick_optional = config_to_load.ReadObject("nick_optional", wxString());
    spells.emplace_back(entry);

    config_to_load.SetPath("/spells");
    go_forward = config_to_load.GetNextGroup(name, index);
  }
  fill_spells_listview();
}

void
spells_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_spells = spells;

  // todo accumulate..
  int total = 0;
  for (const auto& elem : spells)
    if (elem.active)
      ++total;

  bs << to_bot_protocol::spells << total;
  for (const auto& elem : spells)
  {
    if (!elem.active)
      continue;
    bs << elem.spellid << elem.cooldown << elem.condtype << elem.condval << elem.target;
    if (elem.target == spell_target::nick)
      bs << elem.nick_optional;
  }
  auto total_size = bs.get_current_size();
}
