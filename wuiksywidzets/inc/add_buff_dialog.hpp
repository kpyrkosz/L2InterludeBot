#include <bot_config.hpp>
#include <wx/wx.h>

class add_buff_dialog : public wxDialog
{
public:
  add_buff_dialog(wxWindow* parent, std::function<void(bot_buff&&)> add_handler);

  std::function<void(bot_buff&&)> add_handler_;

  wxTextCtrl* spellid;
  wxTextCtrl* playername;
  wxTextCtrl* buffbefore;
  wxCheckBox* is_targetless;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
