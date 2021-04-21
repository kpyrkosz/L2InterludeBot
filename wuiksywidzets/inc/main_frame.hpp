#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <event_identifiers.hpp>
#include <wx/notebook.h>
#include <wx/socket.h>

class main_frame : public wxFrame
{
public:
  main_frame(const wxString& title, const wxPoint& pos, const wxSize& size);
  ~main_frame();
  void Auth();

private:
  wxTextCtrl* log_window_;
  wxBookCtrlBase* character_list_;
  wxImageList* icon_imagelist_;
  wxSocketServer* acceptor_;

  // GUI initialization helpers and builders
  void create_menu();
  void create_log();
  void create_bottom_statusbar();
  void load_icons();
  void start_listening();

  // event handlers
  void OnHello(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnServerEvent(wxSocketEvent& event);
  wxDECLARE_EVENT_TABLE();
};
