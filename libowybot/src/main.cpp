#include <Windows.h>
#include <action_executor.hpp>
#include <configuration_setter.hpp>
#include <cstdio>
#include <mainloop_hook.hpp>
#include <network_table_hook.hpp>

/*struct init_data
{
  int current_process_id;  // by loader
  int dll_base;            // by boy
  int private_checksum_XD; // by bot
  HMODULE(WINAPI* realGetModuleHandleA)(_In_opt_ LPCSTR lpModuleName);
  FARPROC(__stdcall* realGetProcAddress)(_In_ HMODULE hModule, _In_ LPCSTR lpProcName);
  char export_name[10];
  int ip;
  short port;
};

extern "C" __declspec(dllexport) void __stdcall foo(init_data* data)
{
  if (data->private_checksum_XD != data->current_process_id * data->dll_base + (int)data)
    return;
  if (GetCurrentProcessId() != data->current_process_id)
    return;

  AllocConsole();
  freopen("CONOUT$", "w", stdout);
  freopen("CONOUT$", "w", stderr);
  mainloop_hook::install_hook();
  network_table_hook::install_hooks();
  configuration_setter::start_listening(data->ip, data->port);
}*/

extern "C" __declspec(dllexport) int tralala()
{
  return 0xDEADBEEF;
};

DWORD WINAPI
DllMain(HINSTANCE h, DWORD reason, LPVOID q)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
      //#ifndef NDEBUG
      AllocConsole();
      freopen("CONOUT$", "w", stdout);
      freopen("CONOUT$", "w", stderr);
      puts("TEST MODE BEZ EXPORTA");
      puts("TEST MODE BEZ EXPORTA");
      puts("TEST MODE BEZ EXPORTA");
      //#endif
      /*mainloop_hook::install_hook();
      network_table_hook::install_hooks();
      configuration_setter::start_listening(0x7F000001, 6111);*/
      // for some reason this does not work, loadlibrary returns 0 and error 1001...
      // i will try to run it inside thread maybe to wait until everything is loaded?
      // action_executor::init_anti_antibots();
      // update 2. i will throw ALL the initialization there
      CreateThread(
        0,
        0,
        [](void*) -> DWORD {
          mainloop_hook::install_hook();
          network_table_hook::install_hooks();
          configuration_setter::start_listening(0x7F000001, 6111);
          action_executor::init_anti_antibots();
          return 1337;
        },
        0,
        0,
        0);
      break;
  }
  return 1;
}

// virtual float __thiscall UNetworkHandler::GetDistance(class FObjectMap*, class FObjectMap*)
