#include <Windows.h>
#include <binary_serializer.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <codecvt>
#include <game_data_facade.hpp>
#include <manor_crops.hpp>
#include <native_lineage_classes.hpp>
#include <network_table_hook.hpp>
#include <party.hpp>
#include <player_character.hpp>
#include <protocol.hpp>
#include <stdio.h>

__declspec(dllimport) FL2GameData GL2GameData;

// void* UNK in handlers MEANS UNETWORKHANDLER + 48 dereferenced
char*(__cdecl* disasm_packet)(void* unetwork_from_unk, char* buffer, const char* format, ...);
// 0x34E5

// 10300000
// standard packets MOV EAX,DWORD PTR DS:[ECX+0x10A57310]
// extended packets MOV EAX,DWORD PTR DS:[ECX+0x10A67610]
struct networkTableEntry
{
  int(__cdecl* handler)(void* unetwork_i_think, char packet[]);
  const wchar_t packetName[128];
};
static_assert(sizeof(networkTableEntry) == 0x104,
              "INTERLUDE networkTableEntry has to be 0x104 in size!");

template <typename Func>
Func
hookHandler(unsigned index, Func myFunc, networkTableEntry* packet_table)
{
  Func realFunc = packet_table[index].handler;
  packet_table[index].handler = myFunc;
  return realFunc;
}

// 06=Die:d(CharID)d(d)d(HAsHideout)d(HasCastle)d(flags)d(sweepable)d(access)
int(__cdecl* realDie)(void* unetwork_i_think, char packet[]);
int __cdecl myDie(void* unetwork_i_think, char packet[])
{
  struct Die
  {
    int objid;
    int unk[4];
    int sweepable;
  };

  Die* data = (Die*)packet;
  User* the_guy = game_data_facade::get_by_id(data->objid);
  if (the_guy)
    the_guy->get_apawn()->get_acontroller()->set_custom_sweepable(data->sweepable);

  // printf("Die obj %X sweepable %d\n", data->objid, data->sweepable);
  return realDie(unetwork_i_think, packet);
}
// 0B=SpawnItem:d(ObjectID)d(ItemID:Get.Func01)d(X)d(Y)d(Z)d(Stackable)d(Count)d(d)
int(__cdecl* realSpawnItem)(void* unetwork_i_think, char packet[]);
int __cdecl mySpawnItem(void* unetwork_i_think, char packet[])
{
  struct SpawnItem
  {
    int objid;
    int itemid;
    int x, y, z;
    int stackable;
    int count;
  };

  SpawnItem* data = (SpawnItem*)packet;
  /*printf("SpawnItem obj %X itemid %d xyz %d %d %d\nname %ws\naddname %ws\n",
         data->objid,
         data->itemid,
         data->x,
         data->y,
         data->z,
         GL2GameData.GetItemName(data->itemid),
         GL2GameData.GetItemAdditionalName(data->itemid));*/
  return realSpawnItem(unetwork_i_think, packet);
}

// 0C=DropItem:d(PlayerID)d(ObjectID)d(ItemID:Get.Func01)d(X)d(Y)d(Z)d(Stackable)d(Count)d(d)
int(__cdecl* realDropItem)(void* unetwork_i_think, char packet[]);
int __cdecl myDropItem(void* unetwork_i_think, char packet[])
{
  struct DropItem
  {
    int playerid;
    int objid;
    int itemid;
    int x, y, z;
    int stackable;
    int count;
  };

  DropItem* data = (DropItem*)packet;
  printf("DropItem player %X obj %X itemid %d xyz %d %d %d\nname %ws\naddname %ws\n",
         data->playerid,
         data->objid,
         data->itemid,
         data->x,
         data->y,
         data->z,
         GL2GameData.GetItemName(data->itemid),
         GL2GameData.GetItemAdditionalName(data->itemid));
  // not entirely sure what player means in that context, but i assume
  // it to be either killed monster or lasthitting party member
  // party::get().members
  if (get_bot_settings().tricks_cfg.dont_show_droped_arrows && data->itemid >= 1341 &&
      data->itemid <= 1345)
  {
    puts("FUCK THE ARROW DROPS!!! CUNT!!!");
    return 0;
  }
  if (game_data_facade::get_my_user().get_target_id() == data->playerid)
  {
    game_data_facade::get_my_drops()[data->objid] = {
      GetTickCount(), data->objid, FVector(data->x, data->y, data->z)};
    puts("TAK, get_target_id() == data->playerid");
    return realDropItem(unetwork_i_think, packet);
  }
  if (game_data_facade::get_my_id() == data->playerid)
  {
    game_data_facade::get_my_drops()[data->objid] = {
      GetTickCount(), data->objid, FVector(data->x, data->y, data->z)};
    puts("TAK, get_my_id() == data->playerid");
    return realDropItem(unetwork_i_think, packet);
  }
  for (const auto& pair : party::get().members())
  {
    if (pair.first == data->playerid)
    {
      game_data_facade::get_my_drops()[data->objid] = {
        GetTickCount(), data->objid, FVector(data->x, data->y, data->z)};
      puts("TAK, pair.first == data->playerid");
      return realDropItem(unetwork_i_think, packet);
    }
    auto* the_guy = game_data_facade::get_by_id(pair.first);
    if (the_guy == nullptr)
      continue;
    if (the_guy->get_target_id() == data->playerid)
    {
      game_data_facade::get_my_drops()[data->objid] = {
        GetTickCount(), data->objid, FVector(data->x, data->y, data->z)};
      puts("TAK,the_guy->get_target_id() == data->playerid");
      return realDropItem(unetwork_i_think, packet);
    }
  }
  return realDropItem(unetwork_i_think, packet);
}
// DeleteObject kasuje z ziemii np podczas TP
// 0D=GetItem:d(PlayerID)d(ObjectID)d(X)d(Y)d(Z)
int(__cdecl* realGetItem)(void* unetwork_i_think, char packet[]);
int __cdecl myGetItem(void* unetwork_i_think, char packet[])
{
  struct GetItem
  {
    int playerid;
    int objid;
    int x, y, z;
  };

  GetItem* data = (GetItem*)packet;
  /*printf("GetItem player %X obj %X  xyz %d %d %d\n",
         data->playerid,
         data->objid,
         data->x,
         data->y,
         data->z);*/
  game_data_facade::get_my_drops().erase(data->objid);
  if (data->playerid == game_data_facade::get_my_id())
    get_penalties().pick_requests_count = 0;
  return realGetItem(unetwork_i_think, packet);
}

// 15=CharSelected:s(Name)d(CharID)s(Title)d(SessionID)d(ClanID)d(d)d(Sex)d(Race)d(ClassID:Get.ClassID)d(active)d(X)d(Y)d(Z)f(Cur_HP)f(Cur_MP)d(SP)q(EXP)d(Level)d(Karma)d(d)d(INT)d(STR)d(CON)d(MEN)d(DEX)d(WIT)z(0128)d(inGameTime)z(0072)
int(__cdecl* realCharSelected)(void* unetwork_i_think, char packet[]);
int __cdecl myCharSelected(void* unetwork_i_think, char packet[])
{
  game_data_facade::is_in_world() = true;
  // 1041007A    68 E84D8810     PUSH engine.10884DE8; ASCII
  // "SdSddddddddddffdQddddddddddddddddddddddddddddddddddddddddddd"
  const wchar_t name[0x30] = {};
  int objid;
  const wchar_t title[0x1E] = {};
  int sessionid;
  int garbage;
  int x, y, z;
  double currhp, currmp;
  // God damn it clang format!
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "SdSddddddddddff",
                            0x30,
                            name,
                            &objid,
                            0x1E,
                            title,
                            &sessionid,
                            &garbage,
                            &garbage,
                            &garbage,
                            &garbage,
                            &garbage,
                            &garbage,
                            &x,
                            &y,
                            &z,
                            &currhp,
                            &currmp);
  printf("BUF przed/po %X %X\nobjid %X sessid %X name %ws\nxyz %d %d %d hp/mp %f %f\n",
         packet,
         res,
         objid,
         sessionid,
         name,
         x,
         y,
         z,
         currhp,
         currmp);
  player_character::get().name() = name;
  extern int sock;
  {
    std::array<char, 2048> to_send;
    binary_serializer bs(to_send);
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    std::string converted_str = converter.to_bytes(name);
    bs << to_controller_protocol::game_entered << converted_str;
    auto total_size = bs.get_current_size();
    send(sock, (char*)&total_size, sizeof(total_size), 0);
    send(sock, to_send.data(), total_size, 0);
  }
  {
    std::array<char, 2048> to_send;
    binary_serializer bs(to_send);
    bs << to_controller_protocol::log_string
       << std::string("duapdusapduasidasuduasoduasodiusaoiuaosiduaosdiuasod");
    auto total_size = bs.get_current_size();
    send(sock, (char*)&total_size, sizeof(total_size), 0);
    send(sock, to_send.data(), total_size, 0);
  }
  return realCharSelected(unetwork_i_think, packet);
}

// 16=NpcInfo:d(ObjectID)d(NpcTypeId:Get.NpcId)d(IsAttackable)d(X)d(Y)d(Z)d(Heading)d(d)d(Maspd)d(Paspd)d(runSpd)d(walkSpd)d(swimRSpd)d(swimWSpd)d(flRSpd)d(flWSpd)d(FlyRSpd)d(FlyWSpd)f(ProperMul)f(Paspd)f(CollisRadius)f(CollisHeight)d(RHand:Get.Func01)d(d)d(LHand:Get.Func01)c(nameabove)c(isRun)c(isInFight)c(isAlikeDead)c(isSummoned)s(Name)s(Title)d(d)d(PvpFlag)d(karma?)h(abnEffect)h(d)d(d)d(d)d(d)d(d)c(c)c(Team)f(collisRadius)f(collisHeight)d(d)
int(__cdecl* realNpcInfo)(void* unetwork_i_think, char packet[]);
int __cdecl myNpcInfo(void* unetwork_i_think, char packet[])
{
  struct NpcInfo
  {
    int objid;
    int npcid;
  };

  NpcInfo* data = (NpcInfo*)packet;
  if (get_bot_settings().tricks_cfg.remove_trained_beastfarm_animals && data->npcid >= 1016013 &&
      data->npcid <= 1016018)
  {
    printf("SKIPPING TRAINED %d\n", data->npcid);
    return 0;
  }
  if (get_bot_settings().tricks_cfg.turn_cancel_animation_visible)
  {
    // 21718
    // 21753 rift guys
    int ids[] = {21718, 21753};
    static int counterqq = 0;
    ++counterqq;
    counterqq %= sizeof(ids) / sizeof(ids[0]);
    data->npcid = 1000000 + ids[counterqq]; // 25273 carnamak
                                            // 25504 nills
                                            // 25512 gigachaos
                                            // 29060 captains
                                            // 29056 sirra
  }
  return realNpcInfo(unetwork_i_think, packet);
}

// 1B=ItemListPacket:h(window)h(ListCount:For.0012)h(itemType1)d(ObjectId)d(ItemID:Get.Func01)d(count)h(itemType2)h(CustType1)h(isEquipped)d(BodyPart)h(EnchantLevel)h(CustType2)d(AugId)d(Shadowtime)
int(__cdecl* realItemListPacket)(void* unetwork_i_think, char packet[]);
int __cdecl myItemListPacket(void* unetwork_i_think, char packet[])
{
  short windowqq, count;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "hh", &windowqq, &count);
  printf("Itemcount %d\n", count);
  auto& inventory = game_data_facade::get_items();
  inventory.clear();
  for (short i = 0; i < count; ++i)
  {
    short item_type1;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "h", &item_type1);

    // 10412479 "dddhhhdhhhhd"
    // d(ObjectId)d(ItemID:Get.Func01)d(count)h(itemType2)h(CustType1)h(isEquipped)d(BodyPart)h(EnchantLevel)h(CustType2)d(AugId)d(Shadowtime)
    int objid, itemid, count, bodypart, augid, shadowtime;
    short itemtype2, custype1, isequipped, enchant, custype2;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "dddhhhdhhdd",
                        &objid,
                        &itemid,
                        &count,
                        &itemtype2,
                        &custype1,
                        &isequipped,
                        &bodypart,
                        &enchant,
                        &custype2,
                        &augid,
                        &shadowtime);
    // temp to see manor seeds in my bag
    /*const wchar_t* item_name = (wchar_t*)GL2GameData.GetItemName(itemid);
    const wchar_t* prefix = L"Seed: ";
    if (wcsncmp(prefix, item_name, wcslen(prefix)) == 0)
    {
      printf("objid %X itemid %X count %d isequip %d name %ws\n\t%ws\n",
             objid,
             itemid,
             count,
             isequipped,
             GL2GameData.GetItemName(itemid),
             GL2GameData.GetItemDescription(itemid));
    }*/
    inventory[itemid] = item_data{objid, count};
  }
  return realItemListPacket(unetwork_i_think, packet);
}
// 27=InventoryUpdate:h(count:For.0013)h(1add2mod3remove)h(itemType1)d(ObjectId)d(ItemId:Get.Func01)d(Count)h(itemType2)h(cusType1)h(isEquipped)d(BodyPart)h(EnchantLevel)h(cusType2)d(AugId)d(Shadowtime)
int(__cdecl* realInventoryUpdate)(void* unetwork_i_think, char packet[]);
int __cdecl myInventoryUpdate(void* unetwork_i_think, char packet[])
{
  short count = 0;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "h", &count);
  // printf("Itemcount %d\n", count);
  auto& inventory = game_data_facade::get_items();

  for (short i = 0; i < count; ++i)
  {
    enum class add_mod_remove : short
    {
      add = 1,
      mod = 2,
      remove = 3
    };
    add_mod_remove amr;
    short item_type1;
    res = disasm_packet(
      (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "hh", &amr, &item_type1);
    // printf("amr %d itemtype1 %d\n", amr, item_type1);

    // 10412479 "dddhhhdhhhhd"
    // d(ObjectId)d(ItemId:Get.Func01)d(Count)h(itemType2)h(cusType1)h(isEquipped)d(BodyPart)h(EnchantLevel)h(cusType2)d(AugId)d(Shadowtime)
    int objid, itemid, count, bodypart, augid, shadowtime;
    short itemtype2, custype1, isequipped, enchant, custype2;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "dddhhhdhhdd",
                        &objid,
                        &itemid,
                        &count,
                        &itemtype2,
                        &custype1,
                        &isequipped,
                        &bodypart,
                        &enchant,
                        &custype2,
                        &augid,
                        &shadowtime);
    /*printf("objid %X itemid %X count %d isequip %d name %ws\n",
           objid,
           itemid,
           count,
           isequipped,
           GL2GameData.GetItemName(itemid));*/
    switch (amr)
    {
      case add_mod_remove::add:
      case add_mod_remove::mod:
        inventory[itemid] = item_data{objid, count};
        break;
      case add_mod_remove::remove:
        inventory.erase(itemid);
        break;
    }
  }
  return realInventoryUpdate(unetwork_i_think, packet);
}

// 29=TargetSelected:d(ObjectID)d(TargetID)d(X)d(Y)d(Z)
int(__cdecl* realTargetSelected)(void* unetwork_i_think, char packet[]);
int __cdecl myTargetSelected(void* unetwork_i_think, char packet[])
{
  struct TargetSelected
  {
    int objid;
    int targetid;
  };

  TargetSelected* data = (TargetSelected*)packet;
  // printf("TargetSelected obj %X target %X\n", data->objid, data->targetid);
  return realTargetSelected(unetwork_i_think, packet);
}

// 2A=TargetUnselected:d(TargetID)d(X)d(Y)d(Z)d(d)
int(__cdecl* realTargetUnselected)(void* unetwork_i_think, char packet[]);
int __cdecl myTargetUnselected(void* unetwork_i_think, char packet[])
{
  struct TargetUnselected
  {
    int objid;
  };

  TargetUnselected* data = (TargetUnselected*)packet;

  // printf("TargetUnselected obj %X\n", data->objid);
  return realTargetUnselected(unetwork_i_think, packet);
}

// 39=AskJoinParty:s(requestorName)d(itemDistribution)
int(__cdecl* realAskJoinParty)(void* unetwork_i_think, char packet[]);
int __cdecl myAskJoinParty(void* unetwork_i_think, char packet[])
{
  wchar_t name[0x30] = {};
  int item_distribution;
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "Sd",
                            0x30,
                            name,
                            &item_distribution);
  printf("AskJoinParty name %ws dist %d\n", name, item_distribution);

  // temp whitelist
  static const std::unordered_set<std::wstring> whitelist{
    L"BlauWeltmeister", L"sed", L"TURBOFRENZIARKA", L"tr", L"less", L"grep", L"Aquilla", L"awk", L"BrokenPipe" , L"KABAN", L"v1", L"d22" };
  // after temp, duplicate if inside

  if (get_bot_settings().acceptparty || whitelist.count(name))
  {
    auto& strname = get_bot_settings().party_inviter;
    std::wstring name_as_widestr(strname.begin(), strname.end());
    if (name_as_widestr == name || whitelist.count(name))
    {
      UNetworkHandler* ziemniak =
        *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
      L2ParamStack params(10);
      params.PushBack((void*)1);
      ziemniak->UNetworkHandler::RequestAnswerJoinParty(params);
      return 0; // dont execute orginal, it just shows the dummy request
    }
  }
  return realAskJoinParty(unetwork_i_think, packet);
}

// 3A=JoinParty:d(response)
int(__cdecl* realJoinParty)(void* unetwork_i_think, char packet[]);
int __cdecl myJoinParty(void* unetwork_i_think, char packet[])
{
  int response;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &response);
  printf("JoinParty response %d\n", response);
  party::get().print();
  return realJoinParty(unetwork_i_think, packet);
}

// 3B=WithdrawalParty:d(partyID)
int(__cdecl* realWithdrawalParty)(void* unetwork_i_think, char packet[]);
int __cdecl myWithdrawalParty(void* unetwork_i_think, char packet[])
{
  int partyid;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &partyid);
  printf("WithdrawalParty partyid %d\n", partyid);
  party::get().print();
  return realWithdrawalParty(unetwork_i_think, packet);
}

// 3C=OustPartyMember:d(d)
int(__cdecl* realOustPartyMember)(void* unetwork_i_think, char packet[]);
int __cdecl myOustPartyMember(void* unetwork_i_think, char packet[])
{
  int d;
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &d);
  party::get().members().erase(d);
  printf("OustPartyMember d??? %d\n", d);
  party::get().print();
  return realOustPartyMember(unetwork_i_think, packet);
}

// 3D=SetOustPartyMember:
int(__cdecl* realSetOustPartyMember)(void* unetwork_i_think, char packet[]);
int __cdecl mySetOustPartyMember(void* unetwork_i_think, char packet[])
{
  party::get().members().clear();
  printf("SetOustPartyMember\n");
  party::get().print();
  return realSetOustPartyMember(unetwork_i_think, packet);
}

// 3E=DismissParty:d(partyID)
int(__cdecl* realDismissParty)(void* unetwork_i_think, char packet[]);
int __cdecl myDismissParty(void* unetwork_i_think, char packet[])
{
  int partyid;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &partyid);
  printf("DismissParty partyid %d\n", partyid);
  party::get().print();
  return realDismissParty(unetwork_i_think, packet);
}

// 3F=SetDismissParty:
int(__cdecl* realSetDismissParty)(void* unetwork_i_think, char packet[]);
int __cdecl mySetDismissParty(void* unetwork_i_think, char packet[])
{
  party::get().members().clear();
  printf("SetDismissParty\n");
  party::get().print();
  return realSetDismissParty(unetwork_i_think, packet);
}

// 48=MagicSkillUse:d(charID)d(targetID)d(skillID:Get.Skill)d(skillLvl)d(hitTime)d(reuseDelay)d(X)d(Y)d(Z)h(count:for.0003)d(d)d(d)d(d)
int(__cdecl* realMagicSkillUse)(void* unetwork_i_think, char packet[]);
int __cdecl myMagicSkillUse(void* unetwork_i_think, char packet[])
{
  // there is some loop at the end i dont know what it means
  int charid, targetid, skillid, skilllvl, hittime, reuse, x, y, z;
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "ddddddddd",
                            &charid,
                            &targetid,
                            &skillid,
                            &skilllvl,
                            &hittime,
                            &reuse,
                            &x,
                            &y,
                            &z);
  /*printf(
    "MagicSkillUse charid %X target %X skill %X (%d) lvl %d\n\thittime %d reuse %d xyz %d %d %d\n",
    charid,
    targetid,
    skillid,
    skillid,
    skilllvl,
    hittime,
    reuse,
    x,
    y,
    z);*/
  if (charid == game_data_facade::get_my_id())
  {
    get_penalties().magic_cast_count = 0;
    get_penalties().spells = GetTickCount() + hittime;
    get_penalties().partybuffs = GetTickCount() + hittime;
    game_data_facade::get_cooldowns()[skillid] = GetTickCount() + reuse;

    User* the_guy = game_data_facade::get_by_id(targetid);

    if (the_guy && skillid == 2098) // harvesting
      the_guy->get_apawn()->get_acontroller()->set_custom_manorable(
        false);                   // not to spam harvest after monster dies
                                  // mby add 2097 and custom sowing cooldown?
    if (the_guy && skillid == 42) // sweeping
      the_guy->get_apawn()->get_acontroller()->set_custom_sweepable(false);
  }

  return realMagicSkillUse(unetwork_i_think, packet);
}

// 49=MagicSkillCanceled:d(objectID)
int(__cdecl* realMagicSkillCanceled)(void* unetwork_i_think, char packet[]);
int __cdecl myMagicSkillCanceled(void* unetwork_i_think, char packet[])
{
  int objid;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &objid);
  // printf("MagicSkillCanceled objid %d\n", objid);
  if (objid == game_data_facade::get_my_id())
  {
    get_penalties().spells = GetTickCount();
    get_penalties().partybuffs = GetTickCount();
  }
  return realMagicSkillCanceled(unetwork_i_think, packet);
}

// 76=MagicSkillLaunched:d(charID)d(skillID:Get.Skill)d(skillLvl)d(failed)d(targetId)
int(__cdecl* realMagicSkillLaunched)(void* unetwork_i_think, char packet[]);
int __cdecl myMagicSkillLaunched(void* unetwork_i_think, char packet[])
{
  // huh, this "failed" is equivalent of target count on h5
  int charid, skillid, skilllvl, failed, targetid;
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "ddddd",
                            &charid,
                            &skillid,
                            &skilllvl,
                            &failed,
                            &targetid);
  /*printf("MagicSkillLaunched charid %X target %X skill %X (%d) lvl %d failed %d\n",
         charid,
         targetid,
         skillid,
         skillid,
         skilllvl,
         failed);*/
  return realMagicSkillLaunched(unetwork_i_think, packet);
}
// 4A=Say2:d(ObjectID)d(textType:Get.Func02)s(charName)s(Message)
int(__cdecl* realSay2)(void* unetwork_i_think, char packet[]);
int __cdecl mySay2(void* unetwork_i_think, char packet[])
{
  // huh, this "failed" is equivalent of target count on h5
  int objid, texttype;
  wchar_t name[50] = {};
  wchar_t msg[300] = {};
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "ddSS",
                            &objid,
                            &texttype,
                            50,
                            name,
                            300,
                            msg);
  // printf("MSG: Objid %x type %d name %ws msg %ws\n", objid, texttype, name, msg);
  return realSay2(unetwork_i_think, packet);
}
// 4E=PartySmallWindowAll:d(objectID)d(Party)d(ListCount:For.0012)d(objID)s(Name)d(cur_CP)d(max_CP)d(cur_HP)d(max_HP)d(cur_MP)d(max_MP)d(lvl)d(classId)d(d)d(d)
int(__cdecl* realPartySmallWindowAll)(void* unetwork_i_think, char packet[]);
int __cdecl myPartySmallWindowAll(void* unetwork_i_think, char packet[])
{
  int QQobjid, partyid, count;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "ddd", &QQobjid, &partyid, &count);
  // printf("PartySmallWindowAll QQobj %X count %d partyid %X\n", QQobjid, count, partyid);

  for (int i = 0; i < count; ++i)
  {
    // d(objID)s(Name)d(cur_CP)d(max_CP)d(cur_HP)d(max_HP)d(cur_MP)d(max_MP)d(lvl)d(classId)d(d)d(d)
    // 104394C2 "dSdddddddddd"

    int objid, cp, maxcp, hp, maxhp, mp, maxmp, lvl, classid, unkd;
    wchar_t name[0x30] = {};

    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "dSdddddddddd", // sprobuje przykrocic..
                        &objid,
                        0x30,
                        name,
                        &cp,
                        &maxcp,
                        &hp,
                        &maxhp,
                        &mp,
                        &maxmp,
                        &lvl,
                        &classid,
                        &unkd,
                        &unkd);
    if (game_data_facade::get_my_id() != objid)
    {
      /*printf("objid %X name %ws lvl %d hp %d/%d mp %d/%d cp %d/%d\n",
             objid,
             name,
             lvl,
             hp,
             maxhp,
             mp,
             maxmp,
             cp,
             maxcp);*/
      party::get().members()[objid] = {objid, 0, 0, 0, hp, maxhp, mp, maxmp, cp, maxcp, {}, name};
    }
  }
  // party::get().print();
  return realPartySmallWindowAll(unetwork_i_think, packet);
}

// 4F=PartySmallWindowAdd:d(playerObjId)d(d)d(memObjId)s(memName)d(cur_CP)d(max_CP)d(cur_HP)d(max_HP)d(cur_MP)d(max_MP)d(lvl)d(classId)d(d)d(d)
int(__cdecl* realPartySmallWindowAdd)(void* unetwork_i_think, char packet[]);
int __cdecl myPartySmallWindowAdd(void* unetwork_i_think, char packet[])
{
  // 10439823 "dddSdddddddddd"
  int playerobjid, unkd, memobjid, cp, maxcp, hp, maxhp, mp, maxmp, lvl, classid;
  wchar_t name[0x30] = {};
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "dddSdddddddddd",
                            &playerobjid, // thefuck, this is the PARTY LEADER ID
                            &unkd,
                            &memobjid, // this is new member
                            0x30,
                            name,
                            &cp,
                            &maxcp,
                            &hp,
                            &maxhp,
                            &mp,
                            &maxmp,
                            &lvl,
                            &classid,
                            &unkd,
                            &unkd);
  if (game_data_facade::get_my_id() != memobjid)
  {
    /*printf(
      "myPartySmallWindowAdd playerobjid %X unk %d memobjid %X name %ws lvl %d hp %d/%d mp %d/%d "
      "cp %d/%d\n",
      playerobjid,
      unkd,
      memobjid,
      name,
      lvl,
      hp,
      maxhp,
      mp,
      maxmp,
      cp,
      maxcp);*/
    party::get().members()[memobjid] = {
      memobjid, 0, 0, 0, hp, maxhp, mp, maxmp, cp, maxcp, {}, name};
    // party::get().print();
  }
  return realPartySmallWindowAdd(unetwork_i_think, packet);
}

// 50=PartySmallWindowDeleteAll:
int(__cdecl* realPartySmallWindowDeleteAll)(void* unetwork_i_think, char packet[]);
int __cdecl myPartySmallWindowDeleteAll(void* unetwork_i_think, char packet[])
{
  party::get().members().clear();
  printf("PartySmallWindowDeleteAll\n");
  party::get().print();
  return realPartySmallWindowDeleteAll(unetwork_i_think, packet);
}

// 51=PartySmallWindowDelete:d(memObjId)s(memberName)
int(__cdecl* realPartySmallWindowDelete)(void* unetwork_i_think, char packet[]);
int __cdecl myPartySmallWindowDelete(void* unetwork_i_think, char packet[])
{
  wchar_t name[0x30] = {};
  int objid;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "dS", &objid, 0x30, name);
  printf("PartySmallWindowDelete name %ws objid %d\n", name, objid);
  party::get().members().erase(objid);
  party::get().print();
  return realPartySmallWindowDelete(unetwork_i_think, packet);
}

// 52=PartySmallWindowUpdate:d(memObjId)s(memberName)d(cur_CP)d(max_CP)d(cur_HP)d(max_HP)d(cur_MP)d(max_MP)d(lvl)d(classId)
int(__cdecl* realPartySmallWindowUpdate)(void* unetwork_i_think, char packet[]);
int __cdecl myPartySmallWindowUpdate(void* unetwork_i_think, char packet[])
{
  int memobjid, cp, maxcp, hp, maxhp, mp, maxmp, lvl, classid;
  wchar_t name[0x30] = {};
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "dSdddddddd",
                            &memobjid,
                            0x30,
                            name,
                            &cp,
                            &maxcp,
                            &hp,
                            &maxhp,
                            &mp,
                            &maxmp,
                            &lvl,
                            &classid);
  /*printf("PartySmallWindowUpdate memobjid %X name %ws lvl %d hp %d/%d mp %d/%d cp %d/%d\n",
         memobjid,
         name,
         lvl,
         hp,
         maxhp,
         mp,
         maxmp,
         cp,
         maxcp);*/
  if (game_data_facade::get_my_id() != memobjid)
  {
    party::get().members()[memobjid].name = name;
    party::get().members()[memobjid].hp = hp;
    party::get().members()[memobjid].maxhp = maxhp;
    party::get().members()[memobjid].mp = mp;
    party::get().members()[memobjid].maxmp = maxmp;
    party::get().members()[memobjid].cp = cp;
    party::get().members()[memobjid].maxcp = maxcp;
    // party::get().print();
  }
  return realPartySmallWindowUpdate(unetwork_i_think, packet);
}

// A7=PartyMemberPosition:d(membercount:For.0004)d(objID)d(X)d(Y)d(Z)
int(__cdecl* realPartyMemberPosition)(void* unetwork_i_think, char packet[]);
int __cdecl myPartyMemberPosition(void* unetwork_i_think, char packet[])
{
  int count;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &count);
  // printf("PartyMemberPosition count %d\n", count);

  for (int i = 0; i < count; ++i)
  {
    int objid, x, y, z;
    res = disasm_packet(
      (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "dddd", &objid, &x, &y, &z);
    if (game_data_facade::get_my_id() == objid)
      continue;
    // printf("objid %X xyz %d %d %d\n", objid, x, y, z);
    party::get().members()[objid].x = x;
    party::get().members()[objid].y = y;
    party::get().members()[objid].z = z;
  }
  // party::get().print();
  return realPartyMemberPosition(unetwork_i_think, packet);
}

// 58=SkillList:d(ListCount:For.0004)d(isPassive)d(lvl)d(SkillID:Get.Skill)c(c)
int(__cdecl* realSkillList)(void* unetwork_i_think, char packet[]);
int __cdecl mySkillList(void* unetwork_i_think, char packet[])
{
  int count = 0;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &count);
  printf("Skillcount %d\n", count);

  auto& my_skills = game_data_facade::get_skills();
  my_skills.clear();

  std::unordered_map<int, std::wstring> active_spells;
  for (int i = 0; i < count; ++i)
  {
    // d(isPassive)d(lvl)d(SkillID:Get.Skill)c(c)
    // 104154FC "dddc"

    int passive, level, skillid;
    char unkc;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "dddc",
                        &passive,
                        &level,
                        &skillid,
                        &unkc);
    auto* data = GL2GameData.GetMSData(skillid, level);
    /*printf("skill id %d level %d passive %d unkc %d DATA %X\nname %ws manacost %d id %d range %d "
           "level %d\n",
           skillid,
           level,
           passive,
           unkc,
           data,
           data->skill_name,
           data->manacost,
           data->skill_id,
           data->range,
           data->skill_level);*/
    if (!passive)
      active_spells[data->skill_id] = (wchar_t*)data->skill_name;
    my_skills[data->skill_id] = data;
  }
  extern int sock;
  {
    std::array<char, 2048> to_send;
    binary_serializer bs(to_send);
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    bs << to_controller_protocol::spell_list << active_spells.size();
    for (const auto p : active_spells)
      bs << p.first << converter.to_bytes(p.second);
    // printf("total spell size %d\n", bs.get_current_size());
    auto total_size = bs.get_current_size();
    send(sock, (char*)&total_size, sizeof(total_size), 0);
    send(sock, to_send.data(), total_size, 0);
  }
  return realSkillList(unetwork_i_think, packet);
}

// 64=SystemMessage:d(MsgID:Get.MsgID)d(typesCount)
int(__cdecl* realSystemMessage)(void* unetwork_i_think, char packet[]);
int __cdecl mySystemMessage(void* unetwork_i_think, char packet[])
{
  struct SystemMessage
  {
    int msgid; // 612 means spoil activated
  };
  // 871	1	a,The seed has been sown.
  // 872	1	a,This seed may not be sown here.
  // 889	1	a,The seed was successfully sown.
  // 890	1	a, The seed was not sown.
  SystemMessage* data = (SystemMessage*)packet;

  User* my_target = game_data_facade::get_my_target();
  if (!my_target)
    return realSystemMessage(unetwork_i_think, packet);

  if (data->msgid == 612)
    my_target->get_apawn()->get_acontroller()->set_custom_sweepable(true);
  if (data->msgid == 871)
    my_target->get_apawn()->get_acontroller()->set_custom_manorable(true);
  if (data->msgid == 872)
    puts("CANNOT SOWN HERE");
  if (data->msgid == 889)
    my_target->get_apawn()->get_acontroller()->set_custom_manorable(true);
  if (data->msgid == 890)
    ; // not really needed i think
  // printf("SystemMessage obj %X\n", data->msgid);
  return realSystemMessage(unetwork_i_think, packet);
}

// 7F=MagicEffectIcons:h(ListCount:For.0003)d(skillID:Get.Skill)h(Lvl)d(Duration)
int(__cdecl* realMagicEffectIcons)(void* unetwork_i_think, char packet[]);
int __cdecl myMagicEffectIcons(void* unetwork_i_think, char packet[])
{
  short count = 0;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "h", &count);
  printf("MagicEffectIcons %d\n", count);
  player_character::get().buffs().clear();
  for (short i = 0; i < count; ++i)
  {
    int skillid, duration;
    short level;
    res = disasm_packet(
      (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "dhd", &skillid, &level, &duration);
    // auto* data = GL2GameData.GetMSData(skillid, level);
    printf("buff %d level %d duration %d\n", skillid, level, duration);
    player_character::get().buffs()[skillid] = (int)GetTickCount() / 1000 + duration;
  }
  return realMagicEffectIcons(unetwork_i_think, packet);
}

// A6=MyTargetSelected:d(objectID)h(color)
int(__cdecl* realMyTargetSelected)(void* unetwork_i_think, char packet[]);
int __cdecl myMyTargetSelected(void* unetwork_i_think, char packet[])
{
  struct MyTargetSelected
  {
    int objid;
  };
  MyTargetSelected* data = (MyTargetSelected*)packet;

  printf("MyTargetSelected obj %X\n", data->objid);
  return realMyTargetSelected(unetwork_i_think, packet);
}

// D6=RecipeBookItemList:d(isDwarven)d(max_MP)d(recipesCount:For.0002)d(recipeId)d(recipeNum)
int(__cdecl* realRecipeBookItemList)(void* unetwork_i_think, char packet[]);
int __cdecl myRecipeBookItemList(void* unetwork_i_think, char packet[])
{
  int isdwarven, maxmp, count;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "ddd", &isdwarven, &maxmp, &count);
  printf("RecipeBookItemList isdwarven %d maxmp %d count %d\n", isdwarven, maxmp, count);
  // int fake[] = {646, 648, 650};
  while (count--)
  {
    //*(int*)res = fake[count];
    int recipeid, num;
    res =
      disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "dd", &recipeid, &num);

    printf(
      "recipeid %X (%d) %ws num %d\n", recipeid, recipeid, GL2GameData.GetItemName(recipeid), num);
  }
  return realRecipeBookItemList(unetwork_i_think, packet);
}

// D7=RecipeItemMakeInfo:d(OID)d(isDwarvenRecipe)d(CurrentMP)d(MaxMP)d(isSuccess)
int(__cdecl* realRecipeItemMakeInfo)(void* unetwork_i_think, char packet[]);
int __cdecl myRecipeItemMakeInfo(void* unetwork_i_think, char packet[])
{
  int oid, isdwarven, currmp, maxmp, success;
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "ddddd",
                            &oid,
                            &isdwarven,
                            &currmp,
                            &maxmp,
                            &success);
  printf("RecipeItemMakeInfo oid %d isdwarven %d currmp %d maxmp %d success %d\n",
         oid,
         isdwarven,
         currmp,
         maxmp,
         success);
  return realRecipeItemMakeInfo(unetwork_i_think, packet);
}

// E8=BuyListSeed:d(money)d(ListID)h(ListCount:for.0007)h(itemType1)d(ObjectID)d(ItemID:Get.Func01)d(Count)h(itemType2)h(h)d(price)
int(__cdecl* realBuyListSeed)(void* unetwork_i_think, char packet[]);
int __cdecl myBuyListSeed(void* unetwork_i_think, char packet[])
{
  int money, listid;
  short count;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "ddh", &money, &listid, &count);
  printf("BuyListSeed money %d listid %d count %d\n", money, listid, count);
  while (count--)
  {
    short item_type, item_type2, unk;
    int objid, itemid, seedcount, price, unkd;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "hdddhh",
                        &item_type,
                        &objid,
                        &itemid,
                        &seedcount,
                        &item_type2,
                        &unk);
    if (item_type <= 3)
      res = disasm_packet(
        (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "dhhh", &unkd, &unk, &unk, &unk);
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "d", &price);
    printf("objid %X (%d) itemid %X (%d) %ws\n\tcount %d price %d 3unk %d %d %d\n",
           objid,
           objid,
           itemid,
           itemid,
           GL2GameData.GetItemName(itemid),
           seedcount,
           price,
           item_type,
           item_type2,
           unk);
  }
  return realBuyListSeed(unetwork_i_think, packet);
}

// E9=SellListProcure:d(money)d(d)h(ListCount:for.0007)h(itemType1)d(ObjectID)d(ItemID:Get.Func01)d(Count)h(itemType2)h(h)d(price)
int(__cdecl* realSellListProcure)(void* unetwork_i_think, char packet[]);
int __cdecl mySellListProcure(void* unetwork_i_think, char packet[])
{
  int money, listid;
  short listcount;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "ddh", &money, &listid, &listcount);
  printf("SellListProcure TODO money %d listid %d listcount %d\n", money, listid, listcount);
  return realSellListProcure(unetwork_i_think, packet);
}

// EE=PartySpelled:d(Summon)d(ObjecID)d(EffectCount:For.0003)d(SkillID:Get.Skill)h(Data)d(Duration)
int(__cdecl* realPartySpelled)(void* unetwork_i_think, char packet[]);
int __cdecl myPartySpelled(void* unetwork_i_think, char packet[])
{
  int summon, objid, count;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "ddd", &summon, &objid, &count);
  if (game_data_facade::get_my_id() != objid)
  {
    party::get().members()[objid].buffid_to_expiration_time.clear();
    printf("PartySpelled count %d OBJ %X\n", count, objid);
    for (int i = 0; i < count; ++i)
    {
      int skillid, duration;
      short level;
      res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                          res,
                          "dhd",
                          &skillid,
                          &level,
                          &duration);
      auto* data = GL2GameData.GetMSData(skillid, level);
      if (!data)
        printf("OMGZOR BUFF DATA FOR %d level %d is MISSING O__O\n", skillid, level);
      else
        printf("buff %ws (%d) level %d duration %d\n", data->skill_name, skillid, level, duration);
      party::get().members()[objid].buffid_to_expiration_time[skillid] =
        (int)GetTickCount() / 1000 + duration;
    }
    party::get().print();
  }
  return realPartySpelled(unetwork_i_think, packet);
}

// FE13=ExFishingStart:h(subID)d(charObjID)d(fishType)d(X)d(Y)d(Z)c(isNightLure)c(c)
int(__cdecl* realFishingStart)(void* unk, char packet[]);
int __cdecl myFishingStart(void* unk, char packet[])
{
  struct FishingStart
  {
    int objid;
    int fishtype;
    int x, y, z;
  };

  FishingStart* data = (FishingStart*)packet;
  if (data->objid == game_data_facade::get_my_id())
  {
    game_data_facade::get_fishing_state().fs = fishing::fishing_state::bait_set;
    printf("Fishing start obj %X finshtype %d xyz %d %d %d\n",
           data->objid,
           data->fishtype,
           data->x,
           data->y,
           data->z);
  }
  return realFishingStart(unk, packet);
}

// FE14=ExFishingEnd:h(subID)d(charObjectId)c(isWin)
int(__cdecl* realFishingEnd)(void* unk, char packet[]);
int __cdecl myFishingEnd(void* unk, char packet[])
{
  struct FishingEnd
  {
    int charid;
    bool iswin;
  };

  FishingEnd* data = (FishingEnd*)packet;
  if (data->charid == game_data_facade::get_my_id())
  {
    game_data_facade::get_fishing_state().fs = fishing::fishing_state::inactive;
    // printf("Fishing end %X win? %d\n", data->charid, data->iswin);
  }
  return realFishingEnd(unk, packet);
}

// FE15=ExFishingStartCombat:h(subID)d(charObjID)d(time)d(HP)c(Fighting)c(LureType)c(isFishDeceptive)
int(__cdecl* realFishingStartCombat)(void* unk, char packet[]);
int __cdecl myFishingStartCombat(void* unk, char packet[])
{
  struct FishingCombat
  {
    int objid;
    int time;
    int hp;
    char mode;
    char luretype;
    char deceptivemode;
  };

  FishingCombat* data = (FishingCombat*)packet;
  if (data->objid == game_data_facade::get_my_id())
  {
    game_data_facade::get_fishing_state().fs = fishing::fishing_state::fight;
    game_data_facade::get_fishing_state().should_reel = data->mode ? true : false;
    // printf("Fishing combat %X time %d mode %d\n", data->objid, data->time, data->mode);
  }
  return realFishingStartCombat(unk, packet);
}

// FE16=ExFishingHpRegen:h(subID)d(charObjID)d(time)d(fish_HP)c(HPstop/rise)c(GoodUse)c(anim)d(penalty)c(BarColor)
int(__cdecl* realFishingHPRegen)(void* unk, char packet[]);
int __cdecl myFishingHPRegen(void* unk, char packet[])
{
  struct FishingHpRegen
  {
    int objid;
    int time;
    int fishhp;
    char isHpRaising;
  };

  FishingHpRegen* data = (FishingHpRegen*)packet;
  if (data->objid == game_data_facade::get_my_id())
  {
    game_data_facade::get_fishing_state().fs = fishing::fishing_state::fight;
    game_data_facade::get_fishing_state().should_reel = data->isHpRaising ? true : false;
    // printf("Fishing regen %X time %d stopraise %d hp %d\n", data->objid, data->time,
    // data->isHpRaising, data->fishhp);
  }
  return realFishingHPRegen(unk, packet);
}

// FE1B=ExSendManorList:h(subID)d(ListCount:For.0002)d(idx)s(Name)
int(__cdecl* realExSendManorList)(void* unetwork_i_think, char packet[]);
int __cdecl myExSendManorList(void* unetwork_i_think, char packet[])
{
  int count;
  char* res =
    disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "d", &count);
  printf("ExSendManorList count %d\n", count);
  manor_crops::castle_names().clear();
  while (count--)
  {
    int id;
    wchar_t name[0x200];
    res = disasm_packet(
      (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), res, "dS", &id, 0x200, name);
    printf("\tid %X name %ws\n", id, name);
    manor_crops::castle_names()[id] = name;
  }
  return realExSendManorList(unetwork_i_think, packet);
}

// FE20=ExShowCropInfoPacket:h(subID)d(ItemID)d(ListCount:For.004)d(GOROD)d(Count)d(Price)c(c)
int(__cdecl* realExShowCropInfoPacket)(void* unetwork_i_think, char packet[]);
int __cdecl myExShowCropInfoPacket(void* unetwork_i_think, char packet[])
{
  manor_crops::expectatios().clear();
  char hidebuttons;
  int manorid, count, unkd;
  char* res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                            packet,
                            "cddd",
                            &hidebuttons,
                            &manorid,
                            &unkd,
                            &count);
  printf("ExShowCropInfoPacket manorid %d count %d\n", manorid, count);
  auto& reward_vector = manor_crops::crop_buy_data()[manorid];
  reward_vector.clear();
  while (count--)
  {
    int cropid, remaingng, buy, price, seedlevel, reward1id, reward2id;
    char reward, unkc;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "ddddcdcdcd",
                        &cropid,
                        &remaingng,
                        &buy,
                        &price,
                        &reward,
                        &seedlevel,
                        &unkc,
                        &reward1id,
                        &unkc,
                        &reward2id);
    if (buy == 0)
      continue;
    printf("\tcropid %d (%X) buyres %d buy %d price %d reward %d\n\tlevel %d rew1 %d (%X) rew2 "
           "%d (%X)\n",
           cropid,
           cropid,
           remaingng,
           buy,
           price,
           reward,
           seedlevel,
           reward1id,
           reward1id,
           reward2id,
           reward2id);
    if (remaingng == 0)
      continue;
    if (game_data_facade::get_items().count(cropid))
    {
      auto to_sell = min(game_data_facade::get_items().at(cropid).count, remaingng);
      printf("You got that crop in %d count! I will try to sell %d!\n",
             game_data_facade::get_items().at(cropid).count,
             to_sell);
      manor_crop to_add;
      to_add.castle = manorid;
      to_add.count = to_sell;
      to_add.itemid = cropid;
      to_add.objid = game_data_facade::get_items().at(cropid).objid;
      manor_crops::expectatios().emplace_back(to_add);
    }
    reward_vector.emplace_back(crop_info{
      reward == 1 ? reward1id : reward2id, price * remaingng, cropid, remaingng, manorid});
  }
  return realExShowCropInfoPacket(unetwork_i_think, packet);
}

// FE21=ManorList1:h(subID)d(d)d(ListCount:For.0012)d(ObjectID)d(itemID)d(d)d(d)c(c)c(c)d(d)d(Reward)d(BuyCount)d(BuyPrice)c(c)d(ItemCount)
int(__cdecl* realManorList1)(void* unetwork_i_think, char packet[]);
int __cdecl myManorList1(void* unetwork_i_think, char packet[])
{
  puts("ManorList1");
  return realManorList1(unetwork_i_think, packet);
}

// FE22=ManorList2:h(subID)d(ItemID)d(ListCount:For.004)d(GOROD)d(Count)d(Price)c(c)
int(__cdecl* realManorList2)(void* unetwork_i_think, char packet[]);
int __cdecl myManorList2(void* unetwork_i_think, char packet[])
{
  int itemid, count;
  char* res = disasm_packet(
    (void*)(*(int*)(((char*)unetwork_i_think) + 0x48)), packet, "dd", &itemid, &count);
  printf("ManorList2 itemid %d (%X) count %d\n", itemid, itemid, count);
  while (count--)
  {
    int gorod, cropcount, price;
    char unkc;
    res = disasm_packet((void*)(*(int*)(((char*)unetwork_i_think) + 0x48)),
                        res,
                        "dddc",
                        &gorod,
                        &cropcount,
                        &price,
                        &unkc);
    printf("\tgorod %d count %d price %d unkc %d\n", gorod, cropcount, price, unkc);
  }
  return realManorList2(unetwork_i_think, packet);
}

void
network_table_hook::install_hooks()
{
  int engineBase = (int)LoadLibraryA("Engine.dll");
  networkTableEntry* standardPackets = (networkTableEntry*)(engineBase + 0x757310);
  networkTableEntry* exPackets = (networkTableEntry*)(engineBase + 0x767610);

  realDie = hookHandler(0x6, myDie, standardPackets);
  realSpawnItem = hookHandler(0xB, mySpawnItem, standardPackets);
  realDropItem = hookHandler(0xC, myDropItem, standardPackets);
  realGetItem = hookHandler(0xD, myGetItem, standardPackets);
  realCharSelected = hookHandler(0x15, myCharSelected, standardPackets);
  realNpcInfo = hookHandler(0x16, myNpcInfo, standardPackets); // temp fun
  realItemListPacket = hookHandler(0x1B, myItemListPacket, standardPackets);
  realInventoryUpdate = hookHandler(0x27, myInventoryUpdate, standardPackets);
  realTargetSelected = hookHandler(0x29, myTargetSelected, standardPackets);
  realTargetUnselected = hookHandler(0x2A, myTargetUnselected, standardPackets);

  realAskJoinParty = hookHandler(0x39, myAskJoinParty, standardPackets);
  realJoinParty = hookHandler(0x3A, myJoinParty, standardPackets);
  realWithdrawalParty = hookHandler(0x3B, myWithdrawalParty, standardPackets);
  realOustPartyMember = hookHandler(0x3C, myOustPartyMember, standardPackets);
  realSetOustPartyMember = hookHandler(0x3D, mySetOustPartyMember, standardPackets);
  realDismissParty = hookHandler(0x3E, myDismissParty, standardPackets);
  realSetDismissParty = hookHandler(0x3F, mySetDismissParty, standardPackets);

  realMagicSkillUse = hookHandler(0x48, myMagicSkillUse, standardPackets);
  realMagicSkillCanceled = hookHandler(0x49, myMagicSkillCanceled, standardPackets);
  realMagicSkillLaunched = hookHandler(0x76, myMagicSkillLaunched, standardPackets);

  realSay2 = hookHandler(0x4A, mySay2, standardPackets);

  realPartySmallWindowAll = hookHandler(0x4E, myPartySmallWindowAll, standardPackets);
  realPartySmallWindowAdd = hookHandler(0x4F, myPartySmallWindowAdd, standardPackets);

  realPartySmallWindowDeleteAll = hookHandler(0x50, myPartySmallWindowDeleteAll, standardPackets);
  realPartySmallWindowDelete = hookHandler(0x51, myPartySmallWindowDelete, standardPackets);
  realPartySmallWindowUpdate = hookHandler(0x52, myPartySmallWindowUpdate, standardPackets);
  realPartyMemberPosition = hookHandler(0xA7, myPartyMemberPosition, standardPackets);

  realSkillList = hookHandler(0x58, mySkillList, standardPackets);
  realSystemMessage = hookHandler(0x64, mySystemMessage, standardPackets);
  realMagicEffectIcons = hookHandler(0x7F, myMagicEffectIcons, standardPackets);
  realMyTargetSelected = hookHandler(0xA6, myMyTargetSelected, standardPackets);
  realPartySpelled = hookHandler(0xEE, myPartySpelled, standardPackets);

  realBuyListSeed = hookHandler(0xE8, myBuyListSeed, standardPackets);
  realSellListProcure = hookHandler(0xE9, mySellListProcure, standardPackets);

  realRecipeBookItemList = hookHandler(0xD6, myRecipeBookItemList, standardPackets);
  realRecipeItemMakeInfo = hookHandler(0xD7, myRecipeItemMakeInfo, standardPackets);

  // expackets
  realExSendManorList = hookHandler(0x1B, myExSendManorList, exPackets);
  // realExShowCropSettingPacket = hookHandler(0x20, myExShowCropSettingPacket, exPackets);
  realExShowCropInfoPacket = hookHandler(0x1D, myExShowCropInfoPacket, exPackets);

  realManorList1 = hookHandler(0x21, myManorList1, exPackets);
  realManorList2 = hookHandler(0x22, myManorList2, exPackets);

  realFishingStart = hookHandler(0x13, myFishingStart, exPackets);
  realFishingEnd = hookHandler(0x14, myFishingEnd, exPackets);
  realFishingStartCombat = hookHandler(0x15, myFishingStartCombat, exPackets);
  realFishingHPRegen = hookHandler(0x16, myFishingHPRegen, exPackets);

  disasm_packet = (char*(__cdecl*)(void*, char*, const char*, ...))(engineBase + 0x34E5);

  puts("Hello network_table_hooks");
}
