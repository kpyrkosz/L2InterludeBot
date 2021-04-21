#include <bot_config.hpp>
#include <wx/wx.h>

class add_beast_dialog : public wxDialog
{
public:
  add_beast_dialog(wxWindow* parent, std::function<void(bot_beast&&)> add_handler);

  std::function<void(bot_beast&&)> add_handler_;

  wxTextCtrl* beast_id;
  wxCheckBox* is_lower_priority;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
