#include <main_frame.hpp>

class main_app : public wxApp
{
public:
  virtual bool OnInit();
};

wxIMPLEMENT_APP(main_app);

bool
main_app::OnInit()
{
  main_frame* frame =
    new main_frame("Six weekends interlude bot", wxPoint(250, 250), wxSize(900, 600));
  frame->Show(true);
  frame->Auth();
  return true;
}
