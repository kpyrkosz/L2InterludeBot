#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>
#include <item_use_module.hpp>
#include <windows.h>

bool
item_use_module::perform_action()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.items))
    return false;
  int current_timestamp = GetTickCount();
  // std::unordered_map<int, item_data>& get_items();
  for (auto& item : get_bot_settings().bot_items)
  {
    // check if the cooldown passed
    if (item.last_use_timestamp + item.cooldown_ms > current_timestamp)
      continue;

    // check, if we have the item in inventory
    if (game_data_facade::get_items().count(item.template_id) == 0)
      continue;

    int objid = game_data_facade::get_items().at(item.template_id).objid;
    printf("Bot item template %d has uid %X\n", item.template_id, objid);
    // check condition
    switch (item.cond_type)
    {
      case class_condition_type::always:
        item.last_use_timestamp = current_timestamp;
        action_executor::use_item(objid);
        printf("Using %X\n", objid);
        update_penalty(penalties.items, 777);
        return true;
      case class_condition_type::hpgt:
        if (game_data_facade::get_my_user().hp * 100 / game_data_facade::get_my_user().maxhp >
            item.cond_val)
        {
          item.last_use_timestamp = current_timestamp;
          action_executor::use_item(objid);
          printf("Using %X\n", objid);
          update_penalty(penalties.items, 777);
          return true;
        }
        break;
      case class_condition_type::hplt:
        if (game_data_facade::get_my_user().hp * 100 / game_data_facade::get_my_user().maxhp <
            item.cond_val)
        {
          item.last_use_timestamp = current_timestamp;
          action_executor::use_item(objid);
          printf("Using %X\n", objid);
          update_penalty(penalties.items, 777);
          return true;
        }
        break;
      case class_condition_type::managt:
        if (game_data_facade::get_my_user().mp * 100 / game_data_facade::get_my_user().maxmp >
            item.cond_val)
        {
          item.last_use_timestamp = current_timestamp;
          action_executor::use_item(objid);
          printf("Using %X\n", objid);
          update_penalty(penalties.items, 777);
          return true;
        }
        break;
      case class_condition_type::manalt:
        if (game_data_facade::get_my_user().mp * 100 / game_data_facade::get_my_user().maxmp <
            item.cond_val)
        {
          item.last_use_timestamp = current_timestamp;
          action_executor::use_item(objid);
          printf("Using %X\n", objid);
          update_penalty(penalties.items, 777);
          return true;
        }
        break;
    }
  }
  update_penalty(penalties.items, 1234);
  return false;
}
