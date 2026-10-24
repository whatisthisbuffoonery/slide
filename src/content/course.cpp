#include "course.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <random>
#include <set>
#include <stdexcept>

namespace slide
{
using Json = nlohmann::json;

static Json read_json(const std::filesystem::path &file)
{
    std::ifstream input(file);
    if (!input)
        throw std::runtime_error(file.string() + ": cannot open file");
    try
    {
        return Json::parse(input);
    }
    catch (const Json::exception &error)
    {
        throw std::runtime_error(file.string() + ": " + error.what());
    }
}
static std::string required_text(const Json &object, const char *key)
{
    std::string value = object.at(key).get<std::string>();
    if (value.empty())
        throw std::runtime_error(std::string(key) + " cannot be empty");
    return value;
}
static double number(const Json &value)
{
    double result = value.get<double>();
    if (!std::isfinite(result))
        throw std::runtime_error("Number must be finite");
    return result;
}
static t_vec3 vector3(const Json &value)
{
    if (!value.is_array() || value.size() != 3)
        throw std::runtime_error("Expected a three-number vector [x, y, z]");
    return {number(value[0]), number(value[1]), number(value[2])};
}
static std::filesystem::path asset_path(const std::filesystem::path &folder, const Json &value)
{
    if (value.is_null())
        return {};
    auto path = folder / std::filesystem::u8path(value.get<std::string>());
    if (!std::filesystem::is_regular_file(path))
        throw std::runtime_error("Missing asset: " + path.string());
    return path; // Preserve the Windows UNC prefix when running from WSL storage.
}
std::vector<std::filesystem::path> load_campaign(const std::filesystem::path &assets)
{
    auto file = assets / "campaign.json";
    Json config = read_json(file);
    std::vector<std::filesystem::path> courses;
    std::set<std::string> ids;
    for (const auto &item : config.at("courses"))
    {
        auto folder = assets / std::filesystem::u8path(item.get<std::string>());
        auto course = load_course(folder);
        if (!ids.insert(course.id).second)
            throw std::runtime_error("Duplicate course id: " + course.id);
        courses.push_back(folder);
    }
    if (courses.empty())
        throw std::runtime_error(file.string() + ": no courses");
    return courses;
}
Course load_course(const std::filesystem::path &folder, unsigned seed)
{
    try
    {
        Json config = read_json(folder / "course.json");
        Course course;
        course.directory = folder;
        course.id = required_text(config, "id");
        course.name = required_text(config, "name");
        course.tip = required_text(config, "tip");
        course.spawn = vector3(config.at("spawn"));
        course.goal = vector3(config.at("goal"));

        Json pool =
            read_json(folder / std::filesystem::u8path(config.at("messages").get<std::string>()));
        std::set<std::string> ids;
        for (const auto &item : pool.at("messages"))
        {
            Message message;
            message.id = required_text(item, "id");
            message.label = required_text(item, "label");
            message.body = required_text(item, "body");
            message.intended = required_text(item, "intended");
            message.actual = required_text(item, "actual");
            message.consequence = required_text(item, "consequence");
            message.avoided = required_text(item, "avoided");
            if (!ids.insert(message.id).second)
                throw std::runtime_error("Duplicate message id: " + message.id);
            course.messages.push_back(message);
        }
        if (course.messages.empty())
            throw std::runtime_error("Message pool cannot be empty");

        for (const auto &item : config.at("platforms"))
        {
            t_vec3 center = vector3(item.at("center"));
            t_vec3 size = vector3(item.at("size"));
            if (size.x <= 0 || size.y <= 0 || size.z <= 0)
                throw std::runtime_error("Platform sizes must be positive");
            t_vec3 half = vec_scale(size, 0.5);
            course.boxes.push_back(
                {vec_sub(center, half), vec_add(center, half), vector3(item.at("color"))});
        }
        if (course.boxes.empty())
            throw std::runtime_error("At least one platform is required");
        for (const auto &value : config.at("checkpoints"))
            course.checkpoints.push_back(vector3(value));
        if (course.checkpoints.size() > 3)
            throw std::runtime_error("This renderer supports at most three checkpoints per course");

        std::mt19937 random(seed);
        std::vector<size_t> shuffled(course.messages.size());
        for (size_t i = 0; i < shuffled.size(); ++i)
            shuffled[i] = i;
        std::shuffle(shuffled.begin(), shuffled.end(), random);
        size_t nextRandom = 0;
        for (const auto &item : config.at("obstacles"))
        {
            Hazard obstacle;
            obstacle.base = vector3(item.at("position"));
            obstacle.amplitude = number(item.at("amplitude"));
            obstacle.speed = number(item.at("speed"));
            obstacle.phase = number(item.at("phase"));
            std::string id = item.at("message").get<std::string>();
            if (id == "*")
            {
                // Draw without replacement until the pool has been exhausted.
                obstacle.message = shuffled[nextRandom++ % shuffled.size()];
            }
            else
            {
                auto found = std::find_if(course.messages.begin(), course.messages.end(),
                                          [&](const Message &message) { return message.id == id; });
                if (found == course.messages.end())
                    throw std::runtime_error("Unknown message id: " + id);
                obstacle.message = size_t(found - course.messages.begin());
            }
            course.hazards.push_back(obstacle);
        }
        if (course.hazards.size() > 4)
            throw std::runtime_error(
                "At most four message cards can be active per course (pool size is unlimited)");

        const auto &audio = config.at("audio");
        course.audio.music = asset_path(folder, audio.at("music"));
        course.audio.jump = asset_path(folder, audio.at("jump"));
        course.audio.checkpoint = asset_path(folder, audio.at("checkpoint"));
        course.audio.defeat = asset_path(folder, audio.at("defeat"));
        course.audio.victory = asset_path(folder, audio.at("victory"));
        course.audio.musicVolume = float(number(audio.at("music_volume")));
        course.audio.effectsVolume = float(number(audio.at("effects_volume")));
        if (course.audio.musicVolume < 0 || course.audio.musicVolume > 1 ||
            course.audio.effectsVolume < 0 || course.audio.effectsVolume > 1)
            throw std::runtime_error("Audio volume must be between 0 and 1");

        if (config.contains("test_route"))
            for (const auto &point : config["test_route"])
                course.testRoute.push_back(vector3(point));
        if (config.contains("test_jumps"))
            for (const auto &point : config["test_jumps"])
                course.testJumps.push_back(vector3(point));
        return course;
    }
    catch (const std::exception &error)
    {
        throw std::runtime_error((folder / "course.json").string() + ": " + error.what());
    }
}
t_vec3 Hazard::at(double time) const
{
    return vec_add(base, {std::sin(time * speed + phase) * amplitude, 0, 0});
}
bool Hazard::touches(t_vec3 center, double radius, double time) const
{
    t_vec3 position = at(time);
    t_vec3 half = messageHalfSize();
    t_vec3 nearest = clamped(center, vec_sub(position, half), vec_add(position, half));
    return vec_magnitude(vec_sub(center, nearest)) < radius;
}
} // namespace slide
