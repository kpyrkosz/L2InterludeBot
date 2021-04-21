#include <add_item_dialog.hpp>

add_item_dialog::add_item_dialog(wxWindow* parent, std::function<void(bot_item&&)> add_handler)
  : wxDialog(parent, wxID_ANY, "Add item", wxDefaultPosition, wxSize(150, 250))
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  itemid = new wxTextCtrl(this, wxID_ANY);
  cooldown_ms = new wxTextCtrl(this, wxID_ANY);
  condtype = new wxTextCtrl(this, wxID_ANY);
  condval = new wxTextCtrl(this, wxID_ANY);

  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Item id"), 0, wxCENTER);
  content_sizer->Add(itemid, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Cooldown (ms)"), 0, wxCENTER);
  content_sizer->Add(cooldown_ms, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Condtype"), 0, wxCENTER);
  content_sizer->Add(condtype, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Condval"), 0, wxCENTER);
  content_sizer->Add(condval, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button = new wxButton(this, wxID_ANY, "Add item", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_item_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_item_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizer(content_sizer);
}

void
add_item_dialog::on_ok(wxCommandEvent& event)
{
  long num_itemid, cd, ncondtype, ncondval;
  if (!itemid->GetValue().ToCLong(&num_itemid))
  {
    wxLogMessage("Unable to convert itemid field to number");
    Close();
    return;
  }
  if (!cooldown_ms->GetValue().ToCLong(&cd))
  {
    wxLogMessage("Unable to convert cd field to number");
    Close();
    return;
  }
  if (!condtype->GetValue().ToCLong(&ncondtype))
  {
    wxLogMessage("Unable to convert condtype field to number");
    Close();
    return;
  }
  if (ncondtype < 0 || ncondtype > 4)
  {
    wxLogMessage("condtype 0-4");
    Close();
    return;
  }
  if (!condval->GetValue().ToCLong(&ncondval))
  {
    wxLogMessage("Unable to convert condval field to number");
    Close();
    return;
  }
  add_handler_(bot_item{true, num_itemid, cd, class_condition_type(ncondtype), ncondval});
}

void
add_item_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
