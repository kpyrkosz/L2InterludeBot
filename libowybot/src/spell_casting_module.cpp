#include <action_executor.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>
#include <spell_casting_module.hpp>
#include <windows.h>

bool
spell_casting_module::perform_action()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.spells))
    return false;
  if (game_data_facade::get_my_user().is_dead())
    return false;
  int current_timestamp = GetTickCount();

  if (penalties.current_intention != intention::idle &&
      penalties.current_intention != intention::casting)
    return false;

  for (auto& spell : get_bot_settings().bot_spells)
  {
    // check if the cooldown passed
    if (game_data_facade::get_cooldowns()[spell.spellid] > current_timestamp)
      continue;

    // hmm this was the source of problem. spell.last_use_timestamp is a timestamp of bot requesting
    // cast and not timestamp when the game launched the spell
    if (spell.last_use_timestamp + spell.cooldown > current_timestamp)
      continue;

    // check, if we have the spell
    if (game_data_facade::get_skills().count(spell.spellid) == 0)
      continue;

    // check, if we have mana to cast it
    if (game_data_facade::get_skills().at(spell.spellid)->manacost >
        game_data_facade::get_my_user().mp)
      continue;

    // main switch is based on target
    // check condition
    User* guy_to_thing = nullptr;
    int his_id = 0; // huh this is bit smelly
    switch (spell.target)
    {
      case spell_target::self:
        guy_to_thing = &game_data_facade::get_my_user();
        his_id = game_data_facade::get_my_id();
        break;
      case spell_target::monster:
        // the targeting module is responsible for targeting a monster, if we dont have a target
        // just leave
        // temp, autoattack also blick spells cause it has picking OMFG
        if (game_data_facade::get_my_user().get_target_id() == 0)
          continue;
        his_id = game_data_facade::get_my_user().get_target_id();
        guy_to_thing = game_data_facade::get_by_id(his_id);
        break;
      case spell_target::nick:
      {
        // the user has to be in party, at least for now
        auto* pt_member = party::get().member_by_nick(spell.nick_optional);
        if (!pt_member)
          continue;
        guy_to_thing = game_data_facade::get_by_id(pt_member->objid);
        his_id = pt_member->objid;
      }
      break;
    }
    if (!guy_to_thing)
      continue;
    // TODO TARGET SELECT OUT OF PARTY??
    // TODO for now i dont check MY hp/mp because the config is about target's hp/mp
    if (guy_to_thing->is_dead())
    {
      penalties.current_intention = intention::idle;
      return false;
    }
    switch (spell.condtype)
    {
      case class_condition_type::always:
        if (game_data_facade::get_my_user().get_target_id() != his_id)
        {
          update_penalty(penalties.target, 333);
          update_penalty(penalties.spells, 222);
          action_executor::action(his_id);
          penalties.current_intention = intention::casting;
          return true;
        }
        // to jest zle - bo to tylko request, a nie use TODO
        spell.last_use_timestamp = current_timestamp;
        action_executor::use_spell(spell.spellid);
        ++penalties.magic_cast_count;
        printf("Using %X\n", spell.spellid);
        penalties.current_intention = intention::casting;
        update_penalty(penalties.spells, 222);
        return true;
      case class_condition_type::hpgt:
        if (guy_to_thing->maxhp != 0 &&
            guy_to_thing->hp * 100 / guy_to_thing->maxhp > spell.condval)
        {
          if (game_data_facade::get_my_user().get_target_id() != his_id)
          {
            update_penalty(penalties.target, 333);
            update_penalty(penalties.spells, 222);
            action_executor::action(his_id);
            penalties.current_intention = intention::casting;
            return true;
          }
          spell.last_use_timestamp = current_timestamp;
          action_executor::use_spell(spell.spellid);
          printf("Using %X\n", spell.spellid);
          update_penalty(penalties.spells, 222);
          penalties.current_intention = intention::casting;
          return true;
        }
        break;
      case class_condition_type::hplt:
        if (guy_to_thing->maxhp != 0 &&
            guy_to_thing->hp * 100 / guy_to_thing->maxhp < spell.condval)
        {
          if (game_data_facade::get_my_user().get_target_id() != his_id)
          {
            update_penalty(penalties.target, 333);
            update_penalty(penalties.spells, 222);
            action_executor::action(his_id);
            penalties.current_intention = intention::casting;
            return true;
          }
          spell.last_use_timestamp = current_timestamp;
          action_executor::use_spell(spell.spellid);
          printf("Using %X\n", spell.spellid);
          update_penalty(penalties.spells, 222);
          penalties.current_intention = intention::casting;
          return true;
        }
        break;
      case class_condition_type::managt:
        if (guy_to_thing->maxmp != 0 &&
            guy_to_thing->mp * 100 / guy_to_thing->maxmp > spell.condval)
        {
          if (game_data_facade::get_my_user().get_target_id() != his_id)
          {
            update_penalty(penalties.target, 333);
            update_penalty(penalties.spells, 222);
            action_executor::action(his_id);
            penalties.current_intention = intention::casting;
            return true;
          }
          spell.last_use_timestamp = current_timestamp;
          action_executor::use_spell(spell.spellid);
          printf("Using %X\n", spell.spellid);
          update_penalty(penalties.spells, 222);
          penalties.current_intention = intention::casting;
          return true;
        }
        break;
      case class_condition_type::manalt:
        if (guy_to_thing->maxmp != 0 &&
            guy_to_thing->mp * 100 / guy_to_thing->maxmp < spell.condval)
        {
          if (game_data_facade::get_my_user().get_target_id() != his_id)
          {
            update_penalty(penalties.target, 333);
            update_penalty(penalties.spells, 222);
            action_executor::action(his_id);
            penalties.current_intention = intention::casting;
            return true;
          }
          spell.last_use_timestamp = current_timestamp;
          action_executor::use_spell(spell.spellid);
          printf("Using %X\n", spell.spellid);
          update_penalty(penalties.spells, 222);
          penalties.current_intention = intention::casting;
          return true;
        }
        break;
    }
  }
  penalties.current_intention = intention::idle;
  update_penalty(penalties.spells, 222);
  return false;
}
