#include "app.hpp"
using namespace slide;

// Diagnostic-only state transitions; deliberately kept out of the normal frame loop.
void Application::smoke_step()
{
    if (frameNumber == 1)
    {
        game.mode = Mode::Playing;
        game.position = {0, .4, 4};
        renderer.camera = game.position + t_vec3{0, 8, -12};
    }
    if (frameNumber == 2 || frameNumber == 5 || frameNumber == 7)
    {
        game.mode = Mode::Playing;
        if (!game.world.hazards.empty())
        {
            const auto &obstacle = game.world.hazards.front();
            game.position = obstacle.at(game.worldTime + 1.0 / 120);
            game.tick({}, 1.0 / 120);
            if (game.mode != Mode::Defeated)
                throw std::runtime_error("Smoke test: message collision did not enter defeat");
            log << "Hit: " << game.world.messages.at(obstacle.message).id << "\n";
        }
    }
    if (frameNumber == 3)
    {
        game.retry();
        game.position.y = -8;
        game.tick({}, 1.0 / 120);
        if (game.lastMessage != -1)
            throw std::runtime_error("Smoke test: fall attributed to a message");
    }
    if ((frameNumber == 4 || frameNumber == 6) && size_t(game.level + 1) < courseFolders.size())
    {
        game.commit_course();
        load_course(size_t(game.level + 1));
        game.mode = Mode::Playing;
    }
    if (frameNumber == 8)
        game.mode = Mode::Won;
    if (frameNumber == 9)
        game.mode = Mode::Complete;
}
