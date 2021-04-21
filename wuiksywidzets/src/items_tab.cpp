#include <add_item_dialog.hpp>
#include <item_data.hpp>
#include <items_tab.hpp>

void
items_tab::fill_items_listview()
{
  items_listctrl->DeleteAllItems();
  for (const auto item : items)
  {
    wxVector<wxVariant> data;
    data.push_back(wxVariant(item.active));
    data.push_back(item_data::get().id_to_name(item.itemid));
    data.push_back(wxString::Format("%d", item.cooldown));

    switch (item.condtype)
    {
      case class_condition_type::always:
        data.push_back(wxVariant("ALWAYS"));
        break;
      case class_condition_type::hpgt:
        data.push_back(wxString::Format("HP > %d", item.condval));
        break;
      case class_condition_type::hplt:
        data.push_back(wxString::Format("HP < %d", item.condval));
        break;
      case class_condition_type::managt:
        data.push_back(wxString::Format("MP > %d", item.condval));
        break;
      case class_condition_type::manalt:
        data.push_back(wxString::Format("MP < %d", item.condval));
        break;
      default:
        data.push_back(wxVariant("BAD_ENUM"));
        break;
    }
    items_listctrl->AppendItem(data);
  }
}

void
items_tab::on_add_item_button(wxCommandEvent& e)
{
  add_item_dialog qq(my_parent_,
                     [this](bot_item&& to_add) { items.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_items_listview();
}

void
items_tab::on_remove_item_button(wxCommandEvent& e)
{
  auto selected = items_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  items.erase(items.begin() + selected);
  fill_items_listview();
}

items_tab::items_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  items_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  items_listctrl->AppendToggleColumn(
    "Active", wxDATAVIEW_CELL_ACTIVATABLE, wxCOL_WIDTH_AUTOSIZE, wxALIGN_CENTER);
  items_listctrl->AppendTextColumn(
    "Item", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT); // name + id
  items_listctrl->AppendTextColumn(
    "Cooldown (ms)", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  items_listctrl->AppendTextColumn(
    "Condition", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add_item =
    new wxButton(content_panel, wxID_ANY, "Add item", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove_item =
    new wxButton(content_panel, wxID_ANY, "Remove item", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add_item, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove_item, wxSizerFlags().DoubleBorder());
  add_item->Bind(wxEVT_BUTTON, &items_tab::on_add_item_button, this);
  remove_item->Bind(wxEVT_BUTTON, &items_tab::on_remove_item_button, this);

  // add will spawn subdialog
  content_sizer->Add(items_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Items", false, 3);
}

void
items_tab::restore_most_recent_config()
{
  items = most_recent_items;
  fill_items_listview();
}

void
items_tab::save_configs(wxFileConfig& config_to_write)
{
  int qqcounter = 0;
  for (const auto& item : items)
  {
    config_to_write.Write(wxString::Format("/items/%d/itemid", qqcounter), item.itemid);
    config_to_write.Write(wxString::Format("/items/%d/active", qqcounter), item.active);
    config_to_write.Write(wxString::Format("/items/%d/condtype", qqcounter), (int)item.condtype);
    config_to_write.Write(wxString::Format("/items/%d/condval", qqcounter), item.condval);
    config_to_write.Write(wxString::Format("/items/%d/cooldown", qqcounter), item.cooldown);
    ++qqcounter;
  }
}

void
items_tab::load_configs(wxFileConfig& config_to_load)
{
  items.clear();

  long index;
  wxString name;
  config_to_load.SetPath("/items");
  bool go_forward = config_to_load.GetFirstGroup(name, index);
  while (go_forward)
  {
    config_to_load.SetPath(wxString::Format("/items/%s", name));

    bot_item entry;
    entry.itemid = config_to_load.ReadLong("itemid", 0);
    entry.active = config_to_load.ReadBool("active", false);
    entry.condtype = static_cast<class_condition_type>(config_to_load.ReadLong("condtype", 0));
    entry.condval = config_to_load.ReadLong("condval", 0);
    entry.cooldown = config_to_load.ReadLong("cooldown", 0);
    items.emplace_back(entry);

    config_to_load.SetPath("/items");
    go_forward = config_to_load.GetNextGroup(name, index);
  }
  fill_items_listview();
}

void
items_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_items = items;

  // todo accumulate..
  int total = 0;
  for (const auto& elem : items)
    if (elem.active)
      ++total;

  bs << to_bot_protocol::items << total;
  for (const auto& elem : items)
  {
    if (!elem.active)
      continue;
    bs << elem.itemid << elem.cooldown << elem.condtype << elem.condval;
  }
}
