#include <Windows.h>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>

FObjectMap*(__fastcall* find_by_uid_ptr)(FArray* monster_array, int fastcallqq, int* uid) =
  (FObjectMap * (__fastcall*)(FArray*, int, int*))((DWORD)GetModuleHandleA("engine.dll") + 0xD04E);

int
game_data_facade::get_my_id()
{
  int* my_id = (int*)((DWORD)GetModuleHandleA("engine.dll") + 0x81F530);
  return *my_id;
}

User&
game_data_facade::get_my_user()
{
  return *get_by_id(get_my_id());
}

User*
game_data_facade::get_my_target()
{
  User* me = get_by_id(get_my_id());
  if (!me)
    return nullptr;
  return get_by_id(me->get_target_id());
}

User*
game_data_facade::get_by_id(int objid)
{
  return UNetworkHandler::get().UNetworkHandler::GetUser(objid);
}

bool&
game_data_facade::is_in_world()
{
  static bool potato = false;
  return potato;
}

// ideally move away, copy pasta from old bot xD
template <typename T>
int
sgn(T val)
{
  return val == 0 ? 0 : val < 0 ? -1 : 1;
}

int
determinant(int ax, int ay, int bx, int by)
{
  return ax * by - ay * bx;
}

bool
game_data_facade::is_in_bounding_box(const FVector& loc)
{
  const auto& bot_config = get_bot_settings();
  if (bot_config.bounding_box.size() < 3) // degenerate box
    return true;

  const auto& xyPairs = bot_config.bounding_box;
  int sign = sgn(determinant(xyPairs[0].first - xyPairs[xyPairs.size() - 1].first,
                             xyPairs[0].second - xyPairs[xyPairs.size() - 1].second,
                             loc.x - xyPairs[xyPairs.size() - 1].first,
                             loc.y - xyPairs[xyPairs.size() - 1].second));
  for (size_t i = 0; i < xyPairs.size() - 1; ++i)
  {
    int tempSgn = sgn(determinant(xyPairs[i + 1].first - xyPairs[i].first,
                                  xyPairs[i + 1].second - xyPairs[i].second,
                                  loc.x - xyPairs[i].first,
                                  loc.y - xyPairs[i].second));
    if (tempSgn != sign)
      return false;
  }
  return true;
}

fishing&
game_data_facade::get_fishing_state()
{
  static fishing potato{fishing::fishing_state::inactive, false};
  return potato;
}

std::vector<dropped_item>
game_data_facade::get_dropped_items(bool only_mine, bool use_blacklist)
{
  std::vector<dropped_item> items;

  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  auto& arr = ziemniak->GetMonsterArray();
  // printf("Monster array addr %X count %d\n", &arr, arr.Num());
  farray_entry* objs = (farray_entry*)arr.GetData();

  int* my_id = (int*)((DWORD)GetModuleHandleA("engine.dll") + 0x81F530);

  // printf("My FObjectMap %X\n", find_by_uid_ptr(&ziemniak->GetMonsterArray(), 0, my_id));

  for (int i = 0; i < arr.Num(); ++i)
  {
    if (!objs[i].ptr)
      continue;
    if (objs[i].ptr->one_or_two != 2)
      continue;
    if (!objs[i].ptr->object_data)
      continue;
    if (only_mine && game_data_facade::get_my_drops().count(objs[i].objid) == 0)
      continue;
    int* im_so_tired = (int*)objs[i].ptr->object_data;
    if (im_so_tired[4] == 0)
      continue;
    if (use_blacklist && !get_bot_settings().manor_blacklist.is_allowed_target(im_so_tired[1]))
      continue;
    float distance = ziemniak->UNetworkHandler::GetDistance(
      find_by_uid_ptr(&ziemniak->GetMonsterArray(), 0, my_id), objs[i].ptr);
    if (distance < -0.5f) // equipped items are sometimes included with -1.0 distance but they seem
                          // to be excluded by the [4] check above anyways
      continue;
    float* xyz_data = (float*)(im_so_tired[4] + 0x1BC);
    /*
    TODO
    Item
{
objid
itemid
jedyneczka
quantity
??_7AL2Pickup@@6B@ pointer
}
*/
    items.emplace_back(
      dropped_item{objs[i].objid, im_so_tired[1], xyz_data[0], xyz_data[1], xyz_data[2], distance});
  }
  return items;
}
bool
game_data_facade::temp_does_item_exist(int objid)
{
  UNetworkHandler* ziemniak =
    *(UNetworkHandler**)((DWORD)GetModuleHandleA("engine.dll") + 0x81F538);
  auto& arr = ziemniak->GetMonsterArray();
  // printf("Monster array addr %X count %d\n", &arr, arr.Num());
  farray_entry* objs = (farray_entry*)arr.GetData();

  int* my_id = (int*)((DWORD)GetModuleHandleA("engine.dll") + 0x81F530);

  // printf("My FObjectMap %X\n", find_by_uid_ptr(&ziemniak->GetMonsterArray(), 0, my_id));

  for (int i = 0; i < arr.Num(); ++i)
  {
    if (!objs[i].ptr)
      continue;
    if (objs[i].ptr->one_or_two != 2)
      continue;
    if (!objs[i].ptr->object_data)
      continue;
    int* im_so_tired = (int*)objs[i].ptr->object_data;
    if (im_so_tired[4] == 0)
      continue;
    return true;
  }
  return false;
}

std::vector<User*>
game_data_facade::get_nearby_players(bool use_boundingbox)
{
  std::vector<User*> players;

  UNetworkHandler& unet = UNetworkHandler::get();
  auto& arr = unet.GetMonsterArray();
  farray_entry* objs = (farray_entry*)arr.GetData();

  for (int i = 0; i < arr.Num(); ++i)
  {
    if (!objs[i].ptr)
      continue;
    if (objs[i].ptr->one_or_two != 1)
      continue;
    if (!objs[i].ptr->object_data)
      continue;
    User* current = (User*)objs[i].ptr->object_data;
    if (!current->is_player())
      continue;
    if (use_boundingbox && !is_in_bounding_box(current->get_xyz()))
      continue;
    if (current->objid() == game_data_facade::get_my_id())
      continue;
    players.emplace_back(current);
  }
  return players;
}

std::vector<User*>
game_data_facade::get_users(std::function<bool(const User& user)> acceptance_predicate)
{
  std::vector<User*> result;

  UNetworkHandler& ziemniak = UNetworkHandler::get();
  auto& arr = ziemniak.GetMonsterArray();
  farray_entry* objs = (farray_entry*)arr.GetData();

  for (int i = 0; i < arr.Num(); ++i)
  {
    if (!objs[i].ptr)
      continue;
    if (objs[i].ptr->one_or_two != 1)
      continue;
    if (!objs[i].ptr->object_data)
      continue;
    User* the_guy = (User*)objs[i].ptr->object_data;
    if (acceptance_predicate(*the_guy))
      result.emplace_back(the_guy);
  }

  return result;
}

std::vector<Item*>
game_data_facade::get_items(std::function<bool(const Item& item)> acceptance_predicate)
{
  std::vector<Item*> result;
  UNetworkHandler& ziemniak = UNetworkHandler::get();
  auto& arr = ziemniak.GetMonsterArray();
  farray_entry* objs = (farray_entry*)arr.GetData();
  for (int i = 0; i < arr.Num(); ++i)
  {
    if (!objs[i].ptr)
      continue;
    if (objs[i].ptr->one_or_two != 2)
      continue;
    if (!objs[i].ptr->object_data)
      continue;
    Item* item = (Item*)objs[i].ptr->object_data;
    if (!item->is_on_ground())
      continue;
    if (acceptance_predicate(*item))
      result.emplace_back(item);
  }
  return result;
}

std::unordered_map<int, int>&
game_data_facade::get_cooldowns()
{
  static std::unordered_map<int, int> potato;
  return potato;
}

std::unordered_map<int, item_data>&
game_data_facade::get_items()
{
  static std::unordered_map<int, item_data> potato;
  return potato;
}

std::unordered_map<int, FL2MagicSkillData*>&
game_data_facade::get_skills()
{
  static std::unordered_map<int, FL2MagicSkillData*> potato;
  return potato;
}

std::unordered_map<int, my_drops>&
game_data_facade::get_my_drops()
{
  static std::unordered_map<int, my_drops> potato;
  for (auto it = potato.begin(); it != potato.end();)
  {
    if (it->second.timestamp + 120000 < GetTickCount())
      it = potato.erase(it);
    else
      ++it;
  }
  return potato;
}

bool
game_data_facade::is_monster_on_me()
{
  // TEMP
  UNetworkHandler& unet = UNetworkHandler::get();
  auto& arr = unet.GetMonsterArray();
  farray_entry* objs = (farray_entry*)arr.GetData();
  for (int i = 0; i < arr.Num(); ++i)
  {
    if (!objs[i].ptr)
      continue;
    if (objs[i].ptr->one_or_two != 1)
      continue;
    if (!objs[i].ptr->object_data)
      continue;
    User* current = (User*)objs[i].ptr->object_data;
    if (current && current->is_monster() && !current->is_dead() &&
        current->get_target_id() == game_data_facade::get_my_id())
      return true;
  }
  return false;
}

std::vector<User*>
helpers_filters::get_monsters_attacking_me_or_party(bool include_party_members)
{
  return game_data_facade::get_users([include_party_members](const User& the_guy) {
    int his_target = the_guy.get_target_id();
    return the_guy.is_monster() && the_guy.template_id() < 16013 && the_guy.template_id() > 16018 &&
           (his_target == game_data_facade::get_my_id() ||
            (include_party_members && party::get().members().count(his_target)));
  });
}

std::vector<Item*>
helpers_filters::get_manaherbs()
{
  return game_data_facade::get_items(
    [](const Item& item) { return item.itemid >= 8603 && item.itemid <= 8605; });
}

std::vector<Item*>
helpers_filters::get_items_near_point(const FVector& xyz, float radius)
{
  return game_data_facade::get_items([&xyz, square_radius = radius * radius](const Item& item) {
    return (item.get_xyz() - xyz).SizeSquared() < square_radius;
  });
}

User*
helpers_filters::closest_among_subset(const std::vector<User*>& users, const FVector& midpoint)
{
  if (users.empty())
    return nullptr;
  auto closest = users.begin();
  float smallest_size_squared = (midpoint - (*closest)->get_xyz()).SizeSquared();
  for (auto it = std::next(closest); it != users.end(); ++it)
  {
    float size_sq = (midpoint - (*it)->get_xyz()).SizeSquared();
    if (size_sq < smallest_size_squared)
    {
      smallest_size_squared = size_sq;
      closest = it;
    }
  }
  return *closest;
}
