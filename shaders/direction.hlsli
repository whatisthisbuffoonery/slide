// Same R + U + forward construction as src/direction.c.
// This game chooses a 60-degree vertical FOV; aspect ratio scales the horizontal axis.
float3 rectilinear(float2 ndc)
{
    float imageScale = tan(radians(60.0) * 0.5);
    float3 R = right.xyz * ndc.x * imageScale * (info.x / info.y);
    float3 U = up.xyz * ndc.y * imageScale;
    return normalize(R + U + forward.xyz);
}
