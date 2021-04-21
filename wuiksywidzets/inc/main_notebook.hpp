#pragma once

#include <base_tab.hpp>
#include <current_character_data.hpp>
#include <set>
#include <wx/notebook.h>
#include <wx/socket.h>
#include <wx/wx.h>

class main_notebook
{
  wxPanel* my_parent_;
  wxBookCtrlBase* character_list_;
  wxNotebook* the_notebook_;
  wxSocketBase* associated_socket_;
  const current_character_data& current_character_data_;

  std::vector<std::unique_ptr<base_tab>> tabs_;

  bool is_active_;
  bounding_box box;

  void on_pause_button(wxCommandEvent& e);
  void on_undo_button(wxCommandEvent& e);
  void on_update_button(wxCommandEvent& e);
  void on_exit_button(wxCommandEvent& e);
  void on_load_button(wxCommandEvent& e);
  void on_save_button(wxCommandEvent& e);

public:
  main_notebook(wxPanel* my_parent,
                wxBookCtrlBase* character_list,
                wxImageList* icon_imagelist,
                wxSocketBase* associated_socket,
                const current_character_data& ccd);

  static std::set<wxSocketBase*> all_sockets; // using it to broadcast requests

  void TEMP_spawn_bounding_box_selection(int tilex, int tiley);
};
