// Pulled directly from the "VulkanDescriptorSets" sample.
// Removing the arrays & using HLSL StructuredBuffer<> would be better.
Texture2D color : register(t0);
SamplerState filter : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PS_IN input) : SV_TARGET
{
    return color.Sample(filter, input.uv);
}