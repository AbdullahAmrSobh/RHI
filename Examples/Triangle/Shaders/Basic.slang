import RendererInterface;
import BxDF;
import Lights;

struct FragmentShaderInput
{
    float4 csPosition: SV_Position;
    float3 wsPosition: TEXCOORD1;
    float3 normal: TEXCOORD1;
    float2 uv: TEXCOORD2;
};

[[shader("vertex")]]
FragmentShaderInput VSMain(VertexInput input, uint vertexID : SV_VertexID)
{
    let worldToView  = scene.sceneView.worldToViewMatrix;
    let viewToClip  = scene.sceneView.viewToClipMatrix;
    let modelToWorld = scene.model.modelToWorldMatrix;
    let modelToView = mul(modelToWorld, worldToView);
    let modelToClip = mul(modelToView, viewToClip);

    let position = mul(float4(input.position, 1.0), modelToClip);

    FragmentShaderInput output;
    output.csPosition = position;
    output.wsPosition = mul(float4(input.position, 1.0), modelToWorld).xyz;
    output.normal = mul(float4(input.normal, 1.0f), modelToWorld).xyz;
    output.uv = input.uv;
    return output;
}

struct GBufferOutput
{
    float4 albedo : SV_Target0;       // Albedo color
    // float3 normal : SV_Target1;       // Normal vector
    // float3 position : SV_Target2;     // World position
    // float2 material : SV_Target3;     // Material properties (e.g., metallic, roughness, ...)
};
[[shader("fragment")]]
GBufferOutput PSMain(FragmentShaderInput input)
{
    let materialData = GetMaterialData(input.normal, input.uv);
    let surface = Surface(
        input.wsPosition,
        GetViewDirection(input.wsPosition),
        materialData.normal,
        input.normal,
        materialData.albedo.rgb,
        materialData.metallic,
        materialData.roughness
    );

    float3 accumulatedLight = float3(0.0, 0.0, 0.0);

    LightData lightData = LightData(scene.sceneView.directionalLight[0]);
    let bsdf = EvaluateBSDF(surface, lightData);

    float dotProduct = dot(surface.normal, lightData.direction);
    let F = (bsdf.diffuse + bsdf.specular) * lightData.intensity * lightData.color.rgb * dotProduct;

    GBufferOutput output;
    output.albedo = float4(accumulatedLight, 1.0);
    return output;
}
