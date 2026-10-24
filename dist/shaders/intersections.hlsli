bool boxHit(float3 ro, float3 rd, float3 lo, float3 hi, float limit)
{
    float3 inv = float3(abs(rd.x) > 1e-8 ? 1 / rd.x : 1e8, abs(rd.y) > 1e-8 ? 1 / rd.y : 1e8,
                        abs(rd.z) > 1e-8 ? 1 / rd.z : 1e8);
    float3 a = (lo - ro) * inv, b = (hi - ro) * inv;
    float3 mn = min(a, b), mx = max(a, b);
    return max(max(mn.x, mn.y), max(mn.z, 0)) <= min(min(mx.x, mx.y), min(mx.z, limit));
}
bool hit_triangle(float3 ro, float3 rd, Triangle tr, inout float closest, out float3 normal)
{
    normal = 0;
    float3 a = tr.a.xyz, e1 = tr.b.xyz - a, e2 = tr.c.xyz - a;
    float3 p = cross(rd, e2);
    float det = dot(e1, p);
    if (abs(det) < 1e-7)
        return false;
    float3 s = ro - a;
    float u = dot(s, p) / det;
    if (u < 0 || u > 1)
        return false;
    float3 q = cross(s, e1);
    float v = dot(rd, q) / det;
    if (v < 0 || u + v > 1)
        return false;
    float t = dot(e2, q) / det;
    if (t < .002 || t >= closest)
        return false;
    closest = t;
    normal = normalize(cross(e1, e2));
    if (dot(normal, rd) > 0)
        normal = -normal;
    return true;
}
bool hit_mesh(float3 ro, float3 rd, inout float closest, out float3 normal, out float3 color)
{
    normal = 0;
    color = 0;
    bool found = false;
    int stack[48];
    int count = 1;
    stack[0] = 0;
    [loop] while (count > 0)
    {
        Node n = nodes[stack[--count]];
        if (!boxHit(ro, rd, n.lo, n.hi, closest))
            continue;
        if (n.left < 0)
        {
            [loop] for (int i = 0; i < n.right; i++)
            {
                Triangle tr = triangles[-n.left - 1 + i];
                float3 nn;
                if (hit_triangle(ro, rd, tr, closest, nn))
                {
                    normal = nn;
                    color = tr.color.rgb;
                    found = true;
                }
            }
        }
        else if (count < 46)
        {
            stack[count++] = n.left;
            stack[count++] = n.right;
        }
    }
    return found;
}
// Adapted from miniRT src/discriminant_sphere.c: use the stable q-form roots.
// GPU arithmetic is float; the CPU exponent-rescaling helper is not needed at this course scale.
bool hit_sphere(float3 origin, float3 direction, float3 center, float radius, inout float closest,
                out float3 normal)
{
    normal = 0;
    float3 difference = origin - center;
    float a = dot(direction, direction);
    float b = 2 * dot(difference, direction);
    float c = dot(difference, difference) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
        return false;
    float root = sqrt(discriminant);
    float q = -0.5 * (b + (b >= 0 ? root : -root));
    float first = abs(q) > 1e-8 ? q / a : -b / (2 * a);
    float second = abs(q) > 1e-8 ? c / q : first;
    float nearRoot = min(first, second);
    float farRoot = max(first, second);
    float distance = nearRoot > .002 ? nearRoot : farRoot;
    if (distance < .002 || distance >= closest)
        return false;
    closest = distance;
    normal = normalize(origin + direction * distance - center);
    return true;
}
bool hit_message(float3 ro, float3 rd, float3 center, inout float closest, out float3 normal)
{
    normal = 0;
    float3 extent = float3(1.2, .6, .16);
    float3 inv = float3(abs(rd.x) > 1e-8 ? 1 / rd.x : 1e8, abs(rd.y) > 1e-8 ? 1 / rd.y : 1e8,
                        abs(rd.z) > 1e-8 ? 1 / rd.z : 1e8);
    float3 a = (center - extent - ro) * inv, b = (center + extent - ro) * inv;
    float3 lo = min(a, b), hi = max(a, b);
    float entry = max(lo.x, max(lo.y, lo.z)), exitT = min(hi.x, min(hi.y, hi.z));
    float t = entry > .002 ? entry : exitT;
    if (entry > exitT || t < .002 || t >= closest)
        return false;
    closest = t;
    float3 local = (ro + rd * t - center) / extent;
    if (abs(local.x) > abs(local.y) && abs(local.x) > abs(local.z))
        normal = float3(sign(local.x), 0, 0);
    else if (abs(local.y) > abs(local.z))
        normal = float3(0, sign(local.y), 0);
    else
        normal = float3(0, 0, sign(local.z));
    return true;
}
