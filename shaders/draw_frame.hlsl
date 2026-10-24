#include "types.hlsli"
#include "direction.hlsli"
#include "intersections.hlsli"
#include "ray_hit.hlsli"
#include "phong.hlsli"

[numthreads(8, 8, 1)] void draw_frame(uint3 id
                                      : SV_DispatchThreadID)
{
    if (id.x >= uint(info.x) || id.y >= uint(info.y))
        return;
    float2 uv = (float2(id.xy) + .5) / info.xy;
    float2 ndc = (uv * 2 - 1) * float2(1, -1);
    float3 rd = rectilinear(ndc);
    float t = 200;
    float3 n, c;
    int kind;
    float3 result = sky(rd);
    if (find_hit_normal(eye.xyz, rd, true, t, n, c, kind))
        result = phong(eye.xyz, rd, t, n, c, kind);
    // A distant ground grid gives the floating course depth without a collider.
    else if (rd.y < -.001)
    {
        float planeT = (-9 - eye.y) / rd.y;
        float3 p = eye.xyz + rd * planeT;
        float2 grid = abs(frac(p.xz * .1) - .5);
        float lines = 1 - smoothstep(.004, .014, min(grid.x, grid.y));
        result += float3(.04, .16, .20) * lines * exp(-planeT * .018);
    }
    result = result / (1 + result);
    result = pow(max(result, 0), 1 / 2.2);
    result *= 1 - .20 * dot(uv - .5, uv - .5);
    float4 ui = hud.Load(int3(id.xy, 0));
    float alpha = max(ui.r, max(ui.g, ui.b)) > .001 ? .96 : 0;
    result = lerp(result, ui.rgb, alpha);
    image[id.xy] = float4(result, 1);
} Texture2D<float4> frameTexture : register(t0);
SamplerState linearSampler : register(s0);
struct Vertex
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};
Vertex VSMain(uint id : SV_VertexID)
{
    Vertex v;
    v.uv = float2((id << 1) & 2, id & 2);
    v.position = float4(v.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return v;
}
float4 PSMain(Vertex v) : SV_Target
{
    return frameTexture.Sample(linearSampler, v.uv);
}
