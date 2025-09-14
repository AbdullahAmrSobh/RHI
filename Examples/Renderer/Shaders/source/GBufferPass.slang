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
    uint32_t4 renderableID;
};

struct VSOutput
{
    float4 csPosition: SV_Position;
    float3 wsPosition: TEXCOORD1;
    float3 normal: TEXCOORD1;
    float2 uv: TEXCOORD2;
};

struct GBufferOutput
{
    float4 wsPosition : SV_Target0; // World position
    // float4 normal : SV_Target1;     // Normal vector
    // float2 material : SV_Target2;   // (UV + material ID + roughness + metallic)
};

static const float3 positions[] =
{
    float3(-0.5, -0.5, 1.0),
    float3( 0.5, -0.5, 1.0),
    float3( 0.0,  0.5, 1.0)
};


[[shader("vertex")]]
VSOutput VSMain(uint vertexID : SV_VertexID)
{
    let v = shaderParams.view.worldToViewMatrix;
    var p = float4(positions[vertexID], 1.0);
    p = mul(v, p);

    // let id = input.renderableID.x;
    // let view = shaderParams.view;
    // let modelToWorld = shaderParams.transforms[id];

    // float4x4 worldToViewMatrix = view.worldToViewMatrix;
    // float4x4 worldToClipMatrix = mul(view.viewToClipMatrix, worldToViewMatrix);

    // float4 wsPosition = mul(worldToViewMatrix, float4(input.position, 1.0f));
    // float4 csPosition = mul(worldToClipMatrix, wsPosition);
    // float4 wsNormal   = mul(modelToWorld, float4(input.normal, 1.0));

    VSOutput output;
    output.csPosition = p;
    output.wsPosition = p.xyz;
    // output.normal = normalize(wsNormal.xyz);
    // output.uv = input.uv;
    return output;
}

[[shader("fragment")]]
GBufferOutput PSMain(VSOutput input)
{
    GBufferOutput output;
    output.wsPosition = float4(input.wsPosition, 1.0);
    // output.normal = float4(input.normal, 1.0f);
    // output.material = input.uv;
    return output;
}
