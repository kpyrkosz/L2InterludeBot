#include <Windows.h>
#include <action_executor.hpp>
#include <game_data_facade.hpp>
#include <native_lineage_classes.hpp>

#if NDEBUG
#define DEBUGPRINT
#else
#define DEBUGPRINT(...) fprintf(stderr, ##__VA_ARGS__)
#endif

bool temp_block_enterworld = false;

void(__fastcall* bypassed_say)(UNetworkHandler&, const L2ParamStack&) = nullptr;

void
action_executor::init_anti_antibots()
{
  // copy the say_bypass into a codecave in NWindow
  int nwinbase = (int)LoadLibraryA("NWindow.dll");
  DEBUGPRINT("Nwindow base %X last error %d\n", nwinbase, GetLastError());
  DEBUGPRINT("Bytes on + 0x1EC0 before patch\n");
  for (int i = 0; i < 16; ++i)
    DEBUGPRINT("%02X\n", *(unsigned char*)(nwinbase + 0x1EC0 + i));
  DEBUGPRINT("\n");
  DWORD old_protect, old_protect2;
  VirtualProtect((void*)(nwinbase + 0x1EC0), 16, PAGE_EXECUTE_READWRITE, &old_protect);
  /*
  PUSH EDX
  CALL offset
  RETN
  */
  // 52 E8 00 00 00 00 C3
  const char say_bypass_template[] = "\x52\xE8\x00\x00\x00\x00\xC3";
  memcpy((void*)(nwinbase + 0x1EC0), (void*)say_bypass_template, 7);
  int say_addr = (int)GetProcAddress(GetModuleHandleA("Engine.dll"),
                                     "?Say2@UNetworkHandler@@UAEXAAVL2ParamStack@@@Z");
  *(int*)(nwinbase + 0x1EC0 + 2) = say_addr - (nwinbase + 0x1EC0 + 1) - 5;
  DEBUGPRINT("Say addr %X source %X offset %x\n",
             say_addr,
             nwinbase + 0x1EC0 + 1,
             *(int*)(nwinbase + 0x1EC0 + 2));
  VirtualProtect((void*)(nwinbase + 0x1EC0), 16, old_protect, &old_protect2);
  DEBUGPRINT("Bytes on + 0x1EC0 after patch\n");
  for (int i = 0; i < 16; ++i)
    DEBUGPRINT("%02X\n", *(unsigned char*)(nwinbase + 0x1EC0 + i));
  DEBUGPRINT("\n");
  bypassed_say = (void(__fastcall*)(UNetworkHandler&, const L2ParamStack&))(nwinbase + 0x1EC0);

  // enterwoorld hook
  // 0x104540 - enterworld handler
  // 0x104542 - int3cave, just tiny bit over the enterworld function
  // 0xDA17 - jump to enterworld handler

  /*
  10404542    50              PUSH EAX
10404543    58              POP EAX
10404544    A1 13321200     MOV EAX,DWORD PTR DS:[0x123213]
10404549    85C0            TEST EAX,EAX
1040454B    74 03           JE SHORT engine.10404550
1040454D    C2 1800         RETN 0x18
  */
  /*const char enterworld_template[] = "\x50\x58\xA1\x00\x00\x00\x00\x85\xC0\x74\x03\xC2\x18\x00";
  auto engine_base = (DWORD)GetModuleHandleA("engine.dll");
  VirtualProtect((void*)(engine_base + 0x104542), 14, PAGE_EXECUTE_READWRITE, &old_protect);
  memcpy((void*)(engine_base + 0x104542), (void*)enterworld_template, 14);
  bool* bool_ptr = &temp_block_enterworld;
  memcpy((void*)(engine_base + 0x104542 + 3), (void*)&bool_ptr, 4);
  VirtualProtect((void*)(engine_base + 0x104542), 14, old_protect, &old_protect2);

  VirtualProtect((void*)(engine_base + 0xDA17), 5, PAGE_EXECUTE_READWRITE, &old_protect);
  *(int*)(engine_base + 0xDA17 + 1) -= 14;
  VirtualProtect((void*)(engine_base + 0xDA17), 5, old_protect, &old_protect2);*/
}

void
action_executor::whisper(const wchar_t* msg, const wchar_t* receiver)
{
  L2ParamStack params(10);
  params.PushBack((void*)2); // channel - 2 whisper
  params.PushBack((void*)msg);
  params.PushBack((void*)receiver);
  // smartguard bypass
  bypassed_say(UNetworkHandler::get(), params);
  // UNetworkHandler::get().UNetworkHandler::Say2(params);
}

void
action_executor::action(int objid, bool shift)
{
  // this is ingenious! let action work only once 250 ms
  static auto lastaction = 0;
  if (lastaction + 250 > GetTickCount())
    return;
  lastaction = GetTickCount();
  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  ziemniak->UNetworkHandler::Action(
    objid,
    {0.0f, 0.0f, 0.0f},
    shift); // not sure about FVector but server files seem to ignore it anyways
}

void
action_executor::to_village()
{
  L2ParamStack params(10);
  params.PushBack((void*)0);
  UNetworkHandler::get().UNetworkHandler::RequestRestartPoint(params);
}

void
action_executor::ctrl_attack(int objid, bool shift)
{
  UNetworkHandler::get().UNetworkHandler::Atk(objid, {0.0f, 0.0f, 0.0f}, shift);
}

void
action_executor::use_spell(int spellid, bool shift, bool ctrl)
{
  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  L2ParamStack params(10);
  params.PushBack((void*)spellid);
  params.PushBack((void*)shift);
  params.PushBack((void*)ctrl);
  ziemniak->UNetworkHandler::RequestMagicSkillUse(params);
}

void
action_executor::move_to_xyz(const FVector& xyz)
{
  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  ziemniak->UNetworkHandler::MoveBackwardToLocation(xyz, game_data_facade::get_my_user().get_xyz());
}

void
action_executor::use_item(int skillid, bool ctrl_pressed)
{
  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  L2ParamStack params(10);
  params.PushBack((void*)skillid);
  params.PushBack((void*)ctrl_pressed);
  ziemniak->UNetworkHandler::RequestUseItem(params);
}

void
action_executor::sell_crops(const std::vector<manor_crop>& crops)
{
  UNetworkHandler& ziemniak = UNetworkHandler::get();
  L2ParamStack params(100);
  params.PushBack((void*)crops.size());
  for (const auto& crop : crops)
  {
    params.PushBack((void*)crop.objid);
    params.PushBack((void*)crop.itemid);
    params.PushBack((void*)crop.castle);
    params.PushBack((void*)crop.count);
  }
  ziemniak.UNetworkHandler::RequestProcureCropList(&params);
}

void
action_executor::buy_seeds(int manor_id, const std::vector<manor_buy_seed>& seeds)
{
  L2ParamStack params(100);
  params.PushBack((void*)manor_id);
  params.PushBack((void*)seeds.size());
  for (const auto& seed : seeds)
  {
    params.PushBack((void*)seed.itemid);
    params.PushBack((void*)seed.count);
  }
  UNetworkHandler::get().UNetworkHandler::RequestBuySeed(params);
}

void
action_executor::craft_click(int recipe_id)
{
  UNetworkHandler::get().UNetworkHandler::RequestRecipeItemMakeSelf(recipe_id);
}
