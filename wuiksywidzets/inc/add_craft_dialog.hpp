#include <bot_config.hpp>
#include <craft_data.hpp>
#include <wx/wx.h>

class add_craft_dialog : public wxDialog
{
public:
  add_craft_dialog(wxWindow* parent, std::function<void(bot_craft&&)> add_handler);

  std::function<void(bot_craft&&)> add_handler_;

  wxChoice* select_recipe_;
  wxCheckBox* recursive_;
  wxTextCtrl* count;

private:
  void on_ok(wxCommandEvent& event);
  void on_cancel(wxCommandEvent& event);
};
