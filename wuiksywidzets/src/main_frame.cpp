#include <binary_deserializer.hpp>
#include <binary_serializer.hpp>
#include <connector.hpp>
#include <main_frame.hpp>
#include <user_panel.hpp>
#include <wx/artprov.h>
#include <wx/treebook.h>

// singletons init
#include <beastfarm_data.hpp>
#include <craft_data.hpp>
#include <item_data.hpp>
#include <manor_data.hpp>
#include <mutex>
#include <npc_data.hpp>
#include <skill_data.hpp>
#include <thread>
#include <vector>

enum request_type : char
{
  auth_key_and_hwid, // int salt, 32char key, hwid not yet
  reverse_lol,
  finalization
};

main_frame::main_frame(const wxString& title, const wxPoint& pos, const wxSize& size)
  : wxFrame(NULL, wxID_ANY, title, pos, size)
{
  create_menu();
  create_log();
  create_bottom_statusbar();
  load_icons();

  // character choicebook
  character_list_ = new wxTreebook(this, wxID_ANY);
  character_list_->SetMinSize(wxSize(750, 400));

  // vertical sizer for main panel and logwindow
  wxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(character_list_, 1, wxEXPAND | wxALL, 5);
  mainSizer->Add(log_window_, 0, wxGROW);

  SetSizerAndFit(mainSizer);

  wxInitAllImageHandlers();
}

main_frame::~main_frame()
{
  delete icon_imagelist_; // todo
  delete acceptor_;
}

void
main_frame::Auth()
{
#ifndef NDEBUG
  start_listening();
#else
  try
  {
    /*connector conn("auth.szaka.eu", "7113");
    conn.init_connection();
    std::string welcome;
    int remaining_time;
    // send key and hwid
    {
      std::array<char, 2048> buffer;
      binary_serializer bs(buffer);
      bs << auth_key_and_hwid << rand() << std::string("12345678912345678912345678912345")
         << std::string(40, 'a');
      conn.send_data(buffer.data(), bs.get_current_size());
      std::string reply = conn.get_reply();
      binary_deserializer bd(reply);
      bd >> welcome >> remaining_time;
    }
    // 15 various strings xD
    for (int i = 0; i < 15; ++i)
    {
      wxLogMessage("Loading module %d...", i);
      std::array<char, 2048> buffer;
      binary_serializer bs(buffer);
      int send_salt = rand();
      std::string send_str(400 + rand() % 800, '0');
      for (auto& c : send_str)
        c = rand();
      bs << reverse_lol << send_salt << send_str;
      conn.send_data(buffer.data(), bs.get_current_size());
      std::string reply = conn.get_reply();
      binary_deserializer bd(reply);
      int recv_salt;
      std::string recv_str;
      bd >> recv_salt >> recv_str;
      if (recv_salt != send_salt || send_str != std::string(recv_str.rbegin(), recv_str.rend()))
        throw std::runtime_error("Error authenticating");
    }
    // finalize
    {
      std::array<char, 2048> buffer;
      binary_serializer bs(buffer);
      int send_salt = rand();
      std::string send_str(400 + rand() % 800, '0');
      for (auto& c : send_str)
        c = rand();
      bs << finalization << send_salt << send_str;
      conn.send_data(buffer.data(), bs.get_current_size());
      std::string reply = conn.get_reply();
      binary_deserializer bd(reply);
      int recv_salt;
      std::string recv_str;
      bd >> recv_salt >> recv_str;
      if (recv_salt != send_salt || send_str != recv_str)
        throw std::runtime_error("Data error");
    }
    wxLogMessage("%s key is valid for %d hours (%d days)",
                 welcome,
                 remaining_time / 3600,
                 remaining_time / (3600 * 24));*/
    start_listening();
  }
  catch (const std::exception& e)
  {
    wxMessageBox(e.what(), "Authentication error");
    exit(0);
  }
  wxLogMessage("Initializing game data with %d threads...", std::thread::hardware_concurrency());
  std::vector<std::thread> static_inits;
  std::mutex log_sync;
  static_inits.emplace_back([&log_sync] {
    beastfarm_data::get();
    std::lock_guard<std::mutex> qq(log_sync);
    wxLogMessage("Beastfarm...");
  });
  static_inits.emplace_back([&log_sync] {
    craft_data::get();
    std::lock_guard<std::mutex> qq(log_sync);
    wxLogMessage("Craft...");
  });
  static_inits.emplace_back([&log_sync] {
    item_data::get();
    std::lock_guard<std::mutex> qq(log_sync);
    wxLogMessage("Items...");
  });
  static_inits.emplace_back([&log_sync] {
    manor_data::get();
    std::lock_guard<std::mutex> qq(log_sync);
    wxLogMessage("Manor...");
  });
  static_inits.emplace_back([&log_sync] {
    npc_data::get();
    std::lock_guard<std::mutex> qq(log_sync);
    wxLogMessage("Npcs...");
  });
  static_inits.emplace_back([&log_sync] {
    skill_data::get();
    std::lock_guard<std::mutex> qq(log_sync);
    wxLogMessage("Skills...");
  });
  for (auto& t : static_inits)
    t.join();
#endif
}

void
main_frame::create_menu()
{
  wxMenuBar* menuBar = new wxMenuBar;

  // general menu
  wxMenu* menuFile = new wxMenu;
  menuFile->Append(
    ID_Hello, "&Hello...\tCtrl-H", "Help string shown in status bar for this menu item");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT, "dupa quit", "help to quit");

  // about menu
  wxMenu* menuHelp = new wxMenu;
  menuHelp->Append(wxID_ABOUT);
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  // boom
  SetMenuBar(menuBar);
}

void
main_frame::create_log()
{
  // log window in the bottom
  log_window_ =
    new wxTextCtrl(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  log_window_->SetMinSize(wxSize(-1, 150));
  wxLog::SetActiveTarget(new wxLogTextCtrl(log_window_));
  wxLogMessage("This is the log window");
}

void
main_frame::create_bottom_statusbar()
{
  CreateStatusBar();
  SetStatusText("Welcome to szakabot!");
}

void
main_frame::load_icons()
{
  const wxSize imageSize(24, 24);

  // this leaks lol
  icon_imagelist_ = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\target.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\farming.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\spells.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\items.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\party.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\manor.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\craft.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\tasks.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\alarm.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\follow.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\pick.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\relax.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\out.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\tricks.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\map.ico)"));
  icon_imagelist_->Add(wxIconLocation(R"(rsrc\buffalo.ico)"));
}

void
main_frame::start_listening()
{
  wxIPV4address addr;
  if (!addr.Service(6111))
  {
    wxMessageBox("Unable to open listening port", "Critical error", wxOK | wxICON_ERROR);
    Close(true);
  }
  acceptor_ = new wxSocketServer(addr);
  if (!acceptor_->Ok())
  {
    wxMessageBox(wxString::Format("Server listener creation error %d!", acceptor_->LastError()),
                 "Critical error",
                 wxOK | wxICON_ERROR);
    acceptor_->Destroy();
    Close(true);
  }
  wxEvtHandler* server_event_handler = new wxEvtHandler;
  acceptor_->SetEventHandler(*server_event_handler, ID_SERVER);

  server_event_handler->Connect(
    ID_SERVER, wxEVT_SOCKET, (wxObjectEventFunction)&main_frame::OnServerEvent, (wxObject*)this);

  acceptor_->SetNotify(wxSOCKET_CONNECTION_FLAG);
  acceptor_->Notify(true);
  wxLogMessage("Created server socket");
}

void
main_frame::OnExit(wxCommandEvent& event)
{
  Close(true);
}

void
main_frame::OnAbout(wxCommandEvent& event)
{
  wxMessageBox("Boooo", "About szakabot", wxOK | wxICON_INFORMATION);
}

void
main_frame::OnServerEvent(wxSocketEvent& event)
{
  // do not use "this" here. for some reason the Connect requires a nonstatic method but it treats
  // it as static??
  wxASSERT(event.GetSocketEvent() == wxSOCKET_CONNECTION);
  wxSocketServer* pServerSocket = (wxSocketServer*)event.GetSocket();
  wxSocketBase* pSocket = pServerSocket->Accept(false);
  if (!pSocket)
    return;
  pSocket->SetFlags(wxSOCKET_NOWAIT);
  main_frame* me = (main_frame*)event.GetEventUserData();
  static wxString name = "Loading...";

  me->character_list_->AddPage(new user_panel(pSocket, me->character_list_, me->icon_imagelist_),
                               name);
  wxLogMessage("Adding next guy!");
  me->Layout();
}

void
main_frame::OnHello(wxCommandEvent& event)
{
}

wxBEGIN_EVENT_TABLE(main_frame, wxFrame) EVT_MENU(ID_Hello, main_frame::OnHello)
  EVT_MENU(wxID_EXIT, main_frame::OnExit) EVT_MENU(wxID_ABOUT, main_frame::OnAbout)
    wxEND_EVENT_TABLE()
