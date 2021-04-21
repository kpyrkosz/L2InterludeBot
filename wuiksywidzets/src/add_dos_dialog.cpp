#include <add_dos_dialog.hpp>
#include <wx/choice.h>

add_dos_dialog::add_dos_dialog(wxWindow* parent, std::function<void(bot_rule&&)> add_handler)
  : wxDialog(parent, wxID_ANY, "Add spell", wxDefaultPosition)
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  wxArrayString what_choices;
  for (const auto& i : {"Target", "Attack", "Manor", "Spoil"})
    what_choices.Add(i);
  what_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, what_choices);
  content_sizer->Add(what_, 0, wxLEFT);

  wxArrayString mode_choices;
  for (const auto& i : {"Include", "Ignore"})
    mode_choices.Add(i);
  mode_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, mode_choices);
  content_sizer->Add(mode_, 0, wxLEFT);

  who_id_ = new wxTextCtrl(this, wxID_ANY);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Monster ID"), 0, wxLEFT);
  content_sizer->Add(who_id_, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button = new wxButton(this, wxID_ANY, "Add rule", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_dos_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_dos_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizerAndFit(content_sizer);
}

void
add_dos_dialog::on_ok(wxCommandEvent& event)
{
  bot_rule to_add;
  long nmobid;
  if (!who_id_->GetValue().ToCLong(&nmobid))
  {
    wxLogMessage("Unable to convert mob id field to number");
    Close();
    return;
  }
  to_add.mob_id = nmobid;
  switch (what_->GetSelection())
  {
    case 0:
      to_add.brw = bot_rule_what::target;
      break;
    case 1:
      to_add.brw = bot_rule_what::attack;
      break;
    case 2:
      to_add.brw = bot_rule_what::manor;
      break;
    case 3:
      to_add.brw = bot_rule_what::spoil;
      break;
    default:
      wxLogMessage("What not selected");
      Close();
      return;
  }
  switch (mode_->GetSelection())
  {
    case 0:
      to_add.is_include = true;
      break;
    case 1:
      to_add.is_include = false;
      break;
    default:
      wxLogMessage("Mode not selected");
      Close();
      return;
  }
  add_handler_(std::move(to_add));
}

void
add_dos_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
