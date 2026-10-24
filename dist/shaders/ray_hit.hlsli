bool find_hit_normal(float3 ro, float3 rd, bool includeBall, inout float t, out float3 n,
                     out float3 color, out int kind)
{
    bool hit = hit_mesh(ro, rd, t, n, color);
    kind = 0;
    float3 nn;
    if (includeBall && hit_sphere(ro, rd, ball.xyz, ball.w, t, nn))
    {
        n = nn;
        color = float3(.8, .92, .97);
        kind = 1;
        hit = true;
    }
    [unroll] for (int i = 0; i < 4; i++)
    {
        if (hazards[i].w > 0 && hit_message(ro, rd, hazards[i].xyz, t, nn))
        {
            n = nn;
            float3 local = ro + rd * t - hazards[i].xyz;
            float2 uv = saturate(float2(local.x / 2.4 + .5, .5 - local.y / 1.2));
            int2 texel = int2(min(int(uv.x * 512), 511), i * 256 + min(int(uv.y * 256), 255));
            color = abs(nn.z) > .5 ? messageAtlas.Load(int3(texel, 0)).rgb : float3(.85, .19, .32);
            kind = 3;
            hit = true;
        }
        float state = markers[i].w;
        if (state >= 0 && state < 3)
        {
            float3 center = markers[i].xyz + float3(0, .30 + .15 * sin(info.z * 2 + i), 0);
            if (hit_sphere(ro, rd, center, state == 2 ? .15 : .28, t, nn))
            {
                n = nn;
                color = state == 2   ? float3(.12, .5, .4)
                        : state == 1 ? float3(.15, 1, .7)
                                     : float3(.18, .30, .35);
                kind = 2;
                hit = true;
            }
        }
        if (state >= 3)
        {
            [unroll] for (int j = 0; j < 12; j++)
            {
                float a = j * 6.2831853 / 12;
                float3 center = markers[i].xyz + float3(cos(a) * 1.1, 1.0 + sin(a) * 1.1, 0);
                if (hit_sphere(ro, rd, center, .15, t, nn))
                {
                    n = nn;
                    color = state >= 4 ? float3(.25, 1, .75) : float3(.35, .25, .15);
                    kind = 2;
                    hit = true;
                }
            }
        }
    }
    return hit;
}
