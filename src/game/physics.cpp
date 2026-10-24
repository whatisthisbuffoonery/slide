#include "game.hpp"
namespace slide
{
void Game::apply_input(Input input, double dt)
{
    t_vec3 direction = normalized({input.x, 0, input.z});
    double acceleration = grounded ? 18.0 : 8.0;
    velocity = vec_add(velocity, vec_scale(direction, acceleration * dt));

    double drag = input.brake ? 10.0 : 0.8;
    double damping = std::exp(-drag * dt);
    velocity.x *= damping;
    velocity.z *= damping;

    double speed = std::hypot(velocity.x, velocity.z);
    if (speed > 8.0)
    {
        velocity.x *= 8.0 / speed;
        velocity.z *= 8.0 / speed;
    }
    if (input.jump && grounded)
    {
        velocity.y = 8.0;
        grounded = false;
        events.push_back(SoundEvent::Jump);
    }
    velocity.y -= 20.0 * dt;
}
static t_vec3 inside_box_normal(t_vec3 position, const Box &box, double &distance)
{
    double distances[] = {position.x - box.lo.x, box.hi.x - position.x, position.y - box.lo.y,
                          box.hi.y - position.y, position.z - box.lo.z, box.hi.z - position.z};
    const t_vec3 normals[] = {{-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}};
    int nearestFace = 0;
    for (int face = 1; face < 6; ++face)
        if (distances[face] < distances[nearestFace])
            nearestFace = face;
    distance = distances[nearestFace];
    return normals[nearestFace];
}
void Game::move_and_collide(double dt)
{
    constexpr double radius = 0.38;
    constexpr int substeps = 2;
    for (int substep = 0; substep < substeps; ++substep)
    {
        position = vec_add(position, vec_scale(velocity, dt / substeps));
        grounded = false;
        for (const Box &box : world.boxes)
        {
            t_vec3 nearest = clamped(position, box.lo, box.hi);
            t_vec3 difference = vec_sub(position, nearest);
            double distance = vec_magnitude(difference);
            if (distance > radius + 0.0001)
                continue;

            t_vec3 normal;
            double penetration;
            if (distance > 1e-6)
            {
                normal = vec_scale(difference, 1.0 / distance);
                penetration = std::max(0.0, radius - distance);
            }
            else
            {
                double faceDistance;
                normal = inside_box_normal(position, box, faceDistance);
                penetration = radius + faceDistance;
            }
            position = vec_add(position, vec_scale(normal, penetration));
            double inwardSpeed = vec_dot(velocity, normal);
            if (inwardSpeed < 0)
                velocity = vec_sub(velocity, vec_scale(normal, inwardSpeed));
            if (normal.y > 0.6)
                grounded = true;
        }
    }
}
} // namespace slide
