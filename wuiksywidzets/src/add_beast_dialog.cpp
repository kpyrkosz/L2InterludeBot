#include <add_beast_dialog.hpp>
#include <beastfarm_data.hpp>

add_beast_dialog::add_beast_dialog(wxWindow* parent, std::function<void(bot_beast&&)> add_handler)
  : wxDialog(parent, wxID_ANY, "Add beast", wxDefaultPosition, wxSize(150, 250))
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);
  beast_id = new wxTextCtrl(this, wxID_ANY);
  is_lower_priority = new wxCheckBox(this, wxID_ANY, "Is lower priority?");

  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Beast id"), 0, wxCENTER);
  content_sizer->Add(beast_id, 0, wxEXPAND);
  content_sizer->Add(is_lower_priority, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button = new wxButton(this, wxID_ANY, "Add", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_beast_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_beast_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizer(content_sizer);
}

void
add_beast_dialog::on_ok(wxCommandEvent& event)
{
  long num_beast_id;
  if (!beast_id->GetValue().ToCLong(&num_beast_id))
  {
    wxLogMessage("Unable to convert spellid field to number");
    Close();
    return;
  }
  if (!beastfarm_data::get().get_data(num_beast_id).is_valid)
  {
    wxLogMessage("Given id is not a feedable beast id");
    Close();
    return;
  }
  add_handler_(bot_beast{num_beast_id, is_lower_priority->GetValue()});
}

void
add_beast_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
