#include "../src/game/game.hpp"
#include "../src/render/mesh.hpp"
#include "../src/audio/wav.hpp"
#include <nlohmann/json.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <chrono>
using namespace slide;
namespace fs = std::filesystem;
using Json = nlohmann::json;
static Json read_json(const fs::path &file)
{
    std::ifstream input(file);
    return Json::parse(input);
}
static void write_json(const fs::path &file, const Json &value)
{
    std::ofstream output(file);
    output << value.dump(2);
}
static void expect_error(const fs::path &folder)
{
    bool rejected = false;
    try
    {
        load_course(folder);
    }
    catch (const std::exception &)
    {
        rejected = true;
    }
    assert(rejected);
}
static void simulation_tests(const Course &course)
{
    Game game;
    game.new_journey(course);
    game.mode = Mode::Playing;
    for (int i = 0; i < 1200; ++i)
        game.tick({}, 1.0 / 120);
    assert(std::abs(game.position.y - .38) < .001 && game.grounded);
    game.tick({0, 0, false, true}, 1.0 / 120);
    assert(game.velocity.y > 7);
    for (int i = 0; i < 150; ++i)
        game.tick({}, 1.0 / 120);
    assert(game.grounded);
    game.position.y = -8;
    game.tick({}, 1.0 / 120);
    assert(game.mode == Mode::Defeated && game.lastMessage == -1);
    game.retry();
    assert(vec_magnitude(vec_sub(game.position, game.respawn)) < .001);
    for (const auto &obstacle : course.hazards)
    {
        game.position = obstacle.at(game.worldTime + 1.0 / 120);
        game.tick({}, 1.0 / 120);
        assert(game.mode == Mode::Defeated && game.lastMessage == int(obstacle.message));
        auto before = game.outcomes(false);
        game.tick({}, 1.0 / 120);
        assert(game.outcomes(false).front().count == before.front().count);
        game.retry();
    }
    int sends = 0;
    for (const auto &outcome : game.outcomes(false))
        sends += outcome.count;
    assert(sends == int(course.hazards.size()));
    game.elapsed = 12;
    game.commit_course();
    game.commit_course();
    game.load(course, 1);
    assert(game.journeyElapsed == 12);
    int carried = 0;
    for (const auto &outcome : game.outcomes(true))
        carried += outcome.count;
    assert(carried == sends);
    game.new_journey(course);
    assert(game.journeyElapsed == 0 && game.outcomes(true).front().count == 0);
    game.mode = Mode::Playing;
    game.position = {0, .38, 0};
    game.velocity = {0, 0, 8};
    game.grounded = true;
    game.world.hazards = {{{0, .75, 3}, 0, 0, 0, 0}};
    game.tick({0, 1, false, true}, 1.0 / 120);
    for (int i = 0; i < 65; ++i)
        game.tick({0, 1, false, false}, 1.0 / 120);
    assert(game.mode == Mode::Playing && game.position.z > 4);
}
static void traversal_test(Course course)
{
    Game game;
    course.hazards.clear();
    game.new_journey(course);
    game.mode = Mode::Playing;
    size_t waypoint = 0;
    int settling = 0;
    std::vector<bool> jumped(course.testJumps.size(), false);
    if (course.testRoute.empty())
        return;
    for (int tick = 0; tick < 120 * 120 && game.mode == Mode::Playing; ++tick)
    {
        t_vec3 difference = vec_sub(course.testRoute[waypoint], game.position);
        difference.y = 0;
        Input input;
        if (vec_magnitude(difference) < .6)
        {
            input.brake = true;
            if (++settling > 35)
            {
                waypoint = std::min(waypoint + 1, course.testRoute.size() - 1);
                settling = 0;
            }
        }
        else
        {
            settling = 0;
            input.x = difference.x;
            input.z = difference.z;
            input.brake = vec_magnitude(difference) < 3;
        }
        for (size_t j = 0; j < course.testJumps.size(); ++j)
        {
            t_vec3 offset = vec_sub(game.position, course.testJumps[j]);
            offset.y = 0;
            t_vec3 direction = normalized(difference);
            double progress = vec_dot(offset, direction);
            t_vec3 sideways = vec_sub(offset, vec_scale(direction, progress));
            if (!jumped[j] && game.grounded && progress >= 0 && progress < .15 &&
                vec_magnitude(sideways) < .8)
            {
                input.jump = true;
                jumped[j] = true;
            }
        }
        game.tick(input, 1.0 / 120);
        assert(game.deaths == 0);
    }
    assert(game.mode == Mode::Won);
    std::cout << course.id << ": course traversed in " << game.elapsed << "s\n";
}
static void content_tests(const fs::path &source)
{
    fs::path temp = fs::temp_directory_path() /
                    ("slide-readable-content-tests-" +
                     std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(temp);
    fs::copy(source, temp, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    Json course = read_json(temp / "course.json");
    Json pool = read_json(temp / "messages.json");
    Json extra = pool["messages"][0];
    extra["id"] = "new-message-without-rebuild";
    extra["body"] = "Edited text with Unicode: caf\u00e9";
    pool["messages"].push_back(extra);
    write_json(temp / "messages.json", pool);
    course["obstacles"][0]["message"] = "new-message-without-rebuild";
    write_json(temp / "course.json", course);
    auto loaded = load_course(temp);
    assert(loaded.messages[loaded.hazards[0].message].body.find("Edited text") !=
           std::string::npos);
    course["obstacles"][0]["message"] = "*";
    course["obstacles"][1]["message"] = "*";
    write_json(temp / "course.json", course);
    loaded = load_course(temp, 7);
    assert(loaded.hazards[0].message != loaded.hazards[1].message);
    auto again = load_course(temp, 7);
    assert(again.hazards[0].message == loaded.hazards[0].message);
    course["audio"]["music"] = nullptr;
    write_json(temp / "course.json", course);
    assert(!load_sound_bank(load_course(temp).audio)->music);
    course["audio"]["music"] = "audio/missing.wav";
    write_json(temp / "course.json", course);
    expect_error(temp);
    course["audio"]["music"] = nullptr;
    course["obstacles"][0]["message"] = "no-such-id";
    write_json(temp / "course.json", course);
    expect_error(temp);
    course["obstacles"][0]["message"] = "*";
    write_json(temp / "course.json", course);
    pool["messages"].push_back(pool["messages"][0]);
    write_json(temp / "messages.json", pool);
    expect_error(temp);
    fs::remove_all(temp);
}
int main(int argc, char **argv)
{
    fs::path assets = argc > 1 ? argv[1] : "assets";
    auto campaign = load_campaign(assets);
    assert(!campaign.empty());
    for (const auto &folder : campaign)
    {
        auto course = load_course(folder);
        auto sound = load_sound_bank(course.audio);
        assert(sound->music && !sound->music->samples.empty());
        Mesh mesh(course);
        assert(mesh.triangles.size() == course.boxes.size() * 12);
        std::vector<int> seen(mesh.triangles.size(), 0);
        for (const auto &node : mesh.nodes)
            if (node.left < 0)
                for (int i = 0; i < node.right; ++i)
                    ++seen.at(size_t(-node.left - 1 + i));
        for (int visits : seen)
            assert(visits == 1);
        simulation_tests(course);
        traversal_test(course);
    }
    content_tests(campaign.front());
    std::cout << "Original C vector maths, simulation, BVH, JSON reloads, message history, and WAV "
                 "loading passed.\n";
}
