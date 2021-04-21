#include <add_craft_dialog.hpp>
#include <craft_data.hpp>
#include <wx/choice.h>

add_craft_dialog::add_craft_dialog(wxWindow* parent, std::function<void(bot_craft&&)> add_handler)
  : wxDialog(parent, wxID_ANY, "Add craft", wxDefaultPosition)
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  count = new wxTextCtrl(this, wxID_ANY);
  recursive_ = new wxCheckBox(this, wxID_ANY, "Recursive?");

  wxArrayString recipe_choices;
  for (const auto& pair : craft_data::get().get_data())
    recipe_choices.Add(pair.first);
  recipe_choices.Sort();

  select_recipe_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, recipe_choices);

  content_sizer->Add(select_recipe_, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Expected count"), 0, wxLEFT);
  content_sizer->Add(count, 0, wxEXPAND);
  content_sizer->Add(recursive_, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button =
    new wxButton(this, wxID_ANY, "Add craft", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_craft_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_craft_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizerAndFit(content_sizer);
}

void
add_craft_dialog::on_ok(wxCommandEvent& event)
{
  bot_craft to_add{};
  long ncraftid, ncount;
  std::string rcp_selection = select_recipe_->GetStringSelection();
  if (craft_data::get().get_data().count(rcp_selection) == 0)
  {
    wxLogMessage("Select recipe");
    Close();
    return;
  }
  ncraftid = craft_data::get().get_data().at(rcp_selection);
  to_add.recipe_id = ncraftid;
  if (!count->GetValue().ToCLong(&ncount))
  {
    wxLogMessage("Unable to convert count field to number");
    Close();
    return;
  }
  to_add.count = ncount;
  to_add.recursive = recursive_->GetValue();
  add_handler_(std::move(to_add));
}

void
add_craft_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
