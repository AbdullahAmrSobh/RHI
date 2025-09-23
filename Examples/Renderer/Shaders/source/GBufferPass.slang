#include "Shaders/GpuCommonStructs.h"

struct GBufferInputs
{
    ConstantBuffer<GPU::SceneGlobalConstants>  consts;
    ConstantBuffer<GPU::SceneView> view;
    StructuredBuffer<float4x4> transforms;
    SamplerState               defaultSampler;
    Texture2D                  simpleTexture;
};
ParameterBlock<GBufferInputs> shaderParams;

struct UGBVertexInput
{
    float3 position;
    float3 normal;
    float2 uv;
    // uint32_t4 renderableID;
};

struct VSOutput
{
    float4 csPosition: SV_Position;
    // float3 wsPosition: TEXCOORD1;
    // float3 normal: TEXCOORD1;
    // float2 uv: TEXCOORD2;
};

struct GBufferOutput
{
    float4 wsPosition : SV_Target0; // World position
    // float4 normal : SV_Target1;     // Normal vector
    // float2 material : SV_Target2;   // (UV + material ID + roughness + metallic)
};

[[shader("vertex")]]
VSOutput VSMain(UGBVertexInput vsInput)
{
    let worldToView = shaderParams.view.worldToViewMatrix;
    let viewToClip = shaderParams.view.viewToClipMatrix;
    let worldToClip = mul(worldToView, viewToClip);

    let position = float4(vsInput.position, 1.0);
    let csPosition = mul(position, worldToClip);

    VSOutput output;
    output.csPosition = csPosition;
    // output.wsPosition = csPosition.xyz;
    // output.normal = normalize(wsNormal.xyz);
    // output.uv = input.uv;
    return output;
}

[[shader("fragment")]]
GBufferOutput PSMain(VSOutput input)
{
    GBufferOutput output;
    output.wsPosition = float4(1.0, 0.4, 0.3, 1.0);
    return output;
}
