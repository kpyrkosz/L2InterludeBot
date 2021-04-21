#pragma once

#include <Windows.h> //GetModuleHandle

class L2ParamStack
{
  char unk[80];

public:
  void* Top();
  L2ParamStack(int);
  ~L2ParamStack(void);
  int PushBack(void*);
};

class FVector
{
public:
  float x, y, z;

  FVector::FVector(void);
  FVector::FVector(float, float, float);
  int Normalize(void);
  float SizeSquared(void) const;
  FVector operator-(class FVector const&) const;
  FVector operator-=(class FVector const&);
  FVector operator+=(class FVector const&);
  FVector operator*=(float);
  FVector operator*(float) const;
  FVector operator+(class FVector const&) const;
};

class FArray
{
  int unk[12];

public:
  void* GetData(void);
  void const* GetData(void) const;
  int IsValidIndex(int) const;
  int Num(void) const;
};

// reversed, 0x80 in size on interlude
struct FL2MagicSkillData
{
  const wchar_t* skill_name;
  int name_size, name_capacity;
  const wchar_t* skill_description;
  int desc_size, desc_capacity;
  int skill_id;
  int skill_level;
  int unk;
  int manacost;
  int unk2;
  int range;
  // more unks - hp cost, is passive, is debuff?
};

enum gno_match_types : int
{
  dupaqq,
  item,
  enemy,
  friendqq,
  party,
  npc,
  creature,
};

class UNetworkHandler
{
public:
  // helpers
  static UNetworkHandler& get()
  {
    static auto instance_addr = (DWORD)GetModuleHandleA("engine.dll") + 0x81F538;
    UNetworkHandler* ziemniak = *(UNetworkHandler**)(instance_addr);
    return *ziemniak;
  }
  FArray& GetMonsterArray() { return *(FArray*)((char*)this + 0x194); }
  // imports
  virtual void Say2(class L2ParamStack&);
  virtual void RequestRestartPoint(class L2ParamStack&);
  virtual void Atk(int, class FVector, int);
  virtual void Action(int, class FVector, int);
  virtual float GetDistance(class FObjectMap*, class FObjectMap*);
  virtual int IsGNOMatch(int, class FObjectMap*);
  virtual void RequestMagicSkillUse(class L2ParamStack&);
  virtual void MoveBackwardToLocation(class FVector, class FVector);
  virtual struct User* GetUser(int);
  virtual struct User* GetUser(unsigned short*);
  virtual int RequestUseItem(class L2ParamStack&);
  virtual int RequestAnswerJoinParty(class L2ParamStack&);
  virtual void RequestProcureCropList(class L2ParamStack*);
  virtual int RequestBuySeed(class L2ParamStack&);
  virtual void RequestRecipeItemMakeSelf(int);
};

class FL2GameData
{
  int unk[123];

public:
  unsigned short const* GetItemName(int);
  unsigned short* GetCurrentZoneName(void);
  unsigned short const* GetItemDescription(int);
  unsigned short const* GetItemAdditionalName(int);
  int __thiscall GetItemPopMsgNum(int);
  unsigned short const* __thiscall GetItemSetEffect(int, int);
  int __thiscall GetItemSetEnchantCount(int);
  unsigned short const* __thiscall GetItemSetEnchantEffect(int);
  unsigned short const* __thiscall GetItemSetListID(int, int);
  class FString __thiscall GetItemSetListName(int, int, int);
  unsigned short const* GetNpcName(int);
  unsigned short const* GetStaticObjectName(int);
  int GetZoneID(float, float, float);
  int GetZoneID(class FVector);
  unsigned short* GetZoneName(int);
  unsigned short* GetZoneName(float, float, float);
  unsigned short* GetZoneName(class FVector);
  class FZoneNameInfo* GetZoneNameInfo(int, int, int, float);
  class UTexture* GetZoneNameTexture(int, int);
  struct FL2MagicSkillData* GetMSData(int skill_id, int skill_level);
  struct FL2RecipeData* GetRecipeDataByIndex(int);
  struct FL2RecipeData* GetRecipeDataByProductID(int);
  struct FL2RecipeData* GetRecipeDataByRecipeID(int);
};

__declspec(dllimport) FL2GameData GL2GameData;

// reversed

struct single_material_requirement
{
  int item_id;
  int count;
};

struct required_materials
{
  single_material_requirement** material_array;
  int mat_count;
};

struct FL2RecipeData
{
  int unk;
  required_materials req;
  int unk_const_21;
  const wchar_t* in_file_recipe_name;
  int in_file_string_capacity;
  int in_file_string_size;
  int recipe_id;      // ordinal exclusive to recipe system
  int recipe_item_id; // associated item that you register in your book
  int required_level_of_craft_skill;
  int resulting_item_id;
  int resulting_itemcount;
  int manacost;
  int chance;
  int material_count;
  int material_extra;
};

class FObjectMap
{
public:
  int one_or_two; // 2 = item 1 = pawn ?
  void* object_data;
};

struct AController
{
  char unk[123];
  virtual int GetSelectedCreatureID(void);
  bool is_attacking()
  {
    char* t = (char*)this;
    return t[0x41C] & 4;
  }
  bool is_dead()
  {
    // 2 jak goni, 6 mnie bije, 1 martwy
    char* t = (char*)this;
    return t[0x41C] & 1;
  }
  bool is_in_combat()
  {
    char* t = (char*)this;
    return t[0x41C] & 2;
  }
  void set_custom_sweepable(bool status)
  {
    // lol let's hope noone uses this flag
    int* t = (int*)((char*)this + 0x41C);
    status ? (*t |= 0x00400000) : (*t &= ~0x00400000);
  }
  bool is_custom_sweepable()
  {
    int* t = (int*)((char*)this + 0x41C);
    return *t & 0x00400000;
  }
  void set_custom_manorable(bool status)
  {
    // lol let's hope noone uses this flag
    int* t = (int*)((char*)this + 0x41C);
    status ? (*t |= 0x00200000) : (*t &= ~0x00200000);
  }
  bool is_custom_manorable()
  {
    int* t = (int*)((char*)this + 0x41C);
    return *t & 0x00200000;
  }
  int unk_41C() { return *(int*)(((char*)this) + 0x41C); }
};

struct APawn
{
  char unk[123];
  AController* get_acontroller()
  {
    return *(AController**)(((char*)this) + 0x14D8); // MOV ECX,DWORD PTR DS:[EDX+0x14D8]
  }
  int unk_6DC() { return *(int*)(((char*)this) + 0x6DC); }
  int unk_34() { return *(int*)(((char*)this) + 0x34); }
  /*10490BCE    BF FDFFFFFF     MOV EDI,-0x3
10490BD3    21BE DC060000   AND DWORD PTR DS:[ESI+0x6DC],EDI

*/
  // CMP BYTE PTR DS:[ESI+0x34],0x3
};

struct Item
{
  int objid;
  int itemid;
  int jedyneczka;
  int quantity;
  APawn* apickup; //??_7AL2Pickup@@6B@ pointer IF THIS IS NULL IT MEANS THE ITEM IS EQUIPPED LOL
  bool is_on_ground() const { return apickup != nullptr; }
  inline const FVector& get_xyz() const { return *(FVector*)((char*)apickup + 0x1BC); };
};

struct User
{
  unsigned short* GetName(void);
  unsigned short* GetNickName(void);
  int GetPledgeID(void);
  int GetPrivateStoreState(void);
  void IsMatchedStoreMsg(unsigned short*);
  int IsMyPartyMember(void);
  int IsMyPledgeMember(void);
  int IsPartyMaster(void);
  int IsPartyMember(void);
  int IsPledgeMaster(void);
  void SetName(unsigned short*);
  void SetNickName(unsigned short*);

  inline bool is_npc() const
  {
    // 00100
    int* let_see_xD = (int*)this;
    return let_see_xD[0] == 0 && let_see_xD[1] == 0 && let_see_xD[2] == 1 && let_see_xD[3] == 0 &&
           let_see_xD[4] == 0;
  }
  inline bool is_monster() const
  {
    // 00101
    int* let_see_xD = (int*)this;
    return let_see_xD[0] == 0 && let_see_xD[1] == 0 && let_see_xD[2] == 1 && let_see_xD[3] == 0 &&
           let_see_xD[4] == 1;
  }
  inline bool is_player() const
  {
    // 00000
    int* let_see_xD = (int*)this;
    return let_see_xD[0] == 0 && let_see_xD[1] == 0 && let_see_xD[2] == 0 && let_see_xD[3] == 0 &&
           let_see_xD[4] == 0;
  }
  inline bool is_guard() const
  {
    if (!is_npc())
      return false;
    const wchar_t* name =
      (wchar_t*)const_cast<User*>(this)->GetName(); // after all it's not MY fault
    if (name && wcscmp(name, L"Patrol") == 0)
      return true;
    const wchar_t* nickname = (wchar_t*)const_cast<User*>(this)->GetNickName();
    if (nickname && wcsstr(nickname, L"Guard"))
      return true;
    return false;
  }
  inline int objid() const
  {
    int* let_see_xD = (int*)this;
    return let_see_xD[6];
  }
  inline int template_id() const
  {
    int* let_see_xD = (int*)this;
    return let_see_xD[5];
  }
  inline bool is_dead() const { return get_apawn()->get_acontroller()->is_dead(); }
  inline bool is_attacking() const { return get_apawn()->get_acontroller()->is_attacking(); }
  char unk[0x7C];
  int maxhp;
  int hp;
  int maxmp;
  int mp;
  inline const FVector& get_xyz() const
  {
    int qq = (*(int*)(((char*)this) + 0x204) + 0x1BC); // MOV EAX,DWORD PTR DS:[EDX+0x204] //chyba
    return *(FVector*)qq;                              // APawn? MOV ECX,DWORD PTR DS:[EAX+0x1BC]
  }
  inline int get_target_id() const
  {
    return get_apawn()->get_acontroller()->AController::GetSelectedCreatureID();
  }
  inline int hp_percentage() const
  {
    if (maxhp == 0)
      return 100;
    return hp * 100 / maxhp;
  }
  inline int mp_percentage() const
  {
    if (maxmp == 0)
      return 100;
    return mp * 100 / maxmp;
  }
  APawn* get_apawn() const { return *(APawn**)(((char*)this) + 0x204); }
};

struct farray_entry
{
  int unk;
  int objid;
  FObjectMap* ptr;
};
