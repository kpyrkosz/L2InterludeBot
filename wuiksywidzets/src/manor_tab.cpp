#include <add_manor_dialog.hpp>
#include <manor_data.hpp>
#include <manor_tab.hpp>

void
manor_tab::fill_manor_listview()
{
  manor_listctrl->DeleteAllItems();
  for (const auto seed : manor_seeds)
  {
    wxVector<wxVariant> data;
    data.push_back(wxVariant(seed.castle));
    data.push_back(wxVariant(seed.seed));
    data.push_back(wxString::Format("%d", seed.expected_count));
    manor_listctrl->AppendItem(data);
  }
}

void
manor_tab::on_add_manor_button(wxCommandEvent& e)
{
  add_manor_dialog qq(my_parent_,
                      [this](bot_manor&& to_add) { manor_seeds.emplace_back(std::move(to_add)); });
  qq.ShowModal();
  fill_manor_listview();
}

void
manor_tab::on_remove_manor_button(wxCommandEvent& e)
{
  auto selected = manor_listctrl->GetSelectedRow();
  if (selected == wxNOT_FOUND)
    return;
  manor_seeds.erase(manor_seeds.begin() + selected);
  fill_manor_listview();
}

manor_tab::manor_tab(wxBookCtrlBase* my_parent)
  : my_parent_(my_parent)
{
  auto* content_panel = new wxPanel(my_parent_, wxID_ANY);
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  manor_listctrl = new wxDataViewListCtrl(content_panel, wxID_ANY);

  manor_listctrl->AppendTextColumn(
    "Territory", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  manor_listctrl->AppendTextColumn(
    "Seed name", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  manor_listctrl->AppendTextColumn(
    "Expected crop count", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT);
  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* add_seed =
    new wxButton(content_panel, wxID_ANY, "Add seed", wxDefaultPosition, wxSize(120, 25));
  wxButton* remove_seed =
    new wxButton(content_panel, wxID_ANY, "Remove seed", wxDefaultPosition, wxSize(120, 25));
  bottom_buttons_sizer->Add(add_seed, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(remove_seed, wxSizerFlags().DoubleBorder());
  add_seed->Bind(wxEVT_BUTTON, &manor_tab::on_add_manor_button, this);
  remove_seed->Bind(wxEVT_BUTTON, &manor_tab::on_remove_manor_button, this);

  content_sizer->Add(manor_listctrl, 1, wxEXPAND);
  content_sizer->Add(bottom_buttons_sizer);
  content_panel->SetSizer(content_sizer);
  my_parent_->AddPage(content_panel, "Manor", false, 5);
}

void
manor_tab::restore_most_recent_config()
{
  manor_seeds = most_recent_manor_seeds;
  fill_manor_listview();
}

void
manor_tab::save_configs(wxFileConfig& config_to_write)
{
}

void
manor_tab::load_configs(wxFileConfig& config_to_load)
{
}

void
manor_tab::update_and_serialize(binary_serializer& bs)
{
  most_recent_manor_seeds = manor_seeds;

  bs << to_bot_protocol::manor << manor_seeds.size();
  for (const auto& s : manor_seeds)
    bs << manor_data::get().get_by_castle(s.castle).seed_to_id.at(s.seed) << s.expected_count
       << manor_data::get().crop_name_to_id(s.seed);
}
