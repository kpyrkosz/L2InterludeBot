#include <aes_boy.hpp>
#include <alarm_tab.hpp>
#include <beastfarm_tab.hpp>
#include <binary_serializer.hpp>
#include <bounding_box_selection_panel.hpp>
#include <craft_tab.hpp>
#include <dos_and_donts_tab.hpp>
#include <event_identifiers.hpp>
#include <farming_tab.hpp>
#include <follow_tab.hpp>
#include <items_tab.hpp>
#include <main_notebook.hpp>
#include <manor_data.hpp>
#include <manor_tab.hpp>
#include <map_tab.hpp>
#include <out_of_party_tab.hpp>
#include <party_tab.hpp>
#include <pick_tab.hpp>
#include <relax_tab.hpp>
#include <special_tricks_tab.hpp>
#include <spells_tab.hpp>
#include <targeting_tab.hpp>
#include <wx/wfstream.h>

std::set<wxSocketBase*> main_notebook::all_sockets;

void
main_notebook::on_pause_button(wxCommandEvent& e)
{
  wxLogMessage(is_active_ ? "Requesting pause" : "Requesting activation");
  is_active_ = !is_active_;
  std::array<char, 2048> to_send;
  binary_serializer bs(to_send);
  bs << to_bot_protocol::pause_bot << is_active_;
  auto total_size = bs.get_current_size();
  associated_socket_->Write((char*)&total_size, sizeof(total_size));
  aes_boy::inplace_encrypt_to_dll(to_send.data(), total_size);
  associated_socket_->Write(to_send.data(), total_size);
}

void
main_notebook::on_undo_button(wxCommandEvent& e)
{
  wxLogMessage("Restoring most recent config!");
  for (const auto& tab : tabs_)
    tab->restore_most_recent_config();
}

void
main_notebook::on_update_button(wxCommandEvent& e)
{
  wxLogMessage("Updating config!");
  // box and antistuck
  {
    std::array<char, 2048> to_send;
    binary_serializer bs(to_send);

    bs << to_bot_protocol::bounding_box << (int)box.box_points.size();
    for (const auto& point : box.box_points)
      bs << point.first << point.second;

    bs << (int)box.antistuck_points.size();
    for (const auto& point : box.antistuck_points)
      bs << point.first << point.second;

    auto total_size = bs.get_current_size();
    associated_socket_->Write((char*)&total_size, sizeof(total_size));
    aes_boy::inplace_encrypt_to_dll(to_send.data(), total_size);
    associated_socket_->Write(to_send.data(), total_size);
  }

  // push updates to bot
  for (const auto& tab : tabs_)
  {
    std::array<char, 2048> to_send;
    binary_serializer bs(to_send);
    tab->update_and_serialize(bs);
    auto total_size = bs.get_current_size();
    associated_socket_->Write((char*)&total_size, sizeof(total_size));
    aes_boy::inplace_encrypt_to_dll(to_send.data(), total_size);
    associated_socket_->Write(to_send.data(), total_size);
  }
}

void
main_notebook::on_exit_button(wxCommandEvent& e)
{
  for (int i = 0; i < character_list_->GetPageCount(); ++i)
  {
    if (character_list_->GetPage(i) == my_parent_)
    {
      wxLogMessage(wxString::Format("JEST!!! %d!", i));
      associated_socket_->Close();
      all_sockets.erase(associated_socket_);
      character_list_->DeletePage(i);
      return;
    }
    wxLogMessage(character_list_->GetPageText(i));
  }
}

void
main_notebook::on_load_button(wxCommandEvent& e)
{
  wxFileDialog dialog(my_parent_,
                      "Load config from a file",
                      wxEmptyString,
                      "config.sbc",
                      "Szakabot config (*.sbc)|*.sbc",
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  dialog.SetFilterIndex(1);
  if (dialog.ShowModal() != wxID_OK)
    return;

  wxLogMessage("%s, filter %d", dialog.GetPath(), dialog.GetFilterIndex());
  wxFileInputStream filein(dialog.GetPath());
  if (!filein.IsOk())
  {
    wxLogMessage("%s UNABLE TO CREATE INPUT FILE", dialog.GetPath());
    return;
  }

  wxFileConfig config_to_load(filein);

  for (const auto& tab : tabs_)
  {
    config_to_load.SetPath("");
    tab->load_configs(config_to_load);
  }
}

void
main_notebook::on_save_button(wxCommandEvent& e)
{
  wxFileDialog dialog(my_parent_,
                      "Export config to a file",
                      wxEmptyString,
                      "config.sbc",
                      "Szakabot config (*.sbc)|*.sbc",
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  dialog.SetFilterIndex(1);

  if (dialog.ShowModal() != wxID_OK)
    return;

  wxLogMessage("%s, filter %d", dialog.GetPath(), dialog.GetFilterIndex());
  wxFileOutputStream fileout(dialog.GetPath());
  if (!fileout.IsOk())
  {
    wxLogMessage("%s UNABLE TO CREATE OUTPUT FILE", dialog.GetPath());
    return;
  }
  wxFileConfig config_to_write;
  for (const auto& tab : tabs_)
    tab->save_configs(config_to_write);

  config_to_write.Save(fileout);
}

main_notebook::main_notebook(wxPanel* my_parent,
                             wxBookCtrlBase* character_list,
                             wxImageList* icon_imagelist,
                             wxSocketBase* associated_socket,
                             const current_character_data& ccd)
  : my_parent_(my_parent)
  , character_list_(character_list)
  , the_notebook_(
      new wxNotebook(my_parent_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE))
  , associated_socket_(associated_socket)
  , current_character_data_(ccd)
  , is_active_(false)
  , box{}
{
  the_notebook_->SetImageList(icon_imagelist);
  tabs_.emplace_back(std::make_unique<targeting_tab>(the_notebook_, associated_socket));
  tabs_.emplace_back(std::make_unique<farming_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<spells_tab>(the_notebook_, current_character_data_));
  tabs_.emplace_back(std::make_unique<items_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<party_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<manor_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<craft_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<dos_and_donts_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<alarm_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<follow_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<pick_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<relax_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<out_of_party_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<map_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<special_tricks_tab>(the_notebook_));
  tabs_.emplace_back(std::make_unique<beastfarm_tab>(the_notebook_));

  wxSizer* vertical_sizer = new wxBoxSizer(wxVERTICAL);
  wxSizer* bottom_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* b_update =
    new wxButton(my_parent_, wxID_ANY, "Update config", wxDefaultPosition, wxSize(90, 30));
  wxButton* b_undo =
    new wxButton(my_parent_, wxID_ANY, "Undo modifications", wxDefaultPosition, wxSize(90, 30));
  wxButton* b_pause =
    new wxButton(my_parent_, wxID_ANY, "Pause bot", wxDefaultPosition, wxSize(90, 30));
  wxButton* b_exit = new wxButton(my_parent_, wxID_ANY, "Exit", wxDefaultPosition, wxSize(90, 30));
  wxButton* b_load =
    new wxButton(my_parent_, wxID_ANY, "Load config", wxDefaultPosition, wxSize(90, 30));
  wxButton* b_save =
    new wxButton(my_parent_, wxID_ANY, "Save config", wxDefaultPosition, wxSize(90, 30));
  bottom_buttons_sizer->Add(b_update, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(b_undo, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(b_pause, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(b_load, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(b_save, wxSizerFlags().DoubleBorder());
  bottom_buttons_sizer->Add(b_exit, wxSizerFlags().DoubleBorder());
  vertical_sizer->Add(the_notebook_, 1, wxEXPAND);
  vertical_sizer->Add(bottom_buttons_sizer);
  my_parent_->SetSizerAndFit(vertical_sizer);
  // add callbacks to the buttons
  b_update->Bind(wxEVT_BUTTON, &main_notebook::on_update_button, this);
  b_undo->Bind(wxEVT_BUTTON, &main_notebook::on_undo_button, this);
  b_pause->Bind(wxEVT_BUTTON, &main_notebook::on_pause_button, this);
  b_exit->Bind(wxEVT_BUTTON, &main_notebook::on_exit_button, this);
  b_load->Bind(wxEVT_BUTTON, &main_notebook::on_load_button, this);
  b_save->Bind(wxEVT_BUTTON, &main_notebook::on_save_button, this);
  all_sockets.insert(associated_socket_);
}

void
main_notebook::TEMP_spawn_bounding_box_selection(int tilex, int tiley)
{
  wxFrame* frame = new wxFrame(NULL,
                               wxID_ANY,
                               wxT("Select bounding box and antistuck points"),
                               wxPoint(50, 50),
                               wxSize(768, 768));
  wxImagePanel* drawPane =
    new wxImagePanel(frame,
                     wxString::Format(R"(rsrc\Maps\%d_%d.jpg)", tilex, tiley),
                     wxBITMAP_TYPE_JPEG,
                     box,
                     tilex,
                     tiley);
  frame->Show();
}
