#include <WinSock2.h>
#include <Windows.h>
#include <binary_deserializer.hpp>
#include <binary_serializer.hpp>
#include <bot_confiuration.hpp>
#include <configuration_setter.hpp>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <game_data_facade.hpp>
#include <manor_crops.hpp>
#include <party.hpp>
#include <protocol.hpp>
#include <stdio.h>
#include <unordered_set>
#include <ws2tcpip.h>

// temp print recipe
void
print_rec(FL2RecipeData& rec)
{
  printf("MEM %X recid %d itemid %d lvl %d resultid %d\n"
         "resultc %d mp %d\n chance %d matcount %d\n%ws\n%ws\n",
         &rec,
         rec.recipe_id,
         rec.recipe_item_id,
         rec.required_level_of_craft_skill,
         rec.resulting_item_id,
         rec.resulting_itemcount,
         rec.manacost,
         rec.chance,
         rec.material_count,
         GL2GameData.GetItemName(rec.resulting_item_id),
         GL2GameData.GetItemName(rec.recipe_item_id));
  for (int i = 0; i < rec.req.mat_count; ++i)
  {
    single_material_requirement* req = rec.req.material_array[i];
    printf("\t%ws %d\n", GL2GameData.GetItemName(req->item_id), req->count);
  }
};

// xDD learning from the best
int sock = 0;
int g_ip = 0;
short g_port = 0;
void
network_controller()
{
#ifdef NDEBUG
  SYSTEMTIME st;
  GetSystemTime(&st);
  // time bomb
  if (st.wYear != 2020 || st.wMonth > 6)
    return;
#endif
  puts("Hello network_controller");
  WSADATA wsaData;
  int iResult;

  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
    printf("WSAStartup failed with error: %d\n", iResult);
    return;
  }
  printf("Attemtping connection to addr %X port %d\n", g_ip, g_port);

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int flag = 1;
  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int)))
  {
    printf("setsockopt failed %d\n", GetLastError());
    return;
  }
  if (!sock)
  {
    printf("socket failed %d\n", GetLastError());
    return;
  }
  SOCKADDR_IN sockadr;
  sockadr.sin_port = htons(g_port);
  sockadr.sin_family = AF_INET;
  sockadr.sin_addr.S_un.S_addr = htonl(g_ip);
  iResult = connect(sock, (sockaddr*)&sockadr, sizeof(sockadr));
  if (iResult != 0)
  {
    printf("connect failed with error: %d\n", GetLastError());
    return;
  }
  puts("Connected!");

  {
    // send test broadcast
    std::array<char, 2048> to_send;
    binary_serializer bs(to_send);
    bs << to_controller_protocol::broadcast << broadcast_protocol::log_string
       << std::string("test broadcast!");
    auto total_size = bs.get_current_size();
    send(sock, (char*)&total_size, sizeof(total_size), 0);
    send(sock, to_send.data(), total_size, 0);
  }
  while (1)
  {
    int recv_size = 0;
    // read header length
    uint32_t message_length;
    while (recv_size < 4)
    {
      auto recv_result =
        recv(sock, ((char*)&message_length) + recv_size, sizeof(uint32_t) - recv_size, 0);
      if (recv_result <= 0)
      {
        printf("critical error, recv returned %d, error %d\n", recv_result, GetLastError());
        closesocket(sock);
        return;
      }
      recv_size += recv_result;
    }
    if (message_length == 0)
    {
      puts("dummy msg");
      continue;
    }
    if (message_length >= 2048)
    {
      printf("critical error, msg len expected: %d\n", message_length);
      closesocket(sock);
      return;
    }
    recv_size = 0;
    char recvbuf[2048] = {};
    while (recv_size < message_length)
    {
      auto recv_result = recv(sock, recvbuf + recv_size, message_length - recv_size, 0);
      if (recv_result <= 0)
      {
        printf("critical error, recv returned %d, error %d\n", recv_result, GetLastError());
        closesocket(sock);
        return;
      }
      recv_size += recv_result;
    }
    std::string data;
    data.resize(2048);
    printf("recvsize %d msglen %d\n", recv_size, message_length);
    {
      CryptoPP::SecByteBlock key(
        (byte*)"\x23\x5C\x58\x6A\x85\x2F\xE3\xD6\x5D\x10\xF0\x31\x24\x09\x61\x10",
        CryptoPP::AES::DEFAULT_KEYLENGTH);
      CryptoPP::SecByteBlock iv(
        (byte*)"\x93\x96\xA4\xF4\x44\x66\x3A\xAB\xE7\x49\x8A\x45\xD4\x3B\xEC\x73",
        CryptoPP::AES::BLOCKSIZE);
      CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(key, key.size(), iv);
      cfbDecryption.ProcessData((byte*)data.data(), (byte*)recvbuf, recv_size);
    }
    binary_deserializer bd(data.data(), recv_size);
    to_bot_protocol opcode;
    bd >> opcode;
    // printf("opcode %d\n", opcode);
    switch (opcode)
    {
      case to_bot_protocol::log_string:
        printf("[MSG]\t%s\n", recvbuf + 1);
        break;
      case to_bot_protocol::pause_bot:
      {
        bool new_state;
        bd >> new_state;
        printf("[PAUSE]\t%d\n", new_state);
        get_bot_settings().is_active = new_state;
        break;
      }
      case to_bot_protocol::target:
      {
        std::string target_name;
        bd >> get_bot_settings().target >> target_name >> get_bot_settings().prioritize >>
          get_bot_settings().assist_wait >> get_bot_settings().retarget;
        get_bot_settings().target_name = std::wstring(begin(target_name), end(target_name));
        printf("[TARGET] %d %ws %d %d %d\n",
               get_bot_settings().target,
               get_bot_settings().target_name.c_str(),
               get_bot_settings().prioritize,
               get_bot_settings().assist_wait,
               get_bot_settings().retarget);
        break;
      }
      case to_bot_protocol::farming:
      {
        bd >> get_bot_settings().player_to_follow >> get_bot_settings().autoattack >>
          get_bot_settings().spoil >> get_bot_settings().follow_player >>
          get_bot_settings().movecloser >> get_bot_settings().DEPRfakemov >>
          get_bot_settings().DEPRpick >> get_bot_settings().DEPRpick_far_away >>
          get_bot_settings().DEPRpick_only_mine >> get_bot_settings().DEPRpick_in_box >>
          get_bot_settings().DEPRexperimental_recharge >> get_bot_settings().DEPRpick_close >>
          get_bot_settings().DEPRprioritize_mana_herbs >> get_bot_settings().DEPR >>
          get_bot_settings().sweep;
        printf("[FARMING] %s\n", get_bot_settings().player_to_follow.c_str());
        break;
      }
      case to_bot_protocol::bounding_box:
      {
        get_bot_settings().bounding_box.clear();
        get_bot_settings().antistuck_points.clear();
        int vert_count, antistuck_count;
        bd >> vert_count;
        for (char i = 0; i < vert_count; ++i)
        {
          int x, y;
          bd >> x >> y;
          printf("Vert %d %d\n", x, y);
          get_bot_settings().bounding_box.emplace_back(std::pair<int, int>{x, y});
        }
        bd >> antistuck_count;
        for (char i = 0; i < antistuck_count; ++i)
        {
          int x, y;
          bd >> x >> y;
          printf("Antistuck %d %d\n", x, y);
          get_bot_settings().antistuck_points.emplace_back(std::pair<int, int>{x, y});
        }
        printf("[BOUNDING BOX] %d %d\n", vert_count, antistuck_count);
        break;
      }
      case to_bot_protocol::spells:
      {
        int count;
        bd >> count;
        printf("[SPELLS] %d\n", count);
        get_bot_settings().bot_spells.clear();
        for (int i = 0; i < count; ++i)
        {
          int id, cooldown;
          class_condition_type cond_type;
          int cond_value;
          spell_target target;
          std::string nick_opt{"NONE"};
          bd >> id >> cooldown >> cond_type >> cond_value >> target;
          if (target == spell_target::nick)
            bd >> nick_opt;
          printf("Spell %d cd %d %d %d %d nick_opt %s\n",
                 id,
                 cooldown,
                 cond_type,
                 cond_value,
                 target,
                 nick_opt.c_str());
          get_bot_settings().bot_spells.emplace_back(
            bot_spell{id, cooldown, cond_type, cond_value, target, nick_opt, 0});
        }
        break;
      }
      case to_bot_protocol::items:
      {
        int count;
        bd >> count;
        printf("[ITEMS] %d\n", count);
        get_bot_settings().bot_items.clear();
        for (int i = 0; i < count; ++i)
        {
          int id, cooldown;
          class_condition_type cond_type;
          int cond_value;
          bd >> id >> cooldown >> cond_type >> cond_value;
          printf("Item %d cd %d %d %d\n", id, cooldown, cond_type, cond_value);
          get_bot_settings().bot_items.emplace_back(
            bot_item{id, cooldown, cond_type, cond_value, 0});
        }
        break;
      }
      case to_bot_protocol::buffs:
      {
        bd >> get_bot_settings().party_inviter >> get_bot_settings().acceptparty;
        int count;
        bd >> count;
        printf("[BUFFS] %d\n", count);
        get_bot_settings().party_buffs.clear();
        for (int i = 0; i < count; ++i)
        {
          int id, buff_before;
          bool is_targetless;
          std::string name;
          bd >> id >> name >> buff_before >> is_targetless;
          printf("Buff %d name %s before %d\n", id, name.c_str(), buff_before);
          get_bot_settings().party_buffs[std::wstring{name.begin(), name.end()}].push_back(
            buff{id, buff_before, is_targetless});
        }
        break;
      }
      case to_bot_protocol::manor:
      {
        int count;
        bd >> count;
        printf("[MANOR] %d\n", count);
        get_bot_settings().manor_seeds.clear();
        for (int i = 0; i < count; ++i)
        {
          bot_manor to_add;
          bd >> to_add.seed_itemid >> to_add.expected_count >> to_add.crop_itemid;
          printf("Manor id %d count %d crop %d name %ws namecrop %ws\n",
                 to_add.seed_itemid,
                 to_add.expected_count,
                 to_add.crop_itemid,
                 GL2GameData.GetItemName(to_add.seed_itemid),
                 GL2GameData.GetItemName(to_add.crop_itemid));
          get_bot_settings().manor_seeds.emplace_back(std::move(to_add));
        }
        break;
      }
      case to_bot_protocol::request_your_position:
      {
        puts("[REQUEST POS]");
        auto& my_xyz = game_data_facade::get_my_user().get_xyz();

        extern int sock;
        {
          std::array<char, 2048> to_send;
          binary_serializer bs(to_send);
          bs << to_controller_protocol::char_position << (int)my_xyz.x << (int)my_xyz.y
             << (int)my_xyz.z;
          auto total_size = bs.get_current_size();
          send(sock, (char*)&total_size, sizeof(total_size), 0);
          send(sock, to_send.data(), total_size, 0);
        }
        break;
      }
      case to_bot_protocol::craft:
      {
        int count;
        bd >> count;
        printf("[CRAFT] %d\n", count);
        get_bot_settings().craft.clear();
        for (int i = 0; i < count; ++i)
        {
          bot_craft to_add;
          bd >> to_add.recipe_id >> to_add.count >> to_add.recursive;
          auto rec_ptr = GL2GameData.GetRecipeDataByIndex(to_add.recipe_id);
          if (!rec_ptr)
          {
            printf("Recipe %d is not recognized by game!!\n", to_add.recipe_id);
            continue;
          }
          print_rec(*rec_ptr);
          get_bot_settings().craft.emplace_back(std::move(to_add));
        }
        break;
      }
      case to_bot_protocol::dos:
      {
        int count;
        bd >> count;
        printf("[DOS N DONTS] %d\n", count);

        get_bot_settings().target_blacklist.clear();
        get_bot_settings().attack_blacklist.clear();
        get_bot_settings().manor_blacklist.clear();
        get_bot_settings().spoil_blacklist.clear();
        for (int i = 0; i < count; ++i)
        {
          bot_rule to_add;
          bd >> to_add.brw >> to_add.is_include >> to_add.mob_id;
          printf("BOT RULE %d %d %d\n", to_add.brw, to_add.is_include, to_add.mob_id);
          switch (to_add.brw)
          {
            case bot_rule_what::target:
              if (to_add.is_include)
                get_bot_settings().target_blacklist.add_as_exclusive(to_add.mob_id);
              else
                get_bot_settings().target_blacklist.add_as_ignored(to_add.mob_id);
              break;
            case bot_rule_what::attack:
              if (to_add.is_include)
                get_bot_settings().attack_blacklist.add_as_exclusive(to_add.mob_id);
              else
                get_bot_settings().attack_blacklist.add_as_ignored(to_add.mob_id);
              break;
            case bot_rule_what::manor:
              if (to_add.is_include)
                get_bot_settings().manor_blacklist.add_as_exclusive(to_add.mob_id);
              else
                get_bot_settings().manor_blacklist.add_as_ignored(to_add.mob_id);
              break;
            case bot_rule_what::spoil:
              if (to_add.is_include)
                get_bot_settings().spoil_blacklist.add_as_exclusive(to_add.mob_id);
              else
                get_bot_settings().spoil_blacklist.add_as_ignored(to_add.mob_id);
              break;
          }
        }
        break;
      }
      case to_bot_protocol::broadcast:
      {
        broadcast_protocol bp;
        bd >> bp;
        if (bp == broadcast_protocol::log_string)
        {
          std::string msg;
          bd >> msg;
          printf("[BROADCAST MSG] %s\n", msg.c_str());
        }
        else
        {
          puts("[BROADCAST UNK]");
        }
        break;
      }
      case to_bot_protocol::relax:
      {
        puts("[RELAX NOT YET]");
        break;
      }
      case to_bot_protocol::tricks:
      {
        puts("[TRICKS]");
        bd >> get_bot_settings().tricks_cfg.remove_trained_beastfarm_animals >>
          get_bot_settings().tricks_cfg.turn_cancel_animation_visible >>
          get_bot_settings().tricks_cfg.dont_show_droped_arrows;
        break;
      }
      case to_bot_protocol::alarm:
      {
        std::string spawn_name;
        bd >> get_bot_settings().alarm_cfg.check_for_spawn >> spawn_name >>
          get_bot_settings().alarm_cfg.on_enter_bounding_box;
        get_bot_settings().alarm_cfg.spawn_name =
          std::wstring(spawn_name.begin(), spawn_name.end());
        printf("[ALARM] %s", spawn_name.c_str());
        break;
      }
      case to_bot_protocol::follow:
      {
        std::string player_name;
        bd >> player_name >> get_bot_settings().follow_cfg.follow_attack_like >>
          get_bot_settings().follow_cfg.keep_distance >> get_bot_settings().follow_cfg.dist_min >>
          get_bot_settings().follow_cfg.dist_max;
        get_bot_settings().follow_cfg.player_name =
          std::wstring(player_name.begin(), player_name.end());
        printf("[FOLLOW] %s", player_name.c_str());
        break;
      }
      case to_bot_protocol::pick:
      {
        puts("[PICK]");
        bd >> get_bot_settings().pick_cfg.picking_on >> get_bot_settings().pick_cfg.non_greedy >>
          get_bot_settings().pick_cfg.out_of_fight >> get_bot_settings().pick_cfg.limit_to_box >>
          get_bot_settings().pick_cfg.manaherb_priority;
        break;
      }
      case to_bot_protocol::out_of_party:
      {
        int count;
        bd >> count;
        printf("[OUT OF PARTY] %d\n", count);
        get_bot_settings().oops.clear();
        for (int i = 0; i < count; ++i)
        {
          std::string playername;
          bot_oop_buff entry;
          bd >> entry.spellid >> playername >> entry.buff_before >> entry.is_targetless >>
            entry.requires_invite;
          entry.playername = std::wstring(playername.begin(), playername.end());
          printf("OOP %d name %s before %d tar %d inv %d\n",
                 entry.spellid,
                 playername.c_str(),
                 entry.buff_before,
                 entry.is_targetless,
                 entry.requires_invite);
          get_bot_settings().oops.emplace_back(std::move(entry));
        }
        break;
      }
      case to_bot_protocol::beastfarm:
      {
        int count;
        bd >> get_bot_settings().beastfarm_cfg.prefer_buffalo >>
          get_bot_settings().beastfarm_cfg.prefer_kooka >>
          get_bot_settings().beastfarm_cfg.prefer_cougar >> count;

        printf("[BEASTFARM] %d\n", count);
        get_bot_settings().beastfarm_cfg.high_priority.clear();
        get_bot_settings().beastfarm_cfg.low_priority.clear();
        for (int i = 0; i < count; ++i)
        {
          int id;
          bool high_priority;
          bd >> id >> high_priority;
          printf("id %d priority %d\n", id, high_priority);
          high_priority ? get_bot_settings().beastfarm_cfg.high_priority.emplace_back(id)
                        : get_bot_settings().beastfarm_cfg.low_priority.emplace_back(id);
        }
        break;
      }
      default:
        printf("Unknown opcode %d", opcode);
        break;
    }
  }
}

void
controller()
{
  puts("Hello controller");

  auto KeyPressed = [](int key) -> bool {
    if (!(GetAsyncKeyState(key) & 0x8000) || !(GetAsyncKeyState(VK_CONTROL) & 0x8000))
      return false;
    DWORD pid;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
  };
  while (1)
  {
    auto& bot_settings = get_bot_settings();
    if (KeyPressed(VK_F2))
    {
      extern bool temp_block_enterworld;
      temp_block_enterworld = !temp_block_enterworld;
      printf("temp_block_enterworld %d\n", temp_block_enterworld);
      Sleep(111);
    }
    if (KeyPressed(VK_F3))
    {
      auto nearby_guys =
        game_data_facade::get_users([](const User& the_guy) { return the_guy.is_player(); });
      for (const auto& g : nearby_guys)
        printf(
          "Guy %ws XYZ %f %f %f\n", g->GetName(), g->get_xyz().x, g->get_xyz().y, g->get_xyz().z);
    }
    if (KeyPressed(VK_F4))
    {
      bot_settings.target_guards_for_delevel_test = !bot_settings.target_guards_for_delevel_test;
      printf("target_guards_for_delevel_test %d\n", bot_settings.target_guards_for_delevel_test);
      Sleep(111);
    }
    if (KeyPressed(VK_F5))
    {
      bot_settings.fishing_test = !bot_settings.fishing_test;
      printf("fishing_test %d\n", bot_settings.fishing_test);
      Sleep(111);
    }
    if (KeyPressed(VK_F12))
    {
      std::unique_ptr<FILE, decltype(&fclose)> fileout(
        fopen("recipes", "w"),
        &fclose); // cause i just LOVE printf and raii
      for (int i = 0; i < 1111; ++i)
      {
        auto* recipe_ptr = GL2GameData.GetRecipeDataByIndex(i);
        if (!recipe_ptr)
          continue;
        fprintf(fileout.get(),
                "%d %ws\n",
                recipe_ptr->recipe_id,
                GL2GameData.GetItemName(recipe_ptr->recipe_item_id));
      }
      Sleep(111);
    }
    if (KeyPressed(VK_F6))
    {
      bot_settings.whisper_test = !bot_settings.whisper_test;
      printf("whisper_test %d\n", bot_settings.whisper_test);
      Sleep(111);
    }
    if (KeyPressed(VK_F8))
    {
      std::unordered_map<int, int> itemid_to_total_price;
      std::unordered_map<int, int> required_seeds;
      struct castle_count_itemid
      {
        int castle;
        int count;
        int reward_itemid;
      };
      std::unordered_map<int, std::vector<castle_count_itemid>> crop_to_castle;
      std::unordered_set<int> rewards_that_interest_me{
        1894, 1884, 1882, 1881, 1880, 1872, 1870, 1867, 4041, 4040, 4039};
      for (const auto& manor_pair : manor_crops::crop_buy_data())
      {
        auto& reward_vector = manor_pair.second;
        for (const auto& reward : reward_vector)
        {
          if (rewards_that_interest_me.count(reward.reward_item_id) == 0)
            continue;
          itemid_to_total_price[reward.reward_item_id] += reward.reward_total_price;
          required_seeds[reward.crop_id] += reward.cropcount;
          crop_to_castle[reward.crop_id].emplace_back(
            castle_count_itemid{reward.castleid, reward.cropcount, reward.reward_item_id});
        }
      }
      puts("------ MANOROWA STATYSTYKA DLA BOCIARZA ------");
      puts("------ REWARDS ------");
      for (const auto& qq : itemid_to_total_price)
      {
        printf("%ws total price %d\n", GL2GameData.GetItemName(qq.first), qq.second);
      }
      puts("------ REQUIRED CROPS ------");
      for (const auto& qq : required_seeds)
      {
        if (crop_to_castle.at(qq.first).size() == 1)
        {
          printf("%ws count %d to castle %ws for %ws\n",
                 GL2GameData.GetItemName(qq.first),
                 qq.second,
                 manor_crops::castle_names()[crop_to_castle.at(qq.first).begin()->castle].c_str(),
                 GL2GameData.GetItemName(crop_to_castle.at(qq.first).begin()->reward_itemid));
          continue;
        }
        printf("%ws count %d\n", GL2GameData.GetItemName(qq.first), qq.second);
        for (const auto& kurwa_leb_mnie_boli : crop_to_castle.at(qq.first))
        {
          printf("\t%d to castle %ws for %ws\n",
                 kurwa_leb_mnie_boli.count,
                 manor_crops::castle_names()[kurwa_leb_mnie_boli.castle].c_str(),
                 GL2GameData.GetItemName(kurwa_leb_mnie_boli.reward_itemid));
        }
      }
      Sleep(111);
    }
    if (KeyPressed(VK_F9))
    {
      bot_settings.manor_buy_test = !bot_settings.manor_buy_test;
      printf("SETTING TEST BUY MANOR %d\n", bot_settings.manor_buy_test);
      Sleep(111);
    }
    if (KeyPressed(VK_F7))
    {
      bot_settings.manor_test = !bot_settings.manor_test;
      printf("SETTING MANOR: %d\n", bot_settings.manor_test);
      Sleep(111);
      /*const auto& xyz = game_data_facade::get_hp_mp().get_xyz();
      printf("MY XYZ %f %f %f\n", xyz.x, xyz.y, xyz.z);
      User* qq = game_data_facade::get_by_id(game_data_facade::get_my_user().get_target_id());
      if (!qq)
        puts("Dupa qq");
      else
        printf("TARGET XYZ %f %f %f\n", qq->get_xyz().x, qq->get_xyz().y, qq->get_xyz().z);
      puts("PARTY");
      party::get().members();
      puts("PO");*/

      /*auto items = game_data_facade::get_dropped_items();
      for (const auto& elem : items)
      {
        printf("%X %f %f %f %f\n", elem.objid, elem.x, elem.y, elem.z, elem.distance);
      }
      putchar('\n');
      auto qq = game_data_facade::get_monsters();
      for (const auto& elem : qq)
      {
        printf("%X %f %f %f %f\n", elem.objid, elem.x, elem.y, elem.z, elem.distance);
      }
      putchar('\n');*/
    }
    Sleep(50);
  }
}

void
configuration_setter::start_listening(int ip, short port)
{
  g_ip = ip;
  g_port = port;
#ifndef NDEBUG
  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)controller, 0, 0, 0);
#endif
  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)network_controller, 0, 0, 0);
}
