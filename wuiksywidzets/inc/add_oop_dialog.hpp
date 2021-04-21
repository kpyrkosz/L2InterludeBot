#include <bot_config.hpp>
#include <wx/wx.h>

class add_oop_dialog : public wxDialog
{
public:
  add_oop_dialog(wxWindow* parent, std::function<void(bot_oop_buff&&)> add_handler);

  std::function<void(bot_oop_buff&&)> add_handler_;

  wxTextCtrl* spellid;
  wxTextCtrl* playername;
  wxTextCtrl* buffbefore;
  wxCheckBox* is_targetless;
  wxCheckBox* requires_invite;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
