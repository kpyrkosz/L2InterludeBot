#include <bot_config.hpp>
#include <current_character_data.hpp>
#include <functional>
#include <wx/wx.h>

class add_spell_dialog : public wxDialog
{
public:
  add_spell_dialog(wxWindow* parent,
                   std::function<void(bot_spell&&)> add_handler,
                   const current_character_data& ccd);

  std::function<void(bot_spell&&)> add_handler_;

  wxTextCtrl* spellid;
  wxTextCtrl* cooldown_ms;
  wxRadioBox* condtype;
  wxTextCtrl* condval;
  wxRadioBox* target;
  wxTextCtrl* nick;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
