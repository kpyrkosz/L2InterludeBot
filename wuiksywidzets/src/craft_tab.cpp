#include <add_craft_dialog.hpp>
#include <craft_data.hpp>
#include <craft_tab.hpp>

void
craft_tab::fill_craft_listview()
{
  craft_listctrl->DeleteAllItems();
  for (const auto craft : crafting)
  {
    wxVector<wxVariant> data;
    data.push_back(craft_data::get().name_from_id(craft.recipe_id));
    data.push_back(wxString::Format("%d", craft.count));
    data.push_back(craft.recursive ? "YES" : "NO");
    craft_listctrl->AppendItem(data);
  }
}

void
craft_tab::on_add_craft_button(wxCommandEvent& e)
{
  add_craft_dialog qq(my_parent_,
                      [this](bot_craft&& to_add) { crafting.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_craft_listview();
}

void
craft_tab::on_remove_craft_button(wxCommandEvent& e)
{
  auto selected = craft_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  crafting.erase(crafting.begin() + selected);
  fill_craft_listview();
}

craft_tab::craft_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  craft_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  craft_listctrl->AppendTextColumn(
    "Recipe", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  craft_listctrl->AppendTextColumn(
    "Expected item count", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  craft_listctrl->AppendTextColumn(
    "Is recursive", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add =
    new wxButton(content_panel, wxID_ANY, "Add recipe", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove =
    new wxButton(content_panel, wxID_ANY, "Remove recipe", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove, wxSizerFlags().DoubleBorder());
  add->Bind(wxEVT_BUTTON, &craft_tab::on_add_craft_button, this);
  remove->Bind(wxEVT_BUTTON, &craft_tab::on_remove_craft_button, this);

  content_sizer->Add(craft_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Craft", false, 6);
}

void
craft_tab::restore_most_recent_config()
{
  crafting = most_recent_crafting;
  fill_craft_listview();
}

void
craft_tab::save_configs(wxFileConfig& config_to_write)
{
}

void
craft_tab::load_configs(wxFileConfig& config_to_load)
{
}

void
craft_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_crafting = crafting;

  bs << to_bot_protocol::craft << crafting.size();
  for (const auto& c : crafting)
    bs << c.recipe_id << c.count << c.recursive;
}
