#include <add_spell_dialog.hpp>
#include <wx/choice.h>

add_spell_dialog::add_spell_dialog(wxWindow* parent,
                                   std::function<void(bot_spell&&)> add_handler,
                                   const current_character_data& ccd)
  : wxDialog(parent, wxID_ANY, "Add spell", wxDefaultPosition)
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);

  spellid = new wxTextCtrl(this, wxID_ANY);
  cooldown_ms = new wxTextCtrl(this, wxID_ANY);
  condval = new wxTextCtrl(this, wxID_ANY);
  nick = new wxTextCtrl(this, wxID_ANY);

  wxString condtype_opts[] = {"Always", "HP GT", "HP LT", "MP GT", "MP LT"};
  condtype = new wxRadioBox(this,
                            wxID_ANY,
                            "Condtype",
                            wxDefaultPosition,
                            wxDefaultSize,
                            sizeof(condtype_opts) / sizeof(condtype_opts[0]),
                            condtype_opts);

  wxString target_opts[] = {"Monster", "Self", "Specify nickname"};
  target = new wxRadioBox(this,
                          wxID_ANY,
                          "Target",
                          wxDefaultPosition,
                          wxDefaultSize,
                          sizeof(target_opts) / sizeof(target_opts[0]),
                          target_opts);

  wxArrayString spell_choices;
  for (const auto& spell : ccd.spells)
    spell_choices.Add(wxString::Format("%s (%d)", spell.second.c_str(), spell.first));
  spell_choices.Sort();
  wxChoice* select_spell =
    new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, spell_choices);
  //.*\((\d*)\)$
  select_spell->Bind(wxEVT_CHOICE, [this, select_spell](wxCommandEvent& event) {
    spellid->SetValue(select_spell->GetStringSelection());
  });

  content_sizer->Add(select_spell, 0, wxLEFT);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Spell id"), 0, wxLEFT);
  content_sizer->Add(spellid, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Cooldown (ms)"), 0, wxLEFT);
  content_sizer->Add(cooldown_ms, 0, wxEXPAND);
  content_sizer->Add(condtype, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Condval"), 0, wxLEFT);
  content_sizer->Add(condval, 0, wxEXPAND);
  content_sizer->Add(target, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Nick"), 0, wxLEFT);
  content_sizer->Add(nick, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button =
    new wxButton(this, wxID_ANY, "Add spell", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_spell_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_spell_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizerAndFit(content_sizer);
}

void
add_spell_dialog::on_ok(wxCommandEvent& event)
{
  bot_spell to_add{true};
  long nspellid, ncooldown_ms, ncondval;
  if (!spellid->GetValue().ToCLong(&nspellid))
  {
    wxLogMessage("Unable to convert spellid field to number");
    Close();
    return;
  }
  to_add.spellid = nspellid;
  if (!cooldown_ms->GetValue().ToCLong(&ncooldown_ms))
  {
    wxLogMessage("Unable to convert cooldown_ms field to number");
    Close();
    return;
  }
  to_add.cooldown = ncooldown_ms;
  if (!condval->GetValue().ToCLong(&ncondval) &&
      condtype->GetSelection() != 0) // always doesnt require it
  {
    wxLogMessage("Unable to convert condval field to number");
    Close();
    return;
  }
  to_add.condval = ncondval;
  switch (condtype->GetSelection())
  {
    case 0:
      to_add.condtype = class_condition_type::always;
      break;
    case 1:
      to_add.condtype = class_condition_type::hpgt;
      break;
    case 2:
      to_add.condtype = class_condition_type::hplt;
      break;
    case 3:
      to_add.condtype = class_condition_type::managt;
      break;
    case 4:
      to_add.condtype = class_condition_type::manalt;
      break;
  }
  switch (target->GetSelection())
  {
    case 0:
      to_add.target = spell_target::monster;
      break;
    case 1:
      to_add.target = spell_target::self;
      break;
    case 2:
      to_add.target = spell_target::nick;
      if (nick->GetValue().empty())
      {
        wxLogMessage("Input nickname when selecting nick target!");
        Close();
        return;
      }
      to_add.nick_optional = nick->GetValue().c_str();
      break;
  }
  add_handler_(std::move(to_add));
}

void
add_spell_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
