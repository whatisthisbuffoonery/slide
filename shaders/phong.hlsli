float3 sky(float3 rd)
{
    float horizon = pow(saturate(1 - abs(rd.y)), 5);
    float3 color = lerp(float3(.012, .022, .055), float3(.09, .18, .25), saturate(rd.y * .7 + .5));
    color += float3(.30, .14, .12) * horizon;
    float sun = pow(saturate(dot(rd, normalize(float3(-.6, .25, .8)))), 180);
    color += float3(1, .58, .25) * sun;
    float2 p = floor(rd.xz / max(.08, abs(rd.y)) * 170);
    float hash = frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
    color += step(.999, hash) * smoothstep(.05, .4, rd.y) * .45;
    return color;
}
float shadow(float3 p, float3 light)
{
    float t = 100;
    float3 n, c;
    if (hit_mesh(p, light, t, n, c))
        return .22;
    if (hit_sphere(p, light, ball.xyz, ball.w, t, n))
        return .22;
    [unroll] for (int i = 0; i < 4;
                  i++) if (hazards[i].w > 0 &&
                           hit_message(p, light, hazards[i].xyz, t, n)) return .22;
    return 1;
}
// GPU translation of the named terms in miniRT src/phong.c.
float3 ambient_term(float3 surfaceColor)
{
    return surfaceColor * .30;
}
float3 diffuse_term(float3 surfaceColor, float3 normal, float3 lightDirection)
{
    float diffuse = max(dot(normal, lightDirection), 0);
    return surfaceColor * diffuse * .85;
}
float3 specular_term(float3 normal, float3 lightDirection, float3 viewDirection, float shininess)
{
    if (dot(normal, lightDirection) <= 0)
        return 0;
    float3 reflected = 2 * normal * dot(normal, lightDirection) - lightDirection;
    return .4 * pow(max(dot(reflected, viewDirection), 0), shininess);
}
float3 phong(float3 ro, float3 rd, float t, float3 n, float3 base, int kind)
{
    float3 p = ro + rd * t;
    if (kind == 2)
        return base * 1.7;
    if (kind == 3)
        return pow(base, 2.2) * 1.5;
    float3 light = normalize(float3(-.5, .85, -.3));
    if (kind == 0 && n.y > .5)
    {
        float2 grid = abs(frac(p.xz * .5) - .5);
        float gridLine = 1 - smoothstep(.015, .035, min(grid.x, grid.y));
        base = lerp(base, base * 1.6, gridLine * .38);
    }
    float visibility = shadow(p + n * .006, light);
    float3 c =
        ambient_term(base) + visibility * (diffuse_term(base, n, light) +
                                           specular_term(n, light, -rd, kind == 1 ? 80 : 35));
    if (kind == 1)
    {
        float3 refl = reflect(rd, n), rn, rc;
        int rk;
        float rt = 100;
        float3 reflected = sky(refl);
        if (find_hit_normal(p + n * .006, refl, false, rt, rn, rc, rk))
            reflected = rc * (rk == 2 ? 1.5 : .65);
        c = lerp(c, reflected, .55) + float3(.03, .12, .15);
    }
    return lerp(c, sky(rd), 1 - exp(-t * .009));
}
