#include "game.hpp"
#include <set>
namespace slide
{
void Game::load(Course course, int courseNumber)
{
    world = std::move(course);
    level = courseNumber;
    position = world.spawn;
    respawn = world.spawn;
    velocity = {};
    collected = 0;
    deaths = 0;
    elapsed = 0;
    worldTime = 0;
    grounded = false;
    lastMessage = -1;
    recapPage = 0;
    sentMessages.clear();
    events.clear();
    committed = false;
}
void Game::new_journey(Course firstCourse)
{
    completedOutcomes.clear();
    journeyElapsed = 0;
    load(std::move(firstCourse), 0);
}
std::vector<Outcome> Game::outcomes(bool wholeJourney) const
{
    std::vector<Outcome> result = wholeJourney ? completedOutcomes : std::vector<Outcome>{};
    std::set<std::string> included;
    for (const Hazard &obstacle : world.hazards)
    {
        const Message &message = world.messages.at(obstacle.message);
        if (!included.insert(message.id).second)
            continue;
        auto count = sentMessages.find(message.id);
        result.push_back({message, count == sentMessages.end() ? 0 : count->second});
    }
    return result;
}
void Game::commit_course()
{
    if (committed)
        return;
    auto current = outcomes(false);
    completedOutcomes.insert(completedOutcomes.end(), current.begin(), current.end());
    journeyElapsed += elapsed;
    committed = true;
}
void Game::fail(int messageIndex)
{
    if (mode != Mode::Playing)
        return;
    lastMessage = messageIndex;
    if (messageIndex >= 0)
        ++sentMessages[world.messages.at(size_t(messageIndex)).id];
    ++deaths;
    velocity = {};
    mode = Mode::Defeated;
    events.push_back(SoundEvent::Defeat);
}
void Game::retry()
{
    position = respawn;
    velocity = {};
    grounded = false;
    lastMessage = -1;
    mode = Mode::Playing;
}
void Game::reset()
{
    retry();
    ++deaths;
    events.push_back(SoundEvent::Defeat);
}
void Game::check_progress()
{
    constexpr double playerRadius = 0.38;
    for (const Hazard &obstacle : world.hazards)
    {
        if (obstacle.touches(position, playerRadius, worldTime))
        {
            fail(int(obstacle.message));
            return;
        }
    }
    if (position.y < -7)
    {
        fail(-1);
        return;
    }
    if (collected < world.checkpoints.size() &&
        vec_magnitude(vec_sub(position, world.checkpoints[collected])) < 1.25)
    {
        respawn = world.checkpoints[collected++];
        respawn.y = 0.42;
        events.push_back(SoundEvent::Checkpoint);
    }
    if (collected == world.checkpoints.size() && vec_magnitude(vec_sub(position, world.goal)) < 1.5)
    {
        mode = Mode::Won;
        events.push_back(SoundEvent::Victory);
    }
}
void Game::tick(Input input, double dt)
{
    if (mode != Mode::Playing)
        return;
    elapsed += dt;
    worldTime += dt;
    apply_input(input, dt);
    move_and_collide(dt);
    check_progress();
}
} // namespace slide
