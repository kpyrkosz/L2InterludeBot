#include <bot_config.hpp>
#include <wx/wx.h>

class add_dos_dialog : public wxDialog
{
public:
  add_dos_dialog(wxWindow* parent, std::function<void(bot_rule&&)> add_handler);

  std::function<void(bot_rule&&)> add_handler_;
  wxTextCtrl* who_id_;
  wxChoice* mode_;
  wxChoice* what_;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
