import Lights;

static const uint32_t kMaxDirLights   = 32u;  // Maximum number of directional lights.
static const uint32_t kMaxPointLights = 32u;  // Maximum number of point lights.
static const uint32_t kMaxSpotLights  = 32u;  // Maximum number of spot lights.

struct Scene
{
    float4x4         worldToViewMatrix;               // The world-to-view matrix transforms from world space to view space.
    float4x4         viewToClipMatrix;                // The view-to-clip matrix transforms from view space to clip space.
    // float4x4         viewToWorldMatrix;            // The view-to-world matrix transforms from view space to world space.
    float3           cameraPosition;                  // The position of the camera in world space.
    float3           ambientIntensity;                // The intensity of the ambient light in the scene.
    uint32_t         numPointLights;                  // The number of point lights in the scene.
    uint32_t         numSpotLights;                   // The number of spot lights in the scene.
    DirectionalLight directionalLight[kMaxDirLights]; // The directional light in the scene.
    PointLight       pointLights[kMaxPointLights];    // The point lights in the scene.
    SpotLight        spotLights[kMaxSpotLights];      // The spot lights in the scene.
};

struct MaterialData
{
    float3       albedo;                        // The albedo color of the material.
    uint32_t     albedoMapIndex;                // The index of the albedo map texture.
    uint32_t     normalMapIndex;                // The index of the normal map texture.
    float        metallic;                      // The metallic property of the material.
    float        roughness;                     // The roughness property of the material.
    uint32_t     metallicRoughnessMapIndex;     // The index of the metallic-roughness map texture.
};

struct ModelData
{
    float4x4 modelToWorldMatrix;
    // TODO: Replace with an index to material array (save space since multiple models may share the same material)
    MaterialData material;
};

struct BindlessTextures
{
    SamplerState sampler;
    Texture2D bindlessTextures[];
};

struct ShaderParamters
{
    ConstantBuffer<Scene> sceneView;
    ConstantBuffer<ModelData> model;
};

ParameterBlock<ShaderParamters> scene;
ParameterBlock<BindlessTextures> bindless;

struct VertexInput
{
    float3 position;
    float3 normal;
    float2 uv;
};

struct MaterialPbrData
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
};

float4 GetMaterialValue(uint32_t index, float4 defaultValue, float2 uv)
{
    return index == 0xFFFFFFFF ? defaultValue : bindless.bindlessTextures[index].Sample(bindless.sampler, uv);
}

MaterialPbrData GetMaterialData(float3 vertexNormal, float2 vertexUV)
{
    let material = scene.model.material;
    let albedo = GetMaterialValue(material.albedoMapIndex, float4(material.albedo, 1.0), vertexUV);
    let normal = GetMaterialValue(material.normalMapIndex, float4(vertexNormal, 1.0), vertexUV).xyz;
    let metallicRoughness = GetMaterialValue(material.metallicRoughnessMapIndex, float4(material.metallic, material.roughness, 0.0, 0.0), vertexUV);

    MaterialPbrData materialData;
    materialData.albedo = albedo;
    materialData.normal = normal;
    materialData.metallic = metallicRoughness.b;
    materialData.roughness = metallicRoughness.g;
    return materialData;
}

float3 GetViewDirection(float3 wsPosition)
{
    float3 viewDirection = normalize(scene.sceneView.cameraPosition - wsPosition);
    return viewDirection;
}