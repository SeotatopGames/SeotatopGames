//Vertex Shader Test

#pragma pack_matrix(row_major)
struct ATTRIBUTES
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    unsigned int illum; // illumination model
};

cbuffer SceneData : register(b0)
{
    float4 sunDirection, sunColor;
    float4x4 viewMatrix, projectionMatrix;
    float4 cameraPosition,sunAmbient;
};


struct VS_Input_Vertex
{
    float3 pos : POS;
    float3 uvw : UVW;
    float3 nrm : NRM;
};

struct VS_Output_Vertex
{
    float4 posH : SV_POSITION;
    float3 posW : WORLD;
    float3 normW : NORMAL;
    float3 uvw : TEXTURE;
};
cbuffer ModelIDS : register(b2)
{
    uint model_id;
    uint material_id;
    int padding;
    int padding2;
};

StructuredBuffer<float4x4> worldMatrix : register(t3);

VS_Output_Vertex main(VS_Input_Vertex inputVertex, uint id : SV_InstanceID)
{
    VS_Output_Vertex outputVertex;
    outputVertex.posH = mul(worldMatrix[model_id + id], float4(inputVertex.pos, 1.0f));
    outputVertex.posW = outputVertex.posH.rgb;
    outputVertex.normW = float3(inputVertex.nrm.r, inputVertex.nrm.g, inputVertex.nrm.b);
    outputVertex.normW = mul(worldMatrix[model_id + id] ,float4(outputVertex.normW, 0.0f)).rgb;
    outputVertex.normW = normalize(outputVertex.normW);
    outputVertex.uvw = inputVertex.uvw;
    outputVertex.posH = mul(outputVertex.posH, viewMatrix);
    outputVertex.posH = mul(outputVertex.posH, projectionMatrix);
    return outputVertex;
}