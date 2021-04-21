#include <aes_boy.hpp>
#include <binary_deserializer.hpp>
#include <event_identifiers.hpp>
#include <user_panel.hpp>

void
user_panel::start_listening()
{
  wxEvtHandler* socket_event_handler = new wxEvtHandler;
  associated_socket_->SetEventHandler(*socket_event_handler, ID_SOCKET);
  associated_socket_->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);

  socket_event_handler->Connect(
    ID_SOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&user_panel::OnSocketEvent, (wxObject*)this);
  associated_socket_->Notify(true);
}

void
user_panel::OnSocketEvent(wxSocketEvent& event)
{
  // do not use "this" here. for some reason the Connect requires a nonstatic method but it treats
  // it as static??
  user_panel* me = (user_panel*)event.GetEventUserData();
  me->on_socket_event(event);
}

void
user_panel::on_socket_event(wxSocketEvent& event)
{
  switch (event.GetSocketEvent())
  {
    case wxSOCKET_INPUT:
    {
      int readsize = 0;
      char readbuf[2000] = {}; // tu jesto cos zjebane jak pakiet jest wiekszy niz recvbuf...
      do
      {
        associated_socket_->Read(readbuf, sizeof(readbuf));
        readsize += associated_socket_->LastReadCount();
        incoming_packet.append(readbuf, associated_socket_->LastReadCount());
      } while (associated_socket_->LastReadCount() > 0);
      wxLogMessage(
        wxString::Format("OnSocketEvent wxSOCKET_INPUT datasize %d %p!", readsize, this));
      check_packet_received();
      break;
    }
    case wxSOCKET_LOST:
      wxLogMessage(wxString::Format("OnSocketEvent wxSOCKET_LOST %p!", this));
      // associated_socket_->Destroy();
      if (!associated_socket_->IsClosed())
        associated_socket_->Close();
      break;
    default:
      wxLogMessage(wxString::Format("OnSocketEvent default %p!", this));
      break;
  }
}

void
user_panel::check_packet_received()
{
  wxLogMessage(wxString::Format("check_packet_received stringsize %d!", incoming_packet.size()));
  if (incoming_packet.size() < 4)
    return;
  unsigned data_size = *(unsigned*)incoming_packet.data();
  if (data_size > 2048)
  {
    wxLogMessage(wxString::Format(
      "check_packet_received wrong datasize %d STRSIZE %d!", data_size, incoming_packet.size()));
    return;
  }
  if (incoming_packet.size() - 4 < data_size)
  {
    wxLogMessage(
      wxString::Format("check_packet_received not yet %d %d!", data_size, incoming_packet.size()));
    return;
  }
  wxLogMessage(wxString::Format("proper packet %d %d!", data_size, incoming_packet.size()));
  binary_deserializer bd(incoming_packet.data() + 4, data_size);
  to_controller_protocol opcode;
  bd >> opcode;
  switch (opcode)
  {
    case to_controller_protocol::log_string:
    {
      std::string msg;
      bd >> msg;
      wxLogMessage(msg.c_str());
    }
    break;
    case to_controller_protocol::game_entered:
    {
      std::string name;
      bd >> name;
      wxLogMessage(name.c_str());
      for (int i = 0; i < my_parent_->GetPageCount(); ++i)
        if (my_parent_->GetPage(i) == this)
          my_parent_->SetPageText(i, name.c_str());
    }
    break;
    case to_controller_protocol::spell_list:
    {
      int count;
      bd >> count;
      current_character_data_.spells.clear();
      wxLogMessage(wxString::Format("Received %d spells!", count));
      for (int i = 0; i < count; ++i)
      {
        int id;
        std::string name;
        bd >> id >> name;
        current_character_data_.spells[id] = name;
      }
    }
    break;
    case to_controller_protocol::item_list:
    {
      int count;
      bd >> count;
      current_character_data_.items.clear();
      wxLogMessage(wxString::Format("Received %d items!", count));
      for (int i = 0; i < count; ++i)
      {
        int id;
        std::string name;
        bd >> id >> name;
        current_character_data_.items[id] = name;
      }
    }
    break;
    case to_controller_protocol::char_position:
    {
      int x, y, z;
      bd >> x >> y >> z;
      int tilex = (x + 20 * 32768) / 32768; // cause integer division for negative
      int tiley = (y + 18 * 32768) / 32768;
      wxLogMessage("You are on xyz %d %d %d = tile %d %d", x, y, z, tilex, tiley);
      main_notebook_.TEMP_spawn_bounding_box_selection(tilex, tiley);
    }
    break;
    case to_controller_protocol::broadcast:
    {
      int count;
      bd >> count;
      current_character_data_.items.clear();
      incoming_packet[4] =
        (char)to_bot_protocol::broadcast; // broadcast id is different both ways qq
      wxLogMessage(wxString::Format("Received broadcast size-1 %d!", data_size - 1));
      aes_boy::inplace_encrypt_to_dll(incoming_packet.data() + 4, data_size);
      for (auto* socket : main_notebook::all_sockets)
      {
        socket->Write((char*)&data_size, sizeof(data_size));
        socket->Write(incoming_packet.data() + 4, data_size);
      }
    }
    break;
    default:
      wxLogMessage(wxString::Format("Unknown opcode %d", (int)opcode));
      break;
  }
  incoming_packet.erase(0, data_size + 4);
  check_packet_received();
}

user_panel::user_panel(wxSocketBase* associated_socket,
                       wxBookCtrlBase* my_parent,
                       wxImageList* icon_imagelist)
  : wxPanel(my_parent, wxID_ANY)
  , main_notebook_(this, my_parent, icon_imagelist, associated_socket, current_character_data_)
  , associated_socket_(associated_socket)
  , my_parent_(my_parent)
{
  start_listening();
}
