#include <bot_config.hpp>
#include <wx/wx.h>

class add_manor_dialog : public wxDialog
{
  wxChoice* select_seed_;
  wxChoice* select_castle_;
  wxTextCtrl* expected_count_;

  std::function<void(bot_manor&&)> add_handler_;

public:
  add_manor_dialog(wxWindow* parent, std::function<void(bot_manor&&)> add_handler);

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
