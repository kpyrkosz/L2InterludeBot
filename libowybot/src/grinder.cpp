#include <Windows.h>
#include <action_executor.hpp>
#include <alarm_module.hpp>
#include <algorithm>
#include <antistuck_module.hpp>
#include <autoattack_module.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <craft_module.hpp>
#include <fishing_module.hpp>
#include <game_data_facade.hpp>
#include <grinder.hpp>
#include <item_use_module.hpp>
#include <manor_crops.hpp>
#include <movement_module.hpp>
#include <party_buff_guard.hpp>
#include <picking_module.hpp>
#include <spell_casting_module.hpp>
#include <targeting_module.hpp>

void
grinder::grind_the_livin_shit_out_of_the_game()
{
  // take it easy
  static auto next_action_timer = GetTickCount();
  if (next_action_timer > GetTickCount())
    return; // mini antispam
  auto& bot_config = get_bot_settings();
  // temp crops..

  if (bot_config.whisper_test)
  {
    bot_config.whisper_test = false;
    action_executor::whisper(L"no hej master", L"justrunning");
    return;
  }
  if (bot_config.manor_buy_test)
  {
    /*
    objid 0 (0) itemid 2021 (8225) Seed: Desert Codr
  count 50 price 1000 3unk 4 5 0
objid 0 (0) itemid 2023 (8227) Seed: Red Coba
  count 50 price 5000 3unk 4 5 0
objid 0 (0) itemid 2024 (8228) Seed: Golden Coba
  count 50 price 6500 3unk 4 5 0
objid 0 (0) itemid 2025 (8229) Seed: Desert Coba
  count 50 price 7500 3unk 4 5 0
objid 0 (0) itemid 2149 (8521) Seed: Sea Coba
  count 50 price 9000 3unk 4 5 0
objid 0 (0) itemid 214B (8523) Seed: Twin Coba
  count 50 price 9000 3unk 4 5 0
objid 0 (0) itemid 214D (8525) Seed: Great Coba
  count 50 price 12000 3unk 4 5 0
  */
    std::vector<manor_buy_seed> seeds{
      {8225, 10}, {8227, 10}, {8228, 10}, {8229, 10}, {8521, 10}, {8523, 10}, {8525, 10}};
    printf("Executing manor BUY count %d\n", seeds.size());
    action_executor::buy_seeds(8, seeds); // 3 - giran, 2 - dion 8 rune
    next_action_timer = GetTickCount() + 88;
    return;
  }
  // objid 405B6341 itemid 13DF count 771 isequip 0 name Blue Codran
  if (bot_config.manor_test)
  {
    std::vector<manor_crop>& crops = manor_crops::expectatios();
    // crops.emplace_back(manor_crop{game_data_facade::get_items().at(5088).objid, 5088, 3, 20});
    // crops.emplace_back(manor_crop{game_data_facade::get_items().at(5087).objid, 5087, 3, 20});
    // crops.emplace_back(manor_crop{game_data_facade::get_items().at(5084).objid, 5084, 4, 8});
    /*
    name	id		loc		sztuk		cena
twin	5092	oren 	800			1000
red		5088	oren	1000		1000
chilly	5085	oren	1000		1000
*/
    if (crops.empty())
    {
      puts("Expectations met!");
      bot_config.manor_test = false;
      return;
    }

    for (const auto& elem : crops)
    {
      if (game_data_facade::get_items().count(elem.itemid) == 0)
      {
        printf("MANOR CRITICAL ERROR: item %d (%X) is set but you dont have it in inventory!",
               elem.itemid,
               elem.itemid);
        bot_config.manor_test = false;
        return;
      }
      if (game_data_facade::get_items().at(elem.itemid).count < elem.count)
      {
        printf("MANOR CRITICAL ERROR: item %d (%X) is set to %d units but you only have %d!",
               elem.itemid,
               elem.itemid,
               elem.count,
               game_data_facade::get_items().at(elem.itemid).count);
        bot_config.manor_test = false;
        return;
      }
    }

    printf("Executing manor count %d\n", crops.size());
    action_executor::sell_crops(crops);

    next_action_timer = GetTickCount() + 33; // tweak even lower, maybe resend status packet?
    return;
  }
  // po temp
  // first, check if bot is active and if we are in world
  if (!bot_config.is_active || !game_data_facade::is_in_world())
    return;

#ifdef NDEBUG
  static bool wasqq = false;
  if (!wasqq)
  {
    wasqq = true;
    SYSTEMTIME st;
    GetSystemTime(&st);
    // time bomb
    if (st.wYear != 2020 || st.wMonth > 6)
      __asm jmp UNetworkHandler::Action
  }
#endif

  auto curr_intention = get_penalties().current_intention;
  static constexpr const char* intentions[] = {"idle",
                                               "combat",
                                               "picking",
                                               "recharging",
                                               "want_to_rest",
                                               "resting",
                                               "buffing",
                                               "buffing_oop",
                                               "casting",
                                               "DUPA"};
  // printf("CURRENT INTENTION %s\n", intentions[(char)curr_intention]);
  fishing_module::perform_action();
  antistuck_module::perform_action();
  picking_module::perform_action();
  craft_module::perform_action();
  party_buff_guard::perform_action();
  item_use_module::perform_action();
  movement_module::perform_action();
  targeting_module::perform_action();
  autoattack_module::perform_action();
  spell_casting_module::perform_action();
  alarm_module::perform_action();

  // nothing this tick
  next_action_timer = GetTickCount() + 111;
}
