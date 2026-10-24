#pragma once
#include "../content/course.hpp"
namespace slide
{
struct F4
{
    float x = 0, y = 0, z = 0, w = 0;
    F4() = default;
    F4(t_vec3 value, float fourth = 0)
        : x(float(value.x)), y(float(value.y)), z(float(value.z)), w(fourth)
    {
    }
};
struct GpuVec3
{
    float x = 0, y = 0, z = 0;
    GpuVec3() = default;
    GpuVec3(t_vec3 value) : x(float(value.x)), y(float(value.y)), z(float(value.z))
    {
    }
};
struct Triangle
{
    F4 a, b, c, color;
};
struct Node
{
    GpuVec3 lo;
    int left;
    GpuVec3 hi;
    int right;
};
static_assert(sizeof(Triangle) == 64 && sizeof(Node) == 32, "Match HLSL structured buffers");
struct Mesh
{
    std::vector<Triangle> triangles;
    std::vector<Node> nodes;
    void box(const Box &b)
    {
        t_vec3 p[8];
        for (int i = 0; i < 8; ++i)
            p[i] = {(i & 1) ? b.hi.x : b.lo.x, (i & 2) ? b.hi.y : b.lo.y,
                    (i & 4) ? b.hi.z : b.lo.z};
        const int faces[12][3] = {{0, 2, 3}, {0, 3, 1}, {4, 5, 7}, {4, 7, 6}, {0, 4, 6}, {0, 6, 2},
                                  {1, 3, 7}, {1, 7, 5}, {2, 6, 7}, {2, 7, 3}, {0, 1, 5}, {0, 5, 4}};
        for (auto &f : faces)
            triangles.push_back({F4(p[f[0]]), F4(p[f[1]]), F4(p[f[2]]), F4(b.color)});
    }
    static t_vec3 xyz(F4 a)
    {
        return {a.x, a.y, a.z};
    }
    int build(int begin, int end)
    {
        int idx = (int)nodes.size();
        nodes.push_back({});
        t_vec3 lo{1e9f, 1e9f, 1e9f}, hi{-1e9f, -1e9f, -1e9f};
        for (int i = begin; i < end; ++i)
        {
            auto &t = triangles[i];
            lo = minimum(lo, minimum(xyz(t.a), minimum(xyz(t.b), xyz(t.c))));
            hi = maximum(hi, maximum(xyz(t.a), maximum(xyz(t.b), xyz(t.c))));
        }
        if (end - begin <= 4)
        {
            nodes[idx] = {lo, -begin - 1, hi, end - begin};
            return idx;
        }
        t_vec3 d = hi - lo;
        int axis = d.y > d.x ? 1 : 0;
        if (d.z > (axis == 0 ? d.x : d.y))
            axis = 2;
        auto centroid = [axis](const Triangle &t)
        {
            t_vec3 v = xyz(t.a) + xyz(t.b) + xyz(t.c);
            return axis == 0 ? v.x : axis == 1 ? v.y : v.z;
        };
        int mid = (begin + end) / 2;
        std::nth_element(triangles.begin() + begin, triangles.begin() + mid,
                         triangles.begin() + end, [&](const Triangle &a, const Triangle &b)
                         { return centroid(a) < centroid(b); });
        int l = build(begin, mid), r = build(mid, end);
        nodes[idx] = {lo, l, hi, r};
        return idx;
    }
    explicit Mesh(const Course &c)
    {
        for (auto &b : c.boxes)
            box(b);
        build(0, (int)triangles.size());
    }
};

} // namespace slide
