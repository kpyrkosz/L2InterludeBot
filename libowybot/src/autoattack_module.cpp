#include <action_executor.hpp>
#include <algorithm>
#include <array>
#include <autoattack_module.hpp>
#include <beastfarm_data.hpp>
#include <bot_action_penalties.hpp>
#include <bot_confiuration.hpp>
#include <game_data_facade.hpp>

bool
autoattack_module::perform_action()
{
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.autoattack))
    return false;

  if (game_data_facade::get_my_user().is_dead())
  {
    if (get_bot_settings().target_guards_for_delevel_test)
      action_executor::to_village();

    update_penalty(penalties.autoattack, 777);
    return true;
  }
  // autoattack, movecloser, spoil, pick temp?
  const auto& bot_config = get_bot_settings();

  if (penalties.current_intention != intention::idle &&
      penalties.current_intention != intention::combat)
    return false;

  User* my_target = game_data_facade::get_my_target();
  // duplicate check in targeting
  if (!bot_config.manor_seeds.empty() && my_target && my_target->is_dead() &&
      my_target->get_apawn()->get_acontroller()->is_custom_manorable() &&
      game_data_facade::get_items().count(5125))
  {
    puts("harvesting");
    penalties.current_intention = intention::combat;
    action_executor::use_item(game_data_facade::get_items().at(5125).objid); // harverster
    update_penalty(penalties.autoattack, 222);
    update_penalty(penalties.target, 444);
    update_penalty(penalties.spells, 444);
    update_penalty(penalties.picking, 444);
    return true;
  }

  // duplicate check in targeting
  if (bot_config.sweep && my_target && my_target->is_dead() &&
      my_target->get_apawn()->get_acontroller()->is_custom_sweepable())
  {
    puts("sweeping");
    penalties.current_intention = intention::combat;
    action_executor::use_spell(42); // sweep
    update_penalty(penalties.autoattack, 222);
    update_penalty(penalties.target, 444);
    update_penalty(penalties.spells, 444);
    update_penalty(penalties.picking, 444);
    return true;
  }

  // do we have a target?
  if (!my_target || my_target->is_dead())
  {
    penalties.current_intention = intention::idle;
    update_penalty(penalties.autoattack, 333);
    return false;
  }
  // ok seems like we do have a target
  // autoattack! //DODAJ CHODZENIE DO CELU WTEDY SIE NIE ZATNIE ; OOO
  // TODO BOX EXPLODING target
  // TODO SPOILING MA CHYBA PRIORYTET no, przeloz nizej, distance check peierwszy
  User* the_guy = game_data_facade::get_by_id(game_data_facade::get_my_user().get_target_id());
  if (!the_guy)
  {
    penalties.current_intention = intention::idle;
    update_penalty(penalties.autoattack, 333);
    return false;
  }
  // check if he's attackable
  if (the_guy->is_dead() ||
      (!the_guy->is_monster() && !bot_config.target_guards_for_delevel_test) ||
      (!the_guy->is_guard() && bot_config.target_guards_for_delevel_test))
  {
    // target is not attackable
    penalties.current_intention = intention::idle;
    update_penalty(penalties.autoattack, 444);
    return false;
  }
  if (bot_config.movecloser)
  {
    auto& his_xyz = the_guy->get_xyz();
    auto& my_xyx = game_data_facade::get_my_user().get_xyz();
    float dist_sq = (his_xyz.x - my_xyx.x) * (his_xyz.x - my_xyx.x) +
                    (his_xyz.y - my_xyx.y) * (his_xyz.y - my_xyx.y) +
                    (his_xyz.z - my_xyx.z) * (his_xyz.z - my_xyx.z);
    if (dist_sq > 150.0f * 150.0f) // chyba za malo, zmien na 200?
    {
      auto destination_xyz = his_xyz;
      destination_xyz.x += (rand() % 40 - 20);
      destination_xyz.y += (rand() % 40 - 20);
      // antistuck samples
      struct antistuck_sample
      {
        FVector initial_xyz;
        unsigned last_timestamp;
        int stuck_count;
      };
      static antistuck_sample as{};
      // check if the sample is old and reset if so
      unsigned curr_timestamp = GetTickCount();
      if (as.last_timestamp + 2500 < curr_timestamp)
      {
        puts("Resetting antistuck - TIME");
        as.last_timestamp = curr_timestamp;
        as.stuck_count = 0;
        as.initial_xyz = my_xyx;
      }
      else
      {
        // so the timestamp is in the 2,5 sec window, check if we advanced at least 150 units from
        // the beggining, if not, update the stuck counter
        FVector dist = as.initial_xyz - my_xyx;
        if (dist.SizeSquared() < 60.0f * 60.0f)
        {
          as.last_timestamp = curr_timestamp;
          ++as.stuck_count;
          printf("Seems like you got stuck, counter %d\n", as.stuck_count);
        }
        else
        {
          // okay, we moved far enough, set this point as initial
          puts("Resetting antistuck - MOVING AWAY");
          as.last_timestamp = curr_timestamp;
          as.stuck_count = 0;
          as.initial_xyz = my_xyx;
        }
        // if the stuck counter reached 5, do antistuck magic
        if (as.stuck_count > 4)
        {
          // TODO antistuck points or blockign the bot unit it's antistucked
          // for now just spam movement around your char a little bit
          // i think most probable scenario is getting stuck on the direct line between you and the
          // target, that's why i'll try to move a bit back and see how it works in practice
          FVector backward = my_xyx - his_xyz;
          backward.Normalize();
          destination_xyz = my_xyx + backward * 100.0f;
          destination_xyz.x += (rand() % 80 - 40);
          destination_xyz.y += (rand() % 80 - 40);
          puts("ANTISTUKZORDDDD");
          penalties.current_intention = intention::combat;
          update_penalty(penalties.autoattack, 1777);
          update_penalty(penalties.movement, 2222);
          action_executor::move_to_xyz(destination_xyz);
          return true;
        }
      }
      puts("movin closer");
      penalties.current_intention = intention::combat;
      update_penalty(penalties.autoattack, 555);
      update_penalty(penalties.movement, 777);
      action_executor::move_to_xyz(destination_xyz);
      return true;
    }
  }
  // is it a beastfarm mob? if so, just feed, dont attack
  if (!bot_config.beastfarm_cfg.high_priority.empty() ||
      !bot_config.beastfarm_cfg.low_priority.empty())
  {
    bool crystal = false, golden = false;
    // search the high priority monsters first
    for (int i = 0; i < bot_config.beastfarm_cfg.high_priority.size(); ++i)
    {
      auto feed = beastfarm_data::get().should_feed(my_target->template_id(),
                                                    bot_config.beastfarm_cfg.high_priority[i]);
      if (feed == beastfarm_feed::any)
      {
        crystal = true;
        golden = true;
        break;
      }
      if (feed == beastfarm_feed::golden)
        golden = true;
      if (feed == beastfarm_feed::crystal)
        crystal = true;
    }
    if (!crystal && !golden)
    {
      // repeat for low priority
      puts("Not feeding in HIGH PRIORITY MODE, maybe LOW?");
      for (int i = 0; i < bot_config.beastfarm_cfg.low_priority.size(); ++i)
      {
        auto feed = beastfarm_data::get().should_feed(my_target->template_id(),
                                                      bot_config.beastfarm_cfg.low_priority[i]);
        if (feed == beastfarm_feed::any)
        {
          crystal = true;
          golden = true;
          break;
        }
        if (feed == beastfarm_feed::golden)
          golden = true;
        if (feed == beastfarm_feed::crystal)
          crystal = true;
      }
    }
    if (!crystal && !golden)
      puts("Not feeding in LOW PRIORITY MODE, FUUUUUK");
    else
    {
      int feed_id = 0;
      // 6644 - crystal spice
      // 6643 golden
      if (crystal)
      {
        if (game_data_facade::get_items().count(6644) == 0)
          puts("FUGGG CRYSTAL SPICES OVER!!");
        else
          feed_id = game_data_facade::get_items().at(6644).objid;
      }
      if (golden)
      {
        if (game_data_facade::get_items().count(6643) == 0)
          puts("FUGGG GOLDEN SPICES OVER!!");
        else
        {
          if (feed_id)
          {
            puts("Both feeds are good - randomizing");
            if (rand() % 2)
              feed_id = game_data_facade::get_items().at(6643).objid;
          }
          else
            feed_id = game_data_facade::get_items().at(6643).objid;
        }
      }
      if (feed_id)
      {
        printf("Eat %X my friend, eat and grow big\n", feed_id);
        action_executor::use_item(feed_id);
        update_penalty(penalties.autoattack, 777);
        return true;
      }
      else
        puts("BUY FEEDS!!!!!");
    }
  }
  // maybe manor seeding?
  if (!is_under_penalty(penalties.autoattack) && !bot_config.manor_seeds.empty() &&
      !my_target->get_apawn()->get_acontroller()->is_custom_manorable() &&
      bot_config.manor_blacklist.is_allowed_target(my_target->template_id()))
  {
    auto& items = game_data_facade::get_items();
    int seed_candidate;

    std::vector<int> seed_candidates;
    for (const auto& elem : bot_config.manor_seeds)
    {
      if (items.count(elem.seed_itemid) && (items.count(elem.crop_itemid) == 0 ||
                                            items.at(elem.crop_itemid).count < elem.expected_count))
        seed_candidates.push_back(elem.seed_itemid);
    }

    // TODO AAAA JAK JUZ WSZYSTKO WYSIALEM TO QQ
    if (seed_candidates.empty())
      seed_candidate = bot_config.manor_seeds[rand() % bot_config.manor_seeds.size()].seed_itemid;
    else
      seed_candidate = seed_candidates[rand() % seed_candidates.size()];

    // a nie... tu przeciez nie przejdzie, git xD
    if (game_data_facade::get_items().count(seed_candidate))
    {
      printf("seeding %d (%X)", seed_candidate, seed_candidate);
      penalties.current_intention = intention::combat;
      action_executor::use_item(game_data_facade::get_items().at(seed_candidate).objid);
      update_penalty(penalties.autoattack, 555);
      update_penalty(penalties.seeding, 2345);
      update_penalty(penalties.target, 555);
      update_penalty(penalties.spells, 555);
      return true;
    }
    puts("You are out of seeds O_O");
    update_penalty(penalties.seeding, 1357);
  }
  // maybe spoil?
  if (bot_config.spoil && !my_target->get_apawn()->get_acontroller()->is_custom_sweepable() &&
      game_data_facade::get_skills().count(254) &&
      game_data_facade::get_skills().at(254)->manacost <= game_data_facade::get_my_user().mp &&
      bot_config.spoil_blacklist.is_allowed_target(the_guy->template_id()))
  {
    // is legal id check
    puts("spoiling");
    penalties.current_intention = intention::combat;
    action_executor::use_spell(254); // spoil
    update_penalty(penalties.autoattack, 555);
    update_penalty(penalties.spells, 777);
    return true;
  }
  if (bot_config.autoattack)
  {
    // are we in assist mode waitin foro the guy to start?
    if (bot_config.target == target_option::assist && bot_config.assist_wait)
    {
      // TODO dont call unetwork directly
      UNetworkHandler& ziemniak = UNetworkHandler::get();
      User* the_guy =
        ziemniak.UNetworkHandler::GetUser((unsigned short*)bot_config.target_name.c_str());
      // is he around? is he attacking
      if (!the_guy || !the_guy->is_attacking())
      {
        update_penalty(penalties.autoattack, 444);
        penalties.current_intention = intention::idle;
        return false;
      }
    }
    penalties.current_intention = intention::combat;
    puts("autoattack");
    update_penalty(penalties.autoattack, 777);
    // hmm tu mozna todo zeby tak nie jebal pakietami podczas ataku, czytac pakiet atkstart
    if (bot_config.target_guards_for_delevel_test)
      action_executor::ctrl_attack(my_target->objid());
    else
      action_executor::action(my_target->objid());
    return true;
  }
  penalties.current_intention = intention::idle;
  return false;
}
