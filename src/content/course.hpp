#pragma once
#include "../math/vector.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace slide
{
struct Message
{
    std::string id;
    std::string label;
    std::string body;
    std::string intended;
    std::string actual;
    std::string consequence;
    std::string avoided;
};
struct Box
{
    t_vec3 lo;
    t_vec3 hi;
    t_vec3 color;
};
inline t_vec3 messageHalfSize()
{
    return {1.2, 0.6, 0.16};
}
struct Hazard
{
    t_vec3 base;
    double amplitude = 0;
    double speed = 0;
    double phase = 0;
    size_t message = 0; // Resolved index within this course's message pool.
    t_vec3 at(double time) const;
    bool touches(t_vec3 center, double radius, double time) const;
};
struct AudioFiles
{
    std::filesystem::path music;
    std::filesystem::path jump;
    std::filesystem::path checkpoint;
    std::filesystem::path defeat;
    std::filesystem::path victory;
    float musicVolume = 0.25f;
    float effectsVolume = 0.75f;
};
struct Course
{
    std::string id;
    std::string name;
    std::string tip;
    std::filesystem::path directory;
    std::vector<Message> messages;
    std::vector<Box> boxes;
    std::vector<t_vec3> checkpoints;
    std::vector<Hazard> hazards;
    std::vector<t_vec3> testRoute;
    std::vector<t_vec3> testJumps;
    t_vec3 spawn{};
    t_vec3 goal{};
    AudioFiles audio;
};
// Relative content paths resolve from the file that contains them.
std::vector<std::filesystem::path> load_campaign(const std::filesystem::path &assets);
Course load_course(const std::filesystem::path &directory, unsigned seed = 1);
} // namespace slide
