#include <add_manor_dialog.hpp>
#include <manor_data.hpp>
#include <wx/choice.h>

add_manor_dialog::add_manor_dialog(wxWindow* parent, std::function<void(bot_manor&&)> add_handler)
  : wxDialog(parent, wxID_ANY, "Add seed", wxDefaultPosition)
  , add_handler_(add_handler)
{
  wxSizer* content_sizer = new wxBoxSizer(wxVERTICAL);
  expected_count_ = new wxTextCtrl(this, wxID_ANY);

  wxArrayString castle_choices;
  for (const auto& castle : manor_data::get().castle_names())
    castle_choices.Add(castle.c_str());
  castle_choices.Sort();

  select_seed_ = new wxChoice(this, wxID_ANY);

  select_castle_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, castle_choices);

  select_castle_->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event) {
    auto& seed_map = manor_data::get()
                       .get_by_castle(select_castle_->GetStringSelection().ToStdString())
                       .seed_to_id;
    std::vector<wxString> opts;
    for (const auto& seed : seed_map)
      opts.emplace_back(seed.first);
    std::sort(begin(opts), end(opts));
    select_seed_->Set(opts);
    ;
  });

  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Castle"), 0, wxLEFT);
  content_sizer->Add(select_castle_, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Seed"), 0, wxLEFT);
  content_sizer->Add(select_seed_, 0, wxEXPAND);
  content_sizer->Add(new wxStaticText(this, wxID_ANY, "Expected crop count"), 0, wxLEFT);
  content_sizer->Add(expected_count_, 0, wxEXPAND);

  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* ok_button = new wxButton(this, wxID_ANY, "Add", wxDefaultPosition, wxSize(70, 25));
  wxButton* no_button = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxSize(70, 25));
  bottom_buttons_sizer->Add(ok_button);
  bottom_buttons_sizer->Add(no_button);
  ok_button->Bind(wxEVT_BUTTON, &add_manor_dialog::on_ok, this);
  no_button->Bind(wxEVT_BUTTON, &add_manor_dialog::on_cancel, this);

  content_sizer->Add(bottom_buttons_sizer);
  this->SetSizerAndFit(content_sizer);
}

void
add_manor_dialog::on_ok(wxCommandEvent& event)
{
  bot_manor to_add;
  long nexpected;
  if (!expected_count_->GetValue().ToCLong(&nexpected))
  {
    wxLogMessage("Unable to convert expected count field to number");
    Close();
    return;
  }
  to_add.expected_count = nexpected;
  auto castle = select_castle_->GetStringSelection();
  if (castle.empty())
  {
    wxLogMessage("Castle is not selected");
    Close();
    return;
  }
  to_add.castle = castle.c_str();
  auto seed = select_seed_->GetStringSelection();
  if (seed.empty())
  {
    wxLogMessage("Seed is not selected");
    Close();
    return;
  }
  to_add.seed = seed.c_str();
  if (manor_data::get().get_by_castle(to_add.castle).seed_to_id.at(to_add.seed) == 0)
  {
    wxLogMessage(
      "The devil! Manor data (seed) for: %s - %s is corrupted!", to_add.castle, to_add.seed);
    Close();
    return;
  }
  if (manor_data::get().crop_name_to_id(to_add.seed) == 0)
  {
    wxLogMessage("The devil! Manor data (crop) for: %s is corrupted!", to_add.seed);
    Close();
    return;
  }
  add_handler_(std::move(to_add));
}

void
add_manor_dialog::on_cancel(wxCommandEvent& event)
{
  Close();
}
