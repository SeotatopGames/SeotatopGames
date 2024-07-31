#pragma pack_matrix(row_major)
//Texture2D color : register(t0);
//SamplerState filter : register(s0);
struct obj_Material
{
    float3 Kd;
    float d;
    float3 Ks;
    float Ns;
    float3 Ka;
    float sharpness;
    float3 Tf;
    float Ni;
    float3 Ke;
    uint illum;
};
struct Lights
{
    float4 color, position, rotation, attributes;
};
cbuffer SceneDataG : register(b0)
{
    float4 sunDirection, sunColor;
    float4x4 viewMatrix, projectionMatrix;
    float4 cameraPos, sunAmbient;
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
    int amountOfLights;
    int padding2;
};
StructuredBuffer<Lights> lightsBuffer : register(t3);
StructuredBuffer<obj_Material> material : register(t4);
float4 main(VS_Output_Vertex PS_Input_Vertex) : SV_TARGET
{
    float3 normalSunDirection = normalize(sunDirection.xyz);
    //return float4(PS_Input_Vertex.normW, 1);
    float lightRatio = saturate(dot(-normalSunDirection, PS_Input_Vertex.normW));
    float3 directLight = 0.0f;
    float3 surfaceColor = material[material_id].Kd;
    //add other lights to directionalLight
    for (int i = 0; i < amountOfLights; ++i)
    {
        switch (lightsBuffer[i].color.w)
        {
            case 0: //point
                float3 pointLightDirection = normalize(lightsBuffer[i].position.xyz - PS_Input_Vertex.posW);
                float3 pointLightRatio = saturate(dot(pointLightDirection, PS_Input_Vertex.normW));
                float pointAttenuation = 1.0f - saturate(length(lightsBuffer[i].position.xyz - PS_Input_Vertex.posW) / lightsBuffer[i].attributes.y);
                pointAttenuation *= pointAttenuation;
                float3 result = saturate(pointLightRatio * lightsBuffer[i].color.rgb * pointAttenuation);
                directLight = saturate(directLight + result);
                break;
            case 1:
                break;
            case 2: //spot
                float3 spotLightDirection = normalize(lightsBuffer[i].position.xyz - PS_Input_Vertex.posW);
                float surfaceRatio = saturate(dot(-spotLightDirection, (lightsBuffer[i].rotation.xyz)));
                float spotFactor = (surfaceRatio > lightsBuffer[i].attributes.z) ? 1.0f : 0.0f;
                float spotLightRatio = saturate(dot(spotLightDirection, PS_Input_Vertex.normW));
                float spotAttenuation = 1.0f - saturate((lightsBuffer[i].attributes.y - surfaceRatio) / (lightsBuffer[i].attributes.y - lightsBuffer[i].attributes.z));
                spotAttenuation *= spotAttenuation;
                directLight = saturate((spotLightRatio * lightsBuffer[i].color.rgb * spotFactor * spotAttenuation) + directLight);
                break;
            case 3:
                break;
        }
    }
    float3 indirectLight = sunAmbient.xyz * material[material_id].Ka;
    float3 emmissiveLight = material[material_id].Ke;
    
    float3 viewDirection = normalize(cameraPos.xyz - PS_Input_Vertex.posW.xyz);
    float3 halfVector = normalize(-normalSunDirection + viewDirection);
    float intensity = max(pow(saturate(dot(PS_Input_Vertex.normW, halfVector)), material[material_id].Ns + 0.000001f), 0);
    float3 reflectedLight = material[material_id].Ks * intensity;
    
    directLight += lightRatio * sunColor.xyz;
    float3 result = saturate(saturate(directLight + indirectLight) * surfaceColor + reflectedLight + emmissiveLight);
    return float4(result, material[material_id].d);
}