#pragma once
#include "vec3.h"
#include <algorithm>

// These conveniences call the original miniRT C functions.
// Physics below uses their explicit vec_* names so its operations stay visible.
inline t_vec3 operator+(t_vec3 a, t_vec3 b)
{
    return vec_add(a, b);
}
inline t_vec3 operator-(t_vec3 a, t_vec3 b)
{
    return vec_sub(a, b);
}
inline t_vec3 operator*(t_vec3 a, double scale)
{
    return vec_scale(a, scale);
}
inline t_vec3 operator/(t_vec3 a, double scale)
{
    return vec_scale(a, 1.0 / scale);
}
inline t_vec3 &operator+=(t_vec3 &a, t_vec3 b)
{
    a = vec_add(a, b);
    return a;
}
inline t_vec3 normalized(t_vec3 value)
{
    if (vec_dot(value, value) < 1e-14)
        return {0, 0, 0};
    return vec_norm(value);
}
inline t_vec3 minimum(t_vec3 a, t_vec3 b)
{
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}
inline t_vec3 maximum(t_vec3 a, t_vec3 b)
{
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}
inline t_vec3 clamped(t_vec3 value, t_vec3 low, t_vec3 high)
{
    return maximum(low, minimum(value, high));
}
