struct Triangle
{
    float4 a, b, c, color;
};
struct Node
{
    float3 lo;
    int left;
    float3 hi;
    int right;
};
StructuredBuffer<Triangle> triangles : register(t0);
StructuredBuffer<Node> nodes : register(t1);
Texture2D<float4> hud : register(t2);
Texture2D<float4> messageAtlas : register(t3);
RWTexture2D<float4> image : register(u0);
cbuffer Frame : register(b0)
{
    float4 eye;
    float4 forward;
    float4 right;
    float4 up;
    float4 ball;
    float4 info;       // width, height, time, level
    float4 markers[4]; // xyz, state
    float4 hazards[4]; // xyz, enabled
};
