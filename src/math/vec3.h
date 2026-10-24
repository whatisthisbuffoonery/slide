#ifndef SLIDE_VEC3_H
#define SLIDE_VEC3_H
#include <math.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct s_vec3
    {
        double x;
        double y;
        double z;
    } t_vec3;
    t_vec3 vec_add(t_vec3 a, t_vec3 b);
    t_vec3 vec_sub(t_vec3 a, t_vec3 b);
    t_vec3 vec_scale(t_vec3 a, double scale);
    double vec_dot(t_vec3 a, t_vec3 b);
    int vec_in_range(t_vec3 value, double low, double high);
    t_vec3 vec_cross(t_vec3 a, t_vec3 b);
    double vec_magnitude(t_vec3 value);
    t_vec3 vec_norm(t_vec3 value);
#ifdef __cplusplus
}
#endif
#endif
