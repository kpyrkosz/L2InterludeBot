#pragma once

#include <bot_config.hpp>
#include <main_notebook.hpp>

class user_panel : public wxPanel
{
  wxSocketBase* associated_socket_;
  wxBookCtrlBase* my_parent_;
  main_notebook main_notebook_;

  std::string incoming_packet;

  current_character_data current_character_data_;

  void start_listening();
  void OnSocketEvent(wxSocketEvent& event);
  void on_socket_event(wxSocketEvent& event);
  void check_packet_received();

public:
  user_panel(wxSocketBase* associated_socket,
             wxBookCtrlBase* my_parent,
             wxImageList* icon_imagelist);
};
