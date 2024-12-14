static const uint32_t kMaxPointLights = 32; // Maximum number of point lights.
static const uint32_t kMaxSpotLights = 32; // Maximum number of spot lights.

struct UBSceneView
{
    float4x4 worldToViewMatrix;              // The world-to-view matrix transforms from world space to view space.
    float4x4 viewToClipMatrix;               // The view-to-clip matrix transforms from view space to clip space.
    float4x4 viewToWorldMatrix;              // The view-to-world matrix transforms from view space to world space.
    float3 cameraPosition;                   // The position of the camera in world space.
    float3 ambientIntensity;                 // The intensity of the ambient light in the scene.
    DirectionalLight directionalLight;       // The directional light in the scene.
    uint32_t numPointLights;                 // The number of point lights in the scene.
    PointLight pointLights[kMaxPointLights]; // The point lights in the scene.
    uint32_t numSpotLights;                  // The number of spot lights in the scene.
    SpotLight spotLights[kMaxSpotLights];    // The spot lights in the scene.
};

struct Material
{
    float3 albedo;                      // The albedo color of the material.
    uint32_t albedoMapIndex;            // The index of the albedo map texture.
    uint32_t normalMapIndex;            // The index of the normal map texture.
    float metallic;                     // The metallic property of the material.
    float roughness;                    // The roughness property of the material.
    uint32_t metallicRoughnessMapIndex; // The index of the metallic-roughness map texture.
};

struct UBModel
{
    float4x4 modelToWorldMatrix;        //
    float3 albedo;                      // The albedo color of the material.
    uint32_t albedoMapIndex;            // The index of the albedo map texture.
    uint32_t normalMapIndex;            // The index of the normal map texture.
    float metallic;                     // The metallic property of the material.
    float roughness;                    // The roughness property of the material.
    uint32_t metallicRoughnessMapIndex; // The index of the metallic-roughness map texture.
};

struct ShaderParamters
{
    ConstantBuffer<UBSceneView> scene;

    ConstantBuffer<UBModel> model;

    SamplerState sampler;

    Texture2D bindlessTextures[];
};

ParameterBlock<ShaderParamters> drawData;

struct VertexInput
{
	float3 position;
    float3 normal;
    float2 uv;
};

struct FragmentShaderInput
{
    float4 csPosition: SV_Position;
    float3 position: TEXCOORD0;
    float3 normal: TEXCOORD1;
    float2 uv: TEXCOORD2;
};

[[shader("vertex")]]
FragmentShaderInput VSMain(VertexInput input)
{
    float4 worldPosition = mul(drawData.model.modelToWorldMatrix, float4(input.position, 1.0));
    float4 viewPosition = mul(drawData.scene.worldToViewMatrix, worldPosition);
    float4 clipPosition = mul(drawData.scene.viewToClipMatrix, viewPosition);
    FragmentShaderInput output;
    output.csPosition = clipPosition;
    output.position = worldPosition.xyz;
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

struct PSOutput
{
    float4 color : SV_Target;
};

import Lighting;
import BxDF;

MaterialSurface GetMaterialSurface(FragmentShaderInput input)
{
    MaterialSurface materialSurface;
    materialSurface.viewDirection = normalize(drawData.scene.cameraPosition - input.position);
    materialSurface.wsPosition = input.position;
    materialSurface.albedo = drawData.model.albedo;
    materialSurface.normal = normalize(input.normal);
    materialSurface.metallic = drawData.model.metallic;
    materialSurface.roughness = drawData.model.roughness;
    // if (drawData.model.albedoMapIndex != 0xFFFFFFFF)
    // {
    //     materialSurface.albedo = drawData.bindlessTextures[drawData.model.albedoMapIndex].Sample(drawData.sampler, input.uv).xyz;
    // }
    // if (drawData.model.normalMapIndex != 0xFFFFFFFF)
    // {
    //     materialSurface.normal = normalize(drawData.bindlessTextures[drawData.model.normalMapIndex].Sample(drawData.sampler, input.uv).xyz * 2.0f - 1.0f);
    // }
    // if (drawData.model.metallicRoughnessMapIndex != 0xFFFFFFFF)
    // {
        // float4 metallicRoughness = drawData.bindlessTextures[drawData.model.metallicRoughnessMapIndex].Sample(drawData.sampler, input.uv);
        // materialSurface.metallic = metallicRoughness.x;
        // materialSurface.roughness = metallicRoughness.y;
    // }
    return materialSurface;
}

[[shader("fragment")]]
PSOutput PSMain(FragmentShaderInput input)
{
    const let viewDirection = input.position - drawData.scene.cameraPosition;

    const let material = GetMaterialSurface(input);

    const let c = BxDFContext(input.normal, viewDirection, drawData.scene.directionalLight.direction);

    const let diffuseColor = float4(input.normal * 0.5 + 0.5, 1.0) * material.roughness;

    let finalColor = drawData.scene.directionalLight.color.rgb * DiffuseLambertian(c, diffuseColor.rgb) * c.nDotL;

    PSOutput output;
    output.color = float4(finalColor, 1.0);
    return output;
}
