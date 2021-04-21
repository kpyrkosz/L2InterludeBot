#include <add_buff_dialog.hpp>

add_buff_dialog::add_buff_dialog(wxWindow* parent, std::function<void(bot_buff&&)> add_handler)
  : wxDialog(parent, wxID_ANY, "Add buff", wxDefaultPosition, wxSize(150, 250))
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);
  spellid = new wxTextCtrl(this, wxID_ANY);
  playername = new wxTextCtrl(this, wxID_ANY);
  buffbefore = new wxTextCtrl(this, wxID_ANY);
  is_targetless = new wxCheckBox(this, wxID_ANY, "Is targetless?");

  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Spell id"), 0, wxCENTER);
  content_sizer->Add(spellid, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Player name"), 0, wxCENTER);
  content_sizer->Add(playername, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Buff before (sec)"), 0, wxCENTER);
  content_sizer->Add(buffbefore, 0, wxEXPAND);
  content_sizer->Add(is_targetless, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button = new wxButton(this, wxID_ANY, "Add buff", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_buff_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_buff_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizer(content_sizer);
}

void
add_buff_dialog::on_ok(wxCommandEvent& event)
{
  long num_spellid, buffbefre;
  if (!spellid->GetValue().ToCLong(&num_spellid))
  {
    wxLogMessage("Unable to convert spellid field to number");
    Close();
    return;
  }
  if (playername->GetValue().empty())
  {
    wxLogMessage("Player name is empty");
    Close();
    return;
  }
  if (!buffbefore->GetValue().ToCLong(&buffbefre))
  {
    wxLogMessage("Unable to convert buffbefore field to number");
    Close();
    return;
  }
  add_handler_(bot_buff{
    true, num_spellid, playername->GetValue().c_str(), buffbefre, is_targetless->GetValue()});
}

void
add_buff_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
