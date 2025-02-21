
// import BxDF;
import Public.GpuScene;
// import DirectLighting;

struct VertexInput
{
    float3 position;
    float3 normal;
    float2 uv;
    uint32_t4 drawId;
};

struct FragmentShaderInput
{
    float4 csPosition: SV_Position;
    float3 wsPosition: TEXCOORD1;
    float3 normal: TEXCOORD1;
    float2 uv: TEXCOORD2;
};

[[shader("vertex")]]
FragmentShaderInput VSMain(VertexInput input)
{
    // let worldToView  = scene.sceneView.worldToViewMatrix;
    // let viewToClip  = scene.sceneView.viewToClipMatrix;
    // let modelToWorld = scene.drawData.modelToWorldMatrix;
    // let modelToView = mul(modelToWorld, worldToView);
    // let modelToClip = mul(modelToView, viewToClip);

    // let position = mul(float4(input.position, 1.0), modelToClip);

    FragmentShaderInput output;
    // output.csPosition = position;
    // output.wsPosition = mul(float4(input.position, 1.0), modelToWorld).xyz;
    // output.normal = mul(float4(input.normal, 1.0f), modelToWorld).xyz;
    output.csPosition = float4(input.position, 1.0);
    output.uv = input.uv;
    return output;
}

struct GBufferOutput
{
    float4 albedo   : SV_Target0; // Albedo color
    float4 position : SV_Target2; // World position
    float4 normal   : SV_Target1; // Normal vector
    float2 material : SV_Target3; // Material properties (e.g., metallic, roughness, ...)
};

[[shader("fragment")]]
GBufferOutput PSMain(FragmentShaderInput input)
{
    GBufferOutput output;
    output.albedo = float4(input.normal * 0.5 + 0.5, 1.0);
    output.normal = float4(input.normal, 1.0);
    output.position = float4(input.wsPosition, 1.0);
    output.material.x = 0;
    output.material.y = 1;
    return output;
}
