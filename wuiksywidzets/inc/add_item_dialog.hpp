#include <bot_config.hpp>
#include <wx/wx.h>

class add_item_dialog : public wxDialog
{
public:
  add_item_dialog(wxWindow* parent, std::function<void(bot_item&&)> add_handler);

  std::function<void(bot_item&&)> add_handler_;

  wxTextCtrl* itemid;
  wxTextCtrl* cooldown_ms;
  wxTextCtrl* condtype;
  wxTextCtrl* condval;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
