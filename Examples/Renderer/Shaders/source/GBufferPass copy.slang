import ShaderBindingTable;

struct GBufferInputs
{
    GPU::SceneView sceneView;
    StructuredBuffer<float4x4> transforms;
    SamplerState defaultSampler;
    Texture2D bindlessTextures[];
};
ParameterBlock<GBufferInputs> shaderInputs;

struct ShaderBindingTable22
{
    GPU::SceneView                 sceneView;
    StructuredBuffer<GPU::DrawRequest>             drawRequests;
    float4 someotherConstants;
    StructuredBuffer<GPU::StaticMeshIndexed>       indexedMeshes;
    float4 someotherConstants1;
    StructuredBuffer<float4x4>                     transforms;
    float4 someotherConstants12;
    StructuredBuffer<GPU::MeshMaterialBindless>    materials;
    RWStructuredBuffer<uint32_t>                        drawParametersCount;
    float4 someotherConstants123;

    // Pass bind points
    Texture2D                                      pass_gBuffer_wsPosition;
    Texture2D                                      pass_gBuffer_normal;;
    Texture2D                                      pass_gBuffer_material;
    Texture2D                                      pass_gBuffer_depth;

    RWStructuredBuffer<GPU::DrawIndexedParameters> outDrawParameters;

    Texture2D                                      pass_lighting_input[4];

    RWTexture2D                                    pass_compose_output;

    Texture2D                                      bindlessTextures[];
};

ParameterBlock<ShaderBindingTable22> theNewwBindingss;

struct UGBVertexInput
{
    float3 position;
    float3 normal;
    float2 uv;
    float4x4 transform;
};

struct GBufferInput
{
    float4 csPosition: SV_Position;
    float3 wsPosition: TEXCOORD1;
    float3 normal: TEXCOORD1;
    float2 uv: TEXCOORD2;
};

struct GBufferOutput
{
    float4 wsPosition : SV_Target0; // World position
    float4 normal : SV_Target1;     // Normal vector
    float2 material : SV_Target2;   // (UV + material ID + roughness + metallic)
};

[[shader("vertex")]]
GBufferInput VSMain(UGBVertexInput input)
{
    var sceneView = GetSceneView();

    float4x4 worldToViewMatrix = sceneView.worldToViewMatrix;
    float4x4 worldToClipMatrix = mul(sceneView.viewToClipMatrix, worldToViewMatrix);

    float4 wsPosition = mul(worldToViewMatrix, float4(input.position, 1.0f));
    float4 csPosition = mul(worldToClipMatrix, wsPosition);
    float3 wsNormal   = mul((float3x3)input.transform, input.normal);

    GBufferInput output;
    output.csPosition = csPosition.xyzw;
    output.wsPosition = wsPosition.xyz;
    output.normal = normalize(wsNormal);
    output.uv = input.uv;
    return output;
}

[[shader("fragment")]]
GBufferOutput PSMain(GBufferInput input,
 uniform GBufferInput inputs)
{
    GBufferOutput output;
    output.wsPosition = float4(input.wsPosition, 1.0);
    output.normal = float4(input.normal, 1.0f);
    output.material = input.uv;
    return output;
}


[[shader("compute"), numthreads(1, 1, 1)]]
void CSMain(
    uint3 threadId : SV_DispatchThreadID,
    ParameterBlock<GBufferInputs> gbuff
)
{

}