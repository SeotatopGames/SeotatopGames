#pragma pack_matrix(row_major)
Texture2D color : register(t0);
SamplerState filter : register(s0);
struct VS_Output_Vertex
{
    float4 posH : SV_POSITION;
    float3 posW : WORLD;
    float3 normW : NORMAL;
    float3 uvw : TEXTURE;
};
float4 main(VS_Output_Vertex PS_Input_Vertex) : SV_TARGET
{
    return color.Sample(filter, PS_Input_Vertex.uvw.xy);
}