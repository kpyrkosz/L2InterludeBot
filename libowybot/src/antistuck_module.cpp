#include <action_executor.hpp>
#include <antistuck_module.hpp>
#include <bot_action_penalties.hpp>
#include <cmath>
#include <deque>
#include <game_data_facade.hpp>

bool
antistuck_module::perform_action()
{
  // scans for XYZ change, if the char is moving, resets the penalty pick/cast
  // counters to 0 otherwise hmm, does random move?
  auto& penalties = get_penalties();
  if (is_under_penalty(penalties.antistuck))
    return false;

  static std::deque<FVector> xyz_samples;
  static int antistuck_attempts = 0;
  xyz_samples.push_back(game_data_facade::get_my_user().get_xyz());
  if (xyz_samples.size() > 5)
    xyz_samples.pop_front();

  // for now let's check if first and last are close
  if ((xyz_samples.front() - xyz_samples.back()).SizeSquared() < 10000.0f)
  {
    // we didnt move for more than 100 units, check if we were trying to cast or pick something for
    // more that 10 times
    if (penalties.magic_cast_count > 10 || penalties.pick_requests_count > 10)
    {
      puts("OMGZOR YOU GOT STUCK!!!");
      ++antistuck_attempts;
      if (antistuck_attempts > 10)
      {
        // seems like we executed 10 antistuck movements and nothing good happened. chill out for a
        // sec
        puts("OMGZORRRR ANTISTUCK GOT STUCK!!");
        antistuck_attempts = 0;
        penalties.magic_cast_count = 0;
        penalties.pick_requests_count = 0;
      }
      FVector curr_loc = game_data_facade::get_my_user().get_xyz();
      float random_angle = ((float)rand() / (float)(RAND_MAX)) * 6.28f;
      curr_loc.x += 50.0f * sin(random_angle) * antistuck_attempts;
      curr_loc.y += 50.0f * cos(random_angle) * antistuck_attempts;
      update_penalty(penalties.antistuck, 88 + 55 * antistuck_attempts);
      action_executor::move_to_xyz(curr_loc);
    }
  }
  else
    antistuck_attempts = 0;
  update_penalty(penalties.antistuck, 555);
  return false;
}
