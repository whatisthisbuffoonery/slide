#pragma once
#include "../content/course.hpp"
#include <map>

namespace slide
{
enum class Mode
{
    Title,
    Playing,
    Paused,
    Defeated,
    Won,
    Complete
};
enum class SoundEvent
{
    Jump,
    Checkpoint,
    Defeat,
    Victory
};
struct Input
{
    double x = 0;
    double z = 0;
    bool brake = false;
    bool jump = false;
};
struct Outcome
{
    Message message;
    int count = 0;
};
struct Game
{
    Course world;
    Mode mode = Mode::Title;
    int level = 0;
    size_t collected = 0;
    int deaths = 0;
    double elapsed = 0;
    double worldTime = 0;
    double journeyElapsed = 0;
    t_vec3 position{}, velocity{}, respawn{};
    bool grounded = false;
    int lastMessage = -1;
    size_t recapPage = 0;
    std::vector<SoundEvent> events;

    void load(Course course, int courseNumber);
    void new_journey(Course firstCourse);
    void commit_course();
    void reset();
    void retry();
    void fail(int messageIndex);
    void tick(Input input, double dt);
    std::vector<Outcome> outcomes(bool wholeJourney) const;

  private:
    std::map<std::string, int> sentMessages;
    std::vector<Outcome> completedOutcomes;
    bool committed = false;
    void apply_input(Input input, double dt);
    void move_and_collide(double dt);
    void check_progress();
};
} // namespace slide
