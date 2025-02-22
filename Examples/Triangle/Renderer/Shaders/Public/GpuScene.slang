#include "GpuScene.h"

struct ShaderParamters
{
    ConstantBuffer<SceneView>         scene;
    StructuredBuffer<F32_3x4>     objects;
    StructuredBuffer<PbrMaterial> materials;
    SamplerState                  sampler;
    Texture2D                     textures[];
};

ParameterBlock<ShaderParamters> bg;

struct MaterialPbrData
{
    float4 albedo;
    float3 normal;
    float metallic;
    float roughness;
};

float3 GetViewDirection()
{
    return normalize(bg.scene.worldToViewMatrix[3].xyz);
}

float4 GetMaterialValue(uint32_t index, float4 defaultValue, float2 uv)
{
    return index == 0xFFFFFFFF ? defaultValue : bg.textures[index].Sample(bg.sampler, uv);
}

MaterialPbrData GetMaterialData(float3 vertexNormal, float2 vertexUV)
{
    let material = bg.materials[0];
    let albedo = GetMaterialValue(material.albedoMapIndex, float4(material.albedo.rgb, 1.0), vertexUV);
    let normal = GetMaterialValue(material.normalMapIndex, float4(vertexNormal, 1.0), vertexUV).xyz;
    let metallicRoughness = GetMaterialValue(material.metallicRoughnessMapIndex, float4(material.metallic, material.roughness, 0.0, 0.0), vertexUV);

    MaterialPbrData materialData;
    materialData.albedo = albedo;
    materialData.normal = normal;
    materialData.metallic = metallicRoughness.b;
    materialData.roughness = metallicRoughness.g;
    return materialData;
}
